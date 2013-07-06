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

import DCG.DCGUtilities.MappingXMLTraversable;
import DCG.DCGUtilities.MappingXMLTraverser;
import DCG.DCGUtilities.IDLTraverser;
import org.openorb.compiler.idl.reflect.idlObject;
import org.openorb.compiler.idl.reflect.idlType;
import org.openorb.compiler.object.IdlEnum;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlModule;
import org.openorb.compiler.object.IdlValue;
import org.openorb.compiler.object.IdlIdent;
import org.openorb.compiler.object.IdlSimple;
import org.openorb.compiler.object.IdlTypeDef;
import org.openorb.compiler.object.IdlStructMember;
import org.w3c.dom.Node;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import java.util.Vector;

/**
 * This class will validate the contents of the Mapping XML file with the contents
 * of the DLRL IDL file and (if present) the DCPS IDL file. The Mapping XML must
 * be validated with DTD syntax before this class can do its work. The DTD to
 * which it
 * needs to be validated is the following:
<?xml version="1.0" encoding="ISO-8859-1"?>

<!ELEMENT Dlrl
       (enumDef | templateDef | associationDef | compoRelationDef| classMapping)*>
<!ATTLIST Dlrl
        name            CDATA           #IMPLIED>

<!ELEMENT enumDef
        (value)*>
<!ATTLIST enumDef
        name            CDATA           #REQUIRED>

<!ELEMENT value
        (#PCDATA)>

<!ELEMENT templateDef EMPTY>
<!ATTLIST templateDef
        name            CDATA           #REQUIRED
        pattern         (Set | StrMap | IntMap) #REQUIRED
        itemType        CDATA           #REQUIRED>

<!ELEMENT associationDef
        (relation,relation)>

<!ELEMENT relation EMPTY>
<!ATTLIST relation
        class           CDATA           #REQUIRED
        attribute       CDATA           #REQUIRED>

<!ELEMENT compoRelationDef EMPTY>
<!ATTLIST compoRelationDef
        class           CDATA           #REQUIRED
        attribute       CDATA           #REQUIRED>

<!ELEMENT classMapping
        (mainTopic?,extensionTopic?, (monoAttribute | multiAttribute | monoRelation | multiRelation | local)*)>
<!ATTLIST classMapping
        name            CDATA           #REQUIRED>

<!ELEMENT mainTopic
        (keyDescription)>
<!ATTLIST mainTopic
        name            CDATA           #REQUIRED
        typename        CDATA           #IMPLIED>

<!ELEMENT extensionTopic
        (keyDescription)>
<!ATTLIST extensionTopic
        name            CDATA           #REQUIRED
        typename        CDATA           #IMPLIED>

<!ELEMENT monoAttribute
        (placeTopic?,valueField+)>
<!ATTLIST monoAttribute
        name            CDATA           #REQUIRED>

<!ELEMENT multiAttribute
        (multiPlaceTopic,valueField+)>
<!ATTLIST multiAttribute
        name            CDATA           #REQUIRED>

<!ELEMENT monoRelation
        (placeTopic?, validityField?, keyDescription)>
<!ATTLIST monoRelation
        name            CDATA           #REQUIRED>

<!ELEMENT validityField EMPTY>
<!ATTLIST validityField
	name		CDATA		#REQUIRED>

<!ELEMENT multiRelation
        (multiPlaceTopic,keyDescription)>
<!ATTLIST multiRelation
        name            CDATA           #REQUIRED>

<!ELEMENT local EMPTY>
<!ATTLIST local
        name            CDATA           #REQUIRED>

<!ELEMENT placeTopic
        (keyDescription)>
<!ATTLIST placeTopic
        name            CDATA           #REQUIRED
        typename        CDATA           #IMPLIED>

<!ELEMENT multiPlaceTopic
        (keyDescription)>
<!ATTLIST multiPlaceTopic
        name            CDATA           #REQUIRED
        typename        CDATA           #IMPLIED
        indexField      CDATA           #IMPLIED>

<!ELEMENT keyDescription
        (keyField*)>
<!ATTLIST keyDescription
        content (FullOid | SimpleOid | NoOid) #REQUIRED>

<!ELEMENT keyField
        (#PCDATA)>

<!ELEMENT valueField
        (#PCDATA)>
 */
public class MappingXMLContentValidator implements MappingXMLTraversable {

    /**
     * The vector containing the DCPS IDL syntax tree
     */
    private Vector rootDCPSIDL;

    /**
     * The vector containing the DLRL IDL syntax tree
     */
    private Vector rootDLRLIDL;

    /**
     * The vector containing the Mapping XML syntax tree
     */
    private Document rootXML;

    /**
     * The reference to the mapping XML traverser class used to traverse the XML tree.
     */
    private MappingXMLTraverser xmlTraverser;

    private Vector classMappingUsedAttributeNames = new Vector();
    private boolean verbose = false;

    private Object valuetypeAttribute;//used for type validation when dealing with mono attributes..

    /* Stores all fields associated with a topic and the nodes the fields are mentioned in <String, Set>,
    Set contains Node classes*/
    private java.util.Map topics = new java.util.HashMap();

    public static final String DDS_MODULE = "DDS";

    /**
     * Specialized constructor
     *
     * @param rootXML The syntax tree containing the Mapping XML
     * @param rootDCPSIDL The syntax tree containing the DCPS IDL
     * @param rootDLRLIDL The syntax tree containing the DLRL IDL
     */
    public MappingXMLContentValidator(Document rootXML, Vector rootDLRLIDL, Vector rootDCPSIDL, boolean verbose) {
        this.rootXML        =   rootXML;
        this.rootDCPSIDL    =   rootDCPSIDL;
        this.rootDLRLIDL    =   rootDLRLIDL;
        this.verbose        =   verbose;
        xmlTraverser        =   new MappingXMLTraverser(this);
    }

