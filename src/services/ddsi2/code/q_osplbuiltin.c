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
#include <stddef.h>

#include "vortex_os.h"
#include "os_defs.h"
#include "os_mutex.h"
#include "os_atomics.h"

#include "u_user.h"
#include "u_writer.h"
#include "u_publisher.h"
#include "u_builtin.h"

#include "c_stringSupport.h"
#include "ut_crc.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_time.h"
#include "v_state.h"
#include "v_topicQos.h"
#include "v_builtin.h"
#include "v_writer.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_policy.h"

#include "sd_serializer.h"
#include "sd_serializerXML.h"

#include "q_config.h"
#include "q_log.h"
#include "q_misc.h"
#include "q_unused.h"
#include "q_entity.h"
#include "q_time.h"
#include "q_error.h"
#include "q_osplserModule.h"
#include "q_builtin_topic.h"
#include "q_osplbuiltin.h"
#include "q_groupset.h"

#include "ut_xmlparser.h"

#define PGID(g) ((g).systemId), ((g).localId), ((g).serial)

struct faked_systems {
  ut_avlNode_t avlnode;
  c_ulong systemId;
  c_ulong participant_count;
};

static int compare_systemId (const void *va, const void *vb);

static u_publisher builtin_topic_publisher;
static u_writer heartbeatInfo_wr;
static u_writer participantInfo_wr;
static u_writer participantCMInfo_wr;
static u_writer publicationInfo_wr;
static u_writer subscriptionInfo_wr;
static u_writer dataWriterCMInfo_wr;
static u_writer dataReaderCMInfo_wr;
static u_writer publisherCMInfo_wr;
static u_writer subscriberCMInfo_wr;
static u_writer topicInfo_wr;
static const ut_avlTreedef_t faked_systems_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct faked_systems, avlnode), offsetof (struct faked_systems, systemId), compare_systemId, 0);
static ut_avlTree_t faked_systems_tree;
static os_mutex faked_systems_lock;

static int compare_systemId (const void *va, const void *vb)
{
  const os_int32 *a = va;
  const os_int32 *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

#if 0
static nn_duration_t os_duration_to_ddsi_duration (os_duration t)
{
  if (OS_DURATION_ISINFINITE (t))
    return nn_to_ddsi_duration (T_NEVER);
  else
  {
    return nn_to_ddsi_duration (t);
  }
}
#endif

static nn_duration_t v_duration_to_ddsi_duration (v_duration t)
{
  if (v_durationIsInfinite (t))
    return nn_to_ddsi_duration (T_NEVER);
  else
  {
    os_int64 tt;
    tt = t.seconds * T_SECOND + t.nanoseconds;
    return nn_to_ddsi_duration (tt);
  }
}

static v_duration ddsi_duration_to_v_duration (nn_duration_t t)
{
  const v_duration inf = V_DURATION_INFINITE;
  os_int64 tt = nn_from_ddsi_duration (t);
  if (tt == T_NEVER)
    return inf;
  else
  {
    v_duration r;
    r.seconds = (c_long) (tt / T_SECOND);
    r.nanoseconds = (c_ulong) (tt % T_SECOND);
    return r;
  }
}

#if 0
static os_duration ddsi_duration_to_os_duration (nn_duration_t t)
{
  const os_duration inf = OS_DURATION_INFINITE;
  os_int64 tt = nn_from_ddsi_duration (t);
  if (tt == T_NEVER)
    return inf;
  else
  {
    os_duration r = OS_DURATION_INIT(tt / T_SECOND, tt % T_SECOND);
    return r;
  }
}
#endif

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
      break;
  }

  xqos->present |= QP_PRISMTECH_SYNCHRONOUS_ENDPOINT;
  xqos->reliability.max_blocking_time = v_duration_to_ddsi_duration(reliability->max_blocking_time);
  xqos->synchronous_endpoint.value = (reliability->synchronous != 0);
}

