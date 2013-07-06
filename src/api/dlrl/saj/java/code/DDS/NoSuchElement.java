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
 * <P>Thrown to indicate that the element requested does not exist 
 * within the collection.</P>
 */
public final class NoSuchElement extends java.lang.Exception{

	/**
	 * Constructs a NoSuchElement exception with no detail message.
	 */
	public NoSuchElement (){
		super();
	}

	/**
	 * Constructs a NoSuchElement exception with the specified detail message.
	 */
	public NoSuchElement (String reason){
		super(reason);
	}

} // class NoSuchElementException