    /**
     * Helper method that returns the last classMapping element of the Mapping XML
     * that was traversed by the mapping xml traverser. This method is purely a
     * convience method.
     *
     * @return IdlObject
     * @throws Exception
     */
    private IdlObject getLastClassMappingIdlObject() throws Exception {
        int index;
        Element classMappingElement;
        String className;

        index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.classMappingElementID);
        if(index != -1){
            classMappingElement = xmlTraverser.getProcessedElement(index);
            className = MappingXMLTraverser.getNodeValue(classMappingElement.getAttributes(),
                                                                    MappingXMLTraverser.classMappingNameAttributeID);
            if(className != null){
                return IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, className);
            }
        }
        return null;
    }

    /**
     * This method returns true if the provided name is not a dlrl keyword for
     * attribute. Current DLRL keyword is: invalid_relations. Meaning no attribute may
     * use this name as an identifier, as it would cause conflicts in generated code
     *
     * @param name The possible DLRL attribute keyword name
     * @return boolean
     */
    private boolean isReservedDLRLAttributeName(String name) {
        /* case sensitive compare the name, return true if the name matches the reserved key words */
        if(name.equals("invalid_relations")){
            return true;
        }
        return false;
    }

    /**
     * This method validates an association def element to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT associationDef (relation,relation)>
     *
     * @param associationNode The Element node containing the associationDef
     * @throws Exception
     */
    public void processXMLMappingAssociationDefElement(Element associationNode) throws Exception {
        int index;

        /* traverse the associationdef element, so we can visit all child elements */
        xmlTraverser.traverseXMLMappingAssociationDefElement(associationNode);

        /* Remove the association def from the processed elements list, its no longer needed for validation purposes. */
        index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.associationDefElementID);
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);

    }

    /**
     * This method validates a class mapping element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT classMapping (mainTopic?, extensionTopic?, (monoAttribute|
     * multiAttribute| monoRelation| multiRelation| local)*)>
     * <!ATTLIST> classMapping
     * name  CDATA  #REQUIRED>
     *
     * @param classMappingNode The Element node containing the classMapping
     * @throws Exception
     */
    public void processXMLMappingClassMappingElement(Element classMappingNode) throws Exception {
        String name;
        IdlValue object;
        idlObject definedInObject;
        IdlValue[] inheritanceList;
        int index;

        /* Validate the name attribute, it should be present */
        name = MappingXMLTraverser.getNodeValue(classMappingNode.getAttributes(),
                                                MappingXMLTraverser.classMappingNameAttributeID);
        if(name == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)classMappingNode, "Attribute '"+
                MappingXMLTraverser.classMappingNameAttributeID+"' must be defined for this element."));
        }

        /* We should be able to find a non forward valuetype with the name specified, if not then this is an
         * invalid mapping.
         */
        object = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, name);
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(classMappingNode,
                                                    "No IDL Valuetype with name '"+name+"' found in the DLRL IDL."));
        }

        /* Check if valuetype is not defined in module DDS, such valuetypes may not be mapped. We exempt all valuetypes
         * from the DDS module as that idl file is included always and could thus be mapped, but naturally we do not
         * want to process such valuetypes as thats not correct usage
         */
        definedInObject = object.idlDefinedIn();
        if(definedInObject != null && definedInObject instanceof IdlModule &&
                            (IDLTraverser.getIDLObjectName((IdlObject)definedInObject).equalsIgnoreCase(DDS_MODULE) )){
            throw new Exception(MappingXMLTraverser.produceElementTrace(classMappingNode,
                        "Illegal valuetype mapping. Valuetype "+name+" is defined in Module "+
                        IDLTraverser.getIDLObjectName((IdlObject)definedInObject)+
                        ". Valuetypes in this module may not be mapped."));
        }

        /* Each mapped DLRL object must inherit from DDS::ObjectRoot, otherwise it is not a valid DLRL object and we can
         * not process it as such
         */
        if(!IDLTraverser.valuetypeInheritsFromObjectRoot(object)){
            throw new Exception(MappingXMLTraverser.produceElementTrace(classMappingNode,"The Valuetype '"+name+
              "' defined in the DLRL IDL does not have the DDS::ObjectRoot as it's highest parent, which is required."));
        }

        /* DLRL only allows single inheritance, conform specification. Verify this is the case. */
        inheritanceList = object.getInheritance();
        if(inheritanceList.length > 1){
            throw new Exception(MappingXMLTraverser.produceElementTrace(classMappingNode,"The Valuetype '"+name+
              "' defined in the DLRL IDL has multiple inheritence defined. Only single inheritance is allowed."));
        }

        /* As we are about to process a new class mapping, we need to clear the 'old' attributes names we found and
         * stored while processing the previous class mapping
         */
        classMappingUsedAttributeNames.clear();

        /* Traverse the class mapping element so we can visit all child elements */
        xmlTraverser.traverseXMLMappingClassMappingElement(classMappingNode);

        /* remove the classmapping and child elements from the processed elements list, its no longer needed for
         * validation purposes. The traverser is the one adding it to that list during the traverse call.
         */
        index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.classMappingElementID);
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);
    }

    /**
     * This method validates a compo relation element to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT compoRelationDef EMPTY>
     * <!ATTLIST compoRelationDef
     *      class        CDATA   #REQUIRED
     *      attribute    CDATA   #REQUIRED>
     *
     * @param compoRelationNode The Element node containing the compoRelation
     * @throws Exception
     */
    public void processXMLMappingCompoRelationDefElement(Element compoRelationNode) throws Exception {
        String className;
        String attributeName;
        IdlObject object;
        Element classMapping;
        Element attributeMapping;
        String attributeTagName;

        /* Get the class name and attribute name values, if not present throw an exception */
        className = MappingXMLTraverser.getNodeValue(compoRelationNode.getAttributes(),
                                                                    MappingXMLTraverser.compoRelationClassAttributeID);
        if(className == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "Attribute '"+
                MappingXMLTraverser.compoRelationClassAttributeID+"' must be defined for this element."));
        }
        attributeName = MappingXMLTraverser.getNodeValue(compoRelationNode.getAttributes(),
                                                                MappingXMLTraverser.compoRelationAttributeAttributeID);
        if(attributeName==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "Attribute '"+
                MappingXMLTraverser.compoRelationAttributeAttributeID+"' must be defined for this element."));
        }

        /* We should be able to find a valuetype (non forward) for the class name and the attribute name specified
         * should be an attribute within the valuetype. Furthermore the valuetype should inherit from DDS::ObjectRoot
         * and if we are able to locate a classMapping then the attribute may not be specified as a local element
         */
        object = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, className);
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode,
                    "No IDL Valuetype with name '"+className+"' found in the DLRL IDL."));
        }
        if(IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName) == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode,
                "IDL Valuetype with name '"+className+"' doesn't contain attribute '"+attributeName+
                                                                                        "' in the provided DLRL IDL."));
        }
        classMapping = MappingXMLTraverser.findClassMappingwithName(rootXML, className);
        if(classMapping == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "The class "+
                "specified is not (mapped as) a DLRL object."));
        }
        attributeMapping = MappingXMLTraverser.getAttributeMapping(classMapping, attributeName);
        if(attributeMapping == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "The attribute "+
                "specified is not mapped in the "+MappingXMLTraverser.classMappingElementID+" of the class."));
        }
        attributeTagName = attributeMapping.getTagName();
        if(attributeTagName.equals(MappingXMLTraverser.localElementID)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "The attribute "+
                "specified is mapped onto a "+MappingXMLTraverser.localElementID+" attribute in the "+
                MappingXMLTraverser.classMappingElementID+" of the class. The attribute is thus not managed by the "+
                "DLRL, and the DLRL can thus not treat it as a compoRelation."));
        }
        if(attributeTagName.equals(MappingXMLTraverser.monoAttributeElementID)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "The attribute "+
                "specified is mapped onto a "+MappingXMLTraverser.monoAttributeElementID+" attribute in the "+
                MappingXMLTraverser.classMappingElementID+" of the class. CompoRelations can only be managed between "+
                "DLRL objects."));
        }
        if(attributeTagName.equals(MappingXMLTraverser.multiAttributeElementID)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationNode, "The attribute "+
                "specified is mapped onto a "+MappingXMLTraverser.multiAttributeElementID+" attribute in the "+
                MappingXMLTraverser.classMappingElementID+" of the class. CompoRelations can only be managed between "+
                "DLRL objects."));
        }
    }

    /**
     * This method validates an enum def element to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT enumDef (value)*>
     * <!ATTLIST enumDef
     * name CDATA   REQUIRED>
     *
     * @param enumDefNode The Element node containing the enumDef
     * @throws Exception
     */
    public void processXMLMappingEnumDefElement(Element enumDefNode) throws Exception {
        String className = null;
        int[] types;
        IdlObject object;
        int enumMembers;
        int index;
        int size;

        //validate name attribute
        className = MappingXMLTraverser.getNodeValue(enumDefNode.getAttributes(),
                                                                        MappingXMLTraverser.enumDefNameAttributeID);
        if(className == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)enumDefNode,
                    "Attribute '"+MappingXMLTraverser.enumDefNameAttributeID+"' must be defined for this element."));
        }
        types = new int[1];
        types[0] = idlType.ENUM;
        object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDLRLIDL, types, className);
        if(object == null || !(object instanceof IdlEnum)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)enumDefNode,
                    "No enumeration defined in DLRL IDL for this element."));
        }
        enumMembers = ((IdlEnum)object).members().length;
        xmlTraverser.traverseXMLMappingEnumDefElement(enumDefNode);
        index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.enumDefElementID);
        size = xmlTraverser.getProcessedElementsSize();
        //size is number of processed elements, index is position of last enumdef element
        //size - index is number of elements found including enumDef and all value elements.
        //substract 1 to get number of value elements and compare that number to the enum members\
        //of the idl object enumeration
        if(index == -1 || !(enumMembers == (size - index -1)) ){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)enumDefNode, true,
                    "DLRL IDL enumeration does not have the correct number of member elements. Found "+enumMembers+
                    "but required "+(size - index -1)+" member elements."));
        }
        //remove the enum def and child elements from the processed list, as these elements arent needed anymore
        //for validation purposes
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);

    }

    /**
     * This method validates an extension topic element to the DLRL IDL contents and
     * the contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT extensionTopic(keyDescription)>
     * <!ATTLIST extensionTopic name CDATA #REQUIRED
                                typename CDATA #OPTIONAL>
     *
     * @param extensionTopicNode The Element node containing the extension topic
     * @throws Exception
     */
    public void processXMLMappingExtensionTopicElement(Element extensionTopicNode) throws Exception {
        String extensionTopicName = null;
        String typeName = null;
        IdlObject valueObject;
        IdlValue[] inheritanceList;
        String topicName;

        /* only validate this is it is not a default mapped extension topic */
        valueObject = getLastClassMappingIdlObject();
        if(valueObject == null || !(valueObject instanceof IdlValue)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)extensionTopicNode,
                "Incorrect classMapping. The mapped class must inherit from be a valuetype."));
        }
        //TODO this is probably incorrect code, but we do not support extension topic yet and it might
        //be removed from the spec, so not putting effort into rewriting this... 19-04-07
        inheritanceList = ((IdlValue)valueObject).getInheritance() ;
        if(inheritanceList.length != 1){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)extensionTopicNode,
                    "Incorrect inheritance list defined in DLRL IDL valuetype '"+
                    IDLTraverser.getIDLObjectFullyqualifiedName(valueObject)+
                    "'. Required exactly one valuetype in the inheritance list."));
        }
        extensionTopicName = MappingXMLTraverser.getNodeValue(extensionTopicNode.getAttributes(),
                                                                MappingXMLTraverser.extensionTopicNameAttributeID);
        validateTopicName(extensionTopicNode, extensionTopicName);
        typeName = MappingXMLTraverser.getNodeValue(extensionTopicNode.getAttributes(),
                                                            MappingXMLTraverser.extensionTopicTypeNameAttributeID);
        topicName =typeName;
        if(topicName == null){
            topicName = extensionTopicName;
        }
        if(topicName != null){
            int[] types = new int[1];
            types[0] = idlType.STRUCT;
            IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null && !MappingXMLTraverser.isElementDefault(extensionTopicNode)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)extensionTopicNode,
                    "No IDL Struct with name '"+topicName+"' found in the DCPS IDL."));
            }
            xmlTraverser.traverseXMLMappingExtensionTopicElement(extensionTopicNode);
        }

    }

    /**
     * This method validates a key description element to the DLRL IDL contents and
     * the contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT keyDescription(keyField*)>
     * <!ATTLIST keyDescription content (FullOid|SimpleOid|NoOid) #REQUIRED>
     *
     * @param keyDescriptionNode The Element node containing the keydescription
     * @throws Exception
     */
    public void processXMLMappingKeyDescriptionElement(Element keyDescriptionNode) throws Exception {
        xmlTraverser.traverseXMLMappingKeyDescriptionElement(keyDescriptionNode);
        int index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.keyDescriptionElementID);
        int size = xmlTraverser.getProcessedElementsSize();
        //size is number of processed elements, index is position of last key description element
        //size - index is number of elements found including key description and all keyfield elements.
        //substract 1 to get number of keyfield elements
        int childElements = size - index -1;
        String content = MappingXMLTraverser.getNodeValue(keyDescriptionNode.getAttributes(), MappingXMLTraverser.keyDescriptionContentAttributeID);
        if(content == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescriptionNode,
                "The '"+MappingXMLTraverser.keyDescriptionContentAttributeID+"' attribute must be defined."));
        }
        if(content.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID)){
            if(childElements!=2){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescriptionNode, true,
                    "Detected "+childElements+" child elements for this "+MappingXMLTraverser.keyDescriptionElementID+
                    ". But the  '"+MappingXMLTraverser.keyDescriptionContentAttributeID+"' attribute value of "+
                    MappingXMLTraverser.keyDescriptionContentAttributeFullOidID+" requires exactly 2 child elements."));
            }
        } else if(content.equals(MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID)){
            if(childElements!=1){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescriptionNode, true,
                    "Detected "+childElements+" child elements for this "+MappingXMLTraverser.keyDescriptionElementID+
                    ". But the  '"+MappingXMLTraverser.keyDescriptionContentAttributeID+"' attribute value of "+
                    MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID+" requires exactly 1 child element."));
            }
        } else if(!content.equals(MappingXMLTraverser.keyDescriptionContentAttributeNoOidID)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescriptionNode, true,
                "Detected any invalid value for attribute "+MappingXMLTraverser.keyDescriptionContentAttributeID+
                ". Valid values are "+MappingXMLTraverser.keyDescriptionContentAttributeFullOidID+
                " or "+MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID+" or "+
                MappingXMLTraverser.keyDescriptionContentAttributeNoOidID));
        }
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);
    }

    public void processXMLMappingValidityFieldElement(Element node) throws Exception{
        String name = MappingXMLTraverser.getNodeValue(node.getAttributes(), MappingXMLTraverser.validityFieldNameAttributeID);
        String topicName = xmlTraverser.getLastTopicIdlName();
        if(topicName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(node, true,
                "This "+MappingXMLTraverser.validityFieldElementID+" is not defined within a classmapping with a topic."));
        }
        validateValidityField(node, name, topicName);
        addTopicField(topicName, name, node);
    }

    public void validateValidityField(Element node, String attributeName, String topicName) throws Exception{
        int[] types = new int[1];
        types[0] = idlType.STRUCT;
        boolean isDefault = xmlTraverser.isLastTopicDefault();
        if(!isDefault){
            IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace(node, true,
                    "No IDL Struct with name '"+topicName+"' found in the DCPS IDL."));
            }
            Object attribute = IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, object, attributeName);
            if(attribute == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace(node, true,
                    "Failed to locate attribute '"+attributeName+"' in IDL Struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"'."));
            }
            if(attribute instanceof IdlStructMember){
                attribute = ((IdlStructMember)attribute).type();
            }
            while(attribute != null && attribute instanceof IdlTypeDef){
                attribute = (IdlObject)((IdlTypeDef)attribute).original();
                if(attribute instanceof IdlIdent){
                    attribute = (IdlObject)((IdlIdent)attribute).original();
                }
            }
            if(attribute instanceof IdlIdent){
                attribute = (IdlObject)((IdlIdent)attribute).original();
            }
            if(!(attribute instanceof IdlSimple)){

                throw new Exception(MappingXMLTraverser.produceElementTrace(node, true,
                    "Attribute '"+attributeName+"' in IDL Struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"' is of an illegal (non simple) type "+
                    "(allowed simple types: boolean, long, long long, octet, short, unsigned long, unsigned long long, unsigned short)."));
            }
            int type = ((IdlSimple)attribute).primitive();
            if( type != IdlSimple.BOOLEAN &&
                type != IdlSimple.LONG &&
                type != IdlSimple.LONGLONG &&
                type != IdlSimple.OCTET &&
                type != IdlSimple.SHORT &&
                type != IdlSimple.ULONG &&
                type != IdlSimple.ULONGLONG &&
                type != IdlSimple.USHORT){
                throw new Exception(MappingXMLTraverser.produceElementTrace(node, true,
                    "Attribute '"+attributeName+"' in IDL Struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"' is of an illegal simple type "+
                    "(allowed types: boolean, long, long long, octet, short, unsigned long, unsigned long long, unsigned short)."));
            }
        }
        //validition field passes all criteria if this point is reached.. owh joy! =)
    }

    /**
     * This method validates a key field element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation(the child element is not validated by
     * this method):
     * <!ELEMENT keyField(#PCDATA)>
     *
     * @param keyFieldNode The Element node containing the keyField
     * @throws Exception
     * @roseuid 40B1A0F902A1
     */
    public void processXMLMappingKeyFieldElement(Element keyFieldNode) throws Exception {
        xmlTraverser.traverseXMLMappingKeyFieldElement(keyFieldNode);
    }

    /**
     * This method validates a key field element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation(the child element is only validated by
     * this method):
     * <!ELEMENT keyField(#PCDATA)>
     *
     * @param keyFieldNode The node containing the text child node of the keyField
     * element
     * @throws Exception
     * @roseuid 40DC1A1602F5
     */
    public void processXMLMappingKeyFieldTextElement(Node keyFieldNode) throws Exception {
        //the topic containing these fields must be a struct within the DCPS IDL containing these fields as attributes
        String topicName = xmlTraverser.getLastTopicIdlName();
        if(topicName==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                "This key field is not defined within a topic."));
        }
        if(!xmlTraverser.isLastTopicDefault()){
            int[] types = new int[1];
            types[0] = idlType.STRUCT;
            IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                    "No IDL Struct with name '"+topicName+"' found in the DCPS IDL."));
            }
            validateKeyFieldTextElement(keyFieldNode, object);
        }
        String attributeName = MappingXMLTraverser.getNodeValue(keyFieldNode);
        addTopicField(topicName, attributeName, keyFieldNode);
    }

    /**
     * This method validates a local element to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT local EMPTY>
     * <!ATTLIST local name CDATA #REQUIRED>
     *
     * @param localNode The Element node containing the local element
     * @throws Exception
     * @roseuid 40B1A1280386
     */
    public void processXMLMappingLocalElement(Element localNode) throws Exception {
        //Validate the name attribute
        String name = MappingXMLTraverser.getNodeValue(localNode.getAttributes(),
                                                                            MappingXMLTraverser.localNameAttributeID);
        String containingClass = IDLTraverser.getIDLObjectFullyqualifiedName(getLastClassMappingIdlObject());
        if(name == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)localNode,
                "No value for attribute '"+MappingXMLTraverser.localNameAttributeID+"' found."));
        }
        if(containingClass == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)localNode,
                "Attribute '"+name+"' is not allowed. No containing IDL class or topic name could be located for this "+
                "attribute."));
        }
        if(isReservedDLRLAttributeName(name)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)localNode,
                "Attribute '"+name+"' is not allowed. The attribute name is a reserved DLRL attribute name."));
        }
        if(classMappingUsedAttributeNames.contains(name)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)localNode,
                "Attribute '"+name+"' is not allowed. The attribute name is already used as another monoRelation, "+
                "monoAttribute, multiRelation, multiAttribute or local element."));
        }
        classMappingUsedAttributeNames.add(name);
        IdlObject object = getLastClassMappingIdlObject();
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)localNode,
                "No IDL Valuetype with name '"+containingClass+"' found in the DLRL IDL."));
        }
        if( IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, name) == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)localNode,
                "IDL Valuetype with name '"+containingClass+"' does not contain attribute '"+name+"' in the DLRL IDL."));
        }
    }

    /**
     * This method validates a main topic element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT mainTopic(keyDescription)>
     * <!ATTLIST mainTopic name CDATA #REQUIRED>
     *
     * @param mainTopicNode The Element node containing the main topic
     * @throws Exception
     * @roseuid 40B1A0F40291
     */
    public void processXMLMappingMainTopicElement(Element mainTopicNode) throws Exception {
        String structName = null;
        String typeName = null;

        //validate the name attribute
        structName = MappingXMLTraverser.getNodeValue(mainTopicNode.getAttributes(), MappingXMLTraverser.mainTopicNameAttributeID);
        validateTopicName(mainTopicNode, structName);
        typeName = MappingXMLTraverser.getNodeValue(mainTopicNode.getAttributes(), MappingXMLTraverser.mainTopicTypeNameAttributeID);
        String topicName = typeName;
        if(topicName == null){
            topicName = structName;
        }
        if(topicName != null){
            //validateClassToIdlTree(rootDCPSIDL, type, name);//type == struct!
            if(!MappingXMLTraverser.isElementDefault(mainTopicNode)){
                int[] types = new int[1];
                types[0] = idlType.STRUCT;
                IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
                if(object == null){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)mainTopicNode,
                        "No IDL Struct with name '"+topicName+"' found in the DCPS IDL."));
                }
            }
            xmlTraverser.traverseXMLMappingMainTopicElement(mainTopicNode);
        }

    }

    /**
     * This method validates a mono attribute element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT monoAttribute(placeTopic?, valueField+)>
     * <!ATTLIST monoAttribute name CDATA #REQUIRED>
     *
     * @param monoAttributeNode The Element node containing the mono attribute
     * @throws Exception
     * @roseuid 40B1A0FD030D
     */
    public void processXMLMappingMonoAttributeElement(Element monoAttributeNode) throws Exception {
        String attributeName=null;
        //Validate the name attribute
        String topicName = xmlTraverser.getLastTopicIdlName();
        String containingClass = IDLTraverser.getIDLObjectFullyqualifiedName(getLastClassMappingIdlObject());
        attributeName = MappingXMLTraverser.getNodeValue(monoAttributeNode.getAttributes(), MappingXMLTraverser.monoAttributeNameAttributeID);
        if(attributeName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "No value for attribute '"+MappingXMLTraverser.monoAttributeNameAttributeID+"' found."));
        }
        if(containingClass == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "Attribute '"+attributeName+"' is not allowed. No containing IDL class or topic name could be located "+
                "for this attribute."));
        }
        if(topicName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
            "Attribute '"+attributeName+"' is not allowed. No matching topic name could be located for this attribute."));
        }
        if(isReservedDLRLAttributeName(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is a reserved DLRL attribute name."));
        }
        if(classMappingUsedAttributeNames.contains(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is already used as another "+
                "monoRelation, monoAttribute, multiRelation, multiAttribute or local element."));
        }
        classMappingUsedAttributeNames.add(attributeName);
        int[] types = new int[2];
        types[0] = idlType.VALUE;
        types[1] = idlType.VALUEBOX;
        IdlObject object = getLastClassMappingIdlObject();
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "No IDL Valuetype with name '"+containingClass+"' found in the DLRL IDL."));
        }
        Object idlAttribute = IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName);
        if(idlAttribute == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "IDL Valuetype with name '"+containingClass+"' does not contain attribute '"+attributeName+
                "' in the DLRL IDL."));
        }
        if(!(idlAttribute instanceof org.openorb.compiler.object.IdlStateMember)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttributeNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL was not an IDL statemember as expected."));
        }
        valuetypeAttribute = idlAttribute;
        //validate the child elements
        xmlTraverser.traverseXMLMappingMonoAttributeElement(monoAttributeNode);
        valuetypeAttribute = null;
        //remove the monoAttributeNode from the processed elements list, its no longer needed for validation purposes.
        int index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.monoAttributeElementID);
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);
    }

    /**
     * This method validates a mono relation element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT monoRelation(placeTopic?, keyDescription)>
     * <!ATTLIST monoRelation name CDATA #REQUIRED>
     *
     * @param monoRelationNode  The Element node containing the mono relation
     * @return java.lang.Void
     * @throws Exception
     * @roseuid 40B1A1240329
     */
    public void processXMLMappingMonoRelationElement(Element monoRelationNode) throws Exception {
        String attributeName = null;
        String validityField = null;
        String topicName = xmlTraverser.getLastTopicIdlName();
        String containingClass = IDLTraverser.getIDLObjectFullyqualifiedName(getLastClassMappingIdlObject());
        //Validate the name attribute
        attributeName = MappingXMLTraverser.getNodeValue(monoRelationNode.getAttributes(), MappingXMLTraverser.monoRelationNameAttributeID);
        if(attributeName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "No value for attribute '"+MappingXMLTraverser.monoRelationNameAttributeID+"' found."));
        }
        if(containingClass == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "Attribute '"+attributeName+"' is not allowed. No containing IDL class or topic name could be located "+
                "for this attribute."));
        }
        if(topicName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
            "Attribute '"+attributeName+"' is not allowed. No matching topic name could be located for this attribute."));
        }
        if(isReservedDLRLAttributeName(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is a reserved DLRL attribute name."));
        }
        if(classMappingUsedAttributeNames.contains(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is already used as another "+
                "monoRelation, monoAttribute, multiRelation, multiAttribute or local element."));
        }
        classMappingUsedAttributeNames.add(attributeName);
        int[] types = new int[2];
        types[0] = idlType.VALUE;
        types[1] = idlType.VALUEBOX;
        IdlObject object = getLastClassMappingIdlObject();
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "No IDL Valuetype with name '"+containingClass+"' found in the DLRL IDL."));
        }
        Object attributeObject = IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName);
        if(attributeObject == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "IDL Valuetype with name '"+containingClass+"' does not contain attribute '"+attributeName+
                "' in the DLRL IDL."));
        }
        //mono relations can only be made towards valuetypes
        IdlObject attributeType = ((IdlObject)((org.openorb.compiler.object.IdlStateMember)attributeObject).stateType()).final_object();
        while(attributeType instanceof org.openorb.compiler.object.IdlTypeDef){
            attributeType = (IdlObject)((org.openorb.compiler.object.IdlTypeDef)attributeType).original();
            if(attributeType instanceof IdlIdent){
                attributeType = (IdlObject)((IdlIdent)attributeType).original();
            }
        }
        if(attributeType instanceof IdlIdent){
            attributeType = (IdlObject)((IdlIdent)attributeType).original();
        }
        if(!(attributeType instanceof IdlValue)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL is invalidly mapped as a  "+MappingXMLTraverser.monoRelationElementID+" element."+
                " The attribute type is not a valuetype, as required."));
        }
        //maybe the valuetype we retrieved from the state member was a forward valuetype, we need the non forward
        //valuetype, so lets get it by all means.
        IdlValue value = (IdlValue)attributeType;
        String fullyQualifiedName = IDLTraverser.getIDLObjectFullyqualifiedName(value);
        value = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, fullyQualifiedName);
        //Now we need to verify the valuetype used for this mono relation actually inherits from DDS::ObjectRoot
        //if it doesnt then its a local object and one cant make mono relations to non dlrl objects ofcourse!
        if(!IDLTraverser.valuetypeInheritsFromObjectRoot(value)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL is invalidly mapped as a "+MappingXMLTraverser.monoRelationElementID+" element."+
                " The attribute type is not a valuetype which inherits from DDS::ObjectRoot."));
        }
        xmlTraverser.traverseXMLMappingMonoRelationElement(monoRelationNode);
        //TODO: should have same content, same number of keyfields, and the types the fields represent MUST be the same
        //the names of the key fields are irrelevant ofcourse (topic depedant)
        Element ownerKeyDescription = getChildElementWithNameForElement(monoRelationNode, MappingXMLTraverser.keyDescriptionElementID, true);
        Element targetClassMapping = getClassMappingForValuetype(value);
        if(targetClassMapping == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL refers to another valuetype with name '"+fullyQualifiedName+
                "'. However no classMapping for this valuetype can be found."));
        }
        Element targetMainTopic = getChildElementWithNameForElement(targetClassMapping, MappingXMLTraverser.mainTopicElementID, true);
        Element targetKeyDescription = getChildElementWithNameForElement(targetMainTopic, MappingXMLTraverser.keyDescriptionElementID, true);
        validateKeyDescriptions(ownerKeyDescription, targetKeyDescription);
        //remove the monoAttributeNode from the processed elements list, its no longer needed for validation purposes.
        int index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.monoRelationElementID);
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);
    }

    /**
     * This method validates a multi attribute element to the DLRL IDL contents and
     * the contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT multiAttribute(multiPlaceTopic, valueField+)>
     * <!ATTLIST multiAttribute name CDATA #REQUIRED>
     *
     * @param multiAttributeNode  The Element node containing the multi attribute
     * @throws Exception
     * @roseuid 40B1A10001A6
     */
    public void processXMLMappingMultiAttributeElement(Element multiAttributeNode) throws Exception {
        String attributeName = null;
        String topicName = xmlTraverser.getLastTopicIdlName();
        String containingClass = IDLTraverser.getIDLObjectFullyqualifiedName(getLastClassMappingIdlObject());
        //Validate the name attribute
        attributeName = MappingXMLTraverser.getNodeValue(multiAttributeNode.getAttributes(), MappingXMLTraverser.multiAttributeNameAttributeID);
        if(attributeName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
                "No value for attribute '"+MappingXMLTraverser.multiAttributeNameAttributeID+"' found."));
        }
        if(containingClass == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
                "Attribute '"+attributeName+"' is not allowed. No containing IDL class or topic name could be located "+
                "for this attribute."));
        }
        if(topicName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
            "Attribute '"+attributeName+"' is not allowed. No matching topic name could be located for this attribute."));
        }
        if(isReservedDLRLAttributeName(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is a reserved DLRL attribute name."));
        }
        if(classMappingUsedAttributeNames.contains(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is already used as another "+
                "monoRelation, monoAttribute, multiRelation, multiAttribute or local element."));
        }
        classMappingUsedAttributeNames.add(attributeName);
        int[] types = new int[2];
        types[0] = idlType.VALUE;
        types[1] = idlType.VALUEBOX;
        IdlObject object = getLastClassMappingIdlObject();
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
                "No IDL Valuetype with name '"+containingClass+"' found in the DLRL IDL."));
        }
        if(IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName) == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiAttributeNode,
                "IDL Valuetype with name '"+containingClass+"' does not contain attribute '"+attributeName+
                "' in the DLRL IDL."));
        }
        xmlTraverser.traverseXMLMappingMultiAttributeElement(multiAttributeNode);
        //TODO validate types of the multi attribute, see mono attribute......
        //remove the multiAttributeNode from the processed elements list, its no longer needed for validation purposes.
        int index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.multiAttributeElementID);
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);
    }

    /**
     * This method validates a multi place topic element to the DLRL IDL contents and
     * the contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT multiPlaceTopic (keyDescription)>
     * <!ATTLIST multiPlaceTopic
     * name CDATA  #REQUIRED
     * indexField CDATA  #IMPLIED>
     *
     * @param multiPlaceTopicNode The Element node containing the multi place topic
     * @throws Exception
     * @roseuid 40B1A12E0308
     */
    public void processXMLMappingMultiPlaceTopicElement(Element multiPlaceTopicNode) throws Exception {
        String name = null;
        String typeName = null;
        name = MappingXMLTraverser.getNodeValue(multiPlaceTopicNode.getAttributes(), MappingXMLTraverser.multiPlaceTopicNameAttributeID);
        validateTopicName(multiPlaceTopicNode, name);
        typeName = MappingXMLTraverser.getNodeValue(multiPlaceTopicNode.getAttributes(), MappingXMLTraverser.multiPlaceTopicTypeNameAttributeID);
        String topicName =typeName;
        if(topicName == null){
            topicName = name;
        }
        //indexField may be null
        String indexField = MappingXMLTraverser.getNodeValue(multiPlaceTopicNode.getAttributes(), MappingXMLTraverser.multiPlaceTopicIndexAttributeID);
        if(!MappingXMLTraverser.isElementDefault(multiPlaceTopicNode)){
            int[] types = new int[1];
            types[0] = idlType.STRUCT;
            IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopicNode,
                    "No IDL Valuetype with name '"+topicName+"' found in the DCPS IDL."));
            }
            if(indexField != null){
                if(IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, object, indexField) == null){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopicNode,
                        "IDL Struct with name '"+topicName+"' does not contain attribute '"+indexField+
                        "' in the DCPS IDL."));
                }
                addTopicField(topicName, indexField, multiPlaceTopicNode);
            }

        }
        xmlTraverser.traverseXMLMappingMultiPlaceTopicElement(multiPlaceTopicNode);

    }

    /**
     * This method validates a multi relation element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT multiRelation(multiPlaceTopic, keyDescription)>
     * <!ATTLIST multiRelation name CDATA #REQUIRED>
     *
     * @param multiRelationNode  The Element node containing the multi relation
     * @throws Exception
     * @roseuid 40B1A1260377
     */
    public void processXMLMappingMultiRelationElement(Element multiRelationNode) throws Exception {
        String attributeName = null;
        String topicName = xmlTraverser.getLastTopicIdlName();
        String containingClass = IDLTraverser.getIDLObjectFullyqualifiedName(getLastClassMappingIdlObject());
        //Validate the name attribute
        attributeName = MappingXMLTraverser.getNodeValue(multiRelationNode.getAttributes(), MappingXMLTraverser.multiRelationNameAttributeID);
        if(attributeName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "No value for attribute '"+MappingXMLTraverser.multiRelationNameAttributeID+"' found."));
        }
        if(containingClass == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "Attribute '"+attributeName+"' is not allowed. No containing IDL class or topic name could be located "+
                "for this attribute."));
        }
        if(topicName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
            "Attribute '"+attributeName+"' is not allowed. No matching topic name could be located for this attribute."));
        }
        if(isReservedDLRLAttributeName(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is a reserved DLRL attribute name."));
        }
        if(classMappingUsedAttributeNames.contains(attributeName)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "Attribute '"+attributeName+"' is not allowed. The attribute name is already used as another "+
                "monoRelation, monoAttribute, multiRelation, multiAttribute or local element."));
        }
        classMappingUsedAttributeNames.contains(attributeName);
        int[] types = new int[2];
        types[0] = idlType.VALUE;
        types[1] = idlType.VALUEBOX;
        IdlObject object = getLastClassMappingIdlObject();//IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDLRLIDL, types, containingClass);
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "No IDL Valuetype with name '"+containingClass+"' found in the DLRL IDL."));
        }
        Object attributeObject = IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName);
        if(attributeObject == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "IDL Valuetype with name '"+containingClass+"' does not contain attribute '"+attributeName+
                "' in the DLRL IDL."));
        }
        xmlTraverser.traverseXMLMappingMultiRelationElement(multiRelationNode);

        //multi relations can only be made towards (forward)valuetypes
        IdlObject attributeType = ((IdlObject)((org.openorb.compiler.object.IdlStateMember)attributeObject).stateType()).final_object();
        while(attributeType instanceof org.openorb.compiler.object.IdlTypeDef){
            attributeType = (IdlObject)((org.openorb.compiler.object.IdlTypeDef)attributeType).original();//resolve typedef
            if(attributeType instanceof IdlIdent){
                attributeType = (IdlObject)((IdlIdent)attributeType).original();
            }
        }
        if(attributeType instanceof IdlIdent){
            attributeType = (IdlObject)((IdlIdent)attributeType).original();
        }
        if(!(attributeType instanceof IdlValue)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL is invalidly mapped as a  "+MappingXMLTraverser.multiRelationElementID+" element."+
                " The attribute type is not a valuetype, as required."));
        }
        if(!(((IdlValue)attributeType).forward())){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL is invalidly mapped as a  "+MappingXMLTraverser.multiRelationElementID+" element."+
                " The attribute type is not a forward valuetype, as required."));
        }
        //now find the template def which describes which item type this multi relation actually refers too:
        Element templateDef = getTemplateDefForForwardValuetype((IdlValue)attributeType);
        if(templateDef == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiRelationNode,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL is invalidly mapped as a  "+MappingXMLTraverser.multiRelationElementID+" element."+
                " No "+MappingXMLTraverser.templateDefElementID+" element can be found matching the attribute type."));
        }
        String itemType = MappingXMLTraverser.getNodeValue(templateDef.getAttributes(), MappingXMLTraverser.templateDefItemTypeAttributeID);
        if(itemType ==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDef,
                "Attribute '"+MappingXMLTraverser.templateDefItemTypeAttributeID+"' not defined."));
        }
        IdlValue value = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, itemType);
        if(value == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDef,
                "No IDL (non forward) valuetype with name '"+itemType+"' found in the DLRL IDL as indicated by the "+
                MappingXMLTraverser.templateDefItemTypeAttributeID+" of the "+MappingXMLTraverser.templateDefElementID+"."));
        }
        Element targetClassMapping = getClassMappingForValuetype(value);
        if(targetClassMapping == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDef,
                "IDL Valuetype with name '"+containingClass+"' and attribute '"+attributeName+
                "' in the DLRL IDL refers to another valuetype with name '"+itemType+
                "'. However no classMapping for this valuetype can be found."));
        }
        Element targetMainTopic = getChildElementWithNameForElement(targetClassMapping, MappingXMLTraverser.mainTopicElementID, true);
        Element targetKeyDescription = getChildElementWithNameForElement(targetMainTopic, MappingXMLTraverser.keyDescriptionElementID, true);
        Element relationTopicTargetKeyDescription = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(multiRelationNode,
                                                                                                    MappingXMLTraverser.keyDescriptionElementID);
        validateKeyDescriptions(relationTopicTargetKeyDescription, targetKeyDescription);

        Element multiPlaceTopic = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(multiRelationNode,
                                                                        MappingXMLTraverser.multiPlaceTopicElementID);
        Element relationTopicOwnerKeyDescription = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(multiPlaceTopic,
                                                                                                    MappingXMLTraverser.keyDescriptionElementID);
        int index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.classMappingElementID);

        Element ownerClassMapping = xmlTraverser.getProcessedElement(index);
        Element topicElement = getChildElementWithNameForElement((Element)ownerClassMapping, MappingXMLTraverser.extensionTopicElementID, false);
        if(topicElement == null){
            topicElement = getChildElementWithNameForElement((Element)ownerClassMapping, MappingXMLTraverser.mainTopicElementID, true);
        }
        Element ownerKeyDescription =  getChildElementWithNameForElement((Element)topicElement, MappingXMLTraverser.keyDescriptionElementID, true);
        validateKeyDescriptions(ownerKeyDescription, relationTopicOwnerKeyDescription);

        String pattern = MappingXMLTraverser.getNodeValue(templateDef.getAttributes(), MappingXMLTraverser.templateDefPatternAttributeID);
        String indexField = MappingXMLTraverser.getNodeValue(multiPlaceTopic.getAttributes(), MappingXMLTraverser.multiPlaceTopicIndexAttributeID);
        if(pattern.equals(MappingXMLTraverser.templateDefPatternAttributeStrMapValueID) ||
           pattern.equals(MappingXMLTraverser.templateDefPatternAttributeIntMapValueID) ||
           pattern.equals(MappingXMLTraverser.templateDefPatternAttributeListValueID)){
            if(indexField == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopic,
                "Missing definition of attribute "+MappingXMLTraverser.multiPlaceTopicIndexAttributeID+"."));
            }
            String name = MappingXMLTraverser.getNodeValue(multiPlaceTopic.getAttributes(), MappingXMLTraverser.multiPlaceTopicNameAttributeID);
            validateTopicName(multiPlaceTopic, name);
            String typeName = MappingXMLTraverser.getNodeValue(multiPlaceTopic.getAttributes(), MappingXMLTraverser.multiPlaceTopicTypeNameAttributeID);
            String tmpTopicName = typeName;
            if(tmpTopicName == null){
                tmpTopicName = name;
            }
            if(tmpTopicName != null && !MappingXMLTraverser.isElementDefault(multiPlaceTopic)){
                types = new int[1];
                types[0] = idlType.STRUCT;
                IdlObject topicObject = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, tmpTopicName);
                if(topicObject == null){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopic,
                        "No IDL Struct with name '"+tmpTopicName+"' found in the DCPS IDL."));
                }
                Object indexFieldAttribute = IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, topicObject, indexField);
                if(indexFieldAttribute == null || !(indexFieldAttribute instanceof IdlObject)){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopic,
                        "IDL Struct with name '"+tmpTopicName+"' does not contain attribute '"+indexField+"' in the DCPS IDL."));
                }
                if(pattern.equals(MappingXMLTraverser.templateDefPatternAttributeStrMapValueID)){
                    if(!IDLTraverser.isAttributeTypeString((IdlObject)indexFieldAttribute)){
                        throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopic,
                            "IndexField attribute '"+indexField+"' of IDL Struct with name '"+tmpTopicName+
                            "' in the DCPS IDL does not have 'string' as it's type, which is required for MultiRelation elements mapped as StrMap."));
                    }
                } else {
                    if(!IDLTraverser.isAttributeTypeLong((IdlObject)indexFieldAttribute)){
                        throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopic,
                            "IndexField attribute '"+indexField+"' of IDL Struct with name '"+tmpTopicName+
                            "' in the DCPS IDL does not have 'long' as it's type, which is required for MultiRelation elements mapped as IntMap or List."));
                    }
                }
            }
        } else if(indexField != null){
            //must be a set and thus no index field allowed
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopic,
                "Definition of attribute "+MappingXMLTraverser.multiPlaceTopicIndexAttributeID+
                " detected on a MultiRelation which is mapped as a Set collection type."));
        }
        //remove the monoAttributeNode from the processed elements list, its no longer needed for validation purposes.
        index = xmlTraverser.getElementLastIndexInProcessedList(MappingXMLTraverser.multiRelationElementID);
        xmlTraverser.removeProcessedElementsFromListFromIndex(index);
    }

    /**
     * This method validates a place topic element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT placeTopic (keyDescription)>
     * <!ATTLIST placeTopic name CDATA  #REQUIRED>
     *
     * @param placeTopicNode The Element node containing the place topic
     * @throws Exception
     * @roseuid 40B1A1420316
     */
    public void processXMLMappingPlaceTopicElement(Element placeTopicNode) throws Exception {
        String name = null;
        String typeName = null;
        name = MappingXMLTraverser.getNodeValue(placeTopicNode.getAttributes(), MappingXMLTraverser.placeTopicNameAttributeID);
        validateTopicName(placeTopicNode, name);
        typeName = MappingXMLTraverser.getNodeValue(placeTopicNode.getAttributes(), MappingXMLTraverser.placeTopicTypeNameAttributeID);
        String topicName = typeName;
        if(topicName == null){
            topicName = name;
        }
        if(topicName != null){
            int[] types = new int[1];
            types[0] = idlType.STRUCT;
            IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null && !MappingXMLTraverser.isElementDefault(placeTopicNode)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)placeTopicNode,
                    "No IDL Struct with name '"+topicName+"' found in the DCPS IDL."));
            }
            xmlTraverser.traverseXMLMappingPlaceTopicElement(placeTopicNode);
        }
    }

    /**
     * This method validates a relation element  to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT relation EMPTY>
     * <!ATTLIST relation
     * class    CDATA   #REQUIRED
     * attribute    CDATA   #REQUIRED>
     *
     * @param relationNode The Element node containing the relation
     * @throws Exception
     * @roseuid 40B1A0BF03C0
     */
    public void processXMLMappingRelationElement(Element relationNode) throws Exception {
        String className = MappingXMLTraverser.getNodeValue(relationNode.getAttributes(), MappingXMLTraverser.relationElementClassAttributeID);
        String attributeName = MappingXMLTraverser.getNodeValue(relationNode.getAttributes(), MappingXMLTraverser.relationElementAttributeAttributeID);
        if(className == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)relationNode,
                "Attribute '"+MappingXMLTraverser.relationElementClassAttributeID+"' not defined."));
        }
        if(attributeName==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)relationNode,
                "Attribute '"+MappingXMLTraverser.relationElementAttributeAttributeID+"' not defined."));
        }
        //validate the className attribute with a valuetype def and the attribute must be an attribute of the found valuetype
        IdlObject object = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootDLRLIDL, className);
        if(object == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)relationNode,
                "No IDL Valuetype with name '"+className+"' found in the DLRL IDL."));
        }
        if(IDLTraverser.getIdlObjectAttribute(rootDLRLIDL, object, attributeName)==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)relationNode,
                "IDL Valuetype with name '"+className+"' does not contain attribute '"+attributeName+"' in the DLRL IDL."));
        }
    }

    /**
     * This method validates an template def to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT templateDef EMPTY>
     * <!ATTLIST templateDef
     * name CDATA   #REQUIRED
     * pattern  (List | StrMap | IntMap | Set)  #REQUIRED
     * itemType CDATA   #REQUIRED>
     *
     * @param templateDefNode The Element node containing the templatedef
     * @throws Exception
     * @roseuid 40B1A0BB0094
     */
    public void processXMLMappingTemplateDefElement(Element templateDefNode) throws Exception {
        String name = MappingXMLTraverser.getNodeValue(templateDefNode.getAttributes(), MappingXMLTraverser.templateDefNameAttributeID );
        String itemType = MappingXMLTraverser.getNodeValue(templateDefNode.getAttributes(), MappingXMLTraverser.templateDefItemTypeAttributeID);
        String pattern = MappingXMLTraverser.getNodeValue(templateDefNode.getAttributes(), MappingXMLTraverser.templateDefPatternAttributeID);
        if(name == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDefNode,
                "Attribute '"+MappingXMLTraverser.templateDefNameAttributeID+"' not defined."));
        }
        if(itemType ==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDefNode,
                "Attribute '"+MappingXMLTraverser.templateDefItemTypeAttributeID+"' not defined."));
        }
        if(pattern == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDefNode,
                "Attribute '"+MappingXMLTraverser.templateDefPatternAttributeID+"' not defined."));
        }
        if(pattern.equals(MappingXMLTraverser.templateDefPatternAttributeListValueID)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDefNode,
                "The value of attribute '"+MappingXMLTraverser.templateDefPatternAttributeID+"' is not yet supported."+
                " Select another value for this attribute."));
        }
        //validate the name to a forward value type
        int[] types = new int[1];
        types[0] = idlType.VALUE;
        IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDLRLIDL, types, name);
        if(object == null || !(object instanceof IdlValue) || !(((IdlValue)object).isForward())){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)templateDefNode,
                "No matching forward valuetype definition found in DLRL IDL. Names must match!"));
        }
        //continue to validate itemType attribute, an exception is raised or the type of relation is returned
        MappingXMLTraverser.validateTemplateDefItemType(rootDLRLIDL, templateDefNode, itemType);
    }



    /**
     * This method validates a value element to the DLRL IDL contents.
     *
     * The Mapping XML DTD used for validation (The child element is not validated by
     * this method):
     * <!ELEMENT value (#PCDATA)>
     *
     * @param valueNode The Element node containing the value
     * @throws Exception
     * @roseuid 40DC0DB200A9
     */
    public void processXMLMappingValueElement(Element valueNode) throws Exception {
        xmlTraverser.traverseXMLMappingValueElement(valueNode);
    }

    /**
     * This method validates a value field element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation:
     * <!ELEMENT valueField (#PCDATA)>
     *
     * @param valueFieldNode  The Element node containing the value field
     * @throws Exception
     * @roseuid 40B1A12C0098
     */
    public void processXMLMappingValueFieldElement(Element valueFieldNode) throws Exception {
        xmlTraverser.traverseXMLMappingValueFieldElement(valueFieldNode);
    }

    /**
     * This method validates a value field element to the DLRL IDL contents and the
     * contents of the DCPS IDL (if present)
     *
     * The Mapping XML DTD used for validation: (The child element is only validated
     * by this method):
     * <!ELEMENT valueField (#PCDATA)>
     *
     * @param valueFieldNode The node containing the text child node of the valueField
     * element
     * @throws Exception
     * @roseuid 40DC198703C1
     */
    public void processXMLMappingValueFieldTextElement(Node valueFieldNode) throws Exception {
        String topicName = xmlTraverser.getLastTopicIdlName();
        boolean isDefault = xmlTraverser.isLastTopicDefault();
        String attributeName =MappingXMLTraverser.getNodeValue(valueFieldNode);
        if(attributeName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)valueFieldNode, true,
                "No value found for this value field."));
        }
        if(topicName == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)valueFieldNode, true,
                "The topic to contain attribute '"+attributeName+"' cannot be resolved."));
        }
        if(!isDefault){
            int[] types = new int[1];
            types[0] = idlType.STRUCT;
            IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)valueFieldNode, true,
                    "No IDL Struct with name '"+topicName+
                    "' found in the DCPS IDL which should contain the attribute specified by this valuefield."));
            }
            Object topicAttribute = IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, object, attributeName.trim());
            if(topicAttribute == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)valueFieldNode, true,
                    "IDL Struct with name '"+topicName+"' does not contain attribute '"+attributeName.trim()+
                    "' in the DCPS IDL."));
            }
            //perform some validation of types.
            if(valuetypeAttribute != null && !IDLTraverser.areAttributeTypesEqual(topicAttribute, valuetypeAttribute)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)valueFieldNode, true,
                    "Type mismatch detected. The type of the topic attribute does not match the type of the DLRL object attribute."));
            }
            addTopicField(topicName, attributeName, valueFieldNode);
        }

    }

    /**
     * Not implemented, as the value of the value text child node will not be
     * validated to anything (following the DDS specification)
     *
     * @param valueTextNode The node containing the text child node of the value
     * element
     * @roseuid 40DC0EC30145
     */
    public void processXMLMappingValueTextElement(Node valueTextNode) {
        //do nothing
    }

    /**
     * Validates the content of the mapping XML syntax tree with the content of the
     * DLRL and DCPS IDL syntax tree.
     *
     * @throws Exception
     * @roseuid 40B1A1D20144
     */
    public void validateMappingXMLContent() throws Exception {
        Element rootNode = (Element)rootXML.getDocumentElement();
        xmlTraverser.traverseXMLMappingDLRLElement(rootNode);
        validateTopicDefinitions((Element)rootXML.getDocumentElement());
        //TODO dds224 this is temp code for thales so they can still use this
        String temp = System.getenv("DLRL_ASSUME_RO_ACCESS_TMP");
        if(temp == null){
            validateTopicFields();//should normally be always done!
        }
    }

    //validate that things like this do not occur in the XML file:
    //mainTopic name='Topic2' typename="t1::Topic2">
    //mainTopic name='Topic2' typename="t2::Topic2">
    //IE one name with two different types!
    private void validateTopicDefinitions(Element rootElement) throws Exception{
        org.w3c.dom.NodeList mainTopics = rootElement.getElementsByTagName(MappingXMLTraverser.mainTopicElementID);
        org.w3c.dom.NodeList extensionTopics = rootElement.getElementsByTagName(MappingXMLTraverser.extensionTopicElementID);
        org.w3c.dom.NodeList placeTopics = rootElement.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
        org.w3c.dom.NodeList multiPlaceTopics = rootElement.getElementsByTagName(MappingXMLTraverser.multiPlaceTopicElementID);
        //for convience add all topic nodes into one vector.
        Vector allTopics = new Vector();
        for(int count = 0; count < mainTopics.getLength(); count++){
            allTopics.add(mainTopics.item(count));
        }
        for(int count = 0; count < extensionTopics.getLength(); count++){
            allTopics.add(extensionTopics.item(count));
        }
        for(int count = 0; count < placeTopics.getLength(); count++){
            allTopics.add(placeTopics.item(count));
        }
        for(int count = 0; count < multiPlaceTopics.getLength(); count++){
            allTopics.add(multiPlaceTopics.item(count));
        }

        //now validate all topic defintions with eachother
        for(int count = 0; count < allTopics.size(); count++){
            Element aTopic = (Element)allTopics.get(count);
            String topicName = MappingXMLTraverser.getNodeValue(aTopic.getAttributes(), MappingXMLTraverser.topicElementNameAttributeID);
            String topicType = MappingXMLTraverser.getNodeValue(aTopic.getAttributes(), MappingXMLTraverser.topicElementTypeNameAttributeID);
            for(int search = 0; search < allTopics.size(); search++){
                Element tmpTopic = (Element)allTopics.get(search);
                String tmpTopicName = MappingXMLTraverser.getNodeValue(tmpTopic.getAttributes(), MappingXMLTraverser.topicElementNameAttributeID);
                String tmpTopicType = MappingXMLTraverser.getNodeValue(tmpTopic.getAttributes(), MappingXMLTraverser.topicElementTypeNameAttributeID);
                if(topicName.equals(tmpTopicName)){
                    if(topicType == null && tmpTopicType != null && tmpTopicType.length() !=0 && !tmpTopicType.equals(topicName)){
                        throw new Exception("Mismatch in topic definitions! Topic name '"+topicName+"' was used twice with different topic type names! "+
                                            "One topic definition did not specify a topic type name, therefore making the topic type name equal to the topic name. "+
                                            "Another topic definition specified topic type name '"+tmpTopicType+"'. Topic defintions must be lined up with eachother!"+
                                            MappingXMLTraverser.produceElementTrace((Node)aTopic, "\nFirst topic XML is as follows:")+
                                            MappingXMLTraverser.produceElementTrace((Node)tmpTopic, "\nSecond topic XML is as follows:"));
                    }
                    if(topicType != null && tmpTopicType == null && topicType.length() !=0 && !topicType.equals(topicName)){
                        throw new Exception("Mismatch in topic definitions! Topic name '"+topicName+"' was used twice with different topic type names! "+
                                            "One topic definition did not specify a topic type name, therefore making the topic type name equal to the topic name. "+
                                            "Another topic definition specified topic type name '"+topicType+"'. Topic defintions must be lined up with eachother!"+
                                            MappingXMLTraverser.produceElementTrace((Node)aTopic, "\nFirst topic XML is as follows:")+
                                            MappingXMLTraverser.produceElementTrace((Node)tmpTopic, "\nSecond topic XML is as follows:"));
                    }
                    if(topicType!= null && tmpTopicType != null && !(topicType.equals(tmpTopicType))){
                        throw new Exception("Mismatch in topic definitions! Topic name '"+topicName+"' was used twice with different topic type names! "+
                                            "Detected type names used are: '"+topicType+"' and '"+tmpTopicType+"'. Topic defintions must be lined up with eachother!"+
                                            MappingXMLTraverser.produceElementTrace((Node)aTopic, "\nFirst topic XML is as follows:")+
                                            MappingXMLTraverser.produceElementTrace((Node)tmpTopic, "\nSecond topic XML is as follows:"));
                    }
                }
            }
        }
    }

    public Element getChildElementWithNameForElement(Element element, String tagName, boolean strict) throws Exception{
        org.w3c.dom.NodeList elements = element.getElementsByTagName(tagName);
        if(strict && elements.getLength() == 0){
            throw new Exception(MappingXMLTraverser.produceElementTrace(element,
                "No "+tagName+" child nodes found. Expected one ("+tagName+") child node"));
        }
        if(strict && elements.getLength() > 1){
            throw new Exception(MappingXMLTraverser.produceElementTrace(element,
                "Multiple "+tagName+" child nodes found. Expected one ("+tagName+") child node"));
        }
        if(elements.getLength() == 1){
            return (Element)elements.item(0);
        }
        return null;
    }


    public Element getClassMappingForValuetype(IdlValue value) throws Exception{
        String valueName = IDLTraverser.getIDLObjectFullyqualifiedName(value);
        org.w3c.dom.NodeList classMappings = rootXML.getElementsByTagName(MappingXMLTraverser.classMappingElementID);
        for(int subCount = 0; subCount < classMappings.getLength(); subCount++){
            Node classMappingNode = classMappings.item(subCount);
            String name = MappingXMLTraverser.getNodeValue(classMappingNode.getAttributes(), MappingXMLTraverser.classMappingNameAttributeID);
            if(name != null && name.equals(valueName)){
                return (Element)classMappingNode;
            }
        }
        return null;
    }

    public Element getTemplateDefForForwardValuetype(IdlValue value) throws Exception{
        String valueName = IDLTraverser.getIDLObjectFullyqualifiedName(value);
        org.w3c.dom.NodeList templateDefs = rootXML.getElementsByTagName(MappingXMLTraverser.templateDefElementID);
        for(int subCount = 0; subCount < templateDefs.getLength(); subCount++){
            Node templateDefNode = templateDefs.item(subCount);
            String name = MappingXMLTraverser.getNodeValue(templateDefNode.getAttributes(), MappingXMLTraverser.templateDefNameAttributeID);
            if(name != null && name.equals(valueName)){
                return (Element)templateDefNode;
            }
        }
        return null;
    }

    private void validateKeyDescriptions(Element keyDescription1, Element keyDescription2) throws Exception{
        String content1 = MappingXMLTraverser.getNodeValue(keyDescription1.getAttributes(), MappingXMLTraverser.keyDescriptionContentAttributeID);
        String content2 = MappingXMLTraverser.getNodeValue(keyDescription2.getAttributes(), MappingXMLTraverser.keyDescriptionContentAttributeID);
        if(!content1.equals(content2)){
            throw new Exception("Mismatch in content types of the keyDescription definitions!"+
                MappingXMLTraverser.produceElementTrace((Node)keyDescription1, "\nFirst keyDescription XML is as follows:")+
                MappingXMLTraverser.produceElementTrace((Node)keyDescription2, "\nSecond keyDescription XML is as follows:"));
        }
        Vector childNodes1 = MappingXMLTraverser.filterNodeList(keyDescription1.getChildNodes(), Node.ELEMENT_NODE);
        Vector childNodes2 = MappingXMLTraverser.filterNodeList(keyDescription2.getChildNodes(), Node.ELEMENT_NODE);
        if(childNodes1.size() != childNodes2.size()){
            throw new Exception("Mismatch in number of keyFields of the keyDescription definitions (should be equal number of fields)!"+
                MappingXMLTraverser.produceElementTrace((Node)keyDescription1, true, "\nFirst keyDescription XML is as follows:")+
                MappingXMLTraverser.produceElementTrace((Node)keyDescription2, true,"\nSecond keyDescription XML is as follows:"));
        }
        /* getTopicForKeyDescription returns null if the topic for this keydescription was defaultly generated */
        IdlObject topic1 = getTopicForKeyDescription(keyDescription1);
        IdlObject topic2 = getTopicForKeyDescription(keyDescription2);
        if(topic1 != null && topic2 != null){
            for(int count = 0; count < childNodes1.size(); count++){
                Node childNode1 = (Node)childNodes1.get(count);
                Node childNode2 = (Node)childNodes2.get(count);
                if(!childNode1.getNodeName().equals(MappingXMLTraverser.keyFieldElementID)){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)childNode1, true,
                        "Unexpected child element '"+childNode1.getNodeName()+"'. Allowed elements: "+MappingXMLTraverser.keyFieldElementID));
                }
                if(!childNode2.getNodeName().equals(MappingXMLTraverser.keyFieldElementID)){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)childNode2, true,
                        "Unexpected child element '"+childNode2.getNodeName()+"'. Allowed elements: "+MappingXMLTraverser.keyFieldElementID));
                }
                String keyfieldName1 = getNameForKeyField(childNode1, topic1);
                String keyfieldName2 = getNameForKeyField(childNode2, topic2);
                IdlObject keyField1 = (IdlObject)IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, topic1, keyfieldName1);
                IdlObject keyField2 = (IdlObject)IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, topic2, keyfieldName2);
                if(!IDLTraverser.areAttributeTypesEqual(keyField1, keyField2)){
                    throw new Exception("The type of keyfield '"+keyfieldName1+
                    "' (defined in topic struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(topic1)+"') is not equal to the type of keyfield '"+
                    keyfieldName2+"' (defined in topic struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(topic2)+"'). Strict equality is enforced. "+
                    MappingXMLTraverser.produceElementTrace((Node)childNode1, true, "\nFirst keyField XML is as follows:")+
                    MappingXMLTraverser.produceElementTrace((Node)childNode2, true, "\nSecond keyField XML is as follows:"));
                }

            }
        }
    }

    public String getNameForKeyField(Node keyFieldElement, IdlObject topic) throws Exception{
        Vector childNodes = MappingXMLTraverser.filterNodeList(keyFieldElement.getChildNodes(), Node.TEXT_NODE);
        if(childNodes.size() == 0){
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldElement,
                "No text child nodes found. Expected one (text) child node"));
        }
        if(childNodes.size() > 1){
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldElement,
                "Multiple text child nodes found. Expected only one (text) child node"));
        }
        Node childNode = (Node)childNodes.get(0);
        validateKeyFieldTextElement(childNode, topic);
        return MappingXMLTraverser.getNodeValue(childNode).trim();
    }

    public IdlObject getTopicForKeyDescription(Element keyDescription) throws Exception{
        IdlObject object = null;

        Node parentElement = keyDescription.getParentNode();
        if(parentElement == null || !(parentElement instanceof Element)){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescription,
                "Invalid parent element detected for the keyDescription element."));
        }
        String tagName = parentElement.getNodeName();
        //need to find the elements topic element, i.e., main topic or extension topic
        if(tagName.equals(MappingXMLTraverser.monoRelationElementID)){
            //need to locate the extension topic, or if no found the main topic!
            parentElement = parentElement.getParentNode();
            if(parentElement == null || !(parentElement instanceof Element)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescription,
                    "Invalid parent element detected for the "+tagName+" element."));
            }
            tagName = parentElement.getNodeName();
            if(!tagName.equals(MappingXMLTraverser.classMappingElementID)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescription,
                    "Invalid parent element detected for the "+tagName+" element. Expected thie keyDescription to reside"+
                    " in a "+MappingXMLTraverser.classMappingElementID+" element."));
            }
            Element tmpElement = getChildElementWithNameForElement((Element)parentElement, MappingXMLTraverser.extensionTopicElementID, false);
            if(tmpElement == null){
                tmpElement = getChildElementWithNameForElement((Element)parentElement, MappingXMLTraverser.mainTopicElementID, true);
            }
            parentElement = tmpElement;
            if(parentElement == null || !(parentElement instanceof Element)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescription,
                    "Invalid parent element detected for the keyDescription element."));
            }
            tagName = parentElement.getNodeName();
        } else if(tagName.equals(MappingXMLTraverser.multiRelationElementID)){
            parentElement = (Node)MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase((Element)parentElement,
                                                                        MappingXMLTraverser.multiPlaceTopicElementID);
            if(parentElement == null || !(parentElement instanceof Element)){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescription,
                    "Invalid parent element detected for the "+tagName+" element."));
            }
            tagName = parentElement.getNodeName();
        }
        if(!(tagName.equals(MappingXMLTraverser.extensionTopicElementID) ||
           tagName.equals(MappingXMLTraverser.mainTopicElementID) ||
           tagName.equals(MappingXMLTraverser.multiPlaceTopicElementID) ||
           tagName.equals(MappingXMLTraverser.placeTopicElementID) )){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescription,
                "Invalid parent element detected for the keyDescription element. Expected one of the following elements"+
                " as parent: "+MappingXMLTraverser.mainTopicElementID+" or "+MappingXMLTraverser.extensionTopicElementID+
                " or "+MappingXMLTraverser.placeTopicElementID+" or "+MappingXMLTraverser.multiPlaceTopicElementID+
                " or "+MappingXMLTraverser.monoRelationElementID+" or "+MappingXMLTraverser.multiRelationElementID));
        }
        String structName = null;
        String typeName = null;
        //validate the name attribute
        structName = MappingXMLTraverser.getNodeValue(((Element)parentElement).getAttributes(), MappingXMLTraverser.topicElementNameAttributeID);
        validateTopicName((Element)parentElement, structName);
        typeName = MappingXMLTraverser.getNodeValue(((Element)parentElement).getAttributes(), MappingXMLTraverser.topicElementTypeNameAttributeID);
        String topicName = typeName;
        if(topicName == null){
            topicName = structName;
        }
        if(topicName != null && !MappingXMLTraverser.isElementDefault(parentElement)){
            int[] types = new int[1];
            types[0] = idlType.STRUCT;
            object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicName);
            if(object == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace(parentElement,
                    "No IDL Struct with name '"+topicName+"' found in the DCPS IDL."));
            }
        }
        return object;
    }

    public void validateKeyFieldTextElement(Node keyFieldNode, IdlObject object) throws Exception{
        String attributeName = MappingXMLTraverser.getNodeValue(keyFieldNode);

        if(attributeName ==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                "No value specified."));
        }

        Node parentNode = keyFieldNode.getParentNode().getParentNode();
        /* disallow keyfields with types like "lar.oid" to indicate keyfields in nested structs for any keydescription
         * with a full or simple oid mapping
         */
        String content = MappingXMLTraverser.getNodeValue(parentNode.getAttributes(), MappingXMLTraverser.keyDescriptionContentAttributeID);
        if(content == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)parentNode,
                "The '"+MappingXMLTraverser.keyDescriptionContentAttributeID+"' attribute must be defined."));
        }
        if((content.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID) ||
                content.equals(MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID)) &&
                attributeName.indexOf('.') != -1){
            throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                "Detected a "+MappingXMLTraverser.keyFieldElementID+" which was nested in a inner type. This is not "+
                "(yet) supported for "+MappingXMLTraverser.keyFieldElementID+" elements within a "+
                MappingXMLTraverser.keyDescriptionElementID+" with a "+
                MappingXMLTraverser.keyDescriptionContentAttributeID+" value of "+
                MappingXMLTraverser.keyDescriptionContentAttributeFullOidID+" or "+
                MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID));
        }

        if(!xmlTraverser.isLastTopicDefault()){
            Object attribute = IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, object, attributeName.trim());
            if(attribute == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                    "Failed to locate attribute '"+attributeName.trim()+"' in IDL Struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"'."));
            }
            if(attribute instanceof String){
                throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                    "Attribute '"+attributeName.trim()+"' in IDL Struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"' is of an illegal type "+
                    "(enumeration field)."));
            }
            int[] excludeTypes = new int[2];
            excludeTypes[0] = idlType.UNION;
            excludeTypes[1] = idlType.UNION_MEMBER;
            for(int tmpCount =0; tmpCount < excludeTypes.length; tmpCount++){
                if(((IdlObject)attribute).idlType() == excludeTypes[tmpCount]){
                    throw new Exception(MappingXMLTraverser.produceElementTrace(keyFieldNode, true,
                        "Attribute '"+attributeName.trim()+"' in IDL Struct '"+IDLTraverser.getIDLObjectFullyqualifiedName(object)+
                                                                "' is of an illegal type (a union or union member)."));
                }
            }
        }
    }

    private void validateTopicName(Element element, String name) throws Exception{
        //TODO ID: determine if _ are allowed as first char of a topic name
        if(!name.matches("[a-zA-Z_-][a-zA-Z0-9_-]*")){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)element,
                                "Illegal topic name '"+name+"'. A topic name is an identifier for a topic, and is "+
                                "defined as any series of characters a...z, A...Z, 0...9, '-', '_' "+
                                "but may not start with a digit. The provided topic name violates this definition"));
        }
    }

    private void addTopicField(String topicName, String attributeName, Node fieldNode) throws Exception{
        java.util.Set fields = (java.util.Set) topics.get(topicName);
        if(fields == null){
            fields = new java.util.HashSet();
            topics.put(topicName, fields);
        }
        Field aField = getFieldByName(fields, attributeName);
        if(aField == null){
            aField = new Field(attributeName, topicName);
            fields.add(aField);
        }
        aField.addNode(fieldNode);
    }

    private void validateTopicFields() throws Exception {
        java.util.Iterator iterator = topics.values().iterator();
        while(iterator.hasNext()){
            java.util.HashSet fields = (java.util.HashSet)iterator.next();
            java.util.Iterator fieldsIterator = fields.iterator();
            while(fieldsIterator.hasNext()){
                Field aField = (Field)fieldsIterator.next();
                aField.validate();
            }
        }
    }

    private Field getFieldByName(java.util.Set fields, String name){
        java.util.Iterator iterator = fields.iterator();
        while(iterator.hasNext()){
            Field aField = (Field)iterator.next();
            if(aField.getName().equals(name)){
                return aField;
            }
        }
        return null;
    }

    private class Field{

        private String name;
        private String topicName;
        private java.util.Vector keyfields =  new java.util.Vector();
        private java.util.Vector mainTopicKeyfields =  new java.util.Vector();
        private java.util.Vector extensionTopicKeyfields =  new java.util.Vector();
        private java.util.Vector valuefields =  new java.util.Vector();
        private java.util.Vector multiPlaceTopicNodes =  new java.util.Vector();
        private java.util.Vector validityFieldNodes =  new java.util.Vector();

        private Field(String name, String topicName){
            this.topicName = topicName;
            this.name = name;
        }

        private void addNode(Node aNode) throws Exception{
            while(aNode.getNodeType() != Node.ELEMENT_NODE){
                aNode = aNode.getParentNode();
            }
            String nodeName = aNode.getNodeName();
            if(nodeName.equals(MappingXMLTraverser.valueFieldElementID)){
                this.valuefields.add(aNode);
            } else if(nodeName.equals(MappingXMLTraverser.multiPlaceTopicElementID)){
                this.multiPlaceTopicNodes.add(aNode);
            } else if(nodeName.equals(MappingXMLTraverser.keyFieldElementID)){
                /* locate the keydescription node, each key field should be defined in a keydescription */
                Node parentNode = aNode.getParentNode();
                while(parentNode!= null && !parentNode.getNodeName().equals(MappingXMLTraverser.keyDescriptionElementID)){
                    parentNode = parentNode.getParentNode();
                }
                if(parentNode == null){
                    throw new Exception(MappingXMLTraverser.produceElementTrace(aNode, true,
                    "The "+MappingXMLTraverser.keyFieldElementID+" was not defined in a '"+MappingXMLTraverser.keyDescriptionElementID+"' element."));
                }
                /* now locate the extension topic */
                Node tmpParentNode = parentNode;
                while(parentNode!= null && !(parentNode.getNodeName().equals(MappingXMLTraverser.extensionTopicElementID))){
                    parentNode = parentNode.getParentNode();
                }
                if(parentNode != null){
                    this.extensionTopicKeyfields.add(aNode);
                } else {
                    /* or if not found, then search for the main topic */
                    while(tmpParentNode!= null && !(tmpParentNode.getNodeName().equals(MappingXMLTraverser.mainTopicElementID))){
                        tmpParentNode = tmpParentNode.getParentNode();
                    }
                    if(tmpParentNode != null){
                        this.mainTopicKeyfields.add(aNode);
                    } else {
                        /* no extension or main topic means its just a key field */
                        this.keyfields.add(aNode);
                    }
                }
            } else if(nodeName.equals(MappingXMLTraverser.validityFieldElementID)){
                this.validityFieldNodes.add(aNode);
            } else {
                throw new Exception(MappingXMLTraverser.produceElementTrace(aNode, true,
                    "Unexpected child element. Expected a "+MappingXMLTraverser.keyFieldElementID+" or "+MappingXMLTraverser.valueFieldElementID+" element."));
            }
        }

        private String getName(){
            return this.name;
        }

        private String getTopicName(){
            return this.topicName;
        }

        private void validate() throws Exception{
            //Rule 1: a field that is an index field may only be mapped once
            if(multiPlaceTopicNodes.size() > 0){
                if(multiPlaceTopicNodes.size() != 1){
                    String message = "The field '"+name+"' defined in topic struct '"+topicName+
                        "' was mapped as an index field for multiple MultiPlaceTopic elements, only one is allowed";
                    for(int count = 0; count < multiPlaceTopicNodes.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopicNodes.get(count), false, "Multiplace topic (containing the index field) "+count+ ": ");
                    }
                    throw new Exception (message);
                } else if(keyfields.size() != 0 || mainTopicKeyfields.size() != 0 || extensionTopicKeyfields.size() != 0 ||
                        valuefields.size() != 0 || validityFieldNodes.size() != 0){
                    String message = "The field '"+name+"' defined in topic struct '"+topicName+
                        "' was mapped as an index field, but also as another type of field. An index field may only be mapped once. The following XML code is involved:";
                    for(int count = 0; count < multiPlaceTopicNodes.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopicNodes.get(count), false, "Multiplace topic (containing the index field) "+count+ ": ");
                    }
                    for(int count = 0; count < keyfields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)keyfields.get(count), true, "Keyfield "+count+ ": ");
                    }
                    for(int count = 0; count < mainTopicKeyfields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)mainTopicKeyfields.get(count), true, "(Maintopic) keyfield "+count+ ": ");
                    }
                    for(int count = 0; count < extensionTopicKeyfields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)extensionTopicKeyfields.get(count), true, "(Extensiontopic) keyfield "+count+ ": ");
                    }
                    for(int count = 0; count < valuefields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)valuefields.get(count), true, "Valuefield "+count+ ": ");
                    }
                    for(int count = 0; count < validityFieldNodes.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)validityFieldNodes.get(count), true, "MonoRelation "+count+ ": ");
                    }
                    throw new Exception(message);
                }
            }
            //Rule 2: only one value field may be mapped, even in multiple classMapping, not complying leads to
            //undefined behavior on writer side (i.e. what change is commited)
            if(valuefields.size() > 1){
                String message = "The field '"+name+"' defined in topic struct '"+topicName+
                    "' was mapped onto multiple (mono/multi) attributes. The following XML code is involved:";
                for(int count = 0; count < valuefields.size(); count++){
                    message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)valuefields.get(count), true, "Valuefield "+count+ ": ");
                }
                throw new Exception(message);
            }
            //Rule 3: if node is keyfield defined in main topic or extension topic, then there must be a
            //valuefield node under a mono attribute unless this keyfield is defined in a keydescription with a content
            //of full or simple oid
            if(mainTopicKeyfields.size() > 0 && valuefields.size() == 0 && MappingXMLTraverser.getNodeValue(((Node)mainTopicKeyfields.get(0)).getParentNode().getAttributes(), MappingXMLTraverser.keyDescriptionContentAttributeID).equals(MappingXMLTraverser.keyDescriptionContentAttributeNoOidID)){
                String message = "The field '"+name+"' defined in topic struct '"+topicName+
                    "' was mapped as a keyfield contained in a mainTopic, but never mapped as a required mono attribute. The following XML code is involved:";
                for(int count = 0; count < mainTopicKeyfields.size(); count++){
                    message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)mainTopicKeyfields.get(count), true, "Keyfield "+count+ ": ");
                }
                throw new Exception(message);
            }
            if(extensionTopicKeyfields.size() > 0 && valuefields.size() != 0){
                String message = "The field '"+name+"' defined in topic struct '"+topicName+
                    " was incorrectly mapped onto a valuefield. Such fields may not be mapped as they are already mapped "+
                    "implicitly by means of the classmapping of the parent class. The following XML code is involved:";
                for(int count = 0; count < extensionTopicKeyfields.size(); count++){
                    message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)extensionTopicKeyfields.get(count), true, "Keyfield "+count+ ": ");
                }
                for(int count = 0; count < valuefields.size(); count++){
                    message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)valuefields.get(count), true, "Valuefield "+count+ ": ");
                }
                throw new Exception(message);
            }
            //Rule 4: if more then 1 key field nodes exist, then one must be defined in main or extension topic
            if(keyfields.size() > 1 && mainTopicKeyfields.size() == 0 && extensionTopicKeyfields.size() == 0){
                String message = "The field '"+name+"' defined in topic struct '"+topicName+
                    "' was mapped onto more then one keyfield element, but not defined in a keyfield contained in a mainTopic or extensionTopic element. The following XML code is involved:";
                for(int count = 0; count < keyfields.size(); count++){
                    message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)keyfields.get(count), true, "Keyfield "+count+ ": ");
                }
                throw new Exception(message);
            }
            //Rule 5: A field defined as a validity field in a mono relation may not be defined as anything else
            if(validityFieldNodes.size() > 0){
                if(validityFieldNodes.size() != 1){
                    String message = "The field '"+name+"' defined in topic struct '"+topicName+
                        "' was mapped as a validity field for multiple MonoRelation elements, only one mapping is allowed";
                    for(int count = 0; count < validityFieldNodes.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)validityFieldNodes.get(count), true, "MonoRelation (containing the validity field) "+count+ ": ");
                    }
                    throw new Exception (message);
                } else if(keyfields.size() != 0 || mainTopicKeyfields.size() != 0 || extensionTopicKeyfields.size() != 0 ||
                        valuefields.size() != 0 || multiPlaceTopicNodes.size() != 0){
                    String message = "The field '"+name+"' defined in topic struct '"+topicName+
                        "' was mapped as a validity field, but also as another type of field. A validity field may only be mapped once. The following XML code is involved:";
                    for(int count = 0; count < validityFieldNodes.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)validityFieldNodes.get(count), false, "MonoRelation  (containing the validity field) "+count+ ": ");
                    }
                    for(int count = 0; count < keyfields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)keyfields.get(count), true, "Keyfield "+count+ ": ");
                    }
                    for(int count = 0; count < mainTopicKeyfields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)mainTopicKeyfields.get(count), true, "(MainTopic) keyfield "+count+ ": ");
                    }
                    for(int count = 0; count < extensionTopicKeyfields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)extensionTopicKeyfields.get(count), true, "(ExtensionTopic) keyfield "+count+ ": ");
                    }
                    for(int count = 0; count < valuefields.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)valuefields.get(count), true, "Valuefield "+count+ ": ");
                    }
                    for(int count = 0; count < multiPlaceTopicNodes.size(); count++){
                        message = message+"\n"+MappingXMLTraverser.produceElementTrace((Node)multiPlaceTopicNodes.get(count), false, "MultiPlaceTopic (containing the index field) "+count+ ": ");
                    }
                    throw new Exception(message);
                }
            }
        }
    }
}
