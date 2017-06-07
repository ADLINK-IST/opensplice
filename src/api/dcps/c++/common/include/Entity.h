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
#ifndef CPP_ENTITY_H
#define CPP_ENTITY_H

#include "CppSuperClass.h"
#include "StatusCondition.h"
#include "cpp_dcps_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(u_entity);
OS_CLASS(cmn_listenerDispatcher);

namespace DDS
{
    namespace OpenSplice
    {
        class ListenerDispatcher;

        class OS_API Entity
            : public virtual DDS::Entity,
              public DDS::OpenSplice::CppSuperClass
        {
        friend class DDS::OpenSplice::StatusCondition;
        friend class DDS::OpenSplice::ListenerDispatcher;

        private:
            u_entity                             uEntity;
            DDS::StatusMask                      interest;
            DDS::OpenSplice::StatusCondition    *statusCondition;
            DDS::InstanceHandle_t                handle;
            cmn_listenerDispatcher               listenerDispatcher;
            bool                                 listenerEnabled;

        protected:
            Entity(ObjectKind kind);

            virtual ~Entity();

            /* This function should be called from within a claim/release section. */
            DDS::ReturnCode_t
            nlReq_init(u_entity uEntity);

            /* This function should be called from within a claim/release section. */
            virtual DDS::ReturnCode_t
            wlReq_deinit();

            /* This function should be called from within a claim/release section. */
            DDS::Boolean
            rlReq_is_enabled();

            /* This function should be called from within a claim/release section. */
            u_entity
            rlReq_get_user_entity();

            DDS::ReturnCode_t
            wlReq_set_listener_mask (
                DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_listener_mask (
                DDS::StatusMask mask
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            disable_callbacks();

            DDS::Listener_ptr listener;
            DDS::StatusMask   listenerMask;
            os_int64 maxSupportedSeconds;

            virtual void
            nlReq_notify_listener(
                DDS::OpenSplice::Entity *sourceEntity,
                DDS::ULong               triggerMask,
                void                    *eventData
            ) = 0;

        public:
            virtual DDS::ReturnCode_t
            enable (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::StatusCondition_ptr
            get_statuscondition (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::StatusMask
            get_status_changes (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::InstanceHandle_t
            get_instance_handle (
            ) THROW_ORB_EXCEPTIONS;

            virtual void
            wlReq_set_listenerDispatcher (
                cmn_listenerDispatcher listenerDispatcher
            ) THROW_ORB_EXCEPTIONS;

            virtual cmn_listenerDispatcher
            rlReq_get_listenerDispatcher (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            nlReq_set_listener (
                DDS::Listener_ptr listener,
                DDS::StatusMask   mask
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            wlReq_set_listener (
                DDS::Listener_ptr listener,
                DDS::StatusMask   mask
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::Listener_ptr
            nlReq_get_listener (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            reset_dataAvailable_status (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            reset_on_data_on_readers_status (
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            set_property (
                const ::DDS::Property & a_property
            ) THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            get_property (
                DDS::Property & a_property
            ) THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            nlReq_notify_listener_removed();

            DDS::ReturnCode_t
            wlReq_wait_listener_removed();

            /*
             * This function should only be used by DBTs.
             */
            u_entity
            nlReq_get_user_entity_for_test (
            ) THROW_ORB_EXCEPTIONS;

        }; /* class Entity */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_ENTITY_H */
