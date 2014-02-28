#include <assert.h>
#include <string.h>
#include <stddef.h>

#include "os_mutex.h"
#include "os_rwlock.h"
#include "os_cond.h"
#include "os_heap.h"
#include "os_defs.h"
#include "os_stdlib.h"
#include "os_socket.h"

#include "v_group.h"
#include "v_partition.h"
#include "v_groupSet.h"
#include "v_entity.h"

#include "q_entity.h"
#include "q_config.h"
#include "q_time.h"
#include "q_misc.h"
#include "q_log.h"
#include "ut_avl.h"
#include "q_whc.h"
#include "q_plist.h"
#include "q_lease.h"
#include "q_osplser.h" /* for topics */
#include "q_qosmatch.h"
#include "q_groupset.h"
#include "q_ephash.h"
#include "q_globals.h"
#include "q_addrset.h"
#include "q_xevent.h" /* qxev_spdp, &c. */
#include "q_ddsi_discovery.h" /* spdp_write, &c. */
#include "q_gc.h"
#include "q_radmin.h"
#include "q_protocol.h" /* NN_ENTITYID_... */
#include "q_unused.h"
#include "q_fill_msg_qos.h"
#include "q_error.h"

#include "sysdeps.h"

struct deleted_participant {
  ut_avlNode_t avlnode;
  nn_guid_t guid;
  os_int64 t_insert;
};

static os_mutex deleted_participants_lock;
static ut_avlTree_t deleted_participants;

static int compare_guid (const void *va, const void *vb);
static void augment_wr_prd_match (void *vnode, const void *vleft, const void *vright);

const ut_avlTreedef_t wr_readers_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct wr_prd_match, avlnode), offsetof (struct wr_prd_match, prd_guid), compare_guid, augment_wr_prd_match);
const ut_avlTreedef_t rd_writers_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct rd_pwr_match, avlnode), offsetof (struct rd_pwr_match, pwr_guid), compare_guid, 0);
const ut_avlTreedef_t pwr_readers_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct pwr_rd_match, avlnode), offsetof (struct pwr_rd_match, rd_guid), compare_guid, 0);
const ut_avlTreedef_t prd_writers_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct prd_wr_match, avlnode), offsetof (struct prd_wr_match, wr_guid), compare_guid, 0);
const ut_avlTreedef_t deleted_participants_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct deleted_participant, avlnode), offsetof (struct deleted_participant, guid), compare_guid, 0);

static const unsigned builtin_writers_besmask =
  NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
  NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER |
  NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER |
  NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;

static int new_writer_guid (const struct nn_guid *guid, struct participant *pp, const struct topic *topic, const struct nn_xqos *xqos);
static int new_reader_guid (const struct nn_guid *guid, struct participant *pp, const struct topic *topic, const struct nn_xqos *xqos, deliver_cb_t deliver_cb, void *deliver_cbarg);
static struct participant *ref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity);
static void unref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity);

static int gcreq_participant (struct participant *pp);
static int gcreq_writer (struct writer *wr);
static int gcreq_reader (struct reader *rd);
static int gcreq_proxy_participant (struct proxy_participant *proxypp);
static int gcreq_proxy_writer (struct proxy_writer *pwr);
static int gcreq_proxy_reader (struct proxy_reader *prd);

static int compare_guid (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (nn_guid_t));
}

nn_entityid_t to_entityid (unsigned u)
{
  nn_entityid_t e;
  e.u = u;
  return e;
}

int is_writer_entityid (nn_entityid_t id)
{
  switch (id.u & NN_ENTITYID_KIND_MASK)
  {
    case NN_ENTITYID_KIND_WRITER_WITH_KEY:
    case NN_ENTITYID_KIND_WRITER_NO_KEY:
      return 1;
    default:
      return 0;
  }
}

int is_reader_entityid (nn_entityid_t id)
{
  switch (id.u & NN_ENTITYID_KIND_MASK)
  {
    case NN_ENTITYID_KIND_READER_WITH_KEY:
    case NN_ENTITYID_KIND_READER_NO_KEY:
      return 1;
    default:
      return 0;
  }
}

int is_builtin_entityid (nn_entityid_t id)
{
  return (id.u & NN_ENTITYID_SOURCE_MASK) == NN_ENTITYID_SOURCE_BUILTIN;
}

static int is_builtin_endpoint (nn_entityid_t id)
{
  return is_builtin_entityid (id) && id.u != NN_ENTITYID_PARTICIPANT;
}

static void entity_common_init (struct entity_common *e, const struct nn_guid *guid, enum entity_kind kind)
{
  e->guid = *guid;
  e->kind = kind;
  os_mutexInit (&e->lock, &gv.mattr);
}

static void entity_common_fini (struct entity_common *e)
{
  os_mutexDestroy (&e->lock);
}

/* DELETED PARTICIPANTS --------------------------------------------- */

int deleted_participants_admin_init (void)
{
  os_mutexInit (&deleted_participants_lock, &gv.mattr);
  ut_avlInit (&deleted_participants_treedef, &deleted_participants);
  return 0;
}

void deleted_participants_admin_fini (void)
{
  ut_avlFree (&deleted_participants_treedef, &deleted_participants, os_free);
  os_mutexDestroy (&deleted_participants_lock);
}

static void prune_deleted_participant_guids (os_int64 tnow)
{
  /* Could do a better job of finding prunable ones efficiently under
     all circumstances, but I expect the tree to be very small at all
     times, so a full scan is fine, too ... */
  os_int64 tprune = tnow - 10000 * T_MILLISECOND;
  struct deleted_participant *dpp;
  os_mutexLock (&deleted_participants_lock);
  dpp = ut_avlFindMin (&deleted_participants_treedef, &deleted_participants);
  while (dpp)
  {
    struct deleted_participant *dpp1 = ut_avlFindSucc (&deleted_participants_treedef, &deleted_participants, dpp);
    if (dpp->t_insert < tprune)
    {
      ut_avlDelete (&deleted_participants_treedef, &deleted_participants, dpp);
      os_free (dpp);
    }
    dpp = dpp1;
  }
  os_mutexUnlock (&deleted_participants_lock);
}

static void remember_deleted_participant_guid (const struct nn_guid *guid)
{
  struct deleted_participant *n;
  ut_avlIPath_t path;
  os_mutexLock (&deleted_participants_lock);
  if (ut_avlLookupIPath (&deleted_participants_treedef, &deleted_participants, guid, &path) == NULL)
  {
    if ((n = os_malloc (sizeof (*n))) != NULL)
    {
      n->guid = *guid;
      n->t_insert = now ();
      ut_avlInsertIPath (&deleted_participants_treedef, &deleted_participants, n, &path);
    }
  }
  os_mutexUnlock (&deleted_participants_lock);
}

int is_deleted_participant_guid (const struct nn_guid *guid)
{
  int known;
  os_mutexLock (&deleted_participants_lock);
  known = (ut_avlLookup (&deleted_participants_treedef, &deleted_participants, guid) != NULL);
  os_mutexUnlock (&deleted_participants_lock);
  return known;
}

static void remove_deleted_participant_guid (const struct nn_guid *guid)
{
  struct deleted_participant *n;
  os_mutexLock (&deleted_participants_lock);
  if ((n = ut_avlLookup (&deleted_participants_treedef, &deleted_participants, guid)) != NULL)
  {
    ut_avlDelete (&deleted_participants_treedef, &deleted_participants, n);
    os_free (n);
  }
  os_mutexUnlock (&deleted_participants_lock);
}

/* DYNAMIC GROUP CREATION FOR WILDCARD ENDPOINTS -------------------- */

static void create_a_group (const char *name, const struct topic *topic)
{
  v_partitionQos pqos;
  v_partition part;
  v_group group;
  nn_xqos_t dummy_part_xqos;

  ASSERT_RDLOCK_HELD (&gv.qoslock);

  /* any non-wildcard one will do */
  /* create it -- partitions require a v_partitionQos parameter, but
   that (thankfully!) isn't used by it ... phew! */
  TRACE (("create_a_group: %s.%s\n", name, topic_name (topic)));
  memset (&pqos, 0, sizeof (pqos));
  part = v_partitionNew (gv.ospl_kernel, name, pqos);
  group = v_groupSetCreate (gv.ospl_kernel->groupSet, part, topic_ospl_topic (topic));
  /* Assuming the kernel will send a group creation event to all
   interested parties, which includes our own local discovery -- so
   that'll then notify the kernel we are interested in it.

   Nonetheless, there are the local readers' "matching_groups" to be
   updated, and, consequently, the local proxy-writers' "groups",
   too. Eventually all this will have to be cleaned up, it'll be
   really too messy and time consuming once qos changes are allowed. */
  nn_xqos_init_empty (&dummy_part_xqos);
  dummy_part_xqos.present = QP_PARTITION;
  dummy_part_xqos.partition.n = 1;
  dummy_part_xqos.partition.strs = (char **) &name;

  {
    struct ephash_enum_reader est_rd;
    struct ephash_enum_proxy_writer est_pwr;
    struct proxy_writer *pwr;
    struct reader *rd;
    ephash_enum_reader_init (&est_rd);
    while ((rd = ephash_enum_reader_next (&est_rd)) != NULL)
    {
      if (rd->topic == topic && partitions_match_p (rd->xqos, &dummy_part_xqos))
        nn_groupset_add_group (rd->matching_groups, group);
    }
    ephash_enum_reader_fini (&est_rd);
    ephash_enum_proxy_writer_init (&est_pwr);
    while ((pwr = ephash_enum_proxy_writer_next (&est_pwr)) != NULL)
    {
      if (pwr->c.topic == topic && partitions_match_p (pwr->c.xqos, &dummy_part_xqos))
        nn_groupset_add_group (pwr->groups, group);
    }
    ephash_enum_proxy_writer_fini (&est_pwr);
  }
  /* Release resources taken by dummy_part_xqos -- that is, need to do
   nothing as long as everything in it aliases something temporary.
   The trouble is that nn_xqos_fini() always wants to free "strs",
   and whatever "strs" points to as well if the aliased bit isn't
   set.  So this is but a reminder. */
}

/* PARTICIPANT ------------------------------------------------------ */

static int pp_allocate_entityid (nn_entityid_t *id, unsigned kind, struct participant *pp)
{
  os_mutexLock (&pp->e.lock);
  if (pp->next_entityid + NN_ENTITYID_ALLOCSTEP < pp->next_entityid)
  {
    os_mutexUnlock (&pp->e.lock);
    return ERR_OUT_OF_IDS;
  }
  *id = to_entityid (pp->next_entityid | kind);
  pp->next_entityid += NN_ENTITYID_ALLOCSTEP;
  os_mutexUnlock (&pp->e.lock);
  return 0;
}

