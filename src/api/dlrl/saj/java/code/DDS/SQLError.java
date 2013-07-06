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
 * <P>Thrown to indicate that there was an invalid SQL expression used.</P>
 */
public final class SQLError extends java.lang.Exception{

	/**
	 * Constructs a SQLError exception with no detail message.
	 */
	public SQLError (){
		super();
	}

	/**
	 * Constructs a SQLError exception with the specified detail message.
	 */
	public SQLError (String reason){
		super(reason);
	}

} // class SQLError
