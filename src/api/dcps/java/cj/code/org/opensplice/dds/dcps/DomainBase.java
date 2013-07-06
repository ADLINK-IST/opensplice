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

public class DomainBase extends SajSuperClass {
	
	public class DomainBaseImpl extends DDS._DomainLocalBase {
		/* domain operations  */
		public int create_persistent_snapshot(java.lang.String partition_expression, java.lang.String topic_expression, java.lang.String URI) { return 0; }	
	}
	
	private DDS._DomainLocalBase base;
	
	public DomainBase() {
		base = new DomainBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}