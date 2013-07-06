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
#include "v_policy.h"
#include "v__messageQos.h"
#include "os_report.h"
#include "os_abstract.h" /* big or little endianness */

#define v_messageQos_strengthSize(qos)   (v_messageQos_isExclusive(qos)?4:0)
#define v_messageQos_latencySize(qos)    (v_messageQos_isZeroLatency(qos)?0:8)
#define v_messageQos_deadlineSize(qos)   (v_messageQos_isInfiniteDeadline(qos)?0:8)
#define v_messageQos_livelinessSize(qos) (v_messageQos_isInfiniteLiveliness(qos)?0:8)
#define v_messageQos_lifespanSize(qos)   (v_messageQos_isInfiniteLifespan(qos)?0:8)

#ifdef PA_BIG_ENDIAN
#define _COPY4_(d,s) d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3]
#define _COPY8_(d,s) d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3]; \
                     d[4]=s[4];d[5]=s[5];d[6]=s[6];d[7]=s[7]
#endif
#ifdef PA_LITTLE_ENDIAN
#define _COPY4_(d,s) d[0]=s[3];d[1]=s[2];d[2]=s[1];d[3]=s[0]
#define _COPY8_(d,s) d[0]=s[3];d[1]=s[2];d[2]=s[1];d[3]=s[0]; \
                     d[4]=s[7];d[5]=s[6];d[6]=s[5];d[7]=s[4]
#endif

v_messageQos
v_messageQos_new(
    v_writer writer)
{
    v_messageQos _this;
    v_writerQos wqos;
    c_base base;
#ifdef _FAST_ACCESS_
    c_long offset     = 10, /* byte0 + byte1 + transport_priority + 4*time_offsets */
#else
    c_long offset     = 6, /* byte0 + byte1 + transport_priority */
#endif
    strength_offset   = 0,
    latency_offset    = 0,
    deadline_offset   = 0,
    liveliness_offset = 0,
    lifespan_offset   = 0;

    c_octet byte0 = 0,
            byte1 = 0;

    c_octet *dst, *src;

    v_publisherQos pqos = writer->pubQos;

    assert(C_TYPECHECK(writer,v_writer));

    wqos = writer->qos;
    base = c_getBase(writer);

    if (writer->msgQosType == NULL) {
        writer->msgQosType = c_metaArrayTypeNew(c_metaObject(base),
                                  "C_ARRAY<c_octet>",
                                  c_octet_t(base),
                                  0);
    }
    byte0 |= _LSHIFT_(wqos->reliability.kind,
                      MQ_BYTE0_RELIABILITY_OFFSET);
    byte0 |= _LSHIFT_(wqos->ownership.kind,
                      MQ_BYTE0_OWNERSHIP_OFFSET);
    byte0 |= _LSHIFT_(wqos->orderby.kind,
                      MQ_BYTE0_ORDERBY_OFFSET);
    byte0 |= _LSHIFT_(wqos->lifecycle.autodispose_unregistered_instances,
                      MQ_BYTE0_AUTODISPOSE_OFFSET);

    byte1 |= _LSHIFT_(wqos->durability.kind,
                      MQ_BYTE1_DURABILITY_OFFSET);
    byte1 |= _LSHIFT_(wqos->liveliness.kind,
                      MQ_BYTE1_LIVELINESS_OFFSET);
    byte1 |= _LSHIFT_(pqos->presentation.access_scope,
                      MQ_BYTE1_PRESENTATION_OFFSET);
    byte1 |= _LSHIFT_(pqos->presentation.coherent_access,
                      MQ_BYTE1_COHERENT_ACCESS_OFFSET);
    byte1 |= _LSHIFT_(pqos->presentation.ordered_access,
                      MQ_BYTE1_ORDERED_ACCESS_OFFSET);

    if (wqos->ownership.kind == V_OWNERSHIP_EXCLUSIVE) {
        strength_offset = offset;
        offset += sizeof(wqos->strength.value);
    }
#ifdef _FAST_ACCESS_
    if (!c_timeIsZero(wqos->latency.duration)) {
        latency_offset = offset;
        offset += sizeof(wqos->latency.duration);
    }
    if (!c_timeIsInfinite(wqos->deadline.period)) {
        deadline_offset = offset;
        offset += sizeof(wqos->deadline.period);
    }
    if (!c_timeIsInfinite(wqos->liveliness.lease_duration)) {
        liveliness_offset = offset;
        offset += sizeof(wqos->liveliness.lease_duration);
    }
    if (!c_timeIsInfinite(wqos->lifespan.duration)) {
        lifespan_offset = offset;
        offset += sizeof(wqos->lifespan.duration);
    }
#else
    if (c_timeIsZero(wqos->latency.duration)) {
        byte0 |= _LSHIFT_(1,MQ_BYTE0_LATENCY_OFFSET);
    } else {
        latency_offset = offset;
        offset += sizeof(wqos->latency.duration);
    }
    if (c_timeIsInfinite(wqos->deadline.period)) {
        byte0 |= _LSHIFT_(1,MQ_BYTE0_DEADLINE_OFFSET);
    } else {
        deadline_offset = offset;
        offset += sizeof(wqos->deadline.period);
    }
    if (c_timeIsInfinite(wqos->liveliness.lease_duration)) {
        byte0 |= _LSHIFT_(1,MQ_BYTE0_LIVELINESS_OFFSET);
    } else {
        liveliness_offset = offset;
        offset += sizeof(wqos->liveliness.lease_duration);
    }
    if (c_timeIsInfinite(wqos->lifespan.duration)) {
        byte0 |= _LSHIFT_(1,MQ_BYTE0_LIFESPAN_OFFSET);
    } else {
        lifespan_offset = offset;
        offset += sizeof(wqos->lifespan.duration);
    }
#endif

    _this = c_newArray((c_collectionType)writer->msgQosType,offset);

    if (_this) {
        ((c_octet *)_this)[0] = byte0;
        ((c_octet *)_this)[1] = byte1;
#ifdef _FAST_ACCESS_
        ((c_octet *)_this)[2] = latency_offset;
        ((c_octet *)_this)[3] = deadline_offset;
        ((c_octet *)_this)[4] = liveliness_offset;
        ((c_octet *)_this)[5] = lifespan_offset;

        src = (c_octet *)&wqos->transport.value;
        dst = (c_octet *)&((c_octet *)_this)[6];
        _COPY4_(dst,src);
#else
        src = (c_octet *)&wqos->transport.value;
        dst = (c_octet *)&((c_octet *)_this)[2];
        _COPY4_(dst,src);
#endif

        if (strength_offset) {
            src = (c_octet *)&wqos->strength.value;
            dst = (c_octet *)&((c_octet *)_this)[strength_offset];
            _COPY4_(dst,src);
        }
        if (latency_offset) {
            src = (c_octet *)&wqos->latency.duration;
            dst = (c_octet *)&((c_octet *)_this)[latency_offset];
            _COPY8_(dst,src);
        }
        if (deadline_offset) {
            src = (c_octet *)&wqos->deadline.period;
            dst = (c_octet *)&((c_octet *)_this)[deadline_offset];
            _COPY8_(dst,src);
        }
        if (liveliness_offset) {
            src = (c_octet *)&wqos->liveliness.lease_duration;
            dst = (c_octet *)&((c_octet *)_this)[liveliness_offset];
            _COPY8_(dst,src);
        }
        if (lifespan_offset) {
            src = (c_octet *)&wqos->lifespan.duration;
            dst = (c_octet *)&((c_octet *)_this)[lifespan_offset];
            _COPY8_(dst,src);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_messageQos_new",0,
                  "Failed to allocate messageQos.");
        assert(FALSE);
    }
    return _this;
}

