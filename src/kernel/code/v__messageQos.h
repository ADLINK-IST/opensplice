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
#ifndef V_MESSAGEQOS_H
#define V_MESSAGEQOS_H

#include "v_kernel.h"

/* The macro _FAST_ACCESS_ will change the messageQoS format.
 * if not defined the four most significant bits of the second byte are used
 * to specify default values of the latency-, deadline-, liveliness-, and
 * lifespan durations and not adding the actual durations values to the
 * messsageQos.
 * If set then these bits are unused and instead the bytes 2..5 of the
 * messageQos are used to specify the offset of the durations in the
 * messageQos where the value zero indicates the default value.
 * So if _FAST_ACCESS_ is set it will add an additional 4 bytes to the
 * messageQos but does not require offset calculations for duration values.
 */
/*#define _FAST_ACCESS_*/

/**
 * The following macro's specify the offset of the bits within the specified
 * byte that contain the value of the QoS Policy.
 * QoS Policies that are represented by a boolean or enumeration are mapped
 * onto a minimum number of bits to reduce the amount of message overhead.
 * The first two bytes of the message overhead are occupied by the 'compressed'
 * QoS policy values.
 */
#define MQ_BYTE0_RELIABILITY_OFFSET     (0)
#define MQ_BYTE0_OWNERSHIP_OFFSET       (1)
#define MQ_BYTE0_ORDERBY_OFFSET         (2)
#define MQ_BYTE0_AUTODISPOSE_OFFSET     (3)

#ifndef _FAST_ACCESS_
#define MQ_BYTE0_LATENCY_OFFSET         (4)
#define MQ_BYTE0_DEADLINE_OFFSET        (5)
#define MQ_BYTE0_LIVELINESS_OFFSET      (6)
#define MQ_BYTE0_LIFESPAN_OFFSET        (7)
#endif

#define MQ_BYTE1_DURABILITY_OFFSET      (0)
#define MQ_BYTE1_LIVELINESS_OFFSET      (2)
#define MQ_BYTE1_PRESENTATION_OFFSET    (4)
#define MQ_BYTE1_ORDERED_ACCESS_OFFSET  (6)
#define MQ_BYTE1_COHERENT_ACCESS_OFFSET (7)

/**
 * The following two macro`s are designed to help subsequent macro to
 * shift bits within a byte.
 */
#define _LSHIFT_(v,n) (((c_octet)(v))<<(n))
#define _RSHIFT_(v,n) (((c_octet)(v))>>(n))

/**
 * The following macro`s implement masks for each 'compressed' QoS policy.
 */
#define MQ_BYTE0_RELIABILITY_MASK     (0x01)
#define MQ_BYTE0_OWNERSHIP_MASK       (0x02)
#define MQ_BYTE0_ORDERBY_MASK         (0x04)
#define MQ_BYTE0_AUTODISPOSE_MASK     (0x08)

#ifndef _FAST_ACCESS_
#define MQ_BYTE0_LATENCY_MASK         (0x10)
#define MQ_BYTE0_DEADLINE_MASK        (0x20)
#define MQ_BYTE0_LIVELINESS_MASK      (0x40)
#define MQ_BYTE0_LIFESPAN_MASK        (0x80)
#endif

#define MQ_BYTE1_DURABILITY_MASK      (0x03)
#define MQ_BYTE1_LIVELINESS_MASK      (0x0c)
#define MQ_BYTE1_PRESENTATION_MASK    (0x30)
#define MQ_BYTE1_ORDERED_ACCESS_MASK  (0x40)
#define MQ_BYTE1_COHERENT_ACCESS_MASK (0x80)
/**
 * The following two helper macro`s are designed to help subsequent macro to
 * translate byte and bit indexes to mask and offset macro labels.
 */
#define _MASK_(byte,bit)   (MQ_BYTE##byte##_##bit##_MASK)
#define _OFFSET_(byte,bit) (MQ_BYTE##byte##_##bit##_OFFSET)

/**
 * The following two helper macro`s are designed to help subsequent macro to
 * translate specific bit values into the actual type value.
 */
#define _bitvalue_(qos,byte,bit) \
        ((((c_octet *)qos)[byte])&_MASK_(byte,bit))

