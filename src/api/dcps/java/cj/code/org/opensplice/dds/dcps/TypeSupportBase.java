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

public abstract class TypeSupportBase extends SajSuperClass {
	
	public class TypeSupportBaseImpl extends DDS._TypeSupportLocalBase {
		/* typesupport operations  */
		public int register_type(DDS.DomainParticipant domain, java.lang.String type_name) { return 0; }
		public java.lang.String get_type_name() { return null; }
	}
	
	private DDS._TypeSupportLocalBase base;
	
	public TypeSupportBase() {
		base = new TypeSupportBaseImpl();
	}
	
	public String[] _ids()	{
		return base._ids();
	}
	
} 