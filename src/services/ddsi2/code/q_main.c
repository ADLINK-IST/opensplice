/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include <assert.h>

#include "os.h"
#include "os_defs.h"
#include "os_mutex.h"
#include "os_socket.h"

#include "u_user.h"
#include "u_domain.h"
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_publisher.h"
#include "u_dataReader.h"
#include "u_writer.h"
#include "u_waitset.h"
#include "u_waitsetEvent.h"
#include "u_networkReader.h"
#include "u_topic.h"

#include "v_networkReaderEntry.h"
#include "v_public.h"
#include "c_iterator.h"
#include "v_event.h"
#include "v_writer.h"
#include "v_observer.h"
#include "v_reader.h"
#include "v_readerSample.h"
#include "v_dataReaderSample.h"
#include "v_public.h"
#include "v_topic.h"
#include "v_builtin.h"
#include "v_service.h"
#include "v_state.h"
#include "v_group.h"
#include "v_networkQueue.h"

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
#include "q_static_assert.h"

#include "ddsi_tran.h"

static u_participant participant = NULL;
static v_service service = NULL;
static u_subscriber networkSubscriber = NULL;
static u_networkReader networkReader = NULL;
static v_networkReader vnetworkReader = NULL;
static u_waitset local_discovery_waitset = NULL;
static os_mutex gluelock;
static os_cond gluecond;
static v_gid ddsi2_participant_gid;
static struct ephash *gid_hash;

static const char *bubble_topic_name = "q_bubble";
static u_topic bubble_topic;
static v_topic bubble_kernel_topic;
static u_publisher bubble_publisher;
static u_writer bubble_writer;
static v_gid bubble_writer_gid;
static int bubble_writer_group_attached;

struct builtin_datareader_set1 {
  u_subscriber subscriber;
  u_dataReader participant_dr;
  u_dataReader subscription_dr;
  u_dataReader publication_dr;
};

struct builtin_datareader_set {
  struct builtin_datareader_set1 e[2];
  c_long participant_off;
  c_long publication_off;
  c_long subscription_off;
};

struct channel_reader_arg
{
  struct builtin_datareader_set *drset;
  ddsi_tran_conn_t transmit_conn;
};

/* Having any tentative participant at all is really rare, so slightly
   inefficient expiry handling is ok */
struct tentative_participant {
  ut_avlNode_t avlnode;
  v_gid key;
  os_int64 t_expiry;
};

static ut_avlTree_t tentative_participants;
static struct xevent *tentative_participants_cleanup_event;

static int compare_gid (const v_gid *a, const v_gid *b);
static const ut_avlTreedef_t tentative_participants_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct tentative_participant, avlnode), offsetof (struct tentative_participant, key), (int (*) (const void *, const void *)) compare_gid, 0);

struct q_globals gv;

static u_result handleParticipants (const struct builtin_datareader_set *drset);
static u_result handleSubscriptions (const struct builtin_datareader_set *drset);
static u_result handlePublications (const struct builtin_datareader_set *drset);
static u_result handleGroups (v_service service);

static void new_fictitious_transient_reader (v_group group);

static int compare_gid (const v_gid *a, const v_gid *b)
{
  return memcmp (a, b, sizeof (v_gid));
}

static void ppguid_from_ppgid (nn_guid_t *ppguid, const struct v_gid_s *ppgid)
{
  ppguid->prefix.u[0] = ppgid->systemId;
  ppguid->prefix.u[1] = ppgid->localId;
  ppguid->prefix.u[2] = ppgid->serial;
  ppguid->entityid.u = NN_ENTITYID_PARTICIPANT;
}

static int new_participant_gid (const struct v_gid_s *ppgid, unsigned flags)
{
  nn_guid_t ppguid;
  ppguid_from_ppgid (&ppguid, ppgid);
  return new_participant_guid (&ppguid, flags);
}

static int delete_participant_gid (const struct v_gid_s *ppgid)
{
  nn_guid_t ppguid;
  ppguid_from_ppgid (&ppguid, ppgid);
  return delete_participant (&ppguid);
}

static int new_writer_gid (const struct v_gid_s *ppgid, const struct v_gid_s *gid, C_STRUCT (v_topic) const * const ospl_topic, const struct nn_xqos *xqos)
{
  nn_guid_t ppguid, guid;
  topic_t topic;
  int res;

  assert (ospl_topic != NULL);

  if (ephash_lookup_writer_gid (gid_hash, gid))
  {
    TRACE (("new_writer(gid %x:%x:%x) - already known\n",
            gid->systemId, gid->localId, gid->serial));
    return ERR_ENTITY_EXISTS;
  }

  TRACE (("new_writer(gid %x:%x:%x)\n", gid->systemId, gid->localId, gid->serial));

  if ((topic = deftopic (ospl_topic, NULL)) == NULL)
    return ERR_UNSPECIFIED;

  ppguid_from_ppgid (&ppguid, ppgid);
  if ((res = new_writer (&guid, &ppguid, topic, xqos)) >= 0)
  {
    struct writer *wr = ephash_lookup_writer_guid (&guid);
    assert (wr);
    wr->c.gid = *gid;
    ephash_insert_writer_gid (gid_hash, wr);
  }
  return res;
}

