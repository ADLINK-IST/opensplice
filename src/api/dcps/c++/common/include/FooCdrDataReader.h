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
#ifndef CPP_DDS_OPENSPLICE_FOOCDRDATAREADER_H
#define CPP_DDS_OPENSPLICE_FOOCDRDATAREADER_H

#include "FooDataReader_impl.h"
#include "cpp_dcps_if.h"

namespace DDS {

    class CDRSample;

    namespace OpenSplice {

        class OS_API FooCdrDataReader
        {
        public:
            FooCdrDataReader(DDS::DataReader *rd);
            ~FooCdrDataReader();

            ::DDS::ReturnCode_t read_cdr(
                ::DDS::CDRSample & received_data,
                ::DDS::SampleInfo & info_seq,
                ::DDS::SampleStateMask sample_states,
                ::DDS::ViewStateMask view_states,
                ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t take_cdr (
                ::DDS::CDRSample & received_data,
                ::DDS::SampleInfo & info_seq,
                ::DDS::SampleStateMask sample_states,
                ::DDS::ViewStateMask view_states,
                ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        private:
            DDS::OpenSplice::FooDataReader_impl *reader;
        }; /* class FooCdrDataReader */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_FOOCDRDATAREADER_H */
