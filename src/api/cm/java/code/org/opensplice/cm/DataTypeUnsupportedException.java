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
package org.opensplice.cm;
/**
 * Exception that is used for notifying that a specific MetaType is not 
 * supported by this API.
 * 
 * @date Mar 4, 2005 
 */
public class DataTypeUnsupportedException extends Exception {
    /**
     * Constructs a new exception.
     *  
     * @param message The message that provides information about why 
     *                the data type is not supported.
     */
    public DataTypeUnsupportedException(String message){
        super(message);
    }
}
