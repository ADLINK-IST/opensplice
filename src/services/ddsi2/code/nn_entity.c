/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "nn_entity.h"

#include "nn_rtps.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_state.h"
#include "v__messageQos.h"
#include "kernelModule.h"
#include "ut_collection.h"
#include "u_participant.h"
#include "os_abstract.h" /* big or little endianness */

#include "nn_ddsi2.h"
#include "nn_config.h"
#include "nn_log.h"
#include "nn_md5.h"
#include "nn_xqos.h"
#include "nn_unused.h"

static ut_table participants = NULL;
static ut_table dataReaders = NULL;
static ut_table dataReadersGuid = NULL;
static ut_table dataWriters = NULL;
static ut_table dataWritersGuid = NULL;

static v_kernel kernel = NULL;
static c_type messageQosType = NULL;

static unsigned lastAdminChecksum = 0;
static volatile int enable_adminChecksum = 0;

OS_STRUCT(in_entity){
  int dummy;
};

OS_STRUCT(in_participant){
    OS_EXTENDS(in_entity);
    v_builtinTopicKey ddsParticipantKey;
    struct participant* rtpsParticipant;
    nn_guid_prefix_t prefix;
};

OS_STRUCT(in_reader){
    OS_EXTENDS(in_entity);
    in_participant participant;
    v_builtinTopicKey ddsReaderKey;
    struct reader* rtpsReader;
    nn_guid_t id;
    v_topic topic;
};

OS_STRUCT(in_writer){
    OS_EXTENDS(in_entity);
    in_participant participant;
    v_builtinTopicKey ddsWriterKey;
    struct writer* rtpsWriter;
    nn_guid_t id;
    v_topic topic; /* selected arbitrarily from the groups the writer is attached to */
};

OS_STRUCT(in_group)
{
    v_group vgroup;
};

static void
resolveKernel(
    v_entity entity,
    c_voidp args);

static os_equality
in_gidCompare(
    v_gid* g1,
    v_gid* g2,
    void* args);

static os_equality
in_guidCompare(
    nn_guid_t* g1,
    nn_guid_t* g2,
    void* args);

static void
in_entityInit(
    in_entity entity);

static void
in_entityDeinit(
    in_entity entity);

void
reportEntities()
{
    if(ut_count(ut_collection(participants))>0){
        nn_log (LC_TRACE, "DomainParticipants:\n");
        ut_walk(ut_collection(participants),
                (ut_actionFunc)in_participantReport, NULL);
    }
    if(ut_count(ut_collection(dataWriters))>0){
        nn_log (LC_TRACE, "DataWriters:\n");
        ut_walk(ut_collection(dataWriters),
                (ut_actionFunc)in_writerReport, NULL);
    }
    if(ut_count(ut_collection(dataReaders))>0){
        nn_log (LC_TRACE, "DataReaders:\n");
        ut_walk(ut_collection(dataReaders),
                (ut_actionFunc)in_readerReport, NULL);
    }
    return;
}

static os_int32 adminChecksum_participant (in_participant participant, md5_state_t *st)
{
  md5_append (st, (unsigned char *) participant, OS_SIZEOF (in_participant));
  return 1;
}

static os_int32 adminChecksum_reader (in_reader reader, md5_state_t *st)
{
  md5_append (st, (unsigned char *) reader, OS_SIZEOF (in_reader));
  return 1;
}

static os_int32 adminChecksum_writer (in_writer writer, md5_state_t *st)
{
  md5_append (st, (unsigned char *) writer, OS_SIZEOF (in_writer));
  return 1;
}

static char tohexdigit (unsigned x)
{
  assert (x < 16);
  return x < 10 ? '0' + x : 'a' + (x-10);
}

unsigned adminChecksum (int verify)
{
  md5_state_t st;
  unsigned char md5[16], md5txt[33];
  unsigned chksum = 0;
  int i;
  if (enable_adminChecksum)
  {
    md5_init (&st);
    ut_walk(ut_collection(participants), (ut_actionFunc)adminChecksum_participant, &st);
    ut_walk(ut_collection(dataWriters), (ut_actionFunc)adminChecksum_writer, &st);
    ut_walk(ut_collection(dataReaders), (ut_actionFunc)adminChecksum_reader, &st);
    md5_finish (&st, md5);
    memcpy (&chksum, md5, sizeof (chksum));
    for (i = 0; i < 16; i++)
    {
      md5txt[2*i+0] = tohexdigit (md5[i] >> 4);
      md5txt[2*i+1] = tohexdigit (md5[i] & 0xf);
    }
    md5txt[2*i] = 0;
    if (!verify)
      nn_log (LC_TRACE, "adminChecksum: %s\n", md5txt);
    if (verify && chksum != lastAdminChecksum)
      nn_log (LC_TRACE, "adminChecksum changed unexpectedly\n");
    lastAdminChecksum = chksum;
  }
  return chksum;
}

os_boolean
in_entityAdminInit(
    u_participant p)
{
    os_boolean success;

    u_entityAction(u_entity(p), resolveKernel, NULL);

    if(kernel){
        /* Allocate table for participants*/
        participants = ut_table(ut_tableNew(
                        (ut_compareElementsFunc)in_gidCompare,
                        NULL));

        /* Allocate table for readers*/
        dataReaders = ut_table(ut_tableNew(
                (ut_compareElementsFunc)in_gidCompare,
                NULL));

        dataReadersGuid = ut_table(ut_tableNew(
                (ut_compareElementsFunc)in_guidCompare,
                NULL));

        /* Allocate table for writers*/
        dataWriters = ut_table(ut_tableNew(
                (ut_compareElementsFunc)in_gidCompare,
                NULL));

        dataWritersGuid = ut_table(ut_tableNew(
            (ut_compareElementsFunc)in_guidCompare,
            NULL));

        adminChecksum (0);
        success = OS_TRUE;
    } else {
        nn_log (LC_ERROR, "Unable to resolve kernel.\n");
        success = OS_FALSE;
    }
    return success;
}

void
in_entityAdminDestroy()
{
  nn_log (LC_TRACE, "in_entityAdminDestroy\n");
  enable_adminChecksum = 0;

    ut_tableFree(dataWriters, NULL, NULL,
            (ut_freeElementFunc)in_writerFree, NULL);

    ut_tableFree(dataWritersGuid, NULL, NULL,
            (ut_freeElementFunc)NULL, NULL);

    ut_tableFree(dataReadersGuid, NULL, NULL,
            (ut_freeElementFunc)in_readerFree, NULL);

    ut_tableFree(dataReaders, NULL, NULL,
            (ut_freeElementFunc)NULL, NULL);

    ut_tableFree(participants, NULL, NULL,
            (ut_freeElementFunc)in_participantFree, NULL);

}

#ifndef NDEBUG
static os_boolean
objectIsParticipant(
    v_object object)
{
    os_boolean result;

    if(object){
        switch(object->kind){
        case K_PARTICIPANT:
        case K_SERVICE:
        case K_DURABILITY:
        case K_SPLICED:
        case K_NETWORKING:
        case K_CMSOAP:
            result = OS_TRUE;
            break;
        default:
            result = OS_FALSE;
            break;
        }
    } else {
        result = OS_FALSE;
    }
    return result;
}
#endif

in_participant
in_participantNew(
    struct v_participantInfo* ddsParticipant)
{
    in_participant p;
    os_int32 inserted;
    v_entity entity;
    unsigned flags = 0;

    adminChecksum (1);
    p = os_malloc(OS_SIZEOF(in_participant));

    if(p){
      entity = v_entity(v_gidClaim(ddsParticipant->key, kernel));
      if (entity == NULL)
      {
        os_free (p);
        return NULL;
      }

      assert(objectIsParticipant(v_object(entity)));
      assert (ddsParticipant->key.systemId == kernel->GID.systemId);

      if (config.minimal_sedp_endpoint_set)
      {
        if (! v_gidEqual (ddsParticipant->key, ddsi2_participant_gid))
          flags |= RTPS_PF_NO_BUILTIN_READERS;
      }

      in_entityInit(in_entity(p));
      p->ddsParticipantKey = ddsParticipant->key;
      p->prefix.u[0] = ddsParticipant->key.systemId;
      p->prefix.u[1] = ddsParticipant->key.localId;
      p->prefix.u[2] = ddsParticipant->key.serial;
      inserted = ut_tableInsert(participants, &p->ddsParticipantKey, p);

      if(!inserted){
        p->rtpsParticipant = NULL;
        in_entityDeinit(in_entity (p));
        os_free (p);
        p = NULL;
      } else {
        p->rtpsParticipant = new_participant(p->prefix, flags);
        adminChecksum (0);
        assert(p->rtpsParticipant);
        nn_log (LC_DISCOVERY, "DomainParticipant created.\n");
        in_participantReport(p, NULL);
      }
      v_gidRelease(ddsParticipant->key, kernel);
    }
    return p;
}

