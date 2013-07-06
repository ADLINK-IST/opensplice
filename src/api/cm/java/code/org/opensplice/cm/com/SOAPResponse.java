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

import org.w3c.dom.Document;
import org.w3c.dom.Node;

/**
 * Concrete SOAP message, which represents a SOAP response. SOAP responses are 
 * received from a SOAP service when a SOAPRequest was sent.
 * 
 * @date Feb 17, 2005 
 */
class SOAPResponse extends SOAPMessage{
    
    /**
     * Construts a new SOAPResponse 
     *  
     *
     * @param dom The XML DOM tree representation of the response.
     */
    public SOAPResponse(Document dom){
        super();
        Node node = dom.getDocumentElement().getFirstChild().getFirstChild().getFirstChild();
        
        if(node != null){
            node = node.getFirstChild();
            body = node.getNodeValue();
        } else {
            body = null;
        }
    }
}
