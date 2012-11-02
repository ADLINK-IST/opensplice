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

public abstract class FooDataReaderViewImpl extends org.opensplice.dds.dcps.DataReaderImpl
{

    private native static int jniRead (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniTake (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniReadWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition);

    private native static int jniTakeWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition);

    private native static int jniReadNextSample (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info);

    private native static int jniTakeNextSample (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info);

    private native static int jniReadInstance (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniTakeInstance (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniReadNextInstance (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniTakeNextInstance (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	int sample_states, 
	int view_states, 
	int instance_states);

    private native static int jniReadNextInstanceWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition);

    private native static int jniTakeNextInstanceWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition);

    private native static int jniGetKeyValue (
	Object DataReaderView,
	long copyCache,
	Object key_holder, 
	long handle);
    
    private native static long jniLookupInstance (
	Object DataReaderView,
	long copyCache,
	Object instance);

    public static int read (
	Object DataReaderView,
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
	    DataReaderView,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int take (
	Object DataReaderView,
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
		DataReaderView,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		sample_states, 
		view_states, 
		instance_states);
    }

    public static int readWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniReadWCondition (
		DataReaderView,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		a_condition);
    }

    public static int takeWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniTakeWCondition (
		DataReaderView,
		copyCache,
		data_values, 
		info_seq, 
		max_samples, 
		a_condition);
    }

    public static int readNextSample (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info)
    {
	return
	    jniReadNextSample (
		DataReaderView,
		copyCache,
		data_values, 
		sample_info);
    }

    public static int takeNextSample (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoHolder sample_info)
    {
	return
	    jniTakeNextSample (
		DataReaderView,
		copyCache,
		data_values, 
		sample_info);
    }

    public static int readInstance (
	Object DataReaderView,
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
		DataReaderView,
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
	Object DataReaderView,
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
		DataReaderView,
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
	Object DataReaderView,
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
		DataReaderView,
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
	Object DataReaderView,
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
		DataReaderView,
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
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniReadNextInstanceWCondition (
		DataReaderView,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		a_condition);
    }

    public static int takeNextInstanceWCondition (
	Object DataReaderView,
	long copyCache,
	Object data_values, 
	DDS.SampleInfoSeqHolder info_seq, 
	int max_samples, 
	long a_handle, 
	DDS.ReadCondition a_condition)
    {
	return
	    jniTakeNextInstanceWCondition (
		DataReaderView,
		copyCache,
		data_values, 
		info_seq, 
		max_samples,
		a_handle, 
		a_condition);
    }

    public static int getKeyValue (
	Object DataReaderView,
	long copyCache,
	Object key_holder, 
	long handle)
    {
	return
	    jniGetKeyValue (
		DataReaderView,
		copyCache,
		key_holder,
		handle);
    }
    
    public static long lookupInstance (
	Object DataReaderView,
	long copyCache,
	Object instance)
    {
	return
	    jniLookupInstance (
		DataReaderView,
		copyCache,
		instance);
    }

}
