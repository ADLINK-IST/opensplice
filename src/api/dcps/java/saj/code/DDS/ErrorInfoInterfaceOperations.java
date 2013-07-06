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
package DDS;

public interface ErrorInfoInterfaceOperations
{
	/* operations  */
	int update();
	int get_code(org.omg.CORBA.IntHolder code);
	int get_message(org.omg.CORBA.StringHolder message);
	int get_location(org.omg.CORBA.StringHolder location);
	int get_source_line(org.omg.CORBA.StringHolder source_line);
	int get_stack_trace(org.omg.CORBA.StringHolder stack_trace);
}
