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

#ifndef SAC_FOODATAREADER_H
#define SAC_FOODATAREADER_H

DDS_ReturnCode_t
DDS_FooDataReader_read (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    );

DDS_ReturnCode_t
DDS_FooDataReader_take (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    );

DDS_ReturnCode_t
DDS_FooDataReader_read_w_condition (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    );

DDS_ReturnCode_t
DDS_FooDataReader_take_w_condition (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    );

DDS_ReturnCode_t
DDS_FooDataReader_read_next_sample (
    DDS_DataReader this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info
    );

DDS_ReturnCode_t
DDS_FooDataReader_take_next_sample (
    DDS_DataReader this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info
    );

DDS_ReturnCode_t
DDS_FooDataReader_read_instance (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    );

DDS_ReturnCode_t
DDS_FooDataReader_take_instance (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    );

DDS_ReturnCode_t
DDS_FooDataReader_read_next_instance (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    );

DDS_ReturnCode_t
DDS_FooDataReader_take_next_instance (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    );

DDS_ReturnCode_t
DDS_FooDataReader_read_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    );

DDS_ReturnCode_t
DDS_FooDataReader_take_next_instance_w_condition (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    );

DDS_ReturnCode_t
DDS_FooDataReader_return_loan (
    DDS_DataReader this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq
    );

DDS_ReturnCode_t
DDS_FooDataReader_get_key_value (
    DDS_DataReader this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle
    );



#endif /* SAC_FOODATAREADER_H */
