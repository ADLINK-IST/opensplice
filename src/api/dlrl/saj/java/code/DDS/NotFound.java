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
 * <P>Thrown to indicate that the related object being navigated to
 * cannot be resolved by the DLRL.</P>
 */
public final class NotFound extends java.lang.Exception{

	/**
	 * Constructs a NotFound exception with no detail message.
	 */
	public NotFound (){
		super();
	} 

	/**
	 * Constructs a NotFound exception with the specified detail message.
	 */
	public NotFound (String reason){
		super(reason);
	}
} // class NotFound
