/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <assert.h>

#include "vortex_os.h"
#include "os_defs.h"
#include "os_mutex.h"
#include "os_socket.h"
#include "os_time.h"
#include "os_semaphore.h"
#include "os_atomics.h"

#include "c_iterator.h"
#include "c_stringSupport.h"

#include "v_networkReaderEntry.h"
#include "v_dataReaderSample.h"
#include "v_topic.h"
#include "v_group.h"
#include "v_builtin.h"
#include "v_service.h"
#include "v_state.h"
#include "v_networkQueue.h"
#include "v_participant.h"
#include "v_instance.h"

#include "u_user.h"
#include "u_domain.h"
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_publisher.h"
#include "u_dataReader.h"
#include "u_writer.h"
#include "u_waitset.h"
#include "u_networking.h"
#include "u_networkReader.h"
#include "u_topic.h"
#include "u_observable.h"

#include "sd_serializer.h"
#include "sd_serializerXML.h"

#include "q_feature_check.h"

#include "q_ddsi2.h"
#include "q_config.h"
#include "q_log.h"
#include "q_misc.h"
#include "q_xmsg.h"
#include "q_unused.h"
#include "q_thread.h"
#include "q_servicelease.h"
#include "q_entity.h"
#include "q_rtps.h"
#include "q_ephash.h"
#include "q_time.h"
#include "q_transmit.h"
#include "q_xevent.h"
#include "q_globals.h"
#include "q_error.h"
#include "q_osplser.h"
#include "q_addrset.h"
#include "q_osplserModule.h"
#include "q_builtin_topic.h"
#include "q_static_assert.h"
#include "q_ddsi_discovery.h"
#include "q_osplbuiltin.h"
#include "q_lease.h"
#include "q_mtreader.h"
#include "q_debmon.h"

#include "ddsi_tran.h"
#ifdef OSPL_ENTRY_OVERRIDE
#include "ospl_entry_override.h"
#endif

static u_participant participant = NULL;
static u_subscriber networkSubscriber = NULL;
static u_networkReader networkReader = NULL;
static u_waitset local_discovery_waitset = NULL;
static os_mutex gluelock;
static os_cond gluecond;
static v_gid ddsi2_participant_gid;
static struct ephash *gid_hash;

#define BUBBLE_TOPIC_NAME "q_bubble"
static u_topic bubble_topic;
static v_topic bubble_kernel_topic;
static u_publisher bubble_publisher;
static u_writer bubble_writer;
static v_gid bubble_writer_gid;
static int bubble_writer_group_attached;

static int check_all_proxy_participants;

#define DDSI_CONTROL_TOPIC_NAME "q_ddsiControl"
static u_topic ddsi_control_topic;

struct builtin_datareader_set1 {
  u_subscriber subscriber;
  u_dataReader ddsi_control_dr;
  u_dataReader participant_dr;
  u_dataReader cm_participant_dr;
  struct mtreader *participant_mtr;
  u_dataReader subscription_dr;
  u_dataReader cm_datareader_dr;
  struct mtreader *datareader_mtr;
  u_dataReader publication_dr;
  u_dataReader cm_datawriter_dr;
  struct mtreader *datawriter_mtr;
  u_dataReader topic_dr;
  u_dataReader cm_publisher_dr;
  struct mtreader *publisher_mtr;
  u_dataReader cm_subscriber_dr;
  struct mtreader *subscriber_mtr;
};

struct builtin_datareader_set {
  struct builtin_datareader_set1 e[2];
  u_topic ddsi_control_tp;
  u_topic participant_tp;
  u_topic cm_participant_tp;
  u_topic publication_tp;
  u_topic cm_datawriter_tp;
  u_topic subscription_tp;
  u_topic cm_datareader_tp;
  u_topic topic_tp;
  u_topic cm_publisher_tp;
  u_topic cm_subscriber_tp;
};

struct channel_reader_arg
{
  struct builtin_datareader_set *drset;
  ddsi_tran_conn_t transmit_conn;
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  struct config_channel_listelem *channel_cfg;
#endif
};

static int compare_gid (const v_gid *a, const v_gid *b);

/* TEMPORARY: until we have subscriber & publisher discovery, which we need for generating the corresponding CM data */
struct group_gid_guid_node {
  ut_avlNode_t avlnode_gid;
  ut_avlNode_t avlnode_guid;
  v_gid gid;
  nn_guid_t guid;
  int grpentityseen;
  os_uint32 refc;
};

static os_mutex group_gid_guid_lock;

static ut_avlTree_t group_gid_guid_tree;
static const ut_avlTreedef_t group_gid_guid_td =
UT_AVL_TREEDEF_INITIALIZER (offsetof (struct group_gid_guid_node, avlnode_gid), offsetof (struct group_gid_guid_node, gid), (int (*) (const void *, const void *)) compare_gid, 0);

static int compare_guid (const nn_guid_t *a, const nn_guid_t *b);
static ut_avlTree_t group_guid_gid_tree;
static const ut_avlTreedef_t group_guid_gid_td =
UT_AVL_TREEDEF_INITIALIZER (offsetof (struct group_gid_guid_node, avlnode_guid), offsetof (struct group_gid_guid_node, guid), (int (*) (const void *, const void *)) compare_guid, 0);

struct q_globals gv;

static u_result handleDDSIControl (const struct builtin_datareader_set *drset);
static u_result handleParticipants (const struct builtin_datareader_set *drset);
static u_result handleParticipantsSelf (const struct builtin_datareader_set *drset);
static u_result handleDataReaders (const struct builtin_datareader_set *drset);
static u_result handleDataWriters (const struct builtin_datareader_set *drset);
static u_result handleTopics (const struct builtin_datareader_set *drset);
static u_result handleSubscribers (const struct builtin_datareader_set *drset);
static u_result handlePublishers (const struct builtin_datareader_set *drset);
static u_result handleDataReader (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state);
static u_result handleDataWriter (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state);
static u_result handleSubscriber (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state);
static u_result handlePublisher (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state);
static u_result handleGroups (v_service service, void *varg);
static void handleGroupsAction (v_public service, void *vres);
static u_result handleTopicReadersWriters (const struct builtin_datareader_set *drset, int setidx, const char *tpname);
static void destroy_builtin_readers (struct builtin_datareader_set *drset);
static u_result create_builtin_readers (struct builtin_datareader_set *drset, u_participant p);
static void new_fictitious_transient_reader (v_group group);

enum ignore_builtin { IGNB_ANY, IGNB_READER, IGNB_WRITER };
static int ignore_builtin_readerwriter (const char *partition, const char *topic_name, enum ignore_builtin whatfor);

static v_copyin_result bubble_writer_copy (UNUSED_ARG (c_type type), const void *data, void *to);

static int compare_gid (const v_gid *a, const v_gid *b)
{
  return memcmp (a, b, sizeof (*a));
}

static int compare_guid (const nn_guid_t *a, const nn_guid_t *b)
{
  return memcmp (a, b, sizeof (*a));
}

static void ppguid_from_ppgid (nn_guid_t *ppguid, const struct v_gid_s *ppgid)
{
  ppguid->prefix.u[0] = ppgid->systemId;
  ppguid->prefix.u[1] = ppgid->localId;
  ppguid->prefix.u[2] = ppgid->serial;
  ppguid->entityid.u = NN_ENTITYID_PARTICIPANT;
}

static int new_participant_gid (const struct v_gid_s *ppgid, const nn_plist_t *ps, unsigned flags)
{
  nn_guid_t ppguid;
  ppguid_from_ppgid (&ppguid, ppgid);
  return new_participant_guid (&ppguid, flags, ps);
}

static int delete_participant_gid (const struct v_gid_s *ppgid)
{
  nn_guid_t ppguid;
  ppguid_from_ppgid (&ppguid, ppgid);
  return delete_participant (&ppguid);
}

static int lookup_group_guid (nn_guid_t *group_guid, const struct v_gid_s *group_gid)
{
  if (v_gidIsNil (*group_gid))
  {
    memset (group_guid, 0, sizeof (*group_guid));
    return 1;
  }
  else
  {
    struct group_gid_guid_node *g;
    os_mutexLock (&group_gid_guid_lock);
    if ((g = ut_avlLookup (&group_gid_guid_td, &group_gid_guid_tree, group_gid)) != NULL)
      *group_guid = g->guid;
    os_mutexUnlock (&group_gid_guid_lock);
    return (g != NULL);
  }
}

static void map_group_gid_to_guid (nn_guid_t *group_guid, const struct v_gid_s *group_gid, struct participant *pp, unsigned kind, int isgrpentity)
{
  ut_avlIPath_t ipath;
  struct group_gid_guid_node *g;
  if (v_gidIsNil (*group_gid))
    memset (group_guid, 0, sizeof (*group_guid));
  else
  {
    os_mutexLock (&group_gid_guid_lock);
    if ((g = ut_avlLookupIPath (&group_gid_guid_td, &group_gid_guid_tree, group_gid, &ipath)) != NULL)
    {
      assert ((g->guid.entityid.u & (NN_ENTITYID_SOURCE_MASK | NN_ENTITYID_KIND_MASK)) == kind);
      assert (ut_avlLookup (&group_guid_gid_td, &group_guid_gid_tree, &g->guid) != NULL);
      /* pp = NULL: simply inquiry; isgrpentity=0: reader/writer; =1
       "creating" the group entity, i.e. responding to a non-disposed
       CM subscriber/publisher topic.  Refcount should only be
       increased if it is for a reader, writer or the first time we
       encounter the CM topic (cos of potential QoS changes). */
      if (!(isgrpentity && g->grpentityseen))
      {
        g->grpentityseen = isgrpentity || g->grpentityseen;
        g->refc++;
      }
      *group_guid = g->guid;
      nn_log (LC_DISCOVERY, "map group "PGIDFMT" -> "PGUIDFMT" refc %u (ref)\n", PGID (*group_gid), PGUID (*group_guid), g->refc);
    }
    else if ((g = os_malloc (sizeof (*g))) == NULL)
    {
      memset (group_guid, 0, sizeof (*group_guid));
    }
    else
    {
      g->gid = *group_gid;
      g->guid.prefix = pp->e.guid.prefix;
      pp_allocate_entityid (&g->guid.entityid, kind, pp);
      g->refc = 1;
      g->grpentityseen = isgrpentity;
      ut_avlInsertIPath (&group_gid_guid_td, &group_gid_guid_tree, g, &ipath);
      ut_avlInsert (&group_guid_gid_td, &group_guid_gid_tree, g);
      *group_guid = g->guid;
      nn_log (LC_DISCOVERY, "map group "PGIDFMT" -> "PGUIDFMT" refc %u (new)\n", PGID (*group_gid), PGUID (*group_guid), g->refc);
    }
    os_mutexUnlock (&group_gid_guid_lock);
  }
}

static void unmap_group_gid_to_guid (const nn_guid_t *group_guid)
{
  ut_avlDPath_t dpath;
  struct group_gid_guid_node *g;
  os_mutexLock (&group_gid_guid_lock);
  if ((g = ut_avlLookupDPath (&group_guid_gid_td, &group_guid_gid_tree, group_guid, &dpath)) != NULL)
  {
    assert (ut_avlLookup (&group_gid_guid_td, &group_gid_guid_tree, &g->gid) != NULL);
    if (--g->refc != 0)
      nn_log (LC_DISCOVERY, "unmap group "PGIDFMT" -> "PGUIDFMT" refc %u (unref)\n", PGID (g->gid), PGUID (g->guid), g->refc);
    else
    {
      nn_log (LC_DISCOVERY, "unmap group "PGIDFMT" -> "PGUIDFMT" refc %u (free)\n", PGID (g->gid), PGUID (g->guid), g->refc);
      ut_avlDeleteDPath (&group_guid_gid_td, &group_guid_gid_tree, g, &dpath);
      ut_avlDelete (&group_gid_guid_td, &group_gid_guid_tree, g);
      os_free (g);
    }
  }
  os_mutexUnlock (&group_gid_guid_lock);
}

static int new_writer_gid (const nn_guid_t *ppguid, const nn_guid_t *group_guid, const struct v_gid_s *group_gid, const struct v_gid_s *gid, C_STRUCT (v_topic) const * const ospl_topic, const struct nn_xqos *xqos, const char *endpoint_name)
{
  nn_guid_t guid;
  sertopic_t topic;
  struct writer *wr;

  assert (ospl_topic != NULL);

  if (ephash_lookup_writer_gid (gid_hash, gid))
  {
    nn_log (LC_DISCOVERY, "new_writer(gid "PGIDFMT") - already known\n", PGID (*gid));
    return ERR_ENTITY_EXISTS;
  }

  nn_log (LC_DISCOVERY, "new_writer(gid "PGIDFMT")\n", PGID (*gid));

  if ((topic = deftopic (ospl_topic)) == NULL)
    return ERR_UNSPECIFIED;

  wr = new_writer (&guid, group_guid, ppguid, topic, xqos, gid, group_gid, endpoint_name);
  assert (wr);
  ephash_insert_writer_gid (gid_hash, wr);

  return 0;
}

