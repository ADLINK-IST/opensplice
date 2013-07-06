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
package org.opensplice.cm.transform;

/**
 * Exception that is used to notify about errors in (de)serialization routines. 
 * 
 * @date Jan 17, 2005 
 */
public class TransformationException extends Exception{
    /**
     * Constructs a new exception.
     *  
     * @param message Provides information about the exception.
     */
    public TransformationException(String message){
        super(message);
    }
}
