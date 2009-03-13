

package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.Condition} interface. 
 */ 
public class ConditionImpl extends SajSuperClass implements DDS.Condition { 

    /* see DDS.ConditionOperations for javadoc */ 
    public boolean get_trigger_value () {
        return jniGetTriggerValue();
    }

    private native boolean jniGetTriggerValue();
}