int delete_writer_gid (const struct v_gid_s *gid)
{
  struct writer *wr;
  assert (v_gidIsValid (*gid));
  if ((wr = ephash_lookup_writer_gid (gid_hash, gid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_writer(gid "PGIDFMT") - unknown gid\n", PGID (*gid));
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_writer(gid "PGIDFMT") ...\n", PGID (*gid));
  ephash_remove_writer_gid (gid_hash, wr);
  unmap_group_gid_to_guid(&wr->c.group_guid);
  delete_writer (&wr->e.guid);
  return 0;
}

static int new_reader_gid (const nn_guid_t *ppguid, const nn_guid_t *group_guid, const struct v_gid_s *group_gid, const struct v_gid_s *gid, C_STRUCT (v_topic) const * const ospl_topic, const struct nn_xqos *xqos, const char *endpoint_name)
{
  /* see new_writer for comments */
  nn_guid_t guid;
  sertopic_t topic;
  struct reader *rd;

  assert (ospl_topic != NULL);

  if (ephash_lookup_reader_gid (gid_hash, gid))
  {
    nn_log (LC_DISCOVERY, "new_reader(gid "PGIDFMT") - already known\n", PGID (*gid));
    return ERR_ENTITY_EXISTS;
  }

  nn_log (LC_DISCOVERY, "new_reader(gid "PGIDFMT")\n", PGID (*gid));

  if ((topic = deftopic (ospl_topic)) == NULL)
    return ERR_UNSPECIFIED;

  rd = new_reader (&guid, group_guid, ppguid, topic, xqos, gid, group_gid, endpoint_name);
  assert (rd);
  ephash_insert_reader_gid (gid_hash, rd);
  return 0;
}

int delete_reader_gid (const struct v_gid_s *gid)
{
  struct reader *rd;
  assert (v_gidIsValid (*gid));
  if ((rd = ephash_lookup_reader_gid (gid_hash, gid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_reader_gid(gid "PGIDFMT") - unknown gid\n", PGID (*gid));
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_reader_gid(gid "PGIDFMT") ...\n", PGID (*gid));
  ephash_remove_reader_gid (gid_hash, rd);
  unmap_group_gid_to_guid(&rd->c.group_guid);
  return delete_reader (&rd->e.guid);
}

static void handle_bubble (C_STRUCT (v_message) const * msg)
{
  const struct bubble_s *data = (const struct bubble_s *) (msg + 1);
  v_gid gid;
  switch (data->kind)
  {
    case BTK_DELETE_WRITER:
      gid.systemId = data->systemId;
      gid.localId = data->localId;
      gid.serial = data->serial;
      nn_log (LC_DISCOVERY, "handle_bubble: delete_writer("PGIDFMT")\n", PGID (gid));
      delete_writer_gid (&gid);
      break;
    default:
      NN_FATAL1 ("ddsi2: handle_bubble: kind %d unknown\n", (int) data->kind);
      break;
  }
}

void trace_v_message_eot (const struct v_messageEOT_s *eot)
{
  const struct v_tid *tids = (const struct v_tid *) eot->tidList;
  unsigned i;
  TRACE (("EOT = { state %u seq %u txnid %u wtime %"PA_PRItime" wrgid "PGIDFMT" wrinstgid "PGIDFMT" }",
          eot->_parent._parent.nodeState, eot->_parent.sequenceNumber, eot->_parent.transactionId,
          OS_TIMEW_PRINT(eot->_parent.writeTime),
          PGID (eot->_parent.writerGID), PGID (eot->_parent.writerInstanceGID)));
  TRACE ((" pubid %u txnid %u %u:{",
          eot->publisherId, eot->transactionId, (unsigned)c_arraySize(eot->tidList)));
  for (i = 0; i < (unsigned)c_arraySize(eot->tidList); i++)
    TRACE ((" "PGIDFMT",%u", PGID (tids[i].wgid), tids[i].seqnr));
  TRACE (("}"));
}

int rtps_write (struct nn_xpack *xp, const struct v_gid_s *wrgid, C_STRUCT (v_message) const *msg)
{
  serdata_t serdata;
  struct writer *wr;

  /* Note: may toggle thread states, so GC may occur and we must not cache a pointer past
     the write/end_coherent_set calls */

  if ((wr = ephash_lookup_writer_gid (gid_hash, wrgid)) == NULL)
  {
    if (is_builtin_topic_writer (wrgid))
      return 0;
    else
    {
      nn_log (LC_DISCOVERY, "rtps_write(gid "PGIDFMT") - unknown gid\n", PGID (*wrgid));
      return ERR_UNKNOWN_ENTITY;
    }
  }

  /* end_coherent_set is a no-op if no transaction is already
     underway, so even when we get multiple copies because the writer
     is publishing in multiple partitions, the transaction will be
     committed only once. */
  if (v_nodeState ((v_message) msg) & L_ENDOFTRANSACTION)
  {
    C_STRUCT (v_messageEOT) const *eot = (v_messageEOT) msg;
    const struct v_tid *tids = (const struct v_tid *) eot->tidList;
    serdata_t d = serialize_empty (gv.serpool, 0, msg);
    nn_prismtech_eotinfo_t *txnid;
    nn_plist_t *ps;
    os_uint32 i;
    TRACE (("rtps_write(gid "PGIDFMT") - seq %u txn id %u ends\n", PGID (*wrgid), msg->sequenceNumber, msg->transactionId));
    trace_v_message_eot (eot);
    TRACE (("\n"));
    ps = os_malloc (sizeof (*ps));
    nn_plist_init_empty (ps);
    ps->present |= PP_PRISMTECH_EOTINFO;
    txnid = &ps->eotinfo;
    txnid->transactionId = eot->transactionId;
    txnid->n = c_arraySize (eot->tidList);
    if (txnid->n == 0)
      txnid->tids = NULL;
    else
    {
      txnid->tids = os_malloc (txnid->n * sizeof (*txnid->tids));
      for (i = 0; i < txnid->n; i++)
      {
        struct writer *some_wr;
        if ((some_wr = ephash_lookup_writer_gid (gid_hash, &tids[i].wgid)) == NULL)
          break;
        txnid->tids[i].writer_entityid = some_wr->e.guid.entityid;
        txnid->tids[i].transactionId = tids[i].seqnr;
      }
      if (i < txnid->n)
      {
        nn_log (LC_DISCOVERY, "rtps_write(gid "PGIDFMT") - dropping transaction including unknown gid "PGIDFMT"\n", PGID (*wrgid), PGID (tids[i].wgid));
        nn_plist_fini (ps);
        os_free (ps);
        return ERR_UNKNOWN_ENTITY;
      }
    }
    return end_coherent_set_gc (xp, wr, ps, d, 1, msg->sequenceNumber);
  }
  else
  {
    /* begin_coherent set is a no-op if a transaction is already open,
       this therefore simply ensures that IF the message is contained
       in a transaction, it will also be in a transaction at DDSI
       level */
    if ((v_nodeState ((v_message) msg) & L_TRANSACTION) && msg->sequenceNumber == msg->transactionId)
    {
      TRACE (("rtps_write(gid "PGIDFMT") - seq %u txn id %u begins\n", PGID (*wrgid), msg->sequenceNumber, msg->transactionId));
      begin_coherent_set (wr);
    }

    switch (v_nodeState ((v_message) msg) & ~(L_SYNCHRONOUS | L_TRANSACTION | L_ENDOFTRANSACTION | L_AUTO))
    {
      case L_WRITE:
      case L_WRITE | L_DISPOSED:
      case L_WRITE | L_DISPOSED | L_UNREGISTER:
        if ((serdata = serialize (gv.serpool, wr->topic, msg)) == NULL)
        {
          NN_WARNING0 ("serialization (data) failed\n");
          return ERR_UNSPECIFIED;
        }
        break;
      case L_DISPOSED:
      case L_UNREGISTER:
      case L_DISPOSED | L_UNREGISTER:
        if ((serdata = serialize_key (gv.serpool, wr->topic, msg)) == NULL)
        {
          NN_WARNING0 ("serialization (key) failed\n");
          return ERR_UNSPECIFIED;
        }
        break;
      case L_REGISTER:
        return 0;
      default:
        NN_WARNING1 ("rtps_write: unhandled message state: %u\n", (unsigned) v_nodeState ((v_message) msg));
        return ERR_UNSPECIFIED;
    }
#ifndef NDEBUG
    if ((config.enabled_logcats & LC_TRACE) && (v_nodeState ((v_message) msg) & L_WRITE))
      assert (serdata_verify (serdata, msg));
#endif
    return write_sample_kernel_seq_gc (xp, wr, serdata, 1, msg->sequenceNumber);
  }
}

static void *channel_reader_thread_main (v_entity e, struct channel_reader_arg *arg)
{
  struct thread_state1 *self = lookup_thread_state ();
  nn_mtime_t next_thread_cputime = { 0 };
  v_networkReader vnetworkReader = v_networkReader(e);
  v_networkQueue vnetworkQueue = NULL;
  struct nn_xpack *xp;
  os_uint32 bw_limit = 0;

#ifdef DDSI_INCLUDE_BANDWIDTH_LIMITING
  bw_limit = arg->channel_cfg->data_bandwidth_limit;
#endif
  xp = nn_xpack_new (arg->transmit_conn, bw_limit);

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  nn_log (LC_CONFIG, "reader_thread: \"%s\" (priority %d) started \n", arg->channel_cfg->name, arg->channel_cfg->priority);
#endif

  while (!gv.terminate && !gv.exception)
  {
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
    const c_ulong queueId = arg->channel_cfg->queueId;
#else
    const c_ulong queueId = gv.networkQueueId;
#endif
    v_networkReaderWaitResult nrwr;
    c_bool sendTo, more;
    v_message message;
    c_ulong sequenceNumber, priority;
    v_gid sender, receiver;
    os_timeE sendBefore;
    v_networkReaderEntry entry;

    LOG_THREAD_CPUTIME (next_thread_cputime);

    nrwr = v_networkReaderWait (vnetworkReader, queueId, &vnetworkQueue);
    if ((nrwr & V_WAITRESULT_MSGWAITING) == V_WAITRESULT_MSGWAITING)
    {
      do {
        thread_state_awake (self);
        if (v_networkQueueTakeFirst (
                    vnetworkQueue, &message, &entry, &sequenceNumber,
                    &sender, &sendTo, &receiver, &sendBefore, &priority, &more))
        {
          if (v_gidEqual (sender, bubble_writer_gid))
          {
            handle_bubble (message);
          }
          else if (rtps_write (xp, &sender, message) == ERR_UNKNOWN_ENTITY)
          {
            u_result dummy;
            /* retry after checking for new publications */
            os_mutexLock (&gluelock);

            (void) u_observableAction (u_observable (participant), handleGroupsAction, &dummy);
            (void) handleTopics (arg->drset);
            (void) handleParticipants (arg->drset);
            (void) handlePublishers (arg->drset);
            (void) handleDataWriters (arg->drset);

            if (rtps_write (xp, &sender, message) == ERR_UNKNOWN_ENTITY)
              nn_log (LC_DISCOVERY, "message dropped because sender "PGIDFMT" is unknown\n", PGID(sender));
            os_mutexUnlock (&gluelock);
          }
          c_free (message);
          c_free (entry);
        }
      } while (more);
      nn_xpack_send (xp);
      thread_state_asleep (self);
    }
    else if ((nrwr & V_WAITRESULT_TRIGGERED) == V_WAITRESULT_TRIGGERED)
    {
      TRACE (("transmit_thread %p: stopping as requested\n", (void*)self));
      break;
    }
  }
  nn_xpack_free (xp);
  os_free (arg);
  return NULL;
}

static void *channel_reader_thread (struct channel_reader_arg *arg)
{
    u_observableAction(u_observable(networkReader), (void (*) (v_public, void *))channel_reader_thread_main, arg);
    return NULL;
}

static void determine_kernel_service_helper (v_public entity, UNUSED_ARG (void *varg))
{
  v_service s = v_service(entity);
  gv.ospl_base = c_getBase (s);
  gv.ospl_kernel = v_object (s)->kernel;
}

static void determine_kernel_service (u_participant participant)
{
  u_observableAction (u_observable(participant), determine_kernel_service_helper, NULL);
}

static void watch_spliced (v_serviceStateKind spliceDaemonState, UNUSED_ARG (void *usrData))
{
  switch (spliceDaemonState)
  {
    case STATE_TERMINATING:
    case STATE_TERMINATED:
    case STATE_DIED:
      if (!gv.exception) {
        gv.terminate = TRUE;
        nn_log (LC_INFO, "splice daemon is terminating and so am I...\n");
        u_serviceChangeState (u_service (participant), STATE_TERMINATING);
        os_mutexLock (&gluelock);
        if (local_discovery_waitset)
          u_waitsetNotify (local_discovery_waitset, NULL);
        if (networkReader) {
#ifndef DDSI_INCLUDE_NETWORK_CHANNELS
          if (gv.networkQueueId)
            u_networkReaderTrigger (networkReader, gv.networkQueueId);
#else
          struct config_channel_listelem *chptr = config.channels;
          while (chptr != NULL)
          {
            if (chptr->queueId)
              u_networkReaderTrigger (networkReader, chptr->queueId);
            chptr = chptr->next;
          }
#endif
        }
        os_mutexUnlock (&gluelock);
      }
      break;
    default:
      break;
  }
}

static u_waitset create_discovery_waitset (u_participant participant, struct builtin_datareader_set *drset)
{
  u_dataReader readers[10 * (sizeof (drset->e) / sizeof (*drset->e))];
  u_waitset waitset;
  int i, idx;

  idx = 0;
  for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++)
  {
    readers[idx++] = drset->e[i].ddsi_control_dr;
    readers[idx++] = drset->e[i].participant_dr;
    readers[idx++] = drset->e[i].cm_participant_dr;
    readers[idx++] = drset->e[i].publication_dr;
    readers[idx++] = drset->e[i].cm_datawriter_dr;
    readers[idx++] = drset->e[i].subscription_dr;
    readers[idx++] = drset->e[i].cm_datareader_dr;
    readers[idx++] = drset->e[i].topic_dr;
    readers[idx++] = drset->e[i].cm_publisher_dr;
    readers[idx++] = drset->e[i].cm_subscriber_dr;
    assert (idx <= (int) (sizeof (readers) / sizeof (*readers)));
  }

  if ((waitset = u_waitsetNew ()) == NULL) {
    NN_ERROR0 ("could not create waitset\n");
    goto err_waitsetNew;
  }
  if (u_waitsetSetEventMask (waitset, V_EVENT_DATA_AVAILABLE | V_EVENT_NEW_GROUP | V_EVENT_SERVICESTATE_CHANGED | V_EVENT_TRIGGER) != U_RESULT_OK) {
    NN_ERROR0 ("could not set event mask of waitset\n");
    goto err_waitsetSetEventMask;
  }

  if (u_observableSetListenerMask ((u_observable) participant, V_EVENT_NEW_GROUP | V_EVENT_SERVICESTATE_CHANGED) != U_RESULT_OK) {
    NN_ERROR0 ("could not set event mask of participant\n");
    goto err_dispatchSetEventMask;
  }

  if (u_serviceFillNewGroups(u_service(participant))  != U_RESULT_OK) {
    NN_ERROR0 ("could not fill service new group event list\n");
    goto err_waitsetAttach;
  }

  if (u_waitsetAttach (waitset, (u_observable) participant, (u_object) participant) != U_RESULT_OK) {
    NN_ERROR0 ("could not attach participant to waitset\n");
    goto err_waitsetAttach;
  }

  for (i = 0; i < (int) (sizeof (readers) / sizeof (*readers)); i++)
  {
    if (readers[i])
    {
      u_dataReader dr = readers[i];
      if (u_observableSetListenerMask ((u_observable) dr, V_EVENT_DATA_AVAILABLE) != U_RESULT_OK) {
        NN_ERROR0 ("could not set event mask of data reader\n");
        break;
      }
      if (u_waitsetAttach (waitset, (u_observable) dr, (u_object) dr) != U_RESULT_OK) {
        NN_ERROR0 ("could not attach data reader to waitset\n");
        break;
      }
    }
  }
  if (i != (int) (sizeof (readers) / sizeof (*readers)))
  {
    while (i--)
    {
      if (readers[i])
        u_waitsetDetach (waitset, (u_observable) readers[i]);
    }
    goto err_attach_readers;
  }

  return waitset;

 err_attach_readers:
  u_waitsetDetach (waitset, u_observable (participant));
 err_waitsetAttach:
 err_dispatchSetEventMask:
 err_waitsetSetEventMask:
  u_objectFree (u_object (waitset));
 err_waitsetNew:
  return NULL;
}

static void destroy_discovery_waitset (u_waitset waitset, u_participant participant, struct builtin_datareader_set *drset)
{
  int i;
  for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++) {
    if (config.enable_control_topic)
      u_waitsetDetach (waitset, (u_observable) drset->e[i].ddsi_control_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].participant_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].cm_participant_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].publication_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].cm_datawriter_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].subscription_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].cm_datareader_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].topic_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].cm_publisher_dr);
    u_waitsetDetach (waitset, (u_observable) drset->e[i].cm_subscriber_dr);
  }
  u_waitsetDetach (waitset, u_observable (participant));
  u_objectFree (u_object (waitset));
}

static int is_local_builtin_partition (const char *partition)
{
  const char *p = partition;
  if (p[0] != '_' || p[1] != '_')
    return 0;
  p += 2;
  if (strncmp (p, "NODE", 4) != 0)
    return 0;
  p += 4;
  while ((*p >= '0'  && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))
    p++;
  if (*p != ' ')
    return 0;
  p++;
  return (strcmp (p, "BUILT-IN PARTITION__") == 0);
}

static int is_builtin_partition (const char *partition)
{
  if (strcmp (partition, config.local_discovery_partition) == 0)
    return 1;
  else if (strcmp (partition, "__BUILT-IN PARTITION__") == 0)
    return 1;
  else
    return is_local_builtin_partition(partition);
}