void
in_participantFree(
    in_participant participant,
    void* dummy)
{
    adminChecksum (1);
    if(dummy){}

    if(participant){
        nn_log (LC_DISCOVERY, "DomainParticipant %d, %d, %d deleted.\n",
                  participant->ddsParticipantKey.systemId,
                  participant->ddsParticipantKey.localId,
                  participant->ddsParticipantKey.serial);
        ut_remove(ut_collection(participants), &participant->ddsParticipantKey);
        adminChecksum (0);
        in_entityDeinit(in_entity(participant));

        if(participant->rtpsParticipant){
            delete_participant(participant->rtpsParticipant);
        }
        os_free(participant);
    }
    return;
}

os_int32
in_participantReport(
    in_participant participant,
    void* arg)
{
    os_int32 result = 1;

    assert(participant);

    if(arg){}

    if(participant && participant->rtpsParticipant){
        nn_log (LC_TRACE, "DomainParticipant\n");
        nn_log (LC_TRACE, "    - Global ID         : %d, %d, %d\n",
            participant->ddsParticipantKey.systemId,
            participant->ddsParticipantKey.localId,
            participant->ddsParticipantKey.serial);
    }
    return result;
}

in_participant
in_participantLookup(
    v_builtinTopicKey* key)
{
    adminChecksum (1);
    return in_participant(ut_get(ut_collection(participants), key));
}

void make_ddsi_id (nn_guid_t *id, ut_table tab, v_builtinTopicKey pkey, v_builtinTopicKey key, unsigned class)
{
  md5_state_t st0, st;
  unsigned char md5[16];
  unsigned seq = 0;

  id->prefix.u[0] = pkey.systemId;
  id->prefix.u[1] = pkey.localId;
  id->prefix.u[2] = pkey.serial;

  md5_init (&st0);
  md5_append (&st0, (unsigned char *) &key, sizeof (key));
  do {
    memcpy (&st, &st0, sizeof (st));
    md5_append (&st, (unsigned char *) &seq, sizeof (unsigned));
    md5_finish (&st, md5);
    id->entityid.u =
      ((unsigned) md5[0] << 24) |
      ((unsigned) md5[1] << 16) |
      ((unsigned) md5[2] << 8) |
      class;
  } while (ut_contains (ut_collection (tab), id) && seq++ < 1024);
  if (ut_contains (ut_collection (tab), id))
  {
    nn_log (LC_FATAL, "No available entityid found\n");
    abort ();
  }
}

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

void qcp_reliability (nn_xqos_t *xqos, const struct v_reliabilityPolicy *reliability)
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

void qcp_durability (nn_xqos_t *xqos, const struct v_durabilityPolicy *durability)
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

void qcp_ownership (nn_xqos_t *xqos, const struct v_ownershipPolicy *ownership)
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

void qcp_ownership_strength (nn_xqos_t *xqos, const struct v_strengthPolicy *strength)
{
  xqos->present |= QP_OWNERSHIP_STRENGTH;
  xqos->ownership_strength.value = strength->value;
}

void qcp_presentation (nn_xqos_t *xqos, const struct v_presentationPolicy *presentation)
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

void qcp_destination_order (nn_xqos_t *xqos, const struct v_orderbyPolicy *orderby)
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

void qcp_partition (nn_xqos_t *xqos, const struct v_builtinPartitionPolicy *ps)
{
  int i;
  xqos->present |= QP_PARTITION;
  xqos->partition.n = c_arraySize (ps->name);
  xqos->partition.strs = os_malloc (xqos->partition.n * sizeof (*xqos->partition.strs));
  for (i = 0; i < xqos->partition.n; i++)
    xqos->partition.strs[i] = os_strdup (ps->name[i]);
}

