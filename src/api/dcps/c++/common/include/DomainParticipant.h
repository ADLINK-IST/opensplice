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
#ifndef CPP_DDS_OPENSPLICE_DOMAINPARTICIPANT_H
#define CPP_DDS_OPENSPLICE_DOMAINPARTICIPANT_H

#include "Entity.h"
#include "EntityContainer.h"
#include "DomainParticipantFactory.h"
#include "StrObjMap.h"
#include "ObjSet.h"

#include "cpp_dcps_if.h"


/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define PARTICIPANT_BUILTINTOPIC_NAME "DCPSParticipant"
#define TOPIC_BUILTINTOPIC_NAME "DCPSTopic"
#define PUBLICATION_BUILTINTOPIC_NAME "DCPSPublication"
#define SUBSCRIPTION_BUILTINTOPIC_NAME "DCPSSubscription"


namespace DDS
{
    namespace OpenSplice
    {
        // Forward declarations.
        class TopicDescription;
        class Topic;
        class ContentFilteredTopic;
        class Publisher;
        class Subscriber;
        class TypeSupport;
        class TypeSupportMetaHolder;

        class OS_API DomainParticipant
            : public virtual ::DDS::DomainParticipant,
              public ::DDS::OpenSplice::EntityContainer,
              public ::DDS::OpenSplice::Entity
        {
            friend class ::DDS::DomainParticipantFactory;
            friend class ::DDS::OpenSplice::Publisher;
            friend class ::DDS::OpenSplice::Subscriber;

        private:
            DDS::DomainParticipantFactory_ptr   factory;
            DDS::PublisherQos                   defaultPublisherQos;
            DDS::SubscriberQos                  defaultSubscriberQos;
            DDS::TopicQos                       defaultTopicQos;
            DDS::OpenSplice::Subscriber         *builtinSubscriber;
            DDS::OpenSplice::ObjSet             *publisherList;
            DDS::OpenSplice::ObjSet             *subscriberList;
            DDS::OpenSplice::ObjSet             *topicList;
            DDS::OpenSplice::ObjSet             *cfTopicList;
            DDS::OpenSplice::ObjSet             *multiTopicList;
            DDS::OpenSplice::ObjSet             *builtinTopicList;
            DDS::OpenSplice::StrObjMap          *typeMetaHolders;
            DDS::Boolean                        factoryAutoEnable;
            DDS::StatusMask                     listenerInterest;

        protected:
            /*
             * These functions will be called by the friendly ::DDS::DomainParticipantFactory class.
             */
            DomainParticipant();

            virtual ~DomainParticipant();

            virtual ::DDS::ReturnCode_t
            init(
                    DDS::DomainParticipantFactory_ptr factory,
                    DDS::DomainId_t domainId,
                    const DDS::DomainParticipantQos &qos);

            DDS::ReturnCode_t nlReq_init(
                    DDS::DomainParticipantFactory_ptr factory,
                    DDS::DomainId_t domainId,
                    const DDS::DomainParticipantQos &qos);

            virtual DDS::ReturnCode_t wlReq_deinit();

            DDS::ReturnCode_t
            nlReq_builtinTopicRegisterTypeSupport();

            DDS::Topic_ptr
            find_builtin_topic (
                    const char *topic_name
            ) THROW_ORB_EXCEPTIONS;

        public:
            virtual ::DDS::Publisher_ptr create_publisher (
                    const ::DDS::PublisherQos &qos,
                    ::DDS::PublisherListener_ptr a_listener,
                    ::DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_publisher (
                    ::DDS::Publisher_ptr p
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::Subscriber_ptr create_subscriber (
                    const ::DDS::SubscriberQos &qos,
                    ::DDS::SubscriberListener_ptr a_listener,
                    ::DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_subscriber (
                    ::DDS::Subscriber_ptr s
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::Subscriber_ptr get_builtin_subscriber (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::Topic_ptr create_topic (
                    const char * topic_name,
                    const char * type_name,
                    const ::DDS::TopicQos &qos,
                    ::DDS::TopicListener_ptr a_listener,
                    ::DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_topic (
                    ::DDS::Topic_ptr a_topic
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::Topic_ptr find_topic (
                    const char * topic_name,
                    const ::DDS::Duration_t &timeout
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::TopicDescription_ptr lookup_topicdescription (
                    const char * name
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ContentFilteredTopic_ptr create_contentfilteredtopic (
                    const char * name,
                    ::DDS::Topic_ptr related_topic,
                    const char * filter_expression,
                    const ::DDS::StringSeq &filter_parameters
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_contentfilteredtopic (
                    ::DDS::ContentFilteredTopic_ptr a_contentfilteredtopic
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::MultiTopic_ptr create_multitopic (
                    const char * name,
                    const char * type_name,
                    const char * subscription_expression,
                    const ::DDS::StringSeq &expression_parameters
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_multitopic (
                    ::DDS::MultiTopic_ptr a_multitopic
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_contained_entities (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t set_qos (
                    const ::DDS::DomainParticipantQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_qos (
                    ::DDS::DomainParticipantQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t set_listener (
                    ::DDS::DomainParticipantListener_ptr a_listener,
                    ::DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::DomainParticipantListener_ptr get_listener (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t ignore_participant (
                    ::DDS::InstanceHandle_t handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t ignore_topic (
                    ::DDS::InstanceHandle_t handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t ignore_publication (
                    ::DDS::InstanceHandle_t handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t ignore_subscription (
                    ::DDS::InstanceHandle_t handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::DomainId_t get_domain_id (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t assert_liveliness (
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t set_default_publisher_qos (
                    const ::DDS::PublisherQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_default_publisher_qos (
                    ::DDS::PublisherQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t set_default_subscriber_qos (
                    const ::DDS::SubscriberQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_default_subscriber_qos (
                    ::DDS::SubscriberQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t set_default_topic_qos (
                    const ::DDS::TopicQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_default_topic_qos (
                    ::DDS::TopicQos &qos
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_discovered_participants (
                    ::DDS::InstanceHandleSeq &participant_handles
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_discovered_participant_data (
                    ::DDS::ParticipantBuiltinTopicData &participant_data,
                    ::DDS::InstanceHandle_t participant_handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_discovered_topics (
                    ::DDS::InstanceHandleSeq &topic_handles
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_discovered_topic_data (
                    ::DDS::TopicBuiltinTopicData &topic_data,
                    ::DDS::InstanceHandle_t topic_handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::Boolean contains_entity (
                    ::DDS::InstanceHandle_t a_handle
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t get_current_time (
                    ::DDS::Time_t &current_time
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t delete_historical_data (
                    const char * partition_expression,
                    const char * topic_expression
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t nlReq_load_type_support_meta_holder (
                    ::DDS::OpenSplice::TypeSupportMetaHolder *meta_holder,
                    const ::DDS::Char *type_name
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t wlReq_load_type_support_meta_holder (
                    ::DDS::OpenSplice::TypeSupportMetaHolder *meta_holder,
                    const ::DDS::Char *type_name
            ) THROW_ORB_EXCEPTIONS;

            virtual ::DDS::ReturnCode_t nlReq_find_type_support_meta_holder (
                    const ::DDS::Char *type_name,
                    ::DDS::OpenSplice::TypeSupportMetaHolder *&factory
            ) THROW_ORB_EXCEPTIONS;

            virtual void
            nlReq_notify_listener(
                    ::DDS::OpenSplice::Entity *sourceEntity,
                    ::DDS::ULong               triggerMask,
                    void                      *eventData
            ) THROW_ORB_EXCEPTIONS;

        private:
            ::DDS::ReturnCode_t
             wlReq_deleteContainedEntities();

            static DDS::Boolean
            rlReq_checkHandlePublisher(DDS::Object_ptr object, ::DDS::InstanceHandle_t *hdl);

            static DDS::Boolean
            rlReq_checkHandleSubscriber(DDS::Object_ptr object, ::DDS::InstanceHandle_t *hdl);

            static DDS::Boolean
            rlReq_checkHandleTopic(DDS::Object_ptr object, DDS::InstanceHandle_t *hdl);

            static DDS::Boolean
            wlReq_deinitTypeMetaHolders(const char *key, DDS::Object_ptr element, DDS::ReturnCode_t *result);

            struct findObjectArg
            {
                const char  *key;
                DDS::Object *match;
            };

            static DDS::Boolean
            rlReq_fnFindTopicDescription(DDS::Object_ptr td, findObjectArg *arg);

            static void
            rlReq_getuTopicType(v_public uTopic, void *arg);

            static DDS::Boolean
            rlReq_fnFindMetaHolderByInternalTypeName(const char *key, DDS::Object_ptr mh, struct findObjectArg *arg);

            DDS::OpenSplice::TopicDescription *
            rlReq_findTopicDescription(const char *topicName);

            const DDS::OpenSplice::TypeSupportMetaHolder *
            wlReq_insertMetaHolder(const char *typeName, DDS::OpenSplice::TypeSupportMetaHolder *tsMetaHolder);

            DDS::OpenSplice::TypeSupportMetaHolder *
            rlReq_findMetaHolderByInternalTypeName(const char *typeName);

            DDS::OpenSplice::TypeSupportMetaHolder *
            rlReq_findMetaHolder(const char *typeName);

            DDS::Boolean
            wlReq_insertPublisher(DDS::OpenSplice::Publisher *pub);

            DDS::Boolean
            wlReq_removePublisher(DDS::OpenSplice::Publisher *pub);

            DDS::Boolean
            wlReq_insertSubscriber(DDS::OpenSplice::Subscriber *sub);

            DDS::Boolean
            wlReq_removeSubscriber(DDS::OpenSplice::Subscriber *sub);

            static void
            nlReq_initBuiltinDataReaderQos(DDS::DataReaderQos *rQos);

            static void
            nlReq_initBuiltinTopicQos(DDS::TopicQos *tQos);

            static void
            nlReq_initBuiltinSubscriberQos(DDS::SubscriberQos *sQos);

            DDS::ReturnCode_t
            nlReq_getDiscoveredEntities(const char *topic_name,
                                        const char *type_name,
                                        DDS::InstanceHandleSeq &handles);

            template<typename TYPE, typename TYPE_SEQ, typename TYPE_READER>
            DDS::ReturnCode_t
            nlReq_getDiscoveredData(const char *topic_name,
                                    const char *type_name,
                                    TYPE &data,
                                    DDS::InstanceHandle_t handle);

            DDS::OpenSplice::TopicDescription *
            rlReq_isCDRProxyForExistingTopic(const char * topic_name,
                                                   const char * type_name);

            DDS::Topic_ptr
            nlReq_createTopic(const char * topic_name,
                              const char * type_name,
                              const DDS::TopicQos &qos,
                              DDS::TopicListener_ptr a_listener,
                              DDS::StatusMask mask,
                              DDS::OpenSplice::ObjSet &dest_list);

            DDS::DataReader_ptr
            nlReq_createBuiltinDataReader(DDS::Subscriber_ptr sub,
                                          const char *topic_name,
                                          const char *type_name);

            DDS::ReturnCode_t
            wlReq_deleteBuiltinSubscriber();

            os_char*
            rlReq_getChildName(const char* prefix);

            DDS::ReturnCode_t
            set_property (
                const ::DDS::Property & a_property
            );

            DDS::ReturnCode_t
            get_property (
                ::DDS::Property & a_property
            );

        }; /* class DomainParticipant */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_DOMAINPARTICIPANT_H */
