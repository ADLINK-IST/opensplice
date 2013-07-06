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
package org.opensplice.dds.dlrl.metamodel;

public class Mapping {
	private        int __value;
	private static int __size = 2;
	private static org.opensplice.dds.dlrl.metamodel.Mapping[] __array = 
                                                    new org.opensplice.dds.dlrl.metamodel.Mapping [__size];

	public static final int _PREDEFINED = 0;
	public static final org.opensplice.dds.dlrl.metamodel.Mapping PREDEFINED = 
                                                    new org.opensplice.dds.dlrl.metamodel.Mapping(_PREDEFINED);
	public static final int _DEFAULT = 1;
	public static final org.opensplice.dds.dlrl.metamodel.Mapping DEFAULT =
                                                    new org.opensplice.dds.dlrl.metamodel.Mapping(_DEFAULT);

	public int value (){
		return __value;
	}

	public static org.opensplice.dds.dlrl.metamodel.Mapping from_int (int value){
		if (value >= 0 && value < __size){
		  return __array[value];
		} else {
		  throw new java.lang.RuntimeException();
		}
	}

	protected Mapping (int value){
		__value = value;
		__array[__value] = this;
	}
} // class Mapping
