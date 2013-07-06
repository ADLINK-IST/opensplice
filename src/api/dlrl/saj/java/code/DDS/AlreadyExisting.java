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

/**
 * <P>Thrown to indicate that the object being created already exists.</P>
 */
public final class AlreadyExisting extends java.lang.Exception{

	/**
	 * Constructs an AlreadyExisting exception with no detail message.
	 */
	public AlreadyExisting (){
		super();
	}

	/**
	 * Constructs an AlreadyExisting exception with the specified detail message.
	 */
	public AlreadyExisting (String reason){
		super(reason);
	}

} // class AlreadyExisting
