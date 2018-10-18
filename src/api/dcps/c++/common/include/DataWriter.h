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
#ifndef CPP_DDS_OPENSPLICE_DATAWRITER_H
#define CPP_DDS_OPENSPLICE_DATAWRITER_H

#include "u_user.h"
#include "Entity.h"
#include "Publisher.h"
#include "TypeSupportMetaHolder.h"
#include "cpp_dcps_if.h"

namespace DDS {
    namespace OpenSplice {

        class Publisher;

        class OS_API DataWriter
            : public virtual DDS::DataWriter,
              public DDS::OpenSplice::Entity
        {
            friend class ::DDS::OpenSplice::Publisher;

        private:
            DDS::OpenSplice::Publisher *publisher;
            DDS::OpenSplice::Topic *topic;

            static u_result copy_liveliness_lost_status(
                u_status info,
                void *arg);

            static u_result copy_deadline_missed_status(
                u_status info,
                void *arg);

            static u_result copy_incompatible_qos_status(
                u_status info,
                void *arg);

            static u_result copy_publication_matched_status(
                u_status info,
                void *arg);

            static v_result copy_matched_subscription(
                u_subscriptionInfo *info,
                void *arg);

            static v_result copy_matched_subscription_data(
                u_subscriptionInfo *info,
                void *arg);

        protected:
            DataWriter ();

            virtual ~DataWriter ();

            virtual DDS::ReturnCode_t
            init(
                DDS::OpenSplice::Publisher *publisher,
                DDS::OpenSplice::DomainParticipant *participant,
                const DDS::DataWriterQos &qos,
                DDS::OpenSplice::Topic *a_topic,
                const char *name,
                DDS::OpenSplice::cxxCopyIn copyIn,
                DDS::OpenSplice::cxxCopyOut copyOut,
                u_writerCopy writerCopy,
                void *cdrMarshaler) = 0;

            DDS::ReturnCode_t
            nlReq_init(
                DDS::OpenSplice::Publisher *publisher,
                const DDS::DataWriterQos &qos,
                DDS::OpenSplice::Topic *a_topic,
                const char *name);

            virtual DDS::ReturnCode_t
            wlReq_deinit();

        public:
            DDS::ReturnCode_t
            set_qos (
                const DataWriterQos &qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_qos (
                DDS::DataWriterQos &qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_listener (
                DDS::DataWriterListener_ptr a_listener,
                DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            DDS::DataWriterListener_ptr
            get_listener (
            ) THROW_ORB_EXCEPTIONS;

            DDS::Topic_ptr
            get_topic (
            ) THROW_ORB_EXCEPTIONS;

            DDS::Publisher_ptr
            get_publisher (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            wait_for_acknowledgments (
                const Duration_t &max_wait
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_liveliness_lost_status (
                LivelinessLostStatus &status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_offered_deadline_missed_status (
                OfferedDeadlineMissedStatus &status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_offered_incompatible_qos_status (
                OfferedIncompatibleQosStatus &status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_publication_matched_status (
                PublicationMatchedStatus &status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            assert_liveliness (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_matched_subscriptions (
                InstanceHandleSeq& subscription_handles
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_matched_subscription_data (
                SubscriptionBuiltinTopicData &subscription_data,
                InstanceHandle_t subscription_handle
            ) THROW_ORB_EXCEPTIONS;

            virtual void
            nlReq_notify_listener(
                DDS::OpenSplice::Entity *sourceEntity,
                DDS::ULong               triggerMask,
                void                    *eventData
            ) THROW_ORB_EXCEPTIONS;

        }; /* class Publisher */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_DATAWRITER_H */
