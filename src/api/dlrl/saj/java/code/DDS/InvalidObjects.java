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
 * <P>Thrown to indicate that the one or more objects within a cache access
 * has invalid relations.</P>
 */
public final class InvalidObjects extends java.lang.Exception{

	/**
	 * Constructs an AlreadyDeleted exception with no detail message.
	 */
	public InvalidObjects (){
		super();
	}
	/**
	 * Constructs an InvalidObjects exception with the specified detail message.
	 */
	public InvalidObjects (String reason){
		super(reason);
	}

} // class InvalidObjects
