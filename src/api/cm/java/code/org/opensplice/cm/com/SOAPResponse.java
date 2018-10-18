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