int delete_writer_gid (const struct v_gid_s *gid)
{
  struct writer *wr;
  assert (v_gidIsValid (*gid));
  if ((wr = ephash_lookup_writer_gid (gid_hash, gid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_writer(gid %x:%x:%x) - unknown gid\n",
            gid->systemId, gid->localId, gid->serial);
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_writer(gid %x:%x:%x) ...\n", gid->systemId, gid->localId, gid->serial);
  ephash_remove_writer_gid (gid_hash, wr);
  delete_writer (&wr->e.guid);
  return 0;
}

static int new_reader_gid (const struct v_gid_s *ppgid, const struct v_gid_s *gid, C_STRUCT (v_topic) const * const ospl_topic, const struct nn_xqos *xqos)
{
  /* see new_writer for comments */
  nn_guid_t ppguid, guid;
  topic_t topic;
  int res;

  assert (ospl_topic != NULL);

  if (ephash_lookup_reader_gid (gid_hash, gid))
  {
    TRACE (("new_reader(gid %x:%x:%x) - already known\n",
            gid->systemId, gid->localId, gid->serial));
    return ERR_ENTITY_EXISTS;
  }

  TRACE (("new_reader(gid %x:%x:%x)\n", gid->systemId, gid->localId, gid->serial));

  if ((topic = deftopic (ospl_topic, NULL)) == NULL)
    return ERR_UNSPECIFIED;

  ppguid_from_ppgid (&ppguid, ppgid);
  if ((res = new_reader (&guid, &ppguid, topic, xqos, 0, 0)) >= 0)
  {
    struct reader *rd = ephash_lookup_reader_guid (&guid);
    assert (rd);
    rd->c.gid = *gid;
    ephash_insert_reader_gid (gid_hash, rd);
  }
  return res;
}

int delete_reader_gid (const struct v_gid_s *gid)
{
  /* FIXME: NEED TO SERIALIZE THIS STUFF -- OR MAYBE NOT (NOW THAT GLUELOCK HAS BEEN REINSTATED) */
  struct reader *rd;
  assert (v_gidIsValid (*gid));
  if ((rd = ephash_lookup_reader_gid (gid_hash, gid)) == NULL)
  {
    nn_log (LC_DISCOVERY, "delete_reader_gid(gid %x:%x:%x) - unknown gid\n",
            gid->systemId, gid->localId, gid->serial);
    return ERR_UNKNOWN_ENTITY;
  }
  nn_log (LC_DISCOVERY, "delete_reader_gid(gid %x:%x:%x) ...\n", gid->systemId, gid->localId, gid->serial);
  ephash_remove_reader_gid (gid_hash, rd);
  return delete_reader (&rd->e.guid);
}

#if 0 /* may be useful for suppressing built-ins when generating them from SPDP/SEDP */
static c_bool in_discoveryisBuiltinGroup (c_char* partition, c_char* topic)
{
  c_bool result = FALSE;
  assert(partition);
  assert(topic);

  if(strcmp(partition, V_BUILTIN_PARTITION) == 0){
    if( (strcmp(topic, V_PARTICIPANTINFO_NAME) == 0) ||
        (strcmp(topic, V_TOPICINFO_NAME) == 0) ||
        (strcmp(topic, V_PUBLICATIONINFO_NAME) == 0) ||
        (strcmp(topic, V_SUBSCRIPTIONINFO_NAME) == 0))
    {
      result = TRUE;
    }
  }
  return result;
}
#endif

static nn_duration_t c_time_to_ddsi_duration (c_time t)
{
  if (c_timeIsInfinite (t))
    return nn_to_ddsi_duration (T_NEVER);
  else
  {
    os_int64 tt;
    tt = t.seconds * T_SECOND + t.nanoseconds;
    return nn_to_ddsi_duration (tt);
  }
}

static void qcp_reliability (nn_xqos_t *xqos, const struct v_reliabilityPolicy *reliability)
{
  xqos->present |= QP_RELIABILITY;
  switch (reliability->kind)
  {
    case V_RELIABILITY_BESTEFFORT:
      xqos->reliability.kind = NN_BEST_EFFORT_RELIABILITY_QOS;
      break;
    case V_RELIABILITY_RELIABLE:
      xqos->reliability.kind = NN_RELIABLE_RELIABILITY_QOS;
      xqos->reliability.max_blocking_time = c_time_to_ddsi_duration (reliability->max_blocking_time);
      break;
  }
}

static void qcp_history (nn_xqos_t *xqos, const struct v_historyPolicy *history)
{
  xqos->present |= QP_HISTORY;
  switch (history->kind)
  {
    case V_HISTORY_KEEPLAST:
      xqos->history.kind = NN_KEEP_LAST_HISTORY_QOS;
      xqos->history.depth = history->depth;
      if (xqos->history.depth <= 0)
      {
        NN_WARNING1 ("kernel history qos with depth %d encountered, using 1 instead\n",
                     (int) xqos->history.depth);
        xqos->history.depth = 1;
      }
      break;
    case V_HISTORY_KEEPALL:
      xqos->history.kind = NN_KEEP_ALL_HISTORY_QOS;
      xqos->history.depth = 1;
      break;
  }
}

static void qcp_durability (nn_xqos_t *xqos, const struct v_durabilityPolicy *durability)
{
  xqos->present |= QP_DURABILITY;
  switch (durability->kind)
  {
    case V_DURABILITY_VOLATILE:
      xqos->durability.kind = NN_VOLATILE_DURABILITY_QOS;
      break;
    case V_DURABILITY_TRANSIENT_LOCAL:
      xqos->durability.kind = NN_TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case V_DURABILITY_TRANSIENT:
      xqos->durability.kind = NN_TRANSIENT_DURABILITY_QOS;
      break;
    case V_DURABILITY_PERSISTENT:
      xqos->durability.kind = NN_PERSISTENT_DURABILITY_QOS;
      break;
  }
}

static void qcp_ownership (nn_xqos_t *xqos, const struct v_ownershipPolicy *ownership)
{
  xqos->present |= QP_OWNERSHIP;
  switch (ownership->kind)
  {
    case V_OWNERSHIP_SHARED:
      xqos->ownership.kind = NN_SHARED_OWNERSHIP_QOS;
      break;
    case V_OWNERSHIP_EXCLUSIVE:
      xqos->ownership.kind = NN_EXCLUSIVE_OWNERSHIP_QOS;
      break;
  }
}

static void qcp_ownership_strength (nn_xqos_t *xqos, const struct v_strengthPolicy *strength)
{
  xqos->present |= QP_OWNERSHIP_STRENGTH;
  xqos->ownership_strength.value = strength->value;
}

static void qcp_presentation (nn_xqos_t *xqos, const struct v_presentationPolicy *presentation)
{
  xqos->present |= QP_PRESENTATION;
  switch (presentation->access_scope)
  {
    case V_PRESENTATION_INSTANCE:
      xqos->presentation.access_scope = NN_INSTANCE_PRESENTATION_QOS;
      break;
    case V_PRESENTATION_TOPIC:
      xqos->presentation.access_scope = NN_TOPIC_PRESENTATION_QOS;
      break;
    case V_PRESENTATION_GROUP:
      xqos->presentation.access_scope = NN_GROUP_PRESENTATION_QOS;
      break;
  }
  xqos->presentation.coherent_access = (presentation->coherent_access != 0);
  xqos->presentation.ordered_access = (presentation->ordered_access != 0);
}

static void qcp_destination_order (nn_xqos_t *xqos, const struct v_orderbyPolicy *orderby)
{
  if (orderby)
  {
    xqos->present |= QP_DESTINATION_ORDER;
    switch (orderby->kind)
    {
      case V_ORDERBY_RECEPTIONTIME:
        xqos->destination_order.kind = NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
      case V_ORDERBY_SOURCETIME:
        xqos->destination_order.kind = NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
    }
  }
}

static void qcp_partition (nn_xqos_t *xqos, const struct v_builtinPartitionPolicy *ps)
{
  int i;
  xqos->present |= QP_PARTITION;
  xqos->partition.n = c_arraySize (ps->name);
  xqos->partition.strs = os_malloc (xqos->partition.n * sizeof (*xqos->partition.strs));
  for (i = 0; i < xqos->partition.n; i++)
    xqos->partition.strs[i] = os_strdup (ps->name[i]);
}

static void qcp_deadline (nn_xqos_t *xqos, const struct v_deadlinePolicy *a)
{
  xqos->present |= QP_DEADLINE;
  xqos->deadline.deadline = c_time_to_ddsi_duration (a->period);
}

static void qcp_latency (nn_xqos_t *xqos, const struct v_latencyPolicy *a)
{
  xqos->present |= QP_LATENCY_BUDGET;
  xqos->latency_budget.duration = c_time_to_ddsi_duration (a->duration);
}

static void qcp_liveliness (nn_xqos_t *xqos, const struct v_livelinessPolicy *a)
{
  xqos->present |= QP_LIVELINESS;
  switch (a->kind)
  {
    case V_LIVELINESS_AUTOMATIC:
      xqos->liveliness.kind = NN_AUTOMATIC_LIVELINESS_QOS;
      break;
    case V_LIVELINESS_PARTICIPANT:
      xqos->liveliness.kind = NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
      break;
    case V_LIVELINESS_TOPIC:
      xqos->liveliness.kind = NN_MANUAL_BY_TOPIC_LIVELINESS_QOS;
      break;
  }
#if 0 /* this causes problems with the builtin topics --
         alternatively, treat 0-duration leases as infinite in all
         lease processing. */
  if (c_timeIsZero (a->lease_duration))
    /* A lease duration of 0 can't work, and it is the value the
       kernel sets it to by default. It therefore seems reasonable to
       interpret it as infinite */
    xqos->liveliness.lease_duration = nn_to_ddsi_duration (T_NEVER);
  else
    xqos->liveliness.lease_duration = c_time_to_ddsi_duration (a->lease_duration);
#else
  xqos->liveliness.lease_duration = c_time_to_ddsi_duration (a->lease_duration);
#endif
}

static void qcp_lifespan (nn_xqos_t *xqos, const struct v_lifespanPolicy *a)
{
  xqos->present |= QP_LIFESPAN;
  xqos->lifespan.duration = c_time_to_ddsi_duration (a->duration);
}

static void qcp_time_based_filter (nn_xqos_t *xqos, const struct v_pacingPolicy *a)
{
  xqos->present |= QP_TIME_BASED_FILTER;
  xqos->time_based_filter.minimum_separation = c_time_to_ddsi_duration (a->minSeperation);
}

static void qcp_rdlifespan (nn_xqos_t *xqos, const struct v_readerLifespanPolicy *a)
{
  if (a->used)
  {
    xqos->present |= QP_LIFESPAN;
    xqos->lifespan.duration = c_time_to_ddsi_duration (a->duration);
  }
}

static void qcp_wrlifecycle (nn_xqos_t *xqos, const struct v_writerLifecyclePolicy *a)
{
  xqos->present |= QP_PRISMTECH_WRITER_DATA_LIFECYCLE;
  xqos->writer_data_lifecycle.autodispose_unregistered_instances =
    (a->autodispose_unregistered_instances != 0);
}

static void qcp_rdlifecycle (nn_xqos_t *xqos, const struct v_readerLifecyclePolicy *a)
{
  xqos->present |= QP_PRISMTECH_READER_DATA_LIFECYCLE;
  xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay =
    c_time_to_ddsi_duration (a->autopurge_nowriter_samples_delay);
  xqos->reader_data_lifecycle.autopurge_disposed_samples_delay =
    c_time_to_ddsi_duration (a->autopurge_disposed_samples_delay);
}

static void qcp_resource_limits (nn_xqos_t *xqos, const struct v_resourcePolicy *a)
{
  xqos->present |= QP_RESOURCE_LIMITS;
  xqos->resource_limits.max_samples = a->max_samples;
  xqos->resource_limits.max_instances = a->max_instances;
  xqos->resource_limits.max_samples_per_instance = a->max_samples_per_instance;
}

static void qcp_transport_priority (nn_xqos_t *xqos, const struct v_transportPolicy *a)
{
  xqos->present |= QP_TRANSPORT_PRIORITY;
  xqos->transport_priority.value = a->value;
}

static void qcp_user_data (nn_xqos_t *xqos, const struct v_builtinUserDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_USER_DATA;
    xqos->user_data.length = c_arraySize (a->value);
    xqos->user_data.value = os_malloc (xqos->user_data.length);
    memcpy (xqos->user_data.value, a->value, xqos->user_data.length);
  }
}

static void qcp_topic_data (nn_xqos_t *xqos, const struct v_builtinTopicDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_TOPIC_DATA;
    xqos->topic_data.length = c_arraySize (a->value);
    xqos->topic_data.value = os_malloc (xqos->topic_data.length);
    memcpy (xqos->topic_data.value, a->value, xqos->topic_data.length);
  }
}

