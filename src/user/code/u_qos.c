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

#include "u__qos.h"

#include "os_heap.h"
#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
v_qos
u_qosNew(
    v_qos tmpl)
{
    v_qos q;
    
    q = NULL;
    if (tmpl != NULL) {
        switch (tmpl->kind) {
        case V_PARTITION_QOS:
            q = (v_qos)u_partitionQosNew((v_partitionQos)tmpl);
        break;
        case V_PARTICIPANT_QOS:
            q = (v_qos)u_participantQosNew((v_participantQos)tmpl);
        break;
        case V_TOPIC_QOS:
            q = (v_qos)u_topicQosNew((v_topicQos)tmpl);
        break;
        case V_WRITER_QOS:
            q = (v_qos)u_writerQosNew((v_writerQos)tmpl);
        break;
        case V_READER_QOS:
            q = (v_qos)u_readerQosNew((v_readerQos)tmpl);
        break;
        case V_PUBLISHER_QOS:
            q = (v_qos)u_publisherQosNew((v_publisherQos)tmpl);
        break;
        case V_SUBSCRIBER_QOS:
            q = (v_qos)u_subscriberQosNew((v_subscriberQos)tmpl);
        break;
        case V_DATAVIEW_QOS:
            q = (v_qos)u_dataViewQosNew((v_dataViewQos)tmpl);
        break;
        default:
            OS_REPORT_1(OS_ERROR, "u_qosNew", 0, "unsupported qos %d", tmpl->kind);
        break;
        }
    }

    return q;
}

/**************************************************************
 * Public functions
 **************************************************************/
