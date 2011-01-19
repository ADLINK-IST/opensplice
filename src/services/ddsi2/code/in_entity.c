/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "in_entity.h"
#include "in_config.h"
#include "rtps.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_groupSet.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_state.h"
#include "v__messageQos.h"
#include "kernelModule.h"
#include "ut_collection.h"
#include "u_participant.h"
#include "os_abstract.h" /* big or little endianness */

#include "ddsi2.h"
#include "md5.h"

static ut_table participants = NULL;
static ut_table dataReaders = NULL;
static ut_table dataReadersGuid = NULL;
static ut_table dataWriters = NULL;
static ut_table dataWritersGuid = NULL;
static ut_table usubscribers = NULL;
static ut_table udataReaders = NULL;

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
    guid_prefix_t prefix;
};

OS_STRUCT(in_reader){
    OS_EXTENDS(in_entity);
    in_participant participant;
    v_builtinTopicKey ddsReaderKey;
    struct reader* rtpsReader;
    guid_t id;
    v_group group;
};

OS_STRUCT(in_writer){
    OS_EXTENDS(in_entity);
    in_participant participant;
    v_builtinTopicKey ddsWriterKey;
    struct writer* rtpsWriter;
    guid_t id;
    v_group group;
};

OS_STRUCT(in_group)
{
    v_group vgroup;
};

static void
in_readerInsert(
    struct datainfo *info,
    v_message message,
    void *arg);

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
in_groupCompare(
    v_group g1,
    v_group g2,
    void* args);

static os_equality
in_guidCompare(
    guid_t* g1,
    guid_t* g2,
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
        in_printf(IN_LEVEL_FINEST, "DomainParticipants:\n");
        ut_walk(ut_collection(participants),
                (ut_actionFunc)in_participantReport, NULL);
    }
    if(ut_count(ut_collection(dataWriters))>0){
        in_printf(IN_LEVEL_FINEST, "DataWriters:\n");
        ut_walk(ut_collection(dataWriters),
                (ut_actionFunc)in_writerReport, NULL);
    }
    if(ut_count(ut_collection(dataReaders))>0){
        in_printf(IN_LEVEL_FINEST, "DataReaders:\n");
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
      in_printf (IN_LEVEL_INFO, "adminChecksum: %s\n", md5txt);
    if (verify && chksum != lastAdminChecksum)
      in_printf (IN_LEVEL_INFO, "adminChecksum changed unexpectedly\n");
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
        in_printf(IN_LEVEL_SEVERE, "Unable to resolve kernel.\n");
        success = OS_FALSE;
    }
    return success;
}