static void qcp_history (nn_xqos_t *xqos, const struct v_historyPolicy *history)
{
  xqos->present |= QP_HISTORY;
  switch (history->kind)
  {
    case V_HISTORY_KEEPLAST:
      xqos->history.kind = NN_KEEP_LAST_HISTORY_QOS;
      break;
    case V_HISTORY_KEEPALL:
      xqos->history.kind = NN_KEEP_ALL_HISTORY_QOS;
      break;
  }
  xqos->history.depth = history->depth;
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

static void qcp_durability_service (nn_xqos_t *xqos, const struct v_durabilityServicePolicy *durabilitysrv)
{
  xqos->present |= QP_DURABILITY_SERVICE;
  switch (durabilitysrv->history_kind)
  {
    case V_HISTORY_KEEPLAST:
      xqos->durability_service.history.kind = NN_KEEP_LAST_HISTORY_QOS;
      break;
    case V_HISTORY_KEEPALL:
      xqos->durability_service.history.kind = NN_KEEP_ALL_HISTORY_QOS;
      break;
  }
  xqos->durability_service.history.depth = durabilitysrv->history_depth;
  xqos->durability_service.service_cleanup_delay = v_duration_to_ddsi_duration (durabilitysrv->service_cleanup_delay);
  xqos->durability_service.resource_limits.max_instances = durabilitysrv->max_instances;
  xqos->durability_service.resource_limits.max_samples = durabilitysrv->max_samples;
  xqos->durability_service.resource_limits.max_samples_per_instance = durabilitysrv->max_samples_per_instance;
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
  unsigned i;
  xqos->present |= QP_PARTITION;
  xqos->partition.n = c_arraySize (ps->name);
  xqos->partition.strs = os_malloc (xqos->partition.n * sizeof (*xqos->partition.strs));
  for (i = 0; i < xqos->partition.n; i++)
    xqos->partition.strs[i] = os_strdup (ps->name[i]);
}

static void qcp_deadline (nn_xqos_t *xqos, const struct v_deadlinePolicy *a)
{
  xqos->present |= QP_DEADLINE;
  xqos->deadline.deadline = v_duration_to_ddsi_duration (a->period);
}

static void qcp_latency (nn_xqos_t *xqos, const struct v_latencyPolicy *a)
{
  xqos->present |= QP_LATENCY_BUDGET;
  xqos->latency_budget.duration = v_duration_to_ddsi_duration (a->duration);
}

static void qcp_entity_factory (nn_xqos_t *xqos, const struct v_entityFactoryPolicy *a)
{
  xqos->present |= QP_PRISMTECH_ENTITY_FACTORY;
  xqos->entity_factory.autoenable_created_entities = a->autoenable_created_entities;
}

static void qcp_share (nn_xqos_t *xqos, const struct v_sharePolicy *a)
{
  xqos->present |= QP_PRISMTECH_SHARE;
  xqos->share.enable = !!a->enable;
  xqos->share.name = a->name ? os_strdup (a->name) : "";
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
  if (v_durationIsZero (a->lease_duration))
    /* A lease duration of 0 can't work, and it is the value the
       kernel sets it to by default. It therefore seems reasonable to
       interpret it as infinite */
    xqos->liveliness.lease_duration = nn_to_ddsi_duration (T_NEVER);
  else
    xqos->liveliness.lease_duration = v_duration_to_ddsi_duration (a->lease_duration);
#else
  xqos->liveliness.lease_duration = v_duration_to_ddsi_duration (a->lease_duration);
#endif
}

static void qcp_lifespan (nn_xqos_t *xqos, const struct v_lifespanPolicy *a)
{
  xqos->present |= QP_LIFESPAN;
  xqos->lifespan.duration = v_duration_to_ddsi_duration (a->duration);
}

static void qcp_time_based_filter (nn_xqos_t *xqos, const struct v_pacingPolicy *a)
{
  xqos->present |= QP_TIME_BASED_FILTER;
  xqos->time_based_filter.minimum_separation = v_duration_to_ddsi_duration (a->minSeperation);
}

static void qcp_rdlifespan (nn_xqos_t *xqos, const struct v_readerLifespanPolicy *a)
{
  if (a->used)
  {
    xqos->present |= QP_LIFESPAN;
    xqos->lifespan.duration = v_duration_to_ddsi_duration (a->duration);
  }
}

static void qcp_wrlifecycle (nn_xqos_t *xqos, const struct v_writerLifecyclePolicy *a)
{
  xqos->present |= QP_PRISMTECH_WRITER_DATA_LIFECYCLE;
  xqos->writer_data_lifecycle.autodispose_unregistered_instances =
    (a->autodispose_unregistered_instances != 0);
  xqos->writer_data_lifecycle.autounregister_instance_delay = v_duration_to_ddsi_duration (a->autounregister_instance_delay);
  xqos->writer_data_lifecycle.autopurge_suspended_samples_delay = v_duration_to_ddsi_duration (a->autopurge_suspended_samples_delay);
}

static void qcp_rdlifecycle (nn_xqos_t *xqos, const struct v_readerLifecyclePolicy *a)
{
  xqos->present |= QP_PRISMTECH_READER_DATA_LIFECYCLE;
  xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay =
    v_duration_to_ddsi_duration (a->autopurge_nowriter_samples_delay);
  xqos->reader_data_lifecycle.autopurge_disposed_samples_delay =
    v_duration_to_ddsi_duration (a->autopurge_disposed_samples_delay);
  xqos->reader_data_lifecycle.autopurge_dispose_all =
    (a->autopurge_dispose_all != 0);
  xqos->reader_data_lifecycle.enable_invalid_samples =
    (a->enable_invalid_samples != 0);
  switch (a->invalid_sample_visibility)
  {
    case V_VISIBILITY_NO_INVALID_SAMPLES:
      xqos->reader_data_lifecycle.invalid_sample_visibility = NN_NO_INVALID_SAMPLE_VISIBILITY_QOS;
      break;
    case V_VISIBILITY_MINIMUM_INVALID_SAMPLES:
      xqos->reader_data_lifecycle.invalid_sample_visibility = NN_MINIMUM_INVALID_SAMPLE_VISIBILITY_QOS;
      break;
    case V_VISIBILITY_ALL_INVALID_SAMPLES:
      xqos->reader_data_lifecycle.invalid_sample_visibility = NN_ALL_INVALID_SAMPLE_VISIBILITY_QOS;
      break;
  }
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

static void qcp_user_data_pp (nn_xqos_t *xqos, const struct v_userDataPolicy *a)
{
  if (a->size > 0 && c_arraySize (a->value) > 0)
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

static void qcp_subscription_keys (nn_xqos_t *xqos, const struct v_userKeyPolicy *a)
{
  if (a->enable)
  {
    c_iter keys;
    unsigned index;
    char *key;

    xqos->present |= QP_PRISMTECH_SUBSCRIPTION_KEYS;
    xqos->subscription_keys.use_key_list = 1;

    /* Procedure taken from v_dataViewInit() */
    keys = c_splitString (a->expression, ", \t");
    if ((xqos->subscription_keys.key_list.n = c_iterLength (keys)) == 0)
      xqos->subscription_keys.key_list.strs = NULL;
    else
      xqos->subscription_keys.key_list.strs = os_malloc (xqos->subscription_keys.key_list.n * sizeof (*xqos->subscription_keys.key_list.strs));
    index = 0;
    while ((key = c_iterTakeFirst (keys)) != NULL)
    {
      assert (index < xqos->subscription_keys.key_list.n);
      xqos->subscription_keys.key_list.strs[index++] = key;
    }
    assert (index == xqos->subscription_keys.key_list.n);
    c_iterFree (keys);
  }
}

static void qcp_reader_lifespan (nn_xqos_t *xqos, const struct v_readerLifespanPolicy *a)
{
  if (a->used)
  {
    xqos->present |= QP_PRISMTECH_READER_LIFESPAN;
    xqos->reader_lifespan.use_lifespan = 1;
    xqos->reader_lifespan.duration = v_duration_to_ddsi_duration (a->duration);
  }
}

static void qcp_reader_share (nn_xqos_t *xqos, const struct v_sharePolicy *a)
{
  if (a->enable)
  {
    xqos->present |= QP_PRISMTECH_SHARE;
    xqos->share.enable = 1;
    xqos->share.name = os_strdup (a->name);
  }
}

static int qcp_specials (nn_xqos_t *xqos, const struct v_builtinUserDataPolicy *a)
{
  sd_serializer ser;
  sd_serializedData serdata;
  c_type type;
  char *value;
  struct name_value *nv;
  unsigned i, sz;
  int rc;

  /* Must at least be null-terminated ... */
  sz = c_arraySize (a->value);
  if (sz == 0)
    return 0;
  value = os_malloc (sz + 1);
  memcpy (value, a->value, sz);
  value[sz] = 0;

  /* ... and if it is, try deserialising it as XML for a sequence of
     name-value pairs ... */
  type = c_resolve (gv.ospl_base, "q_osplserModule::seq_name_value");
  ser = sd_serializerXMLNewTyped (type);
  serdata = sd_serializerFromString (ser, value);
  nv = sd_serializerDeserialize (ser, serdata);
  sd_serializedDataFree (serdata);
  rc = (nv == NULL) ? -1 : 0;
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
      TRACE (("qcp_specials: relaxed_qos_matching = %d\n", xqos->relaxed_qos_matching.value));
    }
  }
  c_free (nv);
  return rc;
}

int init_reader_qos (const struct v_subscriptionInfo *data, const struct v_dataReaderCMInfo *cmdata, nn_xqos_t *xqos, int interpret_user_data)
{
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

  qcp_history (xqos, &cmdata->history);
  qcp_rdlifecycle (xqos, &cmdata->reader_data_lifecycle);
  qcp_resource_limits (xqos, &cmdata->resource_limits);
  qcp_reader_lifespan (xqos, &cmdata->reader_lifespan);
  qcp_reader_share (xqos, &cmdata->share);
  qcp_subscription_keys(xqos, &cmdata->subscription_keys);

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

enum product_data_st_mode {
  PDST_WRITER,
  PDST_PARTICIPANT
};

enum product_data_st_param {
  PDST_TRANSPORT_PRIORITY,
  PDST_NODE_NAME,
  PDST_PID,
  PDST_PARTICIPANT_NAME,
  PDST_EXEC_NAME,
  PDST_SERVICE_TYPE
};

struct product_data_st {
  enum product_data_st_mode mode;
  os_address next_elem;
  os_address product_elem;
  os_address x_elem;
  enum product_data_st_param x_param;
  int nest;
  int ignore;
  nn_xqos_t *xqos;
  nn_plist_t *plist;
};

static int product_data_elem_open (void *vst, os_address parentinfo, os_address *eleminfo, const char *name)
{
  struct product_data_st *st = vst;
  OS_UNUSED_ARG (parentinfo);
  *eleminfo = ++st->next_elem;
  st->nest++;
  if (st->ignore)
    return 0;
  else if (st->nest == 1 && strcmp (name, "Product") == 0)
    st->product_elem = *eleminfo;
  else if (st->product_elem != 0 && st->nest == 2)
  {
    switch (st->mode)
    {
      case PDST_WRITER:
        if (strcmp (name, "transport_priority") == 0)
        {
          st->x_elem = *eleminfo;
          st->x_param = PDST_TRANSPORT_PRIORITY;
        }
        else
        {
          st->ignore = st->nest;
        }
        break;
      case PDST_PARTICIPANT:
        if (strcmp (name, "NodeName") == 0)
        {
          st->x_elem = *eleminfo;
          st->x_param = PDST_NODE_NAME;
        }
        else if (strcmp (name, "PID") == 0)
        {
          st->x_elem = *eleminfo;
          st->x_param = PDST_PID;
        }
        else if (strcmp (name, "ParticipantName") == 0)
        {
          st->x_elem = *eleminfo;
          st->x_param = PDST_PARTICIPANT_NAME;
        }
        else if (strcmp (name, "ExecName") == 0)
        {
          st->x_elem = *eleminfo;
          st->x_param = PDST_EXEC_NAME;
        }
        else if (strcmp (name, "ServiceType") == 0)
        {
          st->x_elem = *eleminfo;
          st->x_param = PDST_SERVICE_TYPE;
        }
        else
        {
          st->ignore = st->nest;
        }
        break;
    }
  }
  else
    st->ignore = st->nest;
  return 0;
}

static int product_data_elem_value (void *vst, os_address eleminfo, const char *value)
{
  struct product_data_st *st = vst;
  if (eleminfo == st->x_elem)
  {
    long val;
    int pos;
    switch (st->x_param)
    {
      case PDST_TRANSPORT_PRIORITY:
        if (sscanf (value, "%ld%n", &val, &pos) != 1 || value[pos] != 0)
          return -1;
        else
        {
          st->xqos->present |= QP_TRANSPORT_PRIORITY;
          st->xqos->transport_priority.value = (c_long) val;
        }
        break;
      case PDST_PARTICIPANT_NAME:
        st->plist->present |= PP_ENTITY_NAME;
        st->plist->entity_name = os_strdup (value);
        break;
      case PDST_EXEC_NAME:
        st->plist->present |= PP_PRISMTECH_EXEC_NAME;
        st->plist->exec_name = os_strdup (value);
        break;
      case PDST_NODE_NAME:
        st->plist->present |= PP_PRISMTECH_NODE_NAME;
        st->plist->node_name = os_strdup (value);
        break;
      case PDST_PID:
        if (sscanf (value, "%ld%n", &val, &pos) != 1 || value[pos] != 0)
          return -1;
        else
        {
          st->plist->present |= PP_PRISMTECH_PROCESS_ID;
          st->plist->process_id = (unsigned) val;
        }
        break;
      case PDST_SERVICE_TYPE:
        if (sscanf (value, "%ld%n", &val, &pos) != 1 || value[pos] != 0)
          return -1;
        else if (val >= 0 || val < (long) V_SERVICETYPE_COUNT)
        {
          st->plist->present |= PP_PRISMTECH_SERVICE_TYPE;
          st->plist->service_type = (unsigned) val;
        }
        break;
    }
  }
  return 0;
}

static int product_data_elem_close (void *vst, os_address eleminfo)
{
  struct product_data_st *st = vst;
  if (st->nest-- == st->ignore)
    st->ignore = 0;
  if (eleminfo == st->x_elem)
    st->x_elem = 0;
  if (eleminfo == st->product_elem)
    st->product_elem = 0;
  return 0;
}

static void product_data_error (void *vst, const char *msg, int line)
{
  OS_UNUSED_ARG (vst);
  OS_UNUSED_ARG (line);
  NN_WARNING1 ("in parsing internal product data: %s", msg);
}

static int qcp_writer_product_data (nn_xqos_t *xqos, const struct v_productDataPolicy *data)
{
  struct ut_xmlpState *xp;
  struct ut_xmlpCallbacks xpcb;
  struct product_data_st st;
  int ret;
  memset (&xpcb, 0, sizeof (xpcb));
  memset (&st, 0, sizeof (st));
  xpcb.elem_close = product_data_elem_close;
  xpcb.elem_data = product_data_elem_value;
  xpcb.elem_open = product_data_elem_open;
  xpcb.error = product_data_error;
  st.mode = PDST_WRITER;
  st.xqos = xqos;
  xp = ut_xmlpNewString (data->value, &st, &xpcb);
  ret = ut_xmlpParse (xp);
  ut_xmlpFree (xp);
  return ret;
}

int init_writer_qos (const struct v_publicationInfo *data, const struct v_dataWriterCMInfo *cmdata, v_topic ospl_topic, nn_xqos_t *xqos, int interpret_user_data)
{
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
  qcp_history (xqos, &cmdata->history);
  qcp_resource_limits (xqos, &cmdata->resource_limits);

  if (ospl_topic)
  {
    /* Contrary to the DDS spec, OpenSplice has no durability service QoS on the writer, but we use it for transient-local (and other implementations could well be using it for transient) */
    v_topicQos tqos = v_topicGetQos(ospl_topic);
    struct v_durabilityServicePolicy dsp;
    v_policyConvToExt_durability_service(&dsp, &tqos->durabilityService);
    qcp_durability_service(xqos, &dsp);
    c_free(tqos);
  }

  if (qcp_writer_product_data(xqos, &cmdata->product) < 0)
  {
    nn_xqos_fini (xqos);
    return -1;
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

int init_reader_qos_from_topicQos (const struct v_topicQos_s *data, struct nn_xqos *xqos)
{
  struct v_topicInfo info;
  if (v_topicQosFillTopicInfo (&info, data) != V_RESULT_OK)
    return -1;
  nn_xqos_init_empty (xqos);
  qcp_durability (xqos, &info.durability);
  qcp_durability_service (xqos, &info.durabilityService);
  qcp_deadline (xqos, &info.deadline);
  qcp_latency (xqos, &info.latency_budget);
  qcp_liveliness (xqos, &info.liveliness);
  qcp_reliability (xqos, &info.reliability);
  qcp_transport_priority (xqos, &info.transport_priority);
  qcp_lifespan (xqos, &info.lifespan);
  qcp_destination_order (xqos, &info.destination_order);
  qcp_history (xqos, &info.history);
  qcp_resource_limits (xqos, &info.resource_limits);
  qcp_ownership (xqos, &info.ownership);
  qcp_topic_data (xqos, &info.topic_data);
  c_free (info.topic_data.value);
  return 0;
}

int init_topic_plist (const struct v_topicInfo *data, struct nn_plist *ps)
{
  nn_xqos_t *xqos = &ps->qos;
  struct v_userKeyPolicy ukey;

  nn_plist_init_empty (ps);

  if ((xqos->topic_name = os_strdup (data->name)) == NULL)
    goto err;
  xqos->present |= QP_TOPIC_NAME;

  if ((xqos->type_name = os_strdup (data->type_name)) == NULL)
    goto err;
  xqos->present |= QP_TYPE_NAME;

  qcp_durability (xqos, &data->durability);
  qcp_durability_service (xqos, &data->durabilityService);
  qcp_deadline (xqos, &data->deadline);
  qcp_latency (xqos, &data->latency_budget);
  qcp_liveliness (xqos, &data->liveliness);
  qcp_reliability (xqos, &data->reliability);
  qcp_transport_priority (xqos, &data->transport_priority);
  qcp_lifespan (xqos, &data->lifespan);
  qcp_destination_order (xqos, &data->destination_order);
  qcp_history (xqos, &data->history);
  qcp_resource_limits (xqos, &data->resource_limits);
  qcp_ownership (xqos, &data->ownership);
  qcp_topic_data (xqos, &data->topic_data);

  /* Fake a v_userKeyPolicy so we can reuse the subscription_keys copy routine */
  ukey.enable = 1;
  ukey.expression = data->key_list;
  qcp_subscription_keys(xqos, &ukey);

  if ((ps->type_description = os_strdup (data->meta_data)) == NULL)
    goto err;
  ps->present |= PP_PRISMTECH_TYPE_DESCRIPTION;

  return 0;
err:
  nn_plist_fini (ps);
  return ERR_OUT_OF_MEMORY;
}

int init_cm_publisher_plist (const struct v_publisherCMInfo *data, const nn_guid_t *guid, struct nn_plist *ps)
{
  nn_xqos_t *xqos = &ps->qos;
  nn_plist_init_empty (ps);
  ps->present |= PP_GROUP_GUID | PP_PRISMTECH_GROUP_GID | PP_ENTITY_NAME;
  ps->group_guid = *guid;
  ps->group_gid = data->key;
  if ((ps->entity_name = os_strdup (data->name ? data->name : "")) == NULL)
    goto err;
  qcp_entity_factory (xqos, &data->entity_factory);
  qcp_partition (xqos, &data->partition);
  return 0;
err:
  nn_plist_fini (ps);
  return ERR_OUT_OF_MEMORY;
}

int init_cm_subscriber_plist (const struct v_subscriberCMInfo *data, const nn_guid_t *guid, struct nn_plist *ps)
{
  nn_xqos_t *xqos = &ps->qos;
  nn_plist_init_empty (ps);
  ps->present |= PP_GROUP_GUID | PP_PRISMTECH_GROUP_GID | PP_ENTITY_NAME;
  ps->group_guid = *guid;
  ps->group_gid = data->key;
  if ((ps->entity_name = os_strdup (data->name ? data->name : "")) == NULL)
    goto err;
  qcp_entity_factory (xqos, &data->entity_factory);
  qcp_share (xqos, &data->share);
  qcp_partition (xqos, &data->partition);
  return 0;
err:
  nn_plist_fini (ps);
  return ERR_OUT_OF_MEMORY;
}

int init_participant_plist (const struct v_participantInfo *data, const struct v_participantCMInfo *cmdata, struct nn_plist *ps)
{
  struct ut_xmlpState *xp;
  struct ut_xmlpCallbacks xpcb;
  struct product_data_st st;
  int ret;
  memset (&xpcb, 0, sizeof (xpcb));
  memset (&st, 0, sizeof (st));

  nn_plist_init_empty (ps);
  qcp_user_data_pp (&ps->qos, &data->user_data);

  xpcb.elem_close = product_data_elem_close;
  xpcb.elem_data = product_data_elem_value;
  xpcb.elem_open = product_data_elem_open;
  xpcb.error = product_data_error;
  st.mode = PDST_PARTICIPANT;
  st.plist = ps;
  st.xqos = &st.plist->qos;
  xp = ut_xmlpNewString (cmdata->product.value, &st, &xpcb);
  ret = ut_xmlpParse (xp);
  ut_xmlpFree (xp);
  if (ret < 0)
  {
    nn_plist_fini (ps);
    return ret;
  }

  /* FIXME: entity factory (add to builtin topic in kernel) */
#if 0
  plist.qos.present |= QP_PRISMTECH_ENTITY_FACTORY;
  plist.qos.entity_factory.autoenable_created_entities = !!kpp->qos->entityFactory.autoenable_created_entities;
#endif

  nn_plist_mergein_missing (ps, &gv.default_plist_pp);
  return 0;
}

static int qpc_user_data_pp (struct v_userDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_USER_DATA) || xqos->user_data.length == 0)
  {
    a->size = 0;
    a->value = NULL;
    return 0;
  }
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->user_data.length)) == NULL)
  {
    return ERR_OUT_OF_MEMORY;
  }
  else
  {
    a->size = (c_long) xqos->user_data.length;
    memcpy (a->value, xqos->user_data.value, xqos->user_data.length);
    return 0;
  }
}

