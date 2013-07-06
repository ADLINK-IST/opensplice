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
 * <P>Thrown to indicate that an error occured in the DCPS layer.
 * The message will contain a reference to the specific DCPS errorcode
 * that resulted in the exception as well as the cause.</P>
 */
public final class DCPSError extends java.lang.RuntimeException{

	/**
	 * Constructs a DCPSError exception with no detail message.
	 */
	public DCPSError (){
		super();
	}

	/**
	 * Constructs a DCPSError exception with the specified detail message.
	 */
	public DCPSError (String reason){
		super(reason);
	}

} // class DCPSError