static void qcp_topic_data_topic (nn_xqos_t *xqos, const struct v_topicDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_TOPIC_DATA;
    xqos->topic_data.length = c_arraySize (a->value);
    xqos->topic_data.value = os_malloc (xqos->topic_data.length);
    memcpy (xqos->topic_data.value, a->value, xqos->topic_data.length);
  }
}

static void qcp_group_data (nn_xqos_t *xqos, const struct v_builtinGroupDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_GROUP_DATA;
    xqos->group_data.length = c_arraySize (a->value);
    xqos->group_data.value = os_malloc (xqos->group_data.length);
    memcpy (xqos->group_data.value, a->value, xqos->group_data.length);
  }
}

static int qcp_specials (nn_xqos_t *xqos, const struct v_builtinUserDataPolicy *a)
{
  sd_serializer ser;
  sd_serializedData serdata;
  c_type type;
  char *value;
  struct name_value *nv;
  int i, sz, rc;

  /* Must at least be null-terminated ... */
  sz = (int) c_arraySize (a->value);
  if (sz == 0)
    return 0;
  if (sz < 0 || (value = os_malloc (sz + 1)) == NULL)
    return -1;
  memcpy (value, a->value, sz);
  value[sz] = 0;

  /* ... and if it is, try deserialising it as XML for a sequence of
     name-value pairs ... */
  type = c_resolve (gv.ospl_base, "q_osplserModule::seq_name_value");
  ser = sd_serializerXMLNewTyped (type);
  serdata = sd_serializerFromString (ser, value);
  nv = sd_serializerDeserializeValidated (ser, serdata);
  sd_serializedDataFree (serdata);
  rc = (nv == NULL || sd_serializerLastValidationResult (ser) != SD_VAL_SUCCESS) ? -1 : 0;
  sd_serializerFree (ser);
  c_free (type);
  os_free (value);
  if (rc < 0)
    return rc;

  /* ... then see if we can find the magic word ... */
  sz = c_arraySize ((c_array) nv);
  for (i = 0; i < sz; i++)
  {
    if (strcmp (nv[i].name, "RelaxedQosMatching") == 0)
    {
      xqos->present |= QP_PRISMTECH_RELAXED_QOS_MATCHING;
      if (strcmp (nv[i].value, "true") == 0)
        xqos->relaxed_qos_matching.value = 1;
      else if (strcmp (nv[i].value, "false") == 0)
        xqos->relaxed_qos_matching.value = 0;
      else
      {
        NN_WARNING2 ("qcp_specials: %s: %s invalid value\n", (char *) nv[i].name, (char *) nv[i].value);
        rc = -1;
        break;
      }
      TRACE (("qcp_specials: relaxed_qos_matching = %d\n", xqos->relaxed_qos_matching));
    }
  }
  c_free (nv);
  return rc;
}

static int init_reader_qos (const struct v_subscriptionInfo *data, nn_xqos_t *xqos, int interpret_user_data)
{
  v_public kpub;

  /* We do what we can with the builtin topic ... */
  nn_xqos_init_empty (xqos);
  qcp_durability (xqos, &data->durability);
  qcp_deadline (xqos, &data->deadline);
  qcp_latency (xqos, &data->latency_budget);
  qcp_liveliness (xqos, &data->liveliness);
  qcp_reliability (xqos, &data->reliability);
  qcp_ownership (xqos, &data->ownership);
  qcp_destination_order (xqos, &data->destination_order);
  qcp_user_data (xqos, &data->user_data);
  qcp_time_based_filter (xqos, &data->time_based_filter);
  qcp_presentation (xqos, &data->presentation);
  qcp_partition (xqos, &data->partition);
  qcp_topic_data (xqos, &data->topic_data);
  qcp_group_data (xqos, &data->group_data);
  qcp_rdlifespan (xqos, &data->lifespan);

  /* The maximum localId normally generated is (2^22)-1 (4096 * 1024).
   * Subscription with relaxes_qos_matching enabled (received through
   * the federation-local partition) don't relate to real readers.
   * These have a localId with a value greater than (2^22)-1. */
  if((v_gidLocalId(data->key) & 0xFFC00000) == 0) {
    /* ... but for some we (sadly) need the gory details ... ideally,
       this data would be obtained using u_entityQos, but since we have
       a gid rather than a user object, we have to go straight to the
       kernel (or so it seems). */
    if (v_gidClaimChecked (data->key, gv.ospl_kernel, &kpub) != V_HANDLE_OK)
    {
      nn_log (LC_DISCOVERY, "init_reader_qos: gidClaimChecked(gid %x:%x:%x) failed\n", data->key.systemId, data->key.localId, data->key.serial);
    }
    else
    {
      /* v_entityQos returns a reference, which really means a race
         condition with v_entitySetQos, so we do it the "wrong" way
         until that issue is fixed. */
#define v_readerEntrySetLock(_this) c_mutexLock(&(_this)->entrySet.mutex)
#define v_readerEntrySetUnlock(_this) c_mutexUnlock(&(_this)->entrySet.mutex)
      v_reader krd = v_reader (kpub);
      v_readerEntrySetLock (krd);

      qcp_history (xqos, &krd->qos->history);
      qcp_rdlifecycle (xqos, &krd->qos->lifecycle);
      qcp_resource_limits (xqos, &krd->qos->resource);
      v_readerEntrySetUnlock (krd);
      v_gidRelease (data->key, gv.ospl_kernel);
#undef v_readerEntrySetUnlock
#undef v_readerEntrySetLock
    }
  }

  if (interpret_user_data)
  {
    if (qcp_specials (xqos, &data->user_data) < 0)
    {
      nn_xqos_fini (xqos);
      return -1;
    }
  }

  return 0;
}

static int init_writer_qos (const struct v_publicationInfo *data, nn_xqos_t *xqos, int interpret_user_data)
{
  /* see also init_reader_qos() */
  v_public kpub;

  nn_xqos_init_empty (xqos);
  qcp_durability (xqos, &data->durability);
  qcp_deadline (xqos, &data->deadline);
  qcp_latency (xqos, &data->latency_budget);
  qcp_liveliness (xqos, &data->liveliness);
  qcp_reliability (xqos, &data->reliability);
  qcp_lifespan (xqos, &data->lifespan);
  qcp_destination_order (xqos, &data->destination_order);
  qcp_user_data (xqos, &data->user_data);
  qcp_ownership (xqos, &data->ownership);
  qcp_ownership_strength (xqos, &data->ownership_strength);
  qcp_presentation (xqos, &data->presentation);
  qcp_partition (xqos, &data->partition);
  qcp_topic_data (xqos, &data->topic_data);
  qcp_group_data (xqos, &data->group_data);
  qcp_wrlifecycle (xqos, &data->lifecycle);

  if (v_gidClaimChecked (data->key, gv.ospl_kernel, &kpub) != V_HANDLE_OK)
  {
    nn_log (LC_DISCOVERY, "init_writer_qos: gidClaimChecked(gid %x:%x:%x) failed\n", data->key.systemId, data->key.localId, data->key.serial);
  }
  else
  {
    v_writer kwr = v_writer (kpub);
    v_observerLock (v_observer (kwr));

    qcp_history (xqos, &kwr->qos->history);
    qcp_resource_limits (xqos, &kwr->qos->resource);
    qcp_transport_priority (xqos, &kwr->qos->transport);

    v_observerUnlock (v_observer (kwr));
    v_gidRelease (data->key, gv.ospl_kernel);
  }

  if (interpret_user_data)
  {
    if (qcp_specials (xqos, &data->user_data) < 0)
    {
      nn_xqos_fini (xqos);
      return -1;
    }
  }

  return 0;
}

static void handle_bubble (C_STRUCT (v_message) const * msg)
{
  const struct bubble_s *data = C_DISPLACE (msg, bubble_kernel_topic->dataField->offset);
  v_gid gid;
  switch (data->kind)
  {
    case BTK_DELETE_WRITER:
      gid.systemId = data->systemId;
      gid.localId = data->localId;
      gid.serial = data->serial;
      nn_log (LC_DISCOVERY, "handle_bubble: delete_writer(%x:%x:%x)\n", gid.systemId, gid.localId, gid.serial);
      delete_writer_gid (&gid);
      break;
    default:
      NN_FATAL1 ("ddsi2: handle_bubble: kind %d unknown\n", (int) data->kind);
      break;
  }
}

