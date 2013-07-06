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

public class AttributeType {
	private        int __value;
	private static int __size = 9;
	private static org.opensplice.dds.dlrl.metamodel.AttributeType[] __array = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType [__size];

	public static final int _DMM_LONG = 0;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_LONG = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_LONG);
	public static final int _DMM_FLOAT = 1;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_FLOAT = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_FLOAT);
	public static final int _DMM_STRING = 2;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_STRING = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_STRING);
	public static final int _DMM_DOUBLE = 3;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_DOUBLE = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_DOUBLE);
	public static final int _DMM_LONGLONG = 4;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_LONGLONG = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_LONGLONG);
	public static final int _DMM_LONGDOUBLE = 5;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_LONGDOUBLE = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_LONGDOUBLE);
	public static final int _DMM_CHAR = 6;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_CHAR = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_CHAR);
	public static final int _DMM_BOOLEAN = 7;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_BOOLEAN = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_BOOLEAN);
	public static final int _DMM_ENUM = 8;
	public static final org.opensplice.dds.dlrl.metamodel.AttributeType DMM_ENUM = 
                                            new org.opensplice.dds.dlrl.metamodel.AttributeType(_DMM_ENUM);
	
    public int value (){
		return __value;
	}

	public static org.opensplice.dds.dlrl.metamodel.AttributeType from_int (int value){
		if (value >= 0 && value < __size){
		  return __array[value];
		} else {
		  throw new java.lang.RuntimeException();
		}
	}

	protected AttributeType (int value){
		__value = value;
		__array[__value] = this;
	}
} // class AttributeType