void qcp_deadline (nn_xqos_t *xqos, const struct v_deadlinePolicy *a)
{
  xqos->present |= QP_DEADLINE;
  xqos->deadline.deadline = c_time_to_ddsi_duration (a->period);
}

void qcp_latency (nn_xqos_t *xqos, const struct v_latencyPolicy *a)
{
  xqos->present |= QP_LATENCY_BUDGET;
  xqos->latency_budget.duration = c_time_to_ddsi_duration (a->duration);
}

void qcp_liveliness (nn_xqos_t *xqos, const struct v_livelinessPolicy *a)
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

void qcp_lifespan (nn_xqos_t *xqos, const struct v_lifespanPolicy *a)
{
  xqos->present |= QP_LIFESPAN;
  xqos->lifespan.duration = c_time_to_ddsi_duration (a->duration);
}

void qcp_time_based_filter (nn_xqos_t *xqos, const struct v_pacingPolicy *a)
{
  xqos->present |= QP_TIME_BASED_FILTER;
  xqos->time_based_filter.minimum_separation = c_time_to_ddsi_duration (a->minSeperation);
}

void qcp_rdlifespan (nn_xqos_t *xqos, const struct v_readerLifespanPolicy *a)
{
  if (a->used)
  {
    xqos->present |= QP_LIFESPAN;
    xqos->lifespan.duration = c_time_to_ddsi_duration (a->duration);
  }
}

void qcp_wrlifecycle (nn_xqos_t *xqos, const struct v_writerLifecyclePolicy *a)
{
  xqos->present |= QP_WRITER_DATA_LIFECYCLE;
  xqos->writer_data_lifecycle.autodispose_unregistered_instances =
    (a->autodispose_unregistered_instances != 0);
}

void qcp_rdlifecycle (nn_xqos_t *xqos, const struct v_readerLifecyclePolicy *a)
{
  xqos->present |= QP_READER_DATA_LIFECYCLE;
  xqos->reader_data_lifecycle.autopurge_nowriter_samples_delay =
    c_time_to_ddsi_duration (a->autopurge_nowriter_samples_delay);
  xqos->reader_data_lifecycle.autopurge_disposed_samples_delay =
    c_time_to_ddsi_duration (a->autopurge_disposed_samples_delay);
}

void qcp_user_data (nn_xqos_t *xqos, const struct v_builtinUserDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_USER_DATA;
    xqos->user_data.length = c_arraySize (a->value);
    xqos->user_data.value = os_malloc (xqos->user_data.length);
    memcpy (xqos->user_data.value, a->value, xqos->user_data.length);
  }
}

void qcp_topic_data (nn_xqos_t *xqos, const struct v_builtinTopicDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_TOPIC_DATA;
    xqos->topic_data.length = c_arraySize (a->value);
    xqos->topic_data.value = os_malloc (xqos->topic_data.length);
    memcpy (xqos->topic_data.value, a->value, xqos->topic_data.length);
  }
}

void qcp_topic_data_topic (nn_xqos_t *xqos, const struct v_topicDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_TOPIC_DATA;
    xqos->topic_data.length = c_arraySize (a->value);
    xqos->topic_data.value = os_malloc (xqos->topic_data.length);
    memcpy (xqos->topic_data.value, a->value, xqos->topic_data.length);
  }
}

void qcp_group_data (nn_xqos_t *xqos, const struct v_builtinGroupDataPolicy *a)
{
  if (c_arraySize (a->value) > 0)
  {
    xqos->present |= QP_GROUP_DATA;
    xqos->group_data.length = c_arraySize (a->value);
    xqos->group_data.value = os_malloc (xqos->group_data.length);
    memcpy (xqos->group_data.value, a->value, xqos->group_data.length);
  }
}

in_writer
in_writerNew(
    in_participant participant,
    struct v_publicationInfo* ddsWriter)
{
  in_writer w;
  v_entity entity;
  os_int32 inserted;
  c_bool hasKeys;
  v_topic topic;

  adminChecksum (1);
  w = os_malloc(OS_SIZEOF(in_writer));

