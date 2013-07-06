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
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.File;
import DCG.Core.MainModel;
import org.w3c.dom.Element;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import DCG.DCGUtilities.MappingXMLTraverser;
import DCG.DCGUtilities.IDLTraverser;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlIdent;
import org.openorb.compiler.object.IdlSimple;
import org.openorb.compiler.object.IdlTypeDef;
import org.openorb.compiler.object.IdlString;
import org.openorb.compiler.object.IdlWString;
import org.openorb.compiler.object.IdlArray;
import org.openorb.compiler.object.IdlSequence;
import org.openorb.compiler.object.IdlEnum;
import org.openorb.compiler.object.IdlUnion;
import org.openorb.compiler.object.IdlValue;
import org.openorb.compiler.object.IdlStruct;
import org.openorb.compiler.object.IdlStateMember;
import org.openorb.compiler.idl.reflect.idlPrimitive;

public class DCPSIDLGenerator2{

    private Vector topics = new Vector();
    private int indent = 0;
    private BufferedWriter out = null;
    private File generatedDCPSFile = null;

    private Document mappingXML;
    private Vector rootDLRLIDL;
    private File dlrlIdlFile;
    private MainModel model;

    //TODO sort keyfields for a topic when generating keyList so that it is most efficient
    //TODO what happens if i map towards a DLRL object which has an inner struct and the field(s) in that inner struct
    //are the key fields
    public DCPSIDLGenerator2(
        Document mappingXML,
        Vector rootDLRLIDL,
        File dlrlIdlFile,
        MainModel model)
    {
        this.model = model;
        this.mappingXML = mappingXML;
        this.rootDLRLIDL = rootDLRLIDL;
        this.dlrlIdlFile = dlrlIdlFile;
    }

    public void writeDcpsIdl(
        ) throws Exception
    {
        String fileName;
        Element dlrlElement;
        File outputResult;


        collectInfo();
        if(topics.size() > 0)
        {
            dlrlElement = mappingXML.getDocumentElement();
            fileName = MappingXMLTraverser.getNodeValue(dlrlElement.getAttributes(), MappingXMLTraverser.dlrlNameAttributeID);
            if(fileName == null)
            {
                throw new Exception(MappingXMLTraverser.produceElementTrace(dlrlElement, "Missing required '"+
                                                            MappingXMLTraverser.dlrlNameAttributeID+"' attribute."));
            }
            outputResult = model.getOutputDirectoryPath();
            outputResult.mkdirs();
            generatedDCPSFile = new File(outputResult.getAbsolutePath()+File.separator+fileName+".idl");
            out = new BufferedWriter(new FileWriter(generatedDCPSFile));
            writeHeader();
            writeIncludes();
            writeTopics();
            out.close();
        }
    }

    public File getGeneratedDCPSFile(
        )
    {
        return generatedDCPSFile;
    }

    public void deleteGeneratedFile(
        )
    {
        if(out != null)
        {
            try
            {
                out.close();//if it was already closed, then this has no effect.
            } catch (java.io.IOException e)
            {
                if(model.getVerbose()){
                    System.out.println("- Warning: Failed to close input stream. Detailed info: \n");
                    e.printStackTrace();
                }
            }
        }
        if(generatedDCPSFile != null)
        {
            if (!generatedDCPSFile.delete())
            {
                if(model.getVerbose())
                {
                    System.out.println("- Notification: Could not delete generated DCPS IDL file '"+
                                                                                generatedDCPSFile.getName()+"'.");
                }
            }
            generatedDCPSFile = null;
        }
    }

    private void writeIncludes(
        ) throws Exception
    {
        write("#include \""+dlrlIdlFile.getName()+"\"", 2);
    }

    private void collectInfo(
        ) throws Exception
    {
        NodeList classMappings;
        NodeList mainTopics;
        Element element;
        Element mainTopic;
        int count;

        classMappings = mappingXML.getElementsByTagName(MappingXMLTraverser.classMappingElementID);
        for(count = 0; count < classMappings.getLength(); count++)
        {
            element = (Element)classMappings.item(count);
            mainTopics = element.getElementsByTagName(MappingXMLTraverser.mainTopicElementID);
            if(mainTopics.getLength() != 1)
            {
                throw new Exception(MappingXMLTraverser.produceElementTrace(element,
                    "Missing the required '"+MappingXMLTraverser.mainTopicElementID+"' element for this '"+
                    MappingXMLTraverser.classMappingElementID+"' element."));//error message does assume 1 maintopic max
            }
            mainTopic = (Element)mainTopics.item(0);
            if(MappingXMLTraverser.isElementDefault(mainTopic))
            {
                processTopic(element, mainTopic);
            }
        }
    }

    private void processTopic(
        Element classMapping,
        Element mainTopic) throws Exception
    {
        String topicName;
        DCPSTopic topic;
        NodeList elements;
        Element keyDescription;
        int count;
        Element anElement;
        String fieldName;
        DCPSField aField;
        String fieldType;

        topicName = MappingXMLTraverser.getNodeValue(mainTopic.getAttributes(), MappingXMLTraverser.mainTopicTypeNameAttributeID);
        if(topicName == null)
        {
            topicName = MappingXMLTraverser.getNodeValue(mainTopic.getAttributes(), MappingXMLTraverser.mainTopicNameAttributeID);
        }

        topic = new DCPSTopic(topicName);
        processKeyDescription(mainTopic, topic, DCPSField.DCPS_FIELD_KEY);

        resolveTopicFields(classMapping, topic);
        topics.add(topic);
    }

