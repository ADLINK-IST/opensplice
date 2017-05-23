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

package org.opensplice.dds.dcps;

/**
 * This interface is implemented by all DDS defined classes and holds the 
 * adress of the equivalent <code>gapi</code> object. The adress is stored as a 
 * <code>long</code> because the <code>gapi</code> can be compiled for any
 * platform using up to 64 bit adressing. 
 */
abstract public class ObjectBase extends org.omg.CORBA.portable.ObjectImpl {
	
}
