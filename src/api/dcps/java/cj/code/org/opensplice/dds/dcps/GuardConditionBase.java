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

public class GuardConditionBase extends ConditionImpl {
	
	public class GuardConditionBaseImpl extends DDS._GuardConditionInterfaceLocalBase {
		/* guardconditioninterface operations  */
		public int set_trigger_value(boolean value) { return 0; }
		
		/* condition operations  */
		public boolean get_trigger_value() { return false; }
	}
	
	private DDS._GuardConditionInterfaceLocalBase base;
	
	public GuardConditionBase() {
		base = new GuardConditionBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}