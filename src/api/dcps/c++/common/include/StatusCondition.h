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
#ifndef CPP_DDS_OPENSPLICE_STATUSCONDITION_H
#define CPP_DDS_OPENSPLICE_STATUSCONDITION_H

#include "CppSuperClass.h"
#include "Condition.h"
#include "cpp_dcps_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    namespace OpenSplice
    {
        class Entity;

        class OS_API StatusCondition
            : public virtual ::DDS::StatusCondition,
              public ::DDS::OpenSplice::Condition
        {
        friend class DDS::OpenSplice::Entity;
        friend class DDS::WaitSet;

        private:
            u_statusCondition uCondition;
            DDS::OpenSplice::Entity *entity;
            DDS::StatusMask enabledStatusMask;

            /* Called by the WaitSet when an StatusCondition is attached or detached. */
            u_statusCondition
            get_user_object();

        protected:
            StatusCondition();

            virtual ~StatusCondition();

            virtual DDS::ReturnCode_t
            init(DDS::OpenSplice::Entity *entity);

            DDS::ReturnCode_t
            nlReq_init (DDS::OpenSplice::Entity *entity);

            virtual DDS::ReturnCode_t
            wlReq_deinit();

            virtual DDS::ReturnCode_t
            attachToWaitset (
                DDS::WaitSet *waitset);

            virtual DDS::ReturnCode_t
            wlReq_detachFromWaitset (
                DDS::WaitSet *waitset);

            virtual DDS::ReturnCode_t
            detachFromWaitset (
                DDS::WaitSet *waitset);

        public:
            DDS::Boolean
            get_trigger_value () THROW_ORB_EXCEPTIONS;

            DDS::StatusMask
            get_enabled_statuses () THROW_ORB_EXCEPTIONS;

            DDS::ReturnCode_t
            set_enabled_statuses (DDS::StatusMask mask) THROW_ORB_EXCEPTIONS;

            DDS::Entity_ptr
            get_entity () THROW_ORB_EXCEPTIONS;

            virtual DDS::ReturnCode_t
            isAlive();

        };

    } /* namespace OpenSplice */
} /* namespace DDS */
#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_STATUSCONDITION_H */
