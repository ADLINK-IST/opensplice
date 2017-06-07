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


/**
 * @file
 */


#include <dds/core/status/Status.hpp>
#include <dds/core/status/State.hpp>


namespace dds
{
namespace core
{
namespace status
{

// 0
template <>
StatusMask get_status<InconsistentTopicStatus>()
{
    return StatusMask::inconsistent_topic();
}
// 1
template <>
StatusMask get_status<OfferedDeadlineMissedStatus>()
{
    return StatusMask::offered_deadline_missed();
}
// 2
template <>
StatusMask get_status<RequestedDeadlineMissedStatus>()
{
    return StatusMask::requested_deadline_missed();
}
// 5
template <>
StatusMask get_status<OfferedIncompatibleQosStatus>()
{
    return StatusMask::offered_incompatible_qos();
}
// 6
template <>
StatusMask get_status<RequestedIncompatibleQosStatus>()
{
    return StatusMask::requested_incompatible_qos();
}
// 7
template <>
StatusMask get_status<SampleLostStatus>()
{
    return StatusMask::sample_lost();
}
// 8
template <>
StatusMask get_status<SampleRejectedStatus>()
{
    return StatusMask::sample_rejected();
}

// 9
template <>
StatusMask get_status<DataOnReadersStatus>()
{
    return StatusMask::data_on_readers();
}

// 10
template <>
StatusMask get_status<DataAvailableStatus>()
{
    return StatusMask::data_available();
}

// 11
template <>
StatusMask get_status<LivelinessLostStatus>()
{
    return StatusMask::liveliness_lost();
}

// 12
template <>
StatusMask get_status<LivelinessChangedStatus>()
{
    return StatusMask::liveliness_changed();
}

// 13
template <>
StatusMask get_status<PublicationMatchedStatus>()
{
    return StatusMask::publication_matched();
}

// 14
template <>
StatusMask get_status<SubscriptionMatchedStatus>()
{
    return StatusMask::subscription_matched();
}


}
}
}