  if(w){
    nn_xqos_t xqos;

    entity = v_entity(v_gidClaim(ddsWriter->participant_key, kernel));
    if (entity == NULL)
    {
      os_free (w);
      return NULL;
    }

    assert(objectIsParticipant(v_object(entity)));
    assert (ddsWriter->key.systemId == kernel->GID.systemId);

    topic = v_lookupTopic (kernel, ddsWriter->topic_name);
    assert (topic != NULL);

    hasKeys = c_arraySize (v_topicMessageKeyList (topic)) > 0;

    in_entityInit(in_entity(w));
    w->participant = participant;
    w->ddsWriterKey = ddsWriter->key;
    w->topic = topic;

    make_ddsi_id (&w->id, dataWritersGuid, w->participant->ddsParticipantKey, w->ddsWriterKey, hasKeys ? NN_ENTITYID_KIND_WRITER_WITH_KEY : NN_ENTITYID_KIND_WRITER_NO_KEY);

    nn_xqos_init_empty (&xqos);
    qcp_durability (&xqos, &ddsWriter->durability);
    qcp_deadline (&xqos, &ddsWriter->deadline);
    qcp_latency (&xqos, &ddsWriter->latency_budget);
    qcp_liveliness (&xqos, &ddsWriter->liveliness);
    qcp_reliability (&xqos, &ddsWriter->reliability);
    qcp_lifespan (&xqos, &ddsWriter->lifespan);
    qcp_destination_order (&xqos, &ddsWriter->destination_order);
    qcp_user_data (&xqos, &ddsWriter->user_data);
    qcp_ownership (&xqos, &ddsWriter->ownership);
    qcp_ownership_strength (&xqos, &ddsWriter->ownership_strength);
    qcp_presentation (&xqos, &ddsWriter->presentation);
    qcp_partition (&xqos, &ddsWriter->partition);
    qcp_topic_data (&xqos, &ddsWriter->topic_data);
    qcp_group_data (&xqos, &ddsWriter->group_data);
    qcp_wrlifecycle (&xqos, &ddsWriter->lifecycle);

    inserted = ut_tableInsert(dataWriters, &w->ddsWriterKey, w);

    if(!inserted){
      w->rtpsWriter = NULL;
      in_entityDeinit (in_entity (w));
      os_free (w);
      w = NULL;
    } else {
      ut_tableInsert(dataWritersGuid, &w->id, w);
      w->rtpsWriter = new_writer(participant->rtpsParticipant, w->id.entityid, w->topic, &xqos);
      adminChecksum (0);
      assert(w->rtpsWriter);
      nn_log (LC_DISCOVERY, "DataWriter created\n");
      in_writerReport(w, NULL);
    }

    nn_xqos_fini (&xqos);
    v_gidRelease(ddsWriter->participant_key, kernel);
 }
  return w;
}

void
in_writerFree(
    in_writer writer,
    void* dummy)
{
    adminChecksum (1);
    assert(writer);

    if(dummy){}

    if(writer){
      nn_log (LC_DISCOVERY, "DataWriter %d, %d, %d deleted.\n",
                  writer->ddsWriterKey.systemId,
                  writer->ddsWriterKey.localId,
                  writer->ddsWriterKey.serial);
        ut_remove(ut_collection(dataWriters), &writer->ddsWriterKey);
        ut_remove(ut_collection(dataWritersGuid), &writer->id);
        adminChecksum (0);
        in_entityDeinit(in_entity(writer));

        if(writer->rtpsWriter){
            delete_writer(writer->rtpsWriter);
        }
#if 0
        c_free(writer->topic);
#endif
        os_free(writer);
    }
}

os_boolean
in_writerWrite(
    struct nn_xpack *xp,
    in_writer writer,
    v_message message)
{
    adminChecksum (1);
    assert(writer);
    assert(message);
    rtps_write(xp, writer->rtpsWriter, message);
    return OS_TRUE;
}

in_writer
in_writerLookup(
    v_builtinTopicKey* key)
{
    adminChecksum (1);
    return in_writer(ut_get(ut_collection(dataWriters), key));
}

os_int32
in_writerReport(
    in_writer writer,
    void* arg)
{
    os_int32 result = 1;

    assert(writer);
    if(arg){}

    if(writer){
        nn_log (LC_TRACE, "DataWriter\n");
        nn_log (LC_TRACE, "    - Global ID         : %d, %d, %d\n",
                writer->ddsWriterKey.systemId,
                writer->ddsWriterKey.localId,
                writer->ddsWriterKey.serial);
        nn_log (LC_TRACE, "    - RtpsWriter        : %p\n", (void*)writer->rtpsWriter);
    }
    return result;
}

