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

public class StatusConditionBase extends ConditionImpl {
	
	public class StatusConditionBaseImpl extends DDS._StatusConditionLocalBase {
		/* statuscondition operations  */
		public int get_enabled_statuses() { return 0; }
		public int set_enabled_statuses(int mask) { return 0; }
		public DDS.Entity get_entity() { return null; }
		
		/* condition operations  */
		public boolean get_trigger_value() { return false; }
	}
	
	private DDS._StatusConditionLocalBase base;
	
	public StatusConditionBase() {
		base = new StatusConditionBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}