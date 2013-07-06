/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "dds_dcps.h"
#include "sac_fooDataReader.h"
#include "gapi.h"

#include <string.h>
#include <assert.h>

#ifndef NULL
#define NULL 0
#endif

static const gapi_char *participantBuiltinTopicTypeName  = "DDS::ParticipantBuiltinTopicData";
static const gapi_char *topicBuiltinTopicTypeName        = "DDS::TopicBuiltinTopicData";
static const gapi_char *publicationBuiltinTopicTypeName  = "DDS::PublicationBuiltinTopicData";
static const gapi_char *subscriptionBuiltinTopicTypeName = "DDS::SubscriptionBuiltinTopicData";

DDS_ReturnCode_t
sac_builtinTopicRegisterTypeSupport (
    DDS_DomainParticipant participant)
{
    DDS_TypeSupport t;
    DDS_ReturnCode_t result;

    t = (DDS_TypeSupport)
        gapi_fooTypeSupport__alloc(
            (const gapi_char *)participantBuiltinTopicTypeName,
            (const gapi_char *)NULL,
            (const gapi_char *)NULL,
            (gapi_typeSupportLoad)NULL,
            (gapi_copyIn)gapi_participantBuiltinTopicData__copyIn,
            (gapi_copyOut)gapi_participantBuiltinTopicData__copyOut,
            (gapi_unsigned_long)(sizeof(DDS_ParticipantBuiltinTopicData)),
            (gapi_topicAllocBuffer)DDS_sequence_DDS_ParticipantBuiltinTopicData_allocbuf,
            (gapi_writerCopy)NULL,
            (gapi_readerCopy)NULL);
    
    if ( t ) {
        result = (DDS_ReturnCode_t)
            gapi_fooTypeSupport_register_type(
                (gapi_typeSupport)t,
                (gapi_domainParticipant)participant,
                (gapi_string)participantBuiltinTopicTypeName);
    } else {
        result = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if ( result == DDS_RETCODE_OK ) {
        t = (DDS_TypeSupport)
            gapi_fooTypeSupport__alloc(
                (const gapi_char *)topicBuiltinTopicTypeName,
                (const gapi_char *)NULL,
                (const gapi_char *)NULL,
                (gapi_typeSupportLoad)NULL,
                (gapi_copyIn)gapi_topicBuiltinTopicData__copyIn,
                (gapi_copyOut)gapi_topicBuiltinTopicData__copyOut,
                (gapi_unsigned_long)(sizeof(DDS_TopicBuiltinTopicData)),
                (gapi_topicAllocBuffer)DDS_sequence_DDS_TopicBuiltinTopicData_allocbuf,
                (gapi_writerCopy)NULL,
                (gapi_readerCopy)NULL);
    
        if ( t ) {
            result = (DDS_ReturnCode_t)
                gapi_fooTypeSupport_register_type(
                    (gapi_typeSupport)t,
                    (gapi_domainParticipant)participant,
                    (gapi_string)topicBuiltinTopicTypeName);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }
        
    if ( result == DDS_RETCODE_OK ) {
        t = (DDS_TypeSupport)
            gapi_fooTypeSupport__alloc(
                (const gapi_char *)publicationBuiltinTopicTypeName,
                (const gapi_char *)NULL,
                (const gapi_char *)NULL,
                (gapi_typeSupportLoad)NULL,
                (gapi_copyIn)gapi_publicationBuiltinTopicData__copyIn,
                (gapi_copyOut)gapi_publicationBuiltinTopicData__copyOut,
                (gapi_unsigned_long)(sizeof(DDS_PublicationBuiltinTopicData)),
                (gapi_topicAllocBuffer)DDS_sequence_DDS_PublicationBuiltinTopicData_allocbuf,
                (gapi_writerCopy)NULL,
                (gapi_readerCopy)NULL);
    
        if ( t ) {
            result = (DDS_ReturnCode_t)
                gapi_fooTypeSupport_register_type(
                    (gapi_typeSupport)t,
                    (gapi_domainParticipant)participant,
                    (gapi_string)publicationBuiltinTopicTypeName);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }

    if ( result == DDS_RETCODE_OK ) {
        t = (DDS_TypeSupport)
            gapi_fooTypeSupport__alloc(
                (const gapi_char *)subscriptionBuiltinTopicTypeName,
                (const gapi_char *)NULL,
                (const gapi_char *)NULL,
                (gapi_typeSupportLoad)NULL,
                (gapi_copyIn)gapi_subscriptionBuiltinTopicData__copyIn,
                (gapi_copyOut)gapi_subscriptionBuiltinTopicData__copyOut,
                (gapi_unsigned_long)(sizeof(DDS_SubscriptionBuiltinTopicData)),
                (gapi_topicAllocBuffer)DDS_sequence_DDS_SubscriptionBuiltinTopicData_allocbuf,
                (gapi_writerCopy)NULL,
                (gapi_readerCopy)NULL);
    
        if ( t ) {
            result = (DDS_ReturnCode_t)
                gapi_fooTypeSupport_register_type(
                    (gapi_typeSupport)t,
                    (gapi_domainParticipant)participant,
                    (gapi_string)subscriptionBuiltinTopicTypeName);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }

    return result;
}
    



DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader this,
    DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader this,
    DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_return_loan (
	    this,
	    (DDS_sequence)data_values,
	    info_seq
	);
    return result;
}

DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader this,
    DDS_ParticipantBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_get_key_value (
	    this,
	    (DDS_sample)key_holder,
	    handle
	);
    return result;
}

DDS_InstanceHandle_t
DDS_ParticipantBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader this,
    const DDS_ParticipantBuiltinTopicData *key_holder)
{
    DDS_InstanceHandle_t handle;

    handle = (DDS_InstanceHandle_t)
        DDS__FooDataReader_lookup_instance (
            this,
            (DDS_sample)key_holder
        );
    return handle;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader this,
    DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader this,
    DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_return_loan (
	    this,
	    (DDS_sequence)data_values,
	    info_seq
	);
    return result;
}

DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader this,
    DDS_TopicBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_get_key_value (
	    this,
	    (DDS_sample)key_holder,
	    handle
	);
    return result;
}