static int qpc_product_data_cmparticipant (struct v_productDataPolicy *a, const struct proxy_participant *proxypp)
{
  /* replicate format generated in v_builtinCreateCMParticipantInfo() */
  static const char product_tag[] = "Product";
  static const char exec_name_tag[] = "ExecName";
  static const char participant_name_tag[] = "ParticipantName";
  static const char process_id_tag[] = "PID";
  static const char node_name_tag[] = "NodeName";
  static const char federation_id_tag[] = "FederationId";
  static const char vendor_id_tag[] = "VendorId";
  static const char service_type_tag[] = "ServiceType";
  const size_t cdata_overhead = 12; /* <![CDATA[ and ]]> */
  const size_t tag_overhead = 5; /* <> and </> */
  const nn_plist_t *plist = proxypp->plist;
  char pidstr[11]; /* unsigned 32-bits, so max < 5e9, or 10 chars + terminator */
  char federationidstr[20]; /* max 2 * unsigned 32-bits hex + separator, terminator */
  char vendoridstr[22]; /* max 2 * unsigned 32-bits + seperator, terminator */
  char servicetypestr[11]; /* unsigned 32-bits */
  unsigned servicetype;
  os_size_t len = 1 + 2*(sizeof(product_tag)-1) + tag_overhead;

  if (plist->present & PP_PRISMTECH_EXEC_NAME)
    len += 2*(sizeof(exec_name_tag)-1) + cdata_overhead + tag_overhead + strlen(plist->exec_name);
  if (plist->present & PP_ENTITY_NAME)
    len += 2*(sizeof(participant_name_tag)-1) + cdata_overhead + tag_overhead + strlen(plist->entity_name);
  if (plist->present & PP_PRISMTECH_PROCESS_ID)
  {
    int n = snprintf (pidstr, sizeof (pidstr), "%u", plist->process_id);
    assert (n > 0 && (os_size_t) n < sizeof (pidstr));
    len += 2*(sizeof(process_id_tag)-1) + tag_overhead + (size_t) n;
  }
  if (plist->present & PP_PRISMTECH_NODE_NAME)
    len += 2*(sizeof(node_name_tag)-1) + cdata_overhead + tag_overhead + strlen(plist->node_name);

  {
    int n = snprintf (vendoridstr, sizeof (vendoridstr), "%u.%u", plist->vendorid.id[0], plist->vendorid.id[1]);
    assert (n > 0 && (os_size_t) n < sizeof (vendoridstr));
    len += 2*(sizeof(vendor_id_tag)-1) + tag_overhead + (size_t) n;
  }

  {
    int n;
    if (vendor_is_opensplice (plist->vendorid))
      n = snprintf (federationidstr, sizeof (federationidstr), "%x", proxypp->e.guid.prefix.u[0]);
    else
      n = snprintf (federationidstr, sizeof (federationidstr), "%x:%x", proxypp->e.guid.prefix.u[0], proxypp->e.guid.prefix.u[1]);
    assert (n > 0 && (os_size_t) n < sizeof (federationidstr));
    len += 2*(sizeof(federation_id_tag)-1) + tag_overhead + (size_t) n;
  }

  if (plist->present & PP_PRISMTECH_SERVICE_TYPE)
    servicetype = plist->service_type;
  else if (proxypp->is_ddsi2_pp)
    servicetype = (unsigned) V_SERVICETYPE_DDSI2;
  else
    servicetype = (unsigned) V_SERVICETYPE_NONE;

  {
    int n = snprintf (servicetypestr, sizeof (servicetypestr), "%u", (unsigned) servicetype);
    assert (n > 0 && (os_size_t) n < sizeof (servicetypestr));
    len += 2*(sizeof(service_type_tag)-1) + tag_overhead + (size_t) n;
  }

  if ((a->value = c_stringMalloc (gv.ospl_base, len)) == NULL)
    return ERR_OUT_OF_MEMORY;

  {
    char *p = a->value;
    int n;
    n = snprintf (p, len, "<%s>", product_tag); assert (n >= 0 && (os_size_t) n < len); p += n; len -= (size_t) n;
    if (plist->present & PP_PRISMTECH_EXEC_NAME)
    {
      n = snprintf (p, len, "<%s><![CDATA[%s]]></%s>", exec_name_tag, plist->exec_name, exec_name_tag);
      assert (n >= 0 && (os_size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (plist->present & PP_ENTITY_NAME)
    {
      n = snprintf (p, len, "<%s><![CDATA[%s]]></%s>", participant_name_tag, plist->entity_name, participant_name_tag);
      assert (n >= 0 && (os_size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (plist->present & PP_PRISMTECH_PROCESS_ID)
    {
      n = snprintf (p, len, "<%s>%s</%s>", process_id_tag, pidstr, process_id_tag);
      assert (n >= 0 && (os_size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (plist->present & PP_PRISMTECH_NODE_NAME)
    {
      n = snprintf (p, len, "<%s><![CDATA[%s]]></%s>", node_name_tag, plist->node_name, node_name_tag);
      assert (n >= 0 && (os_size_t) n < len);
      p += n; len -= (size_t) n;
    }
    n = snprintf (p, len, "<%s>%s</%s>", federation_id_tag, federationidstr, federation_id_tag);
    assert (n >= 0 && (os_size_t) n < len);
    p += n; len -= (size_t) n;
    n = snprintf (p, len, "<%s>%s</%s>", vendor_id_tag, vendoridstr, vendor_id_tag);
    assert (n >= 0 && (os_size_t) n < len);
    p += n; len -= (size_t) n;

    {
      n = snprintf (p, len, "<%s>%s</%s>", service_type_tag, servicetypestr, service_type_tag);
      assert (n >= 0 && (os_size_t) n < len);
      p += n; len -= (size_t) n;
    }

    n = snprintf (p, len, "</%s>", product_tag);
    assert (n >= 0 && (os_size_t) n == len-1);
    (void) n;
  }
  return 0;
}

static int qpc_topic_name (c_string *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_TOPIC_NAME);
  *a = c_stringNew_s (gv.ospl_base, xqos->topic_name);
  return (*a == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int qpc_type_name (c_string *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_TYPE_NAME);
  *a = c_stringNew_s (gv.ospl_base, xqos->type_name);
  return (*a == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int qpc_entity_name (c_string *a, const char *name)
{
  *a = c_stringNew_s (gv.ospl_base, name);
  return (*a == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static int qpc_share_policy (struct v_sharePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_SHARE);
  a->enable = xqos->share.enable;
  a->name = c_stringNew_s (gv.ospl_base, xqos->share.name);
  return (a->name == NULL) ? ERR_OUT_OF_MEMORY : 0;
}

static void qpc_entity_factory (struct v_entityFactoryPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_ENTITY_FACTORY);
  a->autoenable_created_entities = xqos->entity_factory.autoenable_created_entities;
}

static void qpc_durability_policy (struct v_durabilityPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DURABILITY);
  switch (xqos->durability.kind)
  {
    case NN_VOLATILE_DURABILITY_QOS:
      a->kind = V_DURABILITY_VOLATILE;
      break;
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS:
      a->kind = V_DURABILITY_TRANSIENT_LOCAL;
      break;
    case NN_TRANSIENT_DURABILITY_QOS:
      a->kind = V_DURABILITY_TRANSIENT;
      break;
    case NN_PERSISTENT_DURABILITY_QOS:
      a->kind = V_DURABILITY_PERSISTENT;
      break;
  }
}

static void qpc_durability_service_policy (struct v_durabilityServicePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DURABILITY_SERVICE);
  switch (xqos->durability_service.history.kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
      a->history_kind = V_HISTORY_KEEPLAST;
      break;
    case NN_KEEP_ALL_HISTORY_QOS:
      a->history_kind = V_HISTORY_KEEPALL;
      break;
  }
  a->history_depth = xqos->durability_service.history.depth;
  a->max_instances = xqos->durability_service.resource_limits.max_instances;
  a->max_samples = xqos->durability_service.resource_limits.max_samples;
  a->max_samples_per_instance = xqos->durability_service.resource_limits.max_samples_per_instance;
  a->service_cleanup_delay = ddsi_duration_to_v_duration (xqos->durability_service.service_cleanup_delay);
}

static void qpc_deadline_policy (struct v_deadlinePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DEADLINE);
  a->period = ddsi_duration_to_v_duration (xqos->deadline.deadline);
}

static void qpc_latency_budget_policy (struct v_latencyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_LATENCY_BUDGET);
  a->duration = ddsi_duration_to_v_duration (xqos->latency_budget.duration);
}

static void qpc_liveliness_policy (struct v_livelinessPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_LIVELINESS);
  switch (xqos->liveliness.kind)
  {
    case NN_AUTOMATIC_LIVELINESS_QOS:
      a->kind = V_LIVELINESS_AUTOMATIC;
      break;
    case NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      a->kind = V_LIVELINESS_PARTICIPANT;
      break;
    case NN_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      a->kind = V_LIVELINESS_TOPIC;
      break;
  }
  a->lease_duration = ddsi_duration_to_v_duration (xqos->liveliness.lease_duration);
}

static void qpc_reliability_policy (struct v_reliabilityPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_RELIABILITY);
  assert (xqos->present & QP_PRISMTECH_SYNCHRONOUS_ENDPOINT);
  switch (xqos->reliability.kind)
  {
    case NN_BEST_EFFORT_RELIABILITY_QOS:
      a->kind = V_RELIABILITY_BESTEFFORT;
      break;
    case NN_RELIABLE_RELIABILITY_QOS:
      a->kind = V_RELIABILITY_RELIABLE;
      break;
  }
  a->max_blocking_time = ddsi_duration_to_v_duration (xqos->reliability.max_blocking_time);
  a->synchronous = xqos->synchronous_endpoint.value;
}

static void qpc_transport_priority_policy (struct v_transportPolicy *a, const nn_xqos_t *xqos)
{
  a->value = xqos->transport_priority.value;
}

static void qpc_lifespan_policy (struct v_lifespanPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_LIFESPAN);
  a->duration = ddsi_duration_to_v_duration (xqos->lifespan.duration);
}

static void qpc_destination_order_policy (struct v_orderbyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_DESTINATION_ORDER);
  switch (xqos->destination_order.kind)
  {
    case NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
      a->kind = V_ORDERBY_RECEPTIONTIME;
      break;
    case NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
      a->kind = V_ORDERBY_SOURCETIME;
      break;
  }
}

static void qpc_history_policy (struct v_historyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_HISTORY);
  switch (xqos->history.kind)
  {
    case NN_KEEP_LAST_HISTORY_QOS:
      a->kind = V_HISTORY_KEEPLAST;
      break;
    case NN_KEEP_ALL_HISTORY_QOS:
      a->kind = V_HISTORY_KEEPALL;
      break;
  }
  a->depth = xqos->history.depth;
}

static void qpc_resource_limits_policy (struct v_resourcePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_RESOURCE_LIMITS);
  a->max_instances = xqos->resource_limits.max_instances;
  a->max_samples = xqos->resource_limits.max_samples;
  a->max_samples_per_instance = xqos->resource_limits.max_samples_per_instance;
}


static int qpc_user_data_policy (struct v_builtinUserDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_USER_DATA) || xqos->user_data.length == 0)
    a->value = NULL;
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->user_data.length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    memcpy (a->value, xqos->user_data.value, xqos->user_data.length);
  return 0;
}

