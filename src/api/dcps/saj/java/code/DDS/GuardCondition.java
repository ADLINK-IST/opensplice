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

import org.opensplice.dds.dcps.ConditionImpl;


public class GuardCondition extends ConditionImpl 
                            implements GuardConditionOperations, DDS.Condition
{
    public GuardCondition(){
        boolean success = false;
        
        try{
            success = jniGuardConditionAlloc();
        } catch(UnsatisfiedLinkError ule){
            /*
             * JNI library is not loaded if no instance of the 
             * DomainParticipantFactory exists.
             */
            DomainParticipantFactory f = DomainParticipantFactory.get_instance();
            
            if(f != null){
                success = jniGuardConditionAlloc();
            }
        }
        if(!success){
            throw new OutOfMemoryError("Could not allocate DDS.GuardCondition.");
        }
    }
    /* see DDS.GuardConditionOperations for javadoc */ 
    public int set_trigger_value (boolean value) {
        return jniSetTriggerValue(value);

    }
    
    protected void finalize(){
        jniGuardConditionFree();
    }
    private native boolean jniGuardConditionAlloc();
    private native void jniGuardConditionFree();
    private native int jniSetTriggerValue(boolean value);
    
}
