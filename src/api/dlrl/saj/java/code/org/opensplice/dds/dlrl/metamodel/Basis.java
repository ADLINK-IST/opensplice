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

public class Basis {
	private        int __value;
	private static int __size = 3;
	private static org.opensplice.dds.dlrl.metamodel.Basis[] __array = 
                                                        new org.opensplice.dds.dlrl.metamodel.Basis [__size];

	public static final int _STR_MAP_BASE = 0;
	public static final org.opensplice.dds.dlrl.metamodel.Basis STRMAPBASE = 
                                                        new org.opensplice.dds.dlrl.metamodel.Basis(_STR_MAP_BASE);
	public static final int _INT_MAP_BASE = 1;
	public static final org.opensplice.dds.dlrl.metamodel.Basis INTMAPBASE = 
                                                        new org.opensplice.dds.dlrl.metamodel.Basis(_INT_MAP_BASE);
	public static final int _SET_BASE = 2;
	public static final org.opensplice.dds.dlrl.metamodel.Basis SETBASE = 
                                                        new org.opensplice.dds.dlrl.metamodel.Basis(_SET_BASE);
	public int value (){
		return __value;
	}

	public static org.opensplice.dds.dlrl.metamodel.Basis from_int (int value){
		if (value >= 0 && value < __size){
		  return __array[value];
		} else {
		  throw new java.lang.RuntimeException();
		}
	}

	protected Basis (int value){
		__value = value;
		__array[__value] = this;
	}
} // class Basis
