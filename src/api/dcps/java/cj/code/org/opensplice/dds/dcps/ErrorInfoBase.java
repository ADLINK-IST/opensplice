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

public class ErrorInfoBase extends SajSuperClass {
	
	public class ErrorInfoBaseImpl extends DDS._ErrorInfoInterfaceLocalBase {
		/* operations  */
		public int update() { return 0; }
		public int get_code(org.omg.CORBA.IntHolder code) { return 0; }
		public int get_message(org.omg.CORBA.StringHolder message) { return 0; }
		public int get_location(org.omg.CORBA.StringHolder location) { return 0; }
		public int get_source_line(org.omg.CORBA.StringHolder source_line) { return 0; }
		public int get_stack_trace(org.omg.CORBA.StringHolder stack_trace) { return 0; }
	}
	
	private DDS._ErrorInfoInterfaceLocalBase base;
	
	public ErrorInfoBase() {
		base = new ErrorInfoBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}