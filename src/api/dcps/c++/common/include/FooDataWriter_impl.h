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
#ifndef CPP_DDS_OPENSPLICE_FOODATAWRITER_IMPL_H
#define CPP_DDS_OPENSPLICE_FOODATAWRITER_IMPL_H

#include "u_types.h"
#include "DataWriter.h"
#include "TypeSupportMetaHolder.h"
#include "cpp_dcps_if.h"

namespace DDS {
    namespace OpenSplice {

        class OS_API FooDataWriter_impl
            : public DDS::OpenSplice::DataWriter
        {
            friend class DDS::OpenSplice::TypeSupportMetaHolder;

        private:
            DDS::OpenSplice::cxxCopyIn copyIn;
            DDS::OpenSplice::cxxCopyOut copyOut;
            u_writerCopy writerCopy;
            void *cdrMarshaler;
            DDS::OpenSplice::DomainParticipant *participant;

            static v_copyin_result
            rlReq_copyIn (
                c_type type,
                void *data,
                void *to);

            static v_copyin_result
            rlReq_cdrCopyIn (
                c_type type,
                void *data,
                void *to);

        protected:
            FooDataWriter_impl();
            ~FooDataWriter_impl();

            DDS::ReturnCode_t
            nlReq_init(
                DDS::OpenSplice::Publisher *publisher,
                DDS::OpenSplice::DomainParticipant *participant,
                const DDS::DataWriterQos &qos,
                DDS::OpenSplice::Topic *a_topic,
                const char *name,
                DDS::OpenSplice::cxxCopyIn copyIn,
                DDS::OpenSplice::cxxCopyOut copyOut,
                u_writerCopy writerCopy,
                void *cdrMarshaler);

            virtual DDS::ReturnCode_t
            wlReq_deinit();

            ::DDS::InstanceHandle_t
            register_instance(
                const void * instance_data) THROW_ORB_EXCEPTIONS;

            ::DDS::InstanceHandle_t
            register_instance_w_timestamp(
                const void * instance_data,
                const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            unregister_instance(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            unregister_instance_w_timestamp(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle,
                const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            write(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            write_w_timestamp(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle,
                const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            dispose(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            dispose_w_timestamp(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle,
                const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            writedispose(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            writedispose_w_timestamp(
                const void * instance_data,
                ::DDS::InstanceHandle_t handle,
                const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

            ::DDS::ReturnCode_t
            get_key_value(
                void * key_holder,
                ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

            ::DDS::InstanceHandle_t
            lookup_instance (
                const void * instance_data) THROW_ORB_EXCEPTIONS;
        };
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_FOODATAWRITER_IMPL_H */
