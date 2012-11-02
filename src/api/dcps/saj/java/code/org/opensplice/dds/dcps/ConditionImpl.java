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
 * Implementation of the {@link DDS.Condition} interface. 
 */ 
public class ConditionImpl extends SajSuperClass implements DDS.Condition { 

    /* see DDS.ConditionOperations for javadoc */ 
    public boolean get_trigger_value () {
        return jniGetTriggerValue();
    }

    private native boolean jniGetTriggerValue();
}