static int is_builtin_topic_partition (int *istransient, const char *topic, const char *partition)
{
  static struct { const char *name; int istransient; } topics[] = {
    { V_PARTICIPANTINFO_NAME, 1 },
    { V_CMPARTICIPANTINFO_NAME, 1 },
    { V_TOPICINFO_NAME, 1 },
    { V_PUBLICATIONINFO_NAME, 1 },
    { V_SUBSCRIPTIONINFO_NAME, 1 },
    { V_CMDATAWRITERINFO_NAME, 1 },
    { V_CMDATAREADERINFO_NAME, 1 },
    { V_CMPUBLISHERINFO_NAME, 1 },
    { V_CMSUBSCRIBERINFO_NAME, 1 },
    { V_HEARTBEATINFO_NAME, 1 }
  };

  if (partition == NULL || is_builtin_partition (partition))
  {
    unsigned i;
    for (i = 0; i < sizeof (topics) / sizeof (*topics); i++)
    {
      if (strcmp (topic, topics[i].name) == 0)
      {
        if (istransient)
          *istransient = topics[i].istransient;
        return 1;
      }
    }
  }
  return 0;
}

static int is_builtin_topic (const char *partition, const char *topic)
{
  return is_builtin_topic_partition (NULL, topic, partition);
}

int is_topic_discoverable (const char *topic)
{
  /* There are two kinds of built-in topics: those that DDSI handles in its discovery protocol,
     and those that are defined by the kernel, but are not otherwise handled by DDSI. Both are
     in a way "built-in", so we distinguish between "built-in" as something DDSI handles and
     "discoverable" as something that needs to go over discovery. */
  static const char *topics[] = { V_DELIVERYINFO_NAME, V_C_AND_M_COMMAND_NAME };
  unsigned i;
  if (is_builtin_topic_partition (NULL, topic, NULL) || strcmp (topic, BUBBLE_TOPIC_NAME) == 0)
    return 0;
  for (i = 0; i < sizeof (topics) / sizeof (*topics); i++)
    if (strcmp (topic, topics[i]) == 0)
      return 0;
  return 1;
}

struct handle_group_arg {
  v_group group;
  u_result res;
};

static u_result handleGroup (v_networkReader vnetworkReader, v_group group)
{
  const char *partition = v_entity (group->partition)->name;
  const char *topic = v_entity (group->topic)->name;
  int suppress_fictitious_transient_reader = 0;
  int is_bubble_group = 0;

  thread_state_awake (lookup_thread_state());

  /* Need to ignore built-in topics (in the built-in partition) if
     configured to generate them from discovery information, but must
     also mark them as "complete" because durability won't be taking
     care of that. There are very few topics in the built-in
     partition, so the overhead of checking against a handful of
     topics if the partition matches the built-in partition should not
     be an issue. But perhaps having these topics names in various
     places is. */
  if (config.generate_builtin_topics)
  {
    int istransient;
    if (is_builtin_topic_partition (&istransient, topic, partition))
    {
      if (!config.coexistWithNativeNetworking && istransient)
      {
        suppress_fictitious_transient_reader = 1;
        (void)v_groupCompleteSet (group, V_ALIGNSTATE_COMPLETE);
        nn_log (LC_DISCOVERY, "new group '%s.%s': marked complete\n", partition, topic);
      }
    }
  }

  /* We can ignore groups that matched "ignored partitions", except for the
     internal q_bubble group.  Endpoints will still be ignored, and the topic
     isn't transient, so there is not much harm in discovering it either */
  if (strcmp (v_entity (group->partition)->name, V_BUILTIN_PARTITION) == 0 &&
      strcmp (v_entity (group->topic)->name, BUBBLE_TOPIC_NAME) == 0)
  {
    is_bubble_group = 1;
  }

  if (ignore_builtin_readerwriter(partition, topic, IGNB_WRITER) && !is_bubble_group)
  {
    /* Ignore vs creating a reader entry is only relevant for writing data, hence checking whether we are ignoring this for writing */
    v_groupNotifyAwareness (group, config.servicename, FALSE);
    nn_log (LC_DISCOVERY, "Found new group '%s.%s'; node-local partition / built-in topic ignored\n", partition, topic);
  }
  else
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  if (is_ignored_partition (partition, topic) && !is_bubble_group)
  {
    v_groupNotifyAwareness (group, config.servicename, FALSE);
    nn_log (LC_DISCOVERY, "Found new group '%s.%s'; ignored, because it matches an ignoredPartition\n", partition, topic);
  }
  else
#endif
  {
    v_networkReaderEntry entry;
    v_networkRoutingMode routing = V_NETWORKROUTING_NONE;

    nn_log (LC_DISCOVERY, "Found new group '%s.%s'; adding networkReaderEntry...\n", partition, topic);

    switch (config.forward_remote_data)
    {
      case BOOLDEF_DEFAULT: routing = V_NETWORKROUTING_FROM_GROUP; break;
      case BOOLDEF_FALSE: routing = V_NETWORKROUTING_NONE; break;
      case BOOLDEF_TRUE: routing = V_NETWORKROUTING_ROUTING; break;
    }

    if ((entry = v_networkReaderEntryNew (vnetworkReader, group, gv.myNetworkId, 1, 0, routing)) == NULL)
    {
      NN_ERROR0 ("creation of networkReaderEntry failed\n");
      return U_RESULT_INTERNAL_ERROR;
    }

    if (v_topicQosRef(group->topic)->durability.v.kind >= V_DURABILITY_TRANSIENT && !suppress_fictitious_transient_reader)
    {
      /* For transient topics, DDSI readers are spontaneously generated
         to ensure data will indeed arrive -- FIXME: currently no
         provision is made to ensure no "early" publications are lost
         while DDSI discovery is still digesting these readers.

         For convenience, we use the regular DDS<->DDSI mapping to
         handle these ficitious readers, and we pretend these ficitious
         readers are owned by the DDSI service participant. That one has
         been created, and as luck has it, the participants are
         discovered before the groups are. So we just look it up. */
      nn_log (LC_DISCOVERY, "Group is transient - creating DDSI data reader...\n");
      new_fictitious_transient_reader (group);
    }
    v_networkReaderEntryNotifyConnected (entry, config.servicename);
    v_networkReaderRemoteActivityDetected (vnetworkReader);
    v_networkReaderEntryFree(entry);
    if (is_bubble_group)
    {
      TRACE (("bubble writer's group now discovered\n"));
      os_mutexLock (&gluelock);
      bubble_writer_group_attached = 1;
      os_condBroadcast (&gluecond);
      os_mutexUnlock (&gluelock);
    }

    /* Update the groupsets of all readers and proxy writers that match with this group */
    {
      struct sertopic *sertopic = deftopic (group->topic);
      if (sertopic != NULL)
        add_group_to_readers_and_proxy_writers (sertopic, partition, group);
    }
  }

  return U_RESULT_OK;
}

static void handleGroupAction (v_public vp, void *varg)
{
  struct handle_group_arg *arg = varg;
  arg->res = handleGroup ((v_networkReader) vp, arg->group);
}

static u_result handleGroups (v_service service, void *varg)
{
  c_iter vgroups = v_serviceTakeNewGroups (service);
  v_group vgroup = (v_group) c_iterTakeFirst (vgroups);
  u_result ret = U_RESULT_OK;
  OS_UNUSED_ARG (varg);

  while (vgroup && ret == U_RESULT_OK)
  {
    struct handle_group_arg hg_arg;
    hg_arg.group = vgroup;
    ret = u_observableAction (u_observable (networkReader), handleGroupAction, &hg_arg);
    if (ret == U_RESULT_OK)
      ret = hg_arg.res;
    c_free (vgroup);
    vgroup = (v_group) c_iterTakeFirst (vgroups);
  }
  c_iterFree (vgroups);
  return ret;
}

static void handleGroupsAction (v_public p, void *vres)
{
  u_result *res = vres;
  *res = handleGroups ((v_service) p, NULL);
}

/** \brief Handle a power event.
 *
 * This method is NOT thread-safe.
 */
static void handlePowerEvents (struct thread_state1 *self, os_timePowerEvents powerEvents)
{
  /* Currently, only detection of resume events is implemented.
     If a resume event is detected all entities whose leases
     have expired will be deleted. */
    /* test if a new resume has occurred */
    if (pa_ld32 (&powerEvents.resumeCount) > pa_ld32 (&gv.powerEvents.resumeCount)) {
      nn_log (LC_INFO, "Resume event %"PA_PRIu32" detected\n", pa_ld32 (&powerEvents.resumeCount));
      /* Remove all entities whose leases have expired at the time when
         the resume event occurred. Unfortunately, the time
         representation used by the powerEvent differs from the
         time representation used by check_and_handle_lease_expiration(),
         so we need a little bit time of transformation magic here. */
      check_and_handle_lease_expiration(self, now_et ());
    }
    gv.powerEvents = powerEvents;
}

static u_result initial_local_discovery (const struct builtin_datareader_set *drset)
{
  struct thread_state1 *self = lookup_thread_state ();
  u_result (*fs[5]) (const struct builtin_datareader_set *);
  u_result res, res1;
  int i;

  nn_log (LC_DISCOVERY, "Mirror DDSI itself ...\n");

  thread_state_awake (self);
  os_mutexLock (&gluelock);
  res = handleParticipantsSelf (drset);
  os_mutexUnlock (&gluelock);
  thread_state_asleep (self);
  if (res != U_RESULT_OK)
  {
    TRACE (("Failed to discover self\n"));
    return res;
  }

  /* First mirror currently existing participants: we create
   fictitious data readers for transient data and pretend they are
   owned by ourself. For that, we need to mirror ourself before we
   start looking at the groups.*/
  nn_log (LC_DISCOVERY, "Initial local discovery ...\n");

  thread_state_awake (self);
  os_mutexLock (&gluelock);
  fs[0] = handleParticipants;
  fs[1] = handlePublishers;
  fs[2] = handleSubscribers;
  fs[3] = handleDataWriters;
  fs[4] = handleDataReaders;
  for (i = 0; i < (int) (sizeof (fs) / sizeof (*fs)); i++)
  {
    if ((res = fs[i] (drset)) != U_RESULT_OK)
    {
      os_mutexUnlock (&gluelock);
      thread_state_asleep (self);
      return res;
    }
  }
  os_mutexUnlock (&gluelock);
  res = u_observableAction (u_observable (participant), handleGroupsAction, &res1);
  if (res == U_RESULT_OK)
    res = res1;
  thread_state_asleep (self);
  return res;
}

struct process_events_context {
    struct thread_state1 *self;
    u_result result;
    const struct builtin_datareader_set *drset;
};

static void
process_events(
    v_waitsetEvent e,
    void *arg)
{
  struct process_events_context *ctx = (struct process_events_context *)arg;

  thread_state_awake (ctx->self);
  if ((e->kind & V_EVENT_DATA_AVAILABLE) == V_EVENT_DATA_AVAILABLE)
  {
    int i, n, handled = 0;
    os_mutexLock (&gluelock);

    n = (int) (sizeof (ctx->drset->e) / sizeof (*ctx->drset->e));
    for (i = 0; i < n; i++)
    {
      if (config.enable_control_topic && e->userData == (u_entity) ctx->drset->e[i].ddsi_control_dr) {
        handled = 1;
        ctx->result = handleDDSIControl (ctx->drset);
      } else if (e->userData == (u_entity) ctx->drset->e[i].participant_dr ||
                 e->userData == (u_entity) ctx->drset->e[i].cm_participant_dr) {
        handled = 1;
        ctx->result = handleParticipants (ctx->drset);
        break;
      } else if (e->userData == (u_entity) ctx->drset->e[i].subscription_dr ||
                 e->userData == (u_entity) ctx->drset->e[i].cm_datareader_dr) {
        handled = 1;
        ctx->result = handleDataReaders (ctx->drset);
        break;
      } else if (e->userData == (u_entity) ctx->drset->e[i].publication_dr ||
                 e->userData == (u_entity) ctx->drset->e[i].cm_datawriter_dr) {
        handled = 1;
        ctx->result = handleDataWriters (ctx->drset);
        break;
      } else if (e->userData == (u_entity) ctx->drset->e[i].topic_dr) {
        handled = 1;
        ctx->result = handleTopics (ctx->drset);
        break;
      } else if (e->userData == (u_entity) ctx->drset->e[i].cm_publisher_dr) {
        handled = 1;
        ctx->result = handlePublishers (ctx->drset);
        break;
      } else if (e->userData == (u_entity) ctx->drset->e[i].cm_subscriber_dr) {
        handled = 1;
        ctx->result = handleSubscribers (ctx->drset);
        break;
      }
    }

    if (handled)
        ; /* all the convoluted stuff is so we don't lose an error */
    else if (e->userData != NULL)
        NN_FATAL0 ("fatal: waitset triggered by unknown entity\n");
    else
        NN_WARNING1 ("warning: waitset triggered by event %x but entity is missing\n", e->kind);

    os_mutexUnlock (&gluelock);
  }
  if ((e->kind & V_EVENT_NEW_GROUP) == V_EVENT_NEW_GROUP)
  {
    u_result res;
    ctx->result = u_observableAction (u_observable (participant), handleGroupsAction, &res);
    if (ctx->result == U_RESULT_OK)
      ctx->result = res;
  }
  if ((e->kind & V_EVENT_TRIGGER) == V_EVENT_TRIGGER)
  {
    ctx->result = U_RESULT_OK;
  }
  if (e->kind & ~(V_EVENT_DATA_AVAILABLE | V_EVENT_NEW_GROUP | V_EVENT_TRIGGER | V_EVENT_SERVICESTATE_CHANGED))
  {
    NN_FATAL1 ("Received unexpected event %d\n", e->kind);
  }
  thread_state_asleep (ctx->self);
}

static u_result monitor_local_entities (const u_waitset waitset, const struct builtin_datareader_set *drset)
{
  struct process_events_context ctx;
  os_duration timeout = OS_DURATION_INIT(0, 100000000);  /* 100 ms */
  os_result osr;
  os_timePowerEvents powerEvents;

  nn_log (LC_DISCOVERY, "Mirroring DCPS entities in DDSI ...\n");

  ctx.result = U_RESULT_OK;
  ctx.drset = drset;
  ctx.self = lookup_thread_state ();
  while (ctx.result == U_RESULT_OK && !gv.terminate)
  {
    ctx.result = u_waitsetWaitAction (waitset, process_events, &ctx, timeout);
    switch (ctx.result)
    {
      case U_RESULT_OK:
        break;
      case U_RESULT_DETACHING:
        nn_log (LC_INFO, "Starting termination now...\n");
        break;
      case U_RESULT_TIMEOUT:
        ctx.result = U_RESULT_OK;
        /* The waitset has timed out, now check if a power event
           has occurred. We would like the check as instantly as
           possible to prevent unnecessary waiting. To do that we
           we immediately request the current power event values
           using zero timeout. In handlePowerEvents() these are
           compared to the previous values and handled when needed. */
        osr = os_timeGetPowerEvents (&powerEvents, 0);
        if (osr == os_resultSuccess)
          handlePowerEvents (ctx.self, powerEvents);
        break;
      default:
        NN_ERROR0 ("Waitset wait failed.\n");
        break;
    }
  }
  return ctx.result;
}

static u_actionResult reader_take_one_helper (c_object o, c_voidp arg)
{
  v_readerSample s = (v_readerSample) o;
  v_readerSample *sample = (v_readerSample *) arg;
  u_actionResult result = 0;
  if (s != NULL)
  {
      *sample = c_keep (s);
  }
  return result;
}

