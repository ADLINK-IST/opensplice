/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
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
#include "os_atomics.h"

#include "v_partition.h"
#include "v_entity.h"
#include "v_groupSet.h"

#include "q_entity.h"
#include "q_config.h"
#include "q_time.h"
#include "q_misc.h"
#include "q_log.h"
#include "ut_avl.h"
#include "q_whc.h"
#include "q_plist.h"
#include "q_lease.h"
#include "q_osplser.h"
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
#include "q_builtin_topic.h"
#include "ddsi_ser.h"

#include "sysdeps.h"

struct deleted_participant {
  ut_avlNode_t avlnode;
  nn_guid_t guid;
  unsigned for_what;
  nn_mtime_t t_prune;
};

static os_mutex deleted_participants_lock;
static ut_avlTree_t deleted_participants;
static const nn_vendorid_t ownvendorid = MY_VENDOR_ID;

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
const ut_avlTreedef_t proxypp_groups_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct proxy_group, avlnode), offsetof (struct proxy_group, guid), compare_guid, 0);

static const unsigned builtin_writers_besmask =
  NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
  NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER |
  NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER |
  NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
static const unsigned prismtech_builtin_writers_besmask =
  NN_DISC_BUILTIN_ENDPOINT_CM_PARTICIPANT_WRITER |
  NN_DISC_BUILTIN_ENDPOINT_CM_PUBLISHER_WRITER |
  NN_DISC_BUILTIN_ENDPOINT_CM_SUBSCRIBER_WRITER;

static struct writer * new_writer_guid
(
  const struct nn_guid *guid,
  const struct nn_guid *group_guid,
  struct participant *pp,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
);
static struct reader * new_reader_guid
(
  const struct nn_guid *guid,
  const struct nn_guid *group_guid,
  struct participant *pp,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
);
static struct participant *ref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity);
static void unref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity);
static void delete_proxy_group_locked (struct proxy_group *pgroup, int isimplicit);

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

int is_builtin_entityid (nn_entityid_t id, nn_vendorid_t vendorid)
{
  if ((id.u & NN_ENTITYID_SOURCE_MASK) == NN_ENTITYID_SOURCE_BUILTIN)
    return 1;
  else if ((id.u & NN_ENTITYID_SOURCE_MASK) != NN_ENTITYID_SOURCE_VENDOR)
    return 0;
  else if (!vendor_is_prismtech (vendorid))
    return 0;
  else
  {
    /* Currently only SOURCE_VENDOR entities are for CM "topics". */
    return 1;
  }
}

static int is_builtin_endpoint (nn_entityid_t id, nn_vendorid_t vendorid)
{
  return is_builtin_entityid (id, vendorid) && id.u != NN_ENTITYID_PARTICIPANT;
}

static void entity_common_init (struct entity_common *e, const struct nn_guid *guid, const char *name, enum entity_kind kind)
{
  e->guid = *guid;
  e->kind = kind;
  e->name = os_strdup (name ? name : "");
  os_mutexInit (&e->lock, NULL);
}

static void entity_common_fini (struct entity_common *e)
{
  os_free (e->name);
  os_mutexDestroy (&e->lock);
}

/* DELETED PARTICIPANTS --------------------------------------------- */

int deleted_participants_admin_init (void)
{
  os_mutexInit (&deleted_participants_lock, NULL);
  ut_avlInit (&deleted_participants_treedef, &deleted_participants);
  return 0;
}

void deleted_participants_admin_fini (void)
{
  ut_avlFree (&deleted_participants_treedef, &deleted_participants, os_free);
  os_mutexDestroy (&deleted_participants_lock);
}

static void prune_deleted_participant_guids_unlocked (nn_mtime_t tnow)
{
  /* Could do a better job of finding prunable ones efficiently under
     all circumstances, but I expect the tree to be very small at all
     times, so a full scan is fine, too ... */
  struct deleted_participant *dpp;
  dpp = ut_avlFindMin (&deleted_participants_treedef, &deleted_participants);
  while (dpp)
  {
    struct deleted_participant *dpp1 = ut_avlFindSucc (&deleted_participants_treedef, &deleted_participants, dpp);
    if (dpp->t_prune.v < tnow.v)
    {
      ut_avlDelete (&deleted_participants_treedef, &deleted_participants, dpp);
      os_free (dpp);
    }
    dpp = dpp1;
  }
}

static void prune_deleted_participant_guids (nn_mtime_t tnow)
{
  os_mutexLock (&deleted_participants_lock);
  prune_deleted_participant_guids_unlocked (tnow);
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
      n->t_prune.v = T_NEVER;
      n->for_what = DPG_LOCAL | DPG_REMOTE;
      ut_avlInsertIPath (&deleted_participants_treedef, &deleted_participants, n, &path);
    }
  }
  os_mutexUnlock (&deleted_participants_lock);
}

int is_deleted_participant_guid (const struct nn_guid *guid, unsigned for_what)
{
  struct deleted_participant *n;
  int known;
  os_mutexLock (&deleted_participants_lock);
  prune_deleted_participant_guids_unlocked (now_mt());
  if ((n = ut_avlLookup (&deleted_participants_treedef, &deleted_participants, guid)) == NULL)
    known = 0;
  else
    known = ((n->for_what & for_what) != 0);
  os_mutexUnlock (&deleted_participants_lock);
  return known;
}

static void remove_deleted_participant_guid (const struct nn_guid *guid, unsigned for_what)
{
  if (!config.prune_deleted_ppant.enforce_delay)
  {
    struct deleted_participant *n;
  TRACE (("remove_deleted_participant_guid(%x:%x:%x:%x for_what=%x)\n", PGUID (*guid), for_what));
    os_mutexLock (&deleted_participants_lock);
    if ((n = ut_avlLookup (&deleted_participants_treedef, &deleted_participants, guid)) != NULL)
    {
    if (config.prune_deleted_ppant.enforce_delay)
    {
      n->t_prune = add_duration_to_mtime (now_mt (), config.prune_deleted_ppant.delay);
    }
    else
    {
      n->for_what &= ~for_what;
      if (n->for_what != 0)
      {
        /* For local participants (remove called with LOCAL, leaving
           REMOTE blacklisted, and has to do with network briding) */
        n->t_prune = add_duration_to_mtime (now_mt (), config.prune_deleted_ppant.delay);
      }
      else
      {
        ut_avlDelete (&deleted_participants_treedef, &deleted_participants, n);
        os_free (n);
      }
    }
  }
    os_mutexUnlock (&deleted_participants_lock);
  }
}

/* DYNAMIC GROUP CREATION FOR WILDCARD ENDPOINTS -------------------- */
static void add_group_to_readers_and_proxy_writers_locked (const struct sertopic *topic, const char *name, v_group group)
{
  nn_xqos_t dummy_part_xqos;
  struct ephash_enum_reader est_rd;
  struct ephash_enum_proxy_writer est_pwr;
  struct proxy_writer *pwr;
  struct reader *rd;

  TRACE (("add_group_to_readers_and_proxy_writers_locked: %s.%s group %p scanning all readers/writers\n", name, topic->name, (void *) group));

  nn_xqos_init_empty (&dummy_part_xqos);
  dummy_part_xqos.present = QP_PARTITION;
  dummy_part_xqos.partition.n = 1;
  dummy_part_xqos.partition.strs = (char **) &name;

  ephash_enum_reader_init (&est_rd);
  while ((rd = ephash_enum_reader_next (&est_rd)) != NULL)
  {
    if (rd->topic == topic && partitions_match_p (rd->xqos, &dummy_part_xqos))
    {
      TRACE (("  add to rd %x:%x:%x:%x\n", PGUID (rd->e.guid)));
      nn_groupset_add_group (rd->matching_groups, group);
    }
  }
  ephash_enum_reader_fini (&est_rd);
  ephash_enum_proxy_writer_init (&est_pwr);
  while ((pwr = ephash_enum_proxy_writer_next (&est_pwr)) != NULL)
  {
    if (pwr->c.topic == topic && partitions_match_p (pwr->c.xqos, &dummy_part_xqos))
    {
      TRACE (("  add to pwr %x:%x:%x:%x\n", PGUID (pwr->e.guid)));
      nn_groupset_add_group (pwr->groups, group);
    }
  }
  ephash_enum_proxy_writer_fini (&est_pwr);

  /* Do NOT call nn_xqos_fini: even if (aliased & QP_PARTITION), it still frees "str". Here we know there are no resources to be freed. */
}

void add_group_to_readers_and_proxy_writers (const struct sertopic *topic, const char *name, v_group group)
{
  os_rwlockRead (&gv.qoslock);
  add_group_to_readers_and_proxy_writers_locked(topic, name, group);
  os_rwlockUnlock (&gv.qoslock);
}

static void create_a_group (const char *name, const struct sertopic *topic)
{
  v_partitionQos pqos;
  v_partition part;
  v_group group;

  ASSERT_RDLOCK_HELD (&gv.qoslock);

  /* any non-wildcard one will do */
  /* create it -- partitions require a v_partitionQos parameter, but
   that (thankfully!) isn't used by it ... phew! */
  TRACE (("create_a_group: %s.%s\n", name, topic->name));
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
  add_group_to_readers_and_proxy_writers_locked (topic, name, group);
}

/* PARTICIPANT ------------------------------------------------------ */

int pp_allocate_entityid (nn_entityid_t *id, unsigned kind, struct participant *pp)
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