    //works for mainTopic child element
    private void processKeyDescription(
        Element element,
        DCPSTopic topic,
        int keyType) throws Exception
    {
        NodeList elements;
        Element keyDescription;
        String content;
        int count;
        Element anElement;
        String fieldName;
        String fieldType = null;
        DCPSField aField;

        keyDescription = MappingXMLTraverser.getKeydescriptionElement(element);
        if(keyDescription == null)
        {
            //error msg assumes max 1 keydescription
            throw new Exception(MappingXMLTraverser.produceElementTrace(element, "Missing required "+
                                                            MappingXMLTraverser.keyDescriptionElementID+" element."));
        }
        content = MappingXMLTraverser.getNodeValue(keyDescription.getAttributes(),
                                                                MappingXMLTraverser.keyDescriptionContentAttributeID);
        elements = keyDescription.getElementsByTagName(MappingXMLTraverser.keyFieldElementID);
        if(content.equals(MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID) && elements.getLength() != 1)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyDescription, true, "Detected a '"+
                MappingXMLTraverser.keyDescriptionContentAttributeID+"' value of '"+
                MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID+"' and with "+elements.getLength()+" '"+
                MappingXMLTraverser.keyFieldElementID+"' child elements. Require 1 child element."));
        }
        if(content.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID) && elements.getLength() != 2)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyDescription, true, "Detected a '"+
                MappingXMLTraverser.keyDescriptionContentAttributeID+"' value of '"+
                MappingXMLTraverser.keyDescriptionContentAttributeFullOidID+"' and with "+elements.getLength()+" '"+
                MappingXMLTraverser.keyFieldElementID+"' child elements. Require 2 child elements."));
        }
        /* create an identity for this keydescription, its an self containing key identity */
        KeyIdentity identity = null;
        if(keyType == DCPSField.DCPS_FIELD_KEY)
        {
            identity = new KeyIdentity();
        }
        for(count = 0; count < elements.getLength(); count++)
        {
            anElement = (Element)elements.item(count);
            fieldName = getKeyOrValuefieldName(anElement);
            Vector arraySizes = new Vector();
            if((content.equals(MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID)) ||
                        (content.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID) && count == 1))
            {
                fieldType = "DDS::DLRLOid";
            } else if(content.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID) && count == 0)
            {
                fieldType = "string";
            } else
            {
                if(element.getTagName().equals(MappingXMLTraverser.monoRelationElementID))
                {
                    fieldType = determinefieldTypeForMonoRelationKeyfield(element, anElement, arraySizes, count);
                } else if(element.getTagName().equals(MappingXMLTraverser.multiRelationElementID))
                {
                    fieldType = determinefieldTypeForMultiRelationKeyDescription(element, keyDescription, count, arraySizes);
                } else if(element.getTagName().equals(MappingXMLTraverser.mainTopicElementID))
                {
                    throw new Exception(MappingXMLTraverser.produceElementTrace(keyDescription, false,
                        "Detected a default generated "+MappingXMLTraverser.mainTopicElementID+" which has a "+
                        MappingXMLTraverser.keyDescriptionElementID+" with a "+
                        MappingXMLTraverser.keyDescriptionContentAttributeID+" type of "+
                        MappingXMLTraverser.keyDescriptionContentAttributeNoOidID+
                        ". This should never have occured"));
                } else if(element.getTagName().equals(MappingXMLTraverser.multiPlaceTopicElementID))
                {
                    throw new Exception(MappingXMLTraverser.produceElementTrace(keyDescription, false,
                        "Detected a default generated "+MappingXMLTraverser.multiPlaceTopicElementID+" which has a "+
                        MappingXMLTraverser.keyDescriptionElementID+" with a "+
                        MappingXMLTraverser.keyDescriptionContentAttributeID+" type of "+
                        MappingXMLTraverser.keyDescriptionContentAttributeNoOidID+
                        ". This should never have occured"));
                } else
                {
                    throw new Exception(MappingXMLTraverser.produceElementTrace(keyDescription,"Unexpected location of "+
                            MappingXMLTraverser.keyDescriptionElementID));
                }
            }
            aField = new DCPSField(fieldName, fieldType, keyType, arraySizes);
            topic.addField(aField);
            if(keyType == DCPSField.DCPS_FIELD_KEY)
            {
                identity.addField(aField);
            }
        }
        if(keyType == DCPSField.DCPS_FIELD_KEY)
        {
            topic.addIdentity(identity);
        }
    }

    private String determinefieldTypeForMultiRelationKeyDescription(
        Element multiRelation,
        Element ownerKeyDescription,
        int index,
        Vector arraySizes) throws Exception
    {
        String owningRelationName;
        IdlObject attributeType;
        String templateDefName;
        Element templateDef;
        String targetObjectName;

        /* step 1) we must also retrieve the name of the monoRelation itself */
        owningRelationName = MappingXMLTraverser.getNodeValue(multiRelation.getAttributes(),
                                                                    MappingXMLTraverser.multiRelationNameAttributeID);
        if(owningRelationName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(multiRelation, "Attribute '"+
                MappingXMLTraverser.multiRelationNameAttributeID+"' must be defined for this element."));
        }
        /* step 2) get the type of the located attribute, it should be an IdlValue (forward). Get it's fully qualified
         * name. resolve any typedefs/idents
         */
        attributeType = getAttributeTypeForClassMappingChildRelation(multiRelation, owningRelationName);
        /* step 3) Get the fully qualified name of the retrieved IdlObject which represents the type of the relation */
        templateDefName = IDLTraverser.getIDLObjectFullyqualifiedName(attributeType);

        /* step 4) find the templatedef with the name of the found fully qualified name */
        templateDef = MappingXMLTraverser.findTemplateDefWithName(mappingXML, templateDefName);
        /* step 5) get the itemtype of the templateDef and locate a non forward idl valuetype based on that name */
        targetObjectName = MappingXMLTraverser.getXMLElementAttributeValue(templateDef,
                                                                    MappingXMLTraverser.templateDefItemTypeAttributeID);
        if(targetObjectName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(templateDef, "Attribute '"+
                MappingXMLTraverser.templateDefItemTypeAttributeID+"' must be defined for this element."));
        }
        /* step 6) determine the type of the keyfield */
        return determineTypeForMainTopicKeyField(ownerKeyDescription, targetObjectName, index, arraySizes);
    }

    private String determineTypeForMainTopicKeyField(
        Element ownerKeyDescription,
        String classNameFQ,
        int index,
        Vector arraySizes) throws Exception
    {
        Element targetClassMapping;
        Element targetMainTopic;
        Element targetKeyDescription;
        NodeList elements;
        Element targetKeyField;
        String targetKeyFieldName;
        Element targetMonoAttribute;
        String targetAttributeName;

        /* step 1) based upon the name, find the classMapping which represents the found valuetype which is the target*/
        targetClassMapping = MappingXMLTraverser.findClassMappingwithName(mappingXML, classNameFQ);
        if(targetClassMapping == null)
        {
            throw new Exception("Unable to locate a classMapping for IDL valuetype '"+classNameFQ+
                "' in the mapping XML.");
        }
        /* step 2) Get the mainTopic, then the keydescription for the found classmapping */
        targetMainTopic = MappingXMLTraverser.getMainTopicElementForClassMapping(targetClassMapping);
        if(targetMainTopic == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(targetClassMapping, "Missing required "+
                MappingXMLTraverser.mainTopicElementID+" child element."));
        }
        targetKeyDescription = MappingXMLTraverser.getKeydescriptionElement(targetMainTopic);
        if(targetKeyDescription == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(targetClassMapping, "Missing required "+
                MappingXMLTraverser.keyDescriptionElementID+" child element."));
        }
        /* step 3) based upon the keydescription and the index of the keyfield in the owning monoRelation find the
         * corresponding keyfield and get it's name
         */
        elements = targetKeyDescription.getElementsByTagName(MappingXMLTraverser.keyFieldElementID);
        if(elements.getLength() < index)
        {
            throw new Exception("Mismatch in number of keyFields of the keyDescription definitions (should be equal "+
                "number of fields)!"+MappingXMLTraverser.produceElementTrace(ownerKeyDescription, true,
                "\nFirst keyDescription XML is as follows:")+MappingXMLTraverser.produceElementTrace(targetKeyDescription,
                true,"\nSecond keyDescription XML is as follows:"));
        }
        targetKeyField = (Element)elements.item(index);
        targetKeyFieldName = getKeyOrValuefieldName(targetKeyField);
        /* step 6) Based upon the name of the keyfield we just retrieved find the monoAttribute which maps that
         * keyField. Remember we are only dealing with keyDescriptions with content of NoOid which are required to
         * have monoAttribute mappings for each keyfield!
         * Once we have the monoAttribute, get the name
         */
        targetMonoAttribute = findMonoAttributeWhichMapsField(targetClassMapping, targetKeyFieldName);
        if(targetMonoAttribute == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(targetClassMapping, "Unable to locate a "+
                MappingXMLTraverser.monoAttributeElementID+" child element which maps "+
                MappingXMLTraverser.keyFieldElementID+" '"+targetKeyFieldName+"'."));
        }
        targetAttributeName = MappingXMLTraverser.getNodeValue(targetMonoAttribute.getAttributes(),
                                                                    MappingXMLTraverser.monoAttributeNameAttributeID);
        if(targetAttributeName == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(targetMonoAttribute, "Attribute '"+
                MappingXMLTraverser.monoRelationNameAttributeID+"' must be defined for this element."));
        }
        return determineFieldType(targetClassMapping, targetMonoAttribute, targetAttributeName, arraySizes);
    }


    private IdlObject getAttributeTypeForClassMappingChildRelation(
        Element element,
        String owningRelationName) throws Exception
    {
        Element owningClassMapping;
        String owningName;
        IdlObject owningObject;
        Object owningAttribute;
        IdlStateMember member;
        IdlObject attributeType;

        /* step 1) get the owning objects classmapping and retrieve the name specified there */
        owningClassMapping = (Element)element.getParentNode();
        owningName = MappingXMLTraverser.getNodeValue(owningClassMapping.getAttributes(),
                                                                    MappingXMLTraverser.classMappingNameAttributeID);
        if(owningName == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(owningClassMapping, "Attribute '"+
                MappingXMLTraverser.classMappingNameAttributeID+"' must be defined for this element."));
        }
        /* step 2) based upon the name of the classMapping we can retrieve IdlObject which the classMapping represents*/
        owningObject = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, owningName);
        if(owningObject == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(owningClassMapping, "No IDL Valuetype with name '"+
                                                                                owningName+"' found in the DLRL IDL."));
        }
        /* step 3) based upon the monoRelation of the xml, try and locate the corresponding idl statemember attribute */
        owningAttribute = IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, owningObject, owningRelationName);
        if(owningAttribute == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(element,
                "IDL Valuetype with name '"+owningName+"' does not contain attribute '"+owningRelationName+
                "' in the DLRL IDL."));
        }
        if(!(owningAttribute instanceof IdlStateMember))
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(element,
                "IDL Valuetype with name '"+owningName+"' and attribute '"+owningRelationName+
                "' in the DLRL IDL was not an IDL statemember as expected."));
        }
        member = (IdlStateMember)owningAttribute;
        /* step 4) resolve any typedefs and transform any IdlIdent into the final object */
        attributeType = ((IdlObject)member.stateType()).final_object();
        while(attributeType != null && attributeType instanceof IdlTypeDef)
        {
            attributeType = (IdlObject)((IdlTypeDef)attributeType).original();
            if(attributeType instanceof IdlIdent)
            {
                attributeType = (IdlObject)((IdlIdent)attributeType).original();
            }
        }
        if(attributeType instanceof IdlIdent)
        {
            attributeType = (IdlObject)((IdlIdent)attributeType).original();
        }
        if(!(attributeType instanceof IdlValue))
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(element,
                "The type of attribute '"+owningRelationName+" in IDL Valuetype '"+owningName+"' "+
                "'in the DLRL IDL was not a valuetype as expected."));
        }
        return attributeType;
    }

    private String determinefieldTypeForMonoRelationKeyfield(
        Element monoRelation,
        Element keyField,
        Vector arraySizes,
        int index) throws Exception
    {
        String owningRelationName;
        IdlObject attributeType;
        String targetObjectName;

        /* step 1) we must retrieve the name of the monoRelation itself */
        owningRelationName = MappingXMLTraverser.getNodeValue(monoRelation.getAttributes(),
                                                                    MappingXMLTraverser.monoRelationNameAttributeID);
        if(owningRelationName == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(monoRelation, "Attribute '"+
                MappingXMLTraverser.monoRelationNameAttributeID+"' must be defined for this element."));
        }
        /* step 2) get the type of the attribute of the mono Relation */
        attributeType = getAttributeTypeForClassMappingChildRelation(monoRelation, owningRelationName);
        /* step 3) Get the fully qualified name of the retrieved IdlObject which represents the type of the relation */
        targetObjectName = IDLTraverser.getIDLObjectFullyqualifiedName(attributeType);
        /* step 4) determine the type of the keyfield */
        return determineTypeForMainTopicKeyField((Element)keyField.getParentNode(), targetObjectName, index, arraySizes);
    }

    private Element findMonoAttributeWhichMapsField(
        Element classMapping,
        String name) throws Exception
    {
        NodeList elements;
        int count;
        Element anElement;
        NodeList tmpElements;
        Element valueField;
        String valueFieldName;

        elements = MappingXMLTraverser.getXMLAllChildElements(classMapping, MappingXMLTraverser.monoAttributeElementID);
        for(count = 0; count < elements.getLength(); count++)
        {
            anElement = (Element)elements.item(count);
            tmpElements = anElement.getElementsByTagName(MappingXMLTraverser.valueFieldElementID);
            if(tmpElements.getLength() != 1)
            {
                //error msg assumes max 1 valuefield
                throw new Exception(MappingXMLTraverser.produceElementTrace(anElement, "Detected '"+
                    tmpElements.getLength()+"' '"+MappingXMLTraverser.valueFieldElementID+
                    "' elements. Currently only 1 child elements of this type is supported"));
            }
            valueField = (Element)tmpElements.item(0);
            valueFieldName = getKeyOrValuefieldName(valueField);
            if(name.equals(valueFieldName))
            {
                return anElement;
            }
        }
        return null;
    }

    private String getKeyOrValuefieldName(
        Element field) throws Exception
    {
        Vector childNodes;
        Node childNode;

        childNodes = MappingXMLTraverser.filterNodeList(field.getChildNodes(), Node.TEXT_NODE);
        if(childNodes.size() != 1)
        {
             throw new Exception(MappingXMLTraverser.produceElementTrace(field, true,
                                                "No text child nodes found. Expected one (text) child nodes"));
        }
        childNode = (Node)childNodes.get(0);
        return MappingXMLTraverser.getNodeValue(childNode);

    }

    private void resolveTopicFields(
        Element classMapping,
        DCPSTopic topic) throws Exception
    {
        Vector childNodes;
        int count;
        Element childNode;
        String nodeName;

        childNodes = MappingXMLTraverser.filterNodeList(classMapping.getChildNodes(), Node.ELEMENT_NODE);
        for(count = 0; count < childNodes.size(); count++)
        {
            childNode = (Element)childNodes.get(count);
            nodeName = childNode.getNodeName();
            if(nodeName.startsWith(MappingXMLTraverser.monoAttributeElementID))
            {
                processMonoAttribute(classMapping, childNode, topic);
            } else if(nodeName.startsWith(MappingXMLTraverser.multiAttributeElementID))
            {
//     processMultiAttribute(classMapping, childNode);
            } else if(nodeName.startsWith(MappingXMLTraverser.monoRelationElementID))
            {
                processMonoRelation(childNode, topic);
            } else if(nodeName.equals(MappingXMLTraverser.multiRelationElementID))
            {
                processMultiRelation(classMapping, childNode);
            }//else ignore
        }
    }

    private void processMultiRelation(
        Element classMapping,
        Element multiRelation) throws Exception
    {
        Element mpTopic;
        String topicName;
        DCPSTopic topic;
        String indexField;
        String indexFieldType;
        DCPSField aField;
        int keyType;

        /* step 1) get the multiplace topic, if default generated, create a new topic with it's topic name */
        mpTopic = MappingXMLTraverser.getMultiPlaceTopic(multiRelation);
        if(mpTopic == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(multiRelation, "Missing required '"+
                    MappingXMLTraverser.multiPlaceTopicElementID+"' child element."));
        }
        if(MappingXMLTraverser.isElementDefault(mpTopic))
        {
            topicName = MappingXMLTraverser.getNodeValue(mpTopic.getAttributes(),
                                                            MappingXMLTraverser.multiPlaceTopicTypeNameAttributeID);
            if(topicName == null)
            {
                topicName = MappingXMLTraverser.getNodeValue(mpTopic.getAttributes(),
                                                            MappingXMLTraverser.multiPlaceTopicNameAttributeID);
            }
            topic = new DCPSTopic(topicName);
            /* step 2) process the keyDescription within the multiPlaceTopic */
            processKeyDescription(mpTopic, topic, DCPSField.DCPS_FIELD_KEY);
            /* step 3) get the indexField defined for the multiPlaceTopic */
            indexField = MappingXMLTraverser.getNodeValue(mpTopic.getAttributes(),
                                                            MappingXMLTraverser.multiPlaceTopicIndexAttributeID);
            if(indexField != null)
            {
                /* step 4) if an indexField is defined we need to find out if this multiRelation represents a StrMap,
                 * IntMap or List so we know the type of the indexField (string, long, long respectively)
                 */
                indexFieldType = MappingXMLTraverser.getNodeValue(mpTopic.getAttributes(),
                                                            MappingXMLTraverser.topicElementIndexFieldTypeAttributeID);

                if(indexFieldType == null)
                {
                    throw new Exception(MappingXMLTraverser.produceElementTrace(mpTopic, "Missing '"+
                    MappingXMLTraverser.topicElementIndexFieldTypeAttributeID+
                   "' attribute for this element. Should have been generated by DCG, but apparently was not."));
                }
                aField = new DCPSField(indexField, indexFieldType, DCPSField.DCPS_FIELD_KEY, new Vector());
                topic.addField(aField);
                /* create an identity for this part of the key, its an self containing key identity */
                KeyIdentity identity = new KeyIdentity();
                identity.addField(aField);
                topic.addIdentity(identity);
            }


            /* step 5) process the keydescription outside the scope of the multiplacetopic, if this is a set (ie no
             * indexField) then the keyfields in the keyDescription are keys, otherwise foreign keys
             */
            if(indexField == null)
            {
                keyType = DCPSField.DCPS_FIELD_KEY;
            } else
            {
                keyType = DCPSField.DCPS_FIELD_FOREIGN_KEY;
            }
            processKeyDescription(multiRelation, topic, keyType);
            topics.add(topic);
        }//else ignore
    }

    private void processMonoAttribute(
        Element classMapping,
        Element monoAttribute,
        DCPSTopic topic) throws Exception
    {
        NodeList elements;
        String fieldName;
        String fieldType = null;
        DCPSField aField;
        Vector arraySizes;

        elements = monoAttribute.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
        if(elements.getLength() == 0)
        {
            String dlrlAttributeName = MappingXMLTraverser.getNodeValue(monoAttribute.getAttributes(),
                                                                    MappingXMLTraverser.monoAttributeNameAttributeID);
            //continue to generate, ignoring attributes mapped onto place topics
            elements = monoAttribute.getElementsByTagName(MappingXMLTraverser.valueFieldElementID);
            if(elements.getLength()!= 1)
            {
                 throw new Exception(MappingXMLTraverser.produceElementTrace(monoAttribute, "Detected '"+
                     elements.getLength()+"' child '"+MappingXMLTraverser.valueFieldElementID+
                     "' elements. No more or less then 1 '"+MappingXMLTraverser.valueFieldElementID+
                     "' element is supported."));
            }

            fieldName = getKeyOrValuefieldName((Element)elements.item(0));
            if(!topic.containsField(fieldName))
            {
                arraySizes = new Vector();
                fieldType = determineFieldType(classMapping, monoAttribute, dlrlAttributeName, arraySizes);
                aField = new DCPSField(fieldName, fieldType, DCPSField.DCPS_FIELD_NORMAL, arraySizes);
                topic.addField(aField);
            }//else was added during keyfield processing
       }//ignore fields inside place topics
    }

    private void processMonoRelation(
        Element monoRelation,
        DCPSTopic topic) throws Exception
    {
        NodeList elements;

        elements = monoRelation.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
        if(elements.getLength() == 0)
        {
            processKeyDescription(monoRelation, topic, DCPSField.DCPS_FIELD_FOREIGN_KEY);
            processValidityField(monoRelation, topic);
        }//ignore fields inside place topics
    }

    private void processValidityField(
        Element monoRelation,
        DCPSTopic topic) throws Exception
    {
        Element validityField;
        String name;
        DCPSField aField;

        validityField = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(
                            monoRelation,
                            MappingXMLTraverser.validityFieldElementID);
        if(validityField != null)
        {
            name = MappingXMLTraverser.getXMLElementAttributeValue(
                            validityField,
                            MappingXMLTraverser.validityFieldNameAttributeID);
            if(name == null)
            {   //must always be specified
                throw new Exception(MappingXMLTraverser.produceElementTrace(
                        validityField,
                        false,
                        "No value provided for the required "+
                        MappingXMLTraverser.validityFieldNameAttributeID+
                        " attribute."));
            }
            aField = new DCPSField(name, "boolean", DCPSField.DCPS_FIELD_NORMAL, new Vector());
            topic.addField(aField);
        }
    }

    private String determineFieldType(
        Element classMapping,
        Element originalElement,
        String attributeName,
        Vector arraySizes) throws Exception
    {
        String name;
        IdlObject object;


        name = MappingXMLTraverser.getNodeValue(classMapping.getAttributes(),
                                                                    MappingXMLTraverser.classMappingNameAttributeID);
        if(name == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(classMapping, "Attribute '"+
                MappingXMLTraverser.classMappingNameAttributeID+"' must be defined for this element."));
        }
        object = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, name);
        if(object == null)
        {
            throw new Exception(MappingXMLTraverser.produceElementTrace(classMapping, "No IDL Valuetype with name '"+
                                                                                    name+"' found in the DLRL IDL."));
        }
        return getFieldTypeForAttribute(object, attributeName, arraySizes, originalElement);
    }

    private String getFieldTypeForAttribute(
        IdlObject object,
        String attributeName,
        Vector arraySizes,
        Element originalElement) throws Exception
    {
        IdlObject attributeType;
        Object idlAttribute;
        IdlStateMember member;
        String name;

        idlAttribute = IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName);
        if(idlAttribute == null)
        {
            name = IDLTraverser.getIDLObjectFullyqualifiedName(object);
            throw new Exception(MappingXMLTraverser.produceElementTrace(originalElement,
                "IDL Valuetype with name '"+name+"' does not contain attribute '"+attributeName+
                "' in the DLRL IDL."));
        }
        if(!(idlAttribute instanceof IdlStateMember))
        {
            name = IDLTraverser.getIDLObjectFullyqualifiedName(object);
            throw new Exception(MappingXMLTraverser.produceElementTrace(originalElement,
                "IDL Valuetype with name '"+name+"' and attribute '"+attributeName+
                "' in the DLRL IDL was not an IDL statemember as expected."));
        }
        member = (IdlStateMember)idlAttribute;
        attributeType = ((IdlObject)member.stateType()).final_object();
        return getStringNameForType(object, attributeName, originalElement, attributeType, arraySizes);
    }

    private String getStringNameForType(
        IdlObject object,
        String attributeName,
        Element originalElement,
        IdlObject attributeType,
        Vector arraySizes) throws Exception
    {
        String name;

        if(attributeType instanceof IdlIdent)
        {
            attributeType = (IdlObject)((IdlIdent)attributeType).original();
            IdlObject tmpObject = attributeType;
            /* resolve any typeDefs */
            while(tmpObject != null && tmpObject instanceof IdlTypeDef)
            {
                tmpObject = (IdlObject)((IdlTypeDef)tmpObject).original();
                if(tmpObject instanceof IdlIdent)
                {
                    tmpObject = (IdlObject)((IdlIdent)tmpObject).original();
                }
            }
            if(tmpObject instanceof IdlValue)
            {
                name = IDLTraverser.getIDLObjectFullyqualifiedName(object);
                throw new Exception(MappingXMLTraverser.produceElementTrace(originalElement, "IDL Valuetype '"+name+
                    "', attribute '"+attributeName+
                    ": An invalid attribute type was detected. Detected a valuetype as type where it was not allowed"));
            }
            return IDLTraverser.getIDLObjectFullyqualifiedName(attributeType);
        } else if(attributeType instanceof IdlSimple)
        {
            return IDLTraverser.getIdlPrimitiveStringName((idlPrimitive)attributeType);
        } else if(attributeType instanceof IdlTypeDef)
        {
            IdlObject tmpObject = attributeType;
            /* resolve any typeDefs */
            while(tmpObject != null && tmpObject instanceof IdlTypeDef)
            {
                tmpObject = (IdlObject)((IdlTypeDef)tmpObject).original();
                if(tmpObject instanceof IdlIdent)
                {
                    tmpObject = (IdlObject)((IdlIdent)tmpObject).original();
                }
            }
            if(tmpObject instanceof IdlValue)
            {
                name = IDLTraverser.getIDLObjectFullyqualifiedName(object);
                throw new Exception(MappingXMLTraverser.produceElementTrace(originalElement, "IDL Valuetype '"+name+
                    "', attribute '"+attributeName+
                    ": An invalid attribute type was detected. Detected a valuetype as type where it was not allowed"));
            }
            return IDLTraverser.getIDLObjectFullyqualifiedName(attributeType);
        } else if(  attributeType instanceof IdlEnum ||
                    attributeType instanceof IdlUnion ||
                    attributeType instanceof IdlStruct)
        {
            return IDLTraverser.getIDLObjectFullyqualifiedName(attributeType);
        }
        else if(attributeType instanceof IdlString)
        {
            int max = ((IdlString)attributeType).max();
            if(max != 0)
            {
                return "string<"+max+">";
            } else
            {
                return "string";
            }
        } else if(attributeType instanceof IdlSequence)
        {
            int size = 	((IdlSequence)attributeType).getSize();
            IdlObject seqType =	(IdlObject)((IdlSequence)attributeType).internal();
            if(size == 0)
            {
                return "sequence<"+getStringNameForType(object, attributeName, originalElement, seqType, arraySizes)+">";
            } else
            {
                return "sequence<"+getStringNameForType(object, attributeName, originalElement, seqType, arraySizes)+
                                                                                                        ", "+size+">";
            }
        } else if(attributeType instanceof IdlArray)
        {
            int[] dimension = ((IdlArray)attributeType).dimensions();
            for(int count = 0; count < dimension.length; count++)
            {
                arraySizes.add(new Integer(dimension[count]));
            }
            IdlObject arrayType = (IdlObject)((IdlArray)attributeType).internal();
            return getStringNameForType(object, attributeName, originalElement, arrayType, arraySizes);
        } else
        {
                name = IDLTraverser.getIDLObjectFullyqualifiedName(object);
                throw new Exception(MappingXMLTraverser.produceElementTrace(originalElement, "IDL Valuetype '"+name+
                    "', attribute '"+attributeName+
                    ": An unknown attribute type ("+attributeType.getClass().getName()+")was detected."));
        }
    }

    private void writeHeader(
        ) throws Exception
    {
        write("/******************************************************************************", 1);
        write(" ***                     Generated code: DO NOT EDIT!!!                     ***", 1);
        write(" ***        Generated by the OpenSplice DLRL Code Generator(OSPLDCG)        ***", 1);
        write(" ***             OpenSplice (c) is a product of PrismTech Ltd.              ***", 1);
        write(" ***                 Visit our website at www.prismtech.com                 ***", 1);
        write(" ***                                                                        ***", 1);
        write(" ***         ----------------------OpenSplice----------------------         ***", 1);
        write(" ***        |  Delivering the right information to the right place |        ***", 1);
        write(" ***        |   at the right time for real-time application users  |        ***", 1);
        write(" ***         ------------------------------------------------------         ***", 1);
        write(" ******************************************************************************/",2);
    }

    private void writeTopics(
        ) throws Exception
    {
        int count;
        DCPSTopic topic;

        for(count = 0; count < topics.size(); count++)
        {
            topic = (DCPSTopic)topics.get(count);
            openModule(topic.getModules());
            write("struct "+topic.getName()+"{", 1);
            indent++;
            writeTopicAttributes(topic);
            indent--;
            write("};",1);
            write("#pragma keylist "+topic.getName()+" "+topic.getKeyFieldsNameList(), 2, 0);
            closeModule(topic.getModules());
        }
    }

    private void writeTopicAttributes(
        DCPSTopic topic) throws Exception
    {
        Vector fields;
        int count;
        DCPSField aField;
        StringBuffer buffer;

        fields = topic.getFields();
        for(count = 0 ; count < fields.size(); count++)
        {
            aField = (DCPSField)fields.get(count);
            buffer = new StringBuffer(aField.getType());
            buffer.append(" ");
            buffer.append(aField.getName());
            Vector arraySizes = aField.getArraySizes();
            for(int arraySizeCount = 0; arraySizeCount < arraySizes.size(); arraySizeCount++)
            {
                Integer arraySize = (Integer)arraySizes.get(arraySizeCount);
                buffer.append("[");
                if(arraySize.intValue() != 0)
                {
                    buffer.append(arraySize.intValue());
                }
                buffer.append("]");
            }
            buffer.append(";");
            if(aField.getFieldType() == DCPSField.DCPS_FIELD_KEY)
            {
                buffer.append("/* keyfield */");
            } else if(aField.getFieldType() == DCPSField.DCPS_FIELD_FOREIGN_KEY)
            {
                buffer.append("/* foreign keyfield */");
            }
            write(buffer.toString(), 1);
        }
    }


    private void openModule(
        String[] modules) throws Exception
    {
        String moduleName;
        int count;

        for(count = 0; count < modules.length; count++)
        {
            moduleName = modules[count];
            if(moduleName.length() > 0)
            {
                write("module "+moduleName+"{",1);
                indent++;
            }
        }
    }

    private void closeModule(
        String[] modules) throws Exception
    {
        int count;

        for(count = 0; count < modules.length; count++){
            if(modules[count].length() > 0){
                indent--;
                write("}; /*closing module "+modules[count]+" */",2);
            }
        }
    }

    private void write(
        String text,
        int newLines,
        int indents) throws Exception
    {
        int count;
        for(count = 0; count < indents; count++)
        {
            out.write('\t');
        }
        out.write(text);
        for (count = 0; count < newLines; count++)
        {
            out.newLine();
        }
    }

    private void write(
        String text,
        int newLines) throws Exception
    {
        write(text, newLines, indent);
    }
}

class DCPSTopic
{

    private String[] modules;
    private String name;
    private String nameFQ;
    private Vector fields = new Vector();
    private Vector keyIdentities = new Vector();

    private DCPSTopic(){}

    public DCPSTopic(
        String nameFQ)
    {
        this.nameFQ = nameFQ;
        parseName(nameFQ);
    }

    private void parseName(
        String nameFQ)
    {
        String[] tokens;
        int count;

        tokens = nameFQ.split("::");
        modules = new String[tokens.length-1];
        for(count = 0; count < modules.length; count++){
            modules[count] = tokens[count];
        }
        name = tokens[tokens.length-1];
    }

    public void addField(
        DCPSField field) throws Exception
    {
        if(containsField(field.getName()))
        {
            throw new Exception("DCPS Field '"+field.getName()+"' was mapped multiple types for topic '"+
                nameFQ+"'. Each field may only be mapped once.");
        }
        fields.add(field);
    }

    public boolean containsField(
        String name)
    {
        int count;
        DCPSField aField;

        for(count = 0; count < fields.size(); count++)
        {
            aField = (DCPSField)fields.get(count);
            if(aField.getName().equals(name))
            {
                return true;
            }
        }
        return false;
    }

    public void addIdentity(
        KeyIdentity identity)
    {
        keyIdentities.add(identity);
    }