DDS_InstanceHandle_t
DDS_TopicBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader this,
    const DDS_TopicBuiltinTopicData *key_holder)
{
    DDS_InstanceHandle_t handle;

    handle = (DDS_InstanceHandle_t)
        DDS__FooDataReader_lookup_instance (
            this,
            (DDS_sample)key_holder
        );
    return handle;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader this,
    DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader this,
    DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_return_loan (
	    this,
	    (DDS_sequence)data_values,
	    info_seq
	);
    return result;
}

DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader this,
    DDS_PublicationBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_get_key_value (
	    this,
	    (DDS_sample)key_holder,
	    handle
	);
    return result;
}

DDS_InstanceHandle_t
DDS_PublicationBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader this,
    const DDS_PublicationBuiltinTopicData *key_holder)
{
    DDS_InstanceHandle_t handle;

    handle = (DDS_InstanceHandle_t)
        DDS__FooDataReader_lookup_instance (
            this,
            (DDS_sample)key_holder
        );
    return handle;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader this,
    DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader this,
    DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_sample (
	    this,
	    (DDS_sample)data_values,
	    sample_info
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    sample_states,
	    view_states,
	    instance_states
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_read_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_take_next_instance_w_condition (
	    this,
	    (DDS_sequence)data_values,
	    info_seq,
	    max_samples,
	    a_handle,
	    a_condition
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_return_loan (
	    this,
	    (DDS_sequence)data_values,
	    info_seq
	);
    return result;
}

DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader this,
    DDS_SubscriptionBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle
    )
{
    DDS_ReturnCode_t result = (DDS_ReturnCode_t)
        DDS__FooDataReader_get_key_value (
	    this,
	    (DDS_sample)key_holder,
	    handle
	);
    return result;
}

DDS_InstanceHandle_t
DDS_SubscriptionBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader this,
    const DDS_SubscriptionBuiltinTopicData *key_holder)
{
    DDS_InstanceHandle_t handle;

    handle = (DDS_InstanceHandle_t)
        DDS__FooDataReader_lookup_instance (
            this,
            (DDS_sample)key_holder
        );
    return handle;
}

