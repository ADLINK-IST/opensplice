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
 * Abstract base class for SOAP messages.
 * 
 * @date Feb 17, 2005 
 */
abstract class SOAPMessage {
    /**
     * The body contents of the SOAP message.
     */
    protected String body = null;

    /**
     * Constructs a new SOAPMessage.
     */
    protected SOAPMessage(){}
    
    /**
     * Provides access to the SOAP body contents.
     * 
     * @return The contents of the SOAP body.
     */
    public String getBodyContent(){
        return body;
    }
}
