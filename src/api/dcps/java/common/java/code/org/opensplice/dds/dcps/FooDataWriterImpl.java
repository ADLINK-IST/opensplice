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

package org.opensplice.dds.dcps;

public abstract class FooDataWriterImpl extends org.opensplice.dds.dcps.DataWriterImpl
{
    private static final long serialVersionUID = 2200670522665717288L;

    @Override
    protected int deinit () { return super.deinit(); }

    public native static long jniRegisterInstance (
        long uWriter,
        long copyCache,
        Object instance_data,
        DDS.Time_t source_timestamp);

    public native static int jniUnregisterInstance (
        long uWriter,
	    long copyCache,
	    Object instance_data,
	    long handle,
	    DDS.Time_t source_timestamp);

    public native static int jniWrite (
        long uWriter,
        long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);

    public native static int jniDispose (
        long uWriter,
        long copyCache,
        Object instance_data,
        long instance_handle,
        DDS.Time_t source_timestamp);

    public native static int jniWritedispose(
        long uWriter,
        long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);

    public native static int jniGetKeyValue (
        long uWriter,
        long copyCache,
        Object key_holder,
        long handle);
	
    public native static long jniLookupInstance (
        long uWriter,
        long copyCache,
        Object instance_data);
}
