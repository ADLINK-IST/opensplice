/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef CCPP_LISTENERUTILS_H
#define CCPP_LISTENERUTILS_H

#include "gapi.h"
#include "ccpp.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    OS_DCPS_API void ccpp_DataWriterListener_copyIn(
              const ::DDS::DataWriterListener_ptr & from,
              gapi_dataWriterListener & to);

    OS_DCPS_API void ccpp_PublisherListener_copyIn(
              const ::DDS::PublisherListener_ptr & from,
              gapi_publisherListener & to);

    OS_DCPS_API void ccpp_DataReaderListener_copyIn(
              const ::DDS::DataReaderListener_ptr & from,
              gapi_dataReaderListener & to);

    OS_DCPS_API void ccpp_SubscriberListener_copyIn(
              const ::DDS::SubscriberListener_ptr & from,
              gapi_subscriberListener & to);

    OS_DCPS_API void ccpp_DomainParticipantListener_copyIn(
              const ::DDS::DomainParticipantListener_ptr & from,
              gapi_domainParticipantListener & to);

    OS_DCPS_API void ccpp_TopicListener_copyIn(
              const ::DDS::TopicListener_ptr & from,
              gapi_topicListener & to);

}

#endif
