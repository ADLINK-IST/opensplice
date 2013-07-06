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
 * <P>Thrown to indicate that an invalid home definition was found or that
 * there was a mismatch between two ObjectHomes.</P>
 */
public final class BadHomeDefinition extends java.lang.Exception{

	/**
	 * Constructs a BadHomeDefinition exception with no detail message.
	 */
    public BadHomeDefinition (){
        super();
    }

	/**
	 * Constructs a BadHomeDefinition exception with the specified detail message.
	 */
    public BadHomeDefinition (String reason){
        super(reason);
    } 

} // class BadHomeDefinition
