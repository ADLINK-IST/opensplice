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

#ifndef ORG_OPENSPLICE_CORE_STATUSCONVERTER_HPP_
#define ORG_OPENSPLICE_CORE_STATUSCONVERTER_HPP_

#include <org/opensplice/core/config.hpp>
#include <dds/core/types.hpp>
#include <dds/core/status/Status.hpp>
#include <org/opensplice/core/memory.hpp>
#include <org/opensplice/core/exception_helper.hpp>


namespace org
{
namespace opensplice
{
namespace core
{

//----------------------------------------------------------------------------
//    DataReader Status
//----------------------------------------------------------------------------

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::LivelinessChangedStatus& lcs,
                                        dds::core::status::LivelinessChangedStatus& liveliness_changed_status);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::SampleRejectedStatus& srs,
                                        dds::core::status::SampleRejectedStatus& sample_rejected_status);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::SampleLostStatus& sls,
                                        dds::core::status::SampleLostStatus& sample_lost_status);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::RequestedDeadlineMissedStatus& rdms,
                                        dds::core::status::RequestedDeadlineMissedStatus& requested_deadline_missed_status);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::RequestedIncompatibleQosStatus& riqs,
                                        dds::core::status::RequestedIncompatibleQosStatus& incompatible_qos_status);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::SubscriptionMatchedStatus& sms,
                                        dds::core::status::SubscriptionMatchedStatus& subscription_matched_status);

//----------------------------------------------------------------------------
//    DataWriter Status
//----------------------------------------------------------------------------

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::OfferedDeadlineMissedStatus& odms,
                                        dds::core::status::OfferedDeadlineMissedStatus& on_offered_deadline_missed);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::OfferedIncompatibleQosStatus& oiqs,
                                        dds::core::status::OfferedIncompatibleQosStatus& on_offered_incompatible_qos);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::LivelinessLostStatus& lls,
                                        dds::core::status::LivelinessLostStatus& on_liveliness_lost);

OSPL_ISOCPP_IMPL_API void convertStatus(const DDS::PublicationMatchedStatus& pms,
                                        dds::core::status::PublicationMatchedStatus& on_publication_matched);


}
}
}
#endif