static u_actionResult reader_take_one_vsnew_helper (c_object o, c_voidp arg)
{
  v_readerSample s = (v_readerSample) o;
  v_readerSample *sample = (v_readerSample *) arg;
  u_actionResult result = 0;
  if (s != NULL)
  {
    v_dataReaderInstance instance = s->instance;
    if (v_stateTest (s->sampleState, L_VALIDDATA) &&
        v_stateTest (v_instanceState(instance), L_NEW))
      *sample = c_keep (s);
    else
    {
      *sample = NULL;
      v_actionResultSet (result, V_PROCEED);
    }
  }
  return result;
}

static u_result reader_take_one (u_dataReader rd, v_dataReaderSample *sample)
{
  *sample = NULL;
  return u_dataReaderTake (rd, U_STATE_ANY, reader_take_one_helper, sample, OS_DURATION_ZERO);
}

static u_result reader_take_one_vsnew (u_dataReader rd, v_dataReaderSample *sample)
{
  *sample = NULL;
  return u_dataReaderTake (rd, U_STATE_ANY, reader_take_one_vsnew_helper, sample, OS_DURATION_ZERO);
}

static const char *mtr_sample_state_str (enum mtr_sample_state samp_state)
{
  switch (samp_state)
  {
    case MTR_SST_DEL: return "delete";
    case MTR_SST_NEW: return "new";
    case MTR_SST_UPD: return "update";
  }
  return "?";
}

static int gid_eq (const void *va, const void *vb)
{
  const v_gid *a = va;
  const v_gid *b = vb;
  return v_gidEqual ((*a), (*b));
}

static int string_eq (const void *va, const void *vb)
{
  const char * const *a = va;
  const char *b = vb;
  return (*a == NULL && b == NULL) || (*a && b && strcmp (*a, b) == 0);
}

static void reset_deaf_mute (struct xevent *xev, UNUSED_ARG (void *varg), UNUSED_ARG (nn_mtime_t tnow))
{
  gv.deaf = 0;
  gv.mute = 0;
  TRACE (("DEAFMUTE auto-reset to [deaf, mute]=[%d, %d]\n", gv.deaf, gv.mute));
  delete_xevent (xev);
}

static u_result handleDDSIControl (const struct builtin_datareader_set *drset)
{
  int setidx;
  TRACE (("handleDDSIControl:"));
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    v_dataReaderSample sample;
    while (reader_take_one (drset->e[setidx].ddsi_control_dr, &sample) == U_RESULT_OK && sample != NULL)
    {
      const struct v_message_s *vmsg = v_dataReaderSampleMessage (sample);
      const struct ddsi_control *c = (const struct ddsi_control *) (vmsg + 1);
      if (c->systemId == 0 ||
          (c->systemId == ddsi2_participant_gid.systemId &&
           (c->localId == 0 ||
            (c->localId == ddsi2_participant_gid.localId &&
             (c->serial == 0 || c->serial == ddsi2_participant_gid.serial)))))
      {
        if (vmsg->_parent.nodeState & L_WRITE) {
          gv.deaf = c->deaf;
          gv.mute = c->mute;
          TRACE ((" DEAFMUTE set [deaf, mute]=[%d, %d]", gv.deaf, gv.mute));
          if (c->duration > 0)
          {
            os_int64 d = (os_int64) (1e9 * c->duration);
            nn_mtime_t when = add_duration_to_mtime (now_mt (), d);
            TRACE ((" reset after %"PA_PRId64" ns", d));
            qxev_callback(when, reset_deaf_mute, 0);
          }
        }
      }
      else
      {
        TRACE ((" ("PGIDFMT" ignore)", PGID(*c)));
      }
      c_free (sample);
    }
  }
  TRACE (("\n"));
  return U_RESULT_OK;
}

static u_result handleSelectEntities (const struct builtin_datareader_set *drset, int setidx, const char *what, struct mtreader *mtr, const struct u_topic_s *topic, const char *field, int (*pred) (const void *va, const void *vb), const void *key, u_result (*handle) (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state))
{
  const struct mtr_sample **res;
  int n;
  n = query_mtreader (mtr, &res, topic, field, pred, key);
  if (n < 0)
    NN_WARNING2 ("handleSelectEndpoints: %s lookup failed (%d)\n", what, n);
  else if (n == 0)
    nn_log (LC_DISCOVERY, "handleSelectEndpoints: no %ss\n", what);
  else
  {
    int i;
    nn_log (LC_DISCOVERY, "handleSelectEndpoints: instantiating %ss\n", what);
    for (i = 0; i < n; i++)
      handle (drset, setidx, res[i], MTR_SST_NEW);
    os_free ((void *) res);
  }
  return U_RESULT_OK;
}

static u_result handleParticipantSubEntities (const struct builtin_datareader_set *drset, int setidx, const v_gid *ppgid)
{
  nn_log (LC_DISCOVERY, "participant "PGIDFMT": instantiating sub-entities\n", PGID (*ppgid));
  (void) handleSelectEntities (drset, setidx, "reader", drset->e[setidx].datareader_mtr, drset->subscription_tp, "participant_key", gid_eq, ppgid, handleDataReader);
  (void) handleSelectEntities (drset, setidx, "writer", drset->e[setidx].datawriter_mtr, drset->publication_tp, "participant_key", gid_eq, ppgid, handleDataWriter);
  (void) handleSelectEntities (drset, setidx, "subscriber", drset->e[setidx].subscriber_mtr, drset->cm_subscriber_tp, "participant_key", gid_eq, ppgid, handleSubscriber);
  (void) handleSelectEntities (drset, setidx, "publisher", drset->e[setidx].publisher_mtr, drset->cm_publisher_tp, "participant_key", gid_eq, ppgid, handlePublisher);
  return U_RESULT_OK;
}

static u_result handleTopicReadersWriters (const struct builtin_datareader_set *drset, int setidx, const char *tpname)
{
  v_topic topic;
  topic = v_lookupTopic (gv.ospl_kernel, tpname);
  c_free (topic);
  if (topic == NULL)
  {
    nn_log (LC_DISCOVERY, "topic %s: topic not yet known in kernel\n", tpname);
    return U_RESULT_PRECONDITION_NOT_MET;
  }
  else
  {
    nn_log (LC_DISCOVERY, "topic %s: instantiating readers/writers\n", tpname);
    (void) handleSelectEntities (drset, setidx, "reader", drset->e[setidx].datareader_mtr, drset->subscription_tp, "topic_name", string_eq, tpname, handleDataReader);
    (void) handleSelectEntities (drset, setidx, "writer", drset->e[setidx].datawriter_mtr, drset->publication_tp, "topic_name", string_eq, tpname, handleDataWriter);
    return U_RESULT_OK;
  }
}

static int is_non_opensplice_bridged_proxy_participant (const struct v_gid_s *pkey)
{
  if (!check_all_proxy_participants)
    return 0;
  else
  {
    struct ephash_enum_proxy_participant e;
    struct proxy_participant *p;
    ephash_enum_proxy_participant_init (&e);
    while ((p = ephash_enum_proxy_participant_next (&e)) != NULL)
    {
      if (v_gidEqual (p->gid, *pkey))
        break;
    }
    ephash_enum_proxy_participant_fini (&e);
    return (p != NULL);
  }
}

static int is_proxypp_or_deleted_pp (const char *label, const struct v_gid_s *pkey, const struct v_gid_s *ekey, enum mtr_sample_state samp_state)
{
  nn_guid_t ppguid;
  ppguid_from_ppgid (&ppguid, pkey);
  if (ephash_lookup_proxy_participant_guid (&ppguid) || is_non_opensplice_bridged_proxy_participant (pkey))
  {
    if (v_gidEqual (*pkey, *ekey))
      nn_log (LC_DISCOVERY, "handle%s: ignoring mt sample for proxypp "PGIDFMT"\n", label, PGID (*pkey));
    else
      nn_log (LC_DISCOVERY, "handle%s: ignoring mt sample for "PGIDFMT" owned by proxypp "PGIDFMT"\n", label, PGID (*ekey), PGID (*pkey));
    return 1;
  }
  else if (samp_state != MTR_SST_DEL && is_deleted_participant_guid (&ppguid, DPG_LOCAL))
  {
    if (v_gidEqual (*pkey, *ekey))
      nn_log (LC_DISCOVERY, "handle%s: ignoring mt sample for recently deleted participant "PGIDFMT"\n", label, PGID (*pkey));
  else
      nn_log (LC_DISCOVERY, "handle%s: ignoring mt sample for "PGIDFMT" owned by deleted participant "PGIDFMT"\n", label, PGID (*ekey), PGID (*pkey));
    return 1;
  }
  else
  {
    nn_log (LC_DISCOVERY, "handle%s: received mt sample for "PGIDFMT" (%s)\n", label, PGID (*ekey), mtr_sample_state_str (samp_state));
    return 0;
  }
}

static u_result handleParticipant (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state)
{
  struct v_participantInfo const * const pdata = (struct v_participantInfo *) (samp->vmsg[0] + 1);
  struct v_participantCMInfo const * const cmpdata = (struct v_participantCMInfo *) (samp->vmsg[1] + 1);
  OS_UNUSED_ARG (setidx);

  if (is_proxypp_or_deleted_pp ("Participant", &pdata->key, &pdata->key, samp_state))
    return U_RESULT_OK;

  /* The participant mapping of the DDSI2 service itself is special-cased, to guarantee it being available before handleGroup() is called for the first time. */
  if (!v_gidEqual (pdata->key, ddsi2_participant_gid) && !config.squash_participants)
  {
    switch (samp_state)
    {
      case MTR_SST_DEL:
        delete_participant_gid (&pdata->key);
        break;
      case MTR_SST_NEW:
        {
          nn_plist_t ps;
          update_mtreader_setflag (drset->e[setidx].participant_mtr, &pdata->key, 1);
          if (init_participant_plist (pdata, cmpdata, &ps) < 0)
            NN_WARNING1 ("participant info initialisation error for "PGIDFMT"\n", PGID(pdata->key));
          else
          {
            unsigned flags = 0;
            if ((ps.present & PP_PRISMTECH_SERVICE_TYPE) &&
                (ps.service_type == (unsigned) V_SERVICETYPE_DDSI2 ||
                 ps.service_type == (unsigned) V_SERVICETYPE_DDSI2E))
            {
              /* Always mark participants that represent DDSI2 as such, to be propagated over
                 discovery for the benefit of Cloud-over-Bridge.  Also give those participants
                 the full complement of readers/writers, regardless of config.besmode, as Cloud
                 otherwise still wouldn't know where to send the data. */
              nn_log (LC_DISCOVERY, "handleParticipants: "PGIDFMT" is a ddsi2 participant\n", PGID (pdata->key));
              flags = RTPS_PF_IS_DDSI2_PP;
            }
            else
            {
              switch (config.besmode)
              {
                case BESMODE_FULL:
                  /* each participant to have all readers & writers */
                  break;
                case BESMODE_WRITERS:
                  flags |= RTPS_PF_NO_BUILTIN_READERS;
                  break;
                case BESMODE_MINIMAL:
                  flags |= RTPS_PF_NO_BUILTIN_READERS | RTPS_PF_NO_BUILTIN_WRITERS;
                  break;
              }
            }
            if (new_participant_gid (&pdata->key, &ps, flags) < 0)
              NN_WARNING1 ("participant "PGIDFMT": failed to create\n", PGID(pdata->key));
            else
              handleParticipantSubEntities (drset, setidx, &pdata->key);
            nn_plist_fini (&ps);
          }
        }
        break;
      case MTR_SST_UPD:
        /* not doing QoS updates yet */
        nn_log (LC_DISCOVERY, "handleParticipant: ignoring update\n");
        break;
    }
  }
  return U_RESULT_OK;
}

static u_result handleBuiltinTopicViaMTR (const struct builtin_datareader_set *drset, int setidx, struct mtreader *mtr, u_topic tp, u_dataReader rd, u_result (*process) (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state))
{
  v_dataReaderSample sample;
  while (reader_take_one (rd, &sample) == U_RESULT_OK && sample != NULL)
  {
    const struct mtr_sample *out[2];
    int i, nout;
    u_result ures;

    nout = update_mtreader(mtr, out, tp, v_readerSample (sample)->sampleState, v_dataReaderSampleMessage (sample));
    c_free (sample);

    for (i = 0; i < nout; i++)
    {
      thread_state_awake (lookup_thread_state());
      if ((ures = process (drset, setidx, out[i], out[i]->state)) != U_RESULT_OK)
        return ures;
    }
  }
  return U_RESULT_OK;
}

static u_result handleParticipants (const struct builtin_datareader_set *drset)
{
  u_result ures;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].participant_mtr, drset->participant_tp, drset->e[setidx].participant_dr, handleParticipant)) != U_RESULT_OK)
      return ures;
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].participant_mtr, drset->cm_participant_tp, drset->e[setidx].cm_participant_dr, handleParticipant)) != U_RESULT_OK)
      return ures;
  }
  return U_RESULT_OK;
}

struct handleParticipantsSelfActionArg {
  u_result result;
  nn_plist_t ps;
  const struct builtin_datareader_set *drset;
};

static void handleParticipantsSelfAction (v_public vpub, void *varg)
{
  struct handleParticipantsSelfActionArg *arg = varg;
  v_kernel kernel = v_objectKernel (vpub);
  v_message p_msg, cm_msg;

  arg->result = U_RESULT_INTERNAL_ERROR;

  p_msg = v_builtinCreateParticipantInfo (kernel->builtin, v_participant(vpub));
  cm_msg = v_builtinCreateCMParticipantInfo (kernel->builtin, v_participant(vpub));

  if (p_msg != NULL && cm_msg != NULL)
  {
    struct v_participantInfo const * const pdata = (struct v_participantInfo *) (p_msg + 1);
    struct v_participantCMInfo const * const cmpdata = (struct v_participantCMInfo *) (cm_msg + 1);

    nn_log (LC_DISCOVERY, "handleParticipantsSelf: found "PGIDFMT" (self)\n", PGID (pdata->key));
    assert (v_gidEqual (ddsi2_participant_gid, pdata->key));
    assert (v_gidEqual (ddsi2_participant_gid, cmpdata->key));

    if (init_participant_plist (pdata, cmpdata, &arg->ps) < 0)
      NN_WARNING1 ("participant info initialisation error for "PGIDFMT"\n", PGID (pdata->key));
    else
      arg->result = U_RESULT_OK;
  }

  c_free (cm_msg);
  c_free (p_msg);
}

static u_result handleParticipantsSelf (const struct builtin_datareader_set *drset)
{
  struct handleParticipantsSelfActionArg arg;
  u_result result;
  arg.drset = drset;
  result = u_observableAction (u_observable(participant), handleParticipantsSelfAction, &arg);
  if (result != U_RESULT_OK) {
    NN_FATAL1 ("handleParticipantsSelf: u_observableAction failed (%u)\n", (unsigned) result);
  } else if (arg.result != U_RESULT_OK) {
    NN_FATAL1 ("handleParticipantsSelf: action routine failed (%u)\n", (unsigned) arg.result);
    result = arg.result;
  } else {
    if (new_participant_gid (&ddsi2_participant_gid, &arg.ps, RTPS_PF_PRIVILEGED_PP | RTPS_PF_IS_DDSI2_PP) < 0)
    {
      NN_FATAL1 ("participant "PGIDFMT" (self): failed to create\n", PGID (ddsi2_participant_gid));
      result = U_RESULT_INTERNAL_ERROR;
    }
    nn_plist_fini (&arg.ps);
  }
  return result;
}

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
static int all_partitions_ignored (struct v_builtinPartitionPolicy partition, const char *topic)
{
  int result = 1;
  c_ulong i;
  for (i = 0; result && i < c_arraySize (partition.name); i++)
  {
    /* a partition is considered ignored if it contains no wildcards
       and does not match the ignored partition patterns */
    result = result &&
             !strchr (partition.name[i], '*') &&
             !strchr (partition.name[i], '?') &&
             is_ignored_partition (partition.name[i], topic);
  }
  return result;
}
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