static void qpc_ownership_policy (struct v_ownershipPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_OWNERSHIP);
  switch (xqos->ownership.kind)
  {
    case NN_SHARED_OWNERSHIP_QOS:
      a->kind = V_OWNERSHIP_SHARED;
      break;
    case NN_EXCLUSIVE_OWNERSHIP_QOS:
      a->kind = V_OWNERSHIP_EXCLUSIVE;
      break;
  }
}

static void qpc_ownership_strength_policy (struct v_strengthPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_OWNERSHIP_STRENGTH);
  a->value = xqos->ownership_strength.value;
}

static void qpc_time_based_filter_policy (struct v_pacingPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_TIME_BASED_FILTER);
  a->minSeperation = ddsi_duration_to_v_duration (xqos->time_based_filter.minimum_separation);
}

static void qpc_presentation_policy (struct v_presentationPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRESENTATION);
  switch (xqos->presentation.access_scope)
  {
    case NN_INSTANCE_PRESENTATION_QOS:
      a->access_scope = V_PRESENTATION_INSTANCE;
      break;
    case NN_TOPIC_PRESENTATION_QOS:
      a->access_scope = V_PRESENTATION_TOPIC;
      break;
    case NN_GROUP_PRESENTATION_QOS:
      a->access_scope = V_PRESENTATION_GROUP;
      break;
  }
  a->coherent_access = xqos->presentation.coherent_access;
  a->ordered_access = xqos->presentation.ordered_access;
}