in_reader
in_fictitiousTransientReaderNew(in_participant participant, v_group group)
{
  in_reader r;
  v_builtinTopicKey fake_key;
  v_topicQos qos;
  c_bool hasKeys;
  nn_xqos_t xqos;

  adminChecksum (1);

  if ((r = os_malloc (OS_SIZEOF (in_reader))) == NULL)
    return NULL;

  in_entityInit (in_entity (r));
  r->participant = participant;
  v_gidSetNil (r->ddsReaderKey);
  r->rtpsReader = NULL;

  fake_key.systemId = participant->ddsParticipantKey.systemId;
  fake_key.localId = (c_ulong) ((c_address) group);
  fake_key.serial = 0;

  hasKeys = c_arraySize (v_topicMessageKeyList (group->topic)) > 0;
  make_ddsi_id (&r->id, dataReadersGuid, participant->ddsParticipantKey, fake_key, hasKeys ? NN_ENTITYID_KIND_READER_WITH_KEY : NN_ENTITYID_KIND_READER_NO_KEY);

  qos = group->topic->qos;

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

  nn_log (LC_DISCOVERY, "in_fictitiousTransientReaderNew: %s.%s guid %x:%x:%x:%x\n",
             v_entity (group->topic)->name, v_entity (group->partition)->name,
             r->id.prefix.u[0], r->id.prefix.u[1], r->id.prefix.u[2], r->id.entityid.u);
  if ((r->rtpsReader = new_reader(participant->rtpsParticipant,
                                  r->id.entityid, group->topic, &xqos)) == NULL)
  {
    nn_xqos_fini (&xqos);
    c_free (r->topic);
    in_entityDeinit (in_entity (r));
    os_free (r);
    return NULL;
  }

  /* insert in dataReadersGuid (for make_ddsi_id) but not in
     dataReaders (for want of a matching DDS reader) */
  nn_xqos_fini (&xqos);
  ut_tableInsert(dataReadersGuid, &r->id, r);
  adminChecksum (0);
  return r;
}

in_reader
in_readerNew(
    in_participant participant,
    struct v_subscriptionInfo* ddsReader)
{
  in_reader r;
  os_int32 inserted;
  v_entity entity;
  v_topic topic;
  c_bool hasKeys;

  adminChecksum (1);
  r = os_malloc(OS_SIZEOF(in_reader));

  if(r){
    nn_xqos_t xqos;

    entity = v_entity(v_gidClaim(ddsReader->participant_key, kernel));
    if (entity == NULL)
    {
      os_free (r);
      return NULL;
    }

    assert(objectIsParticipant(v_object(entity)));
    assert (ddsReader->key.systemId == kernel->GID.systemId);

    topic = v_lookupTopic (kernel, ddsReader->topic_name);
    assert (topic != NULL);

    hasKeys = c_arraySize (v_topicMessageKeyList (topic)) > 0;

    in_entityInit(in_entity(r));
    r->participant = participant;
    r->ddsReaderKey = ddsReader->key;
    r->rtpsReader = NULL;
    r->topic = topic;

    make_ddsi_id (&r->id, dataReadersGuid, r->participant->ddsParticipantKey, r->ddsReaderKey, hasKeys ? NN_ENTITYID_KIND_READER_WITH_KEY : NN_ENTITYID_KIND_READER_NO_KEY);

    nn_xqos_init_empty (&xqos);
    qcp_durability (&xqos, &ddsReader->durability);
    qcp_deadline (&xqos, &ddsReader->deadline);
    qcp_latency (&xqos, &ddsReader->latency_budget);
    qcp_liveliness (&xqos, &ddsReader->liveliness);
    qcp_reliability (&xqos, &ddsReader->reliability);
    qcp_ownership (&xqos, &ddsReader->ownership);
    qcp_destination_order (&xqos, &ddsReader->destination_order);
    qcp_user_data (&xqos, &ddsReader->user_data);
    qcp_time_based_filter (&xqos, &ddsReader->time_based_filter);
    qcp_presentation (&xqos, &ddsReader->presentation);
    qcp_partition (&xqos, &ddsReader->partition);
    qcp_topic_data (&xqos, &ddsReader->topic_data);
    qcp_group_data (&xqos, &ddsReader->group_data);
    qcp_rdlifespan (&xqos, &ddsReader->lifespan);

    inserted = ut_tableInsert(dataReaders, &r->ddsReaderKey, r);

    if(!inserted){
      r->rtpsReader = NULL;
      in_entityDeinit (in_entity (r));
      os_free (r);
      r = NULL;
    } else {
      ut_tableInsert(dataReadersGuid, &r->id, r);
      r->rtpsReader = new_reader(participant->rtpsParticipant,
                                 r->id.entityid, r->topic, &xqos);
      adminChecksum (0);
      assert(r->rtpsReader);
      nn_log (LC_DISCOVERY, "DataReader created\n");
      in_readerReport(r, NULL);
    }
    nn_xqos_fini (&xqos);
    v_gidRelease(ddsReader->participant_key, kernel);
  }
  return r;
}

