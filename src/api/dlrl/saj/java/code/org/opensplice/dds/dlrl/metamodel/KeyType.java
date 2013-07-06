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

public class KeyType {
	private        int __value;
	private static int __size = 4;
	private static org.opensplice.dds.dlrl.metamodel.KeyType[] __array =
                                                    new org.opensplice.dds.dlrl.metamodel.KeyType [__size];

	public static final int _KEY = 0;
	public static final org.opensplice.dds.dlrl.metamodel.KeyType KEY = 
                                                    new org.opensplice.dds.dlrl.metamodel.KeyType(_KEY);
	public static final int _SHARED_KEY = 1;
	public static final org.opensplice.dds.dlrl.metamodel.KeyType SHARED_KEY = 
                                                    new org.opensplice.dds.dlrl.metamodel.KeyType(_SHARED_KEY);
	public static final int _FOREIGN_KEY = 2;
	public static final org.opensplice.dds.dlrl.metamodel.KeyType FOREIGN_KEY = 
                                                    new org.opensplice.dds.dlrl.metamodel.KeyType(_FOREIGN_KEY);
	public static final int _NORMAL = 3;
	public static final org.opensplice.dds.dlrl.metamodel.KeyType NORMAL = 
                                                    new org.opensplice.dds.dlrl.metamodel.KeyType(_NORMAL);
	public int value (){
		return __value;
	}

	public static org.opensplice.dds.dlrl.metamodel.KeyType from_int (int value){
		if (value >= 0 && value < __size){
		  return __array[value];
		} else {
		  throw new java.lang.RuntimeException();
		}
	}

	protected KeyType (int value){
		__value = value;
		__array[__value] = this;
	}
} // class KeyType
