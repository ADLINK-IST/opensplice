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


/**
 * @file
 */

#include <org/opensplice/core/StatusConverter.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

//--------------------------------------------------------------------------------
//  DataReader Status
//--------------------------------------------------------------------------------
void convertStatus(const DDS::LivelinessChangedStatus& lcs,
                   dds::core::status::LivelinessChangedStatus& liveliness_changed_status)
{
    liveliness_changed_status->alive_count(lcs.alive_count);
    liveliness_changed_status->not_alive_count(lcs.not_alive_count);
    liveliness_changed_status->alive_count_change(lcs.alive_count_change);
    liveliness_changed_status->not_alive_count_change(lcs.not_alive_count_change);
}

void convertStatus(const DDS::SampleRejectedStatus& srs,
                   dds::core::status::SampleRejectedStatus& sample_rejected_status)
{
    sample_rejected_status->total_count(srs.total_count);
    sample_rejected_status->total_count_change(srs.total_count_change);
    dds::core::status::SampleRejectedState::MaskType mask(srs.last_reason);
    dds::core::status::SampleRejectedState last_reason(mask);
    sample_rejected_status->last_reason(mask);
    dds::core::InstanceHandle ih(srs.last_instance_handle);
    sample_rejected_status->last_instance_handle(ih);
}

void convertStatus(const DDS::SampleLostStatus& sls,
                   dds::core::status::SampleLostStatus& sample_lost_status)
{
    sample_lost_status->total_count(sls.total_count);
    sample_lost_status->total_count_change(sls.total_count_change);
}

void convertStatus(const DDS::RequestedDeadlineMissedStatus& rdms,
                   dds::core::status::RequestedDeadlineMissedStatus& requested_deadline_missed_status)
{
    requested_deadline_missed_status->total_count(rdms.total_count);
    requested_deadline_missed_status->total_count_change(rdms.total_count_change);
    dds::core::InstanceHandle ih(rdms.last_instance_handle);
    requested_deadline_missed_status->last_instance_handle(ih);
}

void convertStatus(const DDS::RequestedIncompatibleQosStatus& riqs,
                   dds::core::status::RequestedIncompatibleQosStatus& incompatible_qos_status)
{
    incompatible_qos_status->total_count(riqs.total_count);
    incompatible_qos_status->total_count_change(riqs.total_count_change);
    incompatible_qos_status->last_policy_id(riqs.last_policy_id);
    dds::core::policy::QosPolicyCountSeq policies;
    for(uint32_t i = 0; i < riqs.policies.length(); i++)
    {
        dds::core::policy::QosPolicyCount policyCount(riqs.policies[i].policy_id, riqs.policies[i].count);
        policies.push_back(policyCount);
    }
    incompatible_qos_status->set_policies(policies);
}

void convertStatus(const DDS::SubscriptionMatchedStatus& sms,
                   dds::core::status::SubscriptionMatchedStatus& subscription_matched_status)
{
    subscription_matched_status->total_count(sms.total_count);
    subscription_matched_status->total_count_change(sms.total_count_change);
    subscription_matched_status->current_count(sms.current_count);
    subscription_matched_status->current_count_change(sms.current_count_change);
    dds::core::InstanceHandle ih(sms.last_publication_handle);
    subscription_matched_status->last_publication_handle(ih);
}

//--------------------------------------------------------------------------------
//  DataWriter Status
//--------------------------------------------------------------------------------

void convertStatus(const DDS::OfferedDeadlineMissedStatus& odms,
                   dds::core::status::OfferedDeadlineMissedStatus& on_offered_deadline_missed)
{
    on_offered_deadline_missed->total_count(odms.total_count);
    on_offered_deadline_missed->total_count_change(odms.total_count_change);
    dds::core::InstanceHandle ih(odms.last_instance_handle);
    on_offered_deadline_missed->last_instance_handle(ih);
}

void convertStatus(const DDS::OfferedIncompatibleQosStatus& oiqs,
                   dds::core::status::OfferedIncompatibleQosStatus& on_offered_incompatible_qos)
{
    on_offered_incompatible_qos->total_count(oiqs.total_count);
    on_offered_incompatible_qos->total_count_change(oiqs.total_count_change);
    on_offered_incompatible_qos->last_policy_id(oiqs.last_policy_id);
    dds::core::policy::QosPolicyCountSeq policies;
    for(uint32_t i = 0; i < oiqs.policies.length(); i++)
    {
        dds::core::policy::QosPolicyCount policyCount(oiqs.policies[i].policy_id, oiqs.policies[i].count);
        policies.push_back(policyCount);
    }
    on_offered_incompatible_qos->set_policies(policies);
}

void convertStatus(const DDS::LivelinessLostStatus& lls,
                   dds::core::status::LivelinessLostStatus& on_liveliness_lost)
{
    on_liveliness_lost->total_count(lls.total_count);
    on_liveliness_lost->total_count_change(lls.total_count_change);
}

void convertStatus(const DDS::PublicationMatchedStatus& pms,
                   dds::core::status::PublicationMatchedStatus& on_publication_matched)
{
    on_publication_matched->total_count(pms.total_count);
    on_publication_matched->total_count_change(pms.total_count_change);
    on_publication_matched->current_count(pms.current_count);
    on_publication_matched->current_count_change(pms.current_count_change);
    dds::core::InstanceHandle ih(pms.last_subscription_handle);
    on_publication_matched->last_subscription_handle(ih);
}

}
}
}
