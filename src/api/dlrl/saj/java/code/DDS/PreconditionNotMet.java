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
 * <P>Thrown to indicate that a specific precondition for the invoked
 * operation has not yet been met.</P>
 */
public final class PreconditionNotMet extends java.lang.Exception{

	/**
	 * Constructs a PreconditionNotMet exception with no detail message.
	 */
	public PreconditionNotMet (){
		super();
	}

	/**
	 * Constructs a PreconditionNotMet exception with the specified detail message.
	 */
	public PreconditionNotMet (String reason){
		super(reason);
	}
} // class PreconditionNotMet
