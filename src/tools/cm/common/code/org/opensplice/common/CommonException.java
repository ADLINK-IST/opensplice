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
 * Contains all classes that are common for SPLICE DDS C&M Tooling.
 */
package org.opensplice.common;

import org.opensplice.cm.CMException;

/**
 * Default exception for the common package.
 * 
 * @date Nov 4, 2004 
 */
public class CommonException extends Exception {
    public static final String CONNECTION_LOST = CMException.CONNECTION_LOST;
    /**
     * Creates a new CommonException with the supplied message. 
     *
     * @param message The message to attach to the exception.
     */
    public CommonException(String message){
        super(message);
    }
}