int new_participant_guid (const nn_guid_t *ppguid, unsigned flags, const nn_plist_t *plist)
{
  struct participant *pp;
  nn_guid_t subguid, group_guid;

  /* no reserved bits may be set */
  assert ((flags & ~(RTPS_PF_NO_BUILTIN_READERS | RTPS_PF_NO_BUILTIN_WRITERS | RTPS_PF_PRIVILEGED_PP | RTPS_PF_IS_DDSI2_PP)) == 0);
  /* privileged participant MUST have builtin readers and writers */
  assert (!(flags & RTPS_PF_PRIVILEGED_PP) || (flags & (RTPS_PF_NO_BUILTIN_READERS | RTPS_PF_NO_BUILTIN_WRITERS)) == 0);

  prune_deleted_participant_guids (now_mt ());

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

  entity_common_init (&pp->e, ppguid, "", EK_PARTICIPANT);
  pp->user_refc = 1;
  pp->builtin_refc = 0;
  pp->builtins_deleted = 0;
  pp->is_ddsi2_pp = (flags & (RTPS_PF_PRIVILEGED_PP | RTPS_PF_IS_DDSI2_PP)) ? 1 : 0;
  os_mutexInit (&pp->refc_lock, NULL);
  pp->next_entityid = NN_ENTITYID_ALLOCSTEP;
  pp->lease_duration = config.lease_duration;
  pp->plist = os_malloc (sizeof (*pp->plist));
  nn_plist_copy (pp->plist, plist);
  nn_plist_mergein_missing (pp->plist, &gv.default_plist_pp);

  if (config.enabled_logcats & LC_TRACE)
  {
    TRACE (("PARTICIPANT %x:%x:%x:%x QOS={", PGUID (pp->e.guid)));
    nn_log_xqos (LC_TRACE, &pp->plist->qos);
    TRACE (("}\n"));
  }

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

  /* Create built-in endpoints (note: these have no GID, and no group GUID). */
  pp->bes = 0;
  pp->prismtech_bes = 0;
  subguid.prefix = pp->e.guid.prefix;
  memset (&group_guid, 0, sizeof (group_guid));
  /* SPDP writer */
#define LAST_WR_PARAMS NULL, NULL, NULL

  /* Note: skip SEDP <=> skip SPDP because of the way ddsi_discovery.c does things
     currently.  */
  if (!(flags & RTPS_PF_NO_BUILTIN_WRITERS))
  {
    subguid.entityid = to_entityid (NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.spdp_endpoint_xqos, LAST_WR_PARAMS);
    /* But we need the as_disc address set for SPDP, because we need to
       send it to everyone regardless of the existence of readers. */
    {
      struct writer *wr = ephash_lookup_writer_guid (&subguid);
      assert (wr != NULL);
      os_mutexLock (&wr->e.lock);
      unref_addrset (wr->as);
      unref_addrset (wr->as_group);
      wr->as = ref_addrset (gv.as_disc);
      wr->as_group = ref_addrset (gv.as_disc_group);
      os_mutexUnlock (&wr->e.lock);
    }
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
  }

  /* Make it globally visible, else the endpoint matching won't work. */
  ephash_insert_participant_guid (pp);

  /* SEDP writers: */
  if (!(flags & RTPS_PF_NO_BUILTIN_WRITERS))
  {
    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->prismtech_bes |= NN_DISC_BUILTIN_ENDPOINT_CM_PARTICIPANT_WRITER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->prismtech_bes |= NN_DISC_BUILTIN_ENDPOINT_CM_PUBLISHER_WRITER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->prismtech_bes |= NN_DISC_BUILTIN_ENDPOINT_CM_SUBSCRIBER_WRITER;
  }

  if (flags & RTPS_PF_PRIVILEGED_PP)
  {
    /* TODO: make this one configurable, we don't want all participants to publish all topics (or even just those that they use themselves) */
    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_TOPIC_ANNOUNCER;
  }

  /* PMD writer: */
  if (!(flags & RTPS_PF_NO_BUILTIN_WRITERS))
  {
    subguid.entityid = to_entityid (NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER);
    new_writer_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_wr, LAST_WR_PARAMS);
    pp->bes |= NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
  }

  /* SPDP, SEDP, PMD readers: */
  if (!(flags & RTPS_PF_NO_BUILTIN_READERS))
  {
    subguid.entityid = to_entityid (NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.spdp_endpoint_xqos, NULL, NULL, NULL);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;

    subguid.entityid = to_entityid (NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
    pp->bes |= NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
    pp->prismtech_bes |= NN_DISC_BUILTIN_ENDPOINT_CM_PARTICIPANT_READER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
    pp->prismtech_bes |= NN_DISC_BUILTIN_ENDPOINT_CM_PUBLISHER_READER;

    subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_READER);
    new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
    pp->prismtech_bes |= NN_DISC_BUILTIN_ENDPOINT_CM_SUBSCRIBER_READER;

    if (config.generate_builtin_topics)
    {
      /* No point in having a reader for topic discovery if we won't be generating the corresponding DCPSTopic, since that is the only thing it is currently being used for */
      subguid.entityid = to_entityid (NN_ENTITYID_SEDP_BUILTIN_TOPIC_READER);
      new_reader_guid (&subguid, &group_guid, pp, NULL, &gv.builtin_endpoint_xqos_rd, NULL, NULL, NULL);
      pp->bes |= NN_DISC_BUILTIN_ENDPOINT_TOPIC_DETECTOR;
    }
  }
#undef LAST_WR_PARAMS

  /* If the participant doesn't have the full set of builtin writers
     it depends on the privileged participant, which must exist, hence
     the reference count of the privileged participant is incremented.
     If it is the privileged participant, set the global variable
     pointing to it. */
  os_mutexLock (&gv.privileged_pp_lock);
  if ((pp->bes & builtin_writers_besmask) != builtin_writers_besmask ||
      (pp->prismtech_bes & prismtech_builtin_writers_besmask) != prismtech_builtin_writers_besmask)
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
    pa_fence ();
    pa_inc32 (&gv.participant_set_generation);
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
       since we're new alive, PMD updates can now start, too.
       Schedule the first update for 100ms in the future to reduce the
       impact of the first sample getting lost.  Note: these two may
       fire before the calls return.  If the initial sample wasn't
       accepted, all is lost, but we continue nonetheless, even though
       the participant won't be able to discover or be discovered.  */
    pp->spdp_xevent = qxev_spdp (add_duration_to_mtime (now_mt (), 100 * T_MILLISECOND), &pp->e.guid, NULL);
  }

  /* Also write the CM data - this one being transient local, we only
   need to write it once (or when it changes, I suppose) */
  sedp_write_cm_participant (pp, 1);

  {
    nn_mtime_t tsched;
    tsched.v = (pp->lease_duration == T_NEVER) ? T_NEVER : 0;
    pp->pmd_update_xevent = qxev_pmd_update (tsched, &pp->e.guid);
  }

  return 0;
}


static void delete_builtin_endpoint (const struct nn_guid *ppguid, unsigned entityid)
{
  nn_guid_t guid;
  guid.prefix = ppguid->prefix;
  guid.entityid.u = entityid;
  assert (is_builtin_entityid (to_entityid (entityid), ownvendorid));
  if (is_writer_entityid (to_entityid (entityid)))
    delete_writer_nolinger (&guid);
  else
    delete_reader (&guid);
}

static struct participant *ref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity)
{
  nn_guid_t stguid;
  os_mutexLock (&pp->refc_lock);
  if (guid_of_refing_entity && is_builtin_endpoint (guid_of_refing_entity->entityid, ownvendorid))
    pp->builtin_refc++;
  else
    pp->user_refc++;

  if (guid_of_refing_entity)
    stguid = *guid_of_refing_entity;
  else
    memset (&stguid, 0, sizeof (stguid));
  TRACE (("ref_participant(%x:%x:%x:%x @ %p <- %x:%x:%x:%x @ %p) user %d builtin %d\n",
          PGUID (pp->e.guid), (void*)pp, PGUID (stguid), (void*)guid_of_refing_entity, pp->user_refc, pp->builtin_refc));
  os_mutexUnlock (&pp->refc_lock);
  return pp;
}

