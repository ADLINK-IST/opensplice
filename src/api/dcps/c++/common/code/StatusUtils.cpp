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
#include "SequenceUtils.h"
#include "StatusUtils.h"


/*
 * Status conversions
 */
DDS::StatusMask
DDS::OpenSplice::Utils::vEventMaskToStatusMask (
    const v_eventMask vMask,
    const v_kind      vKind)
{
    DDS::StatusMask mask = 0;

    switch(vKind) {
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        if (vMask & V_EVENT_INCONSISTENT_TOPIC) {
            mask |= DDS::INCONSISTENT_TOPIC_STATUS;
        }
        if (vMask & V_EVENT_ALL_DATA_DISPOSED) {
            mask |= DDS::ALL_DATA_DISPOSED_TOPIC_STATUS;
        }
    break;
    case K_SUBSCRIBER:
        if (vMask & V_EVENT_ON_DATA_ON_READERS) {
            mask |= DDS::DATA_ON_READERS_STATUS;
        }
    break;
    case K_WRITER:
        if (vMask & V_EVENT_LIVELINESS_LOST) {
            mask |= DDS::LIVELINESS_LOST_STATUS;
        }
        if (vMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            mask |= DDS::OFFERED_DEADLINE_MISSED_STATUS;
        }
        if (vMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            mask |= DDS::OFFERED_INCOMPATIBLE_QOS_STATUS;
        }
        if (vMask & V_EVENT_PUBLICATION_MATCHED) {
            mask |= DDS::PUBLICATION_MATCHED_STATUS;
        }
    break;
    case K_READER:
    case K_GROUPQUEUE:
    case K_DATAREADER:
    case K_GROUPSTREAM:
        if (vMask & V_EVENT_SAMPLE_REJECTED) {
            mask |= DDS::SAMPLE_REJECTED_STATUS;
        }
        if (vMask & V_EVENT_LIVELINESS_CHANGED) {
            mask |= DDS::LIVELINESS_CHANGED_STATUS;
        }
        if (vMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            mask |= DDS::REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if (vMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            mask |= DDS::REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if (vMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            mask |= DDS::SUBSCRIPTION_MATCHED_STATUS;
        }
        if (vMask & V_EVENT_DATA_AVAILABLE) {
            mask |= DDS::DATA_AVAILABLE_STATUS;
        }
        if (vMask & V_EVENT_SAMPLE_LOST) {
            mask |= DDS::SAMPLE_LOST_STATUS;
        }
    break;
    case K_PARTICIPANT:
    case K_PUBLISHER:
    case K_DOMAIN:
    case K_KERNEL:
    break;
    default:
        assert(FALSE);
    }

    return mask;
}

v_eventMask
DDS::OpenSplice::Utils::vEventMaskFromStatusMask (
    const DDS::StatusMask mask)
{
    v_eventMask vMask = 0;

    if (mask & DDS::INCONSISTENT_TOPIC_STATUS) {
        vMask |= V_EVENT_INCONSISTENT_TOPIC;
    }
    if (mask & DDS::LIVELINESS_LOST_STATUS) {
        vMask |= V_EVENT_LIVELINESS_LOST;
    }
    if (mask & DDS::OFFERED_DEADLINE_MISSED_STATUS) {
        vMask |= V_EVENT_OFFERED_DEADLINE_MISSED;
    }
    if (mask & DDS::OFFERED_INCOMPATIBLE_QOS_STATUS) {
        vMask |= V_EVENT_OFFERED_INCOMPATIBLE_QOS;
    }
    if (mask & DDS::DATA_ON_READERS_STATUS) {
        vMask |= V_EVENT_ON_DATA_ON_READERS;
    }
    if (mask & DDS::SAMPLE_LOST_STATUS) {
        vMask |= V_EVENT_SAMPLE_LOST;
    }
    if (mask & DDS::DATA_AVAILABLE_STATUS) {
        vMask |= V_EVENT_DATA_AVAILABLE;
    }
    if (mask & DDS::SAMPLE_REJECTED_STATUS) {
        vMask |= V_EVENT_SAMPLE_REJECTED;
    }
    if (mask & DDS::LIVELINESS_CHANGED_STATUS) {
        vMask |= V_EVENT_LIVELINESS_CHANGED;
    }
    if (mask & DDS::REQUESTED_DEADLINE_MISSED_STATUS) {
        vMask |= V_EVENT_REQUESTED_DEADLINE_MISSED;
    }
    if (mask & DDS::REQUESTED_INCOMPATIBLE_QOS_STATUS) {
        vMask |= V_EVENT_REQUESTED_INCOMPATIBLE_QOS;
    }
    if (mask & DDS::PUBLICATION_MATCHED_STATUS) {
        vMask |= V_EVENT_PUBLICATION_MATCHED;
    }
    if (mask & DDS::SUBSCRIPTION_MATCHED_STATUS) {
        vMask |= V_EVENT_SUBSCRIPTION_MATCHED;
    }
    if (mask & DDS::ALL_DATA_DISPOSED_TOPIC_STATUS) {
        vMask |= V_EVENT_ALL_DATA_DISPOSED;
    }
    return vMask;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_inconsistentTopicInfo &from,
    DDS::InconsistentTopicStatus &to)
{
    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_livelinessLostInfo &from,
    DDS::LivelinessLostStatus &to)
{
    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_deadlineMissedInfo &from,
    DDS::RequestedDeadlineMissedStatus &to)
{
    DDS::ReturnCode_t result;
    v_object instance;

    result = DDS::RETCODE_ERROR;

    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    if (!v_handleIsNil(from.instanceHandle)) {
        if (v_handleClaim(from.instanceHandle, &instance) == V_HANDLE_OK) {
            to.last_instance_handle = u_instanceHandleNew(v_public(instance));
            if (v_handleRelease(from.instanceHandle) == V_HANDLE_OK) {
                result = DDS::RETCODE_OK;
            }
        }
    } else {
        result = DDS::RETCODE_OK;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_deadlineMissedInfo &from,
    DDS::OfferedDeadlineMissedStatus &to)
{
    DDS::ReturnCode_t result;
    v_object instance;

    result = DDS::RETCODE_ERROR;

    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    if (!v_handleIsNil(from.instanceHandle)) {
        if (v_handleClaim(from.instanceHandle, &instance) == V_HANDLE_OK) {
            to.last_instance_handle = u_instanceHandleNew(v_public(instance));
            if (v_handleRelease(from.instanceHandle) == V_HANDLE_OK) {
                result = DDS::RETCODE_OK;
            }
        }
    } else {
        result = DDS::RETCODE_OK;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_sampleRejectedInfo &from,
    DDS::SampleRejectedStatus &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    to.last_instance_handle = u_instanceHandleFromGID(from.instanceHandle);
    switch (from.lastReason) {
    case S_NOT_REJECTED:
        to.last_reason = DDS::NOT_REJECTED;
    break;
    case S_REJECTED_BY_INSTANCES_LIMIT:
        to.last_reason = DDS::REJECTED_BY_INSTANCES_LIMIT;
    break;
    case S_REJECTED_BY_SAMPLES_LIMIT:
        to.last_reason = DDS::REJECTED_BY_SAMPLES_LIMIT;
    break;
    case S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
        to.last_reason = DDS::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
    break;
    default:
        result = DDS::RETCODE_ERROR;
        break;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_livelinessChangedInfo &from,
    DDS::LivelinessChangedStatus &to)
{
    to.alive_count = from.activeCount;
    to.not_alive_count = from.inactiveCount;
    to.alive_count_change = from.activeChanged;
    to.not_alive_count_change = from.inactiveChanged;
    to.last_publication_handle = u_instanceHandleFromGID(from.instanceHandle);
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_incompatibleQosInfo &from,
    DDS::RequestedIncompatibleQosStatus &to)
{
    DDS::ReturnCode_t result;

    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    to.last_policy_id = from.lastPolicyId;
    result = DDS::OpenSplice::Utils::copySequenceOut(from.policyCount, from.totalCount, to.policies);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_incompatibleQosInfo &from,
    DDS::OfferedIncompatibleQosStatus &to)
{
    DDS::ReturnCode_t result;

    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    to.last_policy_id = from.lastPolicyId;

    result = DDS::OpenSplice::Utils::copySequenceOut(from.policyCount, from.totalCount, to.policies);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
        const v_sampleLostInfo &from,
        DDS::SampleLostStatus &to)
{

    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_topicMatchInfo &from,
    DDS::SubscriptionMatchedStatus &to)
{
    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    to.current_count = from.currentCount;
    to.current_count_change = from.currentChanged;
    to.last_publication_handle = u_instanceHandleFromGID(from.instanceHandle);
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyStatusOut(
    const v_topicMatchInfo &from,
    DDS::PublicationMatchedStatus &to)
{
    to.total_count = from.totalCount;
    to.total_count_change = from.totalChanged;
    to.current_count = from.currentCount;
    to.current_count_change = from.currentChanged;
    to.last_subscription_handle = u_instanceHandleFromGID(from.instanceHandle);
    return DDS::RETCODE_OK;
}