static int ignore_builtin_readerwriter (const char *partition, const char *topic_name, enum ignore_builtin whatfor)
{
  /* The durability client in OpenSplice uses the same federation-specific alignment partition that is used by DDSI
   * (__NODE%08"PA_PRIx32" BUILT-IN PARTITION__). HistoricalData topics that are published on this partition
   * must NOT be ignored.
   */
  if (is_local_builtin_partition(partition) && whatfor != IGNB_ANY && (topic_name == NULL || strcmp(topic_name, "d_historicalData") != 0))
    return 1;
  if (topic_name == NULL)
    return 0;
  if ((whatfor == IGNB_READER || !config.advertise_builtin_topic_writers) && config.generate_builtin_topics && is_builtin_topic (partition, topic_name))
    return 1;
  return 0;
}

static int ignore_builtin_readerwriter_partpol (struct v_builtinPartitionPolicy partition, const char *topic_name, enum ignore_builtin whatfor)
{
  if (c_arraySize(partition.name) != 1)
    return 0;
  return ignore_builtin_readerwriter(partition.name[0], topic_name, whatfor);
}

static u_result handleDataReaderWriter_check (const v_gid *ppgid, const v_gid *group_gid, const v_gid *key, struct v_builtinPartitionPolicy partition, const char *topic_name, int isreader, v_topic *topic, nn_guid_t *ppguid, nn_guid_t *group_guid)
{
  struct participant *pp;
  *topic = NULL;

  /* only handle subscriptions that have at least 1 partition that is not ignored */
  if (v_gidEqual ((*ppgid), ddsi2_participant_gid) && strcmp (topic_name, DDSI_CONTROL_TOPIC_NAME) != 0)
  {
    /* ignore own (ddsi2 internal) readers/writers */
    return U_RESULT_PRECONDITION_NOT_MET;
  }
  if (ignore_builtin_readerwriter_partpol(partition, topic_name, isreader ? IGNB_READER : IGNB_WRITER))
  {
    nn_log (LC_DISCOVERY, "handleDataReaderWriter_check: %s: node-local partition / built-in topic ignored\n", topic_name);
    return U_RESULT_PRECONDITION_NOT_MET;
  }
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  if (all_partitions_ignored (partition, topic_name))
  {
    nn_log (LC_DISCOVERY, "handleDataReaderWriter_check: %s: ignored for all partitions, due to ignoredPartitions\n", topic_name);
    return U_RESULT_PRECONDITION_NOT_MET;
  }
#endif /* DDSI_INCLUDE_NETWORK_PARTITIONS */

  if ((*topic = v_lookupTopic (gv.ospl_kernel, topic_name)) == NULL)
  {
    nn_log (LC_DISCOVERY, "handleDataReaderWriter_check: "PGIDFMT" topic %s unknown\n", PGID (*key), topic_name);
    return U_RESULT_PRECONDITION_NOT_MET;
  }

  if (config.squash_participants)
    ppguid_from_ppgid (ppguid, &ddsi2_participant_gid);
  else
    ppguid_from_ppgid (ppguid, ppgid);

  if ((pp = ephash_lookup_participant_guid (ppguid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "handleDataReaderWriter_check: "PGIDFMT" participant "PGIDFMT" unknown\n", PGID (*key), PGID (*ppgid));
    return U_RESULT_PRECONDITION_NOT_MET;
  }

  if (v_gidIsNil (*group_gid))
    memset (group_guid, 0, sizeof (*group_guid));
  else
  {
    unsigned kind;
    kind = (isreader ? NN_ENTITYID_KIND_PRISMTECH_SUBSCRIBER : NN_ENTITYID_KIND_PRISMTECH_PUBLISHER) | NN_ENTITYID_SOURCE_VENDOR;
    map_group_gid_to_guid(group_guid, group_gid, pp, kind, 0);
  }

  return U_RESULT_OK;
}

static u_result handleDataReader (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state)
{
  struct v_subscriptionInfo const * const pdata = (struct v_subscriptionInfo *) (samp->vmsg[0] + 1);
  struct v_dataReaderCMInfo const * const cmpdata = (struct v_dataReaderCMInfo *) (samp->vmsg[1] + 1);
  nn_guid_t ppguid, group_guid;
  v_topic topic;

  if (is_proxypp_or_deleted_pp ("DataReader", &pdata->participant_key, &pdata->key, samp_state))
    return U_RESULT_OK;

  switch (samp_state)
  {
    case MTR_SST_DEL:
      delete_reader_gid (&pdata->key);
      break;
    case MTR_SST_UPD:
      nn_log (LC_DISCOVERY, "handleDataReader: ignoring update\n");
      break;
    case MTR_SST_NEW:
      if (handleDataReaderWriter_check (&pdata->participant_key, &cmpdata->subscriber_key, &pdata->key, pdata->partition, pdata->topic_name, 1, &topic, &ppguid, &group_guid) == U_RESULT_OK)
      {
        nn_xqos_t xqos;
        int res;
        update_mtreader_setflag (drset->e[setidx].datareader_mtr, &pdata->key, 1);
        if (init_reader_qos (pdata, cmpdata, &xqos, setidx != 0) < 0)
          res = ERR_INVALID_DATA;
        else
        {
          res = new_reader_gid (&ppguid, &group_guid, &cmpdata->subscriber_key, &pdata->key, topic, &xqos, cmpdata->name);
          nn_xqos_fini (&xqos);
        }
        if (res < 0)
        {
          unmap_group_gid_to_guid(&group_guid);
          if (res != ERR_ENTITY_EXISTS)
            NN_ERROR1 ("handleDataReader: new_reader: error %d\n", res);
        }
        c_free (topic);
      }
      break;
  }
  /* Intentionally ignoring errors: we log them but continue, becasue they generally aren't fatal,  */
  return U_RESULT_OK;
}

static u_result handleDataReaders (const struct builtin_datareader_set *drset)
{
  u_result ures;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].datareader_mtr, drset->subscription_tp, drset->e[setidx].subscription_dr, handleDataReader)) != U_RESULT_OK)
      return ures;
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].datareader_mtr, drset->cm_datareader_tp, drset->e[setidx].cm_datareader_dr, handleDataReader)) != U_RESULT_OK)
      return ures;
  }
  return U_RESULT_OK;
}

static void schedule_delete_writer (const v_gid *key)
{
  struct bubble_s data;
  u_result res;
  if (!bubble_writer_group_attached)
  {
    nn_log (LC_DISCOVERY, "schedule_delete_writer("PGIDFMT") deleting immediately: bubble writer's group not yet attached\n", PGID (*key));
    delete_writer_gid (key);
  }
  else
  {
    nn_log (LC_DISCOVERY, "schedule_delete_writer("PGIDFMT") writing bubble\n", PGID (*key));
    data.kind = BTK_DELETE_WRITER;
    data.systemId = key->systemId;
    data.localId = key->localId;
    data.serial = key->serial;
    if ((res = u_writerWrite (bubble_writer, bubble_writer_copy, &data, os_timeWGet(), U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    {
      NN_WARNING2 ("schedule_delete_writer("PGIDFMT") failed with result %d, deleting immediately instead\n", PGID (*key), (int) res);
      delete_writer_gid (key);
    }
  }
}

static u_result handleDataWriter (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state)
{
  struct v_publicationInfo const * const pdata = (struct v_publicationInfo *) (samp->vmsg[0] + 1);
  struct v_dataWriterCMInfo const * const cmpdata = (struct v_dataWriterCMInfo *) (samp->vmsg[1] + 1);
  nn_guid_t ppguid, group_guid;
  v_topic topic;

  if (is_proxypp_or_deleted_pp ("DataWriter", &pdata->participant_key, &pdata->key, samp_state))
    return U_RESULT_OK;

  switch (samp_state)
  {
    case MTR_SST_DEL:
      schedule_delete_writer (&pdata->key);
      break;
    case MTR_SST_UPD:
      nn_log (LC_DISCOVERY, "handleDataWriter: ignoring update\n");
      break;
    case MTR_SST_NEW:
      if (handleDataReaderWriter_check (&pdata->participant_key, &cmpdata->publisher_key, &pdata->key, pdata->partition, pdata->topic_name, 0, &topic, &ppguid, &group_guid) == U_RESULT_OK)
      {
        nn_xqos_t xqos;
        int res;
        update_mtreader_setflag (drset->e[setidx].datawriter_mtr, &pdata->key, 1);
        if (init_writer_qos (pdata, cmpdata, topic, &xqos, setidx != 0) < 0)
          res = ERR_INVALID_DATA;
        else
        {
          res = new_writer_gid (&ppguid, &group_guid, &cmpdata->publisher_key, &pdata->key, topic, &xqos, cmpdata->name);
          nn_xqos_fini (&xqos);
        }
        if (res < 0)
        {
          unmap_group_gid_to_guid(&group_guid);
          if (res != ERR_ENTITY_EXISTS)
            NN_ERROR1 ("handleDataWriter: new_writer: error %d\n", res);
        }
        c_free (topic);
      }
      break;
  }
  /* Intentionally ignoring errors: we log them but continue, becasue they generally aren't fatal,  */
  return U_RESULT_OK;
}

static u_result handleDataWriters (const struct builtin_datareader_set *drset)
{
  u_result ures;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].datawriter_mtr, drset->publication_tp, drset->e[setidx].publication_dr, handleDataWriter)) != U_RESULT_OK)
      return ures;
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].datawriter_mtr, drset->cm_datawriter_tp, drset->e[setidx].cm_datawriter_dr, handleDataWriter)) != U_RESULT_OK)
      return ures;
  }
  return U_RESULT_OK;
}

struct handleTopicReadersWriters_cb_arg {
  const struct builtin_datareader_set *drset;
  int setidx;
  char *name;
  int retries;
};

static void handleTopicReadersWriters_cb (struct xevent *xev, void *varg, nn_mtime_t tnow)
{
  struct handleTopicReadersWriters_cb_arg *arg = varg;
  u_result ures;

  os_mutexLock (&gluelock);
  if (gv.terminate || tnow.v == T_NEVER)
    ures = U_RESULT_TIMEOUT;
  else if ((ures = handleTopicReadersWriters (arg->drset, arg->setidx, arg->name)) == U_RESULT_OK)
    ;
  else if (ures != U_RESULT_PRECONDITION_NOT_MET)
    ;
  else if (--arg->retries <= 0)
    ures = U_RESULT_TIMEOUT;
  else
    resched_xevent_if_earlier (xev, add_duration_to_mtime (tnow, 100 * T_MILLISECOND));
  os_mutexUnlock (&gluelock);

  if (ures != U_RESULT_PRECONDITION_NOT_MET)
  {
    if (ures != U_RESULT_OK)
      nn_log (LC_DISCOVERY, "handleTopicReadersWriters_cb: giving up on topic %s ...\n", arg->name);
    delete_xevent (xev);
    os_free (arg->name);
    os_free (arg);
  }
}

static u_result handleTopics (const struct builtin_datareader_set *drset)
{
  v_dataReaderSample sample;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    while (reader_take_one_vsnew (drset->e[setidx].topic_dr, &sample) == U_RESULT_OK && sample != NULL)
    {
      v_message msg = v_dataReaderSampleMessage (sample);
      struct v_topicInfo const * const data = (struct v_topicInfo *) (msg + 1);
      nn_plist_t ps;

      /* See also ddsi_discovery.c: we are only forwarding topic definitions, not interpreting
         them (perhaps we will change this at some point). */
      if (!is_topic_discoverable(data->name))
        nn_log (LC_DISCOVERY, "TOPIC %s/%s: skipping built-in\n", data->name, data->type_name);
      else
      {
        nn_log (LC_DISCOVERY, "TOPIC %s/%s\n", data->name, data->type_name);
        if (init_topic_plist (data, &ps) >= 0)
        {
          struct participant *pp;
          os_mutexLock (&gv.privileged_pp_lock);
          assert (gv.privileged_pp);
          pp = gv.privileged_pp;
          os_mutexUnlock (&gv.privileged_pp_lock);
          sedp_write_topic (pp, &ps);
          nn_plist_fini (&ps);
        }
      }

      /* Once the topic is known, we can create any matching readers/writers */
      if (handleTopicReadersWriters (drset, setidx, data->name) == U_RESULT_PRECONDITION_NOT_MET)
      {
        /* Not yet known, try again in a little while */
        struct handleTopicReadersWriters_cb_arg *arg = os_malloc (sizeof (*arg));
        arg->drset = drset;
        arg->setidx = setidx;
        arg->name = os_strdup (data->name);
        arg->retries = 10;
        qxev_callback (add_duration_to_mtime (now_mt (), 10 * T_MILLISECOND), handleTopicReadersWriters_cb, arg);
      }
      c_free (sample);
    }
  }
  /* Intentionally ignoring errors: we log them but continue, becasue they generally aren't fatal,  */
  return U_RESULT_OK;
}

static int handlePublisherWriteSEDP (const void *vdata, const nn_guid_t *group_guid, int alive)
{
  struct v_publisherCMInfo const * const cmpdata = vdata;
  nn_plist_t ps;
  if (init_cm_publisher_plist (cmpdata, group_guid, &ps) >= 0)
  {
    sedp_write_cm_publisher (&ps, alive);
    nn_plist_fini (&ps);
  }
  return 0;
}

static int handleSubscriberWriteSEDP (const void *vdata, const nn_guid_t *group_guid, int alive)
{
  struct v_subscriberCMInfo const * const cmpdata = vdata;
  nn_plist_t ps;
  if (init_cm_subscriber_plist (cmpdata, group_guid, &ps) >= 0)
  {
    sedp_write_cm_subscriber (&ps, alive);
    nn_plist_fini (&ps);
  }
  return 0;
}