static int qpc_group_data_policy (struct v_builtinGroupDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_GROUP_DATA) || xqos->group_data.length == 0)
    a->value = NULL;
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->group_data.length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    memcpy (a->value, xqos->group_data.value, xqos->group_data.length);
  return 0;
}

static int qpc_topic_data_policy (struct v_builtinTopicDataPolicy *a, const nn_xqos_t *xqos)
{
  if (!(xqos->present & QP_TOPIC_DATA) || xqos->topic_data.length == 0)
    a->value = NULL;
  else if ((a->value = c_arrayNew_s (c_octet_t (gv.ospl_base), xqos->topic_data.length)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    memcpy (a->value, xqos->topic_data.value, xqos->topic_data.length);
  return 0;
}

static int qpc_partition_policy (struct v_builtinPartitionPolicy *a, const nn_xqos_t *xqos)
{
  const int present = (xqos->present & QP_PARTITION) != 0;
  c_type type = c_string_t (gv.ospl_base);
  a->name = c_arrayNew_s (type, (!present || xqos->partition.n == 0) ? 1 : xqos->partition.n);
  if (a->name == NULL)
    return ERR_OUT_OF_MEMORY;
  else
  {
    c_string *ns = (c_string *) a->name;
    if (!present || xqos->partition.n == 0)
    {
      if ((ns[0] = c_stringNew_s (gv.ospl_base, "")) == NULL)
        return ERR_OUT_OF_MEMORY;
    }
    else
    {
      unsigned i;
      for (i = 0; i < xqos->partition.n; i++)
      {
        if ((ns[i] = c_stringNew_s (gv.ospl_base, xqos->partition.strs[i])) == NULL)
          return ERR_OUT_OF_MEMORY;
      }
    }
  }
  return 0;
}

static void qpc_writer_data_lifecycle_policy (struct v_writerLifecyclePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_WRITER_DATA_LIFECYCLE);
  a->autodispose_unregistered_instances = xqos->writer_data_lifecycle.autodispose_unregistered_instances;
  a->autopurge_suspended_samples_delay = ddsi_duration_to_v_duration (xqos->writer_data_lifecycle.autopurge_suspended_samples_delay);
  a->autounregister_instance_delay = ddsi_duration_to_v_duration (xqos->writer_data_lifecycle.autounregister_instance_delay);
}

static void qpc_reader_data_lifecycle_policy (struct v_readerLifecyclePolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_READER_DATA_LIFECYCLE);
  a->autopurge_nowriter_samples_delay = ddsi_duration_to_v_duration (xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
  a->autopurge_disposed_samples_delay = ddsi_duration_to_v_duration (xqos->reader_data_lifecycle.autopurge_disposed_samples_delay);
  a->autopurge_dispose_all = xqos->reader_data_lifecycle.autopurge_dispose_all;
  a->enable_invalid_samples = xqos->reader_data_lifecycle.enable_invalid_samples;
  switch (xqos->reader_data_lifecycle.invalid_sample_visibility)
  {
    case NN_NO_INVALID_SAMPLE_VISIBILITY_QOS:
      a->invalid_sample_visibility = V_VISIBILITY_NO_INVALID_SAMPLES;
      break;
    case NN_MINIMUM_INVALID_SAMPLE_VISIBILITY_QOS:
      a->invalid_sample_visibility = V_VISIBILITY_MINIMUM_INVALID_SAMPLES;
      break;
    case NN_ALL_INVALID_SAMPLE_VISIBILITY_QOS:
      a->invalid_sample_visibility = V_VISIBILITY_ALL_INVALID_SAMPLES;
      break;
  }
}

static void qpc_reader_lifespan_policy (struct v_readerLifespanPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_READER_LIFESPAN);
  a->used = xqos->reader_lifespan.use_lifespan;
  a->duration = ddsi_duration_to_v_duration (xqos->reader_lifespan.duration);
}

static int qpc_subscription_keys_policy (struct v_userKeyPolicy *a, const nn_xqos_t *xqos)
{
  assert (xqos->present & QP_PRISMTECH_SUBSCRIPTION_KEYS);
  if (!xqos->subscription_keys.use_key_list || xqos->subscription_keys.key_list.n == 0)
  {
    a->enable = 0;
    a->expression = c_stringNew_s (gv.ospl_base, "");
    if (a->expression == NULL)
      return ERR_OUT_OF_MEMORY;
  }
  else
  {
    os_size_t len;
    char *p;
    unsigned i;
    a->enable = 1;
    /* subscription keys in topicInfo is a comma-separated list of keys, terminated by a 0, so string length is simply the length of each key increased by 1 */
    len = 0;
    for (i = 0; i < xqos->subscription_keys.key_list.n; i++)
      len += strlen (xqos->subscription_keys.key_list.strs[i]) + 1;

    a->expression = p = c_stringMalloc(gv.ospl_base, len);
    if (a->expression == NULL)
      return ERR_OUT_OF_MEMORY;
    for (i = 0; i < xqos->subscription_keys.key_list.n; i++)
    {
      os_size_t sz = strlen (xqos->subscription_keys.key_list.strs[i]);
      memcpy (p, xqos->subscription_keys.key_list.strs[i], sz);
      p += sz;
      *p++ = ','; /* for last one still be replaced with terminating 0 */
    }
    p[-1] = 0;
  }
  return 0;
}

static int qpc_topic_meta_data_and_key_list (c_string *md, c_string *kl, const nn_plist_t *datap)
{
  struct v_userKeyPolicy ukp;
  assert (datap->present & PP_PRISMTECH_TYPE_DESCRIPTION);

  *md = c_stringNew_s (gv.ospl_base, datap->type_description);
  if (*md == NULL)
    return ERR_OUT_OF_MEMORY;
  if (qpc_subscription_keys_policy (&ukp, &datap->qos) < 0)
    return ERR_OUT_OF_MEMORY;
  *kl = ukp.expression;
  return 0;
}

