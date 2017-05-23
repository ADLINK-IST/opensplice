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
    c_base base,
    v_qosKind kind)
{
    v_qos qos;
    c_type type;

#define _CASE_(l,t) case l: type = c_resolve(base,t); break

    switch (kind) {
    _CASE_(V_PARTITION_QOS,     "kernelModuleI::v_partitionQos");
    _CASE_(V_PARTICIPANT_QOS,   "kernelModuleI::v_participantQos");
    _CASE_(V_TOPIC_QOS,         "kernelModuleI::v_topicQos");
    _CASE_(V_WRITER_QOS,        "kernelModuleI::v_writerQos");
    _CASE_(V_READER_QOS,        "kernelModuleI::v_readerQos");
    _CASE_(V_PUBLISHER_QOS,     "kernelModuleI::v_publisherQos");
    _CASE_(V_SUBSCRIBER_QOS,    "kernelModuleI::v_subscriberQos");
    _CASE_(V_INDEX_QOS,         "kernelModuleI::v_indexQos");
    _CASE_(V_WRITERHISTORY_QOS, "kernelModuleI::v_writerHistoryQos");
    _CASE_(V_GROUPHISTORY_QOS,  "kernelModuleI::v_groupHistoryQos");
    _CASE_(V_VIEW_QOS,          "kernelModuleI::v_viewQos");
    _CASE_(V_DATAVIEW_QOS,      "kernelModuleI::v_dataViewQos");
    _CASE_(V_KERNEL_QOS,        "kernelModuleI::v_kernelQos");
    default:
        OS_REPORT(OS_CRITICAL,"v_qos::Create",V_RESULT_ILL_PARAM,
                    "Illegal Qos kind specified (%s)",
                    v_qosKindImage(kind));
        return NULL;
    }

    qos = v_qos(c_new_s(type));
    c_free(type);
    if (qos) {
        qos->kind = kind;
    } else {
        OS_REPORT(OS_FATAL,
                  "v_qosCreate",V_RESULT_INTERNAL_ERROR,
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
