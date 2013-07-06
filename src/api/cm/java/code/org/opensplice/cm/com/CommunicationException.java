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
package org.opensplice.cm.com;

/**
 * Represents an exception that occurred in the communication package. When
 * an exception of this kind occurs, something went wrong in the communication
 * layer (org.opensplice.api.cm.com).
 * 
 * @date Jan 17, 2005 
 */
public class CommunicationException extends Exception{
    /**
     * Constructs a new CommunicationException.
     *
     * @param message The exception error message.
     */
    public CommunicationException(String message){
        super(message);
    }
}