static v_copyin_result write_builtin_topic_copyin_heartbeatInfo (UNUSED_ARG (c_type type), UNUSED_ARG (const void *vdatap), UNUSED_ARG (void *vto))
{
  /* DCPSHeartbeat is written using v_groupWrite to control the
     network id and prevent it from leaving the machine, hence has no
     copy-in routine. */
  assert (0);
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_participantInfo (UNUSED_ARG (c_type type), const void *vproxypp, void *vto)
{
  /* copyin routine generates data on the fly, taking proxy participant as input (well, why not?) */
  const struct proxy_participant *proxypp = vproxypp;
  const nn_plist_t *plist = proxypp->plist;
  struct v_participantInfo *to = vto;
  to->key = proxypp->gid;
  if (qpc_user_data_pp (&to->user_data, &plist->qos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_participantCMInfo (UNUSED_ARG (c_type type), const void *vproxypp, void *vto)
{
  /* copyin routine generates data on the fly, taking proxy participant as input (well, why not?) */
  const struct proxy_participant *proxypp = vproxypp;
  struct v_participantCMInfo *to = vto;
  to->key = proxypp->gid;
  if (qpc_product_data_cmparticipant (&to->product, proxypp) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_publicationInfo (UNUSED_ARG (c_type type), const void *vpwr, void *vto)
{
  /* never called for DDSI built-in writers */
  const struct proxy_writer *pwr = vpwr;
  const nn_xqos_t *xqos = pwr->c.xqos;
  struct v_publicationInfo *to = vto;
  to->key = pwr->c.gid;
  to->participant_key = pwr->c.proxypp->gid;
  /* Note: topic gets set lazily, so may still be NULL, but the topic name is in the QoS */
  if (qpc_topic_name (&to->topic_name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_type_name (&to->type_name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_durability_policy (&to->durability, xqos);
  qpc_deadline_policy (&to->deadline, xqos);
  qpc_latency_budget_policy (&to->latency_budget, xqos);
  qpc_liveliness_policy (&to->liveliness, xqos);
  qpc_reliability_policy (&to->reliability, xqos);
  qpc_lifespan_policy (&to->lifespan, xqos);
  qpc_destination_order_policy (&to->destination_order, xqos);
  if (qpc_user_data_policy (&to->user_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_ownership_policy (&to->ownership, xqos);
  qpc_ownership_strength_policy (&to->ownership_strength, xqos);
  qpc_presentation_policy (&to->presentation, xqos);
  if (qpc_partition_policy (&to->partition, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_topic_data_policy (&to->topic_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_group_data_policy (&to->group_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_writer_data_lifecycle_policy (&to->lifecycle, xqos);
  to->alive = 1; /* FIXME -- depends on full implementation of liveliness */
  return V_COPYIN_RESULT_OK;
}

static size_t base64_length (size_t n)
{
  size_t pad = (n % 3) == 0 ? 0 : 3 - (n % 3);
  return 4 * (n + pad) / 3;
}

static int base64_encode (char *buf, size_t bufsz, const unsigned char *xs, size_t n)
{
  /* like snprintf in its behaviour */
  static const char *codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  char * const bufs = buf;
  const char * const bufe = buf + bufsz;
  size_t i;
  for (i = 0; i < n; i += 3)
  {
    unsigned char b = (unsigned char) ((xs[i] & 0xfc) >> 2);
    if (buf < bufe) *buf++ = codes[b];
    b = (unsigned char) ((xs[i] & 0x03) << 4);
    if (i + 1 < n)
    {
      b |= (unsigned char) ((xs[i + 1] & 0xf0) >> 4);
      if (buf < bufe) *buf++ = codes[b];
      b = (unsigned char) ((xs[i + 1] & 0x0f) << 2);
      if (i + 2 < n)
      {
        b |= (unsigned char) ((xs[i + 2] & 0xc0) >> 6);
        if (buf < bufe) *buf++ = codes[b];
        b = (unsigned char) (xs[i + 2] & 0x3f);
        if (buf < bufe) *buf++ = codes[b];
      }
      else
      {
        if (buf < bufe) *buf++ = codes[b];
        if (buf < bufe) *buf++ = '=';
      }
    }
    else
    {
      if (buf < bufe) *buf++ = codes[b];
      if (buf < bufe) *buf++ = '=';
      if (buf < bufe) *buf++ = '=';
    }
  }
  if (buf < bufe) *buf = 0;
  else if (bufsz) bufs[bufsz-1] = 0;
  return (int) (buf - bufs);
}

static v_copyin_result write_builtin_topic_copyin_dataWriterCMInfo (UNUSED_ARG (c_type type), const void *vpwr, void *vto)
{
  /* never called for DDSI built-in writers */
  const struct proxy_writer *pwr = vpwr;
  const nn_xqos_t *xqos = pwr->c.xqos;
  struct v_dataWriterCMInfo *to = vto;

  static const char product_tag[] = "Product";
  static const char transport_priority_tag[] = "transport_priority";
  static const char rti_typecode_tag[] = "rti_typecode";
  const size_t tag_overhead = 5; /* <> and </> */
  char tpriostr[12]; /* signed 32-bits, so min ~= -2e9, or 11 chars + terminator */
  size_t len = 1 + 2*(sizeof(product_tag)-1) + tag_overhead;
  int product_string_needed = 0;

  if (xqos->transport_priority.value != 0) {
    int n = snprintf (tpriostr, sizeof (tpriostr), "%d", (int) xqos->transport_priority.value);
    assert (n > 0 && (size_t) n < sizeof (tpriostr));
    len += 2*(sizeof(transport_priority_tag)-1) + tag_overhead + (size_t) n;
    product_string_needed = 1;
  }

  if (xqos->present & QP_RTI_TYPECODE) {
    len += 2*(sizeof(rti_typecode_tag)-1) + tag_overhead + base64_length(xqos->rti_typecode.length);
    product_string_needed = 1;
  }

  to->key = pwr->c.gid;
  if (product_string_needed == 0)
    to->product.value = c_stringNew (gv.ospl_base, "");
  else
  {
    char *p;
    int n;
    if ((to->product.value = c_stringMalloc_s (gv.ospl_base, len)) == NULL)
      return V_COPYIN_RESULT_OUT_OF_MEMORY;
    p = to->product.value;
    n = snprintf (p, len, "<%s>", product_tag); assert (n >= 0 && (size_t) n < len); p += n; len -= (size_t) n;
    if (xqos->transport_priority.value != 0) {
      n = snprintf (p, len, "<%s>%s</%s>", transport_priority_tag, tpriostr, transport_priority_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    if (xqos->present & QP_RTI_TYPECODE)
    {
      n = snprintf (p, len, "<%s>", rti_typecode_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
      n = base64_encode (p, len, xqos->rti_typecode.value, xqos->rti_typecode.length);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
      n = snprintf (p, len, "</%s>", rti_typecode_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    n = snprintf (p, len, "</%s>", product_tag);
    assert (n >= 0 && (os_size_t) n == len-1);
    (void) n;
  }
  to->publisher_key = pwr->c.group_gid;
  if (qpc_entity_name (&to->name, pwr->e.name) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_history_policy (&to->history, xqos);
  qpc_resource_limits_policy (&to->resource_limits, xqos);
  qpc_writer_data_lifecycle_policy (&to->writer_data_lifecycle, xqos);
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_subscriptionInfo (UNUSED_ARG (c_type type), const void *vprd, void *vto)
{
  /* never called for DDSI built-in readers */
  const struct proxy_reader *prd = vprd;
  const nn_xqos_t *xqos = prd->c.xqos;
  struct v_subscriptionInfo *to = vto;
  to->key = prd->c.gid;
  to->participant_key = prd->c.proxypp->gid;
  /* Note: topic gets set lazily, so may still be NULL, but the topic name is in the QoS */
  if (qpc_topic_name (&to->topic_name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_type_name (&to->type_name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_durability_policy (&to->durability, xqos);
  qpc_deadline_policy (&to->deadline, xqos);
  qpc_latency_budget_policy (&to->latency_budget, xqos);
  qpc_liveliness_policy (&to->liveliness, xqos);
  qpc_reliability_policy (&to->reliability, xqos);
  qpc_ownership_policy (&to->ownership, xqos);
  qpc_destination_order_policy (&to->destination_order, xqos);
  if (qpc_user_data_policy (&to->user_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_time_based_filter_policy (&to->time_based_filter, xqos);
  qpc_presentation_policy (&to->presentation, xqos);
  if (qpc_partition_policy (&to->partition, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_topic_data_policy (&to->topic_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_group_data_policy (&to->group_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_reader_lifespan_policy (&to->lifespan, xqos);
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_dataReaderCMInfo (UNUSED_ARG (c_type type), const void *vprd, void *vto)
{
  /* never called for DDSI built-in readers */
  const struct proxy_reader *prd = vprd;
  const nn_xqos_t *xqos = prd->c.xqos;
  struct v_dataReaderCMInfo *to = vto;
  static const char product_tag[] = "Product";
  static const char rti_typecode_tag[] = "rti_typecode";
  const size_t tag_overhead = 5; /* <> and </> */
  size_t len = 1 + 2*(sizeof(product_tag)-1) + tag_overhead;
  int product_string_needed = 0;

  if (xqos->present & QP_RTI_TYPECODE) {
    len += 2*(sizeof(rti_typecode_tag)-1) + tag_overhead + base64_length(xqos->rti_typecode.length);
    product_string_needed = 1;
  }

  to->key = prd->c.gid;
  if (product_string_needed == 0)
    to->product.value = c_stringNew (gv.ospl_base, "");
  else
  {
    char *p;
    int n;
    if ((to->product.value = c_stringMalloc_s (gv.ospl_base, len)) == NULL)
      return V_COPYIN_RESULT_OUT_OF_MEMORY;
    p = to->product.value;
    n = snprintf (p, len, "<%s>", product_tag); assert (n >= 0 && (size_t) n < len); p += n; len -= (size_t) n;
    if (xqos->present & QP_RTI_TYPECODE)
    {
      n = snprintf (p, len, "<%s>", rti_typecode_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
      n = base64_encode (p, len, xqos->rti_typecode.value, xqos->rti_typecode.length);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
      n = snprintf (p, len, "</%s>", rti_typecode_tag);
      assert (n >= 0 && (size_t) n < len);
      p += n; len -= (size_t) n;
    }
    n = snprintf (p, len, "</%s>", product_tag);
    assert (n >= 0 && (os_size_t) n == len-1);
    (void) n;
  }
  to->subscriber_key = prd->c.group_gid;
  if (qpc_entity_name (&to->name, prd->e.name) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_history_policy (&to->history, xqos);
  qpc_resource_limits_policy (&to->resource_limits, xqos);
  qpc_reader_data_lifecycle_policy (&to->reader_data_lifecycle, xqos);
  if (qpc_subscription_keys_policy (&to->subscription_keys, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_reader_lifespan_policy (&to->reader_lifespan, xqos);
  qpc_share_policy (&to->share, xqos);
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_publisherCMInfo (UNUSED_ARG (c_type type), const void *vpgroup, void *vto)
{
  /* never called for DDSI built-in readers */
  const struct proxy_group *pgroup = vpgroup;
  const nn_xqos_t *xqos = pgroup->xqos;
  struct v_publisherCMInfo *to = vto;
  to->key = pgroup->gid;
  if ((to->product.value = c_stringNew_s (gv.ospl_base, "")) == NULL)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  to->participant_key = pgroup->proxypp->gid;
  if (qpc_entity_name (&to->name, pgroup->name) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_entity_factory (&to->entity_factory, xqos);
  if (qpc_partition_policy (&to->partition, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_subscriberCMInfo (UNUSED_ARG (c_type type), const void *vpgroup, void *vto)
{
  /* never called for DDSI built-in readers */
  const struct proxy_group *pgroup = vpgroup;
  const nn_xqos_t *xqos = pgroup->xqos;
  struct v_subscriberCMInfo *to = vto;
  to->key = pgroup->gid;
  if ((to->product.value = c_stringNew_s (gv.ospl_base, "")) == NULL)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  to->participant_key = pgroup->proxypp->gid;
  if (qpc_entity_name (&to->name, pgroup->name) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_entity_factory (&to->entity_factory, xqos);
  qpc_share_policy (&to->share, xqos);
  if (qpc_partition_policy (&to->partition, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  return V_COPYIN_RESULT_OK;
}

static v_copyin_result write_builtin_topic_copyin_topicInfo (UNUSED_ARG (c_type type), const void *vdatap, void *vto)
{
  /* never called for DDSI built-in readers */
  const nn_plist_t *datap = vdatap;
  const nn_xqos_t *xqos = &datap->qos;
  struct v_topicInfo *to = vto;
  to->key.systemId = ut_crcCalculate (xqos->topic_name, strlen (xqos->topic_name));
  to->key.localId = ut_crcCalculate (xqos->type_name, strlen (xqos->type_name));
  to->key.serial = 0;
  /* Note: topic gets set lazily, so may still be NULL, but the topic name is in the QoS */
  if (qpc_topic_name (&to->name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_type_name (&to->type_name, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  qpc_durability_policy (&to->durability, xqos);
  qpc_durability_service_policy (&to->durabilityService, xqos);
  qpc_deadline_policy (&to->deadline, xqos);
  qpc_latency_budget_policy (&to->latency_budget, xqos);
  qpc_liveliness_policy (&to->liveliness, xqos);
  qpc_reliability_policy (&to->reliability, xqos);
  qpc_transport_priority_policy (&to->transport_priority, xqos);
  qpc_lifespan_policy (&to->lifespan, xqos);
  qpc_destination_order_policy (&to->destination_order, xqos);
  qpc_history_policy (&to->history, xqos);
  qpc_resource_limits_policy (&to->resource_limits, xqos);
  qpc_ownership_policy (&to->ownership, xqos);
  if (qpc_topic_data_policy (&to->topic_data, xqos) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  if (qpc_topic_meta_data_and_key_list (&to->meta_data, &to->key_list, datap) < 0)
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
  return V_COPYIN_RESULT_OK;
}

void write_builtin_topic_proxy_participant (const struct proxy_participant *proxypp)
{
  /* Note: we discover/create/call write_builtin... for the proxy participant
   based on the arrival of the SPDP data, but this usually occurs before we have
   the CM data as well (strictly speaking, it could be that the CM data arrives
   in parallel to the creation of the proxy participant, and if builtin data can
   be processed in parallel (which currently it can't be, but who knows when that
   might change) it might overtake it.

   So for OpenSplice, there's no point in publishing CM stuff here; but perhaps
   we should when talking to, e.g., RTI. */
  u_result res;
  struct faked_systems *node;

  os_mutexLock(&faked_systems_lock);
  node = ut_avlLookup(&faked_systems_treedef, &faked_systems_tree, &proxypp->gid.systemId);
  if (node == NULL) {
    node = os_malloc(sizeof(*node));
    node->participant_count = 1;
    node->systemId = proxypp->gid.systemId;
    ut_avlInsert(&faked_systems_treedef, &faked_systems_tree, node);
    (void)u_builtinWriteFakeHeartbeat(
        heartbeatInfo_wr, proxypp->gid.systemId, L_WRITE);
  } else {
    node->participant_count++;
  }
  os_mutexUnlock(&faked_systems_lock);

  if ((res = u_writerWrite (participantInfo_wr, write_builtin_topic_copyin_participantInfo, (void *) proxypp, os_timeWGet(), U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("write_builtin_topic_proxy_participant: u_writerWrite error %d\n", (int) res);
}

void write_builtin_topic_proxy_participant_cm (const struct proxy_participant *proxypp)
{
  u_result res;
  if ((res = u_writerWrite (participantCMInfo_wr, write_builtin_topic_copyin_participantCMInfo, (void *) proxypp, os_timeWGet(), U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("write_builtin_topic_proxy_participant_cm: u_writerWrite error %d\n", (int) res);
}

void dispose_builtin_topic_proxy_participant (const struct proxy_participant *proxypp, int isimplicit)
{
  /* Note: dispose_... is called after all endpoints have been cleaned up and are
     no longer referenced anywhere, as it is called when the number of references to
     the proxy participant drops to 0.  So at this point, we can safely
     dispose/unregister the CM data.  But we do need to writeDispose it, as it may not
     have been published yet (no data, or just not received yet). */
  const os_timeW tnow = os_timeWGet();
  u_result res;
  struct faked_systems *node;

  TRACE (("dispose_builtin_topic_proxy_participant %x:%x:%x%s\n", PGID (proxypp->gid), isimplicit ? " implicit" : ""));
  if ((res = u_writerWriteDispose (participantInfo_wr, write_builtin_topic_copyin_participantInfo, (void *) proxypp, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_participant: u_writerWriteDispose error %d\n", (int) res);
  if ((res = u_writerWriteDispose (participantCMInfo_wr, write_builtin_topic_copyin_participantCMInfo, (void *) proxypp, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_participant[cm]: u_writerWriteDispose error %d\n", (int) res);
  if ((res = u_writerUnregisterInstance (participantInfo_wr, write_builtin_topic_copyin_participantInfo, (void *) proxypp, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_participant: u_writerUnregisterInstance error %d\n", (int) res);
  if ((res = u_writerUnregisterInstance (participantCMInfo_wr, write_builtin_topic_copyin_participantCMInfo, (void *) proxypp, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_participant[cm]: u_writerUnregisterInstance error %d\n", (int) res);

  os_mutexLock(&faked_systems_lock);
  node = ut_avlLookup(&faked_systems_treedef, &faked_systems_tree, &proxypp->gid.systemId);
  if (node != NULL) {
    if (--node->participant_count == 0) {
      v_state state = L_UNREGISTER;
      if (isimplicit)
        state |= L_IMPLICIT;
      ut_avlDelete(&faked_systems_treedef, &faked_systems_tree, node);
      os_free(node);
      (void)u_builtinWriteFakeHeartbeat(heartbeatInfo_wr, proxypp->gid.systemId, state);
    }
  }
  os_mutexUnlock(&faked_systems_lock);
}

void write_builtin_topic_proxy_writer (const struct proxy_writer *pwr)
{
  const os_timeW tnow = os_timeWGet();
  u_result res;

  /* NOTE: If and when QoS changes are implemented, need to make sure
     disposes and unregisters are generated properly by DDSI - we're
     no longer relying on spliced to do that. */

  if ((res = u_writerWrite (publicationInfo_wr, write_builtin_topic_copyin_publicationInfo, (void *) pwr, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("write_builtin_topic_proxy_writer: u_writerWrite error %d\n", (int) res);
  if ((res = u_writerWrite (dataWriterCMInfo_wr, write_builtin_topic_copyin_dataWriterCMInfo, (void *) pwr, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("write_builtin_topic_proxy_writer[cm]: u_writerWrite error %d\n", (int) res);
}

struct disconnect_writer_arg {
  u_writer uwriter;
  struct v_publicationInfo *pubinfo;
  c_bool isimplicit;
  os_timeW timestamp;
  v_group group;
};

static void disconnect_writer_action (v_public dummy, void *varg)
{
  struct disconnect_writer_arg *arg = varg;
  v_group g = arg->group;
  OS_UNUSED_ARG (dummy);
  if (strcmp (v_partitionName (v_groupPartition (g)), V_BUILTIN_PARTITION) != 0 ||
      (strcmp (v_topicName (v_groupTopic (g)), V_TOPICINFO_NAME) != 0 &&
       strcmp (v_topicName (v_groupTopic (g)), V_TYPEINFO_NAME) != 0))
  {
    v_groupDisconnectWriter (g, arg->pubinfo, arg->timestamp, 0, arg->isimplicit);
  }
}

static int disconnect_writer_helper (v_group g, void *varg)
{
  struct disconnect_writer_arg *arg = varg;
  arg->group = g;
  (void) u_observableAction (u_observable (arg->uwriter), disconnect_writer_action, arg);
  return 0;
}

void dispose_builtin_topic_proxy_writer (const struct proxy_writer *pwr, int isimplicit)
{
  const os_timeW tnow = os_timeWGet();
  struct v_publicationInfo pubinfo;
  struct disconnect_writer_arg arg;
  u_result res;
  TRACE (("dispose_builtin_topic_proxy_writer %x:%x:%x%s\n", PGID (pwr->c.gid), isimplicit ? " implicit" : ""));

  memset (&pubinfo, 0, sizeof (pubinfo));
  pubinfo.key = pwr->c.gid;
  arg.uwriter = publicationInfo_wr;
  arg.pubinfo = &pubinfo;
  arg.isimplicit = (isimplicit != 0);
  arg.timestamp = tnow;
  (void) nn_groupset_foreach (pwr->groups, disconnect_writer_helper, &arg);

  if ((res = u_writerWriteDispose (publicationInfo_wr, write_builtin_topic_copyin_publicationInfo, (void *) pwr, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_writer: u_writerWriteDispose error %d\n", (int) res);
  if ((res = u_writerWriteDispose (dataWriterCMInfo_wr, write_builtin_topic_copyin_dataWriterCMInfo, (void *) pwr, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_writer[cm]: u_writerWriteDispose error %d\n", (int) res);
  if ((res = u_writerUnregisterInstance (publicationInfo_wr, write_builtin_topic_copyin_publicationInfo, (void *) pwr, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_writer: u_writerUnregisterInstance error %d\n", (int) res);
  if ((res = u_writerUnregisterInstance (dataWriterCMInfo_wr, write_builtin_topic_copyin_dataWriterCMInfo, (void *) pwr, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_writer[cm]: u_writerUnregisterInstance error %d\n", (int) res);
}

void write_builtin_topic_proxy_reader (const struct proxy_reader *prd)
{
  const os_timeW tnow = os_timeWGet();
  u_result res;
  if ((res = u_writerWrite (subscriptionInfo_wr, write_builtin_topic_copyin_subscriptionInfo, (void *) prd, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("write_builtin_topic_proxy_reader: u_writerWrite error %d\n", (int) res);
  if ((res = u_writerWrite (dataReaderCMInfo_wr, write_builtin_topic_copyin_dataReaderCMInfo, (void *) prd, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("write_builtin_topic_proxy_reader[cm]: u_writerWrite error %d\n", (int) res);
}

void dispose_builtin_topic_proxy_reader (const struct proxy_reader *prd, int isimplicit)
{
  const os_timeW tnow = os_timeWGet();
  u_result res;
  TRACE (("dispose_builtin_topic_proxy_reader %x:%x:%x%s\n", PGID (prd->c.gid), isimplicit ? " implicit" : ""));
  if ((res = u_writerWriteDispose (subscriptionInfo_wr, write_builtin_topic_copyin_subscriptionInfo, (void *) prd, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_reader: u_writerWriteDispose error %d\n", (int) res);
  if ((res = u_writerWriteDispose (dataReaderCMInfo_wr, write_builtin_topic_copyin_dataReaderCMInfo, (void *) prd, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_reader[cm]: u_writerWriteDispose error %d\n", (int) res);
  if ((res = u_writerUnregisterInstance (subscriptionInfo_wr, write_builtin_topic_copyin_subscriptionInfo, (void *) prd, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_reader: u_writerUnregisterInstance error %d\n", (int) res);
  if ((res = u_writerUnregisterInstance (dataReaderCMInfo_wr, write_builtin_topic_copyin_dataReaderCMInfo, (void *) prd, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
    NN_ERROR1 ("dispose_builtin_topic_proxy_reader[cm]: u_writerUnregisterInstance error %d\n", (int) res);
}

void write_builtin_topic_proxy_group (const struct proxy_group *pgroup)
{
  const os_timeW tnow = os_timeWGet();
  u_result res;
  TRACE (("write_builtin_topic_proxy_group %x:%x:%x:%x gid %x:%x:%x\n", PGUID (pgroup->guid), PGID (pgroup->gid)));
  switch (pgroup->guid.entityid.u & (NN_ENTITYID_SOURCE_MASK | NN_ENTITYID_KIND_MASK))
  {
    case NN_ENTITYID_SOURCE_VENDOR | NN_ENTITYID_KIND_PRISMTECH_PUBLISHER:
      if ((res = u_writerWrite (publisherCMInfo_wr, write_builtin_topic_copyin_publisherCMInfo, (void *) pgroup, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
        NN_ERROR1 ("write_builtin_topic_proxy_group[pub]: u_writerWrite error %d\n", (int) res);
      break;
    case NN_ENTITYID_SOURCE_VENDOR | NN_ENTITYID_KIND_PRISMTECH_SUBSCRIBER:
      if ((res = u_writerWrite (subscriberCMInfo_wr, write_builtin_topic_copyin_subscriberCMInfo, (void *) pgroup, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
        NN_ERROR1 ("write_builtin_topic_proxy_group[sub]: u_writerWrite error %d\n", (int) res);
      break;
  }
}

void dispose_builtin_topic_proxy_group (const struct proxy_group *pgroup, int isimplicit)
{
  const os_timeW tnow = os_timeWGet();
  u_result res;
  TRACE (("dispose_builtin_topic_proxy_group %x:%x:%x:%x gid %x:%x:%x%s\n", PGUID (pgroup->guid), PGID (pgroup->gid), isimplicit ? " implicit" : ""));
  switch (pgroup->guid.entityid.u & (NN_ENTITYID_SOURCE_MASK | NN_ENTITYID_KIND_MASK))
  {
    case NN_ENTITYID_SOURCE_VENDOR | NN_ENTITYID_KIND_PRISMTECH_PUBLISHER:
      if ((res = u_writerWriteDispose (publisherCMInfo_wr, write_builtin_topic_copyin_publisherCMInfo, (void *) pgroup, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
        NN_ERROR1 ("dispose_builtin_topic_proxy_group[pub]: u_writerWriteDispose error %d\n", (int) res);
      if ((res = u_writerUnregisterInstance (publisherCMInfo_wr, write_builtin_topic_copyin_publisherCMInfo, (void *) pgroup, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
        NN_ERROR1 ("dispose_builtin_topic_proxy_group[pub]: u_writerUnregisterInstance error %d\n", (int) res);
      break;
    case NN_ENTITYID_SOURCE_VENDOR | NN_ENTITYID_KIND_PRISMTECH_SUBSCRIBER:
      if ((res = u_writerWriteDispose (subscriberCMInfo_wr, write_builtin_topic_copyin_subscriberCMInfo, (void *) pgroup, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
        NN_ERROR1 ("dispose_builtin_topic_proxy_group[sub]: u_writerWriteDispose error %d\n", (int) res);
      if ((res = u_writerUnregisterInstance (subscriberCMInfo_wr, write_builtin_topic_copyin_subscriberCMInfo, (void *) pgroup, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
        NN_ERROR1 ("dispose_builtin_topic_proxy_group[sub]: u_writerUnregisterInstance error %d\n", (int) res);
      break;
  }
}

void write_builtin_topic_proxy_topic (const struct nn_plist *datap)
{
  const char *tpname;
  assert (datap->qos.present & QP_TOPIC_NAME);
  tpname = datap->qos.topic_name;
  if (!is_topic_discoverable(tpname))
    TRACE ((" (ignoring built-in topic %s)", tpname));
  else
  {
    const os_timeW tnow = os_timeWGet();
    u_result res;
    if ((res = u_writerWrite (topicInfo_wr, write_builtin_topic_copyin_topicInfo, (void *) datap, tnow, U_INSTANCEHANDLE_NIL)) != U_RESULT_OK)
      NN_ERROR1 ("write_builtin_topic_proxy_topic: u_writerWrite error %d\n", (int) res);
  }
}

static const struct {
  v_infoId tpid;
  u_writer *wrp;
  u_writerCopy copyin;
} builtin_topic_wrdefs[] = {
  { V_HEARTBEATINFO_ID, &heartbeatInfo_wr, write_builtin_topic_copyin_heartbeatInfo},
  { V_PARTICIPANTINFO_ID, &participantInfo_wr, write_builtin_topic_copyin_participantInfo},
  { V_CMPARTICIPANTINFO_ID, &participantCMInfo_wr, write_builtin_topic_copyin_participantCMInfo},
  { V_PUBLICATIONINFO_ID, &publicationInfo_wr, write_builtin_topic_copyin_publicationInfo},
  { V_SUBSCRIPTIONINFO_ID, &subscriptionInfo_wr, write_builtin_topic_copyin_subscriptionInfo},
  { V_TOPICINFO_ID, &topicInfo_wr, write_builtin_topic_copyin_topicInfo},
  { V_CMDATAWRITERINFO_ID, &dataWriterCMInfo_wr, write_builtin_topic_copyin_dataWriterCMInfo},
  { V_CMDATAREADERINFO_ID, &dataReaderCMInfo_wr, write_builtin_topic_copyin_dataReaderCMInfo},
  { V_CMPUBLISHERINFO_ID, &publisherCMInfo_wr, write_builtin_topic_copyin_publisherCMInfo},
  { V_CMSUBSCRIBERINFO_ID, &subscriberCMInfo_wr, write_builtin_topic_copyin_subscriberCMInfo},
};
static v_gid builtin_topic_writer_gids[sizeof (builtin_topic_wrdefs) / sizeof (builtin_topic_wrdefs[0])];

u_result create_builtin_topic_writers (u_participant p)
{
  v_publisherQos pqos = NULL;
  int i;

  if ((pqos = u_publisherQosNew (NULL)) == NULL)
    goto err_pqos;
  os_free (pqos->partition.v);
  pqos->partition.v = os_strdup (V_BUILTIN_PARTITION);
  pqos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
  if ((builtin_topic_publisher = u_publisherNew (p, "ddsi2 builtin topic publisher", pqos, TRUE)) == NULL)
    goto err_publisher;

  for (i = 0; i < (int) (sizeof (builtin_topic_wrdefs) / sizeof (builtin_topic_wrdefs[0])); i++)
  {
    if ((*builtin_topic_wrdefs[i].wrp = u_builtinWriterNew (builtin_topic_publisher, builtin_topic_wrdefs[i].tpid)) == NULL)
    {
      while (i--)
        u_objectFree(u_object(*builtin_topic_wrdefs[i].wrp));
      goto err_writer;
    }

    u_entityEnable(u_entity(*builtin_topic_wrdefs[i].wrp));
    builtin_topic_writer_gids[i] = u_observableGid(u_observable (*builtin_topic_wrdefs[i].wrp));
  }
  ut_avlInit(&faked_systems_treedef, &faked_systems_tree);
  os_mutexInit (&faked_systems_lock, NULL);

  u_publisherQosFree (pqos);
  return U_RESULT_OK;

 err_writer:
  u_objectFree (u_object(builtin_topic_publisher));
 err_publisher:
  u_publisherQosFree (pqos);
 err_pqos:
  return U_RESULT_INTERNAL_ERROR;
}

int is_builtin_topic_writer (const struct v_gid_s *gid)
{
  int i;
  if (gid->systemId != builtin_topic_writer_gids[0].systemId)
    return 0;
  for (i = 0; i < (int) (sizeof (builtin_topic_writer_gids) / sizeof (builtin_topic_writer_gids[0])); i++)
    if (gid->localId == builtin_topic_writer_gids[i].localId && gid->serial == builtin_topic_writer_gids[i].serial)
      return 1;
  return 0;
}

void destroy_builtin_topic_writers (void)
{
  int i;
  for (i = 0; i < (int) (sizeof (builtin_topic_wrdefs) / sizeof (builtin_topic_wrdefs[0])); i++)
    (void) u_objectFree (u_object(*builtin_topic_wrdefs[i].wrp));
  (void) u_objectFree (u_object(builtin_topic_publisher));
  ut_avlFree(&faked_systems_treedef, &faked_systems_tree, os_free);
  os_mutexDestroy (&faked_systems_lock);
}

/* SHA1 not available (unoffical build.) */
