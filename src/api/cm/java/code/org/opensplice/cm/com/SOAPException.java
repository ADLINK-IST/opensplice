/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.cm.com;

/**
 * Exception that occurs when an error in SOAP communication occurs.
 * 
 * @date Feb 17, 2005
 */
public class SOAPException extends Exception{

    private static final long serialVersionUID     = 7322879413529566297L;
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