    public String[] getModules(
        )
    {
        return this.modules;
    }

    public String getName(
        )
    {
        return this.name;
    }

    public Vector getFields(
        )
    {
        return this.fields;
    }

//set: onwer_oid, owner_string target_oid target_stirng
//map: owner_oid, owner_string, index(maybe string)
//normal: oid, string
    public String getKeyFieldsNameList(
        )
    {
        StringBuffer buffer = new StringBuffer("");
        KeyIdentity identity;
        for(int count = 0; count < keyIdentities.size(); count++)
        {
            identity = (KeyIdentity)keyIdentities.get(count);
            identity.finalizeIdentity();
            Vector fields = identity.getFields();
            for(int fieldCount = 0; fieldCount < fields.size(); fieldCount++)
            {
                DCPSField aField = (DCPSField)fields.get(fieldCount);
                if(aField.getType().equals("DDS::DLRLOid"))
                {
                    buffer.append(aField.getName());
                    buffer.append(".systemId");
                    buffer.append(" ");
                    buffer.append(aField.getName());
                    buffer.append(".localId");
                    buffer.append(" ");
                    buffer.append(aField.getName());
                    buffer.append(".serial");
                } else
                {
                    buffer.append(aField.getName());
                }
                if(fieldCount+1 < fields.size())
                {
                    buffer.append(" ");
                }
            }
            if(count+1 < keyIdentities.size())
            {
                buffer.append(" ");
            }
        }
        return buffer.toString();
    }
}


