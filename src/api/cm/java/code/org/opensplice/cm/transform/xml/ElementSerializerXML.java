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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Class that provides facilities to transform a DOM Element to its String
 * representation.
 * 
 * @date Apr 5, 2005
 */
public class ElementSerializerXML {
    /**
     * Transforms the supplied DOM tree Element into its String representation
     * (recursively).
     * 
     * @param element
     *            The element to serialize.
     * @return The String representation of the Element.
     */
    public String serializeElement(Element element){
        StringWriter strWriter = new StringWriter();
        NodeList children = element.getChildNodes();

        strWriter.write("<" + element.getTagName() + ">");

        for(int i=0; i<children.getLength(); i++){
            this.writeNode(strWriter, children.item(i));
        }
        strWriter.write("</" + element.getTagName() + ">");
        strWriter.flush();
        return strWriter.toString();
    }

    private void writeNode(StringWriter stringWriter, Node node){
        switch(node.getNodeType()){
            case Node.ATTRIBUTE_NODE:
                stringWriter.write(" " + node.getNodeName() + "=\"" + node.getNodeValue() + "\"");
                break;
            case Node.ELEMENT_NODE:
                stringWriter.write("<" + node.getNodeName());
                NamedNodeMap nnm = node.getAttributes();

                for(int i=0; i<nnm.getLength(); i++){
                    this.writeNode(stringWriter, nnm.item(i));
                }
                stringWriter.write(">");

                NodeList list = node.getChildNodes();

                for(int i=0; i<list.getLength(); i++){
                    this.writeNode(stringWriter, list.item(i));
                }
                stringWriter.write("</" + node.getNodeName() + ">");
                break;
            case Node.TEXT_NODE:
                this.encodeString(node.getNodeValue(), stringWriter);
                break;
            case Node.CDATA_SECTION_NODE:
                stringWriter.write("<![CDATA[" + node.getNodeValue() + "]]>");
                break;
            default:
                break;
        }
    }

    private void encodeString(String value, StringWriter stringWriter){
        char c;
        String str;

        for(int i=0; i<value.length(); i++){
            c = value.charAt(i);

            switch(c){
            case '<':
                stringWriter.write("&lt;");
                break;
            case '>':
                stringWriter.write("&gt;");
                break;
            case '"':
                stringWriter.write("&quot;");
                break;
            case '&':
                str = value.substring(i);

                if(str.startsWith("&amp;")){
                    i += 4;
                    stringWriter.write("&amp;amp;");
                } else if(str.startsWith("&lt;")){
                    i += 3;
                    stringWriter.write("&amp;lt;");
                } else if(str.startsWith("&gt;")){
                    i += 3;
                    stringWriter.write("&amp;gt;");
                } else if(str.startsWith("&quot;")){
                    i += 5;
                    stringWriter.write("&amp;quot;");
                } else {
                    stringWriter.write("&amp;");
                }
                break;
            default:
                stringWriter.write(c);
                break;
            }
        }
    }
}
