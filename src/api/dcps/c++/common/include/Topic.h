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
#ifndef CPP_DDS_OPENSPLICE_TOPIC_H
#define CPP_DDS_OPENSPLICE_TOPIC_H

#include "Entity.h"
#include "TopicDescription.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    namespace OpenSplice
    {
        DDS::Boolean
        set_topic_listener_mask (
            DDS::Object_ptr element,
            void *arg);

        class Publisher;

        class OS_API Topic
            : public virtual ::DDS::Topic,
              public ::DDS::OpenSplice::Entity,
              public ::DDS::OpenSplice::TopicDescription

        {
            friend class ::DDS::OpenSplice::DomainParticipant;
            friend class ::DDS::OpenSplice::Publisher;
            friend class ::DDS::OpenSplice::DataWriter;
            friend DDS::Boolean DDS::OpenSplice::set_topic_listener_mask (DDS::Object_ptr element, void *arg);

        private:
            DDS::StatusMask topicListenerInterest;
            DDS::StatusMask participantListenerInterest;

        protected:
            Topic();

            virtual ~Topic();

            ::DDS::ReturnCode_t
            virtual init(
                    const u_topic uTopic,
                    DDS::OpenSplice::DomainParticipant *participant,
                    const DDS::Char *topic_name,
                    const DDS::Char *type_name,
                    DDS::OpenSplice::TypeSupportMetaHolder *type_meta_holder);

            ::DDS::ReturnCode_t
            nlReq_init (
                    const u_topic uTopic,
                    DDS::OpenSplice::DomainParticipant *participant,
                    const DDS::Char *topic_name,
                    const DDS::Char *type_name,
                    DDS::OpenSplice::TypeSupportMetaHolder *type_meta_holder);

            virtual ::DDS::ReturnCode_t
            wlReq_deinit();

            DDS::ReturnCode_t
            set_participant_listener_mask (
                    ::DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

        public:
            virtual ::DDS::ReturnCode_t
            get_inconsistent_topic_status (
                    ::DDS::InconsistentTopicStatus & a_status
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            get_all_data_disposed_topic_status (
                    ::DDS::AllDataDisposedTopicStatus & a_status
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            get_qos (
                    ::DDS::TopicQos & qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            set_qos (
                    const ::DDS::TopicQos & qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::TopicListener_ptr
            get_listener (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            set_listener (
                    ::DDS::TopicListener_ptr a_listener,
                    ::DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t
            dispose_all_data (
            ) THROW_ORB_EXCEPTIONS;

            virtual void
            nlReq_notify_listener(
                    ::DDS::OpenSplice::Entity *sourceEntity,
                    ::DDS::ULong               triggerMask,
                    void                      *eventData
            ) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            validate_filter (
                    const DDS::Char *filter_expression,
                    const DDS::StringSeq &filter_parameters
            ) THROW_ORB_EXCEPTIONS;

        }; /* class Topic */
    } /* namespace OpenSplice */
}/* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_TOPIC_H */
