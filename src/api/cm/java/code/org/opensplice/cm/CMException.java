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
/**
 * Provides the Java Control & Monitoring API.
 * 
 * @date May 6, 2004
 */
package org.opensplice.cm;

/**
 * Control & Monitoring exception.
 *  
 * The exception provides information about the exception that occurred. This
 * message can be accessed by calling the getMessage function.
 */
public class CMException extends Exception{
    public static final String CONNECTION_LOST = "Connection with node lost.";
    /**
     * Initializes the exception.
     *
     * @param message The message that is associated with the exception.
     */
    public CMException(String message){
        super(message);
    }
}