static void unref_participant (struct participant *pp, const struct nn_guid *guid_of_refing_entity)
{
  static const unsigned builtin_endpoints_tab[] = {
    NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER,
    NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER,
    NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER,
    NN_ENTITYID_SEDP_BUILTIN_TOPIC_READER,
    NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER,
    NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER,
    NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER,
    /* PrismTech ones: */
    NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_READER,
    NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_READER,
    NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_WRITER,
    NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_READER
  };
  nn_guid_t stguid;

  os_mutexLock (&pp->refc_lock);
  if (guid_of_refing_entity && is_builtin_endpoint (guid_of_refing_entity->entityid, ownvendorid))
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
          PGUID (pp->e.guid), (void*)pp, PGUID (stguid), (void*)guid_of_refing_entity, pp->user_refc, pp->builtin_refc));

  if (pp->user_refc == 0 && (pp->bes != 0 || pp->prismtech_bes != 0) && !pp->builtins_deleted)
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

    /* We don't care, but other implementations might: */
    sedp_write_cm_participant (pp, 0);

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

    if ((pp->bes & builtin_writers_besmask) != builtin_writers_besmask ||
        (pp->prismtech_bes & prismtech_builtin_writers_besmask) != prismtech_builtin_writers_besmask)
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
      pa_fence_rel ();
      pa_inc32 (&gv.participant_set_generation);

      /* Deleting the socket will usually suffice to wake up the
         receiver threads, but in general, no one cares if it takes a
         while longer for it to wakeup. */

      ddsi_conn_free (pp->m_conn);
    }
    nn_plist_fini (pp->plist);
    os_free (pp->plist);
    os_mutexDestroy (&pp->refc_lock);
    entity_common_fini (&pp->e);
    remove_deleted_participant_guid (&pp->e.guid, DPG_LOCAL);
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
  TRACE (("gc_delete_participant(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (pp->e.guid)));
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
  unsigned bes_mask = 0, prismtech_bes_mask = 0;

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
    case NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_WRITER:
      prismtech_bes_mask = NN_DISC_BUILTIN_ENDPOINT_CM_PARTICIPANT_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_WRITER:
      prismtech_bes_mask = NN_DISC_BUILTIN_ENDPOINT_CM_PUBLISHER_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_WRITER:
      prismtech_bes_mask = NN_DISC_BUILTIN_ENDPOINT_CM_SUBSCRIBER_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER:
      bes_mask = NN_DISC_BUILTIN_ENDPOINT_TOPIC_ANNOUNCER;
      break;
    default:
      NN_FATAL1 ("get_builtin_writer called with entityid %x\n", entityid);
      return NULL;
  }

  if ((pp->bes & bes_mask) || (pp->prismtech_bes & prismtech_bes_mask))
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

static int rebuild_writer_addrset_addone_mc (struct addrset *as, const struct wr_prd_match *m
                                             )
{
  /* prd->c.as better be constant, or else we need to do this sort of
     thing also when the proxy reader's address set changes */
  struct proxy_reader *prd;
  nn_locator_t loc;
  if ((prd = ephash_lookup_proxy_reader_guid (&m->prd_guid)) != NULL)
  {
    if (addrset_any_mc (prd->c.as, &loc))
      add_to_addrset (as, &loc);
    else if (addrset_any_uc (prd->c.as, &loc))
      add_to_addrset (as, &loc);
  }
  return 0;
}

static int rebuild_writer_addrset_addone_uc (struct addrset *as, const struct wr_prd_match *m)
{
  /* Returns 1 iff reader requires use of multicast, else 0. */
  struct proxy_reader *prd;
  nn_locator_t loc;
  if ((prd = ephash_lookup_proxy_reader_guid (&m->prd_guid)) == NULL)
    return 0;
  else if (!addrset_any_uc (prd->c.as, &loc))
    return 1;
  else
  {
    add_to_addrset (as, &loc);
    return 0;
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
    struct wr_prd_match *m;
    ut_avlIter_t it;
    int need_mc = 0;
    /* First try if we are happy with just unicast; if transmitting
       multicasts is not an option, then we are by definition */
    for (m = ut_avlIterFirst (&wr_readers_treedef, &wr->readers, &it); m; m = ut_avlIterNext (&it))
      need_mc += rebuild_writer_addrset_addone_uc (newas, m);
    if (config.allowMulticast & ~AMC_SPDP)
    {
      /* Try multicasting if there are readers that do not have a
         unicast address (can that happen?) or if we need multiple
         unicasts */
      if (addrset_count_uc (newas) > 1 || need_mc)
      {
        addrset_purge (newas);
        for (m = ut_avlIterFirst (&wr_readers_treedef, &wr->readers, &it); m; m = ut_avlIterNext (&it))
        {
          (void) rebuild_writer_addrset_addone_mc (newas, m);
        }
      }
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

#include "v_public.h"
#include "v_reader.h"

static void notify_wait_for_historical_data_impl (const nn_guid_t *rd_guid)
{
  struct reader *rd;
  v_public kr;

  if ((rd = ephash_lookup_reader_guid(rd_guid)) == NULL)
  {
    TRACE(("wfh(%x:%x:%x:%x ddsi2-reader-gone)\n", PGUID(*rd_guid)));
    return;
  }
  if (v_gidIsNil(rd->c.gid))
  {
    return;
  }
  TRACE(("wfh(%x:%x:%x:%x gid %x:%x:%x ", PGUID(*rd_guid), rd->c.gid.systemId, rd->c.gid.localId, rd->c.gid.serial));
  if (rd->c.gid.systemId != gv.ospl_kernel->GID.systemId)
  {
    /* bridging mode can mean proxying remote entities, no possibility to do anything for those */
    TRACE(("bridged-remote-reader)\n"));
    return;
  }

  /* Must not actually notify the reader until in sync with all matching proxy writers. Maintaining that state in the reader is hard because of all the asynchronous events and the attendant risk of deadlocks (though not impossible), but checking it here, from the delivery queue thread is "easy". */
  {
    struct rd_pwr_match *rdm;
    int all_complete = 1;
    os_mutexLock (&rd->e.lock);
    rdm = ut_avlFindMin (&rd_writers_treedef, &rd->writers);
    while (rdm != NULL && all_complete)
    {
      nn_guid_t pwr_guid = rdm->pwr_guid;
      struct proxy_writer *pwr;
      if ((pwr = ephash_lookup_proxy_writer_guid (&pwr_guid)) != NULL)
      {
        struct pwr_rd_match *pwrm;
        os_mutexUnlock (&rd->e.lock);
        os_mutexLock (&pwr->e.lock);
        if ((pwrm = ut_avlLookup (&pwr_readers_treedef, &pwr->readers, rd_guid)) != NULL && pwrm->in_sync != PRMSS_SYNC)
        {
          TRACE(("pwr %x:%x:%x:%x incomplete)\n", PGUID(pwr_guid)));
          all_complete = 0;
        }
        os_mutexUnlock (&pwr->e.lock);
        os_mutexLock (&rd->e.lock);
      }
      rdm = ut_avlLookupSucc (&rd_writers_treedef, &rd->writers, &pwr_guid);
    }
    os_mutexUnlock (&rd->e.lock);
    if (!all_complete)
    {
      return;
    }
  }

  if (v_gidClaimChecked (rd->c.gid, gv.ospl_kernel, &kr) != V_HANDLE_OK)
  {
    TRACE(("kernel-reader-gone)\n"));
    return;
  }

  if (c_instanceOf(kr, "v_reader"))
  {
    TRACE(("notifying)\n"));
    v_readerNotifyStateChange(v_reader(kr),TRUE);
  }
  else
  {
    TRACE(("kernel-object-not-a-reader)\n"));
  }

  v_gidRelease (rd->c.gid, gv.ospl_kernel);
}

struct notify_wait_for_historical_data_cb_arg {
  nn_guid_t rd_guid;
};

static void notify_wait_for_historical_data_cb (void *varg)
{
  struct notify_wait_for_historical_data_cb_arg *arg = varg;
  notify_wait_for_historical_data_impl (&arg->rd_guid);
  os_free(arg);
}

void notify_wait_for_historical_data (struct proxy_writer *pwr, const nn_guid_t *rd_guid)
{
  /* always trigger asynchronously via the delivery queue: data received in "out-of-sync" mode is
   delivered asynchronously, and the overhead of the asynchronous notification is negligible */
  nn_vendorid_t vendorid = MY_VENDOR_ID;
  if (!is_builtin_entityid(rd_guid->entityid, vendorid))
  {
    struct notify_wait_for_historical_data_cb_arg *arg;
    TRACE (("msr_in_sync(%x:%x:%x:%x queue-wfh)\n", PGUID (*rd_guid)));
    arg = os_malloc(sizeof(*arg));
    arg->rd_guid = *rd_guid;
    nn_dqueue_enqueue_callback(pwr ? pwr->dqueue : gv.builtins_dqueue, notify_wait_for_historical_data_cb, arg);
  }
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
  if (m)
  {
    os_free (m);
  }
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

static void writer_drop_connection (const struct nn_guid * wr_guid, const struct proxy_reader * prd)
{
  struct writer *wr;
  if ((wr = ephash_lookup_writer_guid (wr_guid)) != NULL)
  {
    struct wr_prd_match *m;
    os_mutexLock (&wr->e.lock);
    if ((m = ut_avlLookup (&wr_readers_treedef, &wr->readers, &prd->e.guid)) != NULL)
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

static void reader_drop_connection (const struct nn_guid *rd_guid, const struct proxy_writer * pwr)
{
  struct reader *rd;
  if ((rd = ephash_lookup_reader_guid (rd_guid)) != NULL)
  {
    struct rd_pwr_match *m;
    os_mutexLock (&rd->e.lock);
    if ((m = ut_avlLookup (&rd_writers_treedef, &rd->writers, &pwr->e.guid)) != NULL)
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
    {
      ut_avlDelete (&pwr_readers_treedef, &pwr->readers, m);
      if (m->in_sync != PRMSS_SYNC)
      {
        pwr->n_readers_out_of_sync--;
        notify_wait_for_historical_data (pwr, &rd->e.guid);
      }
    }
    if (rd->reliable)
    {
      pwr->n_reliable_readers--;
    }


    os_mutexUnlock (&pwr->e.lock);
    if (m != NULL)
    {
      update_reader_init_acknack_count (&rd->e.guid, m->count);
    }
    free_pwr_rd_match (m);
  }
}

static void proxy_reader_drop_connection
  (const struct nn_guid *prd_guid, struct writer * wr)
{
  struct proxy_reader *prd;
  if ((prd = ephash_lookup_proxy_reader_guid (prd_guid)) != NULL)
  {
    struct prd_wr_match *m;
    os_mutexLock (&prd->e.lock);
    m = ut_avlLookup (&prd_writers_treedef, &prd->writers, &wr->e.guid);
    if (m)
    {
      ut_avlDelete (&prd_writers_treedef, &prd->writers, m);
    }
    os_mutexUnlock (&prd->e.lock);
    free_prd_wr_match (m);
  }
}

static void writer_add_connection (struct writer *wr, struct proxy_reader *prd)
{
  struct wr_prd_match *m = os_malloc (sizeof (*m));
  ut_avlIPath_t path;
  int pretend_everything_acked;
  m->prd_guid = prd->e.guid;
  m->is_reliable = (prd->c.xqos->reliability.kind > NN_BEST_EFFORT_RELIABILITY_QOS);
  m->assumed_in_sync = (config.retransmit_merging == REXMIT_MERGE_ALWAYS);
  m->has_replied_to_hb = !m->is_reliable;
  m->all_have_replied_to_hb = 0;
  m->non_responsive_count = 0;
  m->rexmit_requests = 0;
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
    pretend_everything_acked = 0;
  }
  os_mutexUnlock (&prd->e.lock);
  m->next_acknack = DDSI_COUNT_MIN;
  m->next_nackfrag = DDSI_COUNT_MIN;
  nn_lat_estim_init (&m->hb_to_ack_latency);
  m->hb_to_ack_latency_tlastlog = now ();
  m->t_acknack_accepted.v = 0;

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
    TRACE (("  writer_add_connection(wr %x:%x:%x:%x prd %x:%x:%x:%x) - ack seq %"PA_PRId64"\n",
            PGUID (wr->e.guid), PGUID (prd->e.guid), m->seq));
    ut_avlInsertIPath (&wr_readers_treedef, &wr->readers, m, &path);
    rebuild_writer_addrset (wr);
    wr->num_reliable_readers += m->is_reliable;
    os_mutexUnlock (&wr->e.lock);

    /* for proxy readers using a non-wildcard partition matching a
     wildcard partition at the writer (and only a wildcard partition),
     ensure that a matching non-wildcard partition exists */
    if (!is_builtin_entityid (wr->e.guid.entityid, ownvendorid))
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
      const nn_mtime_t tnext = add_duration_to_mtime (now_mt (), delta);
      os_mutexLock (&wr->e.lock);
      /* To make sure that we keep sending heartbeats at a higher rate
         at the start of this discovery, reset the hbs_since_last_write
         count to zero. */
      wr->hbcontrol.hbs_since_last_write = 0;
      if (tnext.v < wr->hbcontrol.tsched.v)
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
  struct rd_pwr_match *m = os_malloc (sizeof (*m));
  ut_avlIPath_t path;

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
  unsigned i;
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

static void proxy_writer_add_connection (struct proxy_writer *pwr, struct reader *rd, nn_mtime_t tnow /* monotonic */, nn_count_t init_count)
{
  struct pwr_rd_match *m = os_malloc (sizeof (*m));
  ut_avlIPath_t path;
  os_int64 last_deliv_seq;

  os_mutexLock (&pwr->e.lock);
  if (ut_avlLookupIPath (&pwr_readers_treedef, &pwr->readers, &rd->e.guid, &path))
    goto already_matched;

  if (pwr->c.topic == NULL)
    pwr->c.topic = rd->topic;

  TRACE (("  proxy_writer_add_connection(pwr %x:%x:%x:%x rd %x:%x:%x:%x)",
          PGUID (pwr->e.guid), PGUID (rd->e.guid)));
  m->rd_guid = rd->e.guid;
  m->tcreate = now_mt ();

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
    if (!is_builtin_entityid (rd->e.guid.entityid, ownvendorid) && ngroups == 0)
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
  m->hb_timestamp.v = 0;
  m->t_heartbeat_accepted.v = 0;
  m->t_last_nack.v = 0;
  m->seq_last_nack = 0;

  /* These can change as a consequence of handling data and/or
     discovery activities. The safe way of dealing with them is to
     lock the proxy writer */
  last_deliv_seq = nn_reorder_next_seq (pwr->reorder) - 1;
  if (!rd->handle_as_transient_local)
  {
    m->in_sync = PRMSS_SYNC;
  }
  else if (last_deliv_seq == 0)
  {
    /* proxy-writer hasn't seen any data yet, in which case this reader is in sync with the proxy writer (i.e., no reader-specific reorder buffer needed), but still should generate a notification when all historical data has been received, except for the built-in ones (for now anyway, it may turn out to be useful for determining discovery status). */
    m->in_sync = is_builtin_entityid (rd->e.guid.entityid, ownvendorid) ? PRMSS_SYNC : PRMSS_TLCATCHUP;
    m->u.not_in_sync.end_of_tl_seq = MAX_SEQ_NUMBER;
    if (m->in_sync != PRMSS_SYNC)
      TRACE ((" - tlcatchup"));
  }
  else if (!config.conservative_builtin_reader_startup && is_builtin_entityid (rd->e.guid.entityid, ownvendorid) && !ut_avlIsEmpty (&pwr->readers))
  {
    /* builtins really don't care about multiple copies */
    m->in_sync = PRMSS_SYNC;
  }
  else
  {
    /* normal transient-local, reader is behind proxy writer */
    m->in_sync = PRMSS_OUT_OF_SYNC;
    m->u.not_in_sync.end_of_tl_seq = MAX_SEQ_NUMBER;
    m->u.not_in_sync.end_of_out_of_sync_seq = last_deliv_seq;
    TRACE ((" - out-of-sync %"PA_PRId64, m->u.not_in_sync.end_of_out_of_sync_seq));
  }
  if (m->in_sync != PRMSS_SYNC)
    pwr->n_readers_out_of_sync++;
  m->count = init_count;
  /* Spec says we may send a pre-emptive AckNack (8.4.2.3.4), hence we
     schedule it for the configured delay * T_MILLISECOND. From then
     on it it'll keep sending pre-emptive ones until the proxy writer
     receives a heartbeat.  (We really only need a pre-emptive AckNack
     per proxy writer, but hopefully it won't make that much of a
     difference in practice.) */
  if (rd->reliable)
  {
    m->acknack_xevent = qxev_acknack (pwr->evq, add_duration_to_mtime (tnow, config.preemptive_ack_delay), &pwr->e.guid, &rd->e.guid);
    m->u.not_in_sync.reorder =
      nn_reorder_new (NN_REORDER_MODE_NORMAL, config.secondary_reorder_maxsamples);
    pwr->n_reliable_readers++;
  }
  else
  {
    m->acknack_xevent = NULL;
    m->u.not_in_sync.reorder =
      nn_reorder_new (NN_REORDER_MODE_MONOTONICALLY_INCREASING, config.secondary_reorder_maxsamples);
  }

  ut_avlInsertIPath (&pwr_readers_treedef, &pwr->readers, m, &path);


  os_mutexUnlock (&pwr->e.lock);
  qxev_pwr_entityid (pwr, &rd->e.guid.prefix);

  TRACE (("\n"));


  return;

already_matched:
  assert (is_builtin_entityid (pwr->e.guid.entityid, pwr->c.vendor) ? (pwr->c.topic == NULL) : (pwr->c.topic != NULL));
  TRACE (("  proxy_writer_add_connection(pwr %x:%x:%x:%x rd %x:%x:%x:%x) - already connected\n",
          PGUID (pwr->e.guid), PGUID (rd->e.guid)));
  os_mutexUnlock (&pwr->e.lock);
  os_free (m);
  return;
}

static void proxy_reader_add_connection (struct proxy_reader *prd, struct writer *wr)
{
  struct prd_wr_match *m = os_malloc (sizeof (*m));
  ut_avlIPath_t path;

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
    qxev_prd_entityid (prd, &wr->e.guid.prefix);
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

    case NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_READER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_WRITER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_READER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_READER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_WRITER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_READER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_READER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_WRITER;
      break;
    case NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_WRITER:
      res.u = NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_READER;
      break;

    default:
      assert (0);
  }
  return res;
}


static void match_writer_with_proxy_readers (struct writer *wr, UNUSED_ARG (nn_mtime_t tnow))
{
  struct proxy_reader *prd;
  os_int32 reason;

  if (!is_builtin_entityid (wr->e.guid.entityid, ownvendorid))
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
      if (is_builtin_entityid (prd->e.guid.entityid, prd->c.vendor))
        continue;
      reason = qos_match_p (prd->c.xqos, wr->xqos);
      if (reason == -1)
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


static void match_reader_with_proxy_writers (struct reader *rd, nn_mtime_t tnow /* monotonic */)
{
  /* see match_writer_with_proxy_readers for comments */
  struct proxy_writer *pwr;
  os_int32 reason;

  if (!is_builtin_entityid (rd->e.guid.entityid, ownvendorid))
  {
    struct ephash_enum_proxy_writer est;
    TRACE (("match_reader_with_proxy_writers(wr %x:%x:%x:%x) scanning all proxy writers\n", PGUID (rd->e.guid)));
    ephash_enum_proxy_writer_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((pwr = ephash_enum_proxy_writer_next (&est)) != NULL)
    {
      if (is_builtin_entityid (pwr->e.guid.entityid, pwr->c.vendor))
        continue;
      reason = qos_match_p (rd->xqos, pwr->c.xqos);
      if (reason == -1)
      {
        nn_count_t init_count;
        reader_add_connection (rd, pwr, &init_count);
        proxy_writer_add_connection (pwr, rd, tnow /* monotonic */, init_count);
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
          proxy_writer_add_connection (pwr, rd, tnow /* monotonic */, init_count);
        }
      }
      ephash_enum_proxy_participant_fini (&est);
    }
  }
}

static void match_proxy_writer_with_readers (struct proxy_writer *pwr, nn_mtime_t tnow /* monotonic */)
{
  /* see match_writer_with_proxy_readers for comments */

  struct reader *rd;
  os_int32 reason;

  if (!is_builtin_entityid (pwr->e.guid.entityid, pwr->c.vendor))
  {
    struct ephash_enum_reader est;
    TRACE (("match_proxy_writer_with_readers(pwr %x:%x:%x:%x) scanning all readers\n", PGUID (pwr->e.guid)));
    ephash_enum_reader_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((rd = ephash_enum_reader_next (&est)) != NULL)
    {
      if (is_builtin_entityid (rd->e.guid.entityid, ownvendorid))
      {
        continue;
      }
      reason = qos_match_p (rd->xqos, pwr->c.xqos);
      if (reason == -1)
      {
        nn_count_t init_count;
        reader_add_connection (rd, pwr, &init_count);
        proxy_writer_add_connection (pwr, rd, tnow /* monotonic */, init_count);
      }
    }
    os_rwlockUnlock (&gv.qoslock);
    ephash_enum_reader_fini (&est);
  }
  else
  {
    nn_entityid_t tgt_ent = builtin_entityid_match (pwr->e.guid.entityid);
    struct ephash_enum_participant est;
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
          proxy_writer_add_connection (pwr, rd, tnow /* monotonic */, init_count);
        }
      }
      ephash_enum_participant_fini (&est);
    }
  }
}

static void match_proxy_reader_with_writers (struct proxy_reader *prd, UNUSED_ARG (nn_mtime_t tnow))
{
  /* see match_writer_with_proxy_readers for comments */
  struct writer *wr;
  os_int32 reason;

  if (!is_builtin_entityid (prd->e.guid.entityid, prd->c.vendor))
  {
    struct ephash_enum_writer est;
    TRACE (("match_proxy_reader_with_writers(prd %x:%x:%x:%x) scanning all writers\n", PGUID (prd->e.guid)));
    ephash_enum_writer_init (&est);
    os_rwlockRead (&gv.qoslock);
    while ((wr = ephash_enum_writer_next (&est)) != NULL)
    {
      if (is_builtin_entityid (wr->e.guid.entityid, ownvendorid))
        continue;
      reason = qos_match_p (prd->c.xqos, wr->xqos);
      if (reason == -1)
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

static void new_reader_writer_common (const struct nn_guid *guid, const struct sertopic * topic, const struct nn_xqos *xqos)
{
  const char *partition = "(default)";
  const char *partition_suffix = "";
  assert (is_builtin_entityid (guid->entityid, ownvendorid) ? (topic == NULL) : (topic != NULL));
  if (is_builtin_entityid (guid->entityid, ownvendorid))
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
          topic ? topic->name : "(null)",
          topic ? topic->typename : "(null)");
}

static void endpoint_common_init
(
  struct entity_common *e,
  struct endpoint_common *c,
  enum entity_kind kind,
  const struct nn_guid *guid,
  const struct nn_guid *group_guid,
  struct participant *pp
  ,const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
)
{
  entity_common_init (e, guid, endpoint_name, kind);
  if (gid)
  {
    assert (group_gid != NULL);
    c->gid = *gid;
    c->group_gid = *group_gid;
  }
  else
  {
    memset (&c->gid, 0, sizeof (c->gid));
    memset (&c->group_gid, 0, sizeof (c->group_gid));
  }
  c->pp = ref_participant (pp, &e->guid);
  if (group_guid)
  {
    c->group_guid = *group_guid;
  }
  else
  {
    memset (&c->group_guid, 0, sizeof (c->group_guid));
  }
}

static void endpoint_common_fini (struct entity_common *e, struct endpoint_common *c)
{
  unref_participant (c->pp, &e->guid);
  entity_common_fini (e);
}

static int set_topic_type_name (nn_xqos_t *xqos, const struct sertopic * topic)
{
  if (!(xqos->present & QP_TYPE_NAME) && topic)
  {
    xqos->present |= QP_TYPE_NAME;
    xqos->type_name = os_strdup (topic->typename);
  }
  if (!(xqos->present & QP_TOPIC_NAME) && topic)
  {
    xqos->present |= QP_TOPIC_NAME;
    xqos->topic_name = os_strdup (topic->name);
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
  int have_replied = n->has_replied_to_hb;

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
  n->all_have_replied_to_hb = have_replied ? 1 : 0;

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

void writer_set_retransmitting (struct writer *wr)
{
  assert (!wr->retransmitting);
  wr->retransmitting = 1;
  if (config.whc_adaptive && wr->whc_high > wr->whc_low)
  {
    os_uint32 m = 8 * wr->whc_high / 10;
    wr->whc_high = (m > wr->whc_low) ? m : wr->whc_low;
  }
}

void writer_clear_retransmitting (struct writer *wr)
{
  wr->retransmitting = 0;
  wr->t_whc_high_upd = wr->t_rexmit_end = now_et();
  os_condBroadcast (&wr->throttle_cond);
}

unsigned remove_acked_messages (struct writer *wr)
{
  unsigned n;
  os_size_t n_unacked;
  assert (wr->e.guid.entityid.u != NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
  ASSERT_MUTEX_HELD (&wr->e.lock);
  n = whc_remove_acked_messages (wr->whc, writer_max_drop_seq (wr));
  /* when transitioning from >= low-water to < low-water, signal
     anyone waiting in throttle_writer() */
  n_unacked = whc_unacked_bytes (wr->whc);
  if (wr->throttling && n_unacked <= wr->whc_low)
    os_condBroadcast (&wr->throttle_cond);
  if (wr->retransmitting && whc_unacked_bytes (wr->whc) == 0)
    writer_clear_retransmitting (wr);
  if (wr->state == WRST_LINGERING && n_unacked == 0)
  {
    nn_log (LC_DISCOVERY, "remove_acked_messages: deleting lingering writer %x:%x:%x:%x\n", PGUID (wr->e.guid));
    delete_writer_nolinger_locked (wr);
  }
  return n;
}

static struct writer * new_writer_guid
(
  const struct nn_guid *guid,
  const struct nn_guid *group_guid,
  struct participant *pp,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
)
{
  const os_size_t sample_overhead = 120; /* INFO_TS + DATA (approximate figure) + inline QoS */
  struct writer *wr;
  nn_mtime_t tnow = now_mt ();

  assert (is_writer_entityid (guid->entityid));
  assert (ephash_lookup_writer_guid (guid) == NULL);
  assert (memcmp (&guid->prefix, &pp->e.guid.prefix, sizeof (guid->prefix)) == 0);

  new_reader_writer_common (guid, topic, xqos);
  wr = os_malloc (sizeof (*wr));

  /* want a pointer to the participant so that a parallel call to
     delete_participant won't interfere with our ability to address
     the participant */

  endpoint_common_init (&wr->e, &wr->c, EK_WRITER, guid, group_guid, pp, gid, group_gid, endpoint_name);

  os_condInit (&wr->throttle_cond, &wr->e.lock, NULL);
  wr->seq = 0;
  wr->cs_seq = 0;
  wr->seq_xmit = 0;
  wr->hbcount = 0;
  wr->state = WRST_OPERATIONAL;
  wr->hbfragcount = 0;
  /* kernel starts the sequenceNumbers at 0, which must be different
     from what we initialise it with */
  wr->last_kernel_seq = 0xffffffff;
  writer_hbcontrol_init (&wr->hbcontrol);
  wr->throttling = 0;
  wr->retransmitting = 0;
  wr->t_rexmit_end.v = 0;
  wr->t_whc_high_upd.v = 0;
  wr->num_reliable_readers = 0;
  wr->num_acks_received = 0;
  wr->num_nacks_received = 0;
  wr->throttle_count = 0;
  wr->rexmit_count = 0;
  wr->rexmit_lost_count = 0;


  /* Copy QoS, merging in defaults */

  wr->xqos = os_malloc (sizeof (*wr->xqos));
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
  if (is_builtin_entityid (wr->e.guid.entityid, ownvendorid))
  {
    assert (wr->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS);
    wr->aggressive_keep_last = 1;
  }
  else
  {
    wr->aggressive_keep_last = (config.aggressive_keep_last_whc && wr->xqos->history.kind == NN_KEEP_LAST_HISTORY_QOS);
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
  wr->as_group = NULL;



  /* for non-builtin writers, select the eventqueue based on the channel it is mapped to */

  {
    wr->evq = gv.xevents;
  }

  /* heartbeat event will be deleted when the handler can't find a
     writer for it in the hash table. T_NEVER => won't ever be
     scheduled, and this can only change by writing data, which won't
     happen until after it becomes visible. */
  if (wr->reliable)
  {
    nn_mtime_t tsched;
    tsched.v = T_NEVER;
    wr->heartbeat_xevent = qxev_heartbeat (wr->evq, tsched, &wr->e.guid);
  }
  else
  {
    wr->heartbeat_xevent = NULL;
  }
  assert (wr->xqos->present & QP_LIVELINESS);
  if (wr->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS ||
      nn_from_ddsi_duration (wr->xqos->liveliness.lease_duration) != T_NEVER)
  {
    nn_log (LC_INFO, "writer %x:%x:%x:%x: incorrectly treating it as of automatic liveliness kind with lease duration = inf (%d, %"PA_PRId64")\n", PGUID (wr->e.guid), (int) wr->xqos->liveliness.kind, nn_from_ddsi_duration (wr->xqos->liveliness.lease_duration));
  }
  wr->lease_duration = T_NEVER; /* FIXME */

  /* Construct WHC -- if aggressive_keep_last1 is set, the WHC will
     drop all samples for which a later update is available.  This
     forces it to maintain a tlidx.  */
  {
    unsigned hdepth, tldepth;
    if (!wr->aggressive_keep_last || wr->xqos->history.kind == NN_KEEP_ALL_HISTORY_QOS)
      hdepth = 0;
    else
      hdepth = (unsigned)wr->xqos->history.depth;
    if (wr->handle_as_transient_local) {
      if (wr->xqos->durability_service.history.kind == NN_KEEP_ALL_HISTORY_QOS)
        tldepth = 0;
      else
        tldepth = (unsigned)wr->xqos->durability_service.history.depth;
    } else if (wr->startup_mode) {
      tldepth = hdepth;
    } else {
      tldepth = 0;
    }
    wr->whc = whc_new (wr->handle_as_transient_local, hdepth, tldepth, sample_overhead);
    if (hdepth > 0)
    {
      /* hdepth > 0 => "aggressive keep last", and in that case: why
         bother blocking for a slow receiver when the entire point of
         KEEP_LAST is to keep going (at least in a typical interpretation
         of the spec */
      wr->whc_low = wr->whc_high = 2147483647u;
    }
    else
    {
      wr->whc_low = config.whc_lowwater_mark;
      wr->whc_high = config.whc_init_highwater_mark.value;
    }
  }

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
  {
    nn_mtime_t tsched = { 0 };
    resched_xevent_if_earlier (pp->pmd_update_xevent, tsched);
  }
  return wr;
}

struct writer * new_writer
(
  struct nn_guid *wrguid,
  const struct nn_guid *group_guid,
  const struct nn_guid *ppguid,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
)
{
  struct participant *pp;
  struct writer * wr;
  unsigned entity_kind;

  if ((pp = ephash_lookup_participant_guid (ppguid)) == NULL)
  {
    TRACE (("new_writer - participant %x:%x:%x:%x not found\n", PGUID (*ppguid)));
    return NULL;
  }
  /* participant can't be freed while we're mucking around cos we are
     awake and do not touch the thread's vtime (ephash_lookup already
     verifies we're awake) */
  entity_kind = (topic->nkeys ? NN_ENTITYID_KIND_WRITER_WITH_KEY : NN_ENTITYID_KIND_WRITER_NO_KEY);
  wrguid->prefix = pp->e.guid.prefix;
  if (pp_allocate_entityid (&wrguid->entityid, entity_kind, pp) < 0)
    return NULL;
  wr = new_writer_guid (wrguid, group_guid, pp, topic, xqos, gid, group_gid, endpoint_name);
  return wr;
}

static void gc_delete_writer (struct gcreq *gcreq)
{
  struct writer *wr = gcreq->arg;
  TRACE (("gc_delete_writer(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (wr->e.guid)));
  gcreq_free (gcreq);

  if (wr->heartbeat_xevent)
  {
    wr->hbcontrol.tsched.v = T_NEVER;
    delete_xevent (wr->heartbeat_xevent);
  }

  /* Tear down connections -- no proxy reader can be adding/removing
      us now, because we can't be found via guid_hash anymore.  We
      therefore need not take lock. */
  while (!ut_avlIsEmpty (&wr->readers))
  {
    struct wr_prd_match *m = ut_avlRoot (&wr_readers_treedef, &wr->readers);
    ut_avlDelete (&wr_readers_treedef, &wr->readers, m);
    proxy_reader_drop_connection (&m->prd_guid, wr);
    free_wr_prd_match (m);
  }

  /* Do last gasp on SEDP and free writer. */
  if (!is_builtin_entityid (wr->e.guid.entityid, ownvendorid))
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
  if (whc_unacked_bytes (wr->whc) == 0)
  {
    TRACE (("delete_writer(guid %x:%x:%x:%x) - no unack'ed samples\n", PGUID (*guid)));
    delete_writer_nolinger_locked (wr);
    os_mutexUnlock (&wr->e.lock);
  }
  else
  {
    nn_mtime_t tsched;
    int tusec;
    os_int64 tsec;
    writer_set_state (wr, WRST_LINGERING);
    os_mutexUnlock (&wr->e.lock);
    tsched = add_duration_to_mtime (now_mt (), config.writer_linger_duration);
    mtime_to_sec_usec (&tsec, &tusec, tsched);
    TRACE (("delete_writer(guid %x:%x:%x:%x) - unack'ed samples, will delete when ack'd or at t = %"PA_PRId64".%06d\n",
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
    unsigned n;
    assert (wr->e.guid.entityid.u != NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
    wr->startup_mode = 0;
    whc_downgrade_to_volatile (wr->whc);
    n = remove_acked_messages (wr);
    writer_clear_retransmitting (wr);
    TRACE (("  wr %x:%x:%x:%x dropped %u entr%s\n", PGUID (wr->e.guid), n, n == 1 ? "y" : "ies"));
  }
  os_mutexUnlock (&wr->e.lock);
}

/* READER ----------------------------------------------------------- */


static struct reader * new_reader_guid
(
  const struct nn_guid *guid,
  const struct nn_guid *group_guid,
  struct participant *pp,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
)
{
  /* see new_writer_guid for commenets */

  struct reader * rd;
  nn_mtime_t tnow = now_mt ();

  assert (!is_writer_entityid (guid->entityid));
  assert (ephash_lookup_reader_guid (guid) == NULL);
  assert (memcmp (&guid->prefix, &pp->e.guid.prefix, sizeof (guid->prefix)) == 0);

  new_reader_writer_common (guid, topic, xqos);
  rd = os_malloc (sizeof (*rd));

  endpoint_common_init (&rd->e, &rd->c, EK_READER, guid, group_guid, pp, gid, group_gid, endpoint_name);

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
  rd->matching_groups = nn_groupset_new ();
  if (!is_builtin_entityid (rd->e.guid.entityid, ownvendorid))
    nn_groupset_fromqos (rd->matching_groups, gv.ospl_kernel, rd->xqos);
  assert (rd->xqos->present & QP_LIVELINESS);
  if (rd->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS ||
      nn_from_ddsi_duration (rd->xqos->liveliness.lease_duration) != T_NEVER)
  {
    nn_log (LC_INFO, "reader %x:%x:%x:%x: incorrectly treating it as of automatic liveliness kind with lease duration = inf (%d, %"PA_PRId64")\n", PGUID (rd->e.guid), (int) rd->xqos->liveliness.kind, nn_from_ddsi_duration (rd->xqos->liveliness.lease_duration));
  }


  ut_avlInit (&rd_writers_treedef, &rd->writers);

  ephash_insert_reader_guid (rd);
  match_reader_with_proxy_writers (rd, tnow);
  sedp_write_reader (rd);
  /* If no writers matched, must notify a wait_for_historical_data in OSPL. In Lite,
     I would argue that wait_for_historical_data should look at the state in DDSI ...
     but that coupling is problematic in OSPL. */
  if (rd->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS)
    notify_wait_for_historical_data (NULL, &rd->e.guid);
  return rd;
}

struct reader * new_reader
(
  struct nn_guid *rdguid,
  const struct nn_guid *group_guid,
  const struct nn_guid *ppguid,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
)
{
  struct participant * pp;
  struct reader * rd;
  unsigned entity_kind;

  if ((pp = ephash_lookup_participant_guid (ppguid)) == NULL)
  {
    TRACE (("new_reader - participant %x:%x:%x:%x not found\n", PGUID (*ppguid)));
    return NULL;
  }
  entity_kind = (topic->nkeys ? NN_ENTITYID_KIND_READER_WITH_KEY : NN_ENTITYID_KIND_READER_NO_KEY);
  rdguid->prefix = pp->e.guid.prefix;
  if (pp_allocate_entityid (&rdguid->entityid, entity_kind, pp) < 0)
    return NULL;
  rd = new_reader_guid (rdguid, group_guid, pp, topic, xqos, gid, group_gid, endpoint_name);
  return rd;
}

static void gc_delete_reader (struct gcreq *gcreq)
{
  /* see gc_delete_writer for comments */
  struct reader *rd = gcreq->arg;
  TRACE (("gc_delete_reader(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (rd->e.guid)));
  gcreq_free (gcreq);

  while (!ut_avlIsEmpty (&rd->writers))
  {
    struct rd_pwr_match *m = ut_avlRoot (&rd_writers_treedef, &rd->writers);
    ut_avlDelete (&rd_writers_treedef, &rd->writers, m);
    proxy_writer_drop_connection (&m->pwr_guid, rd);
    free_rd_pwr_match (m);
  }

  if (!is_builtin_entityid (rd->e.guid.entityid, ownvendorid))
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
  gcreq_reader (rd);
  return 0;
}

/* PROXY-PARTICIPANT ------------------------------------------------ */
static void gc_proxy_participant_lease (struct gcreq *gcreq)
{
  lease_free (gcreq->arg);
  gcreq_free (gcreq);
}

void proxy_participant_reassign_lease (struct proxy_participant *proxypp, struct lease *newlease)
{
  /* Lease renewal is done by the receive thread without locking the
     proxy participant (and I'd like to keep it that way), but that
     means we must guarantee that the lease pointer remains valid once
     loaded.

     By loading/storing the pointer atomically, we ensure we always
     read a valid (or once valid) value, by delaying the freeing
     through the garbage collector, we ensure whatever lease update
     occurs in parallel completes before the memory is released.

     The lease_renew(never) call ensures the lease will never expire
     while we are messing with it. */
  os_mutexLock (&proxypp->e.lock);
  if (proxypp->owns_lease)
  {
    const nn_etime_t never = { T_NEVER };
    struct gcreq *gcreq = gcreq_new (gv.gcreq_queue, gc_proxy_participant_lease);
    struct lease *oldlease = pa_ldvoidp (&proxypp->lease);
    lease_renew (oldlease, never);
    gcreq->arg = oldlease;
    gcreq_enqueue (gcreq);
    proxypp->owns_lease = 0;
  }
  pa_stvoidp (&proxypp->lease, newlease);
  os_mutexUnlock (&proxypp->e.lock);
}

void new_proxy_participant
(
  const struct nn_guid *ppguid,
  unsigned bes,
  unsigned prismtech_bes,
  const struct nn_guid *privileged_pp_guid,
  struct addrset *as_default,
  struct addrset *as_meta,
  const nn_plist_t *plist,
  os_int64 tlease_dur,
  nn_vendorid_t vendor,
  unsigned custom_flags
)
{
  /* No locking => iff all participants use unique guids, and sedp
     runs on a single thread, it can't go wrong. FIXME, maybe? The
     same holds for the other functions for creating entities. */
  struct proxy_participant *proxypp;

  assert (ppguid->entityid.u == NN_ENTITYID_PARTICIPANT);
  assert (ephash_lookup_proxy_participant_guid (ppguid) == NULL);
  assert (privileged_pp_guid == NULL || privileged_pp_guid->entityid.u == NN_ENTITYID_PARTICIPANT);

  prune_deleted_participant_guids (now_mt ());

  proxypp = os_malloc (sizeof (*proxypp));

  entity_common_init (&proxypp->e, ppguid, "", EK_PROXY_PARTICIPANT);
  proxypp->refc = 1;
  proxypp->lease_expired = 0;
  proxypp->vendor = vendor;
  proxypp->bes = bes;
  proxypp->prismtech_bes = prismtech_bes;
  if (privileged_pp_guid) {
    proxypp->privileged_pp_guid = *privileged_pp_guid;
  } else {
    memset (&proxypp->privileged_pp_guid.prefix, 0, sizeof (proxypp->privileged_pp_guid.prefix));
    proxypp->privileged_pp_guid.entityid.u = NN_ENTITYID_PARTICIPANT;
  }
  if ((plist->present & PP_PRISMTECH_PARTICIPANT_VERSION_INFO) &&
      (plist->prismtech_participant_version_info.flags & NN_PRISMTECH_FL_DDSI2_PARTICIPANT_FLAG) &&
      (plist->prismtech_participant_version_info.flags & NN_PRISMTECH_FL_PARTICIPANT_IS_DDSI2))
    proxypp->is_ddsi2_pp = 1;
  else
    proxypp->is_ddsi2_pp = 0;
  if ((plist->present & PP_PRISMTECH_PARTICIPANT_VERSION_INFO) &&
      (plist->prismtech_participant_version_info.flags & NN_PRISMTECH_FL_MINIMAL_BES_MODE))
    proxypp->minimal_bes_mode = 1;
  else
    proxypp->minimal_bes_mode = 0;

  {
    struct proxy_participant *privpp;
    privpp = ephash_lookup_proxy_participant_guid (&proxypp->privileged_pp_guid);
    if (privpp != NULL && privpp->is_ddsi2_pp)
    {
      pa_stvoidp (&proxypp->lease, pa_ldvoidp (&privpp->lease));
      proxypp->owns_lease = 0;
    }
    else
    {
      pa_stvoidp (&proxypp->lease, lease_new (tlease_dur, &proxypp->e));
      proxypp->owns_lease = 1;
    }
  }

  proxypp->as_default = as_default;
  proxypp->as_meta = as_meta;
  proxypp->endpoints = NULL;
  proxypp->plist = nn_plist_dup (plist);
  ut_avlInit (&proxypp_groups_treedef, &proxypp->groups);

  /* - the proxypp gid only matters when generate_builtin_topics is set
     - with generate_builtin_topics set:
       - proxy gids are faked unless ENDPOINT_GID present, from 6.4.1
       - DISCOVERY_INCLUDES_GID is present from 6.4.1p1
     - 6.5.0p11 fixed the fake GID generation, but missed the proxypp GID
       (the foreign keys in the built-in topics are correct, though)
     so a test for DISCOVERY_INCLUDES_GID means we do the right thing for
     all other implementations and for all OpenSplice versions other than
     6.4.1 ... but the 6.4.1 already has the version info internals. */
  if ((plist->present & PP_PRISMTECH_PARTICIPANT_VERSION_INFO) &&
      ((plist->prismtech_participant_version_info.flags & NN_PRISMTECH_FL_DISCOVERY_INCLUDES_GID) ||
       version_info_is_6_4_1 (plist->prismtech_participant_version_info.internals)))
  {
    proxypp->gid.systemId = ppguid->prefix.u[0];
    proxypp->gid.localId = ppguid->prefix.u[1];
    proxypp->gid.serial = ppguid->prefix.u[2];
  }
  else
  {
    nn_guid_to_ospl_gid (&proxypp->gid, ppguid, vendor_is_opensplice (vendor));
  }

  if (custom_flags & CF_INC_KERNEL_SEQUENCE_NUMBERS)
    proxypp->kernel_sequence_numbers = 1;
  else
    proxypp->kernel_sequence_numbers = 0;
  if (custom_flags & CF_IMPLICITLY_CREATED_PROXYPP)
    proxypp->implicitly_created = 1;
  else
    proxypp->implicitly_created = 0;

  if (custom_flags & CF_PROXYPP_NO_SPDP)
    proxypp->proxypp_have_spdp = 0;
  else
    proxypp->proxypp_have_spdp = 1;
  /* Non-PrismTech doesn't implement the PT extensions and therefore won't generate
     a CMParticipant; if a PT peer does not implement a CMParticipant writer, then it
     presumably also is a handicapped implementation (perhaps simply an old one) */
  if (!vendor_is_prismtech(proxypp->vendor) ||
      (proxypp->bes != 0 && !(proxypp->prismtech_bes & NN_DISC_BUILTIN_ENDPOINT_CM_PARTICIPANT_WRITER)))
    proxypp->proxypp_have_cm = 1;
  else
    proxypp->proxypp_have_cm = 0;

  /* Proxy participant must be in the hash tables for
     new_proxy_{writer,reader} to work */
  ephash_insert_proxy_participant_guid (proxypp);

  /* Add proxy endpoints based on the advertised (& possibly augmented
     ...) built-in endpoint set. */
  {
#define PT_TE(ap_, a_, bp_, b_) { 0, NN_##ap_##BUILTIN_ENDPOINT_##a_, NN_ENTITYID_##bp_##_BUILTIN_##b_ }
#define TE(ap_, a_, bp_, b_) { NN_##ap_##BUILTIN_ENDPOINT_##a_, 0, NN_ENTITYID_##bp_##_BUILTIN_##b_ }
#define LTE(a_, bp_, b_) { NN_##BUILTIN_ENDPOINT_##a_, 0, NN_ENTITYID_##bp_##_BUILTIN_##b_ }
    static const struct bestab {
      unsigned besflag;
      unsigned prismtech_besflag;
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
      LTE (PARTICIPANT_MESSAGE_DATA_READER, P2P, PARTICIPANT_MESSAGE_READER),
      TE (DISC_, TOPIC_ANNOUNCER, SEDP, TOPIC_WRITER),
      TE (DISC_, TOPIC_DETECTOR, SEDP, TOPIC_READER),
      PT_TE (DISC_, CM_PARTICIPANT_READER, SEDP, CM_PARTICIPANT_READER),
      PT_TE (DISC_, CM_PARTICIPANT_WRITER, SEDP, CM_PARTICIPANT_WRITER),
      PT_TE (DISC_, CM_PUBLISHER_READER, SEDP, CM_PUBLISHER_READER),
      PT_TE (DISC_, CM_PUBLISHER_WRITER, SEDP, CM_PUBLISHER_WRITER),
      PT_TE (DISC_, CM_SUBSCRIBER_READER, SEDP, CM_SUBSCRIBER_READER),
      PT_TE (DISC_, CM_SUBSCRIBER_WRITER, SEDP, CM_SUBSCRIBER_WRITER)
    };
#undef PT_TE
#undef TE
#undef LTE
    nn_plist_t plist_rd, plist_wr;
    int i;
    /* Note: no entity name or group GUID supplied, but that shouldn't
       matter, as these are internal to DDSI and don't use group
       coherency */
    nn_plist_init_empty (&plist_wr);
    nn_plist_init_empty (&plist_rd);
    nn_xqos_copy (&plist_wr.qos, &gv.builtin_endpoint_xqos_wr);
    nn_xqos_copy (&plist_rd.qos, &gv.builtin_endpoint_xqos_rd);
    for (i = 0; i < (int) (sizeof (bestab) / sizeof (*bestab)); i++)
    {
      const struct bestab *te = &bestab[i];
      if ((proxypp->bes & te->besflag) || (proxypp->prismtech_bes & te->prismtech_besflag))
      {
        nn_guid_t guid1;
        guid1.prefix = proxypp->e.guid.prefix;
        guid1.entityid.u = te->entityid;
        assert (is_builtin_entityid (guid1.entityid, proxypp->vendor));
        if (is_writer_entityid (guid1.entityid))
        {
          new_proxy_writer (ppguid, &guid1, proxypp->as_meta, &plist_wr, gv.builtins_dqueue, gv.xevents);
        }
        else
        {
          new_proxy_reader (ppguid, &guid1, proxypp->as_meta, &plist_rd);
        }
      }
    }
    nn_plist_fini (&plist_wr);
    nn_plist_fini (&plist_rd);
  }

  /* Register lease, but be careful not to accidentally re-register
     DDSI2's lease, as we may have become dependent on DDSI2 any time
     after ephash_insert_proxy_participant_guid even if
     privileged_pp_guid was NULL originally */
  os_mutexLock (&proxypp->e.lock);
  if (proxypp->owns_lease)
    lease_register (pa_ldvoidp (&proxypp->lease));
  os_mutexUnlock (&proxypp->e.lock);

  if (config.generate_builtin_topics)
  {
    os_mutexLock ((os_mutex *) &proxypp->e.lock);
    if (proxypp->proxypp_have_spdp)
    {
      write_builtin_topic_proxy_participant (proxypp);
      if (proxypp->proxypp_have_cm)
        write_builtin_topic_proxy_participant_cm (proxypp);
    }
    os_mutexUnlock ((os_mutex *) &proxypp->e.lock);
  }
}

int update_proxy_participant_plist_locked (struct proxy_participant *proxypp, const struct nn_plist *datap, enum update_proxy_participant_source source)
{
  /* Currently, built-in processing is single-threaded, and it is only through this function and the proxy participant deletion (which necessarily happens when no-one else potentially references the proxy participant anymore).  So at the moment, the lock is superfluous. */
  nn_plist_t *new_plist;

  new_plist = nn_plist_dup (datap);
  nn_plist_mergein_missing (new_plist, proxypp->plist);
  nn_plist_fini (proxypp->plist);
  os_free (proxypp->plist);
  proxypp->plist = new_plist;

  if (config.generate_builtin_topics)
  {
    switch (source)
    {
      case UPD_PROXYPP_SPDP:
        write_builtin_topic_proxy_participant (proxypp);
        if (!proxypp->proxypp_have_spdp && proxypp->proxypp_have_cm)
          write_builtin_topic_proxy_participant_cm (proxypp);
        proxypp->proxypp_have_spdp = 1;
        break;
      case UPD_PROXYPP_CM:
        if (proxypp->proxypp_have_spdp)
          write_builtin_topic_proxy_participant_cm (proxypp);
        proxypp->proxypp_have_cm = 1;
        break;
    }
  }
  return 0;
}

int update_proxy_participant_plist (struct proxy_participant *proxypp, const struct nn_plist *datap, enum update_proxy_participant_source source)
{
  nn_plist_t tmp;

  /* FIXME: find a better way of restricting which bits can get updated */
  os_mutexLock (&proxypp->e.lock);
  switch (source)
  {
    case UPD_PROXYPP_SPDP:
      update_proxy_participant_plist_locked (proxypp, datap, source);
      break;
    case UPD_PROXYPP_CM:
      tmp = *datap;
      tmp.present &=
        PP_PRISMTECH_NODE_NAME | PP_PRISMTECH_EXEC_NAME | PP_PRISMTECH_PROCESS_ID |
        PP_PRISMTECH_WATCHDOG_SCHEDULING | PP_PRISMTECH_LISTENER_SCHEDULING |
        PP_PRISMTECH_SERVICE_TYPE | PP_ENTITY_NAME;
      tmp.qos.present &= QP_PRISMTECH_ENTITY_FACTORY;
      update_proxy_participant_plist_locked (proxypp, &tmp, source);
      break;
  }
  os_mutexUnlock (&proxypp->e.lock);
  return 0;
}

static void ref_proxy_participant (struct proxy_participant *proxypp, struct proxy_endpoint_common *c)
{
  os_mutexLock (&proxypp->e.lock);
  c->proxypp = proxypp;
  proxypp->refc++;

  c->next_ep = proxypp->endpoints;
  c->prev_ep = NULL;
  if (c->next_ep)
  {
    c->next_ep->prev_ep = c;
  }
  proxypp->endpoints = c;
  os_mutexUnlock (&proxypp->e.lock);
}

static void unref_proxy_participant (struct proxy_participant *proxypp, struct proxy_endpoint_common *c)
{
  os_uint32 refc;

  os_mutexLock (&proxypp->e.lock);
  refc = --proxypp->refc;

  if (c != NULL)
  {
    if (c->next_ep)
      c->next_ep->prev_ep = c->prev_ep;
    if (c->prev_ep)
      c->prev_ep->next_ep = c->next_ep;
    else
      proxypp->endpoints = c->next_ep;
  }

  if (refc == 0)
  {
    assert (proxypp->endpoints == NULL);
    os_mutexUnlock (&proxypp->e.lock);
    TRACE (("unref_proxy_participant(%x:%x:%x:%x): refc=0, freeing\n", PGUID (proxypp->e.guid)));

    if (config.generate_builtin_topics)
    {
      if (proxypp->proxypp_have_spdp)
        dispose_builtin_topic_proxy_participant (proxypp, proxypp->lease_expired);
    }

    unref_addrset (proxypp->as_default);
    unref_addrset (proxypp->as_meta);
    nn_plist_fini (proxypp->plist);
    os_free (proxypp->plist);
    if (proxypp->owns_lease)
      lease_free (pa_ldvoidp (&proxypp->lease));
    entity_common_fini (&proxypp->e);
    remove_deleted_participant_guid (&proxypp->e.guid, DPG_LOCAL | DPG_REMOTE);
    os_free (proxypp);
  }
  else if (proxypp->endpoints == NULL && proxypp->implicitly_created)
  {
    assert (refc == 1);
    os_mutexUnlock (&proxypp->e.lock);
    TRACE (("unref_proxy_participant(%x:%x:%x:%x): refc=%u, no endpoints, implicitly created, deleting\n", PGUID (proxypp->e.guid), (unsigned) refc));
    delete_proxy_participant_by_guid(&proxypp->e.guid, 1);
    /* Deletion is still (and has to be) asynchronous. A parallel endpoint creation may or may not
       succeed, and if it succeeds it will be deleted along with the proxy participant. So "your
       mileage may vary". Also, the proxy participant may be blacklisted for a little ... */
  }
  else
  {
    os_mutexUnlock (&proxypp->e.lock);
    TRACE (("unref_proxy_participant(%x:%x:%x:%x): refc=%u\n", PGUID (proxypp->e.guid), (unsigned) refc));
  }
}

static void gc_delete_proxy_participant (struct gcreq *gcreq)
{
  struct proxy_participant *proxypp = gcreq->arg;
  TRACE (("gc_delete_proxy_participant(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (proxypp->e.guid)));
  gcreq_free (gcreq);
  unref_proxy_participant (proxypp, NULL);
}

static struct entity_common *entity_common_from_proxy_endpoint_common (const struct proxy_endpoint_common *c)
{
  assert (offsetof (struct proxy_writer, e) == 0);
  assert (offsetof (struct proxy_reader, e) == offsetof (struct proxy_writer, e));
  assert (offsetof (struct proxy_reader, c) == offsetof (struct proxy_writer, c));
  assert (c != NULL);
  return (struct entity_common *) ((char *) c - offsetof (struct proxy_writer, c));
}

static void delete_ppt (struct proxy_participant * proxypp, int isimplicit)
{
  struct proxy_endpoint_common * c;
  int ret;

  /* if any proxy participants depend on this participant, delete them */
  TRACE (("delete_ppt(%x:%x:%x:%x) - deleting dependent proxy participants\n", PGUID (proxypp->e.guid)));
  {
    struct ephash_enum_proxy_participant est;
    struct proxy_participant *p;
    ephash_enum_proxy_participant_init (&est);
    while ((p = ephash_enum_proxy_participant_next (&est)) != NULL)
    {
      if (memcmp (&p->privileged_pp_guid, &proxypp->e.guid, sizeof (proxypp->e.guid)) == 0)
        (void) delete_proxy_participant_by_guid (&p->e.guid, isimplicit);
    }
    ephash_enum_proxy_participant_fini (&est);
  }

  /* delete_proxy_{reader,writer} merely schedules the actual delete
     operation, so we can hold the lock -- at least, for now. */

  os_mutexLock (&proxypp->e.lock);
  if (isimplicit)
    proxypp->lease_expired = 1;

  TRACE (("delete_ppt(%x:%x:%x:%x) - deleting groups\n", PGUID (proxypp->e.guid)));
  while (!ut_avlIsEmpty (&proxypp->groups))
    delete_proxy_group_locked (ut_avlRoot (&proxypp_groups_treedef, &proxypp->groups), isimplicit);

  TRACE (("delete_ppt(%x:%x:%x:%x) - deleting endpoints\n", PGUID (proxypp->e.guid)));
  c = proxypp->endpoints;
  while (c)
  {
    struct entity_common *e = entity_common_from_proxy_endpoint_common (c);
    if (is_writer_entityid (e->guid.entityid))
    {
      ret = delete_proxy_writer (&e->guid, isimplicit);
    }
    else
    {
      ret = delete_proxy_reader (&e->guid, isimplicit);
    }
    (void) ret;
    c = c->next_ep;
  }
  os_mutexUnlock (&proxypp->e.lock);

  gcreq_proxy_participant (proxypp);
}

typedef struct proxy_purge_data {
  struct proxy_participant *proxypp;
  const nn_locator_t *loc;
} *proxy_purge_data_t;

static void purge_helper (const nn_locator_t *n, void * varg)
{
  proxy_purge_data_t data = (proxy_purge_data_t) varg;
  if (compare_locators (n, data->loc) == 0)
    delete_proxy_participant_by_guid (&data->proxypp->e.guid, 1);
}

void purge_proxy_participants (const nn_locator_t *loc, c_bool delete_from_as_disc)
{
  /* FIXME: check whether addr:port can't be reused for a new connection by the time we get here. */
  /* NOTE: This function exists for the sole purpose of cleaning up after closing a TCP connection in ddsi_tcp_close_conn and the state of the calling thread could be anything at this point. Because of that we do the unspeakable and toggle the thread state conditionally. We can't afford to have it in "asleep", as that causes a race with the garbage collector. */
  struct thread_state1 * const self = lookup_thread_state();
  const int self_is_awake = vtime_awake_p (self->vtime);
  struct ephash_enum_proxy_participant est;
  struct proxy_purge_data data;

  if (!self_is_awake)
    thread_state_awake(self);

  data.loc = loc;
  ephash_enum_proxy_participant_init (&est);
  while ((data.proxypp = ephash_enum_proxy_participant_next (&est)) != NULL)
    addrset_forall (data.proxypp->as_meta, purge_helper, &data);
  ephash_enum_proxy_participant_fini (&est);

  /* Shouldn't try to keep pinging clients once they're gone */
  if (delete_from_as_disc)
    remove_from_addrset (gv.as_disc, loc);

  if (!self_is_awake)
    thread_state_asleep(self);
}

int delete_proxy_participant_by_guid (const struct nn_guid * guid, int isimplicit)
{
  struct proxy_participant * ppt;

  TRACE (("delete_proxy_participant_by_guid(%x:%x:%x:%x) ", PGUID (*guid)));
  os_mutexLock (&gv.lock);
  ppt = ephash_lookup_proxy_participant_guid (guid);
  if (ppt == NULL)
  {
    os_mutexUnlock (&gv.lock);
    TRACE (("- unknown\n"));
    return ERR_UNKNOWN_ENTITY;
  }
  TRACE (("- deleting\n"));
  remember_deleted_participant_guid (&ppt->e.guid);
  ephash_remove_proxy_participant_guid (ppt);
  os_mutexUnlock (&gv.lock);
  delete_ppt (ppt, isimplicit);

  return 0;
}

/* PROXY-GROUP --------------------------------------------------- */

int new_proxy_group (const struct nn_guid *guid, const struct v_gid_s *gid, const char *name, const struct nn_xqos *xqos)
{
  struct proxy_participant *proxypp;
  nn_guid_t ppguid;
  ppguid.prefix = guid->prefix;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((proxypp = ephash_lookup_proxy_participant_guid (&ppguid)) == NULL)
  {
    TRACE (("new_proxy_group(%x:%x:%x:%x) - unknown participant\n", PGUID (*guid)));
    return 0;
  }
  else
  {
    struct proxy_group *pgroup;
    ut_avlIPath_t ipath;
    int is_sub;
    switch (guid->entityid.u & (NN_ENTITYID_SOURCE_MASK | NN_ENTITYID_KIND_MASK))
    {
      case NN_ENTITYID_SOURCE_VENDOR | NN_ENTITYID_KIND_PRISMTECH_PUBLISHER:
        is_sub = 0;
        break;
      case NN_ENTITYID_SOURCE_VENDOR | NN_ENTITYID_KIND_PRISMTECH_SUBSCRIBER:
        is_sub = 1;
        break;
      default:
        NN_WARNING1 ("new_proxy_group: unrecognised entityid: %x\n", guid->entityid.u);
        return ERR_INVALID_DATA;
    }
    os_mutexLock (&proxypp->e.lock);
    if ((pgroup = ut_avlLookupIPath (&proxypp_groups_treedef, &proxypp->groups, guid, &ipath)) != NULL)
    {
      /* Complete proxy group definition if it was a partial
         definition made by creating a proxy reader or writer,
         otherwise ignore this call */
      if (pgroup->name != NULL)
        goto out;
    }
    else
    {
      /* Always have a guid, may not have a gid */
      TRACE (("new_proxy_group(%x:%x:%x:%x): new\n", PGUID (*guid)));
      pgroup = os_malloc (sizeof (*pgroup));
      pgroup->guid = *guid;
      if (gid)
      {
        pgroup->gid = *gid;
      }
      else
      {
        nn_guid_to_ospl_gid (&pgroup->gid, guid, vendor_is_opensplice (proxypp->vendor));
      }
      pgroup->proxypp = proxypp;
      pgroup->name = NULL;
      pgroup->xqos = NULL;
      ut_avlInsertIPath (&proxypp_groups_treedef, &proxypp->groups, pgroup, &ipath);
    }
    if (name)
    {
      assert (xqos != NULL);
      TRACE (("new_proxy_group(%x:%x:%x:%x): setting name (%s) and qos\n", PGUID (*guid), name));
      pgroup->name = os_strdup (name);
      pgroup->xqos = nn_xqos_dup (xqos);
      nn_xqos_mergein_missing (pgroup->xqos, is_sub ? &gv.default_xqos_sub : &gv.default_xqos_pub);
      if (config.generate_builtin_topics)
        write_builtin_topic_proxy_group (pgroup);
    }
  out:
    os_mutexUnlock (&proxypp->e.lock);
    TRACE (("\n"));
    return 0;
  }
}

static void delete_proxy_group_locked (struct proxy_group *pgroup, int isimplicit)
{
  struct proxy_participant *proxypp = pgroup->proxypp;
  assert ((pgroup->xqos != NULL) == (pgroup->name != NULL));
  TRACE (("delete_proxy_group_locked %x:%x:%x:%x\n", PGUID (pgroup->guid)));
  ut_avlDelete (&proxypp_groups_treedef, &proxypp->groups, pgroup);
  /* Publish corresponding built-in topic only if it is not a place
     holder: in that case we haven't announced its presence and
     therefore don't need to dispose it, and this saves us from having
     to handle null pointers for name and QoS in the built-in topic
     generation */
  if (pgroup->name)
  {
    if (config.generate_builtin_topics)
      dispose_builtin_topic_proxy_group (pgroup, isimplicit);
    nn_xqos_fini (pgroup->xqos);
    os_free (pgroup->xqos);
    os_free (pgroup->name);
  }
  os_free (pgroup);
}

void delete_proxy_group (const nn_guid_t *guid, int isimplicit)
{
  struct proxy_participant *proxypp;
  nn_guid_t ppguid;
  ppguid.prefix = guid->prefix;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((proxypp = ephash_lookup_proxy_participant_guid (&ppguid)) != NULL)
  {
    struct proxy_group *pgroup;
    os_mutexLock (&proxypp->e.lock);
    if ((pgroup = ut_avlLookup (&proxypp_groups_treedef, &proxypp->groups, guid)) != NULL)
      delete_proxy_group_locked (pgroup, isimplicit);
    os_mutexUnlock (&proxypp->e.lock);
  }
}

/* PROXY-ENDPOINT --------------------------------------------------- */

static void proxy_endpoint_common_init
(
  struct entity_common *e, struct proxy_endpoint_common *c,
  enum entity_kind kind, const struct nn_guid *guid, struct proxy_participant *proxypp,
  struct addrset *as, const nn_plist_t *plist
)
{
  const char *name;

  if (is_builtin_entityid (guid->entityid, proxypp->vendor))
    assert ((plist->qos.present & (QP_TOPIC_NAME | QP_TYPE_NAME)) == 0);
  else
    assert ((plist->qos.present & (QP_TOPIC_NAME | QP_TYPE_NAME)) == (QP_TOPIC_NAME | QP_TYPE_NAME));

  name = (plist->present & PP_ENTITY_NAME) ? plist->entity_name : "";
  entity_common_init (e, guid, name, kind);
  c->xqos = nn_xqos_dup (&plist->qos);
  c->as = ref_addrset (as);
  c->topic = NULL; /* set from first matching reader/writer */
  c->vendor = proxypp->vendor;

  if (plist->present & PP_GROUP_GUID)
    c->group_guid = plist->group_guid;
  else
    memset (&c->group_guid, 0, sizeof (c->group_guid));
  if (plist->present & PP_PRISMTECH_GROUP_GID)
    c->group_gid = plist->group_gid;
  else if (plist->present & PP_GROUP_GUID)
    nn_guid_to_ospl_gid (&c->group_gid, &c->group_guid, vendor_is_opensplice (proxypp->vendor));
  else
    memset (&c->group_gid, 0, sizeof (c->group_gid));

  if (is_builtin_endpoint (guid->entityid, proxypp->vendor))
    memset (&c->gid, 0, sizeof (c->gid));
  else if (plist->present & PP_PRISMTECH_ENDPOINT_GID)
    c->gid = plist->endpoint_gid;
  else if (!config.generate_builtin_topics)
    memset (&c->gid, 0, sizeof (c->gid)); /* done lazily */
  else
    nn_guid_to_ospl_gid (&c->gid, guid, vendor_is_opensplice (proxypp->vendor));

  ref_proxy_participant (proxypp, c);
}

static void proxy_endpoint_common_fini (struct entity_common *e, struct proxy_endpoint_common *c)
{
  unref_proxy_participant (c->proxypp, c);

  nn_xqos_fini (c->xqos);
  os_free (c->xqos);
  unref_addrset (c->as);

  entity_common_fini (e);
}

/* PROXY-WRITER ----------------------------------------------------- */

int new_proxy_writer (const struct nn_guid *ppguid, const struct nn_guid *guid, struct addrset *as, const nn_plist_t *plist, struct nn_dqueue *dqueue, struct xeventq *evq)
{
  struct proxy_participant *proxypp;
  struct proxy_writer *pwr;
  int isreliable;
  nn_mtime_t tnow = now_mt ();

  assert (is_writer_entityid (guid->entityid));
  assert (ephash_lookup_proxy_writer_guid (guid) == NULL);

  if ((proxypp = ephash_lookup_proxy_participant_guid (ppguid)) == NULL)
  {
    NN_WARNING1 ("new_proxy_writer(%x:%x:%x:%x): proxy participant unknown\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }

  pwr = os_malloc (sizeof (*pwr));
  proxy_endpoint_common_init (&pwr->e, &pwr->c, EK_PROXY_WRITER, guid, proxypp, as, plist);

  ut_avlInit (&pwr_readers_treedef, &pwr->readers);
  pwr->groups = nn_groupset_new ();
  pwr->v_message_qos = new_v_message_qos (pwr->c.xqos);
  pwr->seq_offset = 0;
  pwr->transaction_id = 0;
  pwr->cs_seq = 0;
  pwr->n_reliable_readers = 0;
  pwr->n_readers_out_of_sync = 0;
  pwr->last_seq = 0;
  pwr->last_fragnum = ~0u;
  pwr->nackfragcount = 0;
  pwr->last_fragnum_reset = 0;
  pa_st32 (&pwr->next_deliv_seq_lowword, 1);
  if (is_builtin_entityid (pwr->e.guid.entityid, pwr->c.vendor)) {
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

  /* Only assert PP lease on receipt of data if enabled (duh) and the proxy participant is a
     "real" participant, rather than the thing we use for endpoints discovered via the DS */
  pwr->assert_pp_lease =
    (unsigned) !!config.arrival_of_data_asserts_pp_and_ep_liveliness;

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
  pwr->tlease_end = add_duration_to_wctime (tnow, pwr->tlease_dur);
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
  if (!is_builtin_entityid (pwr->e.guid.entityid, pwr->c.vendor) && config.generate_builtin_topics)
  {
    write_builtin_topic_proxy_writer (pwr);
  }

  return 0;
}

void update_proxy_writer (struct proxy_writer * pwr, struct addrset * as)
{
  struct reader * rd;
  struct  pwr_rd_match * m;
  ut_avlIter_t iter;

  /* Update proxy writer endpoints (from SEDP alive) */

  os_mutexLock (&pwr->e.lock);
  if (! addrset_eq_onesidederr (pwr->c.as, as))
  {
    unref_addrset (pwr->c.as);
    ref_addrset (as);
    pwr->c.as = as;
    m = ut_avlIterFirst (&pwr_readers_treedef, &pwr->readers, &iter);
    while (m)
    {
      rd = ephash_lookup_reader_guid (&m->rd_guid);
      if (rd)
      {
        qxev_pwr_entityid (pwr, &rd->e.guid.prefix);
      }
      m = ut_avlIterNext (&iter);
    }
  }
  os_mutexUnlock (&pwr->e.lock);
}

void update_proxy_reader (struct proxy_reader * prd, struct addrset * as)
{
  struct prd_wr_match * m;
  nn_guid_t wrguid;

  memset (&wrguid, 0, sizeof (wrguid));

  os_mutexLock (&prd->e.lock);
  if (! addrset_eq_onesidederr (prd->c.as, as))
  {
    /* Update proxy reader endpoints (from SEDP alive) */

    unref_addrset (prd->c.as);
    ref_addrset (as);
    prd->c.as = as;

    /* Rebuild writer endpoints */

    while ((m = ut_avlLookupSuccEq (&prd_writers_treedef, &prd->writers, &wrguid)) != NULL)
    {
      struct prd_wr_match *next;
      nn_guid_t guid_next;
      struct writer * wr;

      wrguid = m->wr_guid;
      next = ut_avlFindSucc (&prd_writers_treedef, &prd->writers, m);
      if (next)
      {
        guid_next = next->wr_guid;
      }
      else
      {
        memset (&guid_next, 0xff, sizeof (guid_next));
        guid_next.entityid.u = (guid_next.entityid.u & ~(unsigned)0xff) | NN_ENTITYID_KIND_WRITER_NO_KEY;
      }

      os_mutexUnlock (&prd->e.lock);
      wr = ephash_lookup_writer_guid (&wrguid);
      if (wr)
      {
        os_mutexLock (&wr->e.lock);
        rebuild_writer_addrset (wr);
        os_mutexUnlock (&wr->e.lock);
        qxev_prd_entityid (prd, &wr->e.guid.prefix);
      }
      wrguid = guid_next;
      os_mutexLock (&prd->e.lock);
    }
  }
  os_mutexUnlock (&prd->e.lock);
}

static void gc_delete_proxy_writer (struct gcreq *gcreq)
{
  struct proxy_writer *pwr = gcreq->arg;
  TRACE (("gc_delete_proxy_writer(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (pwr->e.guid)));
  gcreq_free (gcreq);

  while (!ut_avlIsEmpty (&pwr->readers))
  {
    struct pwr_rd_match *m = ut_avlRoot (&pwr_readers_treedef, &pwr->readers);
    ut_avlDelete (&pwr_readers_treedef, &pwr->readers, m);
    reader_drop_connection (&m->rd_guid, pwr);
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

int delete_proxy_writer (const struct nn_guid *guid, int isimplicit)
{
  struct proxy_writer *pwr;
  TRACE (("delete_proxy_writer (%x:%x:%x:%x) ", PGUID (*guid)));
  os_mutexLock (&gv.lock);
  if ((pwr = ephash_lookup_proxy_writer_guid (guid)) == NULL)
  {
    os_mutexUnlock (&gv.lock);
    TRACE (("- unknown\n"));
    return ERR_UNKNOWN_ENTITY;
  }
  TRACE (("- deleting\n"));
  ephash_remove_proxy_writer_guid (pwr);
  os_mutexUnlock (&gv.lock);
  if (!is_builtin_entityid (pwr->e.guid.entityid, pwr->c.vendor) && config.generate_builtin_topics)
  {
    dispose_builtin_topic_proxy_writer (pwr, isimplicit);
  }
  gcreq_proxy_writer (pwr);
  return 0;
}

/* PROXY-READER ----------------------------------------------------- */

int new_proxy_reader (const struct nn_guid *ppguid, const struct nn_guid *guid, struct addrset *as, const nn_plist_t *plist
                      )
{
  struct proxy_participant *proxypp;
  struct proxy_reader *prd;
  nn_mtime_t tnow = now_mt ();

  assert (!is_writer_entityid (guid->entityid));
  assert (ephash_lookup_proxy_reader_guid (guid) == NULL);

  if ((proxypp = ephash_lookup_proxy_participant_guid (ppguid)) == NULL)
  {
    NN_WARNING1 ("new_proxy_reader(%x:%x:%x:%x): proxy participant unknown\n", PGUID (*guid));
    return ERR_UNKNOWN_ENTITY;
  }

  prd = os_malloc (sizeof (*prd));
  proxy_endpoint_common_init (&prd->e, &prd->c, EK_PROXY_READER, guid, proxypp, as, plist);

  prd->deleting = 0;
  prd->is_fict_trans_reader = 0;
  /* Only assert PP lease on receipt of data if enabled (duh) and the proxy participant is a
     "real" participant, rather than the thing we use for endpoints discovered via the DS */
  prd->assert_pp_lease = (unsigned) !!config.arrival_of_data_asserts_pp_and_ep_liveliness;
  if (!is_builtin_entityid (guid->entityid, prd->c.vendor) &&
      (proxypp->plist->present & PP_PRISMTECH_PARTICIPANT_VERSION_INFO) &&
      (proxypp->plist->prismtech_participant_version_info.flags & NN_PRISMTECH_FL_DISCOVERY_INCLUDES_GID) &&
      !((plist->present & PP_PRISMTECH_ENDPOINT_GID) &&
        (plist->endpoint_gid.systemId != 0 || plist->endpoint_gid.localId != 0 ||
         plist->endpoint_gid.serial != 0)))
  {
    TRACE (("proxy_reader(%x:%x:%x:%x): no GID, considering fictitious transient data reader\n", PGUID (*guid)));
    prd->is_fict_trans_reader = 1;
  }

  ut_avlInit (&prd_writers_treedef, &prd->writers);
  ephash_insert_proxy_reader_guid (prd);
  match_proxy_reader_with_writers (prd, tnow);
  if (!is_builtin_entityid (prd->e.guid.entityid, prd->c.vendor) && !prd->is_fict_trans_reader && config.generate_builtin_topics)
  {
    write_builtin_topic_proxy_reader (prd);
  }
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
      wrguid_next.entityid.u = (wrguid_next.entityid.u & ~(unsigned)0xff) | NN_ENTITYID_KIND_WRITER_NO_KEY;
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
        writer_clear_retransmitting (wr);
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
  TRACE (("gc_delete_proxy_reader(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (prd->e.guid)));
  gcreq_free (gcreq);

  while (!ut_avlIsEmpty (&prd->writers))
  {
    struct prd_wr_match *m = ut_avlRoot (&prd_writers_treedef, &prd->writers);
    ut_avlDelete (&prd_writers_treedef, &prd->writers, m);
    writer_drop_connection (&m->wr_guid, prd);
    free_prd_wr_match (m);
  }

  proxy_endpoint_common_fini (&prd->e, &prd->c);
  os_free (prd);
}

int delete_proxy_reader (const struct nn_guid *guid, int isimplicit)
{
  struct proxy_reader *prd;
  TRACE (("delete_proxy_reader (%x:%x:%x:%x) ", PGUID (*guid)));
  os_mutexLock (&gv.lock);
  if ((prd = ephash_lookup_proxy_reader_guid (guid)) == NULL)
  {
    os_mutexUnlock (&gv.lock);
    TRACE (("- unknown\n"));
    return ERR_UNKNOWN_ENTITY;
  }
  ephash_remove_proxy_reader_guid (prd);
  os_mutexUnlock (&gv.lock);
  TRACE (("- deleting\n"));

  /* If the proxy reader is reliable, pretend it has just acked all
     messages: this allows a throttled writer to once again make
     progress, which in turn is necessary for the garbage collector to
     do its work. */
  proxy_reader_set_delete_and_ack_all_messages (prd);

  if (!is_builtin_entityid (prd->e.guid.entityid, prd->c.vendor) && !prd->is_fict_trans_reader && config.generate_builtin_topics)
  {
    dispose_builtin_topic_proxy_reader (prd, isimplicit);
  }
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
  TRACE (("gc_delete_proxy_writer_dqueue_bubble(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (pwr->e.guid)));
  gcreq_requeue (gcreq, gc_delete_proxy_writer);
}

static void gc_delete_proxy_writer_dqueue (struct gcreq *gcreq)
{
  /* delete proxy_writer, phase 2 */
  struct proxy_writer *pwr = gcreq->arg;
  struct nn_dqueue *dqueue = pwr->dqueue;
  TRACE (("gc_delete_proxy_writer_dqueue(%p, %x:%x:%x:%x)\n", (void *) gcreq, PGUID (pwr->e.guid)));
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
