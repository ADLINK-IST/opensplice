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
#ifndef CCPP_DOMAINPARTICIPANTFACTORY_H
#define CCPP_DOMAINPARTICIPANTFACTORY_H

#include "ccpp.h"
#include "os_report.h"
#include "gapi.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  typedef DomainParticipantFactoryInterface_ptr DomainParticipantFactory_ptr;
  typedef DomainParticipantFactoryInterface_var DomainParticipantFactory_var;

  class OS_DCPS_API DomainParticipantFactory
    : public virtual DomainParticipantFactoryInterface,
      public LOCAL_REFCOUNTED_OBJECT
    {
    private:
        static gapi_domainParticipantFactory _gapi_self;

        static os_mutex factoryMutex;
        DomainParticipantFactory( );

    public:
        ~DomainParticipantFactory( );
        static ::DDS::DomainParticipantFactory_ptr get_instance() THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DomainParticipant_ptr create_participant (
            ::DDS::DomainId_t domainId,
            const ::DDS::DomainParticipantQos & qos,
            ::DDS::DomainParticipantListener_ptr a_listener,
            ::DDS::StatusMask mask
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t delete_participant (
            ::DDS::DomainParticipant_ptr a_participant
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DomainParticipant_ptr lookup_participant (
           ::DDS::DomainId_t domainId
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::Domain_ptr lookup_domain (
           ::DDS::DomainId_t domainId
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t delete_domain (
            ::DDS::Domain_ptr a_domain
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t delete_contained_entities (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t set_qos (
          const ::DDS::DomainParticipantFactoryQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_qos (
          ::DDS::DomainParticipantFactoryQos & qos
        ) THROW_ORB_EXCEPTIONS;


        virtual ::DDS::ReturnCode_t set_default_participant_qos (
            const ::DDS::DomainParticipantQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_default_participant_qos (
            ::DDS::DomainParticipantQos & qos
        ) THROW_ORB_EXCEPTIONS;

        static DomainParticipantFactory_ptr _nil (void)
        {
          return (DomainParticipantFactory_ptr)0;
        }

        static const ::DDS::DomainParticipantQos  * participant_qos_default (void);
        static const ::DDS::TopicQos              * topic_qos_default (void);
        static const ::DDS::PublisherQos          * publisher_qos_default (void);
        static const ::DDS::SubscriberQos         * subscriber_qos_default (void);
        static const ::DDS::DataReaderQos         * datareader_qos_default (void);
        static const ::DDS::DataReaderViewQos     * datareaderview_qos_default (void);
        static const ::DDS::DataReaderQos         * datareader_qos_use_topic_qos (void);
        static const ::DDS::DataWriterQos         * datawriter_qos_default (void);
        static const ::DDS::DataWriterQos         * datawriter_qos_use_topic_qos (void);
    };
}

#endif /* DOMAINPARTICIPANTFACTORY */
