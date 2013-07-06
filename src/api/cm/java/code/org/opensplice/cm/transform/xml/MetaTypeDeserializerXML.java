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

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.meta.MetaClass;
import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.cm.meta.MetaUnionCase;
import org.opensplice.cm.transform.MetaTypeDeserializer;
import org.opensplice.cm.transform.TransformationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * Offers facilities to deserialize a MetaType from XML to a Java representation.
 * For parsing DOM is used.
 *
 * @date May 25, 2004
 */
public class MetaTypeDeserializerXML implements MetaTypeDeserializer{
    /**
     * Constructs a MetaType deserializer that is capable of
     * transforming an XML type to a MetaType. This is a concrete implementation
     * of MetaTypeDeserializer.
     *
     * @throws ParserConfigurationException Thrown if the DocumentBuilderFactory
     *                                      could not be initialized.
     */
    public MetaTypeDeserializerXML() throws ParserConfigurationException{
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        builder = factory.newDocumentBuilder();
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }

    @Override
    public synchronized MetaType deserializeMetaType(Object type) throws TransformationException, DataTypeUnsupportedException{
        Document document;
        Element rootElement = null;

        if(type == null){
            throw new TransformationException("Supplied type is not valid (1).");
        }

        if(type instanceof String){
            String xmlType = (String)type;
            metaType = new MetaType(xmlType);
            /*
            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML", "deserializeMetaType",
                                                   "Deserializing UserDataType from string:\n" +
                                                   xmlType);
            */
            try {
                document = builder.parse(new InputSource(new StringReader(xmlType)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                           "deserializeMetaType",
                                           "SAXException occurred, MetaType could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                           "deserializeMetaType",
                                           "IOException occurred, MetaType could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }

            rootElement = document.getDocumentElement();
        } else if(type instanceof Element){
            rootElement = (Element)type;
            ElementSerializerXML es = new ElementSerializerXML();
            String xmlType = es.serializeElement(rootElement);
            metaType = new MetaType(xmlType);
        } else {
            throw new TransformationException("Supplied type is not valid (2).");
        }
        Node scope = this.getChildNode(rootElement, "name");
        MetaField result = null;

        if(scope != null){
            scope = scope.getFirstChild();

            if(scope != null){
                result = this.parseType(scope.getNodeValue().trim(), rootElement);
                metaType.setField(result);
            }
        }
        boolean success = metaType.finalizeType();

        if((result == null) || (!success)){
            throw new DataTypeUnsupportedException("Data type could not be parsed.");
        }
        return metaType;
    }

    /**
     * Parses a type in the supplied element.
     *
     * @param name The name of the field.
     * @param typeElement The DOM tree element which contains the type.
     * @return The deserialized field.
     */
    private MetaField parseType(String name, Element typeElement){
        Node childNode = null;
        Node childNodeValue = null;
        String childName, childContent;
        MetaField result = null;

        if(name == null){
            return result;
        }
        childNode = this.getChildNode(typeElement, "name");

        if(childNode == null){
            logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                        "parseType",
                                        "Name not found");
            metaType.mayNotBeFinalized();
            return result;
        }
        if(childNode.getFirstChild() != null){
            childName = childNode.getFirstChild().getNodeValue().trim();
        } else {
            childName = null;
        }
        childNode = getChildNode(typeElement, "kind");

        if(childNode == null){
            logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                        "parseType",
                                        "Kind not found");
            metaType.mayNotBeFinalized();
            return result;
        }
        childNodeValue = childNode.getFirstChild();

        if(childNodeValue != null){
            childContent = childNodeValue.getNodeValue().trim();

            if("M_PRIMITIVE".equals(childContent)){
                result = this.parsePrimitive(name, typeElement);
            }
            else if("M_COLLECTION".equals(childContent)){
                result = this.parseCollection(name, childName, typeElement);
            }
            else if("M_ENUMERATION".equals(childContent)){
                result = this.parseEnumeration(name, childName, typeElement);
            }
            else if("M_STRUCTURE".equals(childContent)){
                result = this.parseStructure(name, typeElement);
            }
            else if("M_TYPEDEF".equals(childContent)){
                result = this.parseTypeDef(name, typeElement);
            }
            else if("M_UNION".equals(childContent)){
                result = this.parseUnion(name, typeElement);
            }
            else if("M_CLASS".equals(childContent)){
                result = this.parseClass(name, typeElement);
            }
            else{
                logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                                        "parseType",
                                                        "-Unsupported type (" +
                                                        childContent + "): " +
                                                        childName);
                metaType.mayNotBeFinalized();
            }
        }
        return result;
    }

