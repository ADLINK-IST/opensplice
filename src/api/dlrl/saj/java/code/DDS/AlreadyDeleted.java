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
 * <P>Thrown to indicate that the object on which an operation was accessed 
 * has already been deleted. This is a run time exception</P>
 */
public final class AlreadyDeleted extends java.lang.RuntimeException{

	/**
	 * Constructs an AlreadyDeleted exception with no detail message.
	 */
	public AlreadyDeleted (){
		super();
	}
	/**
	 * Constructs an AlreadyDeleted exception with the specified detail message.
	 */
	public AlreadyDeleted (String reason){
		super(reason);
	}

} // class AlreadyDeleted
