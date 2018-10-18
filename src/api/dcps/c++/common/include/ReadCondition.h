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
#ifndef CPP_DDS_OPENSPLICE_READCONDITION_H
#define CPP_DDS_OPENSPLICE_READCONDITION_H

#include "Entity.h"
#include "DataReader.h"
#include "DataReaderView.h"
#include "FooDataReader_impl.h"
#include "FooDataReaderView_impl.h"
#include "CppSuperClass.h"
#include "cpp_dcps_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    namespace OpenSplice
    {
        class OS_API ReadCondition
            : public virtual ::DDS::ReadCondition,
              public ::DDS::OpenSplice::Condition
        {
            friend class DDS::OpenSplice::Entity;
            friend class DDS::WaitSet;
            friend class DDS::OpenSplice::DataReader;
            friend class DDS::OpenSplice::DataReaderView;
            friend class DDS::OpenSplice::FooDataReader_impl;
            friend class DDS::OpenSplice::FooDataReaderView_impl;


            private:

            protected:
                DDS::SampleStateMask sample_states;
                DDS::ViewStateMask view_states;
                DDS::InstanceStateMask instance_states;
                DDS::OpenSplice::Entity *reader;

                u_query uQuery;

                ReadCondition();
                ReadCondition(DDS::OpenSplice::ObjectKind);

                ~ReadCondition();

                DDS::ReturnCode_t
                init (DDS::OpenSplice::Entity *,
                        DDS::SampleStateMask,
                        DDS::ViewStateMask,
                        DDS::InstanceStateMask);

                DDS::ReturnCode_t
                nlReq_init (DDS::OpenSplice::Entity *,
                        DDS::SampleStateMask,
                        DDS::ViewStateMask,
                        DDS::InstanceStateMask);

                DDS::ReturnCode_t
                nlReq_init (DDS::OpenSplice::DataReaderView *,
                        DDS::SampleStateMask,
                        DDS::ViewStateMask,
                        DDS::InstanceStateMask);

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

                static unsigned char
                test_sample_states (
                    c_object o,
                    c_voidp args);

                u_query
                get_user_query();

                virtual DDS::ReturnCode_t
                read (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    void*);


                virtual DDS::ReturnCode_t
                take (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    void*);

                virtual DDS::ReturnCode_t
                read_next_instance (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    DDS::InstanceHandle_t,
                    void*);

                virtual DDS::ReturnCode_t
                take_next_instance (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    DDS::InstanceHandle_t,
                    void*);

                virtual DDS::ReturnCode_t
                isAlive();

            public:

                DDS::Boolean
                get_trigger_value() THROW_ORB_EXCEPTIONS;
                DDS::SampleStateMask
                get_sample_state_mask() THROW_ORB_EXCEPTIONS;

                DDS::ViewStateMask
                get_view_state_mask() THROW_ORB_EXCEPTIONS;

                DDS::InstanceStateMask
                get_instance_state_mask() THROW_ORB_EXCEPTIONS;

                DDS::DataReader_ptr
                get_datareader() THROW_ORB_EXCEPTIONS;

                DDS::DataReaderView_ptr
                get_datareaderview() THROW_ORB_EXCEPTIONS;

        };
    } /* namespace OpenSplice */
} /* namespace DDS */
#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_READCONDITION_H */
