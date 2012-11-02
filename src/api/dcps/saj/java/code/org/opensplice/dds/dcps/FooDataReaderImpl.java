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

package org.opensplice.dds.dcps;

public abstract class FooDataReaderImpl extends org.opensplice.dds.dcps.DataReaderImpl
{

    private native static int jniRead (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniTake (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniReadWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition);

    private native static int jniTakeWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition);

    private native static int jniReadNextSample (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info);

    private native static int jniTakeNextSample (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info);

    private native static int jniReadInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniTakeInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniReadNextInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniTakeNextInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniReadNextInstanceWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition);

    private native static int jniTakeNextInstanceWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition);

    private native static int jniGetKeyValue (
	Object DataReader,
	long copyCache,
	Object key_holder, 
	long handle);
    
    private native static long jniLookupInstance (
	Object DataReader,
	long copyCache,
	Object instance);

    public static int read (
	Object DataReader,
	long copyCache,
	Object data_values,
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	int sample_states, 
	int view_states, 
	int instance_states)
    {
	return
	    jniRead (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int take (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	int sample_states, 
	int view_states, 
	int instance_states)
    {
	return
	    jniTake (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int readWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniReadWCondition (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		a_condition);
    }

    public static int takeWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniTakeWCondition (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		a_condition);
    }

    public static int readNextSample (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info)
    {
	return
	    jniReadNextSample (
		DataReader,
		copyCache,
		data_values, 
		sample_info);
    }

    public static int takeNextSample (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info)
    {
	return
	    jniTakeNextSample (
		DataReader,
		copyCache,
		data_values, 
		sample_info);
    }

    public static int readInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states)
    {
	return
	    jniReadInstance (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int takeInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states)
    {
	return
	    jniTakeInstance (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int readNextInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states)
    {
	return
	    jniReadNextInstance (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int takeNextInstance (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states)
    {
	return
	    jniTakeNextInstance (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int readNextInstanceWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniReadNextInstanceWCondition (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		a_condition);
    }

    public static int takeNextInstanceWCondition (
	Object DataReader,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniTakeNextInstanceWCondition (
		DataReader,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		a_condition);
    }

    public static int getKeyValue (
	Object DataReader,
	long copyCache,
	Object key_holder, 
	long handle)
    {
	return
	    jniGetKeyValue (
		DataReader,
		copyCache,
		key_holder,
		handle);
    }
    
    public static long lookupInstance (
	Object DataReader,
	long copyCache,
	Object instance)
    {
	return
	    jniLookupInstance (
		DataReader,
		copyCache,
		instance);
    }

}
