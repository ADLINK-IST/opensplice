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
#include "Constants.h"
#include "MiscUtils.h"
#include "SequenceUtils.h"
#include "PolicyUtils.h"
#include "ReportUtils.h"


#define USE_OLD_ReaderDataLifecycleQosPolicy

namespace DDS {
namespace OpenSplice {

#define HISTORY_DEPTH_DEFAULT  (1)
#define RESOURCE_LIMIT_INFINITE (-1)

#define MAX_BLOCKING_TIME_DEFAULT {0, 100000000 } /* 100ms */

};
};


/*
 * Default QPolicies
 */

const DDS::OwnershipStrengthQosPolicy
    DDS::OpenSplice::Utils::OwnershipStrengthQosPolicy_default = {
        0
    };

const DDS::DurabilityQosPolicy
    DDS::OpenSplice::Utils::DurabilityQosPolicy_default = {
        DDS::VOLATILE_DURABILITY_QOS
    };

const DDS::DeadlineQosPolicy
    DDS::OpenSplice::Utils::DeadlineQosPolicy_default = {
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC }
    };

const DDS::LatencyBudgetQosPolicy
    DDS::OpenSplice::Utils::LatencyBudgetQosPolicy_default = {
        { DDS::DURATION_ZERO_SEC, DDS::DURATION_ZERO_NSEC }
    };

const DDS::LivelinessQosPolicy
    DDS::OpenSplice::Utils::LivelinessQosPolicy_default = {
        DDS::AUTOMATIC_LIVELINESS_QOS,
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC }
    };

const DDS::ReliabilityQosPolicy
    DDS::OpenSplice::Utils::ReliabilityQosPolicy_default = {
        DDS::BEST_EFFORT_RELIABILITY_QOS,
        MAX_BLOCKING_TIME_DEFAULT,
        FALSE
    };

const DDS::ReliabilityQosPolicy
    DDS::OpenSplice::Utils::ReliabilityQosPolicy_writerDefault = {
        DDS::RELIABLE_RELIABILITY_QOS,
        MAX_BLOCKING_TIME_DEFAULT,
        FALSE
    };

const DDS::DestinationOrderQosPolicy
    DDS::OpenSplice::Utils::DestinationOrderQosPolicy_default = {
        DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS
    };

const DDS::HistoryQosPolicy
    DDS::OpenSplice::Utils::HistoryQosPolicy_default = {
        DDS::KEEP_LAST_HISTORY_QOS,
        HISTORY_DEPTH_DEFAULT
    };

const DDS::ResourceLimitsQosPolicy
    DDS::OpenSplice::Utils::ResourceLimitsQosPolicy_default = {
        RESOURCE_LIMIT_INFINITE,
        RESOURCE_LIMIT_INFINITE,
        RESOURCE_LIMIT_INFINITE
    };

const DDS::TransportPriorityQosPolicy
    DDS::OpenSplice::Utils::TransportPriorityQosPolicy_default = {
       0
    };

const DDS::LifespanQosPolicy
    DDS::OpenSplice::Utils::LifespanQosPolicy_default = {
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC }
    };

const DDS::OwnershipQosPolicy
    DDS::OpenSplice::Utils::OwnershipQosPolicy_default = {
        DDS::SHARED_OWNERSHIP_QOS
    };

const DDS::PresentationQosPolicy
    DDS::OpenSplice::Utils::PresentationQosPolicy_default = {
        DDS::INSTANCE_PRESENTATION_QOS,
        FALSE,
        FALSE
    };

const DDS::EntityFactoryQosPolicy
    DDS::OpenSplice::Utils::EntityFactoryQosPolicy_default = {
        TRUE
    };

const DDS::WriterDataLifecycleQosPolicy
    DDS::OpenSplice::Utils::WriterDataLifecycleQosPolicy_default = {
        TRUE,
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC },
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC }
    };

const DDS::SchedulingQosPolicy
    DDS::OpenSplice::Utils::SchedulingQosPolicy_default = {
        { DDS::SCHEDULE_DEFAULT },
        { DDS::PRIORITY_RELATIVE },
        0
    };

const DDS::UserDataQosPolicy
    DDS::OpenSplice::Utils::UserDataQosPolicy_default = {
        DDS::octSeq()
    };

const DDS::TopicDataQosPolicy
    DDS::OpenSplice::Utils::TopicDataQosPolicy_default = {
        DDS::octSeq()
    };

const DDS::GroupDataQosPolicy
    DDS::OpenSplice::Utils::GroupDataQosPolicy_default = {
        DDS::octSeq()
    };

const DDS::PartitionQosPolicy
    DDS::OpenSplice::Utils::PartitionQosPolicy_default = {
        DDS::StringSeq()
    };

const DDS::ReaderDataLifecycleQosPolicy
    DDS::OpenSplice::Utils::ReaderDataLifecycleQosPolicy_default = {
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC },
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC },
        FALSE,
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
        TRUE,
#endif
        { DDS::MINIMUM_INVALID_SAMPLES }
    };

const DDS::TimeBasedFilterQosPolicy
    DDS::OpenSplice::Utils::TimeBasedFilterQosPolicy_default = {
        { DDS::DURATION_ZERO_SEC, DDS::DURATION_ZERO_NSEC }
    };

const DDS::SubscriptionKeyQosPolicy
    DDS::OpenSplice::Utils::SubscriptionKeyQosPolicy_default = {
        FALSE,
        DDS::StringSeq()
    };

const DDS::ReaderLifespanQosPolicy
    DDS::OpenSplice::Utils::ReaderLifespanQosPolicy_default = {
        FALSE,
        { DDS::DURATION_INFINITE_SEC, DDS::DURATION_INFINITE_NSEC }
    };

const DDS::ShareQosPolicy
    DDS::OpenSplice::Utils::ShareQosPolicy_default = {
        "",
        FALSE
    };

const DDS::ViewKeyQosPolicy
    DDS::OpenSplice::Utils::ViewKeyQosPolicy_default = {
        FALSE,
        DDS::StringSeq()
    };

const DDS::DurabilityServiceQosPolicy
    DDS::OpenSplice::Utils::DurabilityServiceQosPolicy_default = {
        { DDS::DURATION_ZERO_SEC, DDS::DURATION_ZERO_NSEC },
        DDS::KEEP_LAST_HISTORY_QOS,
        HISTORY_DEPTH_DEFAULT,
        RESOURCE_LIMIT_INFINITE,
        RESOURCE_LIMIT_INFINITE,
        RESOURCE_LIMIT_INFINITE
    };