v_messageQos
v_messageQos_copy (
    v_messageQos src)
{
    v_messageQos _this;
    c_long size;
    c_type type = NULL;

    size = c_arraySize(src);
    type = c_getType(src);
    _this = c_newArray((c_collectionType)type,size);
    if (_this) {
        memcpy(_this,src,size);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_messageQos_copy",0,
                  "Failed to allocate messageQos.");
    }

    return _this;
}


/**
 * Endianess independent Getters.
 */
c_long
v_messageQos_getTransportPriority(
    v_messageQos qos)
{
    c_long priority;
    c_octet *dst = (c_octet *)&priority;
    c_octet *src = &((c_octet *)qos)[2];
    _COPY4_(dst,src);
    return priority;
}

v_duration
v_messageQos_getLifespanPeriod(
    v_messageQos qos)
{
    v_duration period;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isInfiniteLifespan(qos)) {
        period = C_TIME_INFINITE;
    } else {
        dst = (c_octet *)&period;
#ifdef _FAST_ACCESS_
        src = &((c_octet *)qos)[((c_long)((c_octet *)qos)[5])];
#else
        src = &((c_octet *)qos)[6+
                                v_messageQos_strengthSize(qos)+
                                v_messageQos_latencySize(qos)+
                                v_messageQos_deadlineSize(qos)+
                                v_messageQos_livelinessSize(qos)];
#endif
        _COPY8_(dst,src);
    }
    return period;
}

v_duration
v_messageQos_getLatencyPeriod(
    v_messageQos qos)
{
    v_duration budget;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isZeroLatency(qos)) {
        budget = C_TIME_ZERO;
    } else {
        dst = (c_octet *)&budget;
#ifdef _FAST_ACCESS_
        src = &((c_octet *)qos)[((c_long)((c_octet *)qos)[2])];
#else
        src = &(((c_octet *)qos))[6+v_messageQos_strengthSize(qos)];
#endif
        _COPY8_(dst,src);
    }
    return budget;
}

c_long
v_messageQos_getOwnershipStrength(
    v_messageQos qos)
{
    c_long strength;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isExclusive(qos)) {
        dst = (c_octet *)&strength;
        src = &(((c_octet *)qos))[6];
        _COPY4_(dst,src);
    } else {
        strength = 0; /* don care value */
    }
    return strength;
}

