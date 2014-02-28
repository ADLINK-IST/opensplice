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
#include <stddef.h>
#include <assert.h>

/* FIXME: rename this file -- there is not even a fill_msg_qos anymore ... */

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

static v_duration ddsi_duration_to_BE_c_time (nn_duration_t dd)
{
  const v_duration kdinf = C_TIME_INFINITE;
  v_duration kd;
#if DDSI_DURATION_ACCORDING_TO_SPEC
  if (dd.sec == 0x7fffffff && dd.nanosec == 0x7fffffff)
  {
    kd.seconds = toBE4 (kdinf.seconds);
    kd.nanoseconds = toBE4 (kdinf.nanoseconds);
  }
  else
  {
    kd.seconds = toBE4 (dd.sec);
    kd.nanoseconds = toBE4 (dd.nanosec);
  }
#else
  if (dd.seconds == 0x7fffffff && dd.fraction == 0xffffffff)
  {
    kd.seconds = toBE4 (kdinf.seconds);
    kd.nanoseconds = toBE4 (kdinf.nanoseconds);
  }
  else
  {
    /* result of division will be truncated (fraction is known to be
       positive, so that means rounded down), and therefore 0 <=
       nanoseconds < 10^9 */
    kd.seconds = toBE4 (dd.seconds);
    kd.nanoseconds = toBE4 ((int) (dd.fraction / 4.294967296));
  }
#endif
  return kd;
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
    v_duration dur = ddsi_duration_to_BE_c_time (xqos->latency_budget.duration);
    if (c_timeIsZero (dur))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_LATENCY_OFFSET);
    else
    {
      memcpy (dst, &dur, sizeof (dur));
      dst += sizeof (dur);
    }
  }

  assert (xqos->present & QP_DEADLINE);
  {
    v_duration dur = ddsi_duration_to_BE_c_time (xqos->deadline.deadline);
    if (c_timeIsInfinite (dur))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_DEADLINE_OFFSET);
    else
    {
      memcpy (dst, &dur, sizeof (dur));
      dst += sizeof (dur);
    }
  }

  assert (xqos->present & QP_LIVELINESS);
  {
    v_duration dur = ddsi_duration_to_BE_c_time (xqos->liveliness.lease_duration);
    if (c_timeIsInfinite (dur))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_LIVELINESS_OFFSET);
    else
    {
      memcpy (dst, &dur, sizeof (dur));
      dst += sizeof (dur);
    }
  }

  assert (xqos->present & QP_LIFESPAN);
  {
    v_duration dur = ddsi_duration_to_BE_c_time (xqos->lifespan.duration);
    if (c_timeIsInfinite (dur))
      byte0 |= _LSHIFT_(1,MQ_BYTE0_LIFESPAN_OFFSET);
    else
    {
      memcpy (dst, &dur, sizeof (dur));
      dst += sizeof (dur);
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

  assert (dst - qosbase <= (ptrdiff_t) sizeof (qosbase));
  qosbase[0] = byte0;
  qosbase[1] = byte1;

  msgqos = c_newArray (gv.ospl_qostype, (int) (dst - qosbase));
  if (msgqos != NULL)
    memcpy (msgqos, qosbase, dst - qosbase);
  return msgqos;
}

/* SHA1 not available (unoffical build.) */
