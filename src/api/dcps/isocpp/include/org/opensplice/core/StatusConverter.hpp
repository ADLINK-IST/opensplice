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