v_duration
v_messageQos_getDeadlinePeriod(
    v_messageQos qos)
{
    v_duration period;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isInfiniteDeadline(qos)) {
        period = C_TIME_INFINITE;
    } else {
        dst = (c_octet *)&period;
#ifdef _FAST_ACCESS_
        src = &((c_octet *)qos)[((c_long)((c_octet *)qos)[3])];
#else
        src = &((c_octet *)qos)[6+
                                v_messageQos_strengthSize(qos)+
                                v_messageQos_latencySize(qos)];
#endif
        _COPY8_(dst,src);
    }
    return period;
}

v_duration
v_messageQos_getLivelinessPeriod(
    v_messageQos qos)
{
    v_duration period;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isInfiniteLiveliness(qos)) {
        period = C_TIME_INFINITE;
    } else {
        dst = (c_octet *)&period;
#ifdef _FAST_ACCESS_
        src = &((c_octet *)qos)[((c_long)((c_octet *)qos)[4])];
#else
        src = &((c_octet *)qos)[6+
                                v_messageQos_strengthSize(qos)+
                                v_messageQos_latencySize(qos)+
                                v_messageQos_deadlineSize(qos)];
#endif
        _COPY8_(dst,src);
    }
    return period;
}

c_bool
v_messageQos_isReaderCompatible (
    v_messageQos _this,
    v_reader r)
{
    c_time time;
    c_octet *dst, *src;
    c_long offset;
    v_readerQos qos;

    qos = r->qos;
    if (_this == NULL) {
        /* Messages without Qos are implied lifecycle changes (for example resulting
         * from a call to the dispose_all_data operation on the topic) and should
         * always be delivered to all readers. Readers that have no corresponding
         * instance will automatically ignore these implied lifecycle changes.
         */
        return TRUE;
    }

    if ((qos->reliability.kind == V_RELIABILITY_RELIABLE) &&
        !v_messageQos_isReliable(_this)) {
        return FALSE;
    }
    if (qos->durability.kind > v_messageQos_durabilityKind(_this)) {
        return FALSE;
    }
    if (qos->ownership.kind != v_messageQos_ownershipKind(_this)) {
        return FALSE;
    }
    if (r->subQos->presentation.access_scope > v_messageQos_presentationKind(_this)) {
        return FALSE;
    }
    if ((r->subQos->presentation.ordered_access == TRUE) &&
        (!v_messageQos_isOrderedAccess(_this))) {
        return FALSE;
    }
    if ((r->subQos->presentation.coherent_access == TRUE) &&
        (!v_messageQos_isCoherentAccess(_this))) {
        return FALSE;
    }
    if (qos->liveliness.kind > v_messageQos_livelinessKind(_this)) {
        return FALSE;
    }
    if (qos->orderby.kind > v_messageQos_orderbyKind(_this)) {
        return FALSE;
    }

    dst = (c_octet *)&time;

#ifdef _FAST_ACCESS_
    offset = ((c_octet *)_this)[2];
    if (offset) {
        if (!c_timeIsInfinite(qos->latency.duration)) {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (c_timeCompare(time, qos->latency.duration) == C_GT) {
                return FALSE;
            }
        }
    }
#else
    offset = 6 + v_messageQos_strengthSize(_this);
    if (!v_messageQos_isZeroLatency(_this)) {
        if (!c_timeIsInfinite(qos->latency.duration)) {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (c_timeCompare(time, qos->latency.duration) == C_GT) {
                return FALSE;
            }
        }
        offset += 8;
    }
#endif
#ifdef _FAST_ACCESS_
    if (!c_timeIsInfinite(qos->deadline.period)) {
        offset = ((c_octet *)_this)[3];
        if (offset) {
            return FALSE;
        } else {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (c_timeCompare(time, qos->deadline.period) == C_GT) {
                return FALSE;
            }
        }
    }
#else
    if (!c_timeIsInfinite(qos->deadline.period)) {
        if (v_messageQos_isInfiniteDeadline(_this)) {
            return FALSE;
        } else {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (c_timeCompare(time, qos->deadline.period) == C_GT) {
                return FALSE;
            }
        }
    }
    offset += v_messageQos_deadlineSize(_this);
#endif
#ifdef _FAST_ACCESS_
    if (!c_timeIsInfinite(qos->liveliness.lease_duration)) {
        offset = ((c_octet *)_this)[4];
        if (offset) {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (c_timeCompare(time, qos->liveliness.lease_duration) == C_GT) {
                return FALSE;
            }
        }
    }
#else
    if (!c_timeIsInfinite(qos->liveliness.lease_duration)) {
        if (!v_messageQos_isInfiniteLiveliness(_this)) {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (c_timeCompare(time, qos->liveliness.lease_duration) == C_GT) {
                return FALSE;
            }
        }
    }
#endif
    return TRUE;
}

