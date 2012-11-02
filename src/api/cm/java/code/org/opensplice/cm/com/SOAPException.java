/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.cm.com;

/**
 * Exception that occurs when an error in SOAP communication occurs.
 * 
 * @date Feb 17, 2005
 */
public class SOAPException extends Exception{

    /**
     * Constructs a new SOAPException.
     * 
     * @param message The message to attach to the exception.
     */
    public SOAPException(String message){
        super(message);
    }
}

