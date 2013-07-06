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

public class ConditionBase extends SajSuperClass {
	
	public class ConditionBaseImpl extends DDS._ConditionLocalBase {
		/* condition operations  */
		public boolean get_trigger_value() { return false; }
	}
	
	private DDS._ConditionLocalBase base;
	
	public ConditionBase() {
		base = new ConditionBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}