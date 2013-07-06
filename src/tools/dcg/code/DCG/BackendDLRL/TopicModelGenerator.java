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
package DCG.BackendDLRL;

import java.util.Vector;
import java.io.File;
import java.io.BufferedWriter;
import java.io.FileWriter;
import DCG.DCGUtilities.IDLTraverser;
import DCG.DCGUtilities.MappingXMLTraverser;
import java.util.Enumeration;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;
import org.openorb.compiler.idl.reflect.idlType;
import org.openorb.compiler.idl.reflect.idlObject;
import org.openorb.compiler.object.IdlArray;
import org.openorb.compiler.object.IdlModule;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlIdent;
import org.openorb.compiler.object.IdlStateMember;
import org.openorb.compiler.object.IdlValue;
import org.openorb.compiler.object.IdlTypeDef;
import org.openorb.compiler.object.IdlUnion;
import org.openorb.compiler.object.IdlSequence;
import org.openorb.compiler.object.IdlStruct;
import org.openorb.compiler.object.IdlString;
import org.openorb.compiler.object.IdlEnum;
import org.openorb.compiler.object.IdlSimple;
import org.openorb.compiler.parser.SymboleJava;
import org.openorb.compiler.parser.SymboleDef;

/**
 * This class will generate any missing mapping information into the existing parsed mapping information.
 * Each newly inserted element will be marked with a special attribute to be able to distinguish between
 * parsed and newly inserted information.
 */
public class TopicModelGenerator {

    /**
     * The vector containing the Mapping XML syntax tree
     */
    private Document mappingXML;

    /**
     * The vector containing the DLRL IDL syntax tree
     */
    private Vector rootDLRLIDL;

    /**
     * The vector containing the DCPS IDL syntax tree
     */
    private Vector rootDCPSIDL;

    private boolean fullOidDefault = false;

    private boolean verbose;

    private File outputDir;

    private int indent = 0;
    private BufferedWriter out = null;

    private boolean generatedMapped = false;

    /**
     * Specialized constructor
     *
     * @param rootDLRLIDL The source syntax tree of the DLRL IDL
     * @param mappingXML The source syntax tree of the Mapping XML
     */
    public TopicModelGenerator(Document mappingXML, Vector dlrlIDL, Vector dcpsIDL, boolean verbose, File outputDir,
                            boolean fullOidDefault){
        this.rootDLRLIDL = dlrlIDL;
        this.rootDCPSIDL = dcpsIDL;
        this.mappingXML = mappingXML;
        this.verbose = verbose;
        this.outputDir = outputDir;
        this.fullOidDefault = fullOidDefault;
    }

    public boolean isDefaultMappingUsed(
        )
    {
        return generatedMapped;
    }

    public void generateMissingMappingXMLInformation(
        boolean validateOnly) throws Exception{
        Vector dlrlValuetypes;
        int count;
        IdlValue value;
        String nameFQ;
        Element classMapping;
        Element mainTopic;
        File outputFile;
        String fileName;
        Element element;

        /* step 1: get all (non forward) valuetypes which (eventually) inherit from DDS::ObjectRoot */
        dlrlValuetypes = IDLTraverser.getAllNonForwardDLRLValuetypes(rootDLRLIDL);
        /* step 2: for each valuetype locate a classMapping element in the mapping XML */
        for(count = 0; count < dlrlValuetypes.size(); count++){
            value = (IdlValue)dlrlValuetypes.get(count);
            nameFQ = IDLTraverser.getIDLObjectFullyqualifiedName(value);
            classMapping = MappingXMLTraverser.findClassMappingwithName(mappingXML, nameFQ);
            /* step 3: if no classMapping element can be found. then generate it and all contents */
            if(classMapping == null){
                generateClassMapping(value);
                generatedMapped = true;
            } else {
                /* step 4: if a classMapping element was found then we need to check if the classMapping contains
                 * a mainTopic
                 */
                mainTopic = MappingXMLTraverser.getMainTopicElementForClassMapping(classMapping);
                /* step 5: if no mainTopic element was found, then complete the classMapping, if the mainTopic was found
                 * then ignore this valuetype */
                if(mainTopic == null){
                    completeClassMapping(value, classMapping);
                    generatedMapped = true;
                }//else ignore
            }
        }
        if(generatedMapped && !validateOnly){
            element = mappingXML.getDocumentElement();
            fileName = MappingXMLTraverser.getNodeValue(element.getAttributes(), MappingXMLTraverser.dlrlNameAttributeID);
            if(fileName == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace(element, "Missing required '"+
                                                            MappingXMLTraverser.dlrlNameAttributeID+"' attribute."));
            }
            outputFile = new File(outputDir.getAbsolutePath()+File.separator+fileName+".xml");
            if(verbose){
                System.out.println("- Notification: Annotated the provided mapping XML file with missing mapping information. "+
                "Writing resulting file to '"+outputFile.getAbsolutePath()+"'.");
            }
            printTopicModel(outputFile);
        }
    }

    private void generateClassMapping(IdlValue value) throws Exception{
        String nameFQ;
        Element classMappingElement;
        Element parentElement;

        nameFQ = IDLTraverser.getIDLObjectFullyqualifiedName(value);
        classMappingElement = createElement(MappingXMLTraverser.classMappingElementID);
        classMappingElement.setAttribute(MappingXMLTraverser.classMappingNameAttributeID, nameFQ);

        completeClassMapping(value, classMappingElement);

        parentElement = mappingXML.getDocumentElement();
        parentElement.appendChild(classMappingElement);
    }

    private void completeClassMapping(IdlValue value, Element classMapping) throws Exception{
        Enumeration content;
        IdlObject childObject;

        /* We dont generate an extension topic, it's useless anyway and a spec issue TODO maybe gen it if the issue
         * gets rejected. Which issue? look and thou shall find :P
         */
        generateTopic(value, classMapping, null, MappingXMLTraverser.mainTopicElementID, "");

        /* We need to generate mapping for each statemember, that is if it is not already mapped! */
        content = value.content();
        while(content.hasMoreElements()){
            childObject = (IdlObject)content.nextElement();
            if(childObject instanceof IdlStateMember){
                generateStateMemberMapping(value, (IdlStateMember)childObject, classMapping);
            } // else ignore it
        }
    }

    /**
     * This method is responsible for generating a (main or extension) topic
     * element(with contents) and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax (only FullOid is
     * generated as value for the content attribute):
     * <!ELEMENT mainTopic (keyDescription)>
     * <!ATTLIST mainTopic name CDATA  #REQUIRED
     *                  typename CDATA #OPTIONAL>
     * or
     * <!ELEMENT extensionTopic (keyDescription)>
     * <!ATTLIST extensionTopic name CDATA  #REQUIRED
     *                      typename CDATA #OPTIONAL>
     * and
     * <!ELEMENT keyDescription (keyField*)>
     * <!ATTLIST keyDescription content (FullOid | SimpleOid | NoOid) #REQUIRED>
     *
     * @param object The source IDL object being processed
     * @param parentElement The parent element to which to append the output
     * @param topicID The string identifying the exact topic name (extension or main)
     * @throws Exception
     */
    private Element generateTopic(IdlObject object, Element parentElement, String insertName, String topicID,
                                                                            String keyFieldPreFix) throws Exception {
        String objectName;
        String objectNameFQ;
        Element topicElement;
        Element keyDescriptionElement;
        Element keyField;
        Text textField;
        String topicName;
        String topicTypeName;
        NodeList children;

        objectName = IDLTraverser.getIDLObjectName(object);
        objectNameFQ = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        /* create an element that holds the tag example: maintopic <mainTopic></mainTopic> */
        topicElement = createElement(topicID);
        if(insertName != null){
            topicName = objectNameFQ.replaceAll("::", "_")+"_"+insertName+"_topic";
            topicTypeName = objectNameFQ+"_"+insertName+"_topic";
        } else {
            topicName = objectNameFQ.replaceAll("::", "_")+"_topic";
            topicTypeName = objectNameFQ+"_topic";
        }
        topicElement.setAttribute(MappingXMLTraverser.topicElementNameAttributeID, topicName);
        topicElement.setAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID, topicTypeName);
        /* verify the typename we made up is actually unique, it might be in use already! Search the DLRL idl and DCPS
         * idl for any identifier with the name we just made.
         */
        validateTopicIdentifierUniqueness(topicTypeName);

        /* create keydescription to append to maintopic element <keyDescription content = "FullOid"></keyDescription> */
        keyDescriptionElement = createElement(MappingXMLTraverser.keyDescriptionElementID);
        if(generateWithFullOid()){
            keyDescriptionElement.setAttribute(MappingXMLTraverser.keyDescriptionContentAttributeID,
                                                MappingXMLTraverser.keyDescriptionContentAttributeFullOidID);
            generateKeyField(keyDescriptionElement, keyFieldPreFix+"typeName");
            generateKeyField(keyDescriptionElement, keyFieldPreFix+"oid");
        } else {
            keyDescriptionElement.setAttribute(MappingXMLTraverser.keyDescriptionContentAttributeID,
                                                MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID);
            generateKeyField(keyDescriptionElement, keyFieldPreFix+"oid");
        }
        topicElement.appendChild(keyDescriptionElement);

        /* insert the topic element as the first child node, for nice formatting purposes only */
        children = parentElement.getChildNodes();
        if(children.getLength() > 0){
            parentElement.insertBefore(topicElement, children.item(0));
        } else {
            parentElement.appendChild(topicElement);
        }
        //result (if full oid and for mainTopic):
        //<classMapping name="objectName">
        //      <mainTopic name="Foo" typename="Bar::Foo">
        //          <keyDescription content = "FullOid">
        //              <keyField>class</keyField>
        //              <keyField>oid</keyField>
        //          </keyDescription>
        //      </mainTopic>
        //</classMapping>
        return topicElement;
    }

    /**
     * This method will generate the appropiate mapping for a state member idl type.
     * This method will either generate the following elements:
     * - A local element; if it is contained as a local element in the original
     * mapping XML syntax tree
     * - A mono attribute element; if the attribute is a basic IDL type
     * - A multi attribute element; If a template def is defined and the pattern is
     * StrMap, IntMap or List and the itemType is not another defined valuetype in the
     * DLRL IDL
     * - A mono relation element; If a template def is defined and the pattern is Ref //TODO ID: 200
     * and the itemType is another defined valuetype in the DLRL IDL
     * - A multi relation element; If a template def is defined and the pattern is
     * StrMap, IntMap or List and the itemType is another defined valuetype in DLRL IDL
     *
     * @param member The IDL object to be processed as a state member
     * @param parentElement The parent element to which the generated XML elements should be
     * appended
     * @param owningValue The name of the IDL valuetype which owns the statemember that is processed
     * @throws Exception
     */
    private void generateStateMemberMapping(IdlValue owningValue, IdlStateMember member, Element parentElement)
                                                                                                    throws Exception {
        String attributeName;

        attributeName = IDLTraverser.getIDLObjectName(member);
        if(!isAttributeMapped(parentElement, attributeName)){
            mapStateMember(parentElement, owningValue, member);
        }
    }

    private void mapStateMember(Element parentElement, IdlValue owningValue, IdlStateMember member) throws Exception{
        String valuetypeNameFQ;
        String attributeName;
        IdlObject tmpAttributeType;
        IdlObject attributeType;
        int type;
        NamedNodeMap attributes;
        Node itemTypeNode;
        String itemType;
        Element templateDef;

        valuetypeNameFQ = IDLTraverser.getIDLObjectFullyqualifiedName(owningValue);
        attributeName = IDLTraverser.getIDLObjectName(member);
        attributeType = ((IdlObject)member.stateType()).final_object();

        /* resolved any typedefs/idl identities */
        while(attributeType instanceof IdlTypeDef){
            attributeType = (IdlObject)((IdlTypeDef)attributeType).original();
            if(attributeType instanceof IdlIdent){
                attributeType = (IdlObject)((IdlIdent)attributeType).original();
            }
        }
        if(attributeType instanceof IdlIdent){
            attributeType = ((IdlIdent)attributeType).internalObject();
        }

        /* lets process the statemember type now to determine what to generate */
        if(attributeType instanceof IdlValue){
            //options: mono/multi relation or multi attribute
            /* if it's forward, then first try to find a non forward variant */
            if(((IdlValue)attributeType).isForward()){
                /*try and get the non forward variant*/
                tmpAttributeType = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL,
                                                            IDLTraverser.getIDLObjectFullyqualifiedName(attributeType));
                if(tmpAttributeType != null){
                    attributeType = tmpAttributeType;
                }/*else could not find it, so dont replace it... */
            }
            /* check if still forward, if so we need to be able to find a templatedef */
            if(((IdlValue)attributeType).isForward()){
                //options left: multi relation/attribute
                //try to locate a template def
                templateDef = MappingXMLTraverser.findTemplateDefWithName(mappingXML,
                                                        IDLTraverser.getIDLObjectFullyqualifiedName(attributeType));
                if(templateDef == null){
                    throw new Exception("Valuetype '"+valuetypeNameFQ+"', attribute '"+attributeName+
                        "': Forward valuetype detected as type for this attribute, but no corresponding templateDef "+
                        "element could be located in the provided XML mapping. Note: when using typedefs in IDL you "+
                        "must provide the eventual name of the forward valuetype to which the typedef points towards "+
                        "in the templateDef definition.");
                }
                attributes = templateDef.getAttributes();
                itemTypeNode = attributes.getNamedItem(MappingXMLTraverser.templateDefItemTypeAttributeID);
                if(itemTypeNode == null) {
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDef,
                        "Valuetype '"+valuetypeNameFQ+"', attribute '"+attributeName+
                        "': the templateDef element misses the '"+MappingXMLTraverser.templateDefItemTypeAttributeID+
                        "' attribute."));
                }
                itemType = MappingXMLTraverser.getNodeValue(itemTypeNode);
                if(itemType == null){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDef,
                        "Valuetype '"+valuetypeNameFQ+"', attribute '"+attributeName+
                        "': the templateDef element misses the '"+MappingXMLTraverser.templateDefItemTypeAttributeID+
                        "' attribute value."));
                }
                //multi relation/attribute
                int multiType = MappingXMLTraverser.validateTemplateDefItemType(rootDLRLIDL, templateDef, itemType);
                if(multiType ==MappingXMLTraverser.MULTI_RELATION){
                    generateMultiRelation(parentElement, valuetypeNameFQ, owningValue, attributeName, itemType,
                                                                                                        templateDef);
                } else if(multiType == MappingXMLTraverser.MULTI_ATTRIBUTE){
                    generateMultiAttribute(parentElement, attributeName, owningValue);
                }
            } else {
                //options left: mono relation
                if(!IDLTraverser.valuetypeInheritsFromObjectRoot((IdlValue)attributeType)){
                    throw new Exception("Valuetype '"+valuetypeNameFQ+"', attribute '"+attributeName+
                        "': The type indicates a valuetype which does not inherit from DDS::ObjectRoot. Either specify"+
                        " the '"+attributeName+"' attribute to be local in the XML or use a valuetype which does "+
                        "inherit from DDS::ObjectRoot as the type for this attribute.");
                }
                generateMonoRelation(parentElement, valuetypeNameFQ, owningValue, attributeName, (IdlValue)attributeType);
            }
        } else if( attributeType instanceof IdlUnion ||
                   attributeType instanceof IdlSequence ||
                   attributeType instanceof IdlStruct ||
                   attributeType instanceof IdlEnum ||
                   attributeType instanceof IdlString ||
                   attributeType instanceof IdlArray){
            generateMonoAttribute(parentElement, attributeName);
        } else if(attributeType instanceof IdlSimple){
            //options: mono attribute
            type = ((IdlSimple)attributeType).primitive();
            if(     type != IdlSimple.BOOLEAN && type != IdlSimple.CHAR &&
                    type != IdlSimple.DOUBLE && type != IdlSimple.FLOAT &&
                    type != IdlSimple.LONG && type != IdlSimple.LONGLONG &&
                    type != IdlSimple.OCTET && type != IdlSimple.SHORT &&
                    type != IdlSimple.ULONG && type != IdlSimple.ULONGLONG &&
                    type != IdlSimple.USHORT){
                throw new Exception("Valuetype '"+valuetypeNameFQ+"', attribute '"+attributeName+"': Unsupported type");
                    //unsupported are thus:IdlSimple.WCHAR, IdlSimple.ANY, IdlSimple.LONGDOUBLE, IdlSimple.OBJECT,
                    //IdlSimple.TYPECODE, IdlSimple.VALUEBASE, IdlSimple.VOID
            }//else do nothing
            generateMonoAttribute(parentElement, attributeName);
        } else {
            throw new Exception("Valuetype '"+valuetypeNameFQ+"', attribute '"+attributeName+"': Unsupported type");
            //unsupported are IdlFixed, IdlWString
        }
    }

    /**
     * This method is responsible for generating a mono attribute element(with
     * contents) and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax(placeTopic is never
     * generated, nor are multiple valuefields):
     * <!ELEMENT monoAttribute (placeTopic?,valueField+)>
     * <!ATTLIST monoAttribute name CDATA  #REQUIRED>
     *
     * @param object The IDL object representing the mono attribute
     * @param parentElement The parent element to which the generated XML elements
     * should be appended
     * @throws Exception
     */
    private void generateMonoAttribute(Element parentElement, String attributeName) throws Exception {
        Element monoAttributeElement;

        //create mono attribute element and append it to the class mapping element
        monoAttributeElement = createElement(MappingXMLTraverser.monoAttributeElementID);
        monoAttributeElement.setAttribute(MappingXMLTraverser.monoAttributeNameAttributeID, attributeName);

        //create value field element
        generateValueField(monoAttributeElement, attributeName);
        parentElement.appendChild(monoAttributeElement);
    }

    /**
     * This method is responsible for generating a valueField element(with contents)
     * and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax:
     * <!ELEMENT valueField (#PCDATA)>
     *
     * @param parentElement The parent element to which the generated XML elements
     * should be appended
     * @param objectName The value of the valueField
     */
    private void generateValueField(Element parentElement, String attributeName) {
        Element valueFieldElement;
        Text valueFieldText;

        valueFieldElement = createElement(MappingXMLTraverser.valueFieldElementID);
        valueFieldText = mappingXML.createTextNode(attributeName);
        valueFieldElement.appendChild(valueFieldText);
        parentElement.appendChild(valueFieldElement);
    }

    /**
     * This method is responsible for generating a mono relation element(with
     * contents) and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax(place topic is never
     * generated) only SimpleOid is generated as value for the content attribute):
     * <!ELEMENT monoRelation (placeTopic?,keyDescription)>
     * <!ATTLIST monoRelation name CDATA  #REQUIRED>
     * <!ELEMENT keyDescription (keyField*)>
     * <!ATTLIST keyDescription content (FullOid | SimpleOid | NoOid) #REQUIRED>
     *
     * @param object The IDL object representing the mono relation attribute
     * @param parentElement The parent element to which the generated XML elements
     * should be appended
     */
    private void generateMonoRelation(Element parentElement, String owningValuetypeNameFQ, IdlValue owningValue,
                                    String attributeName, IdlValue targetValue) throws Exception{
        Element monoRelationElement;
        Element mainTopic;
        Element classMapping;
        Element keyDescriptionElement;
        String contentValue;
        Element tmpKeydesElement = null;
        String className = attributeName+"_typeName";
        String oidName = attributeName+"_oid";
        NodeList keyFields;
        int count;
        Element aKeyField;
        String keyFieldName;

        //create mono relation element and append it to the parent element
        monoRelationElement = createElement(MappingXMLTraverser.monoRelationElementID);
        monoRelationElement.setAttribute(MappingXMLTraverser.monoRelationNameAttributeID, attributeName);

        keyDescriptionElement = createElement(MappingXMLTraverser.keyDescriptionElementID);

        /*need to determine the details of the keydescription and its child elements*/
        classMapping = MappingXMLTraverser.findClassMappingwithName(mappingXML,
                                                            IDLTraverser.getIDLObjectFullyqualifiedName(targetValue));
        if(classMapping != null){
            mainTopic = MappingXMLTraverser.getMainTopicElementForClassMapping(classMapping);
            if(mainTopic != null){
                tmpKeydesElement = MappingXMLTraverser.getKeydescriptionElement(mainTopic);
            }
        }
        if(tmpKeydesElement != null){
            contentValue = MappingXMLTraverser.getNodeValue(tmpKeydesElement.getAttributes(),
                                                                MappingXMLTraverser.keyDescriptionContentAttributeID);
            keyFields = MappingXMLTraverser.getXMLAllChildElements(tmpKeydesElement,
                                                                                MappingXMLTraverser.keyFieldElementID);
            for(count = 0; count < keyFields.getLength(); count++){
                aKeyField = (Element)keyFields.item(count);
                keyFieldName = attributeName+"_"+MappingXMLTraverser.getKeyFieldName(aKeyField);
                validateAttributeNameUniqueness(owningValue, keyFieldName, owningValuetypeNameFQ, attributeName);
                generateKeyField(keyDescriptionElement, keyFieldName);
            }
        } else if(generateWithFullOid()){
            contentValue = MappingXMLTraverser.keyDescriptionContentAttributeFullOidID;
            validateAttributeNameUniqueness(owningValue, className, owningValuetypeNameFQ, attributeName);
            generateKeyField(keyDescriptionElement, className);
            validateAttributeNameUniqueness(owningValue, oidName, owningValuetypeNameFQ, attributeName);
            generateKeyField(keyDescriptionElement, oidName);
        } else {
            contentValue = MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID;
            validateAttributeNameUniqueness(owningValue, oidName, owningValuetypeNameFQ, attributeName);
            generateKeyField(keyDescriptionElement, oidName);
        }
        keyDescriptionElement.setAttribute(MappingXMLTraverser.keyDescriptionContentAttributeID, contentValue);

        monoRelationElement.appendChild(keyDescriptionElement);

        parentElement.appendChild(monoRelationElement);
    }

    /**
     * This method is responsible for generating a multi relation element(with
     * contents) and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax  (only SimpleOid is
     * generated as value for the content attribute):
     * <!ELEMENT multiRelation (multiPlaceTopic,keyDescription)>
     * <!ATTLIST multiRelation name CDATA  #REQUIRED>
     * <!ELEMENT keyDescription (keyField*)>
     * <!ATTLIST keyDescription content (FullOid | SimpleOid | NoOid) #REQUIRED>
     *
     * @param object The IDL object representing the multi relation attribute
     * @param parentElement The parent element to which the generated XML elements
     * should be appended
     */
    private void generateMultiRelation(Element parentElement, String valuetypeNameFQ, IdlValue owningValue,
                            String attributeName, String targetNameFQ, Element templateDef) throws Exception{
        Element multiRelationElement;
        Element keyDescriptionElement;
        Element classMapping;
        Element mainTopic;
        Element tmpKeydesElement = null;
        String contentValue;
        String pattern;
        Element topicElement;

        multiRelationElement = createElement(MappingXMLTraverser.multiRelationElementID);
        multiRelationElement.setAttribute(MappingXMLTraverser.multiRelationNameAttributeID, attributeName);

        //generate a multiplace topic with the multi relation element as parent element
        topicElement = generateTopic(owningValue, multiRelationElement, attributeName,
                                                                MappingXMLTraverser.multiPlaceTopicElementID, "owner_");
        pattern = MappingXMLTraverser.getNodeValue(templateDef.getAttributes(),
                                                                    MappingXMLTraverser.templateDefPatternAttributeID);
        if(pattern.equals(MappingXMLTraverser.templateDefPatternAttributeStrMapValueID)){
                topicElement.setAttribute(MappingXMLTraverser.topicElementIndexFieldAttributeID, "index");
                topicElement.setAttribute(MappingXMLTraverser.topicElementIndexFieldTypeAttributeID, "string");
        } else if(pattern.equals(MappingXMLTraverser.templateDefPatternAttributeIntMapValueID) ||
                pattern.equals(MappingXMLTraverser.templateDefPatternAttributeListValueID)){
                topicElement.setAttribute(MappingXMLTraverser.topicElementIndexFieldAttributeID, "index");
                topicElement.setAttribute(MappingXMLTraverser.topicElementIndexFieldTypeAttributeID, "long");
        }//else do nothing
        //create key description element and append it to the multi relation element
        keyDescriptionElement = createElement(MappingXMLTraverser.keyDescriptionElementID);

        classMapping = MappingXMLTraverser.findClassMappingwithName(mappingXML, targetNameFQ);
        if(classMapping != null){
            mainTopic = MappingXMLTraverser.getMainTopicElementForClassMapping(classMapping);
            if(mainTopic != null){
                tmpKeydesElement = MappingXMLTraverser.getKeydescriptionElement(mainTopic);
            }
        }
        if(tmpKeydesElement != null){
            contentValue = MappingXMLTraverser.getNodeValue(tmpKeydesElement.getAttributes(),
                                                                MappingXMLTraverser.keyDescriptionContentAttributeID);
            NodeList keyFields = MappingXMLTraverser.getXMLAllChildElements(tmpKeydesElement, MappingXMLTraverser.keyFieldElementID);
            for(int count = 0; count < keyFields.getLength(); count++){
                Element aKeyField = (Element)keyFields.item(count);
                String keyFieldName = "target_"+MappingXMLTraverser.getKeyFieldName(aKeyField);
                generateKeyField(keyDescriptionElement, keyFieldName);
            }
        } else if(generateWithFullOid()){
            contentValue = MappingXMLTraverser.keyDescriptionContentAttributeFullOidID;
            generateKeyField(keyDescriptionElement, "target_typeName");
            generateKeyField(keyDescriptionElement, "target_oid");
        } else {
            contentValue = MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID;
            generateKeyField(keyDescriptionElement, "target_oid");
        }
        keyDescriptionElement.setAttribute(MappingXMLTraverser.keyDescriptionContentAttributeID, contentValue);
        multiRelationElement.appendChild(keyDescriptionElement);
        //finaly insert the newly generated multiRelation into the document.
        parentElement.appendChild(multiRelationElement);
    }

    /**
     * This method is responsible for generating a multi attribute element(with
     * contents) and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax:
     * <!ELEMENT multiAttribute (multiPlaceTopic,valueField+)>
     * <!ATTLIST multiAttribute name CDATA  #REQUIRED>
     *
     * @param object The IDL object representing the multi attribute
     * @param parentElement The parent element to which the generated XML elements
     * should be appended
     * @throws Exception
     */
    private void generateMultiAttribute(Element parentElement, String attributeName, IdlValue owningValue)
                                                                                                    throws Exception{
        Element multiAttributeElement;
        Element keyDescriptionElement;
        Element classMapping;

        multiAttributeElement = createElement(MappingXMLTraverser.multiAttributeElementID);
        multiAttributeElement.setAttribute(MappingXMLTraverser.multiAttributeNameAttributeID, attributeName);

        //generate a multiplace topic with the multi relation element as parent element
        generateTopic(owningValue, multiAttributeElement, attributeName, MappingXMLTraverser.multiPlaceTopicElementID,
                                                                                                            "owner_");
        generateValueField(multiAttributeElement, "element_"+attributeName);
        parentElement.appendChild(multiAttributeElement);
    }

    /**
     * This method is responsible for generating a keyfield element (with contents)
     * and appending it to the provided parent element.
     * The output XML will adhere to the following DTD syntax:
     * <!ELEMENT keyField (#PCDATA)>
     *
     * @param parentElement The parent element to which the generated XML elements
     * should be appended
     * @param objectName The value of the key field
     */
    private void generateKeyField(Element parentElement, String keyFieldName) {
        Element keyFieldElement = createElement(MappingXMLTraverser.keyFieldElementID);
        Text keyFieldText = mappingXML.createTextNode(keyFieldName);
        keyFieldElement.appendChild(keyFieldText);
        parentElement.appendChild(keyFieldElement);
    }

    private boolean isAttributeMapped(Element classMapping, String attributeName){
        if(MappingXMLTraverser.getAttributeMapping(classMapping, attributeName) != null){
            return true;
        }
        return false;
    }

    private boolean generateWithFullOid(){
        return fullOidDefault;
    }

    private Element createElement(String ElementID){
        Element element;

        element = mappingXML.createElement(ElementID);
        element.setAttribute(MappingXMLTraverser.isDefaultAttributeID, MappingXMLTraverser.isDefaultAttributeTrueValue);
        return element;
    }

    //used to validate for monoRelations
    private void validateAttributeNameUniqueness(IdlValue value, String name, String owningValuetypeNameFQ,
                                                                    String validatedAttributeName) throws Exception{
        IdlObject childObject;
        String attributeName;
        java.util.Enumeration content;

        content = value.content();
        while(content.hasMoreElements()){
            childObject = (IdlObject)content.nextElement();
            if(childObject instanceof IdlStateMember){
                attributeName = IDLTraverser.getIDLObjectName((IdlStateMember)childObject);
                if(name.equals(attributeName)){
                    IdlObject attributeType = ((IdlObject)((IdlStateMember)childObject).stateType()).final_object();
                    while(attributeType instanceof IdlTypeDef){
                        attributeType = (IdlObject)((IdlTypeDef)attributeType).original();
                        if(attributeType instanceof IdlIdent){
                            attributeType = (IdlObject)((IdlIdent)attributeType).original();
                        }
                    }
                    if(attributeType instanceof IdlIdent){
                        attributeType = ((IdlIdent)attributeType).internalObject();
                    }
                    if(attributeType instanceof IdlValue && ((IdlValue)attributeType).isForward()){
                        IdlValue tmpAttributeType = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL,
                                                            IDLTraverser.getIDLObjectFullyqualifiedName(attributeType));
                        if(tmpAttributeType != null){
                            attributeType = tmpAttributeType;
                        }
                    }
                    if(!((attributeType instanceof IdlValue) && (IDLTraverser.valuetypeInheritsFromObjectRoot((IdlValue)attributeType)))){
                        throw new Exception("Valuetype '"+owningValuetypeNameFQ+"', attribute '"+validatedAttributeName+
                          "': Default mapping to monoRelation of this attribute failed! Name clash detected while "+
                          "generating the foreign keyFields needed to describe the monoRelation. The name '"+
                          name+"' already identifies another (mono) attribute in the valuetype. Please "+
                          "resolve this name clash");

                    }
                }
            }// else ignore it
        }
    }

    private void validateTopicIdentifierUniqueness(String topicTypeName) throws Exception{
        if(IDLTraverser.doesIdlObjectWithFullyQualifiedNameExist(rootDLRLIDL, IDLTraverser.getAllIdlTypes(), topicTypeName)){
            throw new Exception("Name clash detected. Tried to generate a topic with name '"+topicTypeName+
                "', but this name is already in use within the provided DLRL IDL!");
        } else if(IDLTraverser.doesIdlObjectWithFullyQualifiedNameExist(rootDCPSIDL, IDLTraverser.getAllIdlTypes(), topicTypeName)){
            throw new Exception("Name clash detected. Tried to generate a topic with name '"+topicTypeName+
                "', but this name is already in use within the provided DCPS IDL!");
        }
    }

    /* specialized printing method, as we want to filter out some information from the
     * document and not print it to the file
     */
    private void printTopicModel(File outputFile) throws Exception{
        out = new BufferedWriter(new FileWriter(outputFile));
        writeHeader();
        Element element = mappingXML.getDocumentElement();
        writeElement(element);
        out.close();
    }

    private void writeHeader() throws Exception{
        write("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>");
        write("<!DOCTYPE Dlrl SYSTEM \"Dlrl.dtd\">");
    }

    private void writeElement(Element element) throws Exception{
        /* we do not write a new line for each element, because if this element only contains
         * text elements then we want to keep the element start tag and the element end tag on the same line
         * for readability purposes. the firstElement boolean is a simple mechanism to help achieve this.
         */
        boolean firstElement = true;
        write(MappingXMLTraverser.getElementFullyExpandedTagName(element), true, false);
        NodeList childNodes = element.getChildNodes();
        if(childNodes.getLength() != 0){
            indent++;
            for(int count = 0; count < childNodes.getLength(); count++){
                Node aNode = childNodes.item(count);
                if(aNode instanceof Element){
                    if(firstElement){
                        writeNewLine();
                        firstElement = false;
                    }
                    writeElement((Element)aNode);
                } else if(aNode instanceof org.w3c.dom.Text){
                    write(aNode.getTextContent().trim(), false, false);
                }
            }
            indent--;
            write("</"+element.getTagName()+">", !firstElement, true);
        } else {
            writeNewLine();
        }
    }

    private void write(String text) throws Exception {
        write(text, true, true);
    }

    private void write(String text, boolean writeIndent, boolean writeNewLine) throws Exception {
        if(writeIndent){
            for(int count = 0; count < indent; count++){
                out.write('\t');
            }
        }
        out.write(text.toString());
        if(writeNewLine){
            writeNewLine();
        }
    }

    private void writeNewLine() throws Exception{
        out.newLine();
    }
}
