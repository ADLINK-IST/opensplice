/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
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
