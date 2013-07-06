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
#include "v_qos.h"
#include "v_subscriberQos.h"
#include "v_publisherQos.h"
#include "v_readerQos.h"
#include "v_writerQos.h"
#include "v_topicQos.h"
#include "v_participantQos.h"
#include "v_kernelQos.h"
#include "os_report.h"

v_qos
v_qosCreate(
    v_kernel kernel,
    v_qosKind kind)
{
    v_qos qos;
    c_base base;
    c_type type;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));

#define _CASE_(l,t) case l: type = c_resolve(base,t); break

    switch (kind) {
    _CASE_(V_PARTITION_QOS,        "kernelModule::v_partitionQos");
    _CASE_(V_PARTICIPANT_QOS,   "kernelModule::v_participantQos");
    _CASE_(V_TOPIC_QOS,         "kernelModule::v_topicQos");
    _CASE_(V_WRITER_QOS,        "kernelModule::v_writerQos");
    _CASE_(V_READER_QOS,        "kernelModule::v_readerQos");
    _CASE_(V_PUBLISHER_QOS,     "kernelModule::v_publisherQos");
    _CASE_(V_SUBSCRIBER_QOS,    "kernelModule::v_subscriberQos");
    _CASE_(V_INDEX_QOS,         "kernelModule::v_indexQos");
    _CASE_(V_WRITERHISTORY_QOS, "kernelModule::v_writerHistoryQos");
    _CASE_(V_GROUPHISTORY_QOS,  "kernelModule::v_groupHistoryQos");
    _CASE_(V_VIEW_QOS,          "kernelModule::v_viewQos");
    _CASE_(V_DATAVIEW_QOS,      "kernelModule::v_dataViewQos");
    _CASE_(V_KERNEL_QOS,        "kernelModule::v_kernelQos");
    default:
        OS_REPORT_1(OS_ERROR,"v_qos::Create",0,
                    "Illegal Qos kind specified (%s)",
                    v_qosKindImage(kind));
        return NULL;
    }

    qos = v_qos(c_new(type));
    c_free(type);
    if (qos) {
        qos->kind = kind;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_qosCreate",0,
                  "Failed to allocate qos.");
        assert(FALSE);
    }

    return qos;

#undef _CASE_
}

const c_char *
v_qosKindImage (
    v_qosKind kind)
{
#define _CASE_(o) case o: return #o

    switch (kind) {
    _CASE_(V_PARTITION_QOS);
    _CASE_(V_PARTICIPANT_QOS);
    _CASE_(V_TOPIC_QOS);
    _CASE_(V_WRITER_QOS);
    _CASE_(V_READER_QOS);
    _CASE_(V_PUBLISHER_QOS);
    _CASE_(V_SUBSCRIBER_QOS);
    _CASE_(V_INDEX_QOS);
    _CASE_(V_WRITERHISTORY_QOS);
    _CASE_(V_GROUPHISTORY_QOS);
    _CASE_(V_VIEW_QOS);
    _CASE_(V_DATAVIEW_QOS);
    _CASE_(V_KERNEL_QOS);
    default:
        return "Unknown Qos specified";
    }
#undef _CASE_
}

void
v_qosFree(
    v_qos qos)
{
    if(qos != NULL){
        switch(qos->kind){
        case V_PARTICIPANT_QOS:
            v_participantQosFree(v_participantQos(qos));
        break;
        case V_TOPIC_QOS:
            v_topicQosFree(v_topicQos(qos));
        break;
        case V_WRITER_QOS:
            v_writerQosFree(v_writerQos(qos));
        break;
        case V_READER_QOS:
            v_readerQosFree(v_readerQos(qos));
        break;
        case V_PUBLISHER_QOS:
            v_publisherQosFree(v_publisherQos(qos));
        break;
        case V_SUBSCRIBER_QOS:
            v_subscriberQosFree(v_subscriberQos(qos));
        break;
        case V_PARTITION_QOS:
        case V_INDEX_QOS:
        case V_WRITERHISTORY_QOS:
        case V_GROUPHISTORY_QOS:
        case V_VIEW_QOS:
        case V_DATAVIEW_QOS:
        case V_KERNEL_QOS:
        case V_COUNT_QOS:
        break;
        default:
        break;
        }
    }
}