int rtps_write (struct nn_xpack *xp, const struct v_gid_s *wrgid, C_STRUCT (v_message) const *msg)
{
  serdata_t serdata;
  struct writer *wr;

  if ((wr = ephash_lookup_writer_gid (gid_hash, wrgid)) == NULL)
  {
    TRACE (("rpts_write(gid %x:%x:%x) - unknown gid\n", wrgid->systemId, wrgid->localId, wrgid->serial));
    return ERR_UNKNOWN_ENTITY;
  }

  /* Can't handle all node states ... though I do believe this is the
   only set we'll ever get. Note: wr->topic is constant, so no need
   to hold the lock while serializing.  */
  switch (v_nodeState ((v_message) msg))
  {
    case L_WRITE:
    case L_WRITE | L_DISPOSED:
      if ((serdata = serialize (gv.serpool, wr->topic, msg)) == NULL)
      {
        NN_WARNING0 ("serialization (data) failed\n");
        return ERR_UNSPECIFIED;
      }
      break;
    case L_DISPOSED:
    case L_UNREGISTER:
      if ((serdata = serialize_key (gv.serpool, wr->topic, msg)) == NULL)
      {
        NN_WARNING0 ("serialization (key) failed\n");
        return ERR_UNSPECIFIED;
      }
      break;
    case L_REGISTER:
      /* DDSI has no notion of "register" messages */
      return 0;
    default:
      NN_WARNING1 ("rtps_write: unhandled message state: %u\n", (unsigned) v_nodeState ((v_message) msg));
      return ERR_UNSPECIFIED;
  }
#ifndef NDEBUG
  if ((config.enabled_logcats & LC_TRACE) && (v_nodeState ((v_message) msg) & L_WRITE))
    assert (serdata_verify (serdata, msg));
#endif

  return write_sample_kernel_seq (xp, wr, serdata, 1, msg->sequenceNumber);
}

static void *channel_reader_thread (struct channel_reader_arg *arg)
{
  struct thread_state1 *self = lookup_thread_state ();
  v_networkQueue vnetworkQueue = NULL;
  struct nn_xpack *xp;

  xp = nn_xpack_new (arg->transmit_conn);


  while (!gv.terminate)
  {
    const c_ulong queueId = gv.networkQueueId;
    v_networkReaderWaitResult nrwr;
    c_bool sendTo, more;
    v_message message;
    c_ulong sequenceNumber, priority;
    v_gid sender, receiver;
    c_time sendBefore;
    v_networkReaderEntry entry;

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
            /* retry after checking for new publications */
            os_mutexLock (&gluelock);
            (void) handlePublications (arg->drset);
            if (rtps_write (xp, &sender, message) == ERR_UNKNOWN_ENTITY)
              nn_log (LC_TRACE, "message dropped because sender %x:%x:%x is unknown\n",
                      sender.systemId, sender.localId, sender.serial);
            os_mutexUnlock (&gluelock);
          }
          c_free (message);
        }
      } while (more);
      nn_xpack_send (xp);
      thread_state_asleep (self);
    }
    else if ((nrwr & V_WAITRESULT_TRIGGERED) == V_WAITRESULT_TRIGGERED)
    {
      TRACE (("transmit_thread %p: stopping as requested\n", self));
      break;
    }
  }
  nn_xpack_free (xp);
  os_free (arg);
  return NULL;
}

static void determine_kernel_service_helper (v_entity entity, void *varg)
{
  v_service *s = varg;
  *s = (v_service) entity;
}

static void determine_kernel_networkReader_helper (v_entity entity, void *varg)
{
  v_networkReader *nwr = varg;
  *nwr = (v_networkReader) entity;
}

static v_service determine_kernel_service (u_participant participant)
{
  v_service s;
  u_entityAction (u_entity (participant), determine_kernel_service_helper, &s);
  return s;
}

static v_networkReader determine_kernel_networkReader (u_networkReader reader)
{
  v_networkReader nwr;
  u_entityAction (u_entity (reader), determine_kernel_networkReader_helper, &nwr);
  return nwr;
}

static void watch_spliced (v_serviceStateKind spliceDaemonState, UNUSED_ARG (void *usrData))
{
  switch (spliceDaemonState)
  {
    case STATE_TERMINATING:
    case STATE_TERMINATED:
    case STATE_DIED:
      nn_log (LC_INFO, "splice daemon is terminating and so am I...\n");
      gv.terminate = TRUE;
      u_serviceChangeState (u_service (participant), STATE_TERMINATING);
      os_mutexLock (&gluelock);
      if (local_discovery_waitset)
        u_waitsetNotify (local_discovery_waitset, NULL);
      if (vnetworkReader)
        v_networkReaderTrigger (vnetworkReader, 0);
      os_mutexUnlock (&gluelock);
      break;
    default:
      break;
  }
}

static u_waitset create_discovery_waitset (u_participant participant, struct builtin_datareader_set *drset)
{
  u_dataReader readers[3 * (sizeof (drset->e) / sizeof (*drset->e))];
  u_waitset waitset;
  int i;

  for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++)
  {
    readers[3*i + 0] = drset->e[i].participant_dr;
    readers[3*i + 1] = drset->e[i].publication_dr;
    readers[3*i + 2] = drset->e[i].subscription_dr;
  }

  if ((waitset = u_waitsetNew (participant)) == NULL) {
    NN_ERROR0 ("could not create waitset\n");
    goto err_waitsetNew;
  }
  if (u_waitsetSetEventMask (waitset, V_EVENT_DATA_AVAILABLE | V_EVENT_NEW_GROUP | V_EVENT_SERVICESTATE_CHANGED | V_EVENT_TRIGGER) != U_RESULT_OK) {
    NN_ERROR0 ("could not set event mask of waitset\n");
    goto err_waitsetSetEventMask;
  }

  if (u_dispatcherSetEventMask ((u_dispatcher) participant, V_EVENT_NEW_GROUP | V_EVENT_SERVICESTATE_CHANGED) != U_RESULT_OK) {
    NN_ERROR0 ("could not set event mask of participant\n");
    goto err_dispatchSetEventMask;
  }

  v_serviceFillNewGroups (service);

  if (u_waitsetAttach (waitset, (u_entity) participant, (u_entity) participant) != U_RESULT_OK) {
    NN_ERROR0 ("could not attach participant to waitset\n");
    goto err_waitsetAttach;
  }

  for (i = 0; i < (int) (sizeof (readers) / sizeof (*readers)); i++)
  {
    u_dataReader dr = readers[i];
    if (u_dispatcherSetEventMask ((u_dispatcher) dr, V_EVENT_DATA_AVAILABLE) != U_RESULT_OK) {
      NN_ERROR0 ("could not set event mask of data reader\n");
      break;
    }
    if (u_waitsetAttach (waitset, (u_entity) dr, (u_entity) dr) != U_RESULT_OK) {
      NN_ERROR0 ("could not attach data reader to waitset\n");
      break;
    }
  }
  if (i != (int) (sizeof (readers) / sizeof (*readers)))
  {
    while (i--)
      u_waitsetDetach (waitset, (u_entity) readers[i]);
    goto err_attach_readers;
  }

  return waitset;

 err_attach_readers:
  u_waitsetDetach (waitset, u_entity (participant));
 err_waitsetAttach:
 err_dispatchSetEventMask:
 err_waitsetSetEventMask:
  u_waitsetFree (waitset);
 err_waitsetNew:
  return NULL;
}

static void destroy_discovery_waitset (u_waitset waitset, u_participant participant, struct builtin_datareader_set *drset)
{
  int i;
  for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++) {
    u_waitsetDetach (waitset, (u_entity) drset->e[i].participant_dr);
    u_waitsetDetach (waitset, (u_entity) drset->e[i].publication_dr);
    u_waitsetDetach (waitset, (u_entity) drset->e[i].subscription_dr);
  }
  u_waitsetDetach (waitset, u_entity (participant));
  u_waitsetFree (waitset);
}

static u_result handleGroup (v_group group)
{
  char * partition = v_entity (group->partition)->name;
  char * topic = v_entity (group->topic)->name;

  {
    v_networkReaderEntry entry;

    nn_log (LC_DISCOVERY, "Found new group '%s.%s'; adding networkReaderEntry...\n",partition, topic);

    if ((entry = v_networkReaderEntryNew (vnetworkReader, group, gv.myNetworkId, 1, 0, FALSE)) == NULL)
    {
      NN_ERROR0 ("creation of networkReaderEntry failed\n");
      return U_RESULT_INTERNAL_ERROR;
    }

    if (group->topic->qos->durability.kind >= V_DURABILITY_TRANSIENT)
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
    if (strcmp (v_entity (group->partition)->name, V_BUILTIN_PARTITION) == 0 &&
        strcmp (v_entity (group->topic)->name, bubble_topic_name) == 0)
    {
      TRACE (("bubble writer's group now discovered\n"));
      os_mutexLock (&gluelock);
      bubble_writer_group_attached = 1;
      os_condBroadcast (&gluecond);
      os_mutexUnlock (&gluelock);
    }
  }
  return U_RESULT_OK;
}

static u_result handleGroups (v_service service)
{
  c_iter vgroups = v_serviceTakeNewGroups(service);
  v_group vgroup = (v_group) c_iterTakeFirst(vgroups);
  u_result result = U_RESULT_OK;
  while (vgroup && result == U_RESULT_OK)
  {
    result = handleGroup (vgroup);
    c_free (vgroup);
    vgroup = (v_group) c_iterTakeFirst(vgroups);
  }
  c_iterFree (vgroups);
  return result;
}

static u_result initial_local_discovery (const struct builtin_datareader_set *drset)
{
  struct thread_state1 *self = lookup_thread_state ();
  u_result (*fs[3]) (const struct builtin_datareader_set *);
  u_result res = U_RESULT_OK;
  int i;

  /* First mirror currently existing participants: we create
     fictitious data readers for transient data and pretend they are
     owned by ourself. For that, we need to mirror ourself before we
     start looking at the groups.*/
  TRACE (("Initial local discovery ...\n"));

  thread_state_awake (self);
  os_mutexLock (&gluelock);
  fs[0] = handleParticipants;
  fs[1] = handlePublications;
  fs[2] = handleSubscriptions;
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
  res = handleGroups (service);
  thread_state_asleep (self);
  return res;
}