#define _bitcast_(type,qos,byte,bit) \
        ((type)_RSHIFT_(_bitvalue_(qos,byte,bit),_OFFSET_(byte,bit)))

/**
 * The following macro`s return a boolean value indicating the state of the
 * specified QoS policy.
 */
#define v_messageQos_isReliable(qos) \
        (_bitcast_(c_bool,qos,0,RELIABILITY))
#define v_messageQos_isExclusive(qos) \
        (_bitcast_(c_bool,qos,0,OWNERSHIP))
#define v_messageQos_isBySource(qos) \
        (_bitcast_(c_bool,qos,0,ORDERBY))
#define v_messageQos_isAutoDispose(qos) \
        (_bitcast_(c_bool,qos,0,AUTODISPOSE))

#ifndef _FAST_ACCESS_
#define v_messageQos_isZeroLatency(qos) \
        (_bitcast_(c_bool,qos,0,LATENCY))
#define v_messageQos_isInfiniteDeadline(qos) \
        (_bitcast_(c_bool,qos,0,DEADLINE))
#define v_messageQos_isInfiniteLiveliness(qos) \
        (_bitcast_(c_bool,qos,0,LIVELINESS))
#define v_messageQos_isInfiniteLifespan(qos) \
        (_bitcast_(c_bool,qos,0,LIFESPAN))
#else
#define v_messageQos_isZeroLatency(qos) \
        (((c_octet *)qos)[2] == 0)
#define v_messageQos_isInfiniteDeadline(qos) \
        (((c_octet *)qos)[3] == 0)
#define v_messageQos_isInfiniteLiveliness(qos) \
        (((c_octet *)qos)[4] == 0)
#define v_messageQos_isInfiniteLifespan(qos) \
        (((c_octet *)qos)[5] == 0)
#endif
#define v_messageQos_isOrderedAccess(qos) \
        (_bitcast_(c_bool,qos,1,ORDERED_ACCESS))
#define v_messageQos_isCoherentAccess(qos) \
        (_bitcast_(c_bool,qos,1,COHERENT_ACCESS))

/**
 * The following macro`s return the QoS policy specific value of the
 * policy kind.
 */
#define v_messageQos_reliabilityKind(qos) \
        (_bitcast_(v_reliabilityKind,qos,0,RELIABILITY))
#define v_messageQos_ownershipKind(qos) \
        (_bitcast_(v_ownershipKind,qos,0,OWNERSHIP))
#define v_messageQos_orderbyKind(qos) \
        (_bitcast_(v_orderbyKind,qos,0,ORDERBY))
#define v_messageQos_durabilityKind(qos) \
        (_bitcast_(v_durabilityKind,qos,1,DURABILITY))
#define v_messageQos_livelinessKind(qos) \
        (_bitcast_(v_livelinessKind,qos,1,LIVELINESS))
#define v_messageQos_presentationKind(qos) \
        (_bitcast_(v_presentationKind,qos,1,PRESENTATION))

/**
 * The message QoS contructor creates a new message QoS object from
 * the writer`s QoS policy value.
 */
v_messageQos
v_messageQos_new(
    v_writer writer);

/**
 * This method constructs a new message QoS object from an existing
 * message QoS object. The new message QoS value will be an exact copy
 * of its origin. This method is only used by the group to create
 * implicit register messages for as long this is needed.
 */
v_messageQos
v_messageQos_copy(
    v_messageQos _this);

/**
 * This method verifies if thwe message QoS value is compatible with
 * the readers QoS value.
 */
c_bool
v_messageQos_isReaderCompatible (
    v_messageQos _this,
    v_reader reader);

/**
 * This method returns the message QoS ownership strength policy value.
 */
c_long
v_messageQos_getOwnershipStrength (
    v_messageQos qos);

/**
 * This method returns the message QoS transport priority policy value.
 */
c_long
v_messageQos_getTransportPriority(
    v_messageQos _this);

/**
 * This method returns the message QoS lifespan policy value.
 */
v_duration
v_messageQos_getLifespanPeriod(
    v_messageQos _this);

/**
 * This method returns the message QoS latency policy value.
 */
v_duration
v_messageQos_getLatencyPeriod(
    v_messageQos _this);

#endif
