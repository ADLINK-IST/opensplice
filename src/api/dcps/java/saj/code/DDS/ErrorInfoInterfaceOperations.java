/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
