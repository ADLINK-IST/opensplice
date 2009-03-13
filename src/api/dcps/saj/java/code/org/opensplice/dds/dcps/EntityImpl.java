

package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.Entity} interface. 
 */ 
public class EntityImpl extends SajSuperClass implements DDS.Entity { 

    /* see DDS.EntityOperations for javadoc */ 
    public int enable () {
        return jniEnable();
    }

    /* see DDS.EntityOperations for javadoc */ 
    public DDS.StatusCondition get_statuscondition () {
        return jniGetStatuscondition();
    }

    /* see DDS.EntityOperations for javadoc */ 
    public int get_status_changes () {
        return jniGetStatusChanges();
    }

    public long get_instance_handle () {
        return jniGetInstanceHandle();
    }

    private native int jniEnable();
    private native DDS.StatusCondition jniGetStatuscondition();
    private native int jniGetStatusChanges();
    private native long jniGetInstanceHandle();
}
