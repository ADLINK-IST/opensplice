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
 * Exception that occurs when an error in SOAP communication occurs.
 * 
 * @date Feb 17, 2005
 */
public class SOAPException extends Exception{

    /* soap fault codes */
    public static final int SOAP_Internal        = 0;
    public static final int SOAP_VersionMismatch = 1;
    public static final int SOAP_MustUnderstand  = 2;
    public static final int SOAP_Client          = 3;
    public static final int SOAP_Server          = 4;

    private int             faultCode            = SOAP_Internal;
    private String          faultString          = null;

    /**
     * Constructs a new SOAPException.
     * 
     * @param message
     *            The message to attach to the exception.
     */
    public SOAPException(String message){
        super(message);
    }

    public SOAPException(String message, String faultcode, String faultstring) {
        super(message);
        if (faultcode.equals("SOAP-ENV:VersionMismatch")) {
            faultCode = SOAP_VersionMismatch;
        } else if (faultcode.equals("SOAP-ENV:MustUnderstand")) {
            faultCode = SOAP_MustUnderstand;
        } else if (faultcode.equals("SOAP-ENV:Client")) {
            faultCode = SOAP_Client;
        } else if (faultcode.equals("SOAP-ENV:VersionMismatch")) {
            faultCode = SOAP_Server;
        }
        faultString = faultstring;
    }

    public int getFaultCode() {
        return faultCode;
    }

    public String getFaultString() {
        return faultString;
    }
}

