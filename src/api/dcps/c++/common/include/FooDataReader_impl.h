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
#ifndef CPP_DDS_OPENSPLICE_FOODATAREADER_IMPL_H
#define CPP_DDS_OPENSPLICE_FOODATAREADER_IMPL_H

#include "DataReader.h"
#include "TypeSupportMetaHolder.h"
#include "cpp_dcps_if.h"

namespace DDS {
    namespace OpenSplice {

    class FooDataReaderView_impl;
    class FooCdrDataReader;

    class OS_API FooDataReader_impl
        : public DDS::OpenSplice::DataReader
        {
        friend class DDS::OpenSplice::FooDataReaderView_impl;
        friend class DDS::OpenSplice::TypeSupportMetaHolder;
        friend class DDS::OpenSplice::ReadCondition;
        friend class DDS::OpenSplice::QueryCondition;
        friend class DDS::OpenSplice::FooCdrDataReader;

        public:
        typedef void *(*cxxDataSeqAlloc) (void *, DDS::ULong);
        typedef void (*cxxDataSeqLength) (void *, DDS::ULong);
        typedef void *(*cxxDataSeqGetBuffer) (void *, DDS::ULong);
        typedef void (*cxxCopyDataOut)(const void *, void *);

        protected:
        FooDataReader_impl ();
       ~FooDataReader_impl ();

        DDS::ReturnCode_t
        nlReq_init (
            DDS::OpenSplice::Subscriber *subscriber,
            const DDS::DataReaderQos &qos,
            DDS::OpenSplice::TopicDescription *a_topic,
            const char *name,
            DDS::OpenSplice::cxxCopyIn copyIn,
            DDS::OpenSplice::cxxCopyOut copyOut,
            DDS::OpenSplice::cxxReaderCopy readerCopy,
            void *cdrMarshaler,
            cxxDataSeqAlloc dataSeqAlloc,
            cxxDataSeqLength dataSeqLength,
            cxxDataSeqGetBuffer dataSeqGetBuffer,
            cxxCopyDataOut copyDataOut);

        virtual DDS::ReturnCode_t
        wlReq_deinit ();

        ::DDS::ReturnCode_t read (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_sample (
            void * received_data,
            ::DDS::SampleInfo & sample_info) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_sample (
            void * received_data,
            ::DDS::SampleInfo & sample_info) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance_internal (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance_internal (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance (
             void * received_data,
             ::DDS::SampleInfoSeq & info_seq,
             ::DDS::Long max_samples,
             ::DDS::InstanceHandle_t a_handle,
             ::DDS::SampleStateMask sample_states,
             ::DDS::ViewStateMask view_states,
             ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance_w_condition (
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq,
            ::DDS::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition) THROW_ORB_EXCEPTIONS;

        DDS::ReturnCode_t wlReq_return_loan (
            void *data_buffer,
            void *info_buffer) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t get_key_value (
            void * key_holder,
            ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

        ::DDS::InstanceHandle_t lookup_instance (
            const void * instance) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_cdr(
            void * received_data,
            ::DDS::SampleInfo & info_seq,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_cdr (
            void * received_data,
            ::DDS::SampleInfo & info_seq,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states) THROW_ORB_EXCEPTIONS;

        DDS::Long
        rlReq_get_workers();

        DDS::ReturnCode_t
        wlReq_set_workers(DDS::Long value);

        DDS::ReturnCode_t
        wlReq_set_ignoreOpenLoansAtDeletionStatus(DDS::Boolean value);

        DDS::Boolean
        rlReq_get_ignoreOpenLoansAtDeletionStatus();

        ::DDS::ReturnCode_t flush (
            void * samplesList,
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq);

        ::DDS::ReturnCode_t flush_cdr (
            void * samplesList,
            void * received_data,
            ::DDS::SampleInfo & info);

        ::DDS::ReturnCode_t actualFlush (
            void * samplesList,
            void * received_data,
            ::DDS::SampleInfoSeq & info_seq);

        static void copySampleOut (
            void *sample,
            void *info,
            void *arg);

        static void copyCDRSampleOut (
            void *sample,
            void * info,
            void *arg);

        private:
        struct Implementation;
        Implementation *pimpl;

        ::DDS::ReturnCode_t
             wlReq_init_cdr();

        };
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_FOODATAREADER_IMPL_H */