int new_participant_guid (const nn_guid_t *ppguid, unsigned flags)
{
  struct participant *pp;
  nn_guid_t subguid;
  os_int64 tnow;

  /* no reserved bits may be set */
  assert ((flags & ~(RTPS_PF_NO_BUILTIN_READERS | RTPS_PF_NO_BUILTIN_WRITERS | RTPS_PF_PRIVILEGED_PP)) == 0);
  /* privileged participant MUST have builtin readers and writers */
  assert (!(flags & RTPS_PF_PRIVILEGED_PP) || (flags & (RTPS_PF_NO_BUILTIN_READERS | RTPS_PF_NO_BUILTIN_WRITERS)) == 0);

  tnow = now ();
  prune_deleted_participant_guids (tnow);

  /* FIXME: FULL LOCKING AROUND NEW_XXX FUNCTIONS, JUST SO EXISTENCE TESTS ARE PRECISE */

  /* Participant may not exist yet, but this test is imprecise: if it
     used to exist, but is currently being deleted and we're trying to
     recreate it. */
  if (ephash_lookup_participant_guid (ppguid) != NULL)
    return ERR_ENTITY_EXISTS;

  if (config.max_participants == 0)
  {
    os_mutexLock (&gv.participant_set_lock);
    ++gv.nparticipants;
    os_mutexUnlock (&gv.participant_set_lock);
  }
  else
  {
    os_mutexLock (&gv.participant_set_lock);
    if (gv.nparticipants < config.max_participants)
    {
      ++gv.nparticipants;
      os_mutexUnlock (&gv.participant_set_lock);
    }
    else
    {
      os_mutexUnlock (&gv.participant_set_lock);
      NN_ERROR2 ("new_participant(%x:%x:%x:%x, %x) failed: max participants reached\n", PGUID (*ppguid), flags);
      return ERR_OUT_OF_IDS;
    }
  }

  nn_log (LC_DISCOVERY, "new_participant(%x:%x:%x:%x, %x)\n", PGUID (*ppguid), flags);

  pp = os_malloc (sizeof (*pp));

  entity_common_init (&pp->e, ppguid, EK_PARTICIPANT);
  pp->user_refc = 1;
  pp->builtin_refc = 0;
  pp->builtins_deleted = 0;
  os_mutexInit (&pp->refc_lock, &gv.mattr);
  pp->next_entityid = NN_ENTITYID_ALLOCSTEP;
  pp->lease_duration = config.lease_duration; /* Default is 100s as per spec */

  if (config.many_sockets_mode)
  {
    pp->m_conn = ddsi_factory_create_conn (gv.m_factory, 0, NULL);
    ddsi_conn_locator (pp->m_conn, &pp->m_locator);
  }

  /* Before we create endpoints -- and may call unref_participant if
     things go wrong -- we must initialize all that unref_participant
     depends on. */
  pp->spdp_xevent = NULL;
  pp->pmd_update_xevent = NULL;

  /* Create built-in endpoints (note: these have no GID). */
  pp->bes = 0;
  subguid.prefix = pp->e.guid.prefix;
  /* SPDP writer */
#if 0 /* SPDP writer incurs no cost */
  /* Note: skip SEDP <=> skip SPDP because of the way ddsi_discovery.c does things
     currently.  */
  if (!(flags & RTPS_PF_NO_BUILTIN_WRITERS))
#endif
  {
    subguid.entityid = to_entityid (NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
    if (new_writer_guid (&subguid, pp, NULL, &gv.spdp_endpoint_xqos) < 0)
      goto err_builtin_endpoint;
    /* But we need the as_disc address set for SPDP, because we need to
       send it to everyone regardless of the existence of readers. */
    {
      struct writer *wr = ephash_lookup_writer_guid (&subguid);
      assert (wr != NULL);
      unref_addrset (wr->as);
      wr->as = ref_addrset (gv.as_disc);
    }
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
  }

  /* Make it globally visible, else the endpoint matching won't work. */
  ephash_insert_participant_guid (pp);

  /* SEDP writers: */
#if 0 /* Can't have SEDP data arriving before SPDP data (FIXME: that's fixable) */
  /* Note: skip SEDP <=> skip SPDP because of the way ddsi_discovery.c does things
     currently.  */
  if (!(flags & RTPS_PF_NO_BUILTIN_WRITERS))
#endif
  {
    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER);
    if (new_writer_guid (&subguid, pp, NULL, &gv.builtin_endpoint_xqos_wr) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER);
    if (new_writer_guid (&subguid, pp, NULL, &gv.builtin_endpoint_xqos_wr) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
  }

  /* PMD writer: */
#if 0 /* PMD should be ok */
  if (!(flags & RTPS_PF_NO_BUILTIN_WRITERS))
#endif
  {
    subguid.entityid = to_entityid (NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER);
    if (new_writer_guid (&subguid, pp, NULL, &gv.builtin_endpoint_xqos_wr) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
  }

  /* SPDP, SEDP, PMD readers: */
  if (!(flags & RTPS_PF_NO_BUILTIN_READERS))
  {
    subguid.entityid = to_entityid (NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER);
    if (new_reader_guid (&subguid, pp, NULL, &gv.spdp_endpoint_xqos, 0, NULL) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
    if (new_reader_guid (&subguid, pp, NULL, &gv.builtin_endpoint_xqos_rd, 0, NULL) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);
    if (new_reader_guid (&subguid, pp, NULL, &gv.builtin_endpoint_xqos_rd, 0, NULL) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;

    subguid.entityid = to_entityid (NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER);
    if (new_reader_guid (&subguid, pp, NULL, &gv.builtin_endpoint_xqos_rd, 0, NULL) < 0)
      goto err_builtin_endpoint;
    pp->bes |= NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
  }

  /* If the participant doesn't have the full set of builtin writers
     it depends on the privileged participant, which must exist, hence
     the reference count of the privileged participant is incremented.
     If it is the privileged participant, set the global variable
     pointing to it. */
  os_mutexLock (&gv.privileged_pp_lock);
  if ((pp->bes & builtin_writers_besmask) != builtin_writers_besmask)
  {
    /* Simply crash when the privileged participant doesn't exist when
       it is needed.  Its existence is a precondition, and this is not
       a public API */
    assert (gv.privileged_pp != NULL);
    ref_participant (gv.privileged_pp, &pp->e.guid);
  }
  if (flags & RTPS_PF_PRIVILEGED_PP)
  {
    /* Crash when two privileged participants are created -- this is
       not a public API. */
    assert (gv.privileged_pp == NULL);
    gv.privileged_pp = pp;
  }
  os_mutexUnlock (&gv.privileged_pp_lock);

  /* Make it globally visible, then signal receive threads if
     necessary. Must do in this order, or the receive thread won't
     find the new participant */

  if (config.many_sockets_mode)
  {
    pa_membar_producer ();
    atomic_inc_u32_nv (&gv.participant_set_generation);
    os_sockWaitsetTrigger (gv.waitset);
  }

  /* SPDP periodic broadcast uses the retransmit path, so the initial
     publication must be done differently. Must be later than making
     the participant globally visible, or the SPDP processing won't
     recognise the participant as a local one. */
  if (spdp_write (pp) >= 0)
  {
    /* Once the initial sample has been written, the automatic and
       asynchronous broadcasting required by SPDP can start. Also,
       since we're new alive, PMD updates can now start, too. Note:
       these two will sometimes fire before the calls return.  If the
       initial sample wasn't accepted, all is lost, but we continue
       nonetheless, even though the participant won't be able to
       discover or be discovered.  */
    pp->spdp_xevent = qxev_spdp (config.spdp_interval, &pp->e.guid, NULL);
  }

  pp->pmd_update_xevent = qxev_pmd_update ((pp->lease_duration == T_NEVER) ? T_NEVER : 0, &pp->e.guid);
  return 0;

err_builtin_endpoint:

  /* We haven't yet counted a reference, so better pretend the writers
     exist even if we haven't tried to create them ... */

  pp->bes |= builtin_writers_besmask;
  unref_participant (pp, NULL);

  return ERR_UNSPECIFIED;
}


static void delete_builtin_endpoint (const struct nn_guid *ppguid, unsigned entityid)
{
  nn_guid_t guid;
  guid.prefix = ppguid->prefix;
  guid.entityid.u = entityid;
  assert (is_builtin_entityid (to_entityid (entityid)));
  if (is_writer_entityid (to_entityid (entityid)))
    delete_writer_nolinger (&guid);
  else
    delete_reader (&guid);
}

static struct participant *ref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity)
{
  nn_guid_t stguid;
  os_mutexLock (&pp->refc_lock);
  if (guid_of_refing_entity && is_builtin_endpoint (guid_of_refing_entity->entityid))
    pp->builtin_refc++;
  else
    pp->user_refc++;

  if (guid_of_refing_entity)
    stguid = *guid_of_refing_entity;
  else
    memset (&stguid, 0, sizeof (stguid));
  TRACE (("ref_participant(%x:%x:%x:%x @ %p <- %x:%x:%x:%x @ %p) user %d builtin %d\n",
          PGUID (pp->e.guid), pp, PGUID (stguid), guid_of_refing_entity, pp->user_refc, pp->builtin_refc));
  os_mutexUnlock (&pp->refc_lock);
  return pp;
}

static void unref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity)
{
  static const unsigned builtin_endpoints_tab[] = {
    NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER,
    NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER,
    NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER,
    NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER,
    NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER,
    NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER
  };
  nn_guid_t stguid;

  os_mutexLock (&pp->refc_lock);
  if (guid_of_refing_entity && is_builtin_endpoint (guid_of_refing_entity->entityid))
    pp->builtin_refc--;
  else
    pp->user_refc--;
  assert (pp->user_refc >= 0);
  assert (pp->builtin_refc >= 0);

  if (guid_of_refing_entity)
    stguid = *guid_of_refing_entity;
  else
    memset (&stguid, 0, sizeof (stguid));
  TRACE (("unref_participant(%x:%x:%x:%x @ %p <- %x:%x:%x:%x @ %p) user %d builtin %d\n",
          PGUID (pp->e.guid), pp, PGUID (stguid), guid_of_refing_entity, pp->user_refc, pp->builtin_refc));

  if (pp->user_refc == 0 && pp->bes != 0 && !pp->builtins_deleted)
  {
    int i;

    /* The builtin ones are never deleted explicitly by the glue code,
       only implicitly by unref_participant, and we need to make sure
       they go at the very end, or else the SEDP disposes and the
       final SPDP message can't be sent.

       If there are no builtins at all, then we must go straight to
       deleting the participant, as unref_participant will never be
       called upon deleting a builtin endpoint.

       First we stop the asynchronous SPDP and PMD publication, then
       we send a dispose+unregister message on SPDP (wonder if I ought
       to send a final PMD one as well), then we kill the readers and
       expect us to finally hit the new_refc == 0 to really free this
       participant.

       The conditional execution of some of this is so we can use
       unref_participant() for some of the error handling in
       new_participant(). Non-existent built-in endpoints can't be
       found in guid_hash and are simply ignored. */
    pp->builtins_deleted = 1;
    os_mutexUnlock (&pp->refc_lock);

    if (pp->spdp_xevent)
      delete_xevent (pp->spdp_xevent);
    if (pp->pmd_update_xevent)
      delete_xevent (pp->pmd_update_xevent);

    /* SPDP relies on the WHC, but dispose-unregister will empty
       it. The event handler verifies the event has already been
       scheduled for deletion when it runs into an empty WHC */
    spdp_dispose_unregister (pp);

    /* If this happens to be the privileged_pp, clear it */
    os_mutexLock (&gv.privileged_pp_lock);
    if (pp == gv.privileged_pp)
      gv.privileged_pp = NULL;
    os_mutexUnlock (&gv.privileged_pp_lock);

    for (i = 0; i < (int) (sizeof (builtin_endpoints_tab) / sizeof (builtin_endpoints_tab[0])); i++)
      delete_builtin_endpoint (&pp->e.guid, builtin_endpoints_tab[i]);
  }
  else if (pp->user_refc == 0 && pp->builtin_refc == 0)
  {
    os_mutexUnlock (&pp->refc_lock);

    if ((pp->bes & builtin_writers_besmask) != builtin_writers_besmask)
    {
      /* Participant doesn't have a full complement of built-in
         writers, therefore, it relies on gv.privileged_pp, and
         therefore we must decrement the reference count of that one.

         Why read it with the lock held, only to release it and use it
         without any attempt to maintain a consistent state?  We DO
         have a counted reference, so it can't be freed, but there is
         no formal guarantee that the pointer we read is valid unless
         we read it with the lock held.  We can't keep the lock across
         the unref_participant, because we may trigger a clean-up of
         it.  */
      struct participant *ppp;
      os_mutexLock (&gv.privileged_pp_lock);
      ppp = gv.privileged_pp;
      os_mutexUnlock (&gv.privileged_pp_lock);
      assert (ppp != NULL);
      unref_participant (ppp, &pp->e.guid);
    }

    os_mutexLock (&gv.participant_set_lock);
    assert (gv.nparticipants > 0);
    if (--gv.nparticipants == 0)
      os_condBroadcast (&gv.participant_set_cond);
    os_mutexUnlock (&gv.participant_set_lock);
    if (config.many_sockets_mode)
    {
      pa_membar_producer ();
      atomic_inc_u32_nv (&gv.participant_set_generation);

      /* Deleting the socket will usually suffice to wake up the
         receiver threads, but in general, no one cares if it takes a
         while longer for it to wakeup. */

      ddsi_conn_free (pp->m_conn);
    }
    os_mutexDestroy (&pp->refc_lock);
    entity_common_fini (&pp->e);
    /* not calling remove_deleted_participant_guid because we want to
       suppress looped-back SPDP packets a while longer */
    os_free (pp);
  }
  else
  {
    os_mutexUnlock (&pp->refc_lock);
  }
}

static void gc_delete_participant (struct gcreq *gcreq)
{
  struct participant *pp = gcreq->arg;
  gcreq_free (gcreq);
  unref_participant (pp, NULL);
}

int delete_participant (const struct nn_guid *ppguid)
{
  struct participant *pp;
  if ((pp = ephash_lookup_participant_guid (ppguid)) == NULL)
    return ERR_UNKNOWN_ENTITY;
  remember_deleted_participant_guid (&pp->e.guid);
  ephash_remove_participant_guid (pp);
  gcreq_participant (pp);
  return 0;
}

struct writer *get_builtin_writer (const struct participant *pp, unsigned entityid)
{
  nn_guid_t bwr_guid;
  unsigned bes_mask;

  /* If the participant the required built-in writer, we use it.  We
     check by inspecting the "built-in endpoint set" advertised by the
     participant, which is a constant. */
  switch (entityid)
  {
    case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
      bes_mask = NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
      bes_mask = NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
      bes_mask = NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
      break;
    case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER:
      bes_mask = NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
      break;
    default:
      NN_FATAL1 ("get_builtin_writer called with entityid %x\n", entityid);
      return NULL;
  }

  if (pp->bes & bes_mask)
  {
    /* Participant has this SEDP writer => use it. */
    bwr_guid.prefix = pp->e.guid.prefix;
    bwr_guid.entityid.u = entityid;
  }
  else
  {
    /* Must have a designated participant to use -- that is, before
       any application readers and writers may be created (indeed,
       before any PMD message may go out), one participant must be
       created with the built-in writers, and this participant then
       automatically becomes the designated participant.  Woe betide
       who deletes it early!  Lock's not really needed but provides
       the memory barriers that guarantee visibility of the correct
       value of privileged_pp. */
    os_mutexLock (&gv.privileged_pp_lock);
    assert (gv.privileged_pp != NULL);
    bwr_guid.prefix = gv.privileged_pp->e.guid.prefix;
    os_mutexUnlock (&gv.privileged_pp_lock);
    bwr_guid.entityid.u = entityid;
  }

  return ephash_lookup_writer_guid (&bwr_guid);
}

/* WRITER/READER/PROXY-WRITER/PROXY-READER CONNECTION ---------------

   These are all located in a separate section because they are so
   very similar that co-locating them eases editing and checking. */

static void rebuild_writer_addrset_addone_mc (struct addrset *as, const struct wr_prd_match *m)
{
  /* prd->c.as better be constant, or else we need to do this sort of
     thing also when the proxy reader's address set changes */
  struct proxy_reader *prd;
  os_sockaddr_storage addr;
  if ((prd = ephash_lookup_proxy_reader_guid (&m->prd_guid)) != NULL)
  {
    if (addrset_any_mc (prd->c.as, &addr))
      add_to_addrset (as, &addr);
    else if (addrset_any_uc (prd->c.as, &addr))
      add_to_addrset (as, &addr);
  }
}

static void rebuild_writer_addrset_addone_uc (struct addrset *as, const struct wr_prd_match *m)
{
  struct proxy_reader *prd;
  os_sockaddr_storage addr;
  if ((prd = ephash_lookup_proxy_reader_guid (&m->prd_guid)) != NULL)
  {
    if (addrset_any_uc (prd->c.as, &addr))
      add_to_addrset (as, &addr);
  }
}

static void rebuild_writer_addrset (struct writer *wr)
{
  /* FIXME way too inefficient in this form */
  struct addrset *newas = new_addrset ();
  struct addrset *oldas = wr->as;

  /* only one operation at a time */
  ASSERT_MUTEX_HELD (&wr->e.lock);

  /* compute new addrset */
  if (!ut_avlIsEmpty (&wr->readers))
  {
    /* FIXME: ugly hack to use unicast if there is just one reader,
       else prefer multicast. */
    if (ut_avlIsSingleton (&wr->readers))
      rebuild_writer_addrset_addone_uc (newas, ut_avlRoot (&wr_readers_treedef, &wr->readers));
    else
    {
      struct wr_prd_match *m;
      for (m = ut_avlFindMin (&wr_readers_treedef, &wr->readers); m; m = ut_avlFindSucc (&wr_readers_treedef, &wr->readers, m))
        rebuild_writer_addrset_addone_mc (newas, m);
    }
  }

  /* swap in new address set; this simple procedure is ok as long as
     wr->as is never accessed without the wr->e.lock held */
  wr->as = newas;
  unref_addrset (oldas);

  TRACE (("rebuild_writer_addrset(%x:%x:%x:%x):", PGUID (wr->e.guid)));
  nn_log_addrset (LC_TRACE, "", wr->as);
  TRACE (("\n"));
}

static void free_wr_prd_match (struct wr_prd_match *m)
{
  if (m)
  {
    nn_lat_estim_fini (&m->hb_to_ack_latency);
    os_free (m);
  }
}

static void free_rd_pwr_match (struct rd_pwr_match *m)
{
  if (m) os_free (m);
}

static void free_pwr_rd_match (struct pwr_rd_match *m)
{
  if (m)
  {
    if (m->acknack_xevent)
      delete_xevent (m->acknack_xevent);
    nn_reorder_free (m->u.not_in_sync.reorder);
    os_free (m);
  }
}

static void free_prd_wr_match (struct prd_wr_match *m)
{
  if (m) os_free (m);
}

static void writer_drop_connection (const struct nn_guid *wr_guid, const struct nn_guid *prd_guid)
{
  struct writer *wr;
  if ((wr = ephash_lookup_writer_guid (wr_guid)) != NULL)
  {
    struct wr_prd_match *m;
    os_mutexLock (&wr->e.lock);
    if ((m = ut_avlLookup (&wr_readers_treedef, &wr->readers, prd_guid)) != NULL)
    {
      ut_avlDelete (&wr_readers_treedef, &wr->readers, m);
      rebuild_writer_addrset (wr);
      remove_acked_messages (wr);
      wr->num_reliable_readers -= m->is_reliable;
    }
    os_mutexUnlock (&wr->e.lock);
    free_wr_prd_match (m);
  }
}

static void reader_drop_connection (const struct nn_guid *rd_guid, const struct nn_guid *pwr_guid)
{
  struct reader *rd;
  if ((rd = ephash_lookup_reader_guid (rd_guid)) != NULL)
  {
    struct rd_pwr_match *m;
    os_mutexLock (&rd->e.lock);
    if ((m = ut_avlLookup (&rd_writers_treedef, &rd->writers, pwr_guid)) != NULL)
      ut_avlDelete (&rd_writers_treedef, &rd->writers, m);
    os_mutexUnlock (&rd->e.lock);
    free_rd_pwr_match (m);
  }
}

static void update_reader_init_acknack_count (const struct nn_guid *rd_guid, nn_count_t count)
{
  struct reader *rd;

  /* Update the initial acknack sequence number for the reader.  See
     also reader_add_connection(). */
  TRACE (("update_reader_init_acknack_count (%x:%x:%x:%x, %d): ", PGUID (*rd_guid), count));
  if ((rd = ephash_lookup_reader_guid (rd_guid)) != NULL)
  {
    os_mutexLock (&rd->e.lock);
    TRACE (("%d -> ", rd->init_acknack_count));
    if (count > rd->init_acknack_count)
      rd->init_acknack_count = count;
    TRACE (("%d\n", count));
    os_mutexUnlock (&rd->e.lock);
  }
  else
  {
    TRACE (("reader no longer exists\n"));
  }
}

static void proxy_writer_drop_connection (const struct nn_guid *pwr_guid, struct reader *rd)
{
  /* Only called by gc_delete_reader, so we actually have a reader pointer */
  struct proxy_writer *pwr;
  if ((pwr = ephash_lookup_proxy_writer_guid (pwr_guid)) != NULL)
  {
    struct pwr_rd_match *m;

    os_mutexLock (&pwr->e.lock);
    if ((m = ut_avlLookup (&pwr_readers_treedef, &pwr->readers, &rd->e.guid)) != NULL)
      ut_avlDelete (&pwr_readers_treedef, &pwr->readers, m);


    os_mutexUnlock (&pwr->e.lock);
    if (m != NULL)
      update_reader_init_acknack_count (&rd->e.guid, m->count);
    free_pwr_rd_match (m);
  }
}

static void proxy_reader_drop_connection (const struct nn_guid *prd_guid, const struct nn_guid *wr_guid)
{
  struct proxy_reader *prd;
  if ((prd = ephash_lookup_proxy_reader_guid (prd_guid)) != NULL)
  {
    struct prd_wr_match *m;
    os_mutexLock (&prd->e.lock);
    if ((m = ut_avlLookup (&prd_writers_treedef, &prd->writers, wr_guid)) != NULL)
      ut_avlDelete (&prd_writers_treedef, &prd->writers, m);
    os_mutexUnlock (&prd->e.lock);
    free_prd_wr_match (m);
  }
}

static void writer_add_connection (struct writer *wr, struct proxy_reader *prd)
{
  struct wr_prd_match *m;
  ut_avlIPath_t path;
  int pretend_everything_acked;
  if ((m = os_malloc (sizeof (*m))) == NULL)
    abort ();
  m->prd_guid = prd->e.guid;
  m->is_reliable = (prd->c.xqos->reliability.kind > NN_BEST_EFFORT_RELIABILITY_QOS);
  m->assumed_in_sync = (config.retransmit_merging == REXMIT_MERGE_ALWAYS);
  m->has_replied_to_hb = 0;
  m->all_have_replied_to_hb = 0;
  /* m->demoted: see below */
  os_mutexLock (&prd->e.lock);
  if (prd->deleting)
  {
    TRACE (("  writer_add_connection(wr %x:%x:%x:%x prd %x:%x:%x:%x) - prd is being deleted\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid)));
    pretend_everything_acked = 1;
  }
  else if (!m->is_reliable)
  {
    /* Pretend a best-effort reader has ack'd everything, even waht is
       still to be published. */
    pretend_everything_acked = 1;
  }
  else
  {
    /* Only guarantee reliability starting now; that is, we assume the
       reader has ack'd everything already published.  We'll happily
       respond to nacks, so transient-local readers can still get the
       history. */
    pretend_everything_acked = 0;
  }
  os_mutexUnlock (&prd->e.lock);
  m->next_acknack = DDSI_COUNT_MIN;
  m->next_nackfrag = DDSI_COUNT_MIN;
  nn_lat_estim_init (&m->hb_to_ack_latency);
  m->hb_to_ack_latency_tlastlog = now ();
  m->t_acknack_accepted = 0;

  os_mutexLock (&wr->e.lock);
  if (pretend_everything_acked)
    m->seq = MAX_SEQ_NUMBER;
  else
    m->seq = wr->seq;
  if (ut_avlLookupIPath (&wr_readers_treedef, &wr->readers, &prd->e.guid, &path))
  {
    TRACE (("  writer_add_connection(wr %x:%x:%x:%x prd %x:%x:%x:%x) - already connected\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid)));
    os_mutexUnlock (&wr->e.lock);
    nn_lat_estim_fini (&m->hb_to_ack_latency);
    os_free (m);
  }
  else
  {
    TRACE (("  writer_add_connection(wr %x:%x:%x:%x prd %x:%x:%x:%x) - ack seq %lld\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid), m->seq));
    ut_avlInsertIPath (&wr_readers_treedef, &wr->readers, m, &path);
    rebuild_writer_addrset (wr);
    wr->num_reliable_readers += m->is_reliable;
    os_mutexUnlock (&wr->e.lock);

    /* for proxy readers using a non-wildcard partition matching a
     wildcard partition at the writer (and only a wildcard partition),
     ensure that a matching non-wildcard partition exists */
    if (!is_builtin_entityid (wr->e.guid.entityid))
    {
      const char *realname = NULL;
      ASSERT_RDLOCK_HELD (&qoslock);
      if (partition_match_based_on_wildcard_in_left_operand (wr->xqos, prd->c.xqos, &realname))
      {
        assert (realname != NULL);
        create_a_group (realname, wr->topic);
      }
    }

    /* If reliable and/or transient-local, we may have data available
       in the WHC, but if all has been acknowledged by the previously
       known proxy readers (or if the is the first proxy reader),
       there is no heartbeat event scheduled.

       A pre-emptive AckNack may be sent, but need not be, and we
       can't be certain it won't have the final flag set. So we must
       ensure a heartbeat is scheduled soon. */
    if (wr->heartbeat_xevent)
    {
      const os_int64 delta = 1 * T_MILLISECOND;
      const os_int64 tnext = now () + delta;
      os_mutexLock (&wr->e.lock);
      /* To make sure that we keep sending heartbeats at a higher rate
         at the start of this discovery, reset the hbs_since_last_write
         count to zero. */
      wr->hbcontrol.hbs_since_last_write = 0;
      if (tnext < wr->hbcontrol.tsched)
      {
        wr->hbcontrol.tsched = tnext;
        resched_xevent_if_earlier (wr->heartbeat_xevent, tnext);
      }
      os_mutexUnlock (&wr->e.lock);
    }
  }
}

static void reader_add_connection (struct reader *rd, struct proxy_writer *pwr, nn_count_t *init_count)
{
  struct rd_pwr_match *m;
  ut_avlIPath_t path;
  if ((m = os_malloc (sizeof (*m))) == NULL)
    abort ();
  m->pwr_guid = pwr->e.guid;

  os_mutexLock (&rd->e.lock);

  /* Initial sequence number of acknacks is the highest stored (+ 1,
     done when generating the acknack) -- existing connections may be
     beyond that already, but this guarantees that one particular
     writer will always see monotonically increasing sequence numbers
     from one particular reader.  This is then used for the
     pwr_rd_match initialization */
  TRACE (("  reader %x:%x:%x:%x init_acknack_count = %d\n", PGUID (rd->e.guid), rd->init_acknack_count));
  *init_count = rd->init_acknack_count;

  if (ut_avlLookupIPath (&rd_writers_treedef, &rd->writers, &pwr->e.guid, &path))
  {
    TRACE (("  reader_add_connection(pwr %x:%x:%x:%x rd %x:%x:%x:%x) - already connected\n",
            PGUID (pwr->e.guid), PGUID (rd->e.guid)));
    os_mutexUnlock (&rd->e.lock);
    os_free (m);
  }
  else
  {
    TRACE (("  reader_add_connection(pwr %x:%x:%x:%x rd %x:%x:%x:%x)\n",
            PGUID (pwr->e.guid), PGUID (rd->e.guid)));
    ut_avlInsertIPath (&rd_writers_treedef, &rd->writers, m, &path);
    os_mutexUnlock (&rd->e.lock);
  }
}

static int add_matching_groups_helper (v_group g, void *varg)
{
  /* Damn. Didn't think it through all the way, so now I need to check
   partition matching *again* :-( Eventually, a real solution will
   be implemented. */
  struct proxy_writer *pwr = varg;
  const char *pname = v_partitionName (g->partition);
  nn_xqos_t tmp;
  ASSERT_MUTEX_HELD (&pwr->e.lock);
  tmp.present = QP_PARTITION;
  tmp.partition.n = 1;
  tmp.partition.strs = (char **) &pname;
  if (!partitions_match_p (pwr->c.xqos, &tmp))
    return 0;
  else if (nn_groupset_add_group (pwr->groups, g) >= 0)
    return 1;
  else
    return 0;
}

static const char *any_nonwildcard_partition (const nn_partition_qospolicy_t *ps)
{
  int i;
  if (ps->n == 0)
  {
    /* The default partition is a valid non-wildcard partition, but
     one way of specifying it is an empty set of partitions.  That
     means no early exit from the loop, and that means we return a
     null pointer. */
    return "";
  }
  for (i = 0; i < ps->n; i++)
    if (!is_wildcard_partition (ps->strs[i]))
      return ps->strs[i];
  return NULL;
}

static void proxy_writer_add_connection (struct proxy_writer *pwr, struct reader *rd, os_int64 tnow, nn_count_t init_count)
{
  struct pwr_rd_match *m;
  ut_avlIPath_t path;
  os_int64 last_deliv_seq;

  if ((m = os_malloc (sizeof (*m))) == NULL)
    abort ();

  os_mutexLock (&pwr->e.lock);
  if (ut_avlLookupIPath (&pwr_readers_treedef, &pwr->readers, &rd->e.guid, &path))
    goto already_matched;

  if (pwr->c.topic == NULL)
    pwr->c.topic = rd->topic;

  TRACE (("  proxy_writer_add_connection(pwr %x:%x:%x:%x rd %x:%x:%x:%x)\n",
          PGUID (pwr->e.guid), PGUID (rd->e.guid)));
  m->rd_guid = rd->e.guid;
  m->tcreate = now ();

  {
    int ngroups;

    /* Add the groups of RD to PWR; again: it is a set, and we don't
       mind if there are any groups in PWR for which currently no
       readers exist. */
    ngroups = nn_groupset_foreach (rd->matching_groups, add_matching_groups_helper, pwr);

    /* We know there's a QoS match, but it may be because PWR uses a
       real partition and RD uses a wildcard and still has a no
       matching groups groupset. If that's the case, pick any real
       partition from PWR and add it to RD. It doesn't matter if we
       attempt adding the same one twice (it is a set, after all), or
       indeed add two different ones. The test for an empty groupset
       is mere optimisation. Creating a group adds the new group to
       all groupsets. */
    if (!is_builtin_entityid (rd->e.guid.entityid) && ngroups == 0)
    {
      const char *name;
      assert (pwr->c.xqos->present & QP_PARTITION);
      name = any_nonwildcard_partition (&pwr->c.xqos->partition);
      assert (name != NULL);
      create_a_group (name, rd->topic);
      assert (!nn_groupset_empty (rd->matching_groups));
    }
  }

  /* We track the last heartbeat count value per reader--proxy-writer
     pair, so that we can correctly handle directed heartbeats. The
     only reason to bother is to prevent a directed heartbeat (with
     the FINAL flag clear) from causing AckNacks from all readers
     instead of just the addressed ones.

     If we don't mind those extra AckNacks, we could track the count
     at the proxy-writer and simply treat all incoming heartbeats as
     undirected. */
  m->next_heartbeat = DDSI_COUNT_MIN;
  m->hb_timestamp = 0;
  m->t_heartbeat_accepted = 0;

  /* These can change as a consequence of handling data and/or
     discovery activities. The safe way of dealing with them is to
     lock the proxy writer */
  last_deliv_seq = nn_reorder_next_seq (pwr->reorder) - 1;
  if (!rd->handle_as_transient_local || last_deliv_seq == 0)
  {
    /* not treated as transient-local or proxy-writer hasn't seen any
       data yet, in which case this reader is in sync with the proxy
       writer even though it is a transient-local one. */
    m->in_sync = 1;
  }
  else if (!config.conservative_builtin_reader_startup &&
           is_builtin_entityid (rd->e.guid.entityid) &&
           !ut_avlIsEmpty (&pwr->readers))
  {
    /* builtins really don't care about multiple copies */
    m->in_sync = 1;
  }
  else
  {
    /* normal transient-local */
    m->in_sync = 0;
    m->u.not_in_sync.end_of_tl_seq = last_deliv_seq;
  }
  m->count = init_count;
  /* Spec says we may send a pre-emptive AckNack (8.4.2.3.4), hence we
     schedule it for the configured delay * T_MILLISECOND. From then
     on it it'll keep sending pre-emptive ones until the proxy writer
     receives a heartbeat.  (We really only need a pre-emptive AckNack
     per proxy writer, but hopefully it won't make that much of a
     difference in practice.) */
  m->acknack_xevent = rd->reliable ? qxev_acknack (pwr->evq, tnow + config.preemptive_ack_delay, &pwr->e.guid, &rd->e.guid) : NULL;
  if (rd->reliable)
  {
    m->u.not_in_sync.reorder =
      nn_reorder_new (NN_REORDER_MODE_NORMAL, config.secondary_reorder_maxsamples);
    pwr->n_reliable_readers++;
  }
  else
  {
    m->u.not_in_sync.reorder =
      nn_reorder_new (NN_REORDER_MODE_MONOTONICALLY_INCREASING, config.secondary_reorder_maxsamples);
  }

  ut_avlInsertIPath (&pwr_readers_treedef, &pwr->readers, m, &path);


  os_mutexUnlock (&pwr->e.lock);
  return;

already_matched:
  assert (is_builtin_entityid (pwr->e.guid.entityid) ? (pwr->c.topic == NULL) : (pwr->c.topic != NULL));
  TRACE (("  proxy_writer_add_connection(pwr %x:%x:%x:%x rd %x:%x:%x:%x) - already connected\n",
          PGUID (pwr->e.guid), PGUID (rd->e.guid)));
  os_mutexUnlock (&pwr->e.lock);
  os_free (m);
  return;
}

static void proxy_reader_add_connection (struct proxy_reader *prd, struct writer *wr)
{
  struct prd_wr_match *m;
  ut_avlIPath_t path;
  if ((m = os_malloc (sizeof (*m))) == NULL)
    abort ();
  m->wr_guid = wr->e.guid;
  os_mutexLock (&prd->e.lock);
  if (prd->c.topic == NULL)
    prd->c.topic = wr->topic;
  if (ut_avlLookupIPath (&prd_writers_treedef, &prd->writers, &wr->e.guid, &path))
  {
    TRACE (("  proxy_reader_add_connection(wr %x:%x:%x:%x prd %x:%x:%x:%x) - already connected\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid)));
    os_mutexUnlock (&prd->e.lock);
    os_free (m);
  }
  else
  {
    TRACE (("  proxy_reader_add_connection(wr %x:%x:%x:%x prd %x:%x:%x:%x)\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid)));
    ut_avlInsertIPath (&prd_writers_treedef, &prd->writers, m, &path);
    os_mutexUnlock (&prd->e.lock);
  }
}

static nn_entityid_t builtin_entityid_match (nn_entityid_t x)
{
  nn_entityid_t res;
  res.u = 0;
  switch (x.u)
  {
    case NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_TOPIC_READER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_TOPIC_READER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
      break;
    case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER:
      res.u = NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER;
      break;
    case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER:
      res.u = NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER;
      break;

    case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
    case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER:
      /* SPDP is special cased because we don't -- indeed can't --
         match readers with writers, only send to matched readers and
         only accept from matched writers. That way discovery wouldn't
         work at all. No entity with NN_ENTITYID_UNKNOWN exists,
         ever, so this guarantees no connection will be made. */
      res.u = NN_ENTITYID_UNKNOWN;
      break;

    default:
      assert (0);
  }
  return res;
}

static void match_writer_with_proxy_readers (struct writer *wr, UNUSED_ARG (os_int64 tnow))
{
  struct proxy_reader *prd;
  if (!is_builtin_entityid (wr->e.guid.entityid))
  {
    struct ephash_enum_proxy_reader est;
    TRACE (("match_writer_with_proxy_readers(wr %x:%x:%x:%x) scanning all proxy readers\n", PGUID (wr->e.guid)));
    /* Note: we visit at least all proxies that existed when we called
       init (with the -- possible -- exception of ones that were
       deleted between our calling init and our reaching it while
       enumerating), but we may visit a single proxy reader multiple
       times. */
    ephash_enum_proxy_reader_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((prd = ephash_enum_proxy_reader_next (&est)) != NULL)
    {
      if (is_builtin_entityid (prd->e.guid.entityid))
        continue;
      if (qos_match_p (prd->c.xqos, wr->xqos))
      {
        /* We know (because we just got prd from the hash tables) that
           both wr & prd will be valid; we know there won't be any QoS
           changes; therefore we can simply add WR to PRD and PRD to
           WR. */
        writer_add_connection (wr, prd);
        proxy_reader_add_connection (prd, wr);
      }
    }
    os_rwlockUnlock (&gv.qoslock);
    ephash_enum_proxy_reader_fini (&est);
  }
  else
  {
    /* Built-ins have fixed QoS */
    struct ephash_enum_proxy_participant est;
    nn_entityid_t tgt_ent = builtin_entityid_match (wr->e.guid.entityid);
    TRACE (("match_writer_with_proxy_readers(wr %x:%x:%x:%x) scanning proxy participants tgt=%x\n", PGUID (wr->e.guid), tgt_ent.u));
    if (tgt_ent.u != NN_ENTITYID_UNKNOWN)
    {
      struct proxy_participant *proxypp;
      ephash_enum_proxy_participant_init (&est);
      while ((proxypp = ephash_enum_proxy_participant_next (&est)) != NULL)
      {
        nn_guid_t tgt_guid;
        tgt_guid.prefix = proxypp->e.guid.prefix;
        tgt_guid.entityid = tgt_ent;
        if ((prd = ephash_lookup_proxy_reader_guid (&tgt_guid)) != NULL)
        {
          writer_add_connection (wr, prd);
          proxy_reader_add_connection (prd, wr);
        }
      }
      ephash_enum_proxy_participant_fini (&est);
    }
  }
}

static void match_reader_with_proxy_writers (struct reader *rd, os_int64 tnow)
{
  /* see match_writer_with_proxy_readers for comments */
  struct proxy_writer *pwr;
  if (!is_builtin_entityid (rd->e.guid.entityid))
  {
    struct ephash_enum_proxy_writer est;
    TRACE (("match_reader_with_proxy_writers(wr %x:%x:%x:%x) scanning all proxy writers\n", PGUID (rd->e.guid)));
    ephash_enum_proxy_writer_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((pwr = ephash_enum_proxy_writer_next (&est)) != NULL)
    {
      if (is_builtin_entityid (pwr->e.guid.entityid))
        continue;
      if (qos_match_p (rd->xqos, pwr->c.xqos))
      {
        nn_count_t init_count;
        reader_add_connection (rd, pwr, &init_count);
        proxy_writer_add_connection (pwr, rd, tnow, init_count);
      }
    }
    os_rwlockUnlock (&gv.qoslock);
    ephash_enum_proxy_writer_fini (&est);
  }
  else
  {
    struct ephash_enum_proxy_participant est;
    nn_entityid_t tgt_ent = builtin_entityid_match (rd->e.guid.entityid);
    TRACE (("match_reader_with_proxy_writers(rd %x:%x:%x:%x) scanning proxy participants tgt=%x\n", PGUID (rd->e.guid), tgt_ent.u));
    if (tgt_ent.u != NN_ENTITYID_UNKNOWN)
    {
      struct proxy_participant *proxypp;
      ephash_enum_proxy_participant_init (&est);
      while ((proxypp = ephash_enum_proxy_participant_next (&est)) != NULL)
      {
        nn_guid_t tgt_guid;
        tgt_guid.prefix = proxypp->e.guid.prefix;
        tgt_guid.entityid = tgt_ent;
        if ((pwr = ephash_lookup_proxy_writer_guid (&tgt_guid)) != NULL)
        {
          nn_count_t init_count;
          reader_add_connection (rd, pwr, &init_count);
          proxy_writer_add_connection (pwr, rd, tnow, init_count);
        }
      }
      ephash_enum_proxy_participant_fini (&est);
    }
  }
}

static void match_proxy_writer_with_readers (struct proxy_writer *pwr, os_int64 tnow)
{
  /* see match_writer_with_proxy_readers for comments */
  struct reader *rd;
  if (!is_builtin_entityid (pwr->e.guid.entityid))
  {
    struct ephash_enum_reader est;
    TRACE (("match_proxy_writer_with_readers(pwr %x:%x:%x:%x) scanning all readers\n", PGUID (pwr->e.guid)));
    ephash_enum_reader_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((rd = ephash_enum_reader_next (&est)) != NULL)
    {
      if (is_builtin_entityid (rd->e.guid.entityid))
        continue;
      if (qos_match_p (rd->xqos, pwr->c.xqos))
      {
        nn_count_t init_count;
        reader_add_connection (rd, pwr, &init_count);
        proxy_writer_add_connection (pwr, rd, tnow, init_count);
      }
    }
    os_rwlockUnlock (&gv.qoslock);
    ephash_enum_reader_fini (&est);
  }
  else
  {
    struct ephash_enum_participant est;
    nn_entityid_t tgt_ent = builtin_entityid_match (pwr->e.guid.entityid);
    TRACE (("match_proxy_writer_with_readers(pwr %x:%x:%x:%x) scanning participants tgt=%x\n", PGUID (pwr->e.guid), tgt_ent.u));
    if (tgt_ent.u != NN_ENTITYID_UNKNOWN)
    {
      struct participant *pp;
      ephash_enum_participant_init (&est);
      while ((pp = ephash_enum_participant_next (&est)) != NULL)
      {
        nn_guid_t tgt_guid;
        tgt_guid.prefix = pp->e.guid.prefix;
        tgt_guid.entityid = tgt_ent;
        if ((rd = ephash_lookup_reader_guid (&tgt_guid)) != NULL)
        {
          nn_count_t init_count;
          reader_add_connection (rd, pwr, &init_count);
          proxy_writer_add_connection (pwr, rd, tnow, init_count);
        }
      }
      ephash_enum_participant_fini (&est);
    }
  }
}

static void match_proxy_reader_with_writers (struct proxy_reader *prd, UNUSED_ARG (os_int64 tnow))
{
  /* see match_writer_with_proxy_readers for comments */
  struct writer *wr;
  if (!is_builtin_entityid (prd->e.guid.entityid))
  {
    struct ephash_enum_writer est;
    TRACE (("match_proxy_reader_with_writers(prd %x:%x:%x:%x) scanning all writers\n", PGUID (prd->e.guid)));
    ephash_enum_writer_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((wr = ephash_enum_writer_next (&est)) != NULL)
    {
      if (is_builtin_entityid (wr->e.guid.entityid))
        continue;
      if (qos_match_p (prd->c.xqos, wr->xqos))
      {
        proxy_reader_add_connection (prd, wr);
        writer_add_connection (wr, prd);
      }
    }
    os_rwlockUnlock (&gv.qoslock);
    ephash_enum_writer_fini (&est);
  }
  else
  {
    struct ephash_enum_participant est;
    nn_entityid_t tgt_ent = builtin_entityid_match (prd->e.guid.entityid);
    TRACE (("match_proxy_reader_with_writers(prd %x:%x:%x:%x) scanning participants tgt=%x\n", PGUID (prd->e.guid), tgt_ent.u));
    if (tgt_ent.u != NN_ENTITYID_UNKNOWN)
    {
      struct participant *pp;
      ephash_enum_participant_init (&est);
      while ((pp = ephash_enum_participant_next (&est)) != NULL)
      {
        nn_guid_t tgt_guid;
        tgt_guid.prefix = pp->e.guid.prefix;
        tgt_guid.entityid = tgt_ent;
        if ((wr = ephash_lookup_writer_guid (&tgt_guid)) != NULL)
        {
          proxy_reader_add_connection (prd, wr);
          writer_add_connection (wr, prd);
        }
      }
      ephash_enum_participant_fini (&est);
    }
  }
}

/* ENDPOINT --------------------------------------------------------- */

static int new_reader_writer_common (const struct nn_guid *guid, const struct topic *topic, const struct nn_xqos *xqos)
{
  const char *partition = "(default)";
  const char *partition_suffix = "";
  assert (is_builtin_entityid (guid->entityid) ? (topic == NULL) : (topic != NULL));
  if (is_builtin_entityid (guid->entityid))
  {
    /* continue printing it as not being in a partition, the actual
       value doesn't matter because it is never matched based on QoS
       settings */
    partition = "(null)";
  }
  else if ((xqos->present & QP_PARTITION) && xqos->partition.n > 0 && strcmp (xqos->partition.strs[0], "") != 0)
  {
    partition = xqos->partition.strs[0];
    if (xqos->partition.n > 1)
      partition_suffix = "+";
  }
  nn_log (LC_DISCOVERY, "new_%s(guid %x:%x:%x:%x, %s%s.%s/%s)\n",
          is_writer_entityid (guid->entityid) ? "writer" : "reader",
          PGUID (*guid),
          partition, partition_suffix,
          topic ? topic_name (topic) : "(null)",
          topic ? topic_typename (topic) : "(null)");
  return 0;
}

static void endpoint_common_init (struct entity_common *e, struct endpoint_common *c, enum entity_kind kind, const struct nn_guid *guid, struct participant *pp)
{
  entity_common_init (e, guid, kind);
  memset (&c->gid, 0, sizeof (c->gid));
  c->pp = ref_participant (pp, &e->guid);
}

static void endpoint_common_fini (struct entity_common *e, struct endpoint_common *c)
{
  unref_participant (c->pp, &e->guid);
  entity_common_fini (e);
}

static int set_topic_type_name (nn_xqos_t *xqos, const struct topic *topic)
{
  if (!(xqos->present & QP_TYPE_NAME) && topic)
  {
    xqos->present |= QP_TYPE_NAME;
    xqos->type_name = os_strdup (topic_typename (topic));
  }
  if (!(xqos->present & QP_TOPIC_NAME) && topic)
  {
    xqos->present |= QP_TOPIC_NAME;
    xqos->topic_name = os_strdup (topic_name (topic));
  }
  return 0;
}

/* WRITER ----------------------------------------------------------- */


static void augment_wr_prd_match (void *vnode, const void *vleft, const void *vright)
{
  struct wr_prd_match *n = vnode;
  const struct wr_prd_match *left = vleft;
  const struct wr_prd_match *right = vright;
  os_int64 min_seq, max_seq;
  unsigned have_replied = n->has_replied_to_hb;

  /* note: this means min <= seq, but not min <= max nor seq <= max!
     note: this guarantees max < MAX_SEQ_NUMBER, which by induction
     guarantees {left,right}.max < MAX_SEQ_NUMBER note: this treats a
     reader that has not yet replied to a heartbeat as a demoted
     one */
  min_seq = n->seq;
  max_seq = (n->seq < MAX_SEQ_NUMBER) ? n->seq : 0;

  /* 1. Compute {min,max} & have_replied. */
  if (left)
  {
    if (left->min_seq < min_seq)
      min_seq = left->min_seq;
    if (left->max_seq > max_seq)
      max_seq = left->max_seq;
    have_replied = have_replied && left->all_have_replied_to_hb;
  }
  if (right)
  {
    if (right->min_seq < min_seq)
      min_seq = right->min_seq;
    if (right->max_seq > max_seq)
      max_seq = right->max_seq;
    have_replied = have_replied && right->all_have_replied_to_hb;
  }
  n->min_seq = min_seq;
  n->max_seq = max_seq;
  n->all_have_replied_to_hb = have_replied;

  /* 2. Compute num_reliable_readers_where_seq_equals_max */
  if (max_seq == 0)
  {
    /* excludes demoted & best-effort readers; note that max == 0
       cannot happen if {left,right}.max > 0 */
    n->num_reliable_readers_where_seq_equals_max = 0;
  }
  else
  {
    /* if demoted or best-effort, seq != max */
    n->num_reliable_readers_where_seq_equals_max =
      (n->seq == max_seq && n->has_replied_to_hb);
    if (left && left->max_seq == max_seq)
      n->num_reliable_readers_where_seq_equals_max +=
        left->num_reliable_readers_where_seq_equals_max;
    if (right && right->max_seq == max_seq)
      n->num_reliable_readers_where_seq_equals_max +=
        right->num_reliable_readers_where_seq_equals_max;
  }

  /* 3. Compute arbitrary unacked reader */
  /* 3a: maybe this reader is itself a candidate */
  if (n->seq < max_seq)
  {
    /* seq < max cannot be true for a best-effort reader or a demoted */
    n->arbitrary_unacked_reader = n->prd_guid;
  }
  else if (n->is_reliable && (n->seq == MAX_SEQ_NUMBER || !n->has_replied_to_hb))
  {
    /* demoted readers and reliable readers that have not yet replied to a heartbeat are candidates */
    n->arbitrary_unacked_reader = n->prd_guid;
  }
  /* 3b: maybe we can inherit from the children */
  else if (left && left->arbitrary_unacked_reader.entityid.u != NN_ENTITYID_UNKNOWN)
  {
    n->arbitrary_unacked_reader = left->arbitrary_unacked_reader;
  }
  else if (right && right->arbitrary_unacked_reader.entityid.u != NN_ENTITYID_UNKNOWN)
  {
    n->arbitrary_unacked_reader = right->arbitrary_unacked_reader;
  }
  /* 3c: else it may be that we can now determine one of our children
     is actually a candidate */
  else if (left && left->max_seq != 0 && left->max_seq < max_seq)
  {
    n->arbitrary_unacked_reader = left->prd_guid;
  }
  else if (right && right->max_seq != 0 && right->max_seq < max_seq)
  {
    n->arbitrary_unacked_reader = right->prd_guid;
  }
  /* 3d: else no candidate in entire subtree */
  else
  {
    n->arbitrary_unacked_reader.entityid.u = NN_ENTITYID_UNKNOWN;
  }
}

os_int64 writer_max_drop_seq (const struct writer *wr)
{
  const struct wr_prd_match *n;
  if (ut_avlIsEmpty (&wr->readers))
    return wr->seq;
  n = ut_avlRoot (&wr_readers_treedef, &wr->readers);
  return (n->min_seq == MAX_SEQ_NUMBER) ? wr->seq : n->min_seq;
}

int writer_must_have_hb_scheduled (const struct writer *wr)
{
  if (ut_avlIsEmpty (&wr->readers) || whc_empty (wr->whc))
  {
    /* Can't transmit a valid heartbeat if there is no data; and it
       wouldn't actually be sent anywhere if there are no readers, so
       there is little point in processing the xevent all the time.

       Note that add_msg_to_whc and add_proxy_reader_to_writer will
       perform a reschedule. 8.4.2.2.3: need not (can't, really!) send
       a heartbeat if no data is available. */
    return 0;
  }
  else if (!((const struct wr_prd_match *) ut_avlRoot (&wr_readers_treedef, &wr->readers))->all_have_replied_to_hb)
  {
    /* Labouring under the belief that heartbeats must be sent
       regardless of ack state */
    return 1;
  }
  else
  {
    /* DDSI 2.1, section 8.4.2.2.3: need not send heartbeats when all
       messages have been acknowledged.  Slightly different from
       requiring a non-empty whc_seq: if it is transient_local,
       whc_seq usually won't be empty even when all msgs have been
       ack'd. */
    return writer_max_drop_seq (wr) < whc_max_seq (wr->whc);
  }
}

int writer_number_of_unacked_samples (const struct writer *wr)
{
  os_int64 seq_acked = writer_max_drop_seq (wr);
  /* wr->seq is highest sequence number sent, seq_acked is lowest
     sequence number acked by all readers (but may be MAX_SEQ_NUMBER,
     if all readers are unreliable).  Normal case: acked <= seq, with
     seq-acked samples unack'd. */
  if (seq_acked >= wr->seq)
    return 0;
  else
  {
    /* Limit can only be reached in extreme cases (where there
       literally are a billion samples in the WHC) and in theory, if
       the scheduling is particularly bad. */
    const int lim = 1000000000;
    os_int64 n = wr->seq - seq_acked;
    return (n > lim) ? lim : (int) n;
  }
}

int remove_acked_messages (struct writer *wr)
{
  int n, nremain;
  assert (wr->e.guid.entityid.u != NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
  ASSERT_MUTEX_HELD (&wr->e.lock);
  n = whc_remove_acked_messages (wr->whc, writer_max_drop_seq (wr));
  nremain = writer_number_of_unacked_samples (wr);
  /* when transitioning from >= low-water to < low-water, signal
     anyone waiting in throttle_writer() */
  if (wr->throttling && nremain < config.whc_lowwater_mark)
    os_condBroadcast (&wr->throttle_cond);
  if (nremain == 0 && wr->state == WRST_LINGERING)
  {
    nn_log (LC_DISCOVERY, "remove_acked_messages: deleting lingering writer %x:%x:%x:%x\n", PGUID (wr->e.guid));
    delete_writer_nolinger_locked (wr);
  }
  return n;
}

static int new_writer_guid (const struct nn_guid *guid, struct participant *pp, const struct topic *topic, const struct nn_xqos *xqos)
{
  struct writer *wr;
  int res;
  os_int64 tnow = now ();

  assert (is_writer_entityid (guid->entityid));
  assert (ephash_lookup_writer_guid (guid) == NULL);
  assert (memcmp (&guid->prefix, &pp->e.guid.prefix, sizeof (guid->prefix)) == 0);

  if ((res = new_reader_writer_common (guid, topic, xqos)) < 0)
    return res;

  if ((wr = os_malloc (sizeof (*wr))) == NULL)
    return ERR_OUT_OF_MEMORY;

  /* want a pointer to the participant so that a parallel call to
     delete_participant won't interfere with our ability to address
     the participant */
  endpoint_common_init (&wr->e, &wr->c, EK_WRITER, guid, pp);
  os_condInit (&wr->throttle_cond, &wr->e.lock, &gv.cattr);
  wr->seq = 0;
  wr->seq_xmit = 0;
  wr->hbcount = 0;
  wr->state = WRST_OPERATIONAL;
  wr->hbfragcount = 0;
  /* kernel starts the sequenceNumbers at 0, which must be different
     from what we initialise it with */
  wr->last_kernel_seq = 0xffffffff;
  writer_hbcontrol_init (&wr->hbcontrol);
  wr->throttling = 0;
  wr->num_reliable_readers = 0;

  /* Copy QoS, merging in defaults */
  if ((wr->xqos = os_malloc (sizeof (*wr->xqos))) == NULL)
  {
    os_condDestroy (&wr->throttle_cond);
    endpoint_common_fini (&wr->e, &wr->c);
    os_free (wr);
    return ERR_OUT_OF_MEMORY;
  }
  nn_xqos_copy (wr->xqos, xqos);
  nn_xqos_mergein_missing (wr->xqos, &gv.default_xqos_wr);
  assert (wr->xqos->aliased == 0);
  set_topic_type_name (wr->xqos, topic);
  if (config.enabled_logcats & LC_TRACE)
  {
    TRACE (("WRITER %x:%x:%x:%x QOS={", PGUID (wr->e.guid)));
    nn_log_xqos (LC_TRACE, wr->xqos);
    TRACE (("}\n"));
  }
  assert (wr->xqos->present & QP_RELIABILITY);
  wr->reliable = (wr->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);
  assert (wr->xqos->present & QP_DURABILITY);
  if (is_builtin_entityid (wr->e.guid.entityid))
  {
    assert (wr->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS);
    wr->aggressive_keep_last1 = 1;
  }
  else
  {
    wr->aggressive_keep_last1 =
      (config.aggressive_keep_last1_whc &&
       wr->xqos->history.kind == NN_KEEP_LAST_HISTORY_QOS &&
       wr->xqos->history.depth == 1);
  }
  wr->handle_as_transient_local = (wr->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS);
  wr->include_keyhash =
    config.generate_keyhash &&
    ((wr->e.guid.entityid.u & NN_ENTITYID_KIND_MASK) == NN_ENTITYID_KIND_WRITER_WITH_KEY);
  /* Startup mode causes the writer to treat data in its WHC as if
     transient-local, for the first few seconds after startup of the
     DDSI service. It is done for volatile reliable writers only
     (which automatically excludes all builtin writers) or for all
     writers except volatile best-effort & transient-local ones.

     Which one to use depends on whether merge policies are in effect
     in durability. If yes, then durability will take care of all
     transient & persistent data; if no, DDSI discovery usually takes
     too long and this'll save you. */
  if (config.startup_mode_full) {
    wr->startup_mode = gv.startup_mode &&
      (wr->xqos->durability.kind >= NN_TRANSIENT_DURABILITY_QOS ||
       (wr->xqos->durability.kind == NN_VOLATILE_DURABILITY_QOS &&
        wr->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS));
  } else {
    wr->startup_mode = gv.startup_mode &&
      (wr->xqos->durability.kind == NN_VOLATILE_DURABILITY_QOS &&
       wr->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);
  }
  wr->topic = topic;
  wr->as = new_addrset ();


  /* for non-builtin writers, select the eventqueue based on the channel it is mapped to */

  {
    wr->evq = gv.xevents;
  }

  /* heartbeat event will be deleted when the handler can't find a
     writer for it in the hash table. T_NEVER => won't ever be
     scheduled, and this can only change by writing data, which won't
     happen until after it becomes visible. */
  wr->heartbeat_xevent = wr->reliable ? qxev_heartbeat (wr->evq, T_NEVER, &wr->e.guid) : NULL;
  assert (wr->xqos->present & QP_LIVELINESS);
  if (wr->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS ||
      nn_from_ddsi_duration (wr->xqos->liveliness.lease_duration) != T_NEVER)
  {
    nn_log (LC_INFO, "writer %x:%x:%x:%x: incorrectly treating it as of automatic liveliness kind with lease duration = inf (%d, %lld)\n", PGUID (wr->e.guid), (int) wr->xqos->liveliness.kind, nn_from_ddsi_duration (wr->xqos->liveliness.lease_duration));
  }
  wr->lease_duration = T_NEVER; /* FIXME */

  /* Construct WHC -- if aggressive_keep_last1 is set, the WHC will
     drop all samples for which a later update is available.  This
     forces it to maintain a tlidx.  */
  wr->whc = whc_new ((wr->handle_as_transient_local || wr->startup_mode), wr->aggressive_keep_last1);

  /* Connection admin */
  ut_avlInit (&wr_readers_treedef, &wr->readers);

  /* guid_hash needed for protocol handling, so add it before we send
     out our first message.  Also: needed for matching, and swapping
     the order if hash insert & matching creates a window during which
     neither of two endpoints being created in parallel can discover
     the other. */
  ephash_insert_writer_guid (wr);

  /* once it exists, match it with proxy writers and broadcast
     existence (I don't think it matters much what the order of these
     two is, but it seems likely that match-then-broadcast has a
     slightly lower likelihood that a response from a proxy reader
     gets dropped) -- but note that without adding a lock it might be
     deleted while we do so */
  match_writer_with_proxy_readers (wr, tnow);
  sedp_write_writer (wr);

  if (wr->lease_duration != T_NEVER)
    resched_xevent_if_earlier (pp->pmd_update_xevent, 0);
  return 0;
}

int new_writer (struct nn_guid *wrguid, const struct nn_guid *ppguid, const struct topic *topic, const struct nn_xqos *xqos)
{
  struct participant *pp;
  unsigned entity_kind;
  int res;

  if ((pp = ephash_lookup_participant_guid (ppguid)) == NULL)
  {
    TRACE (("new_writer - participant %x:%x:%x:%x not found\n", PGUID (*ppguid)));
    return ERR_UNKNOWN_ENTITY;
  }
  /* participant can't be freed while we're mucking around cos we are
     awake and do not touch the thread's vtime (ephash_lookup already
     verifies we're awake) */
  entity_kind = (topic_haskey (topic) ? NN_ENTITYID_KIND_WRITER_WITH_KEY : NN_ENTITYID_KIND_WRITER_NO_KEY);
  wrguid->prefix = pp->e.guid.prefix;
  if ((res = pp_allocate_entityid (&wrguid->entityid, entity_kind, pp)) < 0)
    return res;
  return new_writer_guid (wrguid, pp, topic, xqos);
}

static void gc_delete_writer (struct gcreq *gcreq)
{
  struct writer *wr = gcreq->arg;
  gcreq_free (gcreq);
  TRACE (("gc_delete_writer(guid %x:%x:%x:%x)\n", PGUID (wr->e.guid)));

  if (wr->heartbeat_xevent)
  {
    wr->hbcontrol.tsched = T_NEVER;
    delete_xevent (wr->heartbeat_xevent);
  }

  /* Tear down connections -- no proxy reader can be adding/removing
      us now, because we can't be found via guid_hash anymore.  We
      therefore need not take lock. */
  while (!ut_avlIsEmpty (&wr->readers))
  {
    struct wr_prd_match *m = ut_avlRoot (&wr_readers_treedef, &wr->readers);
    ut_avlDelete (&wr_readers_treedef, &wr->readers, m);
    proxy_reader_drop_connection (&m->prd_guid, &wr->e.guid);
    free_wr_prd_match (m);
  }

  /* Do last gasp on SEDP and free writer. */
  if (!is_builtin_entityid (wr->e.guid.entityid))
    sedp_dispose_unregister_writer (wr);

  whc_free (wr->whc);
  unref_addrset (wr->as); /* must remain until readers gone (rebuilding of addrset) */
  nn_xqos_fini (wr->xqos);
  os_free (wr->xqos);
  os_condDestroy (&wr->throttle_cond);

  endpoint_common_fini (&wr->e, &wr->c);
  os_free (wr);
}

static void writer_set_state (struct writer *wr, enum writer_state newstate)
{
  ASSERT_MUTEX_HELD (&wr->e.lock);
  TRACE (("writer_set_state(%x:%x:%x:%x) state transition %d -> %d\n", PGUID (wr->e.guid), wr->state, newstate));
  assert (newstate > wr->state);
  if (wr->state == WRST_OPERATIONAL)
  {
    /* It Is particularly important that the writer is removed from
       the GID hash table only once. */
    /* ephash_remove_writer_gid (wr); FIXME - why is this commented out (Steve) */

    /* Unblock all throttled writers (alternative method: clear WHC --
       but with parallel writes and very small limits on the WHC size,
       that doesn't guarantee no-one will block). A truly blocked
       write() is a problem because it prevents the gc thread from
       cleaning up the writer.  (Note: late assignment to wr->state is
       ok, 'tis all protected by the writer lock.) */
    os_condBroadcast (&wr->throttle_cond);
  }
  wr->state = newstate;
}

int delete_writer_nolinger_locked (struct writer *wr)
{
  nn_log (LC_DISCOVERY, "delete_writer_nolinger(guid %x:%x:%x:%x) ...\n", PGUID (wr->e.guid));
  ASSERT_MUTEX_HELD (&wr->e.lock);
  ephash_remove_writer_guid (wr);
  writer_set_state (wr, WRST_DELETING);
  gcreq_writer (wr);
  return 0;
}

int delete_writer_nolinger (const struct nn_guid *guid)
{
  struct writer *wr;
  /* We take no care to ensure application writers are not deleted
     while they still have unacknowledged data (unless it takes too
     long), but we don't care about the DDSI built-in writers: we deal
     with that anyway because of the potential for crashes of remote
     DDSI participants. But it would be somewhat more elegant to do it
     differently. */
  assert (is_writer_entityid (guid->entityid));
  if ((wr = ephash_lookup_writer_guid (guid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_writer_nolinger(guid %x:%x:%x:%x) - unknown guid\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_writer_nolinger(guid %x:%x:%x:%x) ...\n", PGUID (*guid));
  os_mutexLock (&wr->e.lock);
  delete_writer_nolinger_locked (wr);
  os_mutexUnlock (&wr->e.lock);
  return 0;
}

int delete_writer (const struct nn_guid *guid)
{
  struct writer *wr;
  if ((wr = ephash_lookup_writer_guid (guid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_writer(guid %x:%x:%x:%x) - unknown guid\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_writer(guid %x:%x:%x:%x) ...\n", PGUID (*guid));
  os_mutexLock (&wr->e.lock);
  /* If no unack'ed data, don't waste time or resources (expected to
     be the usual case), do it immediately.  If more data is still
     coming in (which can't really happen at the moment, but might
     again in the future) it'll potentially be discarded.  */
  if (writer_number_of_unacked_samples (wr) == 0)
  {
    TRACE (("delete_writer(guid %x:%x:%x:%x) - no unack'ed samples\n", PGUID (*guid)));
    delete_writer_nolinger_locked (wr);
    os_mutexUnlock (&wr->e.lock);
  }
  else
  {
    os_int64 tsched;
    int tsec, tusec;
    writer_set_state (wr, WRST_LINGERING);
    os_mutexUnlock (&wr->e.lock);
    tsched = now () + config.writer_linger_duration;
    time_to_sec_usec (&tsec, &tusec, tsched);
    TRACE (("delete_writer(guid %x:%x:%x:%x) - unack'ed samples, will delete when ack'd or at t = %d.%06d\n",
            PGUID (*guid), tsec, tusec));
    qxev_delete_writer (tsched, &wr->e.guid);
  }
  return 0;
}

void writer_exit_startup_mode (struct writer *wr)
{
  os_mutexLock (&wr->e.lock);
  /* startup mode and handle_as_transient_local may not both be set */
  assert (!(wr->startup_mode && wr->handle_as_transient_local));
  if (!wr->startup_mode)
    TRACE (("  wr %x:%x:%x:%x skipped\n", PGUID (wr->e.guid)));
  else
  {
    int n;
    assert (wr->e.guid.entityid.u != NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
    wr->startup_mode = 0;
    whc_downgrade_to_volatile (wr->whc);
    n = remove_acked_messages (wr);
    TRACE (("  wr %x:%x:%x:%x dropped %d entr%s\n", PGUID (wr->e.guid), n, n == 1 ? "y" : "ies"));
  }
  os_mutexUnlock (&wr->e.lock);
}

/* READER ----------------------------------------------------------- */


static int new_reader_guid (const struct nn_guid *guid, struct participant *pp, const struct topic *topic, const struct nn_xqos *xqos, deliver_cb_t deliver_cb, void *deliver_cbarg)
{
  /* see new_writer_guid for commenets */
  struct reader *rd;
  int res;
  os_int64 tnow = now ();

  assert (!is_writer_entityid (guid->entityid));
  assert (ephash_lookup_reader_guid (guid) == NULL);
  assert (memcmp (&guid->prefix, &pp->e.guid.prefix, sizeof (guid->prefix)) == 0);

  if ((res = new_reader_writer_common (guid, topic, xqos)) < 0)
    return res;

  if ((rd = os_malloc (sizeof (*rd))) == NULL)
    return ERR_OUT_OF_MEMORY;

  endpoint_common_init (&rd->e, &rd->c, EK_READER, guid, pp);

  /* Copy QoS, merging in defaults */
  rd->xqos = os_malloc (sizeof (*rd->xqos));
  nn_xqos_copy (rd->xqos, xqos);
  nn_xqos_mergein_missing (rd->xqos, &gv.default_xqos_rd);
  assert (rd->xqos->aliased == 0);
  set_topic_type_name (rd->xqos, topic);
  if (config.enabled_logcats & LC_TRACE)
  {
    TRACE (("READER %x:%x:%x:%x QOS={", PGUID (rd->e.guid)));
    nn_log_xqos (LC_TRACE, rd->xqos);
    TRACE (("}\n"));
  }
  assert (rd->xqos->present & QP_RELIABILITY);
  rd->reliable = (rd->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);
  assert (rd->xqos->present & QP_DURABILITY);
  rd->handle_as_transient_local = (rd->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS);
  rd->topic = topic;
  rd->init_acknack_count = 0;
  (void) deliver_cbarg;
  (void) deliver_cb;
  rd->matching_groups = nn_groupset_new ();
  if (!is_builtin_entityid (rd->e.guid.entityid))
    nn_groupset_fromqos (rd->matching_groups, gv.ospl_kernel, rd->xqos);
  assert (rd->xqos->present & QP_LIVELINESS);
  if (rd->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS ||
      nn_from_ddsi_duration (rd->xqos->liveliness.lease_duration) != T_NEVER)
  {
    nn_log (LC_INFO, "reader %x:%x:%x:%x: incorrectly treating it as of automatic liveliness kind with lease duration = inf (%d, %lld)\n", PGUID (rd->e.guid), (int) rd->xqos->liveliness.kind, nn_from_ddsi_duration (rd->xqos->liveliness.lease_duration));
  }


  ut_avlInit (&rd_writers_treedef, &rd->writers);

  ephash_insert_reader_guid (rd);
  match_reader_with_proxy_writers (rd, tnow);
  sedp_write_reader (rd);
  return 0;
}

int new_reader (struct nn_guid *rdguid, const struct nn_guid *ppguid, const struct topic *topic, const struct nn_xqos *xqos, deliver_cb_t deliver_cb, void *deliver_cbarg)
{
  /* see new_writer for comments */
  struct participant *pp;
  unsigned entity_kind;
  int res;

  if ((pp = ephash_lookup_participant_guid (ppguid)) == NULL)
  {
    TRACE (("new_reader - participant %x:%x:%x:%x not found\n", PGUID (*ppguid)));
    return ERR_UNKNOWN_ENTITY;
  }
  entity_kind = (topic_haskey (topic) ? NN_ENTITYID_KIND_READER_WITH_KEY : NN_ENTITYID_KIND_READER_NO_KEY);
  rdguid->prefix = pp->e.guid.prefix;
  if ((res = pp_allocate_entityid (&rdguid->entityid, entity_kind, pp)) < 0)
    return res;
  return new_reader_guid (rdguid, pp, topic, xqos, deliver_cb, deliver_cbarg);
}

static void gc_delete_reader (struct gcreq *gcreq)
{
  /* see gc_delete_writer for comments */
  struct reader *rd = gcreq->arg;
  gcreq_free (gcreq);
  TRACE (("gc_delete_reader(guid %x:%x:%x:%x)\n", PGUID (rd->e.guid)));

  while (!ut_avlIsEmpty (&rd->writers))
  {
    struct rd_pwr_match *m = ut_avlRoot (&rd_writers_treedef, &rd->writers);
    ut_avlDelete (&rd_writers_treedef, &rd->writers, m);
    proxy_writer_drop_connection (&m->pwr_guid, rd);
    free_rd_pwr_match (m);
  }

  if (!is_builtin_entityid (rd->e.guid.entityid))
    sedp_dispose_unregister_reader (rd);

  nn_groupset_free (rd->matching_groups);

  nn_xqos_fini (rd->xqos);
  os_free (rd->xqos);

  endpoint_common_fini (&rd->e, &rd->c);
  os_free (rd);
}

int delete_reader (const struct nn_guid *guid)
{
  struct reader *rd;
  assert (!is_writer_entityid (guid->entityid));
  if ((rd = ephash_lookup_reader_guid (guid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_reader_guid(guid %x:%x:%x:%x) - unknown guid\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_reader_guid(guid %x:%x:%x:%x) ...\n", PGUID (*guid));
  ephash_remove_reader_guid (rd);
  /* ephash_remove_reader_gid (rd); FIXME why is this commented out (Steve) */
  gcreq_reader (rd);
  return 0;
}

/* PROXY-PARTICIPANT ------------------------------------------------ */
void new_proxy_participant 
(
  const struct nn_guid *ppguid,
  unsigned bes,
  const struct nn_guid *privileged_pp_guid,
  struct addrset *as_default,
  struct addrset *as_meta,
  os_int64 tlease_dur,
  nn_vendorid_t vendor,
  unsigned custom_flags
)
{
  /* No locking => iff all participants use unique guids, and sedp
     runs on a single thread, it can't go wrong. FIXME, maybe? The
     same holds for the other functions for creating entities. */
  struct proxy_participant *proxypp;
  os_int64 tnow;

  assert (ppguid->entityid.u == NN_ENTITYID_PARTICIPANT);
  assert (ephash_lookup_proxy_participant_guid (ppguid) == NULL);

  tnow = now ();
  prune_deleted_participant_guids (tnow);

  proxypp = os_malloc (sizeof (*proxypp));

  entity_common_init (&proxypp->e, ppguid, EK_PROXY_PARTICIPANT);
  proxypp->refc = 1;
  proxypp->vendor = vendor;
  proxypp->bes = bes;
  proxypp->privileged_pp_guid = *privileged_pp_guid;
  proxypp->lease = lease_new (tlease_dur, &proxypp->e);
  proxypp->as_default = as_default;
  proxypp->as_meta = as_meta;
  proxypp->endpoints = NULL;

  if (custom_flags & CF_INC_KERNEL_SEQUENCE_NUMBERS)
    proxypp->kernel_sequence_numbers = 1;
  else
    proxypp->kernel_sequence_numbers = 0;

  /* Proxy participant must be in the hash tables for
     new_proxy_{writer,reader} to work */
  ephash_insert_proxy_participant_guid (proxypp);

  /* Add proxy endpoints based on the advertised (& possibly augmented
     ...) built-in endpoint set. */
  {
#define TE(ap_, a_, bp_, b_) { NN_##ap_##BUILTIN_ENDPOINT_##a_, NN_ENTITYID_##bp_##_BUILTIN_##b_ }
#define LTE(a_, bp_, b_) { NN_##BUILTIN_ENDPOINT_##a_, NN_ENTITYID_##bp_##_BUILTIN_##b_ }
    static const struct bestab {
      unsigned besflag;
      unsigned entityid;
    } bestab[] = {
#if 0
      /* SPDP gets special treatment => no need for proxy
         writers/readers */
      TE (DISC_, PARTICIPANT_ANNOUNCER, SPDP, PARTICIPANT_WRITER),
#endif
      TE (DISC_, PARTICIPANT_DETECTOR, SPDP, PARTICIPANT_READER),
      TE (DISC_, PUBLICATION_ANNOUNCER, SEDP, PUBLICATIONS_WRITER),
      TE (DISC_, PUBLICATION_DETECTOR, SEDP, PUBLICATIONS_READER),
      TE (DISC_, SUBSCRIPTION_ANNOUNCER, SEDP, SUBSCRIPTIONS_WRITER),
      TE (DISC_, SUBSCRIPTION_DETECTOR, SEDP, SUBSCRIPTIONS_READER),
      LTE (PARTICIPANT_MESSAGE_DATA_WRITER, P2P, PARTICIPANT_MESSAGE_WRITER),
      LTE (PARTICIPANT_MESSAGE_DATA_READER, P2P, PARTICIPANT_MESSAGE_READER)
    };
#undef TE
#undef LTE
    int i;
    for (i = 0; i < (int) (sizeof (bestab) / sizeof (*bestab)); i++)
    {
      const struct bestab *te = &bestab[i];
      if (proxypp->bes & te->besflag)
      {
        nn_xqos_t *xqos = os_malloc (sizeof (*xqos));
        nn_guid_t guid1;
        guid1.prefix = proxypp->e.guid.prefix;
        guid1.entityid.u = te->entityid;
        if (is_writer_entityid (guid1.entityid))
        {
          nn_xqos_copy (xqos, &gv.builtin_endpoint_xqos_wr);
          assert (is_builtin_entityid (guid1.entityid));
          new_proxy_writer (&guid1, proxypp->as_meta, xqos, gv.builtins_dqueue, gv.xevents);
        }
        else
        {
          nn_xqos_copy (xqos, &gv.builtin_endpoint_xqos_rd);
          new_proxy_reader (&guid1, proxypp->as_meta, xqos);
        }
      }
    }
  }

  lease_register (proxypp->lease);
}

static struct proxy_participant *ref_proxy_participant (struct proxy_participant *proxypp)
{
  atomic_inc_u32_nv (&proxypp->refc);
  return proxypp;
}

static void unref_proxy_participant (struct proxy_participant *proxypp)
{
  if (atomic_dec_u32_nv (&proxypp->refc) == 0)
  {
    assert (proxypp->endpoints == NULL);

    unref_addrset (proxypp->as_default);
    unref_addrset (proxypp->as_meta);
    lease_free (proxypp->lease);
    entity_common_fini (&proxypp->e);
    remove_deleted_participant_guid (&proxypp->e.guid);
    os_free (proxypp);
  }
}

static void gc_delete_proxy_participant (struct gcreq *gcreq)
{
  struct proxy_participant *proxypp = gcreq->arg;
  gcreq_free (gcreq);
  unref_proxy_participant (proxypp);
}

static struct entity_common *entity_common_from_proxy_endpoint_common (const struct proxy_endpoint_common *c)
{
  assert (offsetof (struct proxy_writer, e) == 0);
  assert (offsetof (struct proxy_reader, e) == offsetof (struct proxy_writer, e));
  assert (offsetof (struct proxy_reader, c) == offsetof (struct proxy_writer, c));
  assert (c != NULL);
  return (struct entity_common *) ((char *) c - offsetof (struct proxy_writer, c));
}

static void delete_ppt (struct proxy_participant * proxypp)
{
  struct proxy_endpoint_common * c;
  int ret;

  /* delete_proxy_{reader,writer} merely schedules the actual delete
     operation, so we can hold the lock -- at least, for now. */

  os_mutexLock (&proxypp->e.lock);
  c = proxypp->endpoints;
  while (c)
  {
    struct entity_common *e = entity_common_from_proxy_endpoint_common (c);
    if (is_writer_entityid (e->guid.entityid))
    {
      ret = delete_proxy_writer (&e->guid);
    }
    else
    {
      ret = delete_proxy_reader (&e->guid);
    }
    (void) ret;
    c = c->next_ep;
  }
  os_mutexUnlock (&proxypp->e.lock);

  gcreq_proxy_participant (proxypp);
}

typedef struct proxy_purge_data
{
  struct proxy_participant *proxypp;
  os_sockaddr_storage * addr;
  os_uint32 port;
}
* proxy_purge_data_t;

static void purge_helper (const os_sockaddr_storage * n, void * varg)
{
  proxy_purge_data_t data = (proxy_purge_data_t) varg;
  os_ushort port = sockaddr_get_port (n);
  if 
  (
    (port == data->port) &&
    (os_sockaddrIPAddressEqual ((const os_sockaddr*) data->addr, (const os_sockaddr*) n))
  )
  {
    delete_proxy_participant (data->proxypp);
  }
}

void purge_proxy_participants (os_sockaddr_storage * addr, os_uint32 port)
{
  struct ephash_enum_proxy_participant est;
  struct proxy_purge_data data;

  data.addr = addr;
  data.port = port;
  ephash_enum_proxy_participant_init (&est);
  while ((data.proxypp = ephash_enum_proxy_participant_next (&est)) != NULL)
  {
    addrset_forall (data.proxypp->as_meta, purge_helper, &data);
  }
  ephash_enum_proxy_participant_fini (&est);
}

void delete_proxy_participant (struct proxy_participant * ppt)
{
  os_mutexLock (&gv.lock);
  remember_deleted_participant_guid (&ppt->e.guid);
  ephash_remove_proxy_participant_guid (ppt);
  os_mutexUnlock (&gv.lock);
  delete_ppt (ppt);
}

int delete_proxy_participant_by_guid (const struct nn_guid * guid)
{
  struct proxy_participant * ppt;

  os_mutexLock (&gv.lock);
  ppt = ephash_lookup_proxy_participant_guid (guid);
  if (ppt == NULL)
  {
    os_mutexUnlock (&gv.lock);
    return ERR_UNKNOWN_ENTITY;
  }
  remember_deleted_participant_guid (&ppt->e.guid);
  ephash_remove_proxy_participant_guid (ppt);
  os_mutexUnlock (&gv.lock);
  delete_ppt (ppt);

  return 0;
}


/* PROXY-ENDPOINT --------------------------------------------------- */

static int proxy_endpoint_common_init (struct entity_common *e, struct proxy_endpoint_common *c, enum entity_kind kind, const struct nn_guid *guid, struct proxy_participant *proxypp, struct addrset *as, nn_xqos_t *xqos)
{
  if (is_builtin_entityid (guid->entityid))
    assert ((xqos->present & (QP_TOPIC_NAME | QP_TYPE_NAME)) == 0);
  else
    assert ((xqos->present & (QP_TOPIC_NAME | QP_TYPE_NAME)) == (QP_TOPIC_NAME | QP_TYPE_NAME));
  assert (xqos->aliased == 0);

  entity_common_init (e, guid, kind);
  c->proxypp = ref_proxy_participant (proxypp);
  c->xqos = xqos;
  c->as = ref_addrset (as);
  c->topic = NULL; /* set from first matching reader/writer */

  os_mutexLock (&proxypp->e.lock);
  c->next_ep = proxypp->endpoints;
  c->prev_ep = NULL;
  if (c->next_ep)
    c->next_ep->prev_ep = c;
  proxypp->endpoints = c;
  os_mutexUnlock (&proxypp->e.lock);
  return 0;
}

static void proxy_endpoint_common_fini (struct entity_common *e, struct proxy_endpoint_common *c)
{
  struct proxy_participant *proxypp = c->proxypp;

  os_mutexLock (&proxypp->e.lock);
  if (c->next_ep)
    c->next_ep->prev_ep = c->prev_ep;
  if (c->prev_ep)
    c->prev_ep->next_ep = c->next_ep;
  else
    proxypp->endpoints = c->next_ep;
  os_mutexUnlock (&proxypp->e.lock);

  nn_xqos_fini (c->xqos);
  os_free (c->xqos);
  unref_addrset (c->as);

  entity_common_fini (e);
  unref_proxy_participant (proxypp);
}

/* PROXY-WRITER ----------------------------------------------------- */

int new_proxy_writer (const struct nn_guid *guid, struct addrset *as, nn_xqos_t *xqos, struct nn_dqueue *dqueue, struct xeventq *evq)
{
  struct proxy_participant *proxypp;
  struct proxy_writer *pwr;
  nn_guid_t ppguid;
  int isreliable;
  os_int64 tnow = now ();

  assert (is_writer_entityid (guid->entityid));
  assert (ephash_lookup_proxy_writer_guid (guid) == NULL);

  ppguid.prefix = guid->prefix;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((proxypp = ephash_lookup_proxy_participant_guid (&ppguid)) == NULL)
  {
    NN_WARNING1 ("new_proxy_writer(%x:%x:%x:%x): proxy participant unknown\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }

  if ((pwr = os_malloc (sizeof (*pwr))) == NULL)
    return ERR_OUT_OF_MEMORY;

  if (proxy_endpoint_common_init (&pwr->e, &pwr->c, EK_PROXY_WRITER, guid, proxypp, as, xqos) != 0)
  {
    os_free (pwr);
    return 0;
  }

  ut_avlInit (&pwr_readers_treedef, &pwr->readers);
  pwr->groups = nn_groupset_new ();
  pwr->v_message_qos = new_v_message_qos (pwr->c.xqos);
  pwr->n_reliable_readers = 0;
  pwr->last_seq = 0;
  pwr->last_fragnum = -1;
  pwr->nackfragcount = 0;
  pwr->next_deliv_seq_lowword = 1;
  if (is_builtin_entityid (pwr->e.guid.entityid)) {
    /* The DDSI built-in proxy writers always deliver
       asynchronously */
    pwr->deliver_synchronously = 0;
  } else if (nn_from_ddsi_duration (pwr->c.xqos->latency_budget.duration) <= config.synchronous_delivery_latency_bound &&
             pwr->c.xqos->transport_priority.value >= config.synchronous_delivery_priority_threshold) {
    /* Regular proxy-writers with a sufficiently low latency_budget
       and a sufficiently high transport_priority deliver
       synchronously */
    pwr->deliver_synchronously = 1;
  } else {
    pwr->deliver_synchronously = 0;
  }
  pwr->have_seen_heartbeat = 0;
  isreliable = (pwr->c.xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);

  assert (pwr->c.xqos->present & QP_LIVELINESS);
  if (pwr->c.xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS)
    TRACE ((" FIXME: only AUTOMATIC liveliness supported"));
#if 0
  pwr->tlease_dur = nn_from_ddsi_duration (pwr->c.xqos->liveliness.lease_duration);
  if (pwr->tlease_dur == 0)
  {
    TRACE ((" FIXME: treating lease_duration=0 as inf"));
    pwr->tlease_dur = T_NEVER;
  }
  pwr->tlease_end = add_duration_to_time (tnow, pwr->tlease_dur);
#endif

  if (isreliable)
  {
    pwr->defrag = nn_defrag_new (NN_DEFRAG_DROP_LATEST, config.defrag_reliable_maxsamples);
    pwr->reorder = nn_reorder_new (NN_REORDER_MODE_NORMAL, config.primary_reorder_maxsamples);
  }
  else
  {
    pwr->defrag = nn_defrag_new (NN_DEFRAG_DROP_OLDEST, config.defrag_unreliable_maxsamples);
    pwr->reorder = nn_reorder_new (NN_REORDER_MODE_MONOTONICALLY_INCREASING, config.primary_reorder_maxsamples);
  }
  pwr->dqueue = dqueue;
  pwr->evq = evq;


  ephash_insert_proxy_writer_guid (pwr);
  match_proxy_writer_with_readers (pwr, tnow);
  return 0;
}

static void gc_delete_proxy_writer (struct gcreq *gcreq)
{
  struct proxy_writer *pwr = gcreq->arg;
  gcreq_free (gcreq);

  while (!ut_avlIsEmpty (&pwr->readers))
  {
    struct pwr_rd_match *m = ut_avlRoot (&pwr_readers_treedef, &pwr->readers);
    ut_avlDelete (&pwr_readers_treedef, &pwr->readers, m);
    reader_drop_connection (&m->rd_guid, &pwr->e.guid);
    update_reader_init_acknack_count (&m->rd_guid, m->count);
    free_pwr_rd_match (m);
  }

  proxy_endpoint_common_fini (&pwr->e, &pwr->c);
  nn_groupset_free (pwr->groups);
  c_free (pwr->v_message_qos);
  nn_defrag_free (pwr->defrag);
  nn_reorder_free (pwr->reorder);
  os_free (pwr);
}

int delete_proxy_writer (const struct nn_guid *guid)
{
  struct proxy_writer *pwr;
  os_mutexLock (&gv.lock);
  if ((pwr = ephash_lookup_proxy_writer_guid (guid)) == NULL)
  {
    os_mutexUnlock (&gv.lock);
    return ERR_UNKNOWN_ENTITY;
  }
  ephash_remove_proxy_writer_guid (pwr);
  os_mutexUnlock (&gv.lock);
  gcreq_proxy_writer (pwr);
  return 0;
}

/* PROXY-READER ----------------------------------------------------- */

int new_proxy_reader (const struct nn_guid *guid, struct addrset *as, nn_xqos_t *xqos)
{
  struct proxy_participant *proxypp;
  struct proxy_reader *prd;
  nn_guid_t ppguid;
  os_int64 tnow = now ();

  assert (!is_writer_entityid (guid->entityid));
  assert (ephash_lookup_proxy_reader_guid (guid) == NULL);

  ppguid.prefix = guid->prefix;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((proxypp = ephash_lookup_proxy_participant_guid (&ppguid)) == NULL)
  {
    NN_WARNING1 ("new_proxy_reader(%x:%x:%x:%x): proxy participant unknown\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }

  if ((prd = os_malloc (sizeof (*prd))) == NULL)
    return ERR_OUT_OF_MEMORY;

  if (proxy_endpoint_common_init (&prd->e, &prd->c, EK_PROXY_READER, guid, proxypp, as, xqos) != 0)
  {
    os_free (prd);
    return ERR_UNSPECIFIED;
  }

  prd->deleting = 0;
  ut_avlInit (&prd_writers_treedef, &prd->writers);
  ephash_insert_proxy_reader_guid (prd);
  match_proxy_reader_with_writers (prd, tnow);
  return 0;
}

static void proxy_reader_set_delete_and_ack_all_messages (struct proxy_reader *prd)
{
  nn_guid_t wrguid;
  struct writer *wr;
  struct prd_wr_match *m;

  memset (&wrguid, 0, sizeof (wrguid));
  os_mutexLock (&prd->e.lock);
  prd->deleting = 1;
  while ((m = ut_avlLookupSuccEq (&prd_writers_treedef, &prd->writers, &wrguid)) != NULL)
  {
    /* have to be careful walking the tree -- pretty is different, but
       I want to check this before I write a lookup_succ function. */
    struct prd_wr_match *m_a_next;
    nn_guid_t wrguid_next;
    wrguid = m->wr_guid;
    if ((m_a_next = ut_avlFindSucc (&prd_writers_treedef, &prd->writers, m)) != NULL)
      wrguid_next = m_a_next->wr_guid;
    else
    {
      memset (&wrguid_next, 0xff, sizeof (wrguid_next));
      wrguid_next.entityid.u = (wrguid_next.entityid.u & ~0xff) | NN_ENTITYID_KIND_WRITER_NO_KEY;
    }

    os_mutexUnlock (&prd->e.lock);
    if ((wr = ephash_lookup_writer_guid (&wrguid)) != NULL)
    {
      struct wr_prd_match *m_wr;
      os_mutexLock (&wr->e.lock);
      if ((m_wr = ut_avlLookup (&wr_readers_treedef, &wr->readers, &prd->e.guid)) != NULL)
      {
        m_wr->seq = MAX_SEQ_NUMBER;
        ut_avlAugmentUpdate (&wr_readers_treedef, m_wr);
        remove_acked_messages (wr);
      }
      os_mutexUnlock (&wr->e.lock);
    }

    wrguid = wrguid_next;
    os_mutexLock (&prd->e.lock);
  }
  os_mutexUnlock (&prd->e.lock);
}

static void gc_delete_proxy_reader (struct gcreq *gcreq)
{
  struct proxy_reader *prd = gcreq->arg;
  gcreq_free (gcreq);

  while (!ut_avlIsEmpty (&prd->writers))
  {
    struct prd_wr_match *m = ut_avlRoot (&prd_writers_treedef, &prd->writers);
    ut_avlDelete (&prd_writers_treedef, &prd->writers, m);
    writer_drop_connection (&m->wr_guid, &prd->e.guid);
    free_prd_wr_match (m);
  }

  proxy_endpoint_common_fini (&prd->e, &prd->c);
  os_free (prd);
}

int delete_proxy_reader (const struct nn_guid *guid)
{
  struct proxy_reader *prd;
  os_mutexLock (&gv.lock);
  if ((prd = ephash_lookup_proxy_reader_guid (guid)) == NULL)
  {
    os_mutexUnlock (&gv.lock);
    return ERR_UNKNOWN_ENTITY;
  }
  ephash_remove_proxy_reader_guid (prd);
  os_mutexUnlock (&gv.lock);

  /* If the proxy reader is reliable, pretend it has just acked all
     messages: this allows a throttled writer to once again make
     progress, which in turn is necessary for the garbage collector to
     do its work. */
  proxy_reader_set_delete_and_ack_all_messages (prd);

  gcreq_proxy_reader (prd);
  return 0;
}

/* CONVENIENCE FUNCTIONS FOR SCHEDULING GC WORK --------------------- */

static int gcreq_participant (struct participant *pp)
{
  struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_delete_participant);
  gcreq->arg = pp;
  gcreq_enqueue (gcreq);
  return 0;
}

static int gcreq_writer (struct writer *wr)
{
  struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_delete_writer);
  gcreq->arg = wr;
  gcreq_enqueue (gcreq);
  return 0;
}

static int gcreq_reader (struct reader *rd)
{
  struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_delete_reader);
  gcreq->arg = rd;
  gcreq_enqueue (gcreq);
  return 0;
}

static int gcreq_proxy_participant (struct proxy_participant *proxypp)
{
  struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_delete_proxy_participant);
  gcreq->arg = proxypp;
  gcreq_enqueue (gcreq);
  return 0;
}

static void gc_delete_proxy_writer_dqueue_bubble_cb (struct gcreq *gcreq)
{
  /* delete proxy_writer, phase 3 */
  struct proxy_writer *pwr = gcreq->arg;
  TRACE (("delete_proxy_writer_dqueue_bubble(%p)\n", pwr));
  gcreq_requeue (gcreq, gc_delete_proxy_writer);
}

static void gc_delete_proxy_writer_dqueue (struct gcreq *gcreq)
{
  /* delete proxy_writer, phase 2 */
  struct proxy_writer *pwr = gcreq->arg;
  struct nn_dqueue *dqueue = pwr->dqueue;
  TRACE (("delete_proxy_writer_dqueue(%p)\n", pwr));
  nn_dqueue_enqueue_callback (dqueue, (void (*) (void *)) gc_delete_proxy_writer_dqueue_bubble_cb, gcreq);
}

static int gcreq_proxy_writer (struct proxy_writer *pwr)
{
  struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_delete_proxy_writer_dqueue);
  gcreq->arg = pwr;
  gcreq_enqueue (gcreq);
  return 0;
}

static int gcreq_proxy_reader (struct proxy_reader *prd)
{
  struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_delete_proxy_reader);
  gcreq->arg = prd;
  gcreq_enqueue (gcreq);
  return 0;
}

/* SHA1 not available (unoffical build.) */
