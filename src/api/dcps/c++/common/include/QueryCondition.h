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
#ifndef CPP_DDS_OPENSPLICE_QUERYCONDITION_H
#define CPP_DDS_OPENSPLICE_QUERYCONDITION_H

#include "Entity.h"
#include "DataReader.h"
#include "CppSuperClass.h"
#include "Condition.h"
#include "ReadCondition.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
namespace DDS
{
    namespace OpenSplice
    {
        class OS_API QueryCondition
                : public virtual ::DDS::QueryCondition,
                  public ::DDS::OpenSplice::ReadCondition
        {
            friend class DDS::OpenSplice::Entity;
            friend class DDS::WaitSet;
            friend class DDS::OpenSplice::DataReader;
            friend class DDS::OpenSplice::DataReaderView;

            private:

            protected:
            char            *query_expression;
            DDS::StringSeq  query_parameters;

                QueryCondition();
                ~QueryCondition();

                virtual DDS::ReturnCode_t
                init (DDS::OpenSplice::Entity *,
                        DDS::SampleStateMask,
                        DDS::ViewStateMask,
                        DDS::InstanceStateMask,
                        const char *queryExpression,
                        const DDS::StringSeq *queryParameters);

                DDS::ReturnCode_t
                nlReq_init (DDS::OpenSplice::Entity *,
                        DDS::SampleStateMask sample_states,
                        DDS::ViewStateMask view_states,
                        DDS::InstanceStateMask instance_states,
                        const char *queryExpression,
                        const DDS::StringSeq *queryParameters);

                virtual DDS::ReturnCode_t
                wlReq_deinit();

                DDS::ReturnCode_t
                read (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    void*);

                DDS::ReturnCode_t
                take (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    void*);

                DDS::ReturnCode_t
                read_next_instance (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    DDS::InstanceHandle_t,
                    void*);

                DDS::ReturnCode_t
                take_next_instance (
                    DDS::OpenSplice::Entity *,
                    void *,
                    DDS::SampleInfoSeq &,
                    const long,
                    DDS::InstanceHandle_t,
                    void*);

            public:
                char * get_query_expression (
                  ) THROW_ORB_EXCEPTIONS;

                DDS::ReturnCode_t get_query_parameters (
                    DDS::StringSeq & query_parameters
                ) THROW_ORB_EXCEPTIONS;

                DDS::ReturnCode_t set_query_parameters (
                  const DDS::StringSeq & query_parameters
                ) THROW_ORB_EXCEPTIONS;

        };
    }; /* namespace OpenSplice */
}; /* namespace DDS */
#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_QUERYCONDITION_H */