    /**
     * Parses the class in the supplied element.
     *
     * @param name The name of the class.
     * @param typeElement The DOM tree element which contains the class.
     * @return The deserialized class.
     */
    private MetaClass parseClass(String fieldName, Element typeElement) {
        NodeList scope = typeElement.getChildNodes();
        Node elementNode, elementContentNode, classNode, childNode;
        NodeList memberNodeList;
        ArrayList members = new ArrayList();
        MetaClass result = null;
        Element scopeElement = null;
        Element extendsElement = null;
        String classTypeName = null;

        for(int i=0; i<scope.getLength(); i++){
            classNode = scope.item(i);

            if("scope".equals(classNode.getNodeName())){
                scopeElement = (Element)classNode;
            }
            else if("name".equals(classNode.getNodeName())){
                classTypeName = classNode.getFirstChild().getNodeValue().trim();
            }
            else if("extends".equals(classNode.getNodeName())){
                extendsElement = (Element)classNode;
            }
        }
        if(extendsElement != null){
            childNode = getChildNode(extendsElement, "name");

            if(childNode != null){
                childNode = childNode.getFirstChild();

                if(childNode != null){
                    String className = childNode.getNodeName();
                    MetaClass cls = this.parseClass(className, extendsElement);
                    MetaField[] fields = cls.getFields();

                    for(int i=0; i<fields.length; i++){
                        members.add(fields[i]);
                    }
                }
            }
        }

        if(scopeElement != null){
            NodeList elements = scopeElement.getChildNodes();

            for(int i=0; i<elements.getLength(); i++){
                elementNode = elements.item(i);
                String name = null;

                if("element".equals(elementNode.getNodeName())){
                    memberNodeList = elementNode.getChildNodes();

                    for(int j=0; j<memberNodeList.getLength(); j++){
                        elementContentNode = memberNodeList.item(j);

                        if("name".equals(elementContentNode.getNodeName())){
                            name = elementContentNode.getFirstChild().getNodeValue().trim();
                        }
                        else if("type".equals(elementContentNode.getNodeName())){
                            MetaField temp = this.parseType(/*fieldName + "." +*/ name, (Element)elementContentNode);
                            members.add(temp);
                        }
                    }
                }
            }
            result = new MetaClass(fieldName, classTypeName, members);
        }
        return result;
    }

