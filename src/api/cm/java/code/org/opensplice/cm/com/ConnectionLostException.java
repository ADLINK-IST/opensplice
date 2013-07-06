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

import org.opensplice.cm.CMException;


/**
 * Exception that is used to notify that the connection is lost.
 * 
 * @date Apr 19, 2005 
 */
public class ConnectionLostException extends CommunicationException{
    public static final String CONNECTION_LOST = CMException.CONNECTION_LOST;
    /**
     *  Constructs a new exception.
     */
    public ConnectionLostException() {
        super(CMException.CONNECTION_LOST);
    }

}
