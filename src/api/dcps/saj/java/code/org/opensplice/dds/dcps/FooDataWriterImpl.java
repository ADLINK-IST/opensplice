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

public abstract class FooDataWriterImpl extends org.opensplice.dds.dcps.DataWriterImpl
{
    private native static long jniRegisterInstance (
	Object DataWriter,
	long copyCache,
	Object instance_data);

    private native static long jniRegisterInstanceWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	DDS.Time_t source_timestamp);

    private native static int jniUnregisterInstance (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle);

    private native static int jniUnregisterInstanceWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle, 
	DDS.Time_t source_timestamp);

    private native static int jniWrite (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle);

    private native static int jniWriteWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle, 
	DDS.Time_t source_timestamp);

    private native static int jniDispose (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long instance_handle);

    private native static int jniDisposeWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long instance_handle, 
	DDS.Time_t source_timestamp);

    private native static int jniWritedispose(
            Object DataWriter,
            long copyCache,
            Object instance_data, 
            long handle);

    private native static int jniWritedisposeWTimestamp(
            Object DataWriter,
            long copyCache,
            Object instance_data, 
            long handle, 
            DDS.Time_t source_timestamp);
    private native static int jniGetKeyValue (
	Object DataWriter,
	long copyCache,
	Object key_holder, 
	long handle);
	
    private native static long jniLookupInstance (
	Object DataWriter,
	long copyCache,
	Object instance_data);


    public static long registerInstance (
	Object DataWriter,
	long copyCache,
	Object instance_data)
    {
        return jniRegisterInstance(
		DataWriter,
		copyCache,
		instance_data);
    }

    public static long registerInstanceWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	DDS.Time_t source_timestamp)
    {
        return jniRegisterInstanceWTimestamp(
		DataWriter,
		copyCache,
		instance_data,
		source_timestamp);
    }

    public static int unregisterInstance (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle)
    {
        return jniUnregisterInstance(
		DataWriter,
		copyCache,
		instance_data,
		handle);
    }

    public static int unregisterInstanceWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle, 
	DDS.Time_t source_timestamp)
    {
        return jniUnregisterInstanceWTimestamp(
		DataWriter,
		copyCache,
		instance_data,
		handle,
		source_timestamp);
    }

    public static int write (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle)
    {
        return jniWrite(
		DataWriter,
		copyCache,
		instance_data,
		handle);
    }

    public static int writeWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long handle, 
	DDS.Time_t source_timestamp)
    {
        return jniWriteWTimestamp(
		DataWriter,
		copyCache,
		instance_data,
		handle,
		source_timestamp);
    }

    public static int dispose (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long instance_handle)
    {
        return jniDispose(
		DataWriter,
		copyCache,
		instance_data,
		instance_handle);
    }

    public static int disposeWTimestamp (
	Object DataWriter,
	long copyCache,
	Object instance_data, 
	long instance_handle, 
	DDS.Time_t source_timestamp)
    {
        return jniDisposeWTimestamp(
		DataWriter,
		copyCache,
		instance_data,
		instance_handle,
		source_timestamp);
    }

    public static int writedispose(
            Object DataWriter,
            long copyCache,
            Object instance_data, 
            long handle)
    {
        return jniWritedispose(
                DataWriter,
                copyCache,
                instance_data,
                handle);
    }

    public static int writedisposeWTimestamp(
            Object DataWriter,
            long copyCache,
            Object instance_data, 
            long handle, 
            DDS.Time_t source_timestamp)
    {
        return jniWritedisposeWTimestamp(
                DataWriter,
                copyCache,
                instance_data,
                handle,
                source_timestamp);
    }
    public static int getKeyValue (
	Object DataWriter,
	long copyCache,
	Object key_holder, 
	long handle)
    {
        return jniGetKeyValue(
		DataWriter,
		copyCache,
		key_holder,
		handle);
    }
	
    public static long lookupInstance (
	Object DataWriter,
	long copyCache,
	Object instance_data)
    {
	return
	    jniLookupInstance (
		DataWriter,
		copyCache,
		instance_data);
    }

}