    /**
     * Parses the collection in the supplied element.
     *
     * @param fieldName The name of the collection.
     * @param collectionName The type name of the collection.
     * @param typeElement The DOM tree element which contains the collection.
     * @return The deserialized collection.
     */
    private MetaCollection parseCollection(String fieldName, String collectionName, Element typeElement){
        NodeList typeChildNodes;
        Node typeChildNode;
        int maxSize = 0;
        MetaCollection udfc = null;

        typeChildNodes = typeElement.getChildNodes();

        for(int i=0; i<typeChildNodes.getLength(); i++){
            typeChildNode = typeChildNodes.item(i);

            if(typeChildNode instanceof Element){
                if("maxSize".equals(typeChildNode.getNodeName())){
                    String tmp = typeChildNode.getFirstChild().getNodeValue();
                    if (tmp != null) {
                        try {
                            maxSize = Integer.parseInt(tmp.trim());
                        } catch (NumberFormatException nfe) {
                            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML", "parseCollection",
                                    "Unsupported integer found:" + tmp);
                        }
                    }
                }
                else if("subType".equals(typeChildNode.getNodeName())){
                    Node subType = this.getChildNode(typeChildNode, "kind");
                    if(subType != null){
                        Node subTypeValue = subType.getFirstChild();

                        if(subTypeValue != null){
                            String subTypeKind = subTypeValue.getNodeValue().trim();

                            if("M_PRIMITIVE".equals(subTypeKind)){
                                MetaPrimitive udfp = this.parsePrimitive("anonymous", (Element)typeChildNode);
                                udfc = new MetaCollection(fieldName, collectionName, maxSize, udfp);
                            }
                            else if("M_COLLECTION".equals(subTypeKind)){
                                Node nameNode = this.getChildNode(typeChildNode, "name");

                                if(nameNode != null){
                                    nameNode = nameNode.getFirstChild();

                                    if(nameNode != null){
                                        String subTypeName = nameNode.getNodeValue().trim();
                                        MetaField udf = this.parseCollection(fieldName + "[]", subTypeName, (Element)typeChildNode);
                                        udfc = new MetaCollection(fieldName, collectionName, maxSize, udf);
                                    }
                                }
                            } else if("M_TYPEDEF".equals(subTypeKind)){
                                udfc = new MetaCollection(fieldName, collectionName, maxSize, this.parseTypeDef(fieldName, (Element)typeChildNode));
                            } else if("M_STRUCTURE".equals(subTypeKind)){
                                udfc = new MetaCollection(fieldName, collectionName, maxSize, this.parseStructure(fieldName, (Element)typeChildNode));
                            } else if("M_UNION".equals(subTypeKind)){
                                udfc = new MetaCollection(fieldName, collectionName, maxSize, this.parseUnion(fieldName, (Element)typeChildNode));
                            } else if("M_ENUMERATION".equals(subTypeKind)){
                                Node enumNameNode = this.getChildNode(typeChildNode, "name");

                                if(enumNameNode != null){
                                    enumNameNode = enumNameNode.getFirstChild();

                                    if(enumNameNode != null){
                                        String enumName = enumNameNode.getNodeValue().trim();
                                        udfc = new MetaCollection(fieldName, collectionName, maxSize, this.parseEnumeration(fieldName, enumName, (Element)typeChildNode));
                                    } else {
                                        logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                                "parseCollection",
                                                "Unsupported enumeration collectionType(2) found:" +
                                                subTypeKind);
                                        metaType.mayNotBeFinalized();
                                    }
                                } else {
                                    logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                            "parseCollection",
                                            "Unsupported enumeration collectionType found:" +
                                            subTypeKind);
                                    metaType.mayNotBeFinalized();
                                }
                            }
                            else{
                                logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                                            "parseCollection",
                                                            "Unsupported collectionType found:" +
                                                            subTypeKind);
                                metaType.mayNotBeFinalized();
                            }
                        } else{
                            logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                    "parseCollection",
                                    "Unsupported recursive collection type found.");
                                metaType.mayNotBeFinalized();
                        }
                    } else {
                        /*Recursive type
                         */
                        logger.logp(Level.INFO,  "MetaTypeDeserializerXML",
                                "parseCollection",
                                "Recursive collection type found.");

                        MetaPrimitive prim = new MetaPrimitive(fieldName, typeChildNode.getFirstChild().getNodeValue());
                        udfc = new MetaCollection(fieldName, collectionName, -1, prim);
                    }
                }
            }
        }
        return udfc;
    }

    /**
     * Parses the primitive in the supplied element.
     *
     * @param fieldName The name of the field.
     * @param primitiveType The DOM tree element which contains the primitive.
     * @return The deserialized primitive.
     */
    private MetaPrimitive parsePrimitive(String fieldName, Element primitiveType){
        String primKind;

        Node nameNode = this.getChildNode(primitiveType, "name");

        if(nameNode != null){
            nameNode = nameNode.getFirstChild();

            if(nameNode != null){
                primKind = nameNode.getNodeValue().trim();
                return new MetaPrimitive(fieldName, primKind);
            } else {
                logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                        "parsePrimitive",
                        "-Unsupported primitve type found(2).");
                metaType.mayNotBeFinalized();
            }
        }
        else{
            logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                        "parsePrimitive",
                                        "-Unsupported primitve type found.");
            metaType.mayNotBeFinalized();
        }
        return null;
    }

    /**
     * Parses the structure in the supplied element.
     *
     * @param fieldName The name of the structure.
     * @param structType The DOM tree element which contains the structure.
     * @return The deserialized structure.
     */
    private MetaStruct parseStructure(String fieldName, Element structType){
        NodeList structContents = structType.getChildNodes();
        NodeList membersContents, memberNodeList;
        Node structNode, memberNode, memberContentNode;
        Node membersNode = null;
        String structTypeName = null;
        MetaStruct result = null;
        ArrayList members = new ArrayList();

        for(int i=0; i<structContents.getLength(); i++){
            structNode = structContents.item(i);

            if(structNode instanceof Element){
                if("members".equals(structNode.getNodeName())){
                    membersNode = structNode;
                }
                else if("name".equals(structNode.getNodeName())){
                    structTypeName = structNode.getFirstChild().getNodeValue().trim();
                }
            }
        }
        if(membersNode != null){
            membersContents = membersNode.getChildNodes();

            for(int i=0; i<membersContents.getLength(); i++){
                memberNode = membersContents.item(i);
                String name = null;

                if(memberNode instanceof Element){
                    if("element".equals(memberNode.getNodeName())){
                        memberNodeList = memberNode.getChildNodes();

                        for(int j=0; j<memberNodeList.getLength(); j++){
                            memberContentNode = memberNodeList.item(j);

                            if("name".equals(memberContentNode.getNodeName())){
                                if(memberContentNode.getFirstChild() != null){
                                    name = memberContentNode.getFirstChild().getNodeValue().trim();
                                } else {
                                    logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                            "parseStructure",
                                            "Unsupported structure type found.");
                                    metaType.mayNotBeFinalized();
                                }
                            }
                            else if("type".equals(memberContentNode.getNodeName())){
                                MetaField temp = this.parseType(/*fieldName + "." +*/ name, (Element)memberContentNode);
                                members.add(temp);
                            }
                        }
                    }
                }
            }
            result = new MetaStruct(fieldName, structTypeName, members);
        }
        return result;
    }

    /**
     * Parses the enumeration in the supplied element.
     *
     * @param fieldName The name of the enumeration.
     * @param typeName The type name of the enumeration.
     * @param enumType The DOM tree element which contains the enumeration.
     * @return The deserialized enumeration.
     */
    private MetaEnum parseEnumeration(String fieldName, String typeName, Element enumType){
        MetaEnum udfe = null;
        NodeList elements = enumType.getElementsByTagName("elements");

        if(elements.getLength()>0){
            Node element = elements.item(0);
            Node elementNode;
            NodeList elementList = element.getChildNodes();
            String[] posValues;
            String elementContent;
            int size, j;

            Node sizeNode = this.getChildNode(element, "size");

            if(sizeNode != null){
                sizeNode = sizeNode.getFirstChild();

                if(sizeNode != null){
                    size = Integer.parseInt(sizeNode.getNodeValue().trim());
                    posValues = new String[size];
                    j = 0;

                    for(int i=0; i<elementList.getLength(); i++){
                        elementNode = elementList.item(i);

                        if(elementNode instanceof Element){
                            if("element".equals(elementNode.getNodeName())){
                                if(elementNode.getFirstChild() != null){
                                    elementContent = elementNode.getFirstChild().getNodeValue().trim();
                                    posValues[j] = elementContent;
                                    j++;
                                }
                            }
                        }
                    }
                    udfe = new MetaEnum(fieldName, typeName, posValues);
                }
            }
        }
        return udfe;
    }

    /**
     * Parses the typedef in the supplied element.
     *
     * @param fieldName The name of the typedef.
     * @param typeDefType The DOM tree element which contains the typedef.
     * @return The deserialized field that is typedeffed, so not the typedef itself.
     */
    private MetaField parseTypeDef(String fieldName, Element typeDefType){
        MetaField result = null;
        Element alias = null;
        Element name = null;

        Node n = this.getChildNode(typeDefType, "alias");

        if(n instanceof Element){
            alias = (Element)n;
        }
        n = this.getChildNode(typeDefType, "name");

        if(n instanceof Element){
            name = (Element)n;
        }

        if((alias != null) && (name != null)){
            n = name.getFirstChild();

            if(n != null){
                String typeDefName = n.getNodeValue().trim();

                result = this.parseType(fieldName, alias);
                metaType.addTypedef(typeDefName, result);
            } else {
                logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                        "parseTypeDef",
                        "No typedef name found(2).");
                metaType.mayNotBeFinalized();
            }
        }
        else {
            logger.logp(Level.SEVERE,  "MetaTypeDeserializerXML",
                                        "parseTypeDef",
                                        "No typedef alias or name found.");
            metaType.mayNotBeFinalized();
        }
        return result;
    }

    /**
     * Parses the union in the supplied element.
     *
     * @param fieldName The name of the union.
     * @param unionType The DOM tree element which contains the union.
     * @return The deserialized union.
     */
    private MetaUnion parseUnion(String fieldName, Element unionType){
        MetaUnion unionEl = null;
        String typeName = null;
        Node nameNode = this.getChildNode(unionType, "name");


        if(nameNode != null){
            nameNode = nameNode.getFirstChild();

            if(nameNode != null){
                typeName = nameNode.getNodeValue().trim();
            } else {
                logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                        "parseUnion",
                        "Found union without typeName.");
               metaType.mayNotBeFinalized();
            }
        }
        else{
            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                        "parseUnion",
                                        "Found union without typeName.");
            metaType.mayNotBeFinalized();
        }
        nameNode = this.getChildNode(unionType, "switchType");

        if(nameNode != null){
            Element switchType = (Element)nameNode;
            MetaField switchField = this.parseUnionSwitchType(fieldName, switchType);

            if (switchField != null) {
                logger.logp(Level.FINEST, "MetaTypeDeserializerXML", "parseUnion", "Found union switchType: "
                        + switchField.toString());
            }
            nameNode = this.getChildNode(unionType, "cases");

            if(nameNode != null){
                Element cases = (Element)(nameNode);
                NodeList caseList = cases.getChildNodes();
                ArrayList caseArray = new ArrayList();
                MetaUnionCase caseField;
                Element c;

                for(int i=0; i<caseList.getLength(); i++){
                    if(caseList.item(i) instanceof Element){
                        c = (Element)(caseList.item(i));

                        if("element".equals(c.getNodeName())){
                            caseField = this.parseUnionCase(fieldName, c, switchField);

                            if(caseField != null){
                                caseArray.add(caseField);
                            }
                        }
                    }
                }
                unionEl = new MetaUnion(fieldName, typeName,
                                                     switchField, caseArray);
            }
            else{
                logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                            "parseUnion",
                                            "Found union without cases.");
                metaType.mayNotBeFinalized();
            }

        }
        else{
            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                        "parseUnion",
                                        "Found union without switchType.");
            metaType.mayNotBeFinalized();
        }
        return unionEl;
    }

    /**
     * Parses the discriminator of a union in the supplied element.
     *
     * @param unionName The name of the union.
     * @param type The DOM tree element which contains the discriminator.
     * @return The deserialized discriminator.
     */
    private MetaField parseUnionSwitchType(String unionName, Element type){
        MetaField udf = null;
        Node kindNode = this.getChildNode(type, "kind");

        if(kindNode != null){
            kindNode = kindNode.getFirstChild();

            if(kindNode != null){
                String kind = kindNode.getNodeValue().trim();
                String fieldName = "switch";

                if("M_TYPEDEF".equals(kind)){
                    Node aliasNode = this.getChildNode(type, "alias");
                    Node nameNode = this.getChildNode(type, "name");

                    if((aliasNode != null) && (nameNode != null)){
                        if((aliasNode instanceof Element) && (nameNode instanceof Element) ){
                            Element name = (Element)(nameNode);
                            String typeDefName = name.getFirstChild().getNodeValue().trim();

                            MetaField result = this.parseUnionSwitchType(unionName, (Element)aliasNode);
                            metaType.addTypedef(typeDefName, result);
                            return result;
                        } else {
                            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                    "parseUnionSwitchType",
                                    "Found union switchType without alias or name.");
                            metaType.mayNotBeFinalized();
                        }
                    }
                    else{
                        logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                                    "parseUnionSwitchType",
                                                    "Found union switchType without type.");
                        metaType.mayNotBeFinalized();
                    }
                }
                else if("M_PRIMITIVE".equals(kind)){
                    String primKind;
                    Node nameNode = this.getChildNode(type, "name");

                    if(nameNode != null){
                        nameNode = nameNode.getFirstChild();

                        if(nameNode != null){
                            primKind = nameNode.getNodeValue().trim();
                            udf = new MetaPrimitive(fieldName, primKind);
                        }
                    }
                }
                if("M_ENUMERATION".equals(kind)){
                    Node nameNode = this.getChildNode(type, "name");
                    String typeName = "anonymous";

                    if(nameNode != null){
                        nameNode = nameNode.getFirstChild();

                        if(nameNode != null){
                            typeName = nameNode.getNodeValue().trim();
                        }
                    }
                    Node element = this.getChildNode(type, "elements");

                    if(element != null){
                        Node elementNode;
                        NodeList elementList = element.getChildNodes();
                        String[] posValues;
                        String elementContent;
                        int size, j;
                        Node sizeNode = this.getChildNode(element, "size");

                        if(sizeNode != null){
                            sizeNode = sizeNode.getFirstChild();

                            if(sizeNode != null){
                                size = Integer.parseInt(sizeNode.getNodeValue().trim());
                                posValues = new String[size];

                                j = 0;

                                for(int i=0; i<elementList.getLength(); i++){
                                    elementNode = elementList.item(i);

                                    if(elementNode instanceof Element){
                                        if("element".equals(elementNode.getNodeName())){
                                            if(elementNode.getFirstChild() != null){
                                                elementContent = elementNode.getFirstChild().getNodeValue();
                                                posValues[j] = elementContent;
                                                j++;
                                            } else {
                                                logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                                        "parseUnionSwitchType",
                                                        "Found union switchType enumeration with error element.");
                                                metaType.mayNotBeFinalized();
                                            }
                                        }
                                    }
                                }
                                udf = new MetaEnum(fieldName, typeName, posValues);
                            } else {
                                logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                        "parseUnionSwitchType",
                                        "Found union switchType enumeration without size (2).");
                                metaType.mayNotBeFinalized();
                            }
                        } else {
                            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                    "parseUnionSwitchType",
                                    "Found union switchType enumeration without size.");
                            metaType.mayNotBeFinalized();
                        }
                    }
                }
            }
        }

        return udf;
    }

    /**
     * Parses the union case in the supplied element.
     *
     * @param unionName The name of the associated union.
     * @param type The DOM tree element which contains the union case.
     * @return The deserialized union case.
     */
    private MetaUnionCase parseUnionCase(String unionName, Element type, MetaField switchField){
        MetaField result;
        MetaUnionCase unionCase = null;
        Node nameNode = this.getChildNode(type, "name");

        if(nameNode == null){
            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                            "parseUnionCase",
                                            "Invalid union: Unioncase without name found.");
            metaType.mayNotBeFinalized();
        }
        else{
            nameNode = nameNode.getFirstChild();

            if(nameNode != null){
                String caseName = nameNode.getNodeValue().trim();
                Node typeNode = this.getChildNode(type, "type");

                if(typeNode != null){
                    Node kindNode = this.getChildNode(typeNode, "kind");

                    if(kindNode != null){
                        kindNode = kindNode.getFirstChild();

                        if(kindNode != null){
                            String caseType = kindNode.getNodeValue().trim();
                            logger.logp(Level.FINEST, "MetaTypeDeserializerXML",
                                    "parseUnionCase",
                                    "Found unioncase: " + unionName +
                                    "." + caseName + " of type " + caseType);
                            result = this.parseType(/*unionName + "." +*/ caseName, (Element)typeNode);

                            NodeList nodeList = type.getChildNodes();
                            boolean found = false;
                            Node labelNode = null;

                            for(int i=0; i<nodeList.getLength() && !found;i++){
                                labelNode = nodeList.item(i);
                                if("labels".equals(labelNode.getNodeName())){
                                    found = true;
                                }
                            }
                            ArrayList labels = new ArrayList();

                            if(found){
                                Element casesElement = (Element)labelNode;
                                Node elementNode, valueNode;
                                nodeList = casesElement.getChildNodes();

                                nodeList = casesElement.getElementsByTagName("element");
                                Node labelValueNode;
                                String label;

                                if(nodeList.getLength() == 0){ /*Default label*/
                                    label = "";
                                    labels.add(label);
                                } else {
                                    for(int i=0; i<nodeList.getLength(); i++){
                                        elementNode = nodeList.item(i);

                                        if("element".equals(elementNode.getNodeName())){
                                            valueNode = this.getChildNode(elementNode, "value");

                                            if(valueNode != null){
                                                labelValueNode = this.getLastChildElement(valueNode);

                                                if(labelValueNode != null){
                                                    labelValueNode = labelValueNode.getFirstChild();

                                                    if(labelValueNode != null){
                                                        label = labelValueNode.getNodeValue().trim();

                                                        if(switchField instanceof MetaEnum){
                                                            try{
                                                                int enumIndex = Integer.parseInt(label);
                                                                String[] values = ((MetaEnum)switchField).getPosValues();

                                                                if(values.length > enumIndex){
                                                                    label = new String(values[enumIndex]);
                                                                }

                                                            } catch(NumberFormatException nfe){
                                                                /* Apparently not a number,
                                                                 * assuming enum label is ok.
                                                                 */
                                                            }
                                                        }
                                                        labels.add(label);
                                                    } else {
                                                        logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                                                "parseUnionCase",
                                                                "Invalid union: Unioncase with invalid label found (2).");
                                                        metaType.mayNotBeFinalized();
                                                    }
                                                } else {
                                                    logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                                            "parseUnionCase",
                                                            "Invalid union: Unioncase with invalid label found.");
                                                    metaType.mayNotBeFinalized();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else{
                                logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                                            "parseUnionCase",
                                                            "Invalid union: Unioncase without labels found.");
                                metaType.mayNotBeFinalized();
                            }
                            unionCase = new MetaUnionCase(caseName, "case", result, labels);

                        } else {
                            logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                    "parseUnionCase",
                                    "Invalid union: Unioncase without type found.");
                            metaType.mayNotBeFinalized();
                        }
                    } else {
                        logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                "parseUnionCase",
                                "Invalid union: Unioncase without type found.");
                        metaType.mayNotBeFinalized();
                    }
                }
                else{
                    logger.logp(Level.SEVERE, "MetaTypeDeserializerXML",
                                                "parseUnionCase",
                                                "Invalid union: Unioncase without type found.");
                    metaType.mayNotBeFinalized();
                }
            }
        }
        return unionCase;
    }

    private Node getChildNode(Node node, String child){
        NodeList members = node.getChildNodes();
        Node childNode = null;

        if(child != null){
            for(int i=0; (i<members.getLength()) && (childNode == null); i++){
                if(child.equals(members.item(i).getNodeName())){
                    childNode = members.item(i);
                }
            }
        }
        return childNode;
    }
    /*
    private Element getFirstChildElement(Node parent){
        NodeList members = parent.getChildNodes();
        Element childNode = null;

        for(int i=0; (i<members.getLength()) && (childNode == null); i++){
            if(members.item(i) instanceof Element){
                childNode = (Element)members.item(i);
            }
        }
        return childNode;
    }
    */
    private Element getLastChildElement(Node parent){
        NodeList members = parent.getChildNodes();
        Element childNode = null;

        for(int i=0; i<members.getLength(); i++){
            if(members.item(i) instanceof Element){
                childNode = (Element)members.item(i);
            }
        }
        return childNode;
    }

    /**
     * Builder that takes care of creating a DOM tree of the XML input.
     */
    private final DocumentBuilder builder;

    /**
     * The result of the deserialization.
     */
    private MetaType metaType;

    /**
     * The logging mechanism.
     */
    private final Logger logger;
}
