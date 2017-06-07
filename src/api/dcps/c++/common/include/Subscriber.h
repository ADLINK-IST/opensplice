/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef CPP_DDS_OPENSPLICE_SUBSCRIBER_H
#define CPP_DDS_OPENSPLICE_SUBSCRIBER_H

#include "Entity.h"
#include "EntityContainer.h"
#include "Topic.h"
#include "DataReader.h"
#include "DomainParticipant.h"
#include "MiscUtils.h"

#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS {
    namespace OpenSplice {
        class OS_API Subscriber
            : public virtual ::DDS::Subscriber,
              public ::DDS::OpenSplice::EntityContainer,
              public ::DDS::OpenSplice::Entity
        {
            friend class ::DDS::OpenSplice::DomainParticipant;
            friend class ::DDS::OpenSplice::DataReader;

        private:
            DDS::OpenSplice::DomainParticipant *participant;
            DataReaderQos defaultReaderQos;
            DDS::OpenSplice::ObjSet *readers;
            DDS::Boolean factoryAutoEnable;

            char *
            create_datareader_name (
                DDS::OpenSplice::TopicDescription *topic);

            DDS::Boolean
            wlReq_insertReader(
                DDS::OpenSplice::DataReader *reader);

            DDS::Boolean
            wlReq_removeReader(
                DDS::OpenSplice::DataReader *reader);

            typedef struct {
                const DDS::Char *topicName;
                DDS::DataReader_ptr reader;
            } lookupByTopicArg;

            static DDS::Boolean
            rlReq_lookupByTopic (
                DDS::Object_ptr element,
                lookupByTopicArg *arg);

            static DDS::Boolean
            rlReq_readerCheckHandle (
                DDS::Object_ptr object,
                DDS::InstanceHandle_t *argHandle);

        protected:
            Subscriber();

            virtual ~Subscriber();

            virtual DDS::ReturnCode_t
            init(
                DDS::OpenSplice::DomainParticipant *participant,
                const DDS::Char *name,
                const DDS::SubscriberQos &qos
            );

            DDS::ReturnCode_t
            nlReq_init (
                DDS::OpenSplice::DomainParticipant *participant,
                const DDS::Char *name,
                const DDS::SubscriberQos &qos
            );

            virtual DDS::ReturnCode_t
            wlReq_deinit();

        public:
            DDS::DataReader_ptr
            create_datareader (
                DDS::TopicDescription_ptr a_topic,
                const ::DDS::DataReaderQos &qos,
                DDS::DataReaderListener_ptr a_listener,
                DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            delete_datareader (
                DDS::DataReader_ptr a_datareader
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            delete_contained_entities (
            ) THROW_ORB_EXCEPTIONS;

            DDS::DataReader_ptr
            lookup_datareader (
                const char *topic_name
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_datareaders (
                DDS::DataReaderSeq &readers,
                DDS::SampleStateMask sample_states,
                DDS::ViewStateMask view_states,
                DDS::InstanceStateMask instance_states
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            notify_datareaders (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_qos (
                const DDS::SubscriberQos & qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_qos (
                DDS::SubscriberQos &qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_listener (
                DDS::SubscriberListener_ptr a_listener,
                DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            DDS::SubscriberListener_ptr
            get_listener (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            begin_access (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            end_access (
            ) THROW_ORB_EXCEPTIONS;

            DDS::DomainParticipant_ptr
            get_participant (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_default_datareader_qos (
                const DDS::DataReaderQos &qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_default_datareader_qos (
                DDS::DataReaderQos & qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            copy_from_topic_qos (
                DDS::DataReaderQos &a_datareader_qos,
                const DDS::TopicQos &a_topic_qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            copy_from_topicdescription (
                DDS::DataReaderQos &a_datareader_qos,
                const DDS::OpenSplice::TopicDescription *a_topic);

            DDS::Boolean
            contains_entity (
                DDS::InstanceHandle_t a_handle
            ) THROW_ORB_EXCEPTIONS;

            virtual void
            nlReq_notify_listener(
                DDS::OpenSplice::Entity *sourceEntity,
                DDS::ULong               triggerMask,
                void                    *eventData
            ) THROW_ORB_EXCEPTIONS;

        }; /* class Subscriber */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_SUBSCRIBER_H */
