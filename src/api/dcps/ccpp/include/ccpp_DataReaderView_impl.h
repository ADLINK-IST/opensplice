/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef CCPP_DATAREADERVIEW_H
#define CCPP_DATAREADERVIEW_H


#include "ccpp.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_DataReader_impl.h"
#include "gapi.h"
#include "gapi_loanRegistry.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    class DataReader_impl;

    class OS_DCPS_API DataReaderView_impl
        : public virtual ::DDS::DataReaderView,
          public ::DDS::Entity_impl
    {
            friend class ::DDS::DataReader_impl;

    protected:
        os_mutex drv_mutex;
        DataReaderView_impl(gapi_dataReaderView handle);
       ~DataReaderView_impl();

        ::DDS::ReturnCode_t read (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_sample (
            void * data_values,
            ::DDS::SampleInfo & sample_info
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_sample (
            void * data_values,
            ::DDS::SampleInfo & sample_info
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t read_next_instance_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t take_next_instance_w_condition (
            void * data_values,
            ::DDS::SampleInfoSeq & info_seq,
            CORBA::Long max_samples,
            ::DDS::InstanceHandle_t a_handle,
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t return_loan (
            void *dataBuf,
            void *infoBuf
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t get_key_value (
            void * key_holder,
            ::DDS::InstanceHandle_t handle
        ) THROW_ORB_EXCEPTIONS;

        ::DDS::InstanceHandle_t lookup_instance (
            const void * instance
        ) THROW_ORB_EXCEPTIONS;

    public:
        virtual ::DDS::ReadCondition_ptr
        create_readcondition (
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::QueryCondition_ptr
        create_querycondition (
            ::DDS::SampleStateMask sample_states,
            ::DDS::ViewStateMask view_states,
            ::DDS::InstanceStateMask instance_states,
            const char * query_expression,
            const ::DDS::StringSeq & query_parameters
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        delete_readcondition (
            ::DDS::ReadCondition_ptr a_condition
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        delete_contained_entities (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_qos (
            const ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_qos (
            ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::StatusCondition_ptr
        get_statuscondition(
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::StatusMask get_status_changes(
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DataReader_ptr get_datareader(
        ) THROW_ORB_EXCEPTIONS;

    };
    typedef DataReaderView_impl* DataReaderView_impl_ptr;

}

#endif /* CCPP_DATAREADER */
