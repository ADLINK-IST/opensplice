/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "v_policy.h"
#include "v_messageQos.h"
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
v_messageQos_from_wqos_new(
    v_writerQos wqos,
    c_type msgQosType,
    v_presentationKind access_scope,
    c_bool coherent_access,
    c_bool ordered_access)
{
    v_messageQos _this;
    c_base base;
    v_duration tdur;
    c_ulong offset    = 6, /* byte0 + byte1 + transport_priority */
    strength_offset   = 0,
    latency_offset    = 0,
    deadline_offset   = 0,
    liveliness_offset = 0,
    lifespan_offset   = 0;

    c_octet byte0, byte1;

    c_octet *dst, *src;

    assert(C_TYPECHECK(wqos,v_writerQos));

    base = c_getBase(wqos);

    if (msgQosType == NULL) {
        msgQosType = c_metaArrayTypeNew(c_metaObject(base),
                                  "C_ARRAY<c_octet>",
                                  c_octet_t(base),
                                  0);
    }
    byte0 = (c_octet) (_LSHIFT_(wqos->reliability.v.kind, MQ_BYTE0_RELIABILITY_OFFSET) |
                       _LSHIFT_(wqos->ownership.v.kind, MQ_BYTE0_OWNERSHIP_OFFSET) |
                       _LSHIFT_(wqos->orderby.v.kind, MQ_BYTE0_ORDERBY_OFFSET) |
                       _LSHIFT_(wqos->lifecycle.v.autodispose_unregistered_instances, MQ_BYTE0_AUTODISPOSE_OFFSET));
    byte1 = (c_octet) (_LSHIFT_(wqos->durability.v.kind, MQ_BYTE1_DURABILITY_OFFSET) |
                       _LSHIFT_(wqos->liveliness.v.kind, MQ_BYTE1_LIVELINESS_OFFSET) |
                       /* writer->resend._d contains the access_scope of the publisher-QoS */
                       _LSHIFT_(access_scope, MQ_BYTE1_PRESENTATION_OFFSET) |
                       _LSHIFT_(coherent_access, MQ_BYTE1_COHERENT_ACCESS_OFFSET) |
                       _LSHIFT_(ordered_access, MQ_BYTE1_ORDERED_ACCESS_OFFSET));

    if (wqos->ownership.v.kind == V_OWNERSHIP_EXCLUSIVE) {
        strength_offset = offset;
        offset += (c_ulong) sizeof(wqos->strength.v.value);
    }
    if (OS_DURATION_ISZERO(wqos->latency.v.duration)) {
        byte0 = (c_octet) (byte0 | _LSHIFT_(1,MQ_BYTE0_LATENCY_OFFSET));
    } else {
        latency_offset = offset;
        offset += (c_ulong) sizeof(wqos->latency.v.duration);
    }
    if (OS_DURATION_ISINFINITE(wqos->deadline.v.period)) {
        byte0 = (c_octet) (byte0 | _LSHIFT_(1,MQ_BYTE0_DEADLINE_OFFSET));
    } else {
        deadline_offset = offset;
        offset += (c_ulong) sizeof(wqos->deadline.v.period);
    }
    if (OS_DURATION_ISINFINITE(wqos->liveliness.v.lease_duration)) {
        byte0 = (c_octet) (byte0 | _LSHIFT_(1,MQ_BYTE0_LIVELINESS_OFFSET));
    } else {
        liveliness_offset = offset;
        offset += (c_ulong) sizeof(wqos->liveliness.v.lease_duration);
    }
    if (OS_DURATION_ISINFINITE(wqos->lifespan.v.duration)) {
        byte0 = (c_octet) (byte0 | _LSHIFT_(1,MQ_BYTE0_LIFESPAN_OFFSET));
    } else {
        lifespan_offset = offset;
        offset += (c_ulong) sizeof(wqos->lifespan.v.duration);
    }

    _this = c_newArray((c_collectionType)msgQosType,offset);

    if (_this) {
        ((c_octet *)_this)[0] = byte0;
        ((c_octet *)_this)[1] = byte1;
        src = (c_octet *)&wqos->transport.v.value;
        dst = (c_octet *)&((c_octet *)_this)[2];
        _COPY4_(dst,src);

        if (strength_offset) {
            src = (c_octet *)&wqos->strength.v.value;
            dst = (c_octet *)&((c_octet *)_this)[strength_offset];
            _COPY4_(dst,src);
        }
        if (latency_offset) {
            tdur = v_durationFromOsDuration(wqos->latency.v.duration);
            src = (c_octet *)&tdur;
            dst = (c_octet *)&((c_octet *)_this)[latency_offset];
            _COPY8_(dst,src);
        }
        if (deadline_offset) {
            tdur = v_durationFromOsDuration(wqos->deadline.v.period);
            src = (c_octet *)&tdur;
            dst = (c_octet *)&((c_octet *)_this)[deadline_offset];
            _COPY8_(dst,src);
        }
        if (liveliness_offset) {
            tdur = v_durationFromOsDuration(wqos->liveliness.v.lease_duration);
            src = (c_octet *)&tdur;
            dst = (c_octet *)&((c_octet *)_this)[liveliness_offset];
            _COPY8_(dst,src);
        }
        if (lifespan_offset) {
            tdur = v_durationFromOsDuration(wqos->lifespan.v.duration);
            src = (c_octet *)&tdur;
            dst = (c_octet *)&((c_octet *)_this)[lifespan_offset];
            _COPY8_(dst,src);
        }
    } else {
        OS_REPORT(OS_CRITICAL,
                  "v_messageQos_new",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate messageQos.");
        assert(FALSE);
    }
    return _this;
}


v_messageQos
v_messageQos_new(
    v_writer writer)
{
    assert(C_TYPECHECK(writer,v_writer));

    return v_messageQos_from_wqos_new(writer->qos, writer->msgQosType, writer->resend._d, writer->coherent_access, writer->ordered_access);
}

v_messageQos
v_messageQos_copy (
    v_messageQos src)
{
    v_messageQos _this;
    c_ulong size;
    c_type type = NULL;

    size = c_arraySize(src);
    type = c_getType(src);
    _this = c_newArray((c_collectionType)type,size);
    if (_this) {
        memcpy(_this,src,size);
    } else {
        OS_REPORT(OS_CRITICAL,
                  "v_messageQos_copy",V_RESULT_INTERNAL_ERROR,
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

os_duration
v_messageQos_getLifespanPeriod(
    v_messageQos qos)
{
    os_duration period;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isInfiniteLifespan(qos)) {
        period = OS_DURATION_INFINITE;
    } else {
        v_duration tmp;
        dst = (c_octet *)&tmp;
        src = &((c_octet *)qos)[6+
                                v_messageQos_strengthSize(qos)+
                                v_messageQos_latencySize(qos)+
                                v_messageQos_deadlineSize(qos)+
                                v_messageQos_livelinessSize(qos)];
        _COPY8_(dst,src);
        period = v_durationToOsDuration(tmp);
    }
    return period;
}

os_duration
v_messageQos_getLatencyPeriod(
    v_messageQos qos)
{
    os_duration budget;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isZeroLatency(qos)) {
        budget = OS_DURATION_ZERO;
    } else {
        v_duration tmp;
        dst = (c_octet *)&tmp;
        src = &(((c_octet *)qos))[6+v_messageQos_strengthSize(qos)];
        _COPY8_(dst,src);
        budget = v_durationToOsDuration(tmp);
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
        strength = 0; /* don't care value */
    }
    return strength;
}

os_duration
v_messageQos_getDeadlinePeriod(
    v_messageQos qos)
{
    os_duration period;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isInfiniteDeadline(qos)) {
        period = OS_DURATION_INFINITE;
    } else {
        v_duration tmp;
        dst = (c_octet *)&tmp;
        src = &((c_octet *)qos)[6+
                                v_messageQos_strengthSize(qos)+
                                v_messageQos_latencySize(qos)];
        _COPY8_(dst,src);
        period = v_durationToOsDuration(tmp);
    }
    return period;
}

os_duration
v_messageQos_getLivelinessPeriod(
    v_messageQos qos)
{
    os_duration period;
    c_octet *dst;
    c_octet *src;

    if (v_messageQos_isInfiniteLiveliness(qos)) {
        period = OS_DURATION_INFINITE;
    } else {
        v_duration tmp;
        dst = (c_octet *)&tmp;
        src = &((c_octet *)qos)[6+
                                v_messageQos_strengthSize(qos)+
                                v_messageQos_latencySize(qos)+
                                v_messageQos_deadlineSize(qos)];
        _COPY8_(dst,src);
        period = v_durationToOsDuration(tmp);
    }
    return period;
}

c_bool
v_messageQos_isReaderCompatible (
    v_messageQos _this,
    v_reader r)
{
    v_duration duration;
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

    if ((qos->reliability.v.kind == V_RELIABILITY_RELIABLE) &&
        !v_messageQos_isReliable(_this)) {
        return FALSE;
    }
    if (qos->durability.v.kind > v_messageQos_durabilityKind(_this)) {
        return FALSE;
    }
    if (qos->ownership.v.kind != v_messageQos_ownershipKind(_this)) {
        return FALSE;
    }
    if (r->subQos->presentation.v.access_scope > v_messageQos_presentationKind(_this)) {
        return FALSE;
    }
    if ((r->subQos->presentation.v.ordered_access == TRUE) &&
        (!v_messageQos_isOrderedAccess(_this))) {
        return FALSE;
    }
    if ((r->subQos->presentation.v.coherent_access == TRUE) &&
        (!v_messageQos_isCoherentAccess(_this))) {
        return FALSE;
    }
    if (qos->liveliness.v.kind > v_messageQos_livelinessKind(_this)) {
        return FALSE;
    }
    if (qos->orderby.v.kind > v_messageQos_orderbyKind(_this)) {
        return FALSE;
    }

    dst = (c_octet *)&duration;

    offset = 6 + v_messageQos_strengthSize(_this);
    if (!v_messageQos_isZeroLatency(_this)) {
        if (!OS_DURATION_ISINFINITE(qos->latency.v.duration)) {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (os_durationCompare(v_durationToOsDuration(duration), qos->latency.v.duration) == OS_MORE) {
                return FALSE;
            }
        }
        offset += 8;
    }

    if (!OS_DURATION_ISINFINITE(qos->deadline.v.period)) {
        if (v_messageQos_isInfiniteDeadline(_this)) {
            return FALSE;
        } else {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (os_durationCompare(v_durationToOsDuration(duration), qos->deadline.v.period) == OS_MORE) {
                return FALSE;
            }
        }
    }
    offset += v_messageQos_deadlineSize(_this);

    if (!OS_DURATION_ISINFINITE(qos->liveliness.v.lease_duration)) {
        if (!v_messageQos_isInfiniteLiveliness(_this)) {
            src = &(((c_octet *)_this))[offset];
            _COPY8_(dst,src);
            if (os_durationCompare(v_durationToOsDuration(duration), qos->liveliness.v.lease_duration) == OS_MORE) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

