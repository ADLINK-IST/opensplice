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
#ifndef CCPP_DATAREADER_H
#define CCPP_DATAREADER_H

#include "ccpp.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_DataReaderView_impl.h"
#include "gapi.h"
#include "gapi_loanRegistry.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    class DataReaderView_impl;

    class OS_DCPS_API DataReader_impl
        : public virtual ::DDS::DataReader,
          public ::DDS::Entity_impl
    {
        friend class ::DDS::Subscriber_impl;

    protected:
        os_mutex dr_mutex;
        DataReader_impl(gapi_dataReader handle);
       ~DataReader_impl();

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
            const ::DDS::DataReaderQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_qos (
            ::DDS::DataReaderQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_listener (
            ::DDS::DataReaderListener_ptr a_listener,
            ::DDS::StatusMask mask
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DataReaderListener_ptr
        get_listener (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::TopicDescription_ptr
        get_topicdescription (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::Subscriber_ptr
        get_subscriber (
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_sample_rejected_status (
          ::DDS::SampleRejectedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_liveliness_changed_status (
          ::DDS::LivelinessChangedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_requested_deadline_missed_status (
          ::DDS::RequestedDeadlineMissedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_requested_incompatible_qos_status (
          ::DDS::RequestedIncompatibleQosStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_subscription_matched_status (
          ::DDS::SubscriptionMatchedStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t get_sample_lost_status (
          ::DDS::SampleLostStatus & a_status
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        wait_for_historical_data (
            const ::DDS::Duration_t & max_wait
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        wait_for_historical_data_w_condition (
            const char * filter_expression,
            const ::DDS::StringSeq & filter_parameters,
            const ::DDS::Time_t & min_source_timestamp,
            const ::DDS::Time_t & max_source_timestamp,
            const ::DDS::ResourceLimitsQosPolicy & resource_limits,
            const ::DDS::Duration_t & max_wait
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_matched_publications (
            ::DDS::InstanceHandleSeq & publication_handles
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_matched_publication_data (
            ::DDS::PublicationBuiltinTopicData & publication_data,
            ::DDS::InstanceHandle_t publication_handle
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::DataReaderView_ptr
        create_view (
            const ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        delete_view (
            ::DDS::DataReaderView_ptr a_view
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_default_datareaderview_qos (
          ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        set_default_datareaderview_qos (
          const ::DDS::DataReaderViewQos & qos
        ) THROW_ORB_EXCEPTIONS;

    };
    typedef DataReader_impl* DataReader_impl_ptr;

    template <class DataSeq, class DataType>
    void
    ccpp_DataReaderCopy (
        gapi_dataSampleSeq *samples,
        gapi_readerInfo    *info
    )
    {
        unsigned int i, len;
        DataSeq *data_seq = reinterpret_cast<DataSeq *>(info->data_buffer);
        ::DDS::SampleInfoSeq *info_seq = reinterpret_cast< ::DDS::SampleInfoSeq * >(info->info_buffer);

        if (samples)
        {
            len = samples->_length;
        }
        else
        {
            len = 0;
            data_seq->length(len);
            info_seq->length(len);
        }

        if ( (info->max_samples != (gapi_unsigned_long)GAPI_LENGTH_UNLIMITED) && (len > info->max_samples) ) {
            len = info->max_samples;
        }
        else if ( data_seq->maximum() > 0 && data_seq->maximum() < len ) {
            len = data_seq->maximum();
        }

        if ( len > 0 ) {
            if ( data_seq->maximum() == 0 ) {
                DataType *dataBuf = DataSeq::allocbuf(len);
                ::DDS::SampleInfo *infoBuf = ::DDS::SampleInfoSeq::allocbuf(len);
                data_seq->replace(len, len, dataBuf, false);
                info_seq->replace(len, len, infoBuf, false);
                if (*(info->loan_registry) == NULL) {
                    *(info->loan_registry) = gapi_loanRegistry_new();
                }
                gapi_loanRegistry_register((gapi_loanRegistry)*(info->loan_registry),
                                           dataBuf,
                                           infoBuf);
            }
            else
            {
                data_seq->length(len);
                info_seq->length(len);
            }

            for ( i = 0; i < len; i++ ) {
                info->copy_out ( samples->_buffer[i].data, &(*data_seq)[i] );
                ccpp_SampleInfo_copyOut( samples->_buffer[i].info, (*info_seq)[i] );
            }
        }
        info->num_samples = len;
    }

}

#endif /* CCPP_DATAREADER */