static u_result handlePublisherSubscriber (const struct builtin_datareader_set *drset, int setidx, const void *data, enum mtr_sample_state samp_state, int is_subscriber, unsigned kind, const v_gid *key, const v_gid *participant_key, const struct v_builtinPartitionPolicy *partition, int (*write_sedp) (const void *vdata, const nn_guid_t *group_guid, int alive))
{
  const char *which = is_subscriber ? "Subscriber" : "Publisher";

  if (is_proxypp_or_deleted_pp (which, participant_key, key, samp_state))
    return U_RESULT_OK;

  if (ignore_builtin_readerwriter_partpol(*partition, NULL, IGNB_ANY))
  {
    nn_log (LC_DISCOVERY, "handle%s "PGIDFMT": node-local partition ignored\n", which, PGID (*key));
    return U_RESULT_OK;
  }

  switch (samp_state)
  {
    case MTR_SST_DEL:
      {
        nn_guid_t group_guid;
        if (lookup_group_guid (&group_guid, key)) {
          write_sedp (data, &group_guid, 0);
          unmap_group_gid_to_guid (&group_guid);
        } else
          TRACE (("  unknown\n"));
      }
      break;
    case MTR_SST_UPD:
      nn_log (LC_DISCOVERY, "handle%s: ignoring update\n", which);
      break;
    case MTR_SST_NEW:
      {
        struct v_gid_s participant_gid = config.squash_participants ? ddsi2_participant_gid : *participant_key;
        struct participant *pp;
        nn_guid_t ppguid;
        ppguid_from_ppgid (&ppguid, &participant_gid);
        if ((pp = ephash_lookup_participant_guid (&ppguid)) != NULL)
        {
          nn_guid_t group_guid;
          map_group_gid_to_guid (&group_guid, key, pp, NN_ENTITYID_SOURCE_VENDOR | kind, 1);
          write_sedp (data, &group_guid, 1);

          if (is_subscriber)
          {
            update_mtreader_setflag (drset->e[setidx].subscriber_mtr, key, 1);
            handleSelectEntities (drset, setidx, "reader", drset->e[setidx].datareader_mtr, drset->cm_datareader_tp, "subscriber_key", gid_eq, key, handleDataReader);
          }
          else
          {
            update_mtreader_setflag (drset->e[setidx].publisher_mtr, key, 1);
            handleSelectEntities (drset, setidx, "writer", drset->e[setidx].datawriter_mtr, drset->cm_datawriter_tp, "publisher_key", gid_eq, key, handleDataWriter);
          }
        }
      }
      break;
  }
  return U_RESULT_OK;
}

static u_result handleSubscriber (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state)
{
  struct v_subscriberCMInfo const * const cmpdata = (const struct v_subscriberCMInfo *) (samp->vmsg[0] + 1);
  OS_UNUSED_ARG (setidx);
  return handlePublisherSubscriber (drset, setidx, cmpdata, samp_state, 1, NN_ENTITYID_KIND_PRISMTECH_SUBSCRIBER, &cmpdata->key, &cmpdata->participant_key, &cmpdata->partition, handleSubscriberWriteSEDP);
}

static u_result handlePublisher (const struct builtin_datareader_set *drset, int setidx, const struct mtr_sample *samp, enum mtr_sample_state samp_state)
{
  struct v_publisherCMInfo const * const cmpdata = (const struct v_publisherCMInfo *) (samp->vmsg[0] + 1);
  OS_UNUSED_ARG (setidx);
  return handlePublisherSubscriber (drset, setidx, cmpdata, samp_state, 0, NN_ENTITYID_KIND_PRISMTECH_PUBLISHER, &cmpdata->key, &cmpdata->participant_key, &cmpdata->partition, handlePublisherWriteSEDP);
}

static u_result handleSubscribers (const struct builtin_datareader_set *drset)
{
  u_result ures;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].subscriber_mtr, drset->cm_subscriber_tp, drset->e[setidx].cm_subscriber_dr, handleSubscriber)) != U_RESULT_OK)
      return ures;
  }
  return U_RESULT_OK;
}

static u_result handlePublishers (const struct builtin_datareader_set *drset)
{
  u_result ures;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    if ((ures = handleBuiltinTopicViaMTR (drset, setidx, drset->e[setidx].publisher_mtr, drset->cm_publisher_tp, drset->e[setidx].cm_publisher_dr, handlePublisher)) != U_RESULT_OK)
      return ures;
  }
  return U_RESULT_OK;
}

static void new_fictitious_transient_reader (v_group group)
{
  v_topicQos const qos = v_topicQosRef(group->topic);
  nn_xqos_t xqos;
  int res = 0;

  if (init_reader_qos_from_topicQos (qos, &xqos) < 0)
  {
    NN_WARNING2 ("new_fictitious_transient_reader: %s.%s: failed to initialize reader QoS from topic QoS\n", v_entity (group->topic)->name, v_entity (group->partition)->name);
    return;
  }

  if (xqos.durability.kind > NN_TRANSIENT_DURABILITY_QOS)
  {
    /* Limit the durability kind of the fictitious transient data
       reader so that it is left to the kernel & the durability
       service to decide how to handle transient DataWriters for a
       persistent topic. (DDS 1.2, section 7.1.3.4 explicitly says the
       fictitious readers should have the topic QoS, but that's just
       ridiculous.  Remote nodes don't know this is such a fictitious
       reader, so doing this at the transport level is not in
       violation of the spec.) */
    xqos.durability.kind = NN_TRANSIENT_DURABILITY_QOS;
  }
  xqos.present |= QP_PARTITION;
  xqos.partition.n = 1;
  xqos.partition.strs = os_malloc (sizeof (*xqos.partition.strs));
  xqos.partition.strs[0] = os_strdup (v_entity (group->partition)->name);

  nn_log (LC_DISCOVERY, "new_fictitious_transient_reader: %s.%s\n",
          v_entity (group->topic)->name, v_entity (group->partition)->name);
  /* the fictitious transient data reader has no gid, no group */
  {
    sertopic_t topic;
    if ((topic = deftopic (group->topic)) == NULL)
    {
      res = ERR_UNSPECIFIED;
    }
    else
    {
      nn_guid_t ppguid, group_guid, guid;
      memset (&group_guid, 0, sizeof (group_guid));
      ppguid_from_ppgid (&ppguid, &ddsi2_participant_gid);
      if (new_reader (&guid, &group_guid, &ppguid, topic, &xqos, NULL, NULL, NULL) == NULL)
      {
        res = ERR_UNKNOWN_ENTITY;
      }
    }
  }

  if (res == ERR_UNKNOWN_ENTITY)
    NN_FATAL0 ("new_fictitious_transient_reader: the ddsi2 participant should've been known already\n");
  else if (res < 0)
    NN_ERROR3 ("new_fictitious_transient_reader(%s.%s): new_reader: error %d - transient data may not work properly\n",
               v_entity (group->topic)->name, v_entity (group->partition)->name, res);
  nn_xqos_fini (&xqos);
}

static v_networkId getNetworkId (void)
{
  os_timeM time;
  /* NOTE: for now, let network ID be a "random"-number. This number
     has to be retrieved from the network/os layer. */
  assert (V_NETWORKID_LOCAL == 0x0);
  time = os_timeMGet();
  return (v_networkId) (OS_TIMEM_GET_NANOSECONDS(time) + 1000000000);
}

static void destroy_builtin_readers (struct builtin_datareader_set *drset)
{
  size_t i;
  for (i = 0; i < sizeof (drset->e) / sizeof (*drset->e); i++)
  {
    if (drset->e[i].participant_mtr)   delete_mtreader (drset->e[i].participant_mtr);
    if (drset->e[i].datareader_mtr)    delete_mtreader (drset->e[i].datareader_mtr);
    if (drset->e[i].datawriter_mtr)    delete_mtreader (drset->e[i].datawriter_mtr);
    if (drset->e[i].subscriber_mtr)    delete_mtreader (drset->e[i].subscriber_mtr);
    if (drset->e[i].publisher_mtr)     delete_mtreader (drset->e[i].publisher_mtr);

    if (drset->e[i].publication_dr)    u_objectFree ((u_object) drset->e[i].publication_dr);
    if (drset->e[i].subscription_dr)   u_objectFree ((u_object) drset->e[i].subscription_dr);
    if (drset->e[i].participant_dr)    u_objectFree ((u_object) drset->e[i].participant_dr);
    if (drset->e[i].topic_dr)          u_objectFree ((u_object) drset->e[i].topic_dr);
    if (drset->e[i].cm_publisher_dr)   u_objectFree ((u_object) drset->e[i].cm_publisher_dr);
    if (drset->e[i].cm_subscriber_dr)  u_objectFree ((u_object) drset->e[i].cm_subscriber_dr);
    if (drset->e[i].cm_datareader_dr)  u_objectFree ((u_object) drset->e[i].cm_datareader_dr);
    if (drset->e[i].cm_datawriter_dr)  u_objectFree ((u_object) drset->e[i].cm_datawriter_dr);
    if (drset->e[i].cm_participant_dr) u_objectFree ((u_object) drset->e[i].cm_participant_dr);
    if (drset->e[i].ddsi_control_dr)   u_objectFree ((u_object) drset->e[i].ddsi_control_dr);

    if (drset->e[i].subscriber)        u_objectFree ((u_object) drset->e[i].subscriber);
  }
  if (drset->ddsi_control_tp)   u_objectFree ((u_object) drset->ddsi_control_tp);
  if (drset->participant_tp)    u_objectFree ((u_object) drset->participant_tp);
  if (drset->cm_participant_tp) u_objectFree ((u_object) drset->cm_participant_tp);
  if (drset->subscription_tp)   u_objectFree ((u_object) drset->subscription_tp);
  if (drset->cm_datareader_tp)  u_objectFree ((u_object) drset->cm_datareader_tp);
  if (drset->publication_tp)    u_objectFree ((u_object) drset->publication_tp);
  if (drset->cm_datawriter_tp)  u_objectFree ((u_object) drset->cm_datawriter_tp);
  if (drset->topic_tp)          u_objectFree ((u_object) drset->topic_tp);
  if (drset->cm_publisher_tp)   u_objectFree ((u_object) drset->cm_publisher_tp);
  if (drset->cm_subscriber_tp)  u_objectFree ((u_object) drset->cm_subscriber_tp);
}