/*
 * Policy validations
 */

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::DeadlineQosPolicy &policy)
{
    return DDS::OpenSplice::Utils::durationIsValid(policy.period);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
        const DDS::DestinationOrderQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (policy.kind == DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS ||
        policy.kind == DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            DDS::DESTINATIONORDER_QOS_POLICY_NAME,
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::DurabilityQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (policy.kind == DDS::VOLATILE_DURABILITY_QOS ||
        policy.kind == DDS::TRANSIENT_DURABILITY_QOS ||
        policy.kind == DDS::TRANSIENT_LOCAL_DURABILITY_QOS ||
        policy.kind == DDS::PERSISTENT_DURABILITY_QOS)
    {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            DDS::DURABILITY_QOS_POLICY_NAME,
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::DurabilityServiceQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (policy.history_kind != DDS::KEEP_LAST_HISTORY_QOS &&
        policy.history_kind != DDS::KEEP_ALL_HISTORY_QOS)
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.history_kind '%d' is invalid.",
            DDS::DURABILITYSERVICE_QOS_POLICY_NAME,
            policy.history_kind);
    } else {
        if (policy.history_kind == DDS::KEEP_LAST_HISTORY_QOS) {
            if (policy.history_depth <= 0) {
                result = DDS::RETCODE_BAD_PARAMETER;
            }
        }
    }
    if (( policy.max_samples < -1  ) ||
        ( policy.max_instances < -1 ) ||
        ( policy.max_samples_per_instance < -1 ) ||
        ( DDS::OpenSplice::Utils::durationIsValid(policy.service_cleanup_delay) != DDS::RETCODE_OK ))
    {
        result = DDS::RETCODE_BAD_PARAMETER;
    } else if (policy.max_samples_per_instance != DDS::LENGTH_UNLIMITED &&
               policy.history_depth > policy.max_samples_per_instance)
    {
        result = DDS::RETCODE_INCONSISTENT_POLICY;
        CPP_REPORT(result, "%s.history_depth is greater than %s.max_samples_per_instance.",
            DDS::DURABILITYSERVICE_QOS_POLICY_NAME,
            DDS::DURABILITYSERVICE_QOS_POLICY_NAME);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::EntityFactoryQosPolicy &policy)
{
    return DDS::OpenSplice::Utils::booleanIsValid(policy.autoenable_created_entities);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::GroupDataQosPolicy &policy)
{
    OS_UNUSED_ARG(policy);
    /* This policy only contains a sequence, which is always valid in c++. */
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::HistoryQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (policy.kind == DDS::KEEP_LAST_HISTORY_QOS) {
        if (policy.depth <= 0) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "%s.depth '%d' is invalid.",
                DDS::HISTORY_QOS_POLICY_NAME,
                policy.depth);
        }
    } else if (policy.kind != DDS::KEEP_ALL_HISTORY_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            DDS::HISTORY_QOS_POLICY_NAME,
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::InvalidSampleVisibilityQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (policy.kind != DDS::ALL_INVALID_SAMPLES &&
        policy.kind != DDS::MINIMUM_INVALID_SAMPLES &&
        policy.kind != DDS::NO_INVALID_SAMPLES)
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            "InvalidSampleVisibilityQosPolicy",
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::LatencyBudgetQosPolicy &policy)
{
    return DDS::OpenSplice::Utils::durationIsValid(policy.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::LifespanQosPolicy &policy)
{
    return DDS::OpenSplice::Utils::durationIsValid(policy.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::LivelinessQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (policy.kind == DDS::AUTOMATIC_LIVELINESS_QOS ||
        policy.kind == DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS ||
        policy.kind == DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        result = DDS::OpenSplice::Utils::durationIsValid(policy.lease_duration);
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            DDS::LIVELINESS_QOS_POLICY_NAME,
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::OwnershipQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (policy.kind == DDS::SHARED_OWNERSHIP_QOS ||
        policy.kind == DDS::EXCLUSIVE_OWNERSHIP_QOS)
    {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            DDS::OWNERSHIP_QOS_POLICY_NAME,
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::OwnershipStrengthQosPolicy &policy)
{
    OS_UNUSED_ARG(policy);
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::PartitionQosPolicy &policy)
{
    OS_UNUSED_ARG(policy);
    return DDS::OpenSplice::Utils::stringSeqenceIsValid(policy.name);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::PresentationQosPolicy &policy)
{
    DDS::ReturnCode_t result;

    if ((result = DDS::OpenSplice::Utils::booleanIsValid(
             policy.coherent_access)) == DDS::RETCODE_OK
     && (result = DDS::OpenSplice::Utils::booleanIsValid(
             policy.ordered_access))  == DDS::RETCODE_OK)
    {
        if (policy.access_scope != DDS::TOPIC_PRESENTATION_QOS &&
            policy.access_scope != DDS::GROUP_PRESENTATION_QOS &&
            policy.access_scope != DDS::INSTANCE_PRESENTATION_QOS)
        {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "%s.access_scope '%d' is invalid.",
                DDS::PRESENTATION_QOS_POLICY_NAME,
                policy.access_scope);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::ReaderDataLifecycleQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if ((result = DDS::OpenSplice::Utils::durationIsValid(
            policy.autopurge_nowriter_samples_delay))   == DDS::RETCODE_OK
     && (result = DDS::OpenSplice::Utils::durationIsValid(
            policy.autopurge_disposed_samples_delay))   == DDS::RETCODE_OK
     && (result = DDS::OpenSplice::Utils::policyIsValid(
            policy.invalid_sample_visibility))          == DDS::RETCODE_OK)
    {
        if (policy.invalid_sample_visibility.kind == DDS::ALL_INVALID_SAMPLES) {
            result = DDS::RETCODE_UNSUPPORTED; /* See OSPL-433 */
            CPP_REPORT(result, "%s.invalid_sample_visibility.kind ALL_INVALID_SAMPLES is unsupported.",
                DDS::READERDATALIFECYCLE_QOS_POLICY_NAME);

#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
        } else if ((result = DDS::OpenSplice::Utils::booleanIsValid(
                        policy.enable_invalid_samples)) == DDS::RETCODE_OK)
        {
            if (!policy.enable_invalid_samples) {
                OS_REPORT(OS_WARNING,
                    "DDS::OpenSplice::Utils::policyIsValid", 0,
                    "%s.%s is deprecated an will be replaced by %s.%s.",
                    DDS::READERDATALIFECYCLE_QOS_POLICY_NAME,
                    "enable_invalid_samples",
                    DDS::READERDATALIFECYCLE_QOS_POLICY_NAME,
                    "invalid_sample_visibility");
                if (policy.invalid_sample_visibility.kind
                        != DDS::MINIMUM_INVALID_SAMPLES)
                {
                    result = DDS::RETCODE_INCONSISTENT_POLICY;
                    CPP_REPORT(result, "%s invalid, %s.%s inconsistent with %s.%s.",
                        DDS::READERDATALIFECYCLE_QOS_POLICY_NAME,
                        DDS::READERDATALIFECYCLE_QOS_POLICY_NAME,
                        "enable_invalid_samples",
                        DDS::READERDATALIFECYCLE_QOS_POLICY_NAME,
                        "invalid_sample_visibility");
                }
            }
#endif
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::ReaderLifespanQosPolicy &policy)
{
    DDS::ReturnCode_t result;

    if ((result = DDS::OpenSplice::Utils::booleanIsValid(
             policy.use_lifespan)) == DDS::RETCODE_OK)
    {
        if (policy.use_lifespan) {
            result = DDS::OpenSplice::Utils::durationIsValid(policy.duration);
        }
    } else {
        CPP_REPORT(result, "%s.use_lifespan is invalid.",
            DDS::READERLIFESPAN_QOS_POLICY_NAME);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::ReliabilityQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (policy.kind == DDS::BEST_EFFORT_RELIABILITY_QOS ||
        policy.kind == DDS::RELIABLE_RELIABILITY_QOS)
    {
        if (policy.synchronous) {
            result = DDS::OpenSplice::Utils::durationIsValid(
                policy.max_blocking_time);
        } else {
            result = DDS::RETCODE_OK;
        }
    } else {
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            DDS::RELIABILITY_QOS_POLICY_NAME);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::ResourceLimitsQosPolicy &policy)
{
    DDS::ReturnCode_t result = RETCODE_OK;

    if (policy.max_samples_per_instance <= 0 &&
        policy.max_samples_per_instance != DDS::LENGTH_UNLIMITED)
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.max_samples_per_instance '%d' is invalid.",
            DDS::RESOURCELIMITS_QOS_POLICY_NAME,
            policy.max_samples_per_instance);
    } else if (policy.max_samples <= 0 &&
               policy.max_samples != DDS::LENGTH_UNLIMITED)
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.max_samples '%d' is invalid.",
            DDS::RESOURCELIMITS_QOS_POLICY_NAME,
            policy.max_samples);
    } else if (policy.max_instances <= 0 &&
               policy.max_instances != DDS::LENGTH_UNLIMITED)
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "%s.max_instances '%d' is invalid.",
            DDS::RESOURCELIMITS_QOS_POLICY_NAME,
            policy.max_instances);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::ShareQosPolicy &policy)
{
    DDS::ReturnCode_t result;

    if ((result = DDS::OpenSplice::Utils::booleanIsValid(
             policy.enable)) == DDS::RETCODE_OK)
    {
        if (policy.enable) {
            if (!policy.name) {
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "%s.name '<NULL>' is invalid.",
                    DDS::SHARE_QOS_POLICY_NAME);
            } else if (policy.name[DDS::ULong(0)] == '\0') {
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "%s.name '' is invalid.",
                    DDS::SHARE_QOS_POLICY_NAME);
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::SchedulingClassQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (policy.kind == DDS::SCHEDULE_REALTIME ||
        policy.kind == DDS::SCHEDULE_TIMESHARING ||
        policy.kind == DDS::SCHEDULE_DEFAULT)
    {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            "SchedulingClass",
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::SchedulingPriorityQosPolicy &policy)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (policy.kind == DDS::PRIORITY_ABSOLUTE ||
        policy.kind == DDS::PRIORITY_RELATIVE)
    {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "%s.kind '%d' is invalid.",
            "SchedulingPriority",
            policy.kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::SchedulingQosPolicy &policy)
{
    DDS::ReturnCode_t result;

    if ((result = policyIsValid(policy.scheduling_class)) == DDS::RETCODE_OK) {
        result = policyIsValid(policy.scheduling_priority_kind);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::SubscriptionKeyQosPolicy &policy)
{
    DDS::ReturnCode_t result;

    if ((result = DDS::OpenSplice::Utils::booleanIsValid(
             policy.use_key_list)) == DDS::RETCODE_OK)
    {
        if (policy.use_key_list) {
            result = DDS::OpenSplice::Utils::stringSeqenceIsValid(
                policy.key_list);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::TimeBasedFilterQosPolicy &policy)
{
    return DDS::OpenSplice::Utils::durationIsValid(policy.minimum_separation);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::TopicDataQosPolicy &policy)
{
    OS_UNUSED_ARG(policy);
    /* This policy only contains a sequence, which is always valid in C++ */
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::TransportPriorityQosPolicy &policy)
{
    OS_UNUSED_ARG(policy);
    /* Any value is valid. */
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::UserDataQosPolicy &policy)
{
    OS_UNUSED_ARG(policy);
    /* This policy only contains a sequence, which is always valid in C++ */
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::ViewKeyQosPolicy &policy)
{
    DDS::ReturnCode_t result;

    if ((result = DDS::OpenSplice::Utils::booleanIsValid(
             policy.use_key_list)) == DDS::RETCODE_OK)
    {
        if (policy.use_key_list) {
            result = DDS::OpenSplice::Utils::stringSeqenceIsValid(
                policy.key_list);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policyIsValid(
    const DDS::WriterDataLifecycleQosPolicy &policy)
{
    return DDS::OpenSplice::Utils::booleanIsValid(policy.autodispose_unregistered_instances);
}




/*
 * Policy consistency checks
 */
DDS::ReturnCode_t
DDS::OpenSplice::Utils::policiesAreConsistent(
    const HistoryQosPolicy &history,
    const ResourceLimitsQosPolicy &resource_limits)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (history.kind == DDS::KEEP_LAST_HISTORY_QOS) {
        if (resource_limits.max_samples_per_instance != DDS::LENGTH_UNLIMITED) {
            if (history.depth > resource_limits.max_samples_per_instance) {
                result = DDS::RETCODE_INCONSISTENT_POLICY;
                CPP_REPORT(result, "%s.depth is greater than %s.max_samples_per_instance.",
                    DDS::HISTORY_QOS_POLICY_NAME,
                    DDS::RESOURCELIMITS_QOS_POLICY_NAME);
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::policiesAreConsistent(
    const DDS::DeadlineQosPolicy &deadline,
    const DDS::TimeBasedFilterQosPolicy &time_based_filter)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if ((deadline.period.sec < time_based_filter.minimum_separation.sec)
             ||
        (deadline.period.sec == time_based_filter.minimum_separation.sec &&
         deadline.period.nanosec < time_based_filter.minimum_separation.nanosec))
    {
        result = DDS::RETCODE_INCONSISTENT_POLICY;
        CPP_REPORT(result, "%s.period is less than %s.minumum_separation.",
            DDS::DEADLINE_QOS_POLICY_NAME,
            DDS::TIMEBASEDFILTER_QOS_POLICY_NAME);
    }

    return result;
}




/*
 * Policy comparison
 */
DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::DeadlineQosPolicy &a,
    const DDS::DeadlineQosPolicy &b)
{
    return durationIsEqual (a.period, b.period);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::DestinationOrderQosPolicy &a,
    const DDS::DestinationOrderQosPolicy &b)
{
    return a.kind == b.kind ? TRUE : FALSE;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::DurabilityQosPolicy &a,
    const DDS::DurabilityQosPolicy &b)
{
    return a.kind == b.kind ? TRUE : FALSE;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::DurabilityServiceQosPolicy &a,
    const DDS::DurabilityServiceQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.history_depth != b.history_depth ||
        a.history_kind != b.history_kind ||
        a.max_instances != b.max_instances ||
        a.max_samples != b.max_samples ||
        a.max_samples_per_instance != b.max_samples_per_instance)
    {
        equal = durationIsEqual (a.service_cleanup_delay, b.service_cleanup_delay);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::EntityFactoryQosPolicy &a,
    const DDS::EntityFactoryQosPolicy &b)
{
    return a.autoenable_created_entities == b.autoenable_created_entities ?
        TRUE : FALSE;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::GroupDataQosPolicy &a,
    const DDS::GroupDataQosPolicy &b)
{
    return octSeqIsEqual (a.value, b.value);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::HistoryQosPolicy &a,
    const DDS::HistoryQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.depth != b.depth || a.kind != b.kind) {
        equal = FALSE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::LatencyBudgetQosPolicy &a,
    const DDS::LatencyBudgetQosPolicy &b)
{
    return durationIsEqual (a.duration, b.duration);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::LifespanQosPolicy &a,
    const DDS::LifespanQosPolicy &b)
{
    return durationIsEqual (a.duration, b.duration);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::LivelinessQosPolicy &a,
    const DDS::LivelinessQosPolicy &b)
{
    DDS::Boolean equal;

    if (a.kind != b.kind) {
        equal = FALSE;
    } else {
        equal = durationIsEqual (a.lease_duration, b.lease_duration);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::OwnershipQosPolicy &a,
    const DDS::OwnershipQosPolicy &b)
{
    return a.kind == b.kind ? TRUE : FALSE;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::OwnershipStrengthQosPolicy &a,
    const DDS::OwnershipStrengthQosPolicy &b)
{
    return a.value == b.value ? TRUE : FALSE;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::PartitionQosPolicy &a,
    const DDS::PartitionQosPolicy &b)
{
    return stringSeqIsEqual (a.name, b.name);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::PresentationQosPolicy &a,
    const DDS::PresentationQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.access_scope != b.access_scope ||
        a.coherent_access != b.coherent_access ||
        a.ordered_access != b.ordered_access)
    {
        equal = FALSE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::ReaderDataLifecycleQosPolicy &a,
    const DDS::ReaderDataLifecycleQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.enable_invalid_samples != b.enable_invalid_samples ||
        a.invalid_sample_visibility.kind != b.invalid_sample_visibility.kind ||
        durationIsEqual (a.autopurge_disposed_samples_delay, b.autopurge_disposed_samples_delay) == FALSE ||
        durationIsEqual (a.autopurge_nowriter_samples_delay, b.autopurge_nowriter_samples_delay) == FALSE)
    {
        equal = FALSE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::ReaderLifespanQosPolicy &a,
    const DDS::ReaderLifespanQosPolicy &b)
{
    DDS::Boolean equal = FALSE;

    if (a.use_lifespan == b.use_lifespan) {
        equal = durationIsEqual (a.duration, b.duration);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::ReliabilityQosPolicy &a,
    const DDS::ReliabilityQosPolicy &b)
{
    DDS::Boolean equal;

    if (a.kind != b.kind || a.synchronous != b.synchronous) {
        equal = FALSE;
    } else {
        equal = durationIsEqual (a.max_blocking_time, b.max_blocking_time);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::ResourceLimitsQosPolicy &a,
    const DDS::ResourceLimitsQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.max_instances != b.max_instances ||
        a.max_samples != b.max_samples ||
        a.max_samples_per_instance != b.max_samples_per_instance)
    {
        equal = FALSE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::SchedulingQosPolicy &a,
    const DDS::SchedulingQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.scheduling_class.kind != b.scheduling_class.kind ||
        a.scheduling_priority != b.scheduling_priority ||
        a.scheduling_priority_kind.kind != b.scheduling_priority_kind.kind)
    {
        equal = FALSE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::ShareQosPolicy &a,
    const DDS::ShareQosPolicy &b)
{
    DDS::Boolean equal = FALSE;

    if (a.enable == b.enable) {
        if (a.enable && a.name.in() != NULL && b.name.in() != NULL && strcmp (a.name, b.name) == 0) {
            equal = TRUE;
        } else if (!a.enable) {
            equal = TRUE;
        }
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::SubscriptionKeyQosPolicy &a,
    const DDS::SubscriptionKeyQosPolicy &b)
{
    DDS::Boolean equal = FALSE;

    if (a.use_key_list == b.use_key_list) {
        equal = stringSeqIsEqual (a.key_list, b.key_list);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::TimeBasedFilterQosPolicy &a,
    const DDS::TimeBasedFilterQosPolicy &b)
{
    return durationIsEqual (a.minimum_separation, b.minimum_separation);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::TopicDataQosPolicy &a,
    const DDS::TopicDataQosPolicy &b)
{
    return octSeqIsEqual (a.value, b.value);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::TransportPriorityQosPolicy &a,
    const DDS::TransportPriorityQosPolicy &b)
{
    return a.value == b.value ? TRUE : FALSE;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::UserDataQosPolicy &a,
    const DDS::UserDataQosPolicy &b)
{
    return octSeqIsEqual (a.value, b.value);
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::ViewKeyQosPolicy &a,
    const DDS::ViewKeyQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.use_key_list != b.use_key_list) {
        equal = stringSeqIsEqual (a.key_list, b.key_list);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::policyIsEqual (
    const DDS::WriterDataLifecycleQosPolicy &a,
    const DDS::WriterDataLifecycleQosPolicy &b)
{
    DDS::Boolean equal = TRUE;

    if (a.autodispose_unregistered_instances != b.autodispose_unregistered_instances ||
        durationIsEqual (a.autopurge_suspended_samples_delay, b.autopurge_suspended_samples_delay) == FALSE ||
        durationIsEqual (a.autounregister_instance_delay, b.autounregister_instance_delay) == FALSE)
    {
        equal = FALSE;
    }

    return equal;
}




/*
 * Policy conversions
 */

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DeadlineQosPolicy &from,
        v_deadlinePolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationIn(from.period, to.period);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DestinationOrderQosPolicy &from,
        v_orderbyPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
        to.kind = V_ORDERBY_RECEPTIONTIME;
        break;
    case DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
        to.kind = V_ORDERBY_SOURCETIME;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DurabilityQosPolicy &from,
        v_durabilityPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::VOLATILE_DURABILITY_QOS:
        to.kind = V_DURABILITY_VOLATILE;
        break;
    case DDS::TRANSIENT_DURABILITY_QOS:
        to.kind = V_DURABILITY_TRANSIENT;
        break;
    case DDS::TRANSIENT_LOCAL_DURABILITY_QOS:
        to.kind = V_DURABILITY_TRANSIENT_LOCAL;
        break;
    case DDS::PERSISTENT_DURABILITY_QOS:
        to.kind = V_DURABILITY_PERSISTENT;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DurabilityServiceQosPolicy &from,
        v_durabilityServicePolicy &to)
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::Utils::copyDurationIn(from.service_cleanup_delay, to.service_cleanup_delay);
    if (result == DDS::RETCODE_OK) {
        switch (from.history_kind) {
        case DDS::KEEP_LAST_HISTORY_QOS:
            to.history_kind = V_HISTORY_KEEPLAST;
            break;
        case DDS::KEEP_ALL_HISTORY_QOS:
            to.history_kind = V_HISTORY_KEEPALL;
            break;
        default:
            result = DDS::RETCODE_BAD_PARAMETER;
            break;
        }
        to.history_depth            = from.history_depth;
        to.max_samples              = from.max_samples;
        to.max_instances            = from.max_instances;
        to.max_samples_per_instance = from.max_samples_per_instance;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::EntityFactoryQosPolicy &from,
        v_entityFactoryPolicy &to)
{
    to.autoenable_created_entities = from.autoenable_created_entities;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::HistoryQosPolicy &from,
        v_historyPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::KEEP_LAST_HISTORY_QOS:
        to.kind = V_HISTORY_KEEPLAST;
        break;
    case DDS::KEEP_ALL_HISTORY_QOS:
        to.kind = V_HISTORY_KEEPALL;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    to.depth = from.depth;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::LatencyBudgetQosPolicy &from,
        v_latencyPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationIn(from.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::LifespanQosPolicy &from,
        v_lifespanPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationIn(from.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::LivelinessQosPolicy &from,
        v_livelinessPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::AUTOMATIC_LIVELINESS_QOS:
        to.kind = V_LIVELINESS_AUTOMATIC;
        break;
    case DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
        to.kind = V_LIVELINESS_PARTICIPANT;
        break;
    case DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS:
        to.kind = V_LIVELINESS_TOPIC;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.lease_duration, to.lease_duration);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::OwnershipQosPolicy &from,
        v_ownershipPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::SHARED_OWNERSHIP_QOS:
        to.kind = V_OWNERSHIP_SHARED;
        break;
    case DDS::EXCLUSIVE_OWNERSHIP_QOS:
        to.kind = V_OWNERSHIP_EXCLUSIVE;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::OwnershipStrengthQosPolicy &from,
        v_strengthPolicy &to)
{
    to.value = from.value;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::PresentationQosPolicy &from,
        v_presentationPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.access_scope) {
    case DDS::INSTANCE_PRESENTATION_QOS:
        to.access_scope = V_PRESENTATION_INSTANCE;
        break;
    case DDS::TOPIC_PRESENTATION_QOS:
        to.access_scope = V_PRESENTATION_TOPIC;
        break;
    case DDS::GROUP_PRESENTATION_QOS:
        to.access_scope = V_PRESENTATION_GROUP;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    to.coherent_access = from.coherent_access;
    to.ordered_access  = from.ordered_access;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ReaderDataLifecycleQosPolicy &from,
        v_readerLifecyclePolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    if (from.enable_invalid_samples == FALSE) {
        to.enable_invalid_samples = from.enable_invalid_samples;
    } else {
#endif
    switch (from.invalid_sample_visibility.kind) {
    case DDS::NO_INVALID_SAMPLES:
        to.enable_invalid_samples = FALSE;
        break;
    case DDS::MINIMUM_INVALID_SAMPLES:
        to.enable_invalid_samples = TRUE;
        break;
    case DDS::ALL_INVALID_SAMPLES:
        result = DDS::RETCODE_UNSUPPORTED;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    }
#endif
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.autopurge_nowriter_samples_delay,
                                                        to.autopurge_nowriter_samples_delay);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.autopurge_disposed_samples_delay,
                                                        to.autopurge_disposed_samples_delay);
    }
    to.autopurge_dispose_all = from.autopurge_dispose_all;
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ReaderLifespanQosPolicy &from,
        v_readerLifespanPolicy &to)
{
    to.used = from.use_lifespan;
    return DDS::OpenSplice::Utils::copyDurationIn(from.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ReliabilityQosPolicy &from,
        v_reliabilityPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::BEST_EFFORT_RELIABILITY_QOS:
        to.kind = V_RELIABILITY_BESTEFFORT;
        break;
    case DDS::RELIABLE_RELIABILITY_QOS:
        to.kind = V_RELIABILITY_RELIABLE;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.max_blocking_time, to.max_blocking_time);
        to.synchronous = from.synchronous;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ResourceLimitsQosPolicy &from,
        v_resourcePolicy &to)
{
    to.max_samples = from.max_samples;
    to.max_instances = from.max_instances;
    to.max_samples_per_instance = from.max_samples_per_instance;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ShareQosPolicy &from,
        v_sharePolicy &to)
{
    if (from.enable) {
        to.enable = TRUE;
    } else {
        to.enable = FALSE;
    }
    if (from.name != NULL) {
        to.name = os_strdup(from.name);
    } else {
        to.name = (char*)NULL;
    }
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::SubscriptionKeyQosPolicy &from,
        v_userKeyPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    to.enable = from.use_key_list;
    if ( from.use_key_list ) {
        result = DDS::OpenSplice::Utils::copySequenceIn(from.key_list, to.expression, (c_string)",");
    } else {
        to.expression = NULL;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::TimeBasedFilterQosPolicy &from,
        v_pacingPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationIn(from.minimum_separation, to.minSeperation);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::TransportPriorityQosPolicy &from,
        v_transportPolicy &to)
{
    to.value = from.value;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::UserDataQosPolicy &from,
        v_userDataPolicy &to)
{
    return DDS::OpenSplice::Utils::copySequenceIn(from.value, to.value, to.size);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ViewKeyQosPolicy &from,
        v_userKeyPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    to.enable = from.use_key_list;
    if ( from.use_key_list ) {
        result = DDS::OpenSplice::Utils::copySequenceIn(from.key_list, to.expression, (c_string)",");
    } else {
        to.expression = NULL;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::WriterDataLifecycleQosPolicy &from,
        v_writerLifecyclePolicy &to)
{
    DDS::ReturnCode_t result;

    to.autodispose_unregistered_instances = from.autodispose_unregistered_instances;
    result = DDS::OpenSplice::Utils::copyDurationIn(from.autopurge_suspended_samples_delay,
                                                    to.autopurge_suspended_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.autounregister_instance_delay,
                                                        to.autounregister_instance_delay);
    }

    return result;
}







DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_deadlinePolicy &from,
        DDS::DeadlineQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationOut(from.period, to.period);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_orderbyPolicy &from,
        DDS::DestinationOrderQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case V_ORDERBY_RECEPTIONTIME:
        to.kind = DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
    case V_ORDERBY_SOURCETIME:
        to.kind = DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_durabilityPolicy &from,
        DDS::DurabilityQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case V_DURABILITY_VOLATILE:
        to.kind = DDS::VOLATILE_DURABILITY_QOS;
        break;
    case V_DURABILITY_TRANSIENT:
        to.kind = DDS::TRANSIENT_DURABILITY_QOS;
        break;
    case V_DURABILITY_TRANSIENT_LOCAL:
        to.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
        break;
    case V_DURABILITY_PERSISTENT:
        to.kind = DDS::PERSISTENT_DURABILITY_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_durabilityServicePolicy &from,
        DDS::DurabilityServiceQosPolicy &to)
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::Utils::copyDurationOut(from.service_cleanup_delay, to.service_cleanup_delay);
    if (result == DDS::RETCODE_OK) {
        switch (from.history_kind) {
        case V_HISTORY_KEEPLAST:
            to.history_kind = DDS::KEEP_LAST_HISTORY_QOS;
            break;
        case V_HISTORY_KEEPALL:
            to.history_kind = DDS::KEEP_ALL_HISTORY_QOS;
            break;
        default:
            result = DDS::RETCODE_BAD_PARAMETER;
            break;
        }
        to.history_depth            = from.history_depth;
        to.max_samples              = from.max_samples;
        to.max_instances            = from.max_instances;
        to.max_samples_per_instance = from.max_samples_per_instance;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_entityFactoryPolicy &from,
        DDS::EntityFactoryQosPolicy &to)
{
    to.autoenable_created_entities = from.autoenable_created_entities;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_historyPolicy &from,
        DDS::HistoryQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case V_HISTORY_KEEPLAST:
        to.kind = DDS::KEEP_LAST_HISTORY_QOS;
        break;
    case V_HISTORY_KEEPALL:
        to.kind = DDS::KEEP_ALL_HISTORY_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    to.depth = from.depth;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_latencyPolicy &from,
        DDS::LatencyBudgetQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationOut(from.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_lifespanPolicy &from,
        DDS::LifespanQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationOut(from.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_livelinessPolicy &from,
        DDS::LivelinessQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case V_LIVELINESS_AUTOMATIC:
        to.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
        break;
    case V_LIVELINESS_PARTICIPANT:
        to.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        break;
    case V_LIVELINESS_TOPIC:
        to.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.lease_duration, to.lease_duration);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_ownershipPolicy &from,
        DDS::OwnershipQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case V_OWNERSHIP_SHARED:
        to.kind = DDS::SHARED_OWNERSHIP_QOS;
        break;
    case V_OWNERSHIP_EXCLUSIVE:
        to.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_strengthPolicy &from,
        DDS::OwnershipStrengthQosPolicy &to)
{
    to.value = from.value;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_presentationPolicy &from,
        DDS::PresentationQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.access_scope) {
    case V_PRESENTATION_INSTANCE:
        to.access_scope = DDS::INSTANCE_PRESENTATION_QOS;
        break;
    case V_PRESENTATION_TOPIC:
        to.access_scope = DDS::TOPIC_PRESENTATION_QOS;
        break;
    case V_PRESENTATION_GROUP:
        to.access_scope = DDS::GROUP_PRESENTATION_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    to.coherent_access = from.coherent_access;
    to.ordered_access  = from.ordered_access;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_readerLifecyclePolicy &from,
        DDS::ReaderDataLifecycleQosPolicy &to)
{
    DDS::ReturnCode_t result;

    to.autopurge_dispose_all = from.autopurge_dispose_all;
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    to.enable_invalid_samples = from.enable_invalid_samples;
#endif
    if (from.enable_invalid_samples == FALSE) {
        to.invalid_sample_visibility.kind = DDS::NO_INVALID_SAMPLES;
    } else {
        /* Use MINIMUM_INVALID_SAMPLES (default) iso ALL_INVALID_SAMPLES. */
        to.invalid_sample_visibility.kind = DDS::MINIMUM_INVALID_SAMPLES;
    }
    result = DDS::OpenSplice::Utils::copyDurationOut(from.autopurge_nowriter_samples_delay,
                                                     to.autopurge_nowriter_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.autopurge_disposed_samples_delay,
                                                         to.autopurge_disposed_samples_delay);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_readerLifespanPolicy &from,
        DDS::ReaderLifespanQosPolicy &to)
{
    to.use_lifespan = from.used;
    return DDS::OpenSplice::Utils::copyDurationOut(from.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_reliabilityPolicy &from,
        DDS::ReliabilityQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case V_RELIABILITY_BESTEFFORT:
        to.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case V_RELIABILITY_RELIABLE:
        to.kind = DDS::RELIABLE_RELIABILITY_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.max_blocking_time, to.max_blocking_time);
        to.synchronous = from.synchronous;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_resourcePolicy &from,
        DDS::ResourceLimitsQosPolicy &to)
{
    to.max_samples = from.max_samples;
    to.max_instances = from.max_instances;
    to.max_samples_per_instance = from.max_samples_per_instance;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_sharePolicy &from,
        DDS::ShareQosPolicy &to)
{
    if (from.enable) {
        to.enable = TRUE;
    } else {
        to.enable = FALSE;
    }
    if (from.name != NULL) {
        to.name = DDS::string_dup(from.name);
    } else {
        to.name = (char*)NULL;
    }
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_userKeyPolicy &from,
        DDS::SubscriptionKeyQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    to.use_key_list = from.enable;
    result = DDS::OpenSplice::Utils::copySequenceOut(from.expression, (c_string)",", to.key_list);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_pacingPolicy &from,
        DDS::TimeBasedFilterQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copyDurationOut(from.minSeperation, to.minimum_separation);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_transportPolicy &from,
        DDS::TransportPriorityQosPolicy &to)
{
    to.value = from.value;
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_userDataPolicy &from,
        DDS::UserDataQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copySequenceOut(from.value, from.size, to.value);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_userKeyPolicy &from,
        DDS::ViewKeyQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    to.use_key_list = from.enable;
    result = DDS::OpenSplice::Utils::copySequenceOut(from.expression, (c_string)",", to.key_list);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_writerLifecyclePolicy &from,
        DDS::WriterDataLifecycleQosPolicy &to)
{
    DDS::ReturnCode_t result;

    to.autodispose_unregistered_instances = from.autodispose_unregistered_instances;
    result = DDS::OpenSplice::Utils::copyDurationOut(from.autopurge_suspended_samples_delay,
                                                     to.autopurge_suspended_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.autounregister_instance_delay,
                                                         to.autounregister_instance_delay);
    }

    return result;
}




DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DeadlineQosPolicy &from,
        v_deadlinePolicyI &to)
{
    return copyDurationIn (from.period, to.v.period);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DestinationOrderQosPolicy &from,
        v_orderbyPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::DurabilityQosPolicy &from,
        v_durabilityPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
    const DDS::DurabilityServiceQosPolicy &from,
    v_durabilityServicePolicyI &to)
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::Utils::copyDurationIn(from.service_cleanup_delay, to.v.service_cleanup_delay);
    if (result == DDS::RETCODE_OK) {
        switch (from.history_kind) {
        case DDS::KEEP_LAST_HISTORY_QOS:
            to.v.history_kind = V_HISTORY_KEEPLAST;
            break;
        case DDS::KEEP_ALL_HISTORY_QOS:
            to.v.history_kind = V_HISTORY_KEEPALL;
            break;
        default:
            result = DDS::RETCODE_BAD_PARAMETER;
            break;
        }
        to.v.history_depth            = from.history_depth;
        to.v.max_samples              = from.max_samples;
        to.v.max_instances            = from.max_instances;
        to.v.max_samples_per_instance = from.max_samples_per_instance;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::EntityFactoryQosPolicy &from,
        v_entityFactoryPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::GroupDataQosPolicy &from,
        v_groupDataPolicyI &to)
{
    return DDS::OpenSplice::Utils::copySequenceIn(from.value, to.v.value, to.v.size);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::HistoryQosPolicy &from,
        v_historyPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::LatencyBudgetQosPolicy &from,
        v_latencyPolicyI &to)
{
    return copyDurationIn (from.duration, to.v.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::LifespanQosPolicy &from,
        v_lifespanPolicyI &to)
{
    return copyDurationIn (from.duration, to.v.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::LivelinessQosPolicy &from,
        v_livelinessPolicyI &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::AUTOMATIC_LIVELINESS_QOS:
        to.v.kind = V_LIVELINESS_AUTOMATIC;
        break;
    case DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
        to.v.kind = V_LIVELINESS_PARTICIPANT;
        break;
    case DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS:
        to.v.kind = V_LIVELINESS_TOPIC;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = copyDurationIn (from.lease_duration, to.v.lease_duration);
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::OwnershipQosPolicy &from,
        v_ownershipPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::OwnershipStrengthQosPolicy &from,
        v_strengthPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::PartitionQosPolicy &from,
        v_partitionPolicyI &to)
{
    return DDS::OpenSplice::Utils::copySequenceIn(from.name, to.v, (c_string)",");
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::PresentationQosPolicy &from,
        v_presentationPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ReaderDataLifecycleQosPolicy &from,
        v_readerLifecyclePolicyI &to)
{
    DDS::ReturnCode_t result;

    result = copyDurationIn (from.autopurge_nowriter_samples_delay, to.v.autopurge_nowriter_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = copyDurationIn (from.autopurge_disposed_samples_delay, to.v.autopurge_disposed_samples_delay);
    }
    if (result == DDS::RETCODE_OK) {
        to.v.autopurge_dispose_all = from.autopurge_dispose_all;
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
        if (from.enable_invalid_samples == FALSE) {
            to.v.enable_invalid_samples = from.enable_invalid_samples;
        } else {
#endif
        switch(from.invalid_sample_visibility.kind) {
        case DDS::NO_INVALID_SAMPLES:
            to.v.enable_invalid_samples = FALSE;
            break;
        case DDS::MINIMUM_INVALID_SAMPLES:
            to.v.enable_invalid_samples = TRUE;
            break;
        case DDS::ALL_INVALID_SAMPLES:
            result = DDS::RETCODE_UNSUPPORTED;
            break;
        default:
            result = DDS::RETCODE_BAD_PARAMETER;
            break;
        }
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
        }
#endif
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ReaderLifespanQosPolicy &from,
        v_readerLifespanPolicyI &to)
{
    to.v.used = from.use_lifespan;
    return copyDurationIn (from.duration, to.v.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ReliabilityQosPolicy &from,
        v_reliabilityPolicyI &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.kind) {
    case DDS::BEST_EFFORT_RELIABILITY_QOS:
        to.v.kind = V_RELIABILITY_BESTEFFORT;
        break;
    case DDS::RELIABLE_RELIABILITY_QOS:
        to.v.kind = V_RELIABILITY_RELIABLE;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.max_blocking_time, to.v.max_blocking_time);
        to.v.synchronous = from.synchronous;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ResourceLimitsQosPolicy &from,
        v_resourcePolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::SchedulingQosPolicy &from,
        v_schedulePolicyI &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.scheduling_class.kind) {
    case DDS::SCHEDULE_DEFAULT:
        to.v.kind = V_SCHED_DEFAULT;
        break;
    case DDS::SCHEDULE_TIMESHARING:
        to.v.kind = V_SCHED_TIMESHARING;
        break;
    case DDS::SCHEDULE_REALTIME:
        to.v.kind = V_SCHED_REALTIME;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    switch (from.scheduling_priority_kind.kind) {
    case DDS::PRIORITY_RELATIVE:
        to.v.priorityKind = V_SCHED_PRIO_RELATIVE;
        break;
    case DDS::PRIORITY_ABSOLUTE:
        to.v.priorityKind = V_SCHED_PRIO_ABSOLUTE;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    to.v.priority = from.scheduling_priority;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ShareQosPolicy &from,
        v_sharePolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::SubscriptionKeyQosPolicy &from,
        v_userKeyPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::TimeBasedFilterQosPolicy &from,
        v_pacingPolicyI &to)
{
    return copyDurationIn (from.minimum_separation, to.v.minSeperation);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::TopicDataQosPolicy &from,
        v_topicDataPolicyI &to)
{
    return DDS::OpenSplice::Utils::copySequenceIn(from.value, to.v.value, to.v.size);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::TransportPriorityQosPolicy &from,
        v_transportPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::UserDataQosPolicy &from,
        v_userDataPolicyI &to)
{
    return DDS::OpenSplice::Utils::copySequenceIn(from.value, to.v.value, to.v.size);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::ViewKeyQosPolicy &from,
        v_userKeyPolicyI &to)
{
    return copyPolicyIn (from, to.v);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyIn(
        const DDS::WriterDataLifecycleQosPolicy &from,
        v_writerLifecyclePolicyI &to)
{
    DDS::ReturnCode_t result;

    to.v.autodispose_unregistered_instances = from.autodispose_unregistered_instances;
    result = DDS::OpenSplice::Utils::copyDurationIn(from.autopurge_suspended_samples_delay,
                                                    to.v.autopurge_suspended_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(from.autounregister_instance_delay,
                                                        to.v.autounregister_instance_delay);
    }
    return result;
}







DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_deadlinePolicyI &from,
        DDS::DeadlineQosPolicy &to)
{
    return copyDurationOut (from.v.period, to.period);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_orderbyPolicyI &from,
        DDS::DestinationOrderQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_durabilityPolicyI &from,
        DDS::DurabilityQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_durabilityServicePolicyI &from,
        DDS::DurabilityServiceQosPolicy &to)
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::Utils::copyDurationOut(from.v.service_cleanup_delay, to.service_cleanup_delay);
    if (result == DDS::RETCODE_OK) {
        switch (from.v.history_kind) {
        case V_HISTORY_KEEPLAST:
            to.history_kind = DDS::KEEP_LAST_HISTORY_QOS;
            break;
        case V_HISTORY_KEEPALL:
            to.history_kind = DDS::KEEP_ALL_HISTORY_QOS;
            break;
        default:
            result = DDS::RETCODE_BAD_PARAMETER;
            break;
        }
        to.history_depth            = from.v.history_depth;
        to.max_samples              = from.v.max_samples;
        to.max_instances            = from.v.max_instances;
        to.max_samples_per_instance = from.v.max_samples_per_instance;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_entityFactoryPolicyI &from,
        DDS::EntityFactoryQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_groupDataPolicyI &from,
        DDS::GroupDataQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copySequenceOut(from.v.value, from.v.size, to.value);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_historyPolicyI &from,
        DDS::HistoryQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_latencyPolicyI &from,
        DDS::LatencyBudgetQosPolicy &to)
{
    return copyDurationOut (from.v.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_lifespanPolicyI &from,
        DDS::LifespanQosPolicy &to)
{
    return copyDurationOut (from.v.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_livelinessPolicyI &from,
        DDS::LivelinessQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.v.kind) {
    case V_LIVELINESS_AUTOMATIC:
        to.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
        break;
    case V_LIVELINESS_PARTICIPANT:
        to.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        break;
    case V_LIVELINESS_TOPIC:
        to.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = copyDurationOut (from.v.lease_duration, to.lease_duration);
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_ownershipPolicyI &from,
        DDS::OwnershipQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_strengthPolicyI &from,
        DDS::OwnershipStrengthQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_partitionPolicyI &from,
        DDS::PartitionQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copySequenceOut(from.v, (c_string)",", to.name);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_presentationPolicyI &from,
        DDS::PresentationQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
    const v_readerLifecyclePolicyI &from,
    DDS::ReaderDataLifecycleQosPolicy &to)
{
    DDS::ReturnCode_t result;

    to.autopurge_dispose_all = from.v.autopurge_dispose_all;
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    to.enable_invalid_samples = from.v.enable_invalid_samples;
#endif
    if (from.v.enable_invalid_samples == FALSE) {
        to.invalid_sample_visibility.kind = DDS::NO_INVALID_SAMPLES;
    } else {
        /* Use MINIMUM_INVALID_SAMPLES (default) iso ALL_INVALID_SAMPLES. */
        to.invalid_sample_visibility.kind = DDS::MINIMUM_INVALID_SAMPLES;
    }
    result = DDS::OpenSplice::Utils::copyDurationOut(from.v.autopurge_nowriter_samples_delay,
                                                     to.autopurge_nowriter_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.v.autopurge_disposed_samples_delay,
                                                         to.autopurge_disposed_samples_delay);
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_readerLifespanPolicyI &from,
        DDS::ReaderLifespanQosPolicy &to)
{
    to.use_lifespan = from.v.used;
    return copyDurationOut (from.v.duration, to.duration);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_reliabilityPolicyI &from,
        DDS::ReliabilityQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.v.kind) {
    case V_RELIABILITY_BESTEFFORT:
        to.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case V_RELIABILITY_RELIABLE:
        to.kind = DDS::RELIABLE_RELIABILITY_QOS;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.v.max_blocking_time, to.max_blocking_time);
        to.synchronous = from.v.synchronous;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_resourcePolicyI &from,
        DDS::ResourceLimitsQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_schedulePolicyI &from,
        DDS::SchedulingQosPolicy &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    switch (from.v.kind) {
    case V_SCHED_DEFAULT:
        to.scheduling_class.kind = DDS::SCHEDULE_DEFAULT;
        break;
    case V_SCHED_TIMESHARING:
        to.scheduling_class.kind = DDS::SCHEDULE_TIMESHARING;
        break;
    case V_SCHED_REALTIME:
        to.scheduling_class.kind = DDS::SCHEDULE_REALTIME;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    switch (from.v.priorityKind) {
    case V_SCHED_PRIO_RELATIVE:
        to.scheduling_priority_kind.kind = DDS::PRIORITY_RELATIVE;
        break;
    case V_SCHED_PRIO_ABSOLUTE:
        to.scheduling_priority_kind.kind = DDS::PRIORITY_ABSOLUTE;
        break;
    default:
        result = DDS::RETCODE_BAD_PARAMETER;
        break;
    }
    to.scheduling_priority = from.v.priority;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_sharePolicyI &from,
        DDS::ShareQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_userKeyPolicyI &from,
        DDS::SubscriptionKeyQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_pacingPolicyI &from,
        DDS::TimeBasedFilterQosPolicy &to)
{
    return copyDurationOut (from.v.minSeperation, to.minimum_separation);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_topicDataPolicyI &from,
        DDS::TopicDataQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copySequenceOut(from.v.value, from.v.size, to.value);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_transportPolicyI &from,
        DDS::TransportPriorityQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_userDataPolicyI &from,
        DDS::UserDataQosPolicy &to)
{
    return DDS::OpenSplice::Utils::copySequenceOut(from.v.value, from.v.size, to.value);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
        const v_userKeyPolicyI &from,
        DDS::ViewKeyQosPolicy &to)
{
    return copyPolicyOut (from.v, to);
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyPolicyOut(
    const v_writerLifecyclePolicyI &from,
    DDS::WriterDataLifecycleQosPolicy &to)
{
    DDS::ReturnCode_t result;

    to.autodispose_unregistered_instances = from.v.autodispose_unregistered_instances;
    result = DDS::OpenSplice::Utils::copyDurationOut(from.v.autopurge_suspended_samples_delay,
                                                     to.autopurge_suspended_samples_delay);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationOut(from.v.autounregister_instance_delay,
                                                         to.autounregister_instance_delay);
    }
    return result;
}