void
in_entityAdminDestroy()
{
  in_printf (IN_LEVEL_INFO, "in_entityAdminDestroy\n");
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

      assert(objectIsParticipant(v_object(entity)));
      assert (ddsParticipant->key.systemId == kernel->GID.systemId);

      if (minimal_sedp_endpoint_set_flag)
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
	in_printf(IN_LEVEL_FINE, "DomainParticipant created.\n");
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
	in_printf(IN_LEVEL_FINE, "DomainParticipant %d, %d, %d deleted.\n",
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
        in_printf(IN_LEVEL_FINEST, "DomainParticipant\n");
        in_printf(IN_LEVEL_FINEST, "    - Global ID         : %d, %d, %d\n",
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

void make_ddsi_id (guid_t *id, ut_table tab, v_builtinTopicKey pkey, v_builtinTopicKey key, unsigned class)
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
    in_printf (IN_LEVEL_SEVERE, "No available entityid found\n");
    abort ();
  }
}

unsigned make_ddsi_mode (
	const struct v_reliabilityPolicy *reliability,
	const struct v_durabilityPolicy *durability,
	const struct v_ownershipPolicy *ownership,
	const struct v_presentationPolicy *presentation,
	const struct v_orderbyPolicy *orderby)
{
  unsigned mode = 0;
  if(reliability->kind == V_RELIABILITY_RELIABLE){
    mode |= MF_RELIABLE;
  }
  switch (durability->kind)
  {
    case V_DURABILITY_VOLATILE:
      mode |= MF_DURABILITY_VOLATILE;
      break;
    case V_DURABILITY_TRANSIENT_LOCAL:
      mode |= MF_DURABILITY_TRANSIENT_LOCAL
	| MF_HANDLE_AS_TRANSIENT_LOCAL;
      break;
    case V_DURABILITY_TRANSIENT:
      mode |= MF_DURABILITY_TRANSIENT;
      break;
    case V_DURABILITY_PERSISTENT:
      mode |= MF_DURABILITY_PERSISTENT;
      break;
  }
  if (ownership->kind == V_OWNERSHIP_EXCLUSIVE)
    mode |= MF_EXCLUSIVE_OWNERSHIP;
  switch (presentation->access_scope)
  {
    case V_PRESENTATION_INSTANCE:
      mode |= MF_ACCESS_SCOPE_INSTANCE;
      break;
    case V_PRESENTATION_TOPIC:
      mode |= MF_ACCESS_SCOPE_TOPIC;
      break;
    case V_PRESENTATION_GROUP:
      mode |= MF_ACCESS_SCOPE_GROUP;
      break;
  }
  switch (orderby->kind)
  {
    case V_ORDERBY_RECEPTIONTIME:
      mode |= MF_DESTINATION_ORDER_RECEPTION;
      break;
    case V_ORDERBY_SOURCETIME:
      mode |= MF_DESTINATION_ORDER_SOURCE;
      break;
  }
  return mode;
}

in_writer
in_writerNew(
    in_participant participant,
    struct v_publicationInfo* ddsWriter)
{
  in_writer w;
  v_entity entity;
  os_int32 inserted;
  unsigned mode = 0;
  v_group group;
  c_bool hasKeys;

  adminChecksum (1);
  w = os_malloc(OS_SIZEOF(in_writer));

  if(w){
    entity = v_entity(v_gidClaim(ddsWriter->participant_key, kernel));

    assert(objectIsParticipant(v_object(entity)));
    assert (ddsWriter->key.systemId == kernel->GID.systemId);

    group = v_groupSetGet(kernel->groupSet,
			  ddsWriter->partition.name[0], ddsWriter->topic_name);
    assert(group);

    hasKeys = c_arraySize (v_topicMessageKeyList (group->topic)) > 0;

    in_entityInit(in_entity(w));
    w->participant = participant;
    w->ddsWriterKey = ddsWriter->key;

    make_ddsi_id (&w->id, dataWritersGuid, w->participant->ddsParticipantKey, w->ddsWriterKey, hasKeys ? ENTITYID_KIND_WRITER_WITH_KEY : ENTITYID_KIND_WRITER_NO_KEY);
    mode = make_ddsi_mode (&ddsWriter->reliability, &ddsWriter->durability, &ddsWriter->ownership, &ddsWriter->presentation, &ddsWriter->destination_order);
    w->group = v_groupSetGet(kernel->groupSet,
			     ddsWriter->partition.name[0], ddsWriter->topic_name);
    assert(w->group);

    inserted = ut_tableInsert(dataWriters, &w->ddsWriterKey, w);

    if(!inserted){
      w->rtpsWriter = NULL;
      in_entityDeinit (in_entity (w));
      os_free (w);
      w = NULL;
    } else {
      ut_tableInsert(dataWritersGuid, &w->id, w);
      w->rtpsWriter = new_writer(participant->rtpsParticipant,
				 w->id.entityid, mode, w->group->topic,
				 ddsWriter->partition.name[0]);
      adminChecksum (0);
      assert(w->rtpsWriter);
      in_printf(IN_LEVEL_FINE, "DataWriter created\n");
      in_writerReport(w, NULL);
    }
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
      in_printf(IN_LEVEL_FINE, "DataWriter %d, %d, %d deleted.\n",
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
        if(writer->group){
            c_free(writer->group);
        }
        os_free(writer);
    }
}

os_boolean
in_writerWrite(
    in_writer writer,
    v_message message)
{
    adminChecksum (1);
    assert(writer);
    assert(message);
#if 0
    in_writerReport(writer, NULL);
    in_printf(IN_LEVEL_FINEST, "Writing message...\n");
#endif
    rtps_write(writer->rtpsWriter, message);
    c_free (message);
    return OS_TRUE;
}

os_boolean
in_writerDispose(
    in_writer writer,
    v_message message)
{
    adminChecksum (1);
    assert(writer);
    assert(message);

#if 0
    in_writerReport(writer, NULL);
    in_printf(IN_LEVEL_FINEST, "Disposing instance...\n");
#endif
    rtps_dispose(writer->rtpsWriter, message);
    c_free (message);
    return OS_TRUE;
}

os_boolean
in_writerUnregister(
    in_writer writer,
    v_message message)
{
    adminChecksum (1);
    assert(writer);
    assert(message);
#if 0
    in_writerReport(writer, NULL);
    in_printf(IN_LEVEL_FINEST, "Unregistering instance...\n");
#endif
    rtps_unregister(writer->rtpsWriter, message);
    c_free (message);
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
        in_printf(IN_LEVEL_FINEST, "DataWriter\n");
        in_printf(IN_LEVEL_FINEST, "    - Global ID         : %d, %d, %d\n",
                writer->ddsWriterKey.systemId,
                writer->ddsWriterKey.localId,
                writer->ddsWriterKey.serial);
        in_printf(IN_LEVEL_FINEST, "    - RtpsWriter        : %p\n", (void*)writer->rtpsWriter);
    }
    return result;
}

in_reader
in_fictitiousTransientReaderNew(in_participant participant, v_group group)
{
  in_reader r;
  v_builtinTopicKey fake_key;
  v_topicQos qos;
  unsigned mode;
  c_bool hasKeys;
  struct v_presentationPolicy presentation;

  adminChecksum (1);

  if ((r = os_malloc (OS_SIZEOF (in_reader))) == NULL)
    return NULL;

  in_entityInit (in_entity (r));
  r->participant = participant;
  v_gidSetNil (r->ddsReaderKey);
  r->rtpsReader = NULL;
  r->group = c_keep (group);
    
  fake_key.systemId = participant->ddsParticipantKey.systemId;
  fake_key.localId = (c_ulong) ((c_address) group);
  fake_key.serial = 0;
  
  hasKeys = c_arraySize (v_topicMessageKeyList (group->topic)) > 0;
  make_ddsi_id (&r->id, dataReadersGuid, participant->ddsParticipantKey, fake_key, hasKeys ? ENTITYID_KIND_READER_WITH_KEY : ENTITYID_KIND_READER_NO_KEY);
  qos = group->topic->qos;
  presentation.access_scope = V_PRESENTATION_INSTANCE;
  mode = make_ddsi_mode (&qos->reliability, &qos->durability, &qos->ownership, &presentation, &qos->orderby);

  in_printf (IN_LEVEL_INFO, "in_fictitiousTransientReaderNew: %s.%s guid %x:%x:%x:%x\n",
	     v_entity (group->topic)->name, v_entity(group->partition)->name,
	     r->id.prefix.u[0], r->id.prefix.u[1], r->id.prefix.u[2], r->id.entityid.u); 
  if ((r->rtpsReader = new_reader(participant->rtpsParticipant,
		 r->id.entityid, mode, group->topic, v_entity(group->partition)->name,
		 in_readerInsert, group)) == NULL)
  {
    c_free (r->group);
    in_entityDeinit (in_entity (r));
    os_free (r);
    return NULL;
  }

  /* insert in dataReadersGuid (for make_ddsi_id) but not in
     dataReaders (for want of a matching DDS reader) */
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
  v_group group;
  unsigned mode = 0;
  c_bool hasKeys;

  adminChecksum (1);
  r = os_malloc(OS_SIZEOF(in_reader));

  if(r){
    entity = v_entity(v_gidClaim(ddsReader->participant_key, kernel));
    assert(objectIsParticipant(v_object(entity)));
    assert (ddsReader->key.systemId == kernel->GID.systemId);

    group = v_groupSetGet(kernel->groupSet,
			  ddsReader->partition.name[0], ddsReader->topic_name);
    assert(group);

    hasKeys = c_arraySize (v_topicMessageKeyList (group->topic)) > 0;

    in_entityInit(in_entity(r));
    r->participant = participant;
    r->ddsReaderKey = ddsReader->key;
    r->rtpsReader = NULL;

    make_ddsi_id (&r->id, dataReadersGuid, r->participant->ddsParticipantKey, r->ddsReaderKey, hasKeys ? ENTITYID_KIND_READER_WITH_KEY : ENTITYID_KIND_READER_NO_KEY);
    mode = make_ddsi_mode (&ddsReader->reliability, &ddsReader->durability, &ddsReader->ownership, &ddsReader->presentation, &ddsReader->destination_order);
    r->group = group;
    inserted = ut_tableInsert(dataReaders, &r->ddsReaderKey, r);

    if(!inserted){
      r->rtpsReader = NULL;
      in_entityDeinit (in_entity (r));
      os_free (r);
      r = NULL;
    } else {
      ut_tableInsert(dataReadersGuid, &r->id, r);
      r->rtpsReader = new_reader(participant->rtpsParticipant,
				 r->id.entityid, mode, r->group->topic,
				 ddsReader->partition.name[0],
				 in_readerInsert, r->group);
      adminChecksum (0);
      assert(r->rtpsReader);
      in_printf(IN_LEVEL_FINE, "DataReader created\n");
      in_readerReport(r, NULL);
    }
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
    in_printf(IN_LEVEL_FINE, "%sDataReader %d, %d, %d deleted.\n",
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
    if(reader->group){
      c_free(reader->group);
    }
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
        in_printf(IN_LEVEL_FINEST, "DataReader\n");
        in_printf(IN_LEVEL_FINEST, "    - Global ID         : %d, %d, %d\n",
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
in_readerInsert (struct datainfo *di, v_message message, void *arg)
{
  v_group group = arg;
  if (message == NULL)
  {
    /* last gasp - need not do anything: callback is never called for
       a reader once that delete_reader() has returned */
    in_printf (IN_LEVEL_FINEST, "in_readerInsert: last gasp: group %p\n", group);
  }
  else if (message->writeTime.nanoseconds & 0x1u)
  {
    /* do nothing: part of nasty hack to call groupWrite only once per
       message */
  }
  else
  {
    message->writeTime.nanoseconds |= 0x1u; /* also part of said hack */
    in_printf (IN_LEVEL_FINEST, "{GW %p}", group);
    v_groupWrite (group, message, NULL, myNetworkId);
  }
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
in_groupCompare(
    v_group g1,
    v_group g2,
    void* args)
{
    os_equality eq;

    if(args) {}

    if(g1 && g2){
        if(g1 == g2){
            eq = OS_EQ;
        } else if(g1 > g2){
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

static os_equality
in_guidCompare(
    guid_t* g1,
    guid_t* g2,
    void* args)
{
    int result;
    os_equality eq;

    if(args) {}

    if(g1 && g2){
        result = memcmp(g1, g2, sizeof(guid_t));
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
    in_entity entity)
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
