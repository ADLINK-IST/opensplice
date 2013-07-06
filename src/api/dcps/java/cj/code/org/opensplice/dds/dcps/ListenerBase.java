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

public class ListenerBase extends SajSuperClass {
	
	public class ListenerBaseImpl extends DDS._ListenerLocalBase {}
	
	private DDS._ListenerLocalBase base;
	
	public ListenerBase() {
		base = new ListenerBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}