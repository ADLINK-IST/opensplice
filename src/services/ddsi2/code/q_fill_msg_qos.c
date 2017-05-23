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
#include <stddef.h>
#include <assert.h>

/* FIXME: rename this file -- there is not even a fill_msg_qos anymore ... */

#include "v_messageQos.h"
#include "kernelModuleI.h"
#include "ut_collection.h"
#include "u_participant.h"
#include "os_abstract.h" /* big or little endianness */

#include "ut_avl.h"
#include "q_log.h"
#include "q_osplser.h"
#include "q_protocol.h"
#include "q_rtps.h"
#include "q_bswap.h"
#include "q_xqos.h"
#include "q_fill_msg_qos.h"
#include "q_entity.h" /* proxy_writer->v_message_qos */
#include "q_globals.h" /* gv.ospl_qostype */

/* To our great surprise and dismay, messageQos has all fields in
   big-edian format. */

static int ddsi_duration_to_BE_c_time (c_time *kd, nn_duration_t dd)
{
  const os_int64 tt = nn_from_ddsi_duration (dd);
  if (tt == T_NEVER)
  {
    const c_time kdinf = C_TIME_INFINITE;
    kd->seconds = toBE4 (kdinf.seconds);
    kd->nanoseconds = toBE4u (kdinf.nanoseconds);
    return 1;
  }
  else
  {
    os_int32 sec = (os_int32) (tt / T_SECOND);
    os_uint32 nsec = (os_uint32) (tt % T_SECOND);
    kd->seconds = toBE4 (sec);
    kd->nanoseconds = toBE4u (nsec);
    return 0;
  }
}

c_array new_v_message_qos (const nn_xqos_t *xqos)
{
  c_octet byte0 = 0, byte1 = 0;
  c_octet qosbase[2 /* the 2 bitmask bytes */
                  + 2 * 4 /* transport priority, ownership strength */
                  + 4 * 8]; /* latency budget, deadline, lease duration, lifespan */
  c_octet *dst = qosbase + 2;
  c_array msgqos;

  /* See v_messageQos.c::v_messageQos__new(). But do note we do it
     ever so slightly differently :)

     (This way, treatment of dst is simpler) */
  assert (xqos->present & QP_TRANSPORT_PRIORITY);
  {
    int value = toBE4 (xqos->transport_priority.value);
    memcpy (dst, &value, sizeof (value));
    dst += sizeof (value);
  }

  assert (xqos->present & QP_RELIABILITY);
  switch (xqos->reliability.kind)
  {
    case NN_BEST_EFFORT_RELIABILITY_QOS:
      byte0 |= _LSHIFT_(V_RELIABILITY_BESTEFFORT, MQ_BYTE0_RELIABILITY_OFFSET);
      break;
    case NN_RELIABLE_RELIABILITY_QOS:
      byte0 |= _LSHIFT_(V_RELIABILITY_RELIABLE, MQ_BYTE0_RELIABILITY_OFFSET);
      break;
  }

  assert (xqos->present & QP_OWNERSHIP);
  switch (xqos->ownership.kind)
  {
    case NN_SHARED_OWNERSHIP_QOS:
      byte0 |= _LSHIFT_(V_OWNERSHIP_SHARED, MQ_BYTE0_OWNERSHIP_OFFSET);
      break;
    case NN_EXCLUSIVE_OWNERSHIP_QOS:
      {
        int strength;
        byte0 |= _LSHIFT_(V_OWNERSHIP_EXCLUSIVE, MQ_BYTE0_OWNERSHIP_OFFSET);
        if (xqos->present & QP_OWNERSHIP_STRENGTH)
          strength = toBE4 (xqos->ownership_strength.value);
        else
          strength = 0;
        memcpy (dst, &strength, sizeof (strength));
        dst += sizeof (strength);
        break;
      }
  }

  assert (xqos->present & QP_DESTINATION_ORDER);
  switch (xqos->destination_order.kind)
  {
    case NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
      byte0 |= _LSHIFT_(V_ORDERBY_RECEPTIONTIME, MQ_BYTE0_ORDERBY_OFFSET);
      break;
    case NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
      byte0 |= _LSHIFT_(V_ORDERBY_SOURCETIME, MQ_BYTE0_ORDERBY_OFFSET);
      break;
  }

  assert (xqos->present & QP_PRISMTECH_WRITER_DATA_LIFECYCLE);
  {
    int autodispose = xqos->writer_data_lifecycle.autodispose_unregistered_instances;
    byte0 |= _LSHIFT_(autodispose, MQ_BYTE0_AUTODISPOSE_OFFSET);
  }

  assert (xqos->present & QP_LATENCY_BUDGET);
  {
    c_time durBE;
    (void) ddsi_duration_to_BE_c_time (&durBE, xqos->latency_budget.duration);
    if (durBE.seconds == 0 && durBE.nanoseconds == 0)
      byte0 |= _LSHIFT_(1,MQ_BYTE0_LATENCY_OFFSET);
    else
    {
      memcpy (dst, &durBE, sizeof (durBE));
      dst += sizeof (durBE);
    }
  }

  assert (xqos->present & QP_DEADLINE);
  {
    c_time durBE;
    if (ddsi_duration_to_BE_c_time (&durBE, xqos->deadline.deadline))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_DEADLINE_OFFSET);
    else
    {
      memcpy (dst, &durBE, sizeof (durBE));
      dst += sizeof (durBE);
    }
  }

  assert (xqos->present & QP_LIVELINESS);
  {
    c_time durBE;
    if (ddsi_duration_to_BE_c_time (&durBE, xqos->liveliness.lease_duration))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_LIVELINESS_OFFSET);
    else
    {
      memcpy (dst, &durBE, sizeof (durBE));
      dst += sizeof (durBE);
    }
  }

  assert (xqos->present & QP_LIFESPAN);
  {
    c_time durBE;
    if (ddsi_duration_to_BE_c_time (&durBE, xqos->lifespan.duration))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_LIFESPAN_OFFSET);
    else
    {
      memcpy (dst, &durBE, sizeof (durBE));
      dst += sizeof (durBE);
    }
  }

  assert (xqos->present & QP_DURABILITY);
  switch (xqos->durability.kind)
  {
    case NN_VOLATILE_DURABILITY_QOS:
      byte1 |= _LSHIFT_(V_DURABILITY_VOLATILE, MQ_BYTE1_DURABILITY_OFFSET);
      break;
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS:
      byte1 |= _LSHIFT_(V_DURABILITY_TRANSIENT_LOCAL, MQ_BYTE1_DURABILITY_OFFSET);
      break;
    case NN_TRANSIENT_DURABILITY_QOS:
      byte1 |= _LSHIFT_(V_DURABILITY_TRANSIENT, MQ_BYTE1_DURABILITY_OFFSET);
      break;
    case NN_PERSISTENT_DURABILITY_QOS:
      byte1 |= _LSHIFT_(V_DURABILITY_PERSISTENT, MQ_BYTE1_DURABILITY_OFFSET);
      break;
  }

  assert (xqos->present & QP_LIVELINESS);
  switch (xqos->liveliness.kind)
  {
    case NN_AUTOMATIC_LIVELINESS_QOS:
      byte1 |= _LSHIFT_(V_LIVELINESS_AUTOMATIC, MQ_BYTE1_LIVELINESS_OFFSET);
      break;
    case NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
      byte1 |= _LSHIFT_(V_LIVELINESS_PARTICIPANT, MQ_BYTE1_LIVELINESS_OFFSET);
      break;
    case NN_MANUAL_BY_TOPIC_LIVELINESS_QOS:
      byte1 |= _LSHIFT_(V_LIVELINESS_TOPIC, MQ_BYTE1_LIVELINESS_OFFSET);
      break;
  }

  assert (xqos->present & QP_PRESENTATION);
  switch (xqos->presentation.access_scope)
  {
    case NN_INSTANCE_PRESENTATION_QOS:
      byte1 |= _LSHIFT_(V_PRESENTATION_INSTANCE, MQ_BYTE1_PRESENTATION_OFFSET);
      break;
    case NN_TOPIC_PRESENTATION_QOS:
      byte1 |= _LSHIFT_(V_PRESENTATION_TOPIC, MQ_BYTE1_PRESENTATION_OFFSET);
      break;
    case NN_GROUP_PRESENTATION_QOS:
      byte1 |= _LSHIFT_(V_PRESENTATION_GROUP, MQ_BYTE1_PRESENTATION_OFFSET);
      break;
  }
  {
    int oaccess = (xqos->presentation.ordered_access != 0);
    int caccess = (xqos->presentation.coherent_access != 0);
    byte1 |= _LSHIFT_(oaccess, MQ_BYTE1_ORDERED_ACCESS_OFFSET);
    byte1 |= _LSHIFT_(caccess, MQ_BYTE1_COHERENT_ACCESS_OFFSET);
  }

  assert ((size_t) (dst - qosbase) <= sizeof (qosbase));
  qosbase[0] = byte0;
  qosbase[1] = byte1;

  msgqos = c_newArray (gv.ospl_qostype, (c_ulong) (dst - qosbase));
  if (msgqos != NULL)
    memcpy (msgqos, qosbase, (size_t) (dst - qosbase));
  return msgqos;
}

/* SHA1 not available (unoffical build.) */
