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

#ifndef ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_
#define ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{

template<typename PUBT>
class OSPL_ISOCPP_IMPL_API PublisherEventForwarder: public DDS::PublisherListener
{
public:
    PublisherEventForwarder(
        const PUBT& pub_,
        dds::pub::PublisherListener* listener);

    virtual ~PublisherEventForwarder();


    virtual dds::pub::PublisherListener* listener();


    //DWL
    virtual void on_offered_deadline_missed(DDS::DataWriter_ptr writer, const DDS::OfferedDeadlineMissedStatus& status) {};
    virtual void on_offered_incompatible_qos(DDS::DataWriter_ptr writer, const DDS::OfferedIncompatibleQosStatus& status) {};
    virtual void on_liveliness_lost(DDS::DataWriter_ptr writer, const DDS::LivelinessLostStatus& status) {};
    virtual void on_publication_matched(DDS::DataWriter_ptr writer, const DDS::PublicationMatchedStatus& status) {};

    PUBT pub_;
    dds::pub::PublisherListener* listener_;
};
}
}
}
#endif /* ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_ */
