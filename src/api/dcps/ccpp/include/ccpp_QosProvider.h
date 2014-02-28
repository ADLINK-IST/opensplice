/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef CCPP_QOSPROVIDER_H
#define CCPP_QOSPROVIDER_H

#include "ccpp.h"

C_CLASS(qp_qosProvider);

namespace DDS {

    typedef QosProviderInterface_ptr QosProvider_ptr;
    typedef QosProviderInterface_var QosProvider_var;

    class OS_DCPS_API QosProvider
      : public virtual ::DDS::QosProviderInterface,
        public LOCAL_REFCOUNTED_OBJECT
    {
    private:
        /** \brief FIXME: comment
         */
        qp_qosProvider qosProvider;

        /** \brief FIXME: comment
         */
        DDS::ReturnCode_t
        is_ready ();

    public:
        QosProvider (
            const char *uri,
            const char *profile);

        ~QosProvider ();

        virtual ::DDS::ReturnCode_t
        get_participant_qos (
            ::DDS::DomainParticipantQos &participantQos,
            const char *id
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_topic_qos (
            ::DDS::TopicQos &topicQos,
            const char *id
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_subscriber_qos (
            ::DDS::SubscriberQos &subscriberQos,
            const char *id
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_datareader_qos (
            ::DDS::DataReaderQos &dataReaderQos,
            const char *id
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_publisher_qos (
            ::DDS::PublisherQos &publisherQos,
            const char *id
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_datawriter_qos (
            ::DDS::DataWriterQos &dataWriterQos,
             const char *id
        ) THROW_ORB_EXCEPTIONS;
    };
}

#endif
