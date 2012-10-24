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
package DDS;

import org.opensplice.dds.dcps.SajSuperClass;

public class ErrorInfo extends SajSuperClass 
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

    public int get_location(DDS.StringHolder location) {
        return jniGetLocation(location);
    }

    public int get_source_line(DDS.StringHolder source_line) {
        return jniGetSourceLine(source_line);
    }

    public int get_stack_trace(DDS.StringHolder stack_trace) {
        return jniGetStackTrace(stack_trace);
    }

    public int get_message(DDS.StringHolder message) {
        return jniGetMessage(message);
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

