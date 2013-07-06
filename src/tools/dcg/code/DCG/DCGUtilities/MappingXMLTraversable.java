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
package DCG.DCGUtilities;

import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
 * This interface defines the methods invoked by the MappingXMLTraverser class and 
 * must be implemented by any class that uses the MappingXMLTraverser class. Note 
 * that a callback must be made from every process method that has a corresponding 
 * traverse method in the MappingXMLTraverser class. To clarify the 
 * processExtensionTopicElement(..) method will be used
 * 
 * public void processExtensionTopicElement(Element extensionTopicNode) throws 
 * Exception{
 *     (...user code before traversing the child elements of the extensionTopic...)
 *     
 * mappingXMLTraverserInstance.traverseXMLMappingExtensionTopicElement(extensionTop
 * icNode);
 *     (...user code after traversing the child elements of the extensionTopic...)
 * }
 * Note that not each process method has a corresponding traverse method, because 
 * not each element has child to traverse.
 */
public interface MappingXMLTraversable {
    
    /**
     * This method will be called by the MappingXMLTraverser each time an 
     * associationDef element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param associationNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDBE0244
     */
    public void processXMLMappingAssociationDefElement(Element associationNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a classMapping 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param classMappingNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDCB00AE
     */
    public void processXMLMappingClassMappingElement(Element classMappingNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a 
     * compoRelationDef element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param compoRelationNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDCE009E
     */
    public void processXMLMappingCompoRelationDefElement(Element compoRelationNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time an enumDef 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param enumDefNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDD001B8
     */
    public void processXMLMappingEnumDefElement(Element enumDefNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time an 
     * extensionTopic element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param extensionTopicNode The Mapping XML element that needs to be processed 
     * and traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDD201B8
     */
    public void processXMLMappingExtensionTopicElement(Element extensionTopicNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a 
     * keyDescription element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param keyDescriptionNode The Mapping XML element that needs to be processed 
     * and traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDD4016A
     */
    public void processXMLMappingKeyDescriptionElement(Element keyDescriptionNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a 
     * validityField element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param node The Mapping XML element that needs to be processed 
     * and traversed further (if possible)
     * @throws Exception
     */
    public void processXMLMappingValidityFieldElement(Element node) throws Exception;

    /**
     * This method will be called by the MappingXMLTraverser each time a keyField 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param keyFieldNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDD60041
     */
    public void processXMLMappingKeyFieldElement(Element keyFieldNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a text child 
     * element of the keyField element is found. The corresponding traverse method in 
     * the MappingXMLTraverser (if present) should be called from the implementation 
     * of this process method, unless of course other functionality is desired. Note 
     * that functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param keyFieldNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDD800BE
     */
    public void processXMLMappingKeyFieldTextElement(Node keyFieldNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a local element 
     * is found. The corresponding traverse method in the MappingXMLTraverser (if 
     * present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param localNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDDA014B
     */
    public void processXMLMappingLocalElement(Element localNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a mainTopic 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param mainTopicNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDDC035E
     */
    public void processXMLMappingMainTopicElement(Element mainTopicNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a monoAttribute 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param monoAttributeNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDDE0235
     */
    public void processXMLMappingMonoAttributeElement(Element monoAttributeNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a monoRelation 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param monoRelationNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDE00060
     */
    public void processXMLMappingMonoRelationElement(Element monoRelationNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a 
     * MultiAttribute element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param multiAttributeNode The Mapping XML element that needs to be processed 
     * and traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDE20003
     */
    public void processXMLMappingMultiAttributeElement(Element multiAttributeNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a 
     * MultiPlaceTopic element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param multiPlaceTopicNode The Mapping XML element that needs to be processed 
     * and traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDE30274
     */
    public void processXMLMappingMultiPlaceTopicElement(Element multiPlaceTopicNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a multiRelation 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param multiRelationNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDE603AC
     */
    public void processXMLMappingMultiRelationElement(Element multiRelationNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time an placeTopic 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param placeTopicNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDE80300
     */
    public void processXMLMappingPlaceTopicElement(Element placeTopicNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time an relation 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param relationNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDEA0226
     */
    public void processXMLMappingRelationElement(Element relationNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a TemplateDef 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param templateDefNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDF1037E
     */
    public void processXMLMappingTemplateDefElement(Element templateDefNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a value element 
     * is found. The corresponding traverse method in the MappingXMLTraverser (if 
     * present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param valueFieldNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDF7038D
     */
    public void processXMLMappingValueElement(Element valueFieldNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a valueField 
     * element is found. The corresponding traverse method in the MappingXMLTraverser 
     * (if present) should be called from the implementation of this process method, 
     * unless of course other functionality is desired. Note that functionality can be 
     * inserted before AND after the corresponding traverse method is called. This 
     * allows for maximum flexibility when traversing the XML tree structure.
     * 
     * @param valueFieldNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDF301D8
     */
    public void processXMLMappingValueFieldElement(Element valueFieldNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a text child 
     * element of the value element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param valueFieldNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDF5017A
     */
    public void processXMLMappingValueFieldTextElement(Node valueFieldNode) throws Exception;
    
    /**
     * This method will be called by the MappingXMLTraverser each time a text child 
     * element of the value element is found. The corresponding traverse method in the 
     * MappingXMLTraverser (if present) should be called from the implementation of 
     * this process method, unless of course other functionality is desired. Note that 
     * functionality can be inserted before AND after the corresponding traverse 
     * method is called. This allows for maximum flexibility when traversing the XML 
     * tree structure.
     * 
     * @param valueNode The Mapping XML element that needs to be processed and 
     * traversed further (if possible)
     * @throws Exception
     * @roseuid 40DBDDFF03CC
     */
    public void processXMLMappingValueTextElement(Node valueNode) throws Exception;
}
