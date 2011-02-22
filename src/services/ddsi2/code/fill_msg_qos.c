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

#include "avl.h"
#include "osplser.h"
#include "protocol.h"
#include "rtps.h"
#include "rtps_private.h"

#ifdef PA_BIG_ENDIAN
#define _COPY4_(d,s) d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3]
#endif
#ifdef PA_LITTLE_ENDIAN
#define _COPY4_(d,s) d[0]=s[3];d[1]=s[2];d[2]=s[1];d[3]=s[0]
#endif

static c_collectionType osplQosType;

int fill_v_message_qos (const struct proxy_endpoint *pwr, struct handle_regular_helper_arg *arg, unsigned statusinfo, c_base base)
{
  v_message msg = arg->payload.v_message;
  c_octet byte0 = 0;
  c_octet byte1 = 0;
  c_octet *dst, *src;
  c_long offset     = 6; /* byte0 + byte1 + transport_priority */
  c_long strength_offset = 0;
  c_long strength = 0;
  c_long transportPriority = 0;
  v_reliabilityKind rkind;
  v_durabilityKind dkind;
  v_livelinessKind lkind;
  v_presentationKind pkind;
  c_bool oaccess;
  c_bool caccess;

  msg->writeTime.seconds = (c_long)(arg->tstamp / 1000000000);
  msg->writeTime.nanoseconds = (c_ulong)((arg->tstamp % 1000000000) & ~0x1u); /* in_readerInsert gets to abuse the lsb */
  msg->writerGID = arg->wri.writerGID;
  msg->writerInstanceGID = arg->wri.writerInstanceGID;
  msg->transactionId = arg->wri.transactionId;
  msg->sequenceNumber = (c_ulong) arg->seq;

  rkind = pwr->reliable ? V_RELIABILITY_RELIABLE : V_RELIABILITY_BESTEFFORT;
  switch (pwr->durability)
  {
    case VOLATILE_DURABILITY_QOS:
      dkind = V_DURABILITY_VOLATILE;
      break;
    case TRANSIENT_LOCAL_DURABILITY_QOS:
      dkind = V_DURABILITY_TRANSIENT_LOCAL;
      break;
    case TRANSIENT_DURABILITY_QOS:
      dkind = V_DURABILITY_TRANSIENT;
      break;
    case PERSISTENT_DURABILITY_QOS:
      dkind = V_DURABILITY_PERSISTENT;
      break;
    default: abort (); dkind = (v_durabilityKind) 0;
  }
  switch(pwr->liveliness_kind)
  {
    case AUTOMATIC_LIVELINESS_QOS:
      lkind = V_LIVELINESS_AUTOMATIC;
      break;
    case MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      lkind = V_LIVELINESS_PARTICIPANT;
      break;
    case MANUAL_BY_TOPIC_LIVELINESS_QOS:
      lkind = V_LIVELINESS_TOPIC;
      break;
    default: abort(); lkind = V_LIVELINESS_AUTOMATIC;
  }
  switch(pwr->presentation_qospolicy.access_scope)
  {
    case INSTANCE_PRESENTATION_QOS:
      pkind = V_PRESENTATION_INSTANCE;
      break;
    case TOPIC_PRESENTATION_QOS:
      pkind = V_PRESENTATION_TOPIC;
      break;
    case GROUP_PRESENTATION_QOS:
      pkind = V_PRESENTATION_GROUP;
      break;
    default: abort(); pkind = V_PRESENTATION_INSTANCE;
  }
  oaccess = (pwr->presentation_qospolicy.ordered_access != 0);
  caccess = (pwr->presentation_qospolicy.coherent_access != 0);

  byte0 |= _LSHIFT_(rkind, MQ_BYTE0_RELIABILITY_OFFSET);
  byte0 |= _LSHIFT_(pwr->ownership_kind, MQ_BYTE0_OWNERSHIP_OFFSET);
  byte0 |= _LSHIFT_(pwr->destination_order_kind, MQ_BYTE0_ORDERBY_OFFSET);
  byte0 |= _LSHIFT_(FALSE, MQ_BYTE0_AUTODISPOSE_OFFSET);
  byte0 |= _LSHIFT_(1,MQ_BYTE0_LATENCY_OFFSET);
  byte0 |= _LSHIFT_(1,MQ_BYTE0_DEADLINE_OFFSET);
  byte0 |= _LSHIFT_(1,MQ_BYTE0_LIVELINESS_OFFSET);
  byte0 |= _LSHIFT_(1,MQ_BYTE0_LIFESPAN_OFFSET);


  byte1 |= _LSHIFT_(dkind, MQ_BYTE1_DURABILITY_OFFSET);
  byte1 |= _LSHIFT_(lkind, MQ_BYTE1_LIVELINESS_OFFSET);
  byte1 |= _LSHIFT_(pkind, MQ_BYTE1_PRESENTATION_OFFSET);
  byte1 |= _LSHIFT_(oaccess, MQ_BYTE1_ORDERED_ACCESS_OFFSET);
  byte1 |= _LSHIFT_(caccess, MQ_BYTE1_COHERENT_ACCESS_OFFSET);


  if (pwr->ownership_kind == EXCLUSIVE_OWNERSHIP_QOS) {
      strength_offset = offset;
      offset += sizeof(offset);
  }

  if(!osplQosType){
      osplQosType = (c_collectionType)c_metaArrayTypeNew(c_metaObject(base),
                        "C_ARRAY<c_octet>",
                        c_octet_t(base),
                        0);
  }
  msg->qos = c_newArray(osplQosType, offset);
  if (msg->qos == NULL)
    return 0;

  ((c_octet *) msg->qos)[0] = byte0;
  ((c_octet *) msg->qos)[1] = byte1;
  src = (c_octet *) &transportPriority;
  dst = (c_octet *) &((c_octet *) msg->qos)[2];
  _COPY4_ (dst,src);


  if (strength_offset) {
      src = (c_octet *)&strength;
      dst = (c_octet *)&((c_octet *)msg->qos)[strength_offset];
      _COPY4_(dst,src);
  }

  if (statusinfo == 0)
    v_stateSet (v_nodeState (msg), L_WRITE);
  else if (statusinfo == STATUSINFO_UNREGISTER)
    v_stateSet (v_nodeState (msg), L_UNREGISTER);
  else if (statusinfo == STATUSINFO_DISPOSE)
    v_stateSet (v_nodeState (msg), L_DISPOSED);
  else
  {
    rtps_trace (DBGFLAG_TRACING, "received unknown statusinfo: %d\n", statusinfo);
    abort ();
  }
  return 1;
}
