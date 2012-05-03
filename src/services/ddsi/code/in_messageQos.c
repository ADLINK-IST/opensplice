#include "v_policy.h"
#include "v__messageQos.h"
#include "in_messageQos.h"
#include "os_abstract.h" /* big or little endianness */
#include "in__ddsiPublication.h"

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

static c_type _v_messageQos_t = NULL;

c_type
in_messageQosType (
    c_base base)
{
    if (_v_messageQos_t == NULL) {
        _v_messageQos_t = c_resolve(base,"kernelModule::v_messageQos");
    }
    return _v_messageQos_t;
}



v_messageQos
in_messageQos_new(
    in_connectivityPeerWriter writer, c_base base)
{

    in_ddsiDiscoveredWriterData i =
        in_connectivityPeerWriterGetInfo(writer);

    /* inefficient, should be permenanent */
    c_type type;
    v_messageQos _this ;
    c_long transportQosValue = 0; /* TODO */
#ifdef _FAST_ACCESS_
    c_long offset            = 10, /* byte0 + byte1 + transport_priority + 4*time_offsets */
#else
    c_long offset            = 6, /* byte0 + byte1 + transport_priority */
#endif
           strength_offset   = 0,
           latency_offset    = 0,
           deadline_offset   = 0,
           liveliness_offset = 0,
           lifespan_offset   = 0;

    c_octet byte0 = 0,
            byte1 = 0;

    c_octet *dst, *src;

    type = c_metaArrayTypeNew(c_metaObject(base),
                              "C_ARRAY<c_octet>",
                              c_octet_t(base),
                              0);

    _this = c_newArray((c_collectionType)type,offset);
    c_free(type); /* remove if "type" becomes permenanent */

    if (_this) {
        byte0 |= _LSHIFT_(i->topicData.info.reliability.kind,
                          MQ_BYTE0_RELIABILITY_OFFSET);
        byte0 |= _LSHIFT_(i->topicData.info.ownership.kind,
                          MQ_BYTE0_OWNERSHIP_OFFSET);
        byte0 |= _LSHIFT_(i->topicData.info.destination_order.kind,
                          MQ_BYTE0_ORDERBY_OFFSET);
        byte0 |= _LSHIFT_(i->topicData.info.lifecycle.autodispose_unregistered_instances,
                          MQ_BYTE0_AUTODISPOSE_OFFSET);
        byte1 |= _LSHIFT_(i->topicData.info.durability.kind,
                          MQ_BYTE1_DURABILITY_OFFSET);
        byte1 |= _LSHIFT_(i->topicData.info.liveliness.kind,
                          MQ_BYTE1_LIVELINESS_OFFSET);
        byte1 |= _LSHIFT_(i->topicData.info.presentation.access_scope,
                          MQ_BYTE1_PRESENTATION_OFFSET);
        byte1 |= _LSHIFT_(i->topicData.info.presentation.coherent_access,
                          MQ_BYTE1_ORDERED_ACCESS_OFFSET);
        byte1 |= _LSHIFT_(i->topicData.info.presentation.ordered_access,
                          MQ_BYTE1_COHERENT_ACCESS_OFFSET);

        if (i->topicData.info.ownership.kind == V_OWNERSHIP_EXCLUSIVE) {
            strength_offset = offset;
            offset += sizeof(i->topicData.info.ownership_strength.value);
        }

        if (c_timeIsZero(i->topicData.info.latency_budget.duration)) {
            byte0 |= _LSHIFT_(1,MQ_BYTE0_LATENCY_OFFSET);
        } else {
            latency_offset = offset;
            offset += sizeof(i->topicData.info.latency_budget.duration);
        }
        if (c_timeIsInfinite(i->topicData.info.deadline.period)) {
            byte0 |= _LSHIFT_(1,MQ_BYTE0_DEADLINE_OFFSET);
        } else {
            deadline_offset = offset;
            offset += sizeof(i->topicData.info.deadline.period);
        }
        if (c_timeIsInfinite(i->topicData.info.liveliness.lease_duration)) {
            byte0 |= _LSHIFT_(1,MQ_BYTE0_LIVELINESS_OFFSET);
        } else {
            liveliness_offset = offset;
            offset += sizeof(i->topicData.info.liveliness.lease_duration);
        }
        if (c_timeIsInfinite(i->topicData.info.lifespan.duration)) {
            byte0 |= _LSHIFT_(1,MQ_BYTE0_LIFESPAN_OFFSET);
        } else {
            lifespan_offset = offset;
            offset += sizeof(i->topicData.info.lifespan.duration);
        }

        ((c_octet *)_this)[0] = byte0;
        ((c_octet *)_this)[1] = byte1;

        src = (c_octet *)&(transportQosValue); /* TODO fix*/
        dst = (c_octet *)&((c_octet *)_this)[2];
        _COPY4_(dst,src);

        if (strength_offset) {
            src = (c_octet *)&(i->topicData.info.ownership_strength.value);
            dst = (c_octet *)&((c_octet *)_this)[strength_offset];
            _COPY4_(dst,src);
        }
        if (latency_offset) {
            src = (c_octet *)&(i->topicData.info.latency_budget.duration);
            dst = (c_octet *)&((c_octet *)_this)[latency_offset];
            _COPY8_(dst,src);
        }
        if (deadline_offset) {
            src = (c_octet *)&(i->topicData.info.deadline.period);
            dst = (c_octet *)&((c_octet *)_this)[deadline_offset];
            _COPY8_(dst,src);
        }
        if (liveliness_offset) {
            src = (c_octet *)&(i->topicData.info.liveliness.lease_duration);
            dst = (c_octet *)&((c_octet *)_this)[liveliness_offset];
            _COPY8_(dst,src);
        }
        if (lifespan_offset) {
            src = (c_octet *)&(i->topicData.info.lifespan.duration);
            dst = (c_octet *)&((c_octet *)_this)[lifespan_offset];
            _COPY8_(dst,src);
        }
    } else {
        assert(FALSE);
    }

    return _this;
}

