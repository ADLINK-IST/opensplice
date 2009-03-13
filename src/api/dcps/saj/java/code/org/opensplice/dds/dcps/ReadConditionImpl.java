

package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.ReadCondition} interface. 
 */ 
public class ReadConditionImpl extends ConditionImpl implements DDS.ReadCondition { 

    /* see DDS.ReadConditionOperations for javadoc */ 
    public int get_sample_state_mask () {
        return jniGetSampleStateMask();
    }

    /* see DDS.ReadConditionOperations for javadoc */ 
    public int get_view_state_mask () {
        return jniGetViewStateMask();
    }

    /* see DDS.ReadConditionOperations for javadoc */ 
    public int get_instance_state_mask () {
        return jniGetInstanceStateMask();
    }

    /* see DDS.ReadConditionOperations for javadoc */ 
    public DDS.DataReader get_datareader () {
        return jniGetDatareader();
    }

    private native int jniGetSampleStateMask();
    private native int jniGetViewStateMask();
    private native int jniGetInstanceStateMask();
    private native DDS.DataReader jniGetDatareader();
}