void
in_readerFree(
    in_reader reader,
    void* dummy)
{
  adminChecksum (1);
  assert(reader);

  if(dummy){}

  if(reader){
    nn_log (LC_DISCOVERY, "%sDataReader %d, %d, %d deleted.\n",
              v_gidIsValid (reader->ddsReaderKey) ? "" : "Fictitious transient",
              reader->ddsReaderKey.systemId,
              reader->ddsReaderKey.localId,
              reader->ddsReaderKey.serial);
    if (v_gidIsValid (reader->ddsReaderKey))
      ut_remove(ut_collection(dataReaders), &reader->ddsReaderKey);
    ut_remove(ut_collection(dataReadersGuid), &reader->id);
    adminChecksum (0);
    in_entityDeinit(in_entity(reader));

    /* Must free reader before c_free'ing the group: only after
       delete_reader() has returned we are certain that the cb won't
       be called any more */
    if(reader->rtpsReader){
      delete_reader(reader->rtpsReader);
      reader->rtpsReader = NULL;
    }
#if 0
    c_free (reader->topic);
#endif
    os_free(reader);
  }
}

os_int32
in_readerReport(
    in_reader reader,
    void* arg)
{
    os_int32 result = 1;
    assert(reader);

    if(arg){}

    if(reader){
        nn_log (LC_TRACE, "DataReader\n");
        nn_log (LC_TRACE, "    - Global ID         : %d, %d, %d\n",
                reader->ddsReaderKey.systemId,
                reader->ddsReaderKey.localId,
                reader->ddsReaderKey.serial);
    }
    return result;
}

in_reader
in_readerLookup(
    v_builtinTopicKey* key)
{
    adminChecksum (1);
    return in_reader(ut_get(ut_collection(dataReaders), key));
}

static void
resolveKernel(
    v_entity entity,
    c_voidp args)
{
    if(args){}

    kernel = v_object(entity)->kernel;

    messageQosType = c_metaArrayTypeNew(c_metaObject(c_getBase(kernel)),
                        "C_ARRAY<c_octet>",
                        c_octet_t(c_getBase(kernel)),
                        0);
    return;
}

static os_equality
in_guidCompare(
    nn_guid_t* g1,
    nn_guid_t* g2,
    void* args)
{
    int result;
    os_equality eq;

    if(args) {}

    if(g1 && g2){
        result = memcmp(g1, g2, sizeof(nn_guid_t));
        if(result == 0){
            eq = OS_EQ;
        } else if(result > 0){
            eq = OS_GT;
        } else {
            eq = OS_LT;
        }
    } else if(g1){
        eq = OS_GT;
    } else if(g2){
        eq = OS_LT;
    } else {
        eq = OS_EQ;
    }
    return eq;
}

static void
in_entityInit(
    in_entity entity)
{
    assert(entity);
    entity->dummy = 0;
}

static void
in_entityDeinit(
        UNUSED_ARG_NDEBUG (in_entity entity))
{
    assert(entity);
}

static os_equality
in_gidCompare(
    v_gid* g1,
    v_gid* g2,
    void* args)
{
    os_equality eq;
    c_equality ceq;

    if(args) {}

    ceq = v_gidCompare(*g1, *g2);

    if(ceq == C_EQ){
        eq = OS_EQ;
    } else if(ceq == C_LT){
        eq = OS_LT;
    } else {
        eq = OS_GT;
    }
    return eq;
}
