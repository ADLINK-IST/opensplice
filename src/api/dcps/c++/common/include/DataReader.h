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
#ifndef CPP_DDS_OPENSPLICE_DATAREADER_H
#define CPP_DDS_OPENSPLICE_DATAREADER_H

#include "Entity.h"
#include "EntityContainer.h"
#include "Topic.h"
#include "TopicDescription.h"
#include "Subscriber.h"
#include "TypeSupportMetaHolder.h"
#include "cpp_dcps_if.h"

namespace DDS {
    namespace OpenSplice {

        class Subscriber;
        class DataReaderView;
        class ReadCondition;
        class QueryCondition;

        class OS_API DataReader :
            public virtual DDS::DataReader,
            public ::DDS::OpenSplice::EntityContainer,
            public ::DDS::OpenSplice::Entity
        {
            friend class ::DDS::OpenSplice::Subscriber;
            friend class ::DDS::OpenSplice::ReadCondition;
            friend class ::DDS::OpenSplice::QueryCondition;
            friend class ::DDS::OpenSplice::DataReaderView;

        protected:
            DataReader ();
            virtual ~DataReader ();

            virtual DDS::ReturnCode_t
            init (
                DDS::OpenSplice::Subscriber *subscriber,
                const DDS::DataReaderQos &qos,
                DDS::OpenSplice::TopicDescription *a_topic,
                const char *name,
                DDS::OpenSplice::cxxCopyIn copyIn,
                DDS::OpenSplice::cxxCopyOut copyOut) = 0;

            DDS::ReturnCode_t
            nlReq_init (
                DDS::OpenSplice::Subscriber *subscriber,
                const DDS::DataReaderQos &qos,
                DDS::OpenSplice::TopicDescription *a_topic,
                const char *name);

            virtual DDS::ReturnCode_t
            wlReq_deinit ();

        public:
            DDS::ReadCondition_ptr
            create_readcondition (
                SampleStateMask sample_states,
                ViewStateMask view_states,
                InstanceStateMask instance_states
            ) THROW_ORB_EXCEPTIONS;

            DDS::QueryCondition_ptr
            create_querycondition (
                SampleStateMask sample_states,
                ViewStateMask view_states,
                InstanceStateMask instance_states,
                const char* query_expression,
                const StringSeq& query_parameters
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            delete_readcondition (
                ReadCondition_ptr a_condition);

            DDS::ReturnCode_t
            delete_contained_entities (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_qos (
                const DataReaderQos& qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_qos (
                DataReaderQos& qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_listener (
                DataReaderListener_ptr a_listener,
                StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            DDS::DataReaderListener_ptr
            get_listener (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_property (
                const ::DDS::Property & a_property
            ) THROW_ORB_EXCEPTIONS;

            DDS::TopicDescription_ptr
            get_topicdescription (
            ) THROW_ORB_EXCEPTIONS;

            DDS::Subscriber_ptr
            get_subscriber (
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_sample_rejected_status (
                SampleRejectedStatus& status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_liveliness_changed_status (
                LivelinessChangedStatus& status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_requested_deadline_missed_status (
                RequestedDeadlineMissedStatus& status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_requested_incompatible_qos_status (
                RequestedIncompatibleQosStatus& status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_subscription_matched_status (
                SubscriptionMatchedStatus& status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_sample_lost_status (
                SampleLostStatus& status
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            wait_for_historical_data (
                const Duration_t& max_wait
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            wait_for_historical_data_w_condition (
                const char *,
                const DDS::StringSeq &,
                const DDS::Time_t &,
                const DDS::Time_t &,
                const DDS::ResourceLimitsQosPolicy &,
                const DDS::Duration_t &
            ) THROW_ORB_EXCEPTIONS;

            DDS::DataReaderView_ptr
            create_view (
                const DDS::DataReaderViewQos &qos
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            delete_view (
                DDS::DataReaderView_ptr
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_default_datareaderview_qos (
                const DDS::DataReaderViewQos&
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_default_datareaderview_qos (
                DDS::DataReaderViewQos&
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_matched_publications (
                InstanceHandleSeq &publication_handles
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            nlReq_get_instance_handles(
                DDS::InstanceHandleSeq &participant_handles
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            get_matched_publication_data (
                PublicationBuiltinTopicData &publication_data,
                InstanceHandle_t publication_handle
            ) THROW_ORB_EXCEPTIONS;

            virtual void
            nlReq_notify_listener(
                DDS::OpenSplice::Entity *sourceEntity,
                DDS::ULong               triggerMask,
                void                    *eventData
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::Long
            rlReq_get_workers() = 0;

            virtual DDS::ReturnCode_t
            wlReq_set_workers(DDS::Long value) = 0;

            virtual DDS::ReturnCode_t
            wlReq_set_ignoreOpenLoansAtDeletionStatus(DDS::Boolean value) = 0;

            virtual DDS::Boolean
            rlReq_get_ignoreOpenLoansAtDeletionStatus() = 0;


        private:
            DDS::OpenSplice::TopicDescription *
            get_topic ();

            static DDS::ReturnCode_t
            deinit_view (
                DDS::OpenSplice::DataReaderView * view);

            static DDS::ReturnCode_t
            deinit_condition (
                DDS::OpenSplice::ReadCondition * view);

            struct Implementation;
            Implementation *pimpl;
        }; /* class DataReader */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_DATAREADER_H */