class KeyIdentity
{
    private Vector fields = new Vector();

    public void addField(
        DCPSField field)
    {
        fields.add(field);
    }

    public Vector getFields(
        )
    {
        return fields;
    }

    public void finalizeIdentity(
        )
    {
        DCPSField aField;
        boolean onlyOid = true;
        Vector tmp = fields;
        fields = new Vector();
        while(tmp.size() > 0)
        {
            DCPSField field = null;
            for(int count = 0; count < tmp.size() && field== null; count++)
            {
                aField = (DCPSField)tmp.get(count);
                if(onlyOid && aField.getType().equals("DDS::DLRLOid"))
                {
                    field = aField;
                } else if(!onlyOid)
                {
                    field = aField;
                }
            }
            if(field != null)
            {
                fields.add(field);
                tmp.remove(field);
            } else
            {
                onlyOid = false;
            }
        }
    }
}

class DCPSField
{

    public static final int DCPS_FIELD_NORMAL = 0;
    public static final int DCPS_FIELD_KEY = 1;
    public static final int DCPS_FIELD_FOREIGN_KEY = 2;

    private String name;
    private String type;
    private int fieldType;
    private Vector arraySizes;

    public DCPSField(
        String name,
        String type,
        int fieldType,
        Vector arraySizes)
    {
        this.name =  name;
        this.type = type;
        this.fieldType = fieldType;
        this.arraySizes = arraySizes;
    }

    public String getName(
        )
    {
        return this.name;
    }

    public String getType(
        )
    {
        return this.type;
    }

    public int getFieldType(
        )
    {
        return this.fieldType;
    }

    public Vector getArraySizes(
        )
    {
        return this.arraySizes;
    }
}
