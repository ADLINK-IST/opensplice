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
 * <P>Thrown to indicate that an invalid parameter was passed to an operation.
 * This exception does not need to be caught.</P>
 */
public final class BadParameter extends java.lang.RuntimeException{

	/**
	 * Constructs a BadParameter exception with no detail message.
	 */
	public BadParameter (){
		super();
	}
	/**
	 * Constructs a BadParameter exception with the specified detail message.
	 */
	public BadParameter (String reason){
		super(reason);
	}

} // class BadParameter