static u_result monitor_local_entities (const u_waitset waitset, const struct builtin_datareader_set *drset)
{
  struct thread_state1 *self = lookup_thread_state ();
  u_waitsetEvent event;
  u_result result;

  TRACE (("Mirroring DCPS entities in DDSI ...\n"));

  result = U_RESULT_OK;
  while (result == U_RESULT_OK && !gv.terminate)
  {
    c_iter events = NULL;
    result = u_waitsetWaitEvents (waitset, &events);
    thread_state_awake (self);
    switch (result)
    {
      case U_RESULT_OK:
        event = (u_waitsetEvent) c_iterTakeFirst (events);
        while (event)
        {
          if ((event->events & V_EVENT_DATA_AVAILABLE) == V_EVENT_DATA_AVAILABLE)
          {
            int i, handled = 0;
            os_mutexLock (&gluelock);
            for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++)
            {
              if (event->entity == (u_entity) drset->e[i].participant_dr) {
                handled = 1;
                result = handleParticipants (drset);
                break;
              } else if (event->entity == (u_entity) drset->e[i].subscription_dr) {
                handled = 1;
                result = handleSubscriptions (drset);
                break;
              } else if (event->entity == (u_entity) drset->e[i].publication_dr) {
                handled = 1;
                result = handlePublications (drset);
                break;
              }
            }
            if (handled)
              ; /* all the convoluted stuff is so we don't lose an error */
            else if (event->entity != NULL)
              NN_FATAL0 ("fatal: waitset triggered by unknown entity\n");
            else
              NN_WARNING1 ("warning: waitset triggered by event %x but entity is missing\n", event->events);
            os_mutexUnlock (&gluelock);
          }
          if ((event->events & V_EVENT_NEW_GROUP) == V_EVENT_NEW_GROUP)
          {
            result = handleGroups (service);
          }
          if ((event->events & V_EVENT_TRIGGER) == V_EVENT_TRIGGER)
          {
            result = U_RESULT_OK;
          }
          if (event->events & ~(V_EVENT_DATA_AVAILABLE | V_EVENT_NEW_GROUP | V_EVENT_TRIGGER))
          {
            NN_FATAL1 ("Received unexpected event %d\n", event->events);
          }
          u_waitsetEventFree (event);
          event = (u_waitsetEvent) c_iterTakeFirst (events);
        }
        break;
      case U_RESULT_DETACHING:
        nn_log (LC_INFO, "Starting termination now...\n");
        break;
      case U_RESULT_TIMEOUT:
        result = U_RESULT_OK;
        break;
      default:
        NN_ERROR0 ("Waitset wait failed.\n");
        break;
    }
    thread_state_asleep (self);

    if (events)
    {
      /* events may be null if waitset was deleted */
      c_iterFree (events);
    }
  }
  return result;
}

static u_actionResult reader_take_one_helper (c_object o, c_voidp arg)
{
  v_readerSample s = (v_readerSample) o;
  v_readerSample *sample = (v_readerSample *) arg;
  u_actionResult result = 0;
  if (s != NULL)
  {
    if (v_stateTest (s->sampleState, L_VALIDDATA))
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
  return u_dataReaderTake (rd, reader_take_one_helper, sample);
}

static void tentative_participants_cleanup_handler (struct xevent *ev, UNUSED_ARG (void *arg), os_int64 tnow)
{
  struct tentative_participant *node;
  os_int64 tnext;

  os_mutexLock (&gluelock);
  node = ut_avlFindMin (&tentative_participants_treedef, &tentative_participants);
  while (node)
  {
    struct tentative_participant *node1 = ut_avlFindSucc (&tentative_participants_treedef, &tentative_participants, node);
    if (node->t_expiry <= tnow)
    {
      TRACE (("tentative_participants_cleanup_handler_helper: %x:%x:%x - deleting: no built-in topic received\n", node->key.systemId, node->key.localId, node->key.serial));
      ut_avlDelete (&tentative_participants_treedef, &tentative_participants, node);
      os_free (node);
    }
    node = node1;
  }
  os_mutexUnlock (&gluelock);

  if (ut_avlIsEmpty (&tentative_participants))
    tnext = T_NEVER;
  else
    tnext = tnow + T_SECOND;
  resched_xevent_if_earlier (ev, tnext);
}

static int do_new_participant (const v_gid *key, int confirmed)
{
  unsigned flags = 0;
  int res;

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

  /* don't care if it already exists, failure is not an option :)
     (actually, it doesn't matter much if it doesn't exist
     afterward)  */
  if ((res = new_participant_gid (key, flags)) < 0)
    return res;

  if (!confirmed)
  {
    struct tentative_participant *tp;
    ut_avlIPath_t path;
    if (ut_avlLookupIPath (&tentative_participants_treedef, &tentative_participants, key, &path) == NULL)
    {
      if ((tp = os_malloc (sizeof (*tp))) != NULL)
      {
        TRACE (("do_new_participant: %x:%x:%x - registering as tentative\n", key->systemId, key->localId, key->serial));
        tp->key = *key;
        tp->t_expiry = now () + 2 * T_SECOND;
        ut_avlInsertIPath (&tentative_participants_treedef, &tentative_participants, tp, &path);
        resched_xevent_if_earlier (tentative_participants_cleanup_event, tp->t_expiry);
      }
    }
  }
  return 0;
}

static u_result handleParticipants (const struct builtin_datareader_set *drset)
{
  v_dataReaderSample sample;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    while (reader_take_one (drset->e[setidx].participant_dr, &sample) == U_RESULT_OK && sample != NULL)
    {
      v_state state = v_readerSample (sample)->sampleState;
      v_message msg = v_dataReaderSampleMessage (sample);
      struct v_participantInfo const * const data =
        (struct v_participantInfo *) C_DISPLACE (msg, drset->participant_off);

      TRACE (("handleParticipants: received sample for %x:%x:%x state %x\n", data->key.systemId, data->key.localId, data->key.serial, state));

      /* The participant mapping of the DDSI2 service itself is
         special-cased, to guarantee it being available before
         handleGroup() is called for the first time. */
      if (!v_gidEqual (data->key, ddsi2_participant_gid) && !config.squash_participants)
      {
        struct tentative_participant *tp;

        if (v_stateTest (state, L_DISPOSED))
          delete_participant_gid (&data->key);
        else
          do_new_participant (&data->key, 1);

        if ((tp = ut_avlLookup (&tentative_participants_treedef, &tentative_participants, &data->key)) != NULL)
        {
          TRACE (("handleParticipants: %x:%x:%x no longer tentative\n", data->key.systemId, data->key.localId, data->key.serial));
          ut_avlDelete (&tentative_participants_treedef, &tentative_participants, tp);
        }
      }
      c_free (sample);
    }
  }
  return U_RESULT_OK;
}

static u_result must_have_participant (const struct builtin_datareader_set *drset, const v_gid *ppgid)
{
  nn_guid_t ppguid;

  /* First do a quick check whether it already exists */
  ppguid_from_ppgid (&ppguid, ppgid);
  if (ephash_lookup_participant_guid (&ppguid))
    return U_RESULT_OK;

  /* If not, try handleParticipants, that being the normal way */
  handleParticipants (drset);

  /* If that doesn't work, go check the kernel & if the participant's
     handle can still be claimed, assume it is ok and set a timer to
     delete it eventually if the built-in topic doesn't show up */
  if (ephash_lookup_participant_guid (&ppguid) == NULL)
  {
    v_public v;
    TRACE (("must_have_participant: %x:%x:%x - still unknown\n", ppgid->systemId, ppgid->localId, ppgid->serial));
    if (v_gidClaimChecked (*ppgid, gv.ospl_kernel, (v_public *) &v) != V_HANDLE_OK)
    {
      TRACE (("must_have_participant: %x:%x:%x - no wonder: kernel doesn't grok the id\n", ppgid->systemId, ppgid->localId, ppgid->serial));
    }
    else
    {
      /* If we can't get it in time from the built-in topics, take
         it straight from the kernel.  This may succeed even when
         the dispose has already been generated.  (And if the
         dispose has already been consumed by handleParticipants,
         but we still succeed in claiming the participant, it'll
         never be deleted.  THIS IS BAD.) */
      TRACE (("must_have_participant: %x:%x:%x - creating tentatively\n", ppgid->systemId, ppgid->localId, ppgid->serial));
      do_new_participant (ppgid, 0);
      v_gidRelease (*ppgid, gv.ospl_kernel);
    }
  }
  return U_RESULT_OK;
}


