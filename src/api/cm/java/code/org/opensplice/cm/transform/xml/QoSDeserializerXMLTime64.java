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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;

import org.opensplice.cm.Time;
import org.opensplice.cm.qos.*;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;

/**
 * The XML implementation of an QoSDeserializer. It is capable of
 * transforming a serialized XML representation to a QoS object.
 *
 * @date Feb 1, 2005
 */
public class QoSDeserializerXMLTime64 extends QoSDeserializerXML {

    /**
     * Creates a new QoSDeserializerXML object which is a concrete descendant
     * of the QoSDeserializer interface.
     *
     * @throws ParserConfigurationException Thrown when no DOM parser is
     *                                      available in the standard Java API.
     */
    public QoSDeserializerXMLTime64() throws ParserConfigurationException{
        super();
    }

    @Override
    protected Time parseDuration(String name, Element e){
        Time result = null;

        Node node = this.getChildNode(e, name);
        if(node != null){
            long value = Long.parseLong(node.getFirstChild().getNodeValue());

            result = new Time(value);
        }
        return result;
    }
}