static u_result create_builtin_readers (struct builtin_datareader_set *drset, u_participant p)
{
#define OFFS(name) \
    offsetof (struct builtin_datareader_set1, name##_dr), \
    offsetof (struct builtin_datareader_set, name##_tp)
  static const struct tptab {
    const char *name;
    int is_transient;
    int proper_defaults;
    int needs_systemId_filter;
    size_t dr_off;
    size_t tp_off;
  } tptab[] = {
    { DDSI_CONTROL_TOPIC_NAME,  0, 1, 0, OFFS (ddsi_control) },
    { V_PARTICIPANTINFO_NAME,   1, 0, 1, OFFS (participant) },
    { V_CMPARTICIPANTINFO_NAME, 1, 0, 1, OFFS (cm_participant) },
    { V_SUBSCRIPTIONINFO_NAME,  1, 0, 1, OFFS (subscription) },
    { V_CMDATAREADERINFO_NAME,  1, 0, 1, OFFS (cm_datareader) },
    { V_PUBLICATIONINFO_NAME,   1, 0, 1, OFFS (publication) },
    { V_CMDATAWRITERINFO_NAME,  1, 0, 1, OFFS (cm_datawriter) },
    { V_TOPICINFO_NAME,         1, 0, 0, OFFS (topic) },
    { V_CMPUBLISHERINFO_NAME,   1, 0, 1, OFFS (cm_publisher) },
    { V_CMSUBSCRIBERINFO_NAME,  1, 0, 1, OFFS (cm_subscriber) }
  };
#undef OFFS
  int need_systemId_filter = 0;
  v_readerQos rdQos = NULL;
  v_subscriberQos sQos = NULL;
  v_gid gid;
  c_value ps[1];
  c_char ns_part[U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE];
  v_durabilityKind durqos[2];
  const char *parts[2];
  size_t nparts;
  u_result result;
  size_t i, j;

  for (j = 0; j < sizeof (tptab) / sizeof (*tptab); j++)
  {
    *((u_topic *) ((char *) drset + tptab[j].tp_off)) = NULL;
    for (i = 0; i < sizeof (parts) / sizeof (*parts); i++)
      *((u_dataReader *) ((char *) &drset->e[i] + tptab[j].dr_off)) = NULL;
  }
  for (i = 0; i < sizeof (parts) / sizeof (*parts); i++)
  {
    drset->e[i].subscriber = NULL;
    drset->e[i].participant_mtr = NULL;
    drset->e[i].datareader_mtr = NULL;
    drset->e[i].datawriter_mtr = NULL;
    drset->e[i].subscriber_mtr = NULL;
    drset->e[i].publisher_mtr = NULL;
  }

  if ((result = u_participantFederationSpecificPartitionName (participant, ns_part, sizeof (ns_part))) != U_RESULT_OK)
    return result;

  /* If creating a special built-in reader set for looking up the ddsi2 participant, we only need one participant. If somehow the local_discovery_partition is configured to be the same as the federation-specific partition name, we also need only one */
  nparts = (strcmp (config.local_discovery_partition, ns_part) == 0) ? 1 : 2;
  assert (nparts <= sizeof (parts) / sizeof (*parts));

  /* "Normal" built-in topics readers are in __BUILT-IN PARTITION__ and filter on systemId, because they should only respond to entities created locally. The ones in the __NODEx BUILT-IN PARTITION__ are (currently only) used by RnR to create "fake" readers with strange GIDs, where the systemId can be some other node. */
  Q_STATIC_ASSERT_CODE (sizeof (parts) / sizeof (*parts) == sizeof (drset->e) / sizeof (*drset->e));
  Q_STATIC_ASSERT_CODE (sizeof (durqos) / sizeof (*durqos) == sizeof (drset->e) / sizeof (*drset->e));
  parts[0] = config.local_discovery_partition;
  durqos[0] = (strcmp (config.local_discovery_partition, ns_part) == 0) ? V_DURABILITY_VOLATILE : V_DURABILITY_TRANSIENT;
  parts[1] = ns_part;
  durqos[1] = V_DURABILITY_VOLATILE;

  /* We need a filter on systemId in "normal" mode where we mirror only the entities within our own federation, as that is the simplest way of ignoring all remote entities. When in bridging mode, we need to see entities from all federation in the domain (so no filter on systemId). In the latter case, if we are lurking in the built-in partition, we still need to suppress the built-in topics for entities we created ourselves but that we do by checking against our known proxy participants */
  if (strcmp (parts[0], V_BUILTIN_PARTITION) == 0)
  {
    TRACE (("create_builtin_readers: using built-in partition for discovery, "));
    need_systemId_filter = (config.mirror_remote_entities != BOOLDEF_TRUE);
  }
  else
  {
    TRACE (("create_builtin_readers: using special partition for discovery, "));
    need_systemId_filter = (config.mirror_remote_entities == BOOLDEF_FALSE);
  }
  if (need_systemId_filter)
  {
    TRACE (("mirroring only local entities\n"));
    check_all_proxy_participants = 0;
  }
  else
  {
    TRACE (("mirroring all entities\n"));
    check_all_proxy_participants = 1;
  }

  /* Create subscribers */
  for (i = 0; i < nparts; i++)
  {
    char name[32];
    TRACE (("create_builtin_readers: subscriber in partition %s\n", parts[i]));
    if ((sQos = u_subscriberQosNew (NULL)) == NULL)
      goto fail;
    sQos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
    if ((sQos->partition.v = os_strdup (parts[i])) == NULL)
      goto fail;
    snprintf (name, sizeof (name), "DDSI2BuiltinSubscriber%d", (int) i);
    if ((drset->e[i].subscriber = u_subscriberNew (p, name, sQos)) == NULL)
      goto fail;
    if(u_entityEnable(u_entity(drset->e[i].subscriber)) != U_RESULT_OK)
      goto fail;
    u_subscriberQosFree (sQos);
    sQos = NULL;
  }

  /* Parameter list for content filters is a constant, whether or not it is used for a reader */
  gid = u_observableGid ((u_observable) drset->e[0].subscriber);
  ps[0].kind = V_ULONG;
  ps[0].is.ULong = gid.systemId;
  TRACE (("create_builtin_readers: systemId = %lx\n", (unsigned long) ps[0].is.ULong));

  /* Reader QoS is mostly constant; durability kind is dependent on the partition */
  if ((rdQos = u_readerQosNew (NULL)) == NULL)
    goto fail;
  rdQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
  rdQos->history.v.kind = V_HISTORY_KEEPLAST;
  rdQos->history.v.depth = 1;

  /* Create readers, lookup topics */
  for (j = 0; j < sizeof (tptab) / sizeof (*tptab); j++)
  {
    /* Control topic may be disabled */
    if (!config.enable_control_topic && strcmp (tptab[j].name, DDSI_CONTROL_TOPIC_NAME) == 0)
      continue;

    if (tptab[j].proper_defaults)
    {
      rdQos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
      rdQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    }
    else
    {
      rdQos->liveliness.v.lease_duration = OS_DURATION_ZERO;
      rdQos->reliability.v.max_blocking_time = OS_DURATION_ZERO;
    }

    /* Topic lookup */
    {
      c_iter topics_iter = u_participantFindTopic (participant, tptab[j].name, 0);
      u_topic *tp_ptr = (u_topic *) ((char *) drset + tptab[j].tp_off);
      *tp_ptr = c_iterTakeFirst (topics_iter);
      if (*tp_ptr == NULL)
      {
        NN_FATAL1 ("Could not find topic %s.\n", tptab[j].name);
        c_iterFree (topics_iter);
        goto fail;
      }
      assert (c_iterLength (topics_iter) == 0);
      c_iterFree (topics_iter);
    }

    for (i = 0; i < nparts; i++)
    {
      /* Durability for the "real" DCPSSubscription is differs from that
       of the one (currently) used by R&R, hence why the durability
       qos is set so late. */
      rdQos->durability.v.kind = tptab[j].is_transient ? durqos[i] : V_DURABILITY_VOLATILE;

      /* Data reader */
      {
        u_dataReader *pdr = (u_dataReader *) ((char *) &drset->e[i] + tptab[j].dr_off);
        char expr[128];
        char rdname[32];
        int n;

        if (i == 0 && need_systemId_filter && tptab[j].needs_systemId_filter)
          n = snprintf (expr, sizeof (expr), "select * from %s where key.systemId = %%0", tptab[j].name);
        else
          n = snprintf (expr, sizeof (expr), "select * from %s", tptab[j].name);
        assert (n < (int) sizeof (expr));
        (void) n;

        n = snprintf (rdname, sizeof (rdname), "%sReader%d", tptab[j].name, (int) i);
        assert (n < (int) sizeof (expr));
        (void) n;

        if ((*pdr = u_subscriberCreateDataReader (drset->e[i].subscriber, rdname, expr, ps, 1, rdQos)) == NULL)
          goto fail;
        if(u_entityEnable(u_entity(*pdr)) != U_RESULT_OK)
          goto fail;
      }
    }
  }
  u_readerQosFree (rdQos);

  /* Now that we have the topics, we can also create the sets of mtreaders */
  for (i = 0; i < nparts; i++)
  {
    u_topic tps[2];
    tps[0] = drset->participant_tp;
    tps[1] = drset->cm_participant_tp;
    drset->e[i].participant_mtr = new_mtreader (2, tps);
    tps[0] = drset->subscription_tp;
    tps[1] = drset->cm_datareader_tp;
    drset->e[i].datareader_mtr = new_mtreader (2, tps);
    tps[0] = drset->publication_tp;
    tps[1] = drset->cm_datawriter_tp;
    drset->e[i].datawriter_mtr = new_mtreader (2, tps);
    tps[0] = drset->cm_subscriber_tp;
    drset->e[i].subscriber_mtr = new_mtreader (1, tps);
    tps[0] = drset->cm_publisher_tp;
    drset->e[i].publisher_mtr = new_mtreader (1, tps);
  }
  return U_RESULT_OK;

 fail:
  destroy_builtin_readers (drset);
  if (rdQos)
    u_readerQosFree (rdQos);
  if (sQos)
    u_subscriberQosFree (sQos);
  return U_RESULT_INTERNAL_ERROR;
}

static v_copyin_result bubble_writer_copy (UNUSED_ARG (c_type type), const void *data, void *to)
{
  memcpy (to, data, sizeof (struct bubble_s));
  return V_COPYIN_RESULT_OK;
}

static u_result create_bubble_topic_writer (u_participant p)
{
  v_publisherQos pqos = NULL;
  v_writerQos wrqos;
  u_topicQos tqos;

  /* For backwards compatibility, set the topic QoS with historical lease_duration and max_blocking_time */
  if ((tqos = u_topicQosNew (NULL)) == NULL)
    goto err_topicqos;
  tqos->liveliness.v.lease_duration = OS_DURATION_ZERO;
  tqos->reliability.v.max_blocking_time = OS_DURATION_ZERO;
  if ((bubble_topic = u_topicNew (p, BUBBLE_TOPIC_NAME, "q_osplserModule::bubble", "", tqos)) == NULL)
    goto err_topic;
  if ((bubble_kernel_topic = v_lookupTopic (gv.ospl_kernel, BUBBLE_TOPIC_NAME)) == NULL)
    goto err_kernel_topic;

  if ((pqos = u_publisherQosNew (NULL)) == NULL)
    goto err_pqos;
  os_free (pqos->partition.v);
  pqos->partition.v = os_strdup (V_BUILTIN_PARTITION);
  if ((bubble_publisher = u_publisherNew (p, "ddsi2 bubble publisher", pqos, TRUE)) == NULL)
    goto err_publisher;

  if ((wrqos = u_writerQosNew (NULL)) == NULL)
    goto err_wrqos;
  wrqos->reliability.v.kind = V_RELIABILITY_RELIABLE;
  wrqos->history.v.kind = V_HISTORY_KEEPALL;
  if ((bubble_writer = u_writerNew (bubble_publisher, "ddsi2 bubble writer", bubble_topic, wrqos)) == NULL)
    goto err_writer;
  u_entityEnable(u_entity(bubble_writer));
  bubble_writer_gid = u_observableGid (u_observable(bubble_writer));
  nn_log (LC_DISCOVERY, "bubble writer: gid "PGIDFMT"\n", PGID(bubble_writer_gid));

  u_writerQosFree (wrqos);
  u_publisherQosFree (pqos);
  u_topicQosFree (tqos);
  return U_RESULT_OK;

 err_writer:
  u_writerQosFree (wrqos);
 err_wrqos:
  u_objectFree (u_object (bubble_publisher));
 err_publisher:
  u_publisherQosFree (pqos);
 err_pqos:
  c_free (bubble_kernel_topic);
 err_kernel_topic:
  u_objectFree (u_object (bubble_topic));
 err_topic:
  u_topicQosFree (tqos);
err_topicqos:
  return U_RESULT_INTERNAL_ERROR;
}

static void destroy_bubble_topic_writer (void)
{
  u_objectFree (u_object (bubble_writer));
  u_objectFree (u_object (bubble_publisher));
  u_objectFree (u_object (bubble_topic));
  c_free (bubble_kernel_topic);
}

static u_result create_ddsi_control_topic (u_participant p)
{
  u_topicQos tqos;

  /* For backwards compatibility, set the topic QoS with historical lease_duration and max_blocking_time */
  if ((tqos = u_topicQosNew (NULL)) == NULL)
    goto err_topicqos;
  tqos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
  tqos->reliability.v.kind = V_RELIABILITY_RELIABLE;
  tqos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
  if ((ddsi_control_topic = u_topicNew (p, DDSI_CONTROL_TOPIC_NAME, "q_osplserModule::ddsi_control", "systemId,localId,serial", tqos)) == NULL)
    goto err_topic;
  u_topicQosFree (tqos);
  return U_RESULT_OK;

err_topic:
  u_topicQosFree (tqos);
err_topicqos:
  return U_RESULT_INTERNAL_ERROR;
}

static struct thread_state1 * create_channel_reader_thread
(
  const char * name,
  struct builtin_datareader_set * drset,
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  struct config_channel_listelem *chptr
#else
  ddsi_tran_conn_t transmit_conn
#endif
)
{
  /* create one (or more, eventually) threads to read from the network
     queue and transmit the data */
  struct channel_reader_arg *arg = os_malloc (sizeof (struct channel_reader_arg));
  char *thread_name = os_malloc (strlen ("xmit.") + strlen (name) + 1);
  struct thread_state1 *ts;
  sprintf (thread_name, "xmit.%s", name);
  arg->drset = drset;
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  arg->channel_cfg = chptr;
  arg->transmit_conn = chptr->transmit_conn;
#else
  arg->transmit_conn = transmit_conn;
#endif
  if ((ts = create_thread (
               thread_name,
               (void * (*) (void *)) channel_reader_thread,
               arg)) == NULL)
    NN_ERROR1 ("creation of network queue monitoring thread %s failed\n", thread_name);
  os_free (thread_name);
  return ts;
}

static void lease_renew_cb (void *vparticipant)
{
  const c_float leaseSec = config.servicelease_expiry_time;
  u_serviceRenewLease (u_service (vparticipant), os_durationMul(OS_DURATION_SECOND, leaseSec));
}

static os_result
exit_request_handler(
    UNUSED_ARG(os_callbackArg ignore),
    UNUSED_ARG(void *callingThreadContext),
    UNUSED_ARG(void * arg))
{
  gv.terminate = TRUE;
  os_mutexLock (&gluelock);
  if (local_discovery_waitset)
    u_waitsetNotify (local_discovery_waitset, NULL);
  if (networkReader) {
#ifndef DDSI_INCLUDE_NETWORK_CHANNELS
    if (gv.networkQueueId)
      u_networkReaderTrigger (networkReader, gv.networkQueueId);
#else
    struct config_channel_listelem *chptr = config.channels;
    while (chptr != NULL)
    {
      if (chptr->queueId)
        u_networkReaderTrigger (networkReader, chptr->queueId);
      chptr = chptr->next;
    }
#endif
  }
  os_mutexUnlock (&gluelock);
  return os_resultSuccess; /* the main thread will take care of termination */
}

static os_result exception_request_handler (
        UNUSED_ARG(void * callingThreadContext), UNUSED_ARG(void * arg))
{
  c_bool ret = TRUE;
  gv.exception = TRUE;
  if (!gv.terminate) {
      ret = u_serviceChangeState (u_service (participant), STATE_DIED);
  }

  return ret ? os_resultSuccess : os_resultFail;
}

void ddsi_impl_init (void)
{
  ddsi_plugin.init_fn = osplser_init;
  ddsi_plugin.fini_fn = osplser_fini;
}

static int debmon_plugin_mtr (ddsi_tran_conn_t conn, debug_monitor_cpf_t cpf, void *varg)
{
  struct thread_state1 *self = lookup_thread_state ();
  struct builtin_datareader_set *drset = varg;
  int setidx;
  int x = 0;
  os_mutexLock (&gluelock);
  x += cpf (conn, "MTR:\n");
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (drset->e[0])); setidx++)
  {
    struct builtin_datareader_set1 *dr1 = &drset->e[setidx];
    struct mtr_iter it;
    const struct mtr_sample *s;
    thread_state_awake (self);
    for (s = mtr_first (dr1->participant_mtr, &it); s; s = mtr_next (&it))
    {
      nn_guid_t ppguid;
      int ispp, isproxypp;

      ppguid_from_ppgid(&ppguid, &s->gid);
      ispp = (ephash_lookup_participant_guid (&ppguid) != NULL);
      isproxypp = (ephash_lookup_proxy_participant_guid (&ppguid) != NULL);
      x += cpf (conn, "  "PGIDFMT" (%s) %d%d ", PGID (s->gid), mtr_sample_state_str (s->state), ispp, isproxypp);
      switch (s->state)
      {
        case MTR_SST_DEL:
          if (ispp || isproxypp)
            x += cpf (conn, "EXISTS");
          break;
        case MTR_SST_NEW:
        case MTR_SST_UPD:
          if (!(ispp || isproxypp))
            x += cpf (conn, "NON-EXISTENT");
      }
      x += cpf (conn, "\n");
    }
    thread_state_asleep (self);
  }
  os_mutexUnlock (&gluelock);
  return x;
}