static u_result handleSubscriptions (const struct builtin_datareader_set *drset)
{
  v_dataReaderSample sample;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    while (reader_take_one (drset->e[setidx].subscription_dr, &sample) == U_RESULT_OK && sample != NULL)
    {
      v_state state = v_readerSample (sample)->sampleState;
      v_message msg = v_dataReaderSampleMessage (sample);
      struct v_subscriptionInfo const * const data =
        (struct v_subscriptionInfo *) C_DISPLACE (msg, drset->subscription_off);

      /* only handle subscriptions that have at least 1 partition that is not ignored*/
      if (v_gidEqual (data->participant_key, ddsi2_participant_gid))
      {
        /* ignore own readers/writers */
      }
      else if (v_stateTest (state, L_DISPOSED))
      {
        delete_reader_gid (&data->key);
      }
      else
      {
        v_gid participant_gid;
        v_topic topic;
        nn_xqos_t xqos;
        int res;
        /* Discovery of all stuff is asynchronous, so it is quite
           possible that we discover a reader before we discover its
           participant. But with the DCPS built-in data for the
           participant _always_ published before that of its
           subscriptions and publications all we need to do is process
           the participants in case we can't find it.

           Considering that handleParticipants is a fairly cheap
           operation if nothing needs to be done, and further
           considering that this discovery stuff is only done outside of
           the data path (with the exception of just-in-time creation of
           writers, which is _really_ rare) there is no good reason not
           call handleParticipants always.

           Even then, we may find that the participant is unknown. But
           that presumably means the participant has already been
           deleted, and we're just too late. In that case, we forget
           about the endpoint.  */
        if (!config.squash_participants)
          participant_gid = data->participant_key;
        else
          participant_gid = ddsi2_participant_gid;
        must_have_participant (drset, &participant_gid);
        topic = v_lookupTopic (gv.ospl_kernel, data->topic_name);
        assert (topic != NULL);
        if (init_reader_qos (data, &xqos, setidx != 0) < 0)
          res = ERR_INVALID_DATA;
        else
        {
          res = new_reader_gid (&participant_gid, &data->key, topic, &xqos);
          nn_xqos_fini (&xqos);
        }
        if (res == ERR_UNKNOWN_ENTITY)
          NN_ERROR0 ("handleSubscriptions: participant not found, presumably already been deleted\n");
        else if (res < 0)
        {
          if (res != ERR_ENTITY_EXISTS)
          {
            /* QoS changes and other things (that I don't understand
               yet) may cause a new sample of the built-in topic to be
               read. But we don't want to create the same subscription
               twice -- they'd get different GUIDs, the new one would
               hide the old one, &c. Horrors. */
            NN_ERROR1 ("handleSubscriptions: new_reader: error %d\n", res);
          }
        }
      }

      c_free(sample);
    }
  }

  /* Intentionally ignoring errors: we log them but continue, becasue they generally aren't fatal,  */
  return U_RESULT_OK;
}

