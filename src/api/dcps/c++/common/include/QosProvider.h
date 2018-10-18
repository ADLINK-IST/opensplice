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
#ifndef CPP_DDS_OPENSPLICE_QOSPROVIDER_H
#define CPP_DDS_OPENSPLICE_QOSPROVIDER_H

#include "CppSuperClass.h"
#include "cpp_dcps_if.h"

C_CLASS(cmn_qosProvider);

namespace DDS {

    typedef QosProviderInterface_ptr QosProvider_ptr;
    typedef QosProviderInterface_var QosProvider_var;

    class OS_API QosProvider
      : public virtual ::DDS::QosProviderInterface,
        public virtual ::DDS::LocalObject
    {
    private:
        /** \brief FIXME: comment
         */
        cmn_qosProvider qosProvider;

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

        virtual ::DDS::ReturnCode_t
        deinit();

    };
}
#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_QOSPROVIDER_H */
