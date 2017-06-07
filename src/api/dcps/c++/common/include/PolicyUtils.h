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
#ifndef DDS_OPENSPLICE_POLICYUTILS_H
#define DDS_OPENSPLICE_POLICYUTILS_H


#include "ccpp.h"
#include "u_user.h"


/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS {
namespace OpenSplice {
namespace Utils {


extern const DDS::DeadlineQosPolicy             DeadlineQosPolicy_default;
extern const DDS::DestinationOrderQosPolicy     DestinationOrderQosPolicy_default;
extern const DDS::DurabilityServiceQosPolicy    DurabilityServiceQosPolicy_default;
extern const DDS::DurabilityQosPolicy           DurabilityQosPolicy_default;
extern const DDS::EntityFactoryQosPolicy        EntityFactoryQosPolicy_default;
extern const DDS::GroupDataQosPolicy            GroupDataQosPolicy_default;
extern const DDS::HistoryQosPolicy              HistoryQosPolicy_default;
extern const DDS::LatencyBudgetQosPolicy        LatencyBudgetQosPolicy_default;
extern const DDS::LifespanQosPolicy             LifespanQosPolicy_default;
extern const DDS::LivelinessQosPolicy           LivelinessQosPolicy_default;
extern const DDS::OwnershipStrengthQosPolicy    OwnershipStrengthQosPolicy_default;
extern const DDS::OwnershipQosPolicy            OwnershipQosPolicy_default;
extern const DDS::PartitionQosPolicy            PartitionQosPolicy_default;
extern const DDS::PresentationQosPolicy         PresentationQosPolicy_default;
extern const DDS::ReaderDataLifecycleQosPolicy  ReaderDataLifecycleQosPolicy_default;
extern const DDS::ReaderLifespanQosPolicy       ReaderLifespanQosPolicy_default;
extern const DDS::ReliabilityQosPolicy          ReliabilityQosPolicy_default;
extern const DDS::ReliabilityQosPolicy          ReliabilityQosPolicy_writerDefault;
extern const DDS::ResourceLimitsQosPolicy       ResourceLimitsQosPolicy_default;
extern const DDS::TransportPriorityQosPolicy    TransportPriorityQosPolicy_default;
extern const DDS::SchedulingQosPolicy           SchedulingQosPolicy_default;
extern const DDS::ShareQosPolicy                ShareQosPolicy_default;
extern const DDS::SubscriptionKeyQosPolicy      SubscriptionKeyQosPolicy_default;
extern const DDS::TimeBasedFilterQosPolicy      TimeBasedFilterQosPolicy_default;
extern const DDS::TopicDataQosPolicy            TopicDataQosPolicy_default;
extern const DDS::UserDataQosPolicy             UserDataQosPolicy_default;
extern const DDS::ViewKeyQosPolicy              ViewKeyQosPolicy_default;
extern const DDS::WriterDataLifecycleQosPolicy  WriterDataLifecycleQosPolicy_default;


/*
 * Policy validations
 */
DDS::ReturnCode_t policyIsValid( const DDS::DeadlineQosPolicy                &policy);
DDS::ReturnCode_t policyIsValid( const DDS::DestinationOrderQosPolicy        &policy);
DDS::ReturnCode_t policyIsValid( const DDS::DurabilityQosPolicy              &policy);
DDS::ReturnCode_t policyIsValid( const DDS::DurabilityServiceQosPolicy       &policy);
DDS::ReturnCode_t policyIsValid( const DDS::EntityFactoryQosPolicy           &policy);
DDS::ReturnCode_t policyIsValid( const DDS::GroupDataQosPolicy               &policy);
DDS::ReturnCode_t policyIsValid( const DDS::HistoryQosPolicy                 &policy);
DDS::ReturnCode_t policyIsValid( const DDS::InvalidSampleVisibilityQosPolicy &policy);
DDS::ReturnCode_t policyIsValid( const DDS::LatencyBudgetQosPolicy           &policy);
DDS::ReturnCode_t policyIsValid( const DDS::LifespanQosPolicy                &policy);
DDS::ReturnCode_t policyIsValid( const DDS::LivelinessQosPolicy              &policy);
DDS::ReturnCode_t policyIsValid( const DDS::OwnershipQosPolicy               &policy);
DDS::ReturnCode_t policyIsValid( const DDS::OwnershipStrengthQosPolicy       &policy);
DDS::ReturnCode_t policyIsValid( const DDS::PartitionQosPolicy               &policy);
DDS::ReturnCode_t policyIsValid( const DDS::PresentationQosPolicy            &policy);
DDS::ReturnCode_t policyIsValid( const DDS::ReaderDataLifecycleQosPolicy     &policy);
DDS::ReturnCode_t policyIsValid( const DDS::ReaderLifespanQosPolicy          &policy);
DDS::ReturnCode_t policyIsValid( const DDS::ReliabilityQosPolicy             &policy);
DDS::ReturnCode_t policyIsValid( const DDS::ResourceLimitsQosPolicy          &policy);
DDS::ReturnCode_t policyIsValid( const DDS::ShareQosPolicy                   &policy);
DDS::ReturnCode_t policyIsValid( const DDS::SchedulingClassQosPolicy         &policy);
DDS::ReturnCode_t policyIsValid( const DDS::SchedulingPriorityQosPolicy      &policy);
DDS::ReturnCode_t policyIsValid( const DDS::SchedulingQosPolicy              &policy);
DDS::ReturnCode_t policyIsValid( const DDS::SubscriptionKeyQosPolicy         &policy);
DDS::ReturnCode_t policyIsValid( const DDS::TimeBasedFilterQosPolicy         &policy);
DDS::ReturnCode_t policyIsValid( const DDS::TopicDataQosPolicy               &policy);
DDS::ReturnCode_t policyIsValid( const DDS::TransportPriorityQosPolicy       &policy);
DDS::ReturnCode_t policyIsValid( const DDS::UserDataQosPolicy                &policy);
DDS::ReturnCode_t policyIsValid( const DDS::ViewKeyQosPolicy                 &policy);
DDS::ReturnCode_t policyIsValid( const DDS::WriterDataLifecycleQosPolicy     &policy);


/*
 * Policy consistency checks
 */
DDS::ReturnCode_t
policiesAreConsistent(
    const DDS::HistoryQosPolicy &history,
    const DDS::ResourceLimitsQosPolicy &resource_limits);

DDS::ReturnCode_t
policiesAreConsistent(
    const DDS::DeadlineQosPolicy &deadline,
    const DDS::TimeBasedFilterQosPolicy &time_based_filter);


/*
 * Policy comparison
 */
DDS::Boolean
policyIsEqual (
    const DDS::DeadlineQosPolicy &a,
    const DDS::DeadlineQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::DestinationOrderQosPolicy &a,
    const DDS::DestinationOrderQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::DurabilityQosPolicy &a,
    const DDS::DurabilityQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::DurabilityServiceQosPolicy &a,
    const DDS::DurabilityServiceQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::EntityFactoryQosPolicy &a,
    const DDS::EntityFactoryQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::GroupDataQosPolicy &a,
    const DDS::GroupDataQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::HistoryQosPolicy &a,
    const DDS::HistoryQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::LatencyBudgetQosPolicy &a,
    const DDS::LatencyBudgetQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::LifespanQosPolicy &a,
    const DDS::LifespanQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::LivelinessQosPolicy &a,
    const DDS::LivelinessQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::OwnershipQosPolicy &a,
    const DDS::OwnershipQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::OwnershipStrengthQosPolicy &a,
    const DDS::OwnershipStrengthQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::PartitionQosPolicy &a,
    const DDS::PartitionQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::PresentationQosPolicy &a,
    const DDS::PresentationQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::ReaderDataLifecycleQosPolicy &a,
    const DDS::ReaderDataLifecycleQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::ReaderLifespanQosPolicy &a,
    const DDS::ReaderLifespanQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::ReliabilityQosPolicy &a,
    const DDS::ReliabilityQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::ResourceLimitsQosPolicy &a,
    const DDS::ResourceLimitsQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::SchedulingQosPolicy &a,
    const DDS::SchedulingQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::ShareQosPolicy &a,
    const DDS::ShareQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::SubscriptionKeyQosPolicy &a,
    const DDS::SubscriptionKeyQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::TimeBasedFilterQosPolicy &a,
    const DDS::TimeBasedFilterQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::TopicDataQosPolicy &a,
    const DDS::TopicDataQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::TransportPriorityQosPolicy &a,
    const DDS::TransportPriorityQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::UserDataQosPolicy &a,
    const DDS::UserDataQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::ViewKeyQosPolicy &a,
    const DDS::ViewKeyQosPolicy &b);

DDS::Boolean
policyIsEqual (
    const DDS::WriterDataLifecycleQosPolicy &a,
    const DDS::WriterDataLifecycleQosPolicy &b);


/*
 * Policy conversions
 */
DDS::ReturnCode_t copyPolicyIn( const DDS::DeadlineQosPolicy            &from,   v_deadlinePolicy          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::DestinationOrderQosPolicy    &from,   v_orderbyPolicy           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::DurabilityQosPolicy          &from,   v_durabilityPolicy        &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::DurabilityServiceQosPolicy   &from,   v_durabilityServicePolicy &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::EntityFactoryQosPolicy       &from,   v_entityFactoryPolicy     &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::HistoryQosPolicy             &from,   v_historyPolicy           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::LatencyBudgetQosPolicy       &from,   v_latencyPolicy           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::LifespanQosPolicy            &from,   v_lifespanPolicy          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::LivelinessQosPolicy          &from,   v_livelinessPolicy        &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::OwnershipQosPolicy           &from,   v_ownershipPolicy         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::OwnershipStrengthQosPolicy   &from,   v_strengthPolicy          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::PresentationQosPolicy        &from,   v_presentationPolicy      &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ReaderDataLifecycleQosPolicy &from,   v_readerLifecyclePolicy   &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ReaderLifespanQosPolicy      &from,   v_readerLifespanPolicy    &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ReliabilityQosPolicy         &from,   v_reliabilityPolicy       &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ResourceLimitsQosPolicy      &from,   v_resourcePolicy          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ShareQosPolicy               &from,   v_sharePolicy             &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::SubscriptionKeyQosPolicy     &from,   v_userKeyPolicy           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::TimeBasedFilterQosPolicy     &from,   v_pacingPolicy            &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::TransportPriorityQosPolicy   &from,   v_transportPolicy         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::UserDataQosPolicy            &from,   v_userDataPolicy          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ViewKeyQosPolicy             &from,   v_userKeyPolicy           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::WriterDataLifecycleQosPolicy &from,   v_writerLifecyclePolicy   &to);


DDS::ReturnCode_t copyPolicyOut( const v_deadlinePolicy          &from,  DDS::DeadlineQosPolicy            &to);
DDS::ReturnCode_t copyPolicyOut( const v_orderbyPolicy           &from,  DDS::DestinationOrderQosPolicy    &to);
DDS::ReturnCode_t copyPolicyOut( const v_durabilityPolicy        &from,  DDS::DurabilityQosPolicy          &to);
DDS::ReturnCode_t copyPolicyOut( const v_durabilityServicePolicy &from,  DDS::DurabilityServiceQosPolicy   &to);
DDS::ReturnCode_t copyPolicyOut( const v_entityFactoryPolicy     &from,  DDS::EntityFactoryQosPolicy       &to);
DDS::ReturnCode_t copyPolicyOut( const v_historyPolicy           &from,  DDS::HistoryQosPolicy             &to);
DDS::ReturnCode_t copyPolicyOut( const v_latencyPolicy           &from,  DDS::LatencyBudgetQosPolicy       &to);
DDS::ReturnCode_t copyPolicyOut( const v_lifespanPolicy          &from,  DDS::LifespanQosPolicy            &to);
DDS::ReturnCode_t copyPolicyOut( const v_livelinessPolicy        &from,  DDS::LivelinessQosPolicy          &to);
DDS::ReturnCode_t copyPolicyOut( const v_ownershipPolicy         &from,  DDS::OwnershipQosPolicy           &to);
DDS::ReturnCode_t copyPolicyOut( const v_strengthPolicy          &from,  DDS::OwnershipStrengthQosPolicy   &to);
DDS::ReturnCode_t copyPolicyOut( const v_presentationPolicy      &from,  DDS::PresentationQosPolicy        &to);
DDS::ReturnCode_t copyPolicyOut( const v_readerLifecyclePolicy   &from,  DDS::ReaderDataLifecycleQosPolicy &to);
DDS::ReturnCode_t copyPolicyOut( const v_readerLifespanPolicy    &from,  DDS::ReaderLifespanQosPolicy      &to);
DDS::ReturnCode_t copyPolicyOut( const v_reliabilityPolicy       &from,  DDS::ReliabilityQosPolicy         &to);
DDS::ReturnCode_t copyPolicyOut( const v_resourcePolicy          &from,  DDS::ResourceLimitsQosPolicy      &to);
DDS::ReturnCode_t copyPolicyOut( const v_sharePolicy             &from,  DDS::ShareQosPolicy               &to);
DDS::ReturnCode_t copyPolicyOut( const v_userKeyPolicy           &from,  DDS::SubscriptionKeyQosPolicy     &to);
DDS::ReturnCode_t copyPolicyOut( const v_pacingPolicy            &from,  DDS::TimeBasedFilterQosPolicy     &to);
DDS::ReturnCode_t copyPolicyOut( const v_transportPolicy         &from,  DDS::TransportPriorityQosPolicy   &to);
DDS::ReturnCode_t copyPolicyOut( const v_userDataPolicy          &from,  DDS::UserDataQosPolicy            &to);
DDS::ReturnCode_t copyPolicyOut( const v_userKeyPolicy           &from,  DDS::ViewKeyQosPolicy             &to);
DDS::ReturnCode_t copyPolicyOut( const v_writerLifecyclePolicy   &from,  DDS::WriterDataLifecycleQosPolicy &to);


DDS::ReturnCode_t copyPolicyIn( const DDS::DeadlineQosPolicy            &from,   v_deadlinePolicyI          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::DestinationOrderQosPolicy    &from,   v_orderbyPolicyI           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::DurabilityQosPolicy          &from,   v_durabilityPolicyI        &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::DurabilityServiceQosPolicy   &from,   v_durabilityServicePolicyI &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::EntityFactoryQosPolicy       &from,   v_entityFactoryPolicyI     &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::GroupDataQosPolicy           &from,   v_groupDataPolicyI         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::HistoryQosPolicy             &from,   v_historyPolicyI           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::LatencyBudgetQosPolicy       &from,   v_latencyPolicyI           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::LifespanQosPolicy            &from,   v_lifespanPolicyI          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::LivelinessQosPolicy          &from,   v_livelinessPolicyI        &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::OwnershipQosPolicy           &from,   v_ownershipPolicyI         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::OwnershipStrengthQosPolicy   &from,   v_strengthPolicyI          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::PartitionQosPolicy           &from,   v_partitionPolicyI         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::PresentationQosPolicy        &from,   v_presentationPolicyI      &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ReaderDataLifecycleQosPolicy &from,   v_readerLifecyclePolicyI   &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ReaderLifespanQosPolicy      &from,   v_readerLifespanPolicyI    &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ReliabilityQosPolicy         &from,   v_reliabilityPolicyI       &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ResourceLimitsQosPolicy      &from,   v_resourcePolicyI          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::SchedulingQosPolicy          &from,   v_schedulePolicyI          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ShareQosPolicy               &from,   v_sharePolicyI             &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::SubscriptionKeyQosPolicy     &from,   v_userKeyPolicyI           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::TimeBasedFilterQosPolicy     &from,   v_pacingPolicyI            &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::TopicDataQosPolicy           &from,   v_topicDataPolicyI         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::TransportPriorityQosPolicy   &from,   v_transportPolicyI         &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::UserDataQosPolicy            &from,   v_userDataPolicyI          &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::ViewKeyQosPolicy             &from,   v_userKeyPolicyI           &to);
DDS::ReturnCode_t copyPolicyIn( const DDS::WriterDataLifecycleQosPolicy &from,   v_writerLifecyclePolicyI   &to);


DDS::ReturnCode_t copyPolicyOut( const v_deadlinePolicyI          &from,  DDS::DeadlineQosPolicy            &to);
DDS::ReturnCode_t copyPolicyOut( const v_orderbyPolicyI           &from,  DDS::DestinationOrderQosPolicy    &to);
DDS::ReturnCode_t copyPolicyOut( const v_durabilityPolicyI        &from,  DDS::DurabilityQosPolicy          &to);
DDS::ReturnCode_t copyPolicyOut( const v_durabilityServicePolicyI &from,  DDS::DurabilityServiceQosPolicy   &to);
DDS::ReturnCode_t copyPolicyOut( const v_entityFactoryPolicyI     &from,  DDS::EntityFactoryQosPolicy       &to);
DDS::ReturnCode_t copyPolicyOut( const v_groupDataPolicyI         &from,  DDS::GroupDataQosPolicy           &to);
DDS::ReturnCode_t copyPolicyOut( const v_historyPolicyI           &from,  DDS::HistoryQosPolicy             &to);
DDS::ReturnCode_t copyPolicyOut( const v_latencyPolicyI           &from,  DDS::LatencyBudgetQosPolicy       &to);
DDS::ReturnCode_t copyPolicyOut( const v_lifespanPolicyI          &from,  DDS::LifespanQosPolicy            &to);
DDS::ReturnCode_t copyPolicyOut( const v_livelinessPolicyI        &from,  DDS::LivelinessQosPolicy          &to);
DDS::ReturnCode_t copyPolicyOut( const v_ownershipPolicyI         &from,  DDS::OwnershipQosPolicy           &to);
DDS::ReturnCode_t copyPolicyOut( const v_strengthPolicyI          &from,  DDS::OwnershipStrengthQosPolicy   &to);
DDS::ReturnCode_t copyPolicyOut( const v_partitionPolicyI         &from,  DDS::PartitionQosPolicy           &to);
DDS::ReturnCode_t copyPolicyOut( const v_presentationPolicyI      &from,  DDS::PresentationQosPolicy        &to);
DDS::ReturnCode_t copyPolicyOut( const v_readerLifecyclePolicyI   &from,  DDS::ReaderDataLifecycleQosPolicy &to);
DDS::ReturnCode_t copyPolicyOut( const v_readerLifespanPolicyI    &from,  DDS::ReaderLifespanQosPolicy      &to);
DDS::ReturnCode_t copyPolicyOut( const v_reliabilityPolicyI       &from,  DDS::ReliabilityQosPolicy         &to);
DDS::ReturnCode_t copyPolicyOut( const v_resourcePolicyI          &from,  DDS::ResourceLimitsQosPolicy      &to);
DDS::ReturnCode_t copyPolicyOut( const v_sharePolicyI             &from,  DDS::ShareQosPolicy               &to);
DDS::ReturnCode_t copyPolicyOut( const v_schedulePolicyI          &from,  DDS::SchedulingQosPolicy          &to);
DDS::ReturnCode_t copyPolicyOut( const v_userKeyPolicyI           &from,  DDS::SubscriptionKeyQosPolicy     &to);
DDS::ReturnCode_t copyPolicyOut( const v_pacingPolicyI            &from,  DDS::TimeBasedFilterQosPolicy     &to);
DDS::ReturnCode_t copyPolicyOut( const v_topicDataPolicyI         &from,  DDS::TopicDataQosPolicy           &to);
DDS::ReturnCode_t copyPolicyOut( const v_transportPolicyI         &from,  DDS::TransportPriorityQosPolicy   &to);
DDS::ReturnCode_t copyPolicyOut( const v_userDataPolicyI          &from,  DDS::UserDataQosPolicy            &to);
DDS::ReturnCode_t copyPolicyOut( const v_userKeyPolicyI           &from,  DDS::ViewKeyQosPolicy             &to);
DDS::ReturnCode_t copyPolicyOut( const v_writerLifecyclePolicyI   &from,  DDS::WriterDataLifecycleQosPolicy &to);


} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */

#endif /* DDS_OPENSPLICE_POLICYUTILS_H */