static void schedule_delete_writer (const v_gid *key)
{
  struct bubble_s data;
  u_result res;
  if (!bubble_writer_group_attached)
  {
    TRACE (("schedule_delete_writer(%x:%x:%x) deleting immediately: bubble writer's group not yet attached\n",
            key->systemId, key->localId, key->serial));
    delete_writer_gid (key);
  }
  else
  {
    TRACE (("schedule_delete_writer(%x:%x:%x) writing bubble\n", key->systemId, key->localId, key->serial));
    data.kind = BTK_DELETE_WRITER;
    data.systemId = key->systemId;
    data.localId = key->localId;
    data.serial = key->serial;
    if ((res = u_writerWrite (bubble_writer, &data, u_timeGet (), U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    {
      NN_WARNING4 ("schedule_delete_writer(%x:%x:%x) failed with result %d, deleting immediately instead\n",
                   key->systemId, key->localId, key->serial, (int) res);
      delete_writer_gid (key);
    }
  }
}

static u_result handlePublications (const struct builtin_datareader_set *drset)
{
  v_dataReaderSample sample;
  int setidx;
  for (setidx = 0; setidx < (int) (sizeof (drset->e) / sizeof (*drset->e)); setidx++)
  {
    while (reader_take_one (drset->e[setidx].publication_dr, &sample) == U_RESULT_OK && sample != NULL)
    {
      v_state state = v_readerSample (sample)->sampleState;
      v_message msg = v_dataReaderSampleMessage (sample);
      struct v_publicationInfo const * const data =
        (struct v_publicationInfo *) C_DISPLACE (msg, drset->publication_off);

      /* We completely want to hide our internal bubble data writer, it
         is but an implementation artefact internal to a single DDSI2
         instance. As long as there is but one such writer, might as
         well test it here. */
      TRACE (("handlePublications: %x:%x:%x state %x\n", data->key.systemId, data->key.localId, data->key.serial, (unsigned) state));
      if (!v_gidEqual (data->participant_key, ddsi2_participant_gid))
      {
        if (v_stateTest (state, L_NEW) || v_stateTest (state, L_WRITE))
        {
          {
            /* see handleSubscriptions for comments */
            v_gid participant_gid;
            v_topic topic;
            nn_xqos_t xqos;
            int res;
            if (!config.squash_participants)
              participant_gid = data->participant_key;
            else
              participant_gid = ddsi2_participant_gid;
            must_have_participant (drset, &participant_gid);
            topic = v_lookupTopic (gv.ospl_kernel, data->topic_name);
            assert (topic != NULL);
            if (init_writer_qos (data, &xqos, setidx != 0) < 0)
              res = ERR_INVALID_DATA;
            else
            {
              res = new_writer_gid (&participant_gid, &data->key, topic, &xqos);
              nn_xqos_fini (&xqos);
            }
            if (res == ERR_UNKNOWN_ENTITY)
              NN_ERROR0 ("handlePublications: participant not found, presumably already been deleted\n");
            else if (res < 0)
            {
              if (res != ERR_ENTITY_EXISTS)
                NN_ERROR1 ("handlePublications: new_writer: error %d\n", res);
            }
          }
        }

        if (v_stateTest (state, L_DISPOSED))
        {
          schedule_delete_writer (&data->key);
        }
      }
      c_free (sample);
    }
  }
  /* Intentionally ignoring errors: we log them but continue, becasue they generally aren't fatal,  */
  return U_RESULT_OK;
}

static void new_fictitious_transient_reader (v_group group)
{
  v_topicQos const qos = group->topic->qos;
  nn_xqos_t xqos;
  int res;

  nn_xqos_init_empty (&xqos);
  qcp_durability (&xqos, &qos->durability);
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
  qcp_deadline (&xqos, &qos->deadline);
  qcp_latency (&xqos, &qos->latency);
  qcp_liveliness (&xqos, &qos->liveliness);
  qcp_reliability (&xqos, &qos->reliability);
  qcp_ownership (&xqos, &qos->ownership);
  qcp_destination_order (&xqos, &qos->orderby);
  xqos.present |= QP_PARTITION;
  xqos.partition.n = 1;
  xqos.partition.strs = os_malloc (sizeof (*xqos.partition.strs));
  xqos.partition.strs[0] = os_strdup (v_entity (group->partition)->name);
  qcp_topic_data_topic (&xqos, &qos->topicData);
  qcp_lifespan (&xqos, &qos->lifespan);

  nn_log (LC_DISCOVERY, "new_fictitious_transient_reader: %s.%s\n",
          v_entity (group->topic)->name, v_entity (group->partition)->name);
  /* the fictitious transient data reader has no gid */
  {
    topic_t topic;
    if ((topic = deftopic (group->topic, NULL)) == NULL)
      res = ERR_UNSPECIFIED;
    else
    {
      nn_guid_t ppguid, guid;
      ppguid_from_ppgid (&ppguid, &ddsi2_participant_gid);
      res = new_reader (&guid, &ppguid, topic, &xqos, 0, 0);
    }
  }

  if (res == ERR_UNKNOWN_ENTITY)
    NN_FATAL0 ("new_fictitious_transient_reader: the ddsi2 participant should've been known already\n");
  else if (res < 0)
    NN_ERROR3 ("new_fictitious_transient_reader(%s.%s): new_reader: error %d - transient data may not work properly\n",
               v_entity (group->topic)->name, v_entity (group->partition)->name, res);
  nn_xqos_fini (&xqos);
}

/* NOTE: Blatantly stolen from the "native" networking implementation
   (src/services/networking/code/nw_controller.c). Methinks these
   should be guaranteed unique, not hopefully unique. Surely one could
   just a "service instance id" or somesuch for this purpose ...

   NOTE 2: And fixed after stealing ... Native networking uses (0 <=
   id < (1e9-1)), we use one in (1e-9 <= id' < (2e9-1)). So there
   never should (1) be an id collision and (2) an accidental use of
   LOCAL by DDSI2. */
static v_networkId getNetworkId (void)
{
  os_time time;
  /* NOTE: for now, let network ID be a "random"-number. This number
     has to be retrieved from the network/os layer. */
  assert (V_NETWORKID_LOCAL == 0x0);
  time = os_timeGet();
  return time.tv_nsec + 1000000000;
}

static void destroy_builtin_readers (struct builtin_datareader_set *drset)
{
  int i;
  for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++)
  {
    if (drset->e[i].publication_dr)
      u_dataReaderFree (drset->e[i].publication_dr);
    if (drset->e[i].subscription_dr)
      u_dataReaderFree (drset->e[i].subscription_dr);
    if (drset->e[i].participant_dr)
      u_dataReaderFree (drset->e[i].participant_dr);
    if (drset->e[i].subscriber)
      u_subscriberFree (drset->e[i].subscriber);
  }
}

static void getTopicDataFieldOffset (v_entity e, c_voidp arg)
{
  c_long *offset = arg;
  *offset = v_topic (e)->dataField->offset;
}

static u_result create_builtin_readers (struct builtin_datareader_set *drset, u_participant p)
{
  v_readerQos rdQos = NULL;
  v_subscriberQos sQos = NULL;
  v_gid gid;
  c_value ps[1];
  c_char ns_part[U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE];
  v_durabilityKind durqos[2];
  const char *parts[2];
  const char *selects[2];
  u_result result;
  int i;

  Q_STATIC_ASSERT_CODE (sizeof (parts) / sizeof (*parts) == sizeof (drset->e) / sizeof (*drset->e));
  Q_STATIC_ASSERT_CODE (sizeof (durqos) / sizeof (*durqos) == sizeof (drset->e) / sizeof (*drset->e));
  parts[0] = V_BUILTIN_PARTITION;
  durqos[0] = V_DURABILITY_TRANSIENT;
  selects[0] = "select * from DCPSSubscription where key.systemId = %0";
  parts[1] = ns_part;
  durqos[1] = V_DURABILITY_VOLATILE;
  selects[1] = "select * from DCPSSubscription";

  if ((result = u_participantFederationSpecificPartitionName (participant, ns_part, sizeof (ns_part))) != U_RESULT_OK)
    return result;

  for (i = 0; i < (int) (sizeof (drset->e) / sizeof (*drset->e)); i++)
  {
    drset->e[i].subscriber = NULL;
    drset->e[i].participant_dr = NULL;
    drset->e[i].subscription_dr = NULL;
    drset->e[i].publication_dr = NULL;
  }

  if ((rdQos = u_readerQosNew (NULL)) == NULL)
    goto fail;
  rdQos->reliability.kind = V_RELIABILITY_RELIABLE;
  rdQos->history.kind = V_HISTORY_KEEPLAST;
  rdQos->history.depth = 1;

  for (i = 0; i < (int) (sizeof (parts) / sizeof (*parts)); i++)
  {
    char name[32];

    if ((sQos = u_subscriberQosNew (NULL)) == NULL)
      goto fail;
    sQos->presentation.access_scope = V_PRESENTATION_TOPIC;
    if ((sQos->partition = os_strdup (parts[i])) == NULL)
      goto fail;
    snprintf (name, sizeof (name), "DDSI2BuiltinSubscriber%d", i);
    if ((drset->e[i].subscriber = u_subscriberNew (p, name, sQos, TRUE)) == NULL)
      goto fail;
    u_subscriberQosFree (sQos);
    sQos = NULL;

    gid = u_entityGid ((u_entity) drset->e[i].subscriber);
    ps[0].kind = V_ULONG;
    ps[0].is.ULong = gid.systemId;
    TRACE (("create_builtin_readers (%s): systemId = %lx\n", parts[i], (unsigned long) ps[0].is.ULong));

    /* Durability for the "real" DCPSSubscription is differs from that
       of the one (currently) used by R&R, hence why the durability
       qos is set so late. */
    rdQos->durability.kind = durqos[i];

    if ((drset->e[i].participant_dr = u_subscriberCreateDataReader (drset->e[i].subscriber, "DCPSParticipantReader", "select * from DCPSParticipant where key.systemId = %0", ps, rdQos, TRUE)) == NULL)
      goto fail;
    if ((drset->e[i].subscription_dr = u_subscriberCreateDataReader (drset->e[i].subscriber, "DCPSSubscriptionReader", selects[i], ps, rdQos, TRUE)) == NULL)
      goto fail;
    if ((drset->e[i].publication_dr = u_subscriberCreateDataReader (drset->e[i].subscriber, "DCPSPublicationReader", "select * from DCPSPublication where key.systemId = %0", ps, rdQos, TRUE)) == NULL)
      goto fail;
  }
  u_readerQosFree (rdQos);

  {
    static const struct topic_offset_tab {
      char *what;
      char *name;
      os_size_t offset;
    } topic_offset_tab[] = {
      { "participant", V_PARTICIPANTINFO_NAME, offsetof (struct builtin_datareader_set, participant_off) },
      { "publication", V_PUBLICATIONINFO_NAME, offsetof (struct builtin_datareader_set, publication_off) },
      { "subscription", V_SUBSCRIPTIONINFO_NAME, offsetof (struct builtin_datareader_set, subscription_off) }
    };
    v_duration duration;
    c_iter topics;
    int i;
    duration.seconds = 0;
    duration.nanoseconds = 0;
    for (i = 0; i < (int) (sizeof (topic_offset_tab) / sizeof (*topic_offset_tab)); i++)
    {
      const struct topic_offset_tab *tt = &topic_offset_tab[i];
      u_topic topic;
      topics = u_participantFindTopic (participant, tt->name, duration);
      topic = c_iterTakeFirst (topics);
      if (topic == NULL)
        NN_FATAL1 ("Could not resolve %s info offset.\n", tt->what);
      else
      {
        u_entityAction (u_entity (topic), getTopicDataFieldOffset, (c_long *) ((char *) drset + tt->offset));
        u_topicFree (topic);
      }
      c_iterFree (topics);
    }
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

static c_bool bubble_writer_copy (UNUSED_ARG (c_type type), void *data, void *to)
{
  memcpy (to, data, sizeof (struct bubble_s));
  return TRUE;
}

static u_result create_bubble_topic_writer (u_participant p)
{
  v_publisherQos pqos = NULL;
  v_writerQos wrqos;

  if ((bubble_topic = u_topicNew (p, bubble_topic_name, "q_osplserModule::bubble", "", NULL)) == NULL)
    goto err_topic;
  if ((bubble_kernel_topic = v_lookupTopic (gv.ospl_kernel, bubble_topic_name)) == NULL)
    goto err_kernel_topic;

  if ((pqos = u_publisherQosNew (NULL)) == NULL)
    goto err_pqos;
  os_free (pqos->partition);
  pqos->partition = os_strdup (V_BUILTIN_PARTITION);
  if ((bubble_publisher = u_publisherNew (p, "ddsi2 bubble publisher", pqos, TRUE)) == NULL)
    goto err_publisher;

  if ((wrqos = u_writerQosNew (NULL)) == NULL)
    goto err_wrqos;
  wrqos->reliability.kind = V_RELIABILITY_RELIABLE;
  wrqos->history.kind = V_HISTORY_KEEPALL;
  if ((bubble_writer = u_writerNew (bubble_publisher, "ddsi2 bubble writer", bubble_topic, bubble_writer_copy, wrqos, TRUE)) == NULL)
    goto err_writer;
  bubble_writer_gid = u_entityGid (u_entity (bubble_writer));
  nn_log (LC_DISCOVERY, "bubble writer: gid %x:%x:%x\n", bubble_writer_gid.systemId, bubble_writer_gid.localId, bubble_writer_gid.serial);

  u_writerQosFree (wrqos);
  u_publisherQosFree (pqos);
  return U_RESULT_OK;

 err_writer:
  u_writerQosFree (wrqos);
 err_wrqos:
  u_publisherFree (bubble_publisher);
 err_publisher:
  u_publisherQosFree (pqos);
 err_pqos:
  c_free (bubble_kernel_topic);
 err_kernel_topic:
  u_topicFree (bubble_topic);
 err_topic:
  return U_RESULT_INTERNAL_ERROR;
}

static void destroy_bubble_topic_writer (void)
{
  u_writerFree (bubble_writer);
  u_publisherFree (bubble_publisher);
  u_topicFree (bubble_topic);
  c_free (bubble_kernel_topic);
}

static struct thread_state1 * create_channel_reader_thread
(
  const char * name,
  struct builtin_datareader_set * drset,
  ddsi_tran_conn_t transmit_conn
)
{
  /* create one (or more, eventually) threads to read from the network
     queue and transmit the data */
  struct channel_reader_arg *arg = os_malloc (sizeof (struct channel_reader_arg));
  char *thread_name = os_malloc (strlen ("xmit.") + strlen (name) + 1);
  struct thread_state1 *ts;
  sprintf (thread_name, "xmit.%s", name);
  arg->drset = drset;
  arg->transmit_conn = transmit_conn;
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
  v_duration p;
  p.seconds = (os_int32) leaseSec;
  p.nanoseconds = (os_int32) ((leaseSec - (float) p.seconds) * 1e9f);
  u_serviceRenewLease (u_service (vparticipant), p);
}

void ospl_ddsi2AtExit (void)
{
  u_userExit ();
}

OPENSPLICE_ENTRYPOINT (ospl_ddsi2)
{
  /* Exit status: 0: ok, 1: configuration error, 2: abnormal
     termination. Default to abnormal termination, then override if
     that is inappriorate */
  int exitstatus = 2;

  /* Default service name and service URI, taken from command line if
     given; config stores the service name in case it is needed at
     some later stage. */
  const char *service_name = "ddsi2";
  const char *service_uri = NULL;
  struct builtin_datareader_set drset;
  u_waitset disc_ws;

  /* Init static log buffer early as possible -- but we don't even
     have a lock yet.  This is ok if we are certain the log functions
     can only be called from the ddsi2 main thread until the mutex
     initialization, which is currently the case.  */
  gv.static_logbuf_lock_inited = 0;
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

  os_mutexAttrInit (&gv.mattr);
  gv.mattr.scopeAttr = OS_SCOPE_PRIVATE;
  os_condAttrInit (&gv.cattr);
  gv.cattr.scopeAttr = OS_SCOPE_PRIVATE;
  os_rwlockAttrInit (&gv.rwattr);
  gv.rwattr.scopeAttr = OS_SCOPE_PRIVATE;

  /* now init the lock */
  if (os_mutexInit (&gv.static_logbuf_lock, &gv.mattr) != os_resultSuccess) {
    NN_ERROR0 ("initialisation of local administration mutex failed\n");
    goto err_static_logbuf_lock;
  }
  gv.static_logbuf_lock_inited = 1;

  /* Init glue lock + cond, used for some internal synchronisation */
  if (os_mutexInit (&gluelock, &gv.mattr) != os_resultSuccess) {
    NN_ERROR0 ("initialisation of local administration mutex failed\n");
    goto err_gluelock;
  }
  if (os_condInit (&gluecond, &gluelock, &gv.cattr) != os_resultSuccess) {
    NN_ERROR0 ("initialisation of local administration cond.var failed\n");
    goto err_gluecond;
  }

  /* Create participant, notify kernel that I am initializing, and
     start monitoring the splicedaemon state, so we can terminate if
     he says so. */
  {
    v_participantQos participantQos;
    if ((participantQos = u_participantQosNew (NULL)) == NULL) {
      NN_ERROR0 ("allocation of participantQos failed\n");
      goto err_participant;
    }
    if ((participant = u_participant (
                 u_serviceNew (service_uri, 0, service_name, NULL,
                               U_SERVICE_DDSI, (v_qos) participantQos))) == NULL) {
      NN_ERROR0 ("creation of participant failed\n");
      u_participantQosFree (participantQos);
      goto err_participant;
    }
    u_participantQosFree (participantQos);
  }
  ddsi2_participant_gid = u_entityGid (u_entity (participant));
  u_serviceChangeState (u_service (participant), STATE_INITIALISING);
  u_serviceWatchSpliceDaemon (u_service (participant), watch_spliced, NULL);

  /* Need to know the kernel's v_service for our u_service -- the
     v_service is rather useful, unlike the u_service. */
  service = determine_kernel_service (participant);
  gv.ospl_base = c_getBase (service);
  gv.ospl_kernel = v_object (service)->kernel;
  gv.myNetworkId = getNetworkId ();
  gv.ospl_qostype =
    (c_collectionType) c_metaArrayTypeNew (c_metaObject (gv.ospl_base),
                                         "C_ARRAY<c_octet>", c_octet_t (gv.ospl_base), 0);

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
    if ((cfgst = config_init (participant, service_name)) == NULL) {
      /* This field is used in NN_ERRORx, but is freed on failed config_init, so
       * it is temporarily initialised for errors in configuration initialisation
       * error reporting. */
      config.servicename = (char*) service_name;
      NN_ERROR0 ("Could not initialise configuration\n");
      exitstatus = 1;
      goto err_config_init;
    }

    if (rtps_config_prep (cfgst) < 0)
    {
      NN_ERROR0 ("Could not initialise configuration\n");
      exitstatus = 1;
      goto err_config_late_error;
    }

    upgrade_main_thread ();
  }

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

  /* Start-up DDSI proper (RTPS + discovery) */
  rtps_init ();

  /* Prepare hash table for mapping GUIDs to entities */
  if ((gid_hash = ephash_new (config.gid_hash_softlimit)) == NULL)
    goto err_gid_hash;

  /* Tentative participants admin */
  ut_avlInit (&tentative_participants_treedef, &tentative_participants);
  tentative_participants_cleanup_event = qxev_callback (T_NEVER, (void (*) (struct xevent *, void *, os_int64)) tentative_participants_cleanup_handler, 0);

  /* Local discovery will eventually discover the DCPS participant of
     DDSI2 itself, but may be late in doing so.  But we know all we
     need to know so create it now.  Note: no reason to clean it up
     explicitly, rtps_term() will take care of that. */
  thread_state_awake (lookup_thread_state ());
  new_participant_gid (&ddsi2_participant_gid, RTPS_PF_PRIVILEGED_PP);
  thread_state_asleep (lookup_thread_state ());

  /* Create subscriber, network reader to receive messages to be transmitted.
     The queue's will be created by the channel_reader_threads*/
  {
    v_subscriberQos subscriberQos;
    c_time resolution;
    if ((subscriberQos = u_subscriberQosNew (NULL)) == NULL) {
      NN_ERROR0 ("allocation of subscriberQos for message subscriber failed\n");
      goto err_client_subscriber;
    }
    os_free (subscriberQos->partition);
    subscriberQos->partition = NULL;

    if ((networkSubscriber = u_subscriberNew (participant, "networkSubscriber", subscriberQos, TRUE)) == NULL) {
      NN_ERROR0 ("creation of message subscriber failed\n");
      u_subscriberQosFree (subscriberQos);
      goto err_client_subscriber;
    }
    u_subscriberQosFree (subscriberQos);

    if ((networkReader = u_networkReaderNew (networkSubscriber, "networkReader", NULL, TRUE)) == NULL) {
      NN_ERROR0 ("creation of network reader failed\n");
      goto err_networkReaderNew;
    }
    vnetworkReader = determine_kernel_networkReader (networkReader);
    assert (vnetworkReader != NULL);
    TRACE (("network reader: %p\n", (void *) networkReader));

    resolution.seconds = 1;
    resolution.nanoseconds = 0;
    if (u_networkReaderCreateQueue (networkReader, config.nw_queue_size, 0, FALSE, FALSE, resolution, FALSE, &gv.networkQueueId, "DDSI2") != U_RESULT_OK) {
      NN_ERROR0 ("creation of network reader queue failed\n");
      goto err_networkReaderCreateQueue;
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


  TRACE (("transmit port %d\n", (int) ddsi_tran_port (gv.data_conn_uc)));
  if ((gv.channel_reader_ts = create_channel_reader_thread ("user", &drset, gv.data_conn_uc)) == NULL)
  {
    goto err_channel_reader_thread;
  }

  /* Mirror local entities in DDSI until requested to stop */
  {
    u_result uresult;
    u_serviceChangeState (u_service (participant), STATE_OPERATIONAL);
    uresult = monitor_local_entities (local_discovery_waitset, &drset);
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
 err_channel_reader_thread:
  v_networkReaderTrigger (vnetworkReader, 0);
  u_waitsetNotify (local_discovery_waitset, NULL);
  if (gv.channel_reader_ts)
    join_thread (gv.channel_reader_ts, NULL);
  os_mutexLock (&gluelock);
  local_discovery_waitset = NULL;
  os_mutexUnlock (&gluelock);
err_make_transmit_socket:
err_initial_local_discovery:
  destroy_bubble_topic_writer ();
err_bubble_topic_writer:
  destroy_discovery_waitset (disc_ws, participant, &drset);
err_create_discovery_waitset:
  destroy_builtin_readers (&drset);
err_create_builtin_readers:
err_networkReaderCreateQueue:
  /* fugly code to avoid an even nastier race condition with
     watch_spliced */
  os_mutexLock (&gluelock);
  u_networkReaderFree (networkReader);
  networkReader = NULL;
  vnetworkReader = NULL;
  os_mutexUnlock (&gluelock);
err_networkReaderNew:
  u_subscriberFree (networkSubscriber);
err_client_subscriber:
  delete_xevent (tentative_participants_cleanup_event);
  ut_avlFree (&tentative_participants_treedef, &tentative_participants, os_free);
  ephash_free (gid_hash);
err_gid_hash:
  rtps_term ();
err_servicelease_start:
  nn_servicelease_free (gv.servicelease);
err_servicelease:
  downgrade_main_thread ();
  thread_states_fini ();
err_config_late_error:
  /* would expect config_fini() here, but no, it is postponed. */
 err_config_init:
  c_free (gv.ospl_qostype);
  u_serviceChangeState (u_service (participant), STATE_TERMINATED);
  if (u_serviceFree (u_service (participant)) != U_RESULT_OK)
    NN_ERROR0 ("deletion of participant failed\n");
err_participant:
  os_condDestroy (&gluecond);
err_gluecond:
  os_mutexDestroy (&gluelock);
 err_gluelock:
  (void) u_userDetach ();
  nn_log (LC_INFO, "Finis.\n");

  /* Must be really late, or nn_log becomes really unhappy -- but it
     should be before os_osExit (which appears to be called from
     u_userExit(), which is not called by u_userDetach but by an exit
     handler, it appears.) */
  config_fini ();
  os_mutexDestroy (&gv.static_logbuf_lock);
err_static_logbuf_lock:
err_userInitialise:
  return exitstatus;
}

/* SHA1 not available (unoffical build.) */
