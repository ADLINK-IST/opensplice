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
#ifndef CPP_DDS_OPENSPLICE_GUARDCONDITION_H
#define CPP_DDS_OPENSPLICE_GUARDCONDITION_H

#include "CppSuperClass.h"
#include "Condition.h"
#include "cpp_dcps_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    class OS_API GuardCondition
        : public virtual DDS::GuardConditionInterface,
          public DDS::OpenSplice::Condition
    {
    friend class DDS::WaitSet;

    private:
        DDS::Boolean triggerValue;

    protected:
        virtual DDS::ReturnCode_t
        init();

        DDS::ReturnCode_t
        nlReq_init();

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
        GuardCondition();

        ~GuardCondition();

        virtual ::DDS::Boolean get_trigger_value () THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t set_trigger_value (DDS::Boolean value) THROW_ORB_EXCEPTIONS;
    };

    typedef GuardConditionInterface_ptr GuardCondition_ptr;
    typedef GuardConditionInterface_var GuardCondition_var;
    typedef GuardConditionInterface_out GuardCondition_out;
} /* namespace DDS */
#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_GUARDCONDITION_H */
