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
package DDS;

import org.opensplice.dds.dcps.ErrorInfoBase;

public class ErrorInfo extends ErrorInfoBase implements DDS.ErrorInfoInterface
{
    public ErrorInfo(){
        boolean success = false;
        
        try{
            success = jniErrorInfoAlloc();
        } catch(UnsatisfiedLinkError ule){
            /*
             * JNI library is not loaded if no instance of the 
             * DomainParticipantFactory exists.
             */
            DomainParticipantFactory f = DomainParticipantFactory.get_instance();
            
            if(f != null){
                success = jniErrorInfoAlloc();
            }
        }
        if(!success){
            throw new OutOfMemoryError("Could not allocate DDS.ErrorInfo.");
        }
    }

    public int update () {
        return jniUpdate();
    }

    public int get_code (DDS.ErrorCodeHolder code) {
        return jniGetCode(code);
    }

    public int get_code (org.omg.CORBA.IntHolder code) {
        DDS.ErrorCodeHolder tmpHolder = new DDS.ErrorCodeHolder();
        tmpHolder.value = code.value;
        return jniGetCode(tmpHolder);
    }

    public int get_location(DDS.StringHolder location) {
        return jniGetLocation(location);
    }

    public int get_location(org.omg.CORBA.StringHolder location) {
        DDS.StringHolder tmpHolder = new DDS.StringHolder();
        tmpHolder.value = location.value;
        return jniGetLocation(tmpHolder);
    }

    public int get_source_line(DDS.StringHolder source_line) {
        return jniGetSourceLine(source_line);
    }

    public int get_source_line(org.omg.CORBA.StringHolder source_line) {
        DDS.StringHolder tmpHolder = new DDS.StringHolder();
        tmpHolder.value = source_line.value;
        return jniGetSourceLine(tmpHolder);
    }

    public int get_stack_trace(DDS.StringHolder stack_trace) {
        return jniGetStackTrace(stack_trace);
    }

    public int get_stack_trace(org.omg.CORBA.StringHolder stack_trace) {
        DDS.StringHolder tmpHolder = new DDS.StringHolder();
        tmpHolder.value = stack_trace.value;
        return jniGetStackTrace(tmpHolder);
    }

    public int get_message(DDS.StringHolder message) {
        return jniGetMessage(message);
    }

    public int get_message(org.omg.CORBA.StringHolder message) {
        DDS.StringHolder tmpHolder = new DDS.StringHolder();
        tmpHolder.value = message.value;
        return jniGetMessage(tmpHolder);
    }

    protected void finalize(){
        jniErrorInfoFree();
    }

    private native boolean jniErrorInfoAlloc();
    private native void jniErrorInfoFree();
    private native int jniUpdate();
    private native int jniGetCode(DDS.ErrorCodeHolder code);
    private native int jniGetLocation(DDS.StringHolder location);
    private native int jniGetSourceLine(DDS.StringHolder source_line);
    private native int jniGetStackTrace(DDS.StringHolder stack_trace);
    private native int jniGetMessage(DDS.StringHolder message);
}

