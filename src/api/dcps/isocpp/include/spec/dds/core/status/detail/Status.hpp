#ifndef OMG_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
#define OMG_DDS_CORE_STATUS_DETAIL_STATUS_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/status/TStatus.hpp>
#include <foo/bar/core/status/Status.hpp>


namespace dds { namespace core { namespace status { namespace detail {
    typedef dds::core::status::TInconsistentTopicStatus< foo::bar::core::InconsistentTopicStatusImpl >
    InconsistentTopicStatus;

    typedef dds::core::status::TLivelinessChangedStatus <foo::bar::core::LivelinessChangedStatusImpl>
    LivelinessChangedStatus;

    typedef dds::core::status::TLivelinessLostStatus<foo::bar::core::LivelinessLostStatusImpl>
    LivelinessLostStatus;

    typedef dds::core::status::TOfferedDeadlineMissedStatus<foo::bar::core::OfferedDeadlineMissedStatusImpl>
    OfferedDeadlineMissedStatus;

    typedef dds::core::status::TOfferedIncompatibleQosStatus<foo::bar::core::OfferedIncompatibleQosStatusImpl>
    OfferedIncompatibleQosStatus;

    typedef dds::core::status::TPublicationMatchedStatus<foo::bar::core::PublicationMatchedStatusImpl>
    PublicationMatchedStatus;

    typedef dds::core::status::TSampleRejectedStatus< foo::bar::core::SampleRejectedStatusImpl >
    SampleRejectedStatus;

    typedef dds::core::status::TRequestedDeadlineMissedStatus<foo::bar::core::RequestedDeadlineMissedStatusImpl>
    RequestedDeadlineMissedStatus;

    typedef dds::core::status::TRequestedIncompatibleQosStatus<foo::bar::core::RequestedIncompatibleQosStatusImpl>
    RequestedIncompatibleQosStatus;

    typedef dds::core::status::TSampleLostStatus<foo::bar::core::SampleLostStatusImpl>
    SampleLostStatus;

    typedef dds::core::status::TSubscriptionMatchedStatus<foo::bar::core::SubscriptionMatchedStatusImpl>
    SubscriptionMatchedStatus;
} } } } // namespace dds::core::status::detail


#endif /* OMG_DDS_CORE_STATUS_DETAIL_STATUS_HPP_ */
