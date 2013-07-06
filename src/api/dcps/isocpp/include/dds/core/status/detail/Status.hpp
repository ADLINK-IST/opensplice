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
