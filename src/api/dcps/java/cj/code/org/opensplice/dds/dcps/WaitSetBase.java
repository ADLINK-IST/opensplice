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

package org.opensplice.dds.dcps;

public class WaitSetBase extends SajSuperClass {
    
    public class WaitSetBaseImpl extends DDS._WaitSetInterfaceLocalBase {
    	/* waitsetinterface operations  */
    	public int _wait(DDS.ConditionSeqHolder active_conditions, DDS.Duration_t timeout) { return 0; }
    	public int attach_condition(DDS.Condition cond) { return 0; }
    	public int detach_condition(DDS.Condition cond) { return 0; }
    	public int get_conditions(DDS.ConditionSeqHolder attached_conditions) { return 0; }
    }
    
    private DDS._WaitSetInterfaceLocalBase base;
    
    public WaitSetBase() {
        base = new WaitSetBaseImpl();
    }
    
    public String[] _ids() {
        return base._ids();
    }
    
}