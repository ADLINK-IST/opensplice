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
#ifndef CPP_DDS_OPENSPLICE_FOODATAREADERVIEW_IMPL_H
#define CPP_DDS_OPENSPLICE_FOODATAREADERVIEW_IMPL_H

#include "DataReaderView.h"
#include "DataReader.h"
#include "FooDataReader_impl.h"
#include "TypeSupportMetaHolder.h"
#include "cpp_dcps_if.h"

namespace DDS {
    namespace OpenSplice {

    class OS_API FooDataReaderView_impl
        : public DDS::OpenSplice::DataReaderView
        {
        friend class DDS::OpenSplice::ReadCondition;
        friend class DDS::OpenSplice::QueryCondition;
        protected:
        typedef void *(*cxxDataSeqAlloc) (void *, DDS::ULong);
        typedef void (*cxxDataSeqLength) (void *, DDS::ULong);
        typedef void (*cxxDataSeqCopyOut)(void *, void *, DDS::ULong);

        FooDataReaderView_impl ();
       ~FooDataReaderView_impl ();

        DDS::ReturnCode_t nlReq_init (
            DDS::OpenSplice::DataReader *reader,
            const char *name,
            const DDS::DataReaderViewQos &qos,
            DDS::OpenSplice::cxxCopyIn copyIn,
            DDS::OpenSplice::cxxCopyOut copyOut,
            cxxDataSeqAlloc dataSeqAlloc,
            cxxDataSeqLength dataSeqLength);

        virtual DDS::ReturnCode_t wlReq_deinit ();

        ::DDS::ReturnCode_t read (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_sample (
            void * received_data,
            ::DDS::SampleInfo & sample_info
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_sample (
            void * received_data,
            ::DDS::SampleInfo & sample_info
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance_internal (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance_internal (
             void * received_data,
             ::DDS::SampleInfoSeq & info_seq,
             DDS::Long max_samples,
             ::DDS::InstanceHandle_t a_handle,
             ::DDS::SampleStateMask sample_states,
             ::DDS::ViewStateMask view_states,
             ::DDS::InstanceStateMask instance_states
         ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t wlReq_return_loan (
            void *data_buffer,
            void *info_buffer) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t get_key_value (
            void * key_holder,
            ::DDS::InstanceHandle_t handle
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::InstanceHandle_t lookup_instance (
            const void * instance
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t flush (
            void * samplesList,
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq);

        private:
        struct Implementation;
        Implementation *pimpl;
        };
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_FOODATAREADERVIEW_IMPL_H */
