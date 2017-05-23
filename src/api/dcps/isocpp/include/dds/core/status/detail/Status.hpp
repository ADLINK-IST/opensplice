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
#ifndef OSPL_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
#define OSPL_DDS_CORE_STATUS_DETAIL_STATUS_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/status/TStatus.hpp>
#include <org/opensplice/core/StatusImpl.hpp>


namespace dds
{
namespace core
{
namespace status
{
namespace detail
{
typedef dds::core::status::TInconsistentTopicStatus< org::opensplice::core::InconsistentTopicStatusImpl >
InconsistentTopicStatus;

typedef dds::core::status::TLivelinessChangedStatus <org::opensplice::core::LivelinessChangedStatusImpl>
LivelinessChangedStatus;

typedef dds::core::status::TLivelinessLostStatus<org::opensplice::core::LivelinessLostStatusImpl>
LivelinessLostStatus;

typedef dds::core::status::TOfferedDeadlineMissedStatus<org::opensplice::core::OfferedDeadlineMissedStatusImpl>
OfferedDeadlineMissedStatus;

typedef dds::core::status::TOfferedIncompatibleQosStatus<org::opensplice::core::OfferedIncompatibleQosStatusImpl>
OfferedIncompatibleQosStatus;

typedef dds::core::status::TPublicationMatchedStatus<org::opensplice::core::PublicationMatchedStatusImpl>
PublicationMatchedStatus;

typedef dds::core::status::TSampleRejectedStatus< org::opensplice::core::SampleRejectedStatusImpl >
SampleRejectedStatus;

typedef dds::core::status::TRequestedDeadlineMissedStatus<org::opensplice::core::RequestedDeadlineMissedStatusImpl>
RequestedDeadlineMissedStatus;

typedef dds::core::status::TRequestedIncompatibleQosStatus<org::opensplice::core::RequestedIncompatibleQosStatusImpl>
RequestedIncompatibleQosStatus;

typedef dds::core::status::TSampleLostStatus<org::opensplice::core::SampleLostStatusImpl>
SampleLostStatus;

typedef dds::core::status::TSubscriptionMatchedStatus<org::opensplice::core::SubscriptionMatchedStatusImpl>
SubscriptionMatchedStatus;
}
}
}
} // namespace dds::core::status::detail


// End of implementation

#endif /* OSPL_DDS_CORE_STATUS_DETAIL_STATUS_HPP_ */
