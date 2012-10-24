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


public class WaitSet extends SajSuperClass implements WaitSetOperations
{
    public WaitSet(){
        boolean success = false;
        
        try{
            success = jniWaitSetAlloc();
        } catch(UnsatisfiedLinkError ule){
            /*
             * JNI library is not loaded if no instance of the 
             * DomainParticipantFactory exists.
             */
            DomainParticipantFactory f = DomainParticipantFactory.get_instance();
            
            if(f != null){
                success = jniWaitSetAlloc();
            }
        }
        if(!success){
            throw new OutOfMemoryError("Could not allocate DDS.WaitSet.");
        }
    }
    /* see DDS.WaitSetOperations for javadoc */ 
    public int _wait (DDS.ConditionSeqHolder active_conditions, DDS.Duration_t timeout) {
        return jniWait(active_conditions, timeout);
    }

    /* see DDS.WaitSetOperations for javadoc */ 
    public int attach_condition (DDS.Condition cond) {
        return jniAttachCondition(cond);
    }

    /* see DDS.WaitSetOperations for javadoc */ 
    public int detach_condition (DDS.Condition cond) {
        return jniDetachCondition(cond);
    }

    /* see DDS.WaitSetOperations for javadoc */ 
    public int get_conditions (DDS.ConditionSeqHolder attached_conditions) {
        return jniGetConditions(attached_conditions);
    }
    
    protected void finalize(){
        jniWaitSetFree();
    }
    
    private native boolean jniWaitSetAlloc();
    private native void jniWaitSetFree();
    private native int jniWait(DDS.ConditionSeqHolder active_conditions, DDS.Duration_t timeout);
    private native int jniAttachCondition(DDS.Condition cond);
    private native int jniDetachCondition(DDS.Condition cond);
    private native int jniGetConditions(DDS.ConditionSeqHolder attached_conditions);
}
