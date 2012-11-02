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

/** 
 * Implementation of the {@link DDS.StatusCondition} interface. 
 */ 
public class StatusConditionImpl extends ConditionImpl implements DDS.StatusCondition { 

    /* see DDS.StatusConditionOperations for javadoc */ 
    public int get_enabled_statuses () {
        return jniGetEnabledStatuses();
    }

    /* see DDS.StatusConditionOperations for javadoc */ 
    public int set_enabled_statuses (int mask) {
        return jniSetEnabledStatuses(mask);
    }

    /* see DDS.StatusConditionOperations for javadoc */ 
    public DDS.Entity get_entity () {
        return jniGetEntity();
    }

    private native int jniGetEnabledStatuses();
    private native int jniSetEnabledStatuses(int mask);
    private native DDS.Entity jniGetEntity();
}