OPENSPLICE_SERVICE_ENTRYPOINT (ospl_ddsi2e, ddsi2e)
{
  /* Exit status: 0: ok, 1: configuration error, 2: abnormal
     termination. Default to abnormal termination, then override if
     that is inappriorate */
  int exitstatus = 2;
  /* According to the specification of os_timeGetPowerevents()
     time zero has to be provided to retrieve the current power
     events. Unfortuntely, the power event interface uses os_time
     while the rest of ddsi uses os_uint64 as time representation.
     Only when the power events interface is changed the use os_time
     may go also. */
  const os_duration zeroTime = OS_DURATION_ZERO;

  /* Default service name and service URI, taken from command line if
     given; config stores the service name in case it is needed at
     some later stage. */
  const char *service_name = "ddsi2e";
  const char *service_uri = NULL;
  struct builtin_datareader_set drset;
  struct debug_monitor *debmon = NULL;
  u_waitset disc_ws;
  nn_mtime_t reset_deaf_mute_time = { T_NEVER };

  /* Init static log buffer early as possible -- but we don't even
     have a lock yet.  This is ok if we are certain the log functions
     can only be called from the ddsi2 main thread until the mutex
     initialization, which is currently the case.  */
  gv.static_logbuf_lock_inited = 0;
  gv.exception = FALSE;
  gv.terminate = FALSE;
  gv.deaf = FALSE;
  gv.mute = FALSE;
  logbuf_init (&gv.static_logbuf);

  if (argc > 1)
    service_name = argv[1];
  if (argc > 2)
    service_uri = argv[2];
  /* This field is used in NN_ERRORx, so is temporarily initialised for early errors */
  config.servicename = (char*)service_name;

#ifdef EVAL_V
  OS_REPORT(OS_INFO,"q_main", 0,
            "++++++++++++++++++++++++++++++" OS_REPORT_NL
            "++ ddsi2 EVALUATION VERSION ++" OS_REPORT_NL
            "++++++++++++++++++++++++++++++\n");
#endif

  /* Necessary to initialize the user layer. Do this just once per process.*/
  if (u_userInitialise () != U_RESULT_OK) {
    NN_ERROR0 ("initialisation of user layer failed\n");
    goto err_userInitialise;
  }

  /* now init the lock */
  if (os_mutexInit (&gv.static_logbuf_lock, NULL) != os_resultSuccess) {
    NN_ERROR0 ("initialisation of local administration mutex failed\n");
    goto err_static_logbuf_lock;
  }
  gv.static_logbuf_lock_inited = 1;

  /* Init glue lock + cond, used for some internal synchronisation */
  if (os_mutexInit (&gluelock, NULL) != os_resultSuccess) {
    NN_ERROR0 ("initialisation of local administration mutex failed\n");
    goto err_gluelock;
  }
  if (os_condInit (&gluecond, &gluelock, NULL) != os_resultSuccess) {
    NN_ERROR0 ("initialisation of local administration cond.var failed\n");
    goto err_gluecond;
  }

  /* Create participant, notify kernel that I am initializing, and
     start monitoring the splicedaemon state, so we can terminate if
     he says so. */
  {
#if DDSI2E_OR_NOT2E
    v_serviceType serviceType = V_SERVICETYPE_DDSI2E;
#else
    v_serviceType serviceType = V_SERVICETYPE_DDSI2;
#endif
    u_participantQos participantQos;
    if ((participantQos = u_participantQosNew (NULL)) == NULL) {
      NN_ERROR0 ("allocation of participantQos failed\n");
      goto err_participant;
    }
    if ((participant = u_participant (u_networkingNew(service_uri, U_DOMAIN_ID_ANY, 0, service_name, serviceType, participantQos, TRUE))) == NULL) {
      NN_ERROR0 ("creation of participant failed\n");
      u_participantQosFree (participantQos);
      goto err_participant;
    }
    u_participantQosFree (participantQos);
  }
  ddsi2_participant_gid = u_observableGid (u_observable(participant));
  u_serviceChangeState (u_service (participant), STATE_INITIALISING);
  u_serviceWatchSpliceDaemon (u_service (participant), watch_spliced, NULL);

  determine_kernel_service (participant);
  gv.myNetworkId = getNetworkId ();
  gv.ospl_qostype = (c_collectionType) c_metaArrayTypeNew (c_metaObject (gv.ospl_base), "C_ARRAY<c_octet>", c_octet_t (gv.ospl_base), 0);

  {
    c_type tid_type;
    tid_type = c_resolve (gv.ospl_base, "kernelModule::v_tid");
    assert (tid_type != NULL);
    gv.ospl_eotgroup_tidlist_type = (c_collectionType) c_metaSequenceTypeNew (c_metaObject (gv.ospl_base), "C_SEQUENCE<v_tid>", tid_type, 0);
    c_free (tid_type);
  }

  /* Parse configuration & open tracing file -- can't do the latter
     any earlier cos the configuration specifies where to write it
     to.

     Strictly speaking, failure to load the configuration can mean we
     ran out of memory (and possibly other weird things), but by far
     the most likely cause is an user error. So, until we decide we
     should be very precise in these matters, we assume a user error
     and exit with status 1 to indicate a configuration error.  */
  {
    struct cfgst *cfgst;
    if ((cfgst = q_config_init (participant, service_name)) == NULL) {
      /* This field is used in NN_ERRORx, but is freed on failed q_config_init, so
       * it is temporarily initialised for errors in configuration initialisation
       * error reporting. */
      config.servicename = (char*) service_name;
      NN_ERROR0 ("Could not initialise configuration\n");
      exitstatus = 1;
      goto err_q_config_init;
    }

    if (config.lease_duration == 0)
    {
      os_int64 xt = (os_int64) (1e9 * config.servicelease_expiry_time);
      if (xt >= 10 * T_SECOND)
        config.lease_duration = xt + 1 * T_SECOND;
      else
        config.lease_duration = xt + xt / 10;
    }

    if (rtps_config_prep (cfgst) < 0)
    {
      NN_ERROR0 ("Could not initialise configuration\n");
      exitstatus = 1;
      goto err_config_late_error;
    }

    upgrade_main_thread ();
  }

  /* Allow configuration to set "deaf_mute" in case we want to start out that way */
  if (config.enable_control_topic) {
    gv.deaf = config.initial_deaf;
    gv.mute = config.initial_mute;
    if (gv.deaf || gv.mute) {
      TRACE (("DEAFMUTE initial deaf=%d mute=%d reset after %"PA_PRId64" ns\n", gv.deaf, gv.mute, config.initial_deaf_mute_reset));
      reset_deaf_mute_time = add_duration_to_mtime (now_mt (), config.initial_deaf_mute_reset);
    }
  }

  /* Initialize the power events. Any power event that happenend before the
   * initialization will not be recognized. Preferable we want to intialize
   * as quick but it has no use to initialize the power events before the
   * configuration has been parsed successfully because no remote
   * communication has started. */
  (void) os_timeGetPowerEvents(&gv.powerEvents, zeroTime);

  /* Start monitoring the liveliness of all threads and renewing the
     service lease if everything seems well. */
  if ((gv.servicelease = nn_servicelease_new (lease_renew_cb, participant)) == NULL) {
    NN_ERROR0 ("initialisation of service lease management failed\n");
    goto err_servicelease;
  }
  if (nn_servicelease_start_renewing (gv.servicelease) < 0) {
    NN_ERROR0 ("startup of thread monitoring and service lease renewal failed\n");
    goto err_servicelease_start;
  }

  /* Discovery starts as soon as we call rtps_init(), so we need our
     built-in writers setup very early on */
  if (create_builtin_topic_writers (participant) != U_RESULT_OK)
    goto err_builtin_topic_writers;

  /* Start-up DDSI proper (RTPS + discovery) */
  if (rtps_init () < 0)
    goto err_rtps_init;

    if (reset_deaf_mute_time.v < T_NEVER)
    {
      qxev_callback(reset_deaf_mute_time, reset_deaf_mute, 0);
    }


  /* Prepare hash table for mapping GUIDs to entities */
  if ((gid_hash = ephash_new (config.gid_hash_softlimit)) == NULL)
    goto err_gid_hash;

  /* Create subscriber, network reader to receive messages to be transmitted.
     The queue's will be created by the channel_reader_threads*/
  {
    v_subscriberQos subscriberQos;
    os_duration resolution;
    if ((subscriberQos = u_subscriberQosNew (NULL)) == NULL) {
      NN_ERROR0 ("allocation of subscriberQos for message subscriber failed\n");
      goto err_client_subscriber;
    }
    os_free (subscriberQos->partition.v);
    subscriberQos->partition.v = NULL;

    if ((networkSubscriber = u_subscriberNew (participant, "networkSubscriber", subscriberQos)) == NULL) {
      NN_ERROR0 ("creation of message subscriber failed\n");
      u_subscriberQosFree (subscriberQos);
      goto err_client_subscriber;
    }
    u_subscriberQosFree (subscriberQos);

    if (u_entityEnable (u_entity(networkSubscriber)) != U_RESULT_OK) {
      NN_ERROR0 ("enabling of message subscriber failed\n");
      goto err_client_subscriber_enable;
    }

    if ((networkReader = u_networkReaderNew (networkSubscriber, "networkReader", NULL, TRUE)) == NULL) {
      NN_ERROR0 ("creation of network reader failed\n");
      goto err_networkReaderNew;
    }

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
    {
      struct config_channel_listelem *chptr = config.channels;
      while (chptr) {
        /* Kernel expects an unsigned, we do a signed following the DDS spec. */
        assert (chptr->queue_size > 0);
        assert (chptr->priority >= 0);
        assert (chptr->resolution >= T_MILLISECOND && chptr->resolution <= T_SECOND);
        resolution = (os_duration) chptr->resolution;
        if (u_networkReaderCreateQueue (networkReader, chptr->queue_size, (os_uint32) chptr->priority, FALSE, FALSE, resolution, FALSE, &chptr->queueId, "DDSI2E") != U_RESULT_OK) {
          NN_ERROR0 ("creation of network reader queue failed\n");
          goto err_networkReaderCreateQueue;
        }
        chptr = chptr->next;
      }
    }
#else
    resolution = OS_DURATION_SECOND;
    if (u_networkReaderCreateQueue (networkReader, config.nw_queue_size, 0, FALSE, FALSE, resolution, FALSE, &gv.networkQueueId, "DDSI2") != U_RESULT_OK) {
      NN_ERROR0 ("creation of network reader queue failed\n");
      goto err_networkReaderCreateQueue;
    }
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */
  }

  /* create GID <-> GUID mapping for implicit group discovery */
  if (os_mutexInit (&group_gid_guid_lock, NULL) != os_resultSuccess) {
    NN_ERROR0 ("initialization of group gid-guid mapping lock failed\n");
    goto err_group_gid_guid_lock;
  }
  ut_avlInit (&group_gid_guid_td, &group_gid_guid_tree);
  ut_avlInit (&group_guid_gid_td, &group_guid_gid_tree);

  /* create control topic */
  if (config.enable_control_topic)
  {
    if (create_ddsi_control_topic (participant) != U_RESULT_OK) {
      NN_ERROR0 ("creation of ddsi control topic failed\n");
      goto err_create_ddsi_control_topic;
    }
  }

  /* create data readers for the builtin topics we use for local
     discovery ... */
  if (create_builtin_readers (&drset, participant) != U_RESULT_OK) {
    NN_ERROR0 ("creation of builtin topic readers failed\n");
    goto err_create_builtin_readers;
  }
  /* ... and a waitset for all events local discovery cares about */
  if ((disc_ws = create_discovery_waitset (participant, &drset)) == NULL) {
    NN_ERROR0 ("creation of local discovery waitset failed\n");
    goto err_create_discovery_waitset;
  }
  os_mutexLock (&gluelock);
  local_discovery_waitset = disc_ws;
  os_mutexUnlock (&gluelock);

  /* create a bubble_topic writer */
  if (create_bubble_topic_writer (participant) != U_RESULT_OK)
    goto err_bubble_topic_writer;

  /* perform initial local discovery so we get the fewest 'unknown
     writers' or late group entry creations */
  if (initial_local_discovery (&drset) != U_RESULT_OK)
  {
    NN_ERROR0 ("initial local discovery failed\n");
    goto err_initial_local_discovery;
  }

  /* Can't continue until attached to bubble writer's group. We may,
     during local discovery, observe the disappearance of writers,
     even though we can't use the bubble writer yet. Therefore, we
     check bubble_writer_group_attached in schedule_delete_writer(),
     and accept that writers that disappear so soon are of no
     consequence. */
  os_mutexLock (&gluelock);
  if (!bubble_writer_group_attached)
  {
    TRACE (("waiting for discovery of bubble writer's group\n"));
    while (!bubble_writer_group_attached)
      os_condWait (&gluecond, &gluelock);
  }
  os_mutexUnlock (&gluelock);
  TRACE (("attached to bubble writer's group, continuing\n"));

#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  {
    struct config_channel_listelem *chptr = config.channels;
    int num_channels = 0;
    while (chptr != NULL)
    {
      num_channels++;
      if ((chptr->channel_reader_ts = create_channel_reader_thread (chptr->name, &drset, chptr)) == NULL)
        goto err_channel_reader_thread;
      chptr = chptr->next;
    }
    nn_log (LC_INFO, "%d channels configured\n", num_channels);
  }
#else

  TRACE (("transmit port %d\n", (int) ddsi_tran_port (gv.data_conn_uc)));
  if ((gv.channel_reader_ts = create_channel_reader_thread ("user", &drset, gv.data_conn_uc)) == NULL)
  {
    goto err_channel_reader_thread;
  }
#endif /* DDSI_INCLUDE_NETWORK_CHANNELS */

  if (config.monitor_port >= 0)
  {
    debmon = new_debug_monitor (config.monitor_port);
    add_debug_monitor_plugin (debmon, debmon_plugin_mtr, &drset);
  }

  /* Mirror local entities in DDSI until requested to stop */
  {
    u_result uresult;
    os_signalHandlerExitRequestHandle erh = os_signalHandlerExitRequestHandleNil;
    os_signalHandlerExceptionHandle eh = os_signalHandlerExceptionHandleNil;

    u_serviceChangeState (u_service (participant), STATE_OPERATIONAL);

    if (!os_serviceGetSingleProcess()){
        erh = os_signalHandlerRegisterExitRequestCallback(exit_request_handler, NULL, NULL, NULL, NULL);
        eh = os_signalHandlerRegisterExceptionCallback(exception_request_handler, NULL, NULL, NULL, NULL);
    }

    uresult = monitor_local_entities (local_discovery_waitset, &drset);

    os_signalHandlerUnregisterExceptionCallback(eh);
    os_signalHandlerUnregisterExitRequestCallback(erh);

    /* when exception occurred then wait here to prevent process exit */
    if (gv.exception) {
      os_sem_t wait_for_ever;
      if (os_sem_init(&wait_for_ever, 0) == os_resultSuccess) {
        os_sem_wait(&wait_for_ever);
      } else {
        os_duration t = OS_DURATION_INIT(100, 0);
        ospl_os_sleep(t);
      }
    }

    if ((uresult != U_RESULT_OK) && (uresult != U_RESULT_DETACHING))
    {
      NN_ERROR0 ("abnormal termination\n");
      goto err_monitor;
    }
  }
  exitstatus = 0;

 err_monitor:
  /* Trigger the transmit thread draining the network queue the
     OpenSplice way, though this may incur data loss.  It would
     arguably be better to send a bubble through, that would guarantee
     that at least all data in the queue at the time of the
     termination notification would be transmitted.  But we don't
     actually guarantee reliability once terminating anyway.  */
  gv.terminate = TRUE;
  rtps_term_prep ();
  if (debmon)
    free_debug_monitor (debmon);
 err_channel_reader_thread:
  u_waitsetNotify (local_discovery_waitset, NULL);
#ifdef DDSI_INCLUDE_NETWORK_CHANNELS
  {
    struct config_channel_listelem *chptr = config.channels;
    while (chptr != NULL)
    {
      u_networkReaderTrigger (networkReader, chptr->queueId);
      if (chptr->channel_reader_ts)
        join_thread (chptr->channel_reader_ts, NULL);
      chptr = chptr->next;
    }
  }
#else
  u_networkReaderTrigger (networkReader, gv.networkQueueId);
  if (gv.channel_reader_ts)
    join_thread (gv.channel_reader_ts, NULL);
#endif
  os_mutexLock (&gluelock);
  local_discovery_waitset = NULL;
  os_mutexUnlock (&gluelock);
err_initial_local_discovery:
  destroy_bubble_topic_writer ();
err_bubble_topic_writer:
  destroy_discovery_waitset (disc_ws, participant, &drset);
err_create_discovery_waitset:
  destroy_builtin_readers (&drset);
err_create_builtin_readers:
  if (config.enable_control_topic)
    u_objectFree (ddsi_control_topic);
err_create_ddsi_control_topic:
  /* note: group_guid_gid_tree aliases group_gid_guid_tree */
  ut_avlFree (&group_gid_guid_td, &group_gid_guid_tree, os_free);
  os_mutexDestroy (&group_gid_guid_lock);
err_group_gid_guid_lock:
err_networkReaderCreateQueue:
  /* fugly code to avoid an even nastier race condition with
     watch_spliced */
  os_mutexLock (&gluelock);
  u_objectFree (u_object (networkReader));
  networkReader = NULL;
  os_mutexUnlock (&gluelock);
err_networkReaderNew:
  /* No undo for enable needed */
err_client_subscriber_enable:
  u_objectFree (u_object (networkSubscriber));
err_client_subscriber:
  ephash_free (gid_hash);
err_gid_hash:
  rtps_term ();
err_rtps_init:
  destroy_builtin_topic_writers ();
err_builtin_topic_writers:
err_servicelease_start:
  nn_servicelease_free (gv.servicelease);
err_servicelease:
  downgrade_main_thread ();
  thread_states_fini ();
err_config_late_error:
  /* would expect q_config_fini() here, but no, it is postponed. */
err_q_config_init:
  c_free (gv.ospl_eotgroup_tidlist_type);
  c_free (gv.ospl_qostype);
  /* Service mgmt framework doesn't allow INITIALISING -> TERMINATED */
  if (exitstatus != 0)
    u_serviceChangeState (u_service (participant), STATE_TERMINATING);
  u_serviceChangeState (u_service (participant), STATE_TERMINATED);
  if (u_objectFree_s (u_object (u_service (participant))) != U_RESULT_OK)
    NN_ERROR0 ("deletion of participant failed\n");
err_participant:
  os_condDestroy (&gluecond);
err_gluecond:
  os_mutexDestroy (&gluelock);
 err_gluelock:
  nn_log (LC_INFO, "Finis.\n");

  /* Must be really late, or nn_log becomes really unhappy -- but it
     should be before os_osExit (which appears to be called from
     u_userExit(), which is not called by u_userDetach but by an exit
     handler, it appears.) */
  q_config_fini ();
  os_mutexDestroy (&gv.static_logbuf_lock);
err_static_logbuf_lock:
err_userInitialise:
  return exitstatus;
}
