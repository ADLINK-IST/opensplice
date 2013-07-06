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

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Enumeration;
import org.openorb.compiler.idl.reflect.idlInterface;
import org.openorb.compiler.idl.reflect.idlObject;
import org.openorb.compiler.idl.reflect.idlParameter;
import org.openorb.compiler.idl.reflect.idlType;
import org.openorb.compiler.object.IdlArray;
import org.openorb.compiler.object.IdlAttribute;
import org.openorb.compiler.object.IdlConst;
import org.openorb.compiler.object.IdlEnum;
import org.openorb.compiler.object.IdlExcept;
import org.openorb.compiler.object.IdlFactory;
import org.openorb.compiler.object.IdlFixed;
import org.openorb.compiler.object.IdlIdent;
import org.openorb.compiler.object.IdlInterface;
import org.openorb.compiler.object.IdlModule;
import org.openorb.compiler.object.IdlNative;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlOp;
import org.openorb.compiler.object.IdlSequence;
import org.openorb.compiler.object.IdlSimple;
import org.openorb.compiler.object.IdlStateMember;
import org.openorb.compiler.object.IdlString;
import org.openorb.compiler.object.IdlStruct;
import org.openorb.compiler.object.IdlStructMember;
import org.openorb.compiler.object.IdlTypeDef;
import org.openorb.compiler.object.IdlUnion;
import org.openorb.compiler.object.IdlUnionMember;
import org.openorb.compiler.object.IdlValue;
import org.openorb.compiler.object.IdlValueBox;
import org.openorb.compiler.object.IdlWString;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import DCG.Core.MainModel;
import DCG.DCGUtilities.IDLTraversable;
import DCG.DCGUtilities.IDLTraverser;
import DCG.DCGUtilities.MappingXMLTraversable;
import DCG.DCGUtilities.MappingXMLTraverser;

/**
 * This class generates a merged XML file containing information from the DLRL IDL
 * file (or any IDL file) and from the Mapping XML file (DTD must be conform the
 * DDS specification)
 *
 * The generated XML file will have an XML syntax according to the following DTD:
 * <!ELEMENT IDL
 *  (TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST | EXCEPTION | INTERFACE |
 * MODULE | VALUEFORWARDDEF | VALUEABSTRACTDEF | VALUEDEF | VAlUEBOXDEF)+>
 *
 * <!ELEMENT MODULE
 *  (TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST | EXCEPTION | INTERFACE |
 * MODULE | VALUEFORWARDDEF | VALUEABSTRACTDEF | VALUEDEF | VAlUEBOXDEF)+>
 * <!ATTLIST MODULE
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT INTERFACE
 *  (INTERFACEINHERITS | TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST |
 * EXCEPTION | ATTRIBUTE | OPERATION)*>
 * <!ATTLIST INTERFACE
 *  NAME        CDATA       #REQUIRED
 *  ABSTRACT    (true|false)    #REQUIRED
 *  LOCAL       (true|false)    #REQUIRED>
 *
 * <!ELEMENT INTERFACEINHERITS EMPTY>
 * <!ATTLIST INTERFACEINHERITS
 *  NAME        CDATA       #REQUIRED>
 *
 * <!ELEMENT VAlUEBOXDEF
 *  (mainTopic,extensionTopic?, (TYPEREF | STRUCT | UNION | ENUM | SEQUENCE |
 * STRING | WSTRING | FIXED))>
 * <!ATTLIST VAlUEBOXDEF
 *  NAME        CDATA       #REQUIRED>
 *
 * <!ELEMENT VALUEFORWARDDEF EMPTY>
 * <!ATTLIST VALUEFORWARDDEF
 *  NAME        CDATA               #REQUIRED
 *  ABSTRACT    (true|false)            #REQUIRED
 *  pattern     (List | StrMap | IntMap | Set)  #REQUIRED
 *         itemType CDATA               #REQUIRED>
 *
 * <!ELEMENT VALUEABSTRACTDEF
 *  (mainTopic, extensionTopic?,(VALUEINHERITS | INTERFACEINHERITS | TYPEDEF |
 * STRUCT | UNION | ENUM | NATIVE | CONST | EXCEPTION | ATTRIBUTE | OPERATION)*)>
 * <!ATTLIST VALUEABSTRACTDEF
 *  NAME        CDATA       #REQUIRED
 *  ABSTRACT    (true)      #REQUIRED
 *  TRUNCATABLE (true|false)    #REQUIRED>
 *
 * <!ELEMENT VALUEINHERITS EMPTY>
 * <!ATTLIST VALUEINHERITS
 *  NAME        CDATA       #REQUIRED>
 *
 * <!ELEMENT VALUEDEF
 *  (mainTopic,extensionTopic?, (VALUEINHERITS | INTERFACEINHERITS | TYPEDEF |
 * STRUCT | UNION | ENUM | NATIVE | CONST | EXCEPTION | ATTRIBUTE | OPERATION |
 * STATEMEMBER | INIT)*)>
 * <!ATTLIST VALUEDEF
 *  NAME        CDATA       #REQUIRED
 *  CUSTOM      (true|false)    #REQUIRED
 *  TRUNCATABLE (true|false)    #REQUIRED>
 *
 * <!ELEMENT STATEMEMBER
 *  ( (TYPEREF | STRUCT | UNION | ENUM | SEQUENCE | STRING | WSTRING | FIXED),
 * DECLARATOR+, ((placeTopic?,keyDescription) | (multiPlaceTopic,keyDescription) |
 * (multiPlaceTopic,valueField+) | (placeTopic?,valueField+) | local), relation*)>
 * <!ATTLIST STATEMEMBER
 *  PUBLIC      (true|false)    #REQUIRED
 *  CompoRelation   (true|false)    #REQUIRED>
 *
 * <!ELEMENT INIT
 *  (PARAMETER)*>
 * <!ATTLIST INIT
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT RAISES EMPTY>
 * <!ATTLIST RAISES
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT DECLARATOR
 *  (ARRAY)*>
 * <!ATTLIST DECLARATOR
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT ARRAY EMPTY>
 * <!ATTLIST ARRAY
 *  LENGTH      CDATA   #REQUIRED>
 *
 * <!ELEMENT TYPEREF EMPTY>
 * <!ATTLIST TYPEREF
 *  TYPE        CDATA   #REQUIRED>
 *
 * <!ELEMENT SEQUENCE
 *  (TYPEREF | SEQUENCE | STRING | WSTRING | FIXED)>
 * <!ATTLIST SEQUENCE
 *  VALUE       CDATA   #REQUIRED>
 *
 * <!ELEMENT STRING EMPTY>
 * <!ATTLIST STRING
 *  LENGTH      CDATA   #IMPLIED>
 *
 * <!ELEMENT WSTRING EMPTY>
 * <!ATTLIST WSTRING
 *  LENGTH      CDATA   #IMPLIED>
 *
 * <!ELEMENT FIXED EMPTY>
 * <!ATTLIST FIXED
 *  VALUE       CDATA   #IMPLIED
 *  DECIMALVALUE    CDATA   #IMPLIED>
 *
 * <!ELEMENT CONST
 *  (TYPEREF | FIXED | STRING | WSTRING)>
 * <!ATTLIST CONST
 *  NAME        CDATA   #REQUIRED
 *  VALUE       CDATA   #REQUIRED>
 *
 * <!ELEMENT TYPEDEF
 *  ((TYPEREF | STRUCT | UNION | ENUM | SEQUENCE | STRING | WSTRING | FIXED),
 * DECLARATOR+)>
 *
 * <!ELEMENT NATIVE EMPTY>
 * <!ATTLIST NATIVE
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT STRUCT
 *  (MEMBER)+>
 * <!ATTLIST STRUCT
 *  NAME        CDATA   #REQUIRED
 *  FORWARD (true|false)    #REQUIRED>
 *
 * <!ELEMENT UNION
 *  ((TYPEREF | ENUM | DECLARATOR), CASE+)>
 * <!ATTLIST UNION
 *  NAME        CDATA   #REQUIRED
 *  FORWARD (true|false)    #REQUIRED>
 *
 * <!ELEMENT CASE
 *  (((TYPEREF  | STRUCT | UNION | ENUM | SEQUENCE | STRING | WSTRING | FIXED),
 * DECLARATOR) | CASE)>
 * <!ATTLIST CASE
 *  VALUE       CDATA   #IMPLIED>
 *
 * <!ELEMENT ENUM
 *  (ENUMMEMBER+, value*)>
 * <!ATTLIST ENUM
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT ENUMMEMBER EMPTY>
 * <!ATTLIST ENUMMEMBER
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT ATTRIBUTE
 *  (TYPEREF | STRING | WSTRING)>
 * <!ATTLIST ATTRIBUTE
 *  NAME        CDATA   #REQUIRED
 *  READONLY (true|false)   #IMPLIED>
 *
 * <!ELEMENT OPERATION
 *  ((TYPEREF | STRING | WSTRING), (PARAMETER | RAISES | CONTEXT)*)>
 * <!ATTLIST OPERATION
 *  NAME        CDATA   #REQUIRED
 *  ONEWAY  (true|false)    #REQUIRED>
 *
 * <!ELEMENT PARAMETER
 *  (TYPEREF | STRING | WSTRING)>
 * <!ATTLIST PARAMETER
 *  NAME        CDATA   #REQUIRED
 *  TYPE    (in|out|inout)  #REQUIRED>
 *
 * <!ELEMENT CONTEXT EMPTY>
 * <!ATTLIST CONTEXT
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT EXCEPTION
 *  (MEMBER)*>
 * <!ATTLIST EXCEPTION
 *  NAME        CDATA   #REQUIRED>
 *
 * <!ELEMENT MEMBER
 *  ((TYPEREF | STRUCT | UNION | ENUM | SEQUENCE | STRING | WSTRING | FIXED),
 * DECLARATOR+)>
 *
 * <!ELEMENT value (#PCDATA)>
 *
 * <!ELEMENT relation EMPTY>
 * <!ATTLIST relation
 *  class       CDATA   #REQUIRED
 *         attribute    CDATA   #REQUIRED>
 *
 * <!ELEMENT mainTopic
 *  (keyDescription)>
 * <!ATTLIST mainTopic
 *  name        CDATA   #REQUIRED>
 *
 * <!ELEMENT extensionTopic
 *  (keyDescription)>
 * <!ATTLIST extensionTopic
 *  name        CDATA   #REQUIRED>
 *
 * <!ELEMENT placeTopic
 *  (keyDescription)>
 * <!ATTLIST placeTopic
 *  name        CDATA   #REQUIRED>
 *
 * <!ELEMENT multiPlaceTopic
 *  (keyDescription)>
 * <!ATTLIST multiPlaceTopic
 *  name        CDATA   #REQUIRED
 *  indexField  CDATA   #IMPLIED>
 *
 * <!ELEMENT keyDescription
 *  (keyField*)>
 * <!ATTLIST keyDescription
 *  content (FullOid | SimpleOid | NoOid)   #REQUIRED>
 *
 * <!ELEMENT keyField
 *  (#PCDATA)>
 *
 * <!ELEMENT valueField
 *  (#PCDATA)>
 *
 * <!ELEMENT local EMPTY>
 */
public class DLRLXMLGenerator implements IDLTraversable, MappingXMLTraversable {

    /**
     * The reference to the IDL traverser used to traverse the DLRL IDL syntax tree.
     */
    private IDLTraverser idlTraverser;

    /**
     * Integer used for pretty printing of the XML output. The values represents the
     * number of tabs to be used when writing a line.
     */
    private int indent = 0;

    /**
     * The output stream class to which the output will be written
     */
    private BufferedWriter out = null;

    /**
     * Contains the DLRL IDL syntax tree.
     */
    private Vector rootIDL;

    /**
     * Contains the DCPS IDL syntax tree.
     */
    private Vector rootDCPSIDL;

    /**
     * Contains the Mapping XML Syntax tree
     */
    private Document rootXML;

    /**
     * The reference to the mapping XML traverser class used to traverse the two XML
     * trees.
     */
    private MappingXMLTraverser xmlTraverser;

    /**
     * The main model used
     */
    private MainModel model;

    private int collectionIndex = 0;
    private int relationIndex = 0;

    /**
     * Specialized constructor
     *
     * @param rootXML The XML document containing the mapping XML information
     * @param rootIDL The DLRL IDL syntax tree
     * @param rootDCPSIDL The DCPS IDL syntax tree
     * @param model The main model used
     * @roseuid 40BF38BE00BB
     */
    public DLRLXMLGenerator(Document rootXML, Vector rootIDL, Vector rootDCPSIDL, MainModel model) {
        this.rootIDL = rootIDL;
        this.rootDCPSIDL = rootDCPSIDL;
        this.rootXML = rootXML;
        idlTraverser =  new IDLTraverser(this);
        xmlTraverser =  new MappingXMLTraverser(this);
        this.model = model;
    }

    /**
     * This method will generate an XML file containing the merged contents of the
     * DLRL IDL file and the Mapping XML file. The merged XML file will follow the XML
     * DTD syntax. This method is responsible for the following part of that DTD:
     * <!ELEMENT IDL (TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST | EXCEPTION |
     * INTERFACE | MODULE | VALUEFORWARDDEF | VALUEABSTRACTDEF | VALUEDEF |
     * VAlUEBOXDEF)+>
     * @return File
     * @throws Exception
     * @roseuid 40BF322801D0
     */
    public File generateDLRLMergedXMLFile(
        java.io.File baseFile,
        java.io.File dcpsBaseFile,
        java.io.File generatedDcpsBaseFile) throws Exception
    {
        /*dcpsBaseFile and generatedDcpsBaseFile may be null*/
        String baseName;
        int index;
        String dcpsBaseName = "";
        String generatedDcpsBaseName = "";
        if(dcpsBaseFile != null)
        {
            dcpsBaseName = dcpsBaseFile.getName();
            index = dcpsBaseName.indexOf('.');
            if(index != -1)
            {
                dcpsBaseName = dcpsBaseName.substring(0, index);
            }
        }

        if(generatedDcpsBaseFile != null)
        {
            generatedDcpsBaseName = generatedDcpsBaseFile.getName();
            index = generatedDcpsBaseName.indexOf('.');
            if(index != -1)
            {
                generatedDcpsBaseName = generatedDcpsBaseName.substring(0, index);
            }
        }

        baseName = baseFile.getName();
        index = baseName.indexOf('.');
        if(index != -1)
        {
            baseName = baseName.substring(0, index);
        }
        File outputDir = model.getOutputDirectoryPath();
        outputDir.mkdirs();
        File outputFile = new File(
            outputDir.getAbsolutePath() +
            File.separator +
            baseName+
            "_idl_dcg_output_tmp_file.xml");
        out = new BufferedWriter(new FileWriter(outputFile));
        if (rootXML != null) {
            rootXML.normalize();
        }

        write("<IDL baseFile=\""+baseName+"\" dcpsBaseFile=\""+dcpsBaseName+"\" generatedDcpsBaseFile=\""+generatedDcpsBaseName+"\">");
        indent++;
        idlTraverser.traverseIDLTree(rootIDL);
        indent--;
        write("</IDL>");
        //write the last chars in the buffer
        out.close();
        return outputFile;
    }

    /**
     * This method is responsible for processing an IDL array and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT ARRAY EMPTY>
     * <!ATTLIST ARRAY
     * LENGTH       CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF2FFA0051
     */
    public void processIDLArray(IdlArray object) throws Exception {
        int[] dimensions = object.dimensions();
        for(int count =0; count < dimensions.length; count++){
            write("<ARRAY LENGTH=\""+dimensions[count]+"\"/>");
        }
    }

    /**
     * This method is responsible for processing an IDL attribute and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT ATTRIBUTE (TYPEREF | STRING | WSTRING)>
     * <!ATTLIST ATTRIBUTE
     * NAME     CDATA   #REQUIRED
     * READONLY (true|false)    #IMPLIED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF2FFC0245
     */
    public void processIDLAttribute(IdlAttribute object) throws Exception {
        write("<ATTRIBUTE NAME=\"" +IDLTraverser.getIDLObjectName(object)+"\" READONLY=\""+object.readOnly()+"\">");
        indent++;
        idlTraverser.traverseIDLAttribute(object);
        indent--;
        write("</ATTRIBUTE>");
    }

    /**
     * This method is responsible for processing an IDL constant and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT CONST (TYPEREF | FIXED | STRING | WSTRING)>
     * <!ATTLIST CONST
     * NAME     CDATA   #REQUIRED
     * VALUE        CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF2FFE035E
     */
    public void processIDLConst(IdlConst object) throws Exception {
        write("<CONST NAME=\"" +IDLTraverser.getIDLObjectFullyqualifiedName(object)+"\" VALUE=\""+object.intValue()+"\"> fromIncludedIdl=\""+object._map+"\"");
        indent++;
        idlTraverser.traverseIDLConst(object);
        indent--;
        write("</CONST>");
    }

    /**
     * This method is responsible for processing an IDL context and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT CONTEXT EMPTY>
     * <!ATTLIST CONTEXT
     * NAME CDATA #REQUIRED>
     *
     * @param context The context string that needs to be written to output.
     * @throws Exception
     * @roseuid 40BF3000037D
     */
    public void processIDLContext(String context) throws Exception {
        write("<CONTEXT NAME=\""+context+"\"/>");
    }

    /**
     * This method is responsible for processing an IDL declarator and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT DECLARATOR (ARRAY)*>
     * <!ATTLIST DECLARATOR
     * NAME     CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF3002036E
     */
    public void processIDLDeclarator(IdlObject object) throws Exception {
        String objectName = null;
        if(object instanceof IdlTypeDef){
            objectName = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        } else {
            objectName = IDLTraverser.getIDLObjectName(object);
        }

        if(object.type() instanceof IdlArray){
            write("<DECLARATOR NAME=\""+objectName+"\">");
            indent++;
            idlTraverser.traverseIDLDeclarator(object);
            indent--;
            write("</DECLARATOR>");
        } else {
            write("<DECLARATOR NAME=\""+objectName+"\"/>");
        }
    }

    /**
     * This method is responsible for processing an IDL enumeration and the Mapping
     * XML needed for this object (if any). The output is written to the output stream
     * by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT ENUM (ENUMMEMBER+, value*)>
     * <!ATTLIST ENUM
     * NAME     CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF3004033F
     */
    public void processIDLEnum(IdlEnum object) throws Exception {
        String objectName = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        write("<ENUM NAME=\""+objectName+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        idlTraverser.traverseIDLEnum(object);
        Element enumDefElement = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(), MappingXMLTraverser.enumDefElementID, MappingXMLTraverser.enumDefNameAttributeID, objectName);
        if(enumDefElement != null){
            xmlTraverser.traverseXMLMappingEnumDefElement(enumDefElement);
        } else {
            //throw new Exception("No "+MappingXMLTraverser.enumDefElementID+" found in mapping XML for enumeration '"+objectName+"'");
        }
        indent--;
        write("</ENUM>");
    }

    /**
     * This method is responsible for processing an IDL enumeration member and the
     * Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT ENUMMEMBER EMPTY>
     * <!ATTLIST ENUMMEMBER
     * NAME     CDATA   #REQUIRED>
     *
     * @param object The identifier of the enumerator member
     * @param indexOfMember The index of the enum member in the list of enum members
     * @param totalMembers The total size of the enumerator where this element belongs too
     * @throws Exception
     * @roseuid 40BF30070041
     */
    public void processIDLEnumMember(String object, int indexOfMember, int totalMembers) throws Exception {
        write("<ENUMMEMBER NAME=\""+object+"\"/>");
    }

    /**
     * This method is responsible for processing an IDL exception and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT EXCEPTION  (MEMBER)*>
     * <!ATTLIST EXCEPTION NAME CDATA #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF3008039D
     */
    public void processIDLExcept(IdlExcept object) throws Exception {
        write("<EXCEPTION NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        idlTraverser.traverseIDLExcept(object);
        indent--;
        write("</EXCEPTION>");
    }

    /**
     * This method is responsible for processing an IDL init def (also known as a
     * factory) and the Mapping XML needed for this object (if any). The output is
     * written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT INIT (PARAMETER)*>
     * <!ATTLIST INIT
     * NAME     CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF300A03AC
     */
    public void processIDLFactory(IdlFactory object) throws Exception {
        write("<INIT NAME=\"" +IDLTraverser.getIDLObjectName(object)+"\">");
        indent++;
        idlTraverser.traverseIDLFactory(object);
        indent--;
        write("</INIT>");
    }

    /**
     * This method is responsible for processing an IDL fixed keyword and the Mapping
     * XML needed for this object (if any). The output is written to the output stream
     * by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT FIXED EMPTY>
     * <!ATTLIST FIXED
     * VALUE        CDATA   #IMPLIED
     * DECIMALVALUE CDATA   #IMPLIED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF300C03CB
     */
    public void processIDLFixed(IdlFixed object) throws Exception {
        throw new Exception("\tThe IDL fixed type is not allowed within DLRL objects");
    }

    /**
     * This method is responsible for processing an IDL interface and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT INTERFACE (INTERFACEINHERITS | TYPEDEF | STRUCT | UNION | ENUM |
     * NATIVE | CONST | EXCEPTION | ATTRIBUTE | OPERATION)*>
     * <!ATTLIST INTERFACE
     * NAME     CDATA       #REQUIRED
     * ABSTRACT (true|false)    #REQUIRED
     * LOCAL        (true|false)    #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF30450282
     */
    public void processIDLInterface(IdlInterface object) throws Exception {
        write("<INTERFACE NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"\" LOCAL=\""+object.isLocal()+"\" ABSTRACT=\""+object.isAbstract()+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        idlTraverser.traverseIDLInterface(object);
        indent--;
        write("</INTERFACE>");
    }

    /**
     * This method is responsible for processing an IDL interface inheritence spec and
     * the Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT INTERFACEINHERITS EMPTY>
     * <!ATTLIST INTERFACEINHERITS
     * NAME     CDATA       #REQUIRED>
     *
     * @param inheritanceList The idlInterface array containing the interfaces that
     * are apart of the inheritancelist
     * @throws Exception
     * @roseuid 40BF31D5018C
     */
    public void processIDLInterfaceInheritance(idlInterface[] inheritanceList) throws Exception {
        for(int count = 0; count < inheritanceList.length; count++){
            if(inheritanceList[count] instanceof IdlInterface){
                IdlInterface parentInterface = (IdlInterface)inheritanceList[count];
                write("<INTERFACEINHERITS NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName((IdlObject)parentInterface)+"\"/>");
            } //else will never happen
        }
    }

    /**
     * This method is responsible for processing an IDL member and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT MEMBER ((TYPEREF | STRUCT | UNION | ENUM | SEQUENCE | STRING |
     * WSTRING | FIXED), DECLARATOR+)>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31D7012F
     */
    public void processIDLMember(IdlStructMember object) throws Exception {
        write("<MEMBER>");
        indent++;
        idlTraverser.traverseIDLMember(object);
        indent--;
        write("</MEMBER>");
    }

    /**
     * This method is responsible for processing an IDL module and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT MODULE  (TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST | EXCEPTION
     * | INTERFACE | MODULE | VALUEFORWARDDEF | VALUEABSTRACTDEF | VALUEDEF |
     * VAlUEBOXDEF)+>
     * <!ATTLIST MODULE
     * NAME     CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31D90100
     */
    public void processIDLModule(IdlObject object) throws Exception {
        write("<MODULE NAME=\"" + IDLTraverser.getIDLObjectName(object) + "\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        idlTraverser.traverseIDLModule(object);
        indent--;
        write("</MODULE>");
    }

    /**
     * This method is responsible for processing an IDL native keyword and the
     * Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT NATIVE EMPTY> <!ATTLIST NATIVE NAME CDATA #REQUIRED>
     *
     * @param object
     *            The source IDL object containing the data for output to the
     *            merged XML file
     * @throws Exception
     * @roseuid 40BF31DB011F
     */
    public void processIDLNative(IdlNative object) throws Exception {
        write("<NATIVE NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"\"/>");
    }

    /**
     * This method is responsible for processing an IDL operation and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT OPERATION ((TYPEREF | STRING | WSTRING), (PARAMETER | RAISES |
     * CONTEXT)*)>
     * <!ATTLIST OPERATION
     * NAME     CDATA   #REQUIRED
     * ONEWAY   (true|false)    #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31DD00F1
     */
    public void processIDLOperation(IdlOp object) throws Exception {
        write("<OPERATION NAME=\"" +IDLTraverser.getIDLObjectName(object)+"\" ONEWAY=\""+object.oneway()+"\">");
        indent++;
        idlTraverser.traverseIDLOperation(object);
        indent--;
        write("</OPERATION>");
    }

    /**
     * This method is responsible for processing an IDL parameter and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT PARAMETER
     * (TYPEREF | STRING | WSTRING)>
     * <!ATTLIST PARAMETER
     * NAME     CDATA   #REQUIRED
     * TYPE (in|out|inout)  #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31DF0064
     */
    public void processIDLParameter(idlParameter object) throws Exception {
        //determine mode
        String paramMode = null;
        if(object.paramMode() == idlParameter.PARAM_IN){
            paramMode = "in";
        } else if(object.paramMode() == idlParameter.PARAM_OUT){
            paramMode = "out";
        } else {
            paramMode = "inout";//default value
        }
        write("<PARAMETER NAME=\"" +IDLTraverser.getIDLObjectName((IdlObject)object)+"\" TYPE=\""+paramMode+"\">");
        indent++;
        idlTraverser.traverseIDLParameter(object);
        indent--;
        write("</PARAMETER>");
    }

    /**
     * This method is responsible for processing an IDL raises nd the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT RAISES EMPTY>
     * <!ATTLIST RAISES
     * NAME     CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31E800C2
     */
    public void processIDLRaisesException(IdlExcept object) throws Exception {
        write("<RAISES NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"\"/>");
    }

    /**
     * This method is responsible for processing an IDL sequence and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT SEQUENCE (TYPEREF | SEQUENCE | STRING | WSTRING | FIXED)>
     * <!ATTLIST SEQUENCE
     * VALUE        CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF32080096
     */
    public void processIDLSequence(IdlSequence object) throws Exception {
        write("<SEQUENCE VALUE=\""+((IdlSequence)object).getSize()+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        idlTraverser.traverseIDLSequence(object);
        indent--;
        write("</SEQUENCE>");
    }

    /**
     * This method is responsible for processing an IDL statemember and the Mapping
     * XML needed for this object (if any). The output is written to the output stream
     * by calling the write(..) method.
     *
     * NOTE:
     * When multiple declarators are defined in the IDL file and seperated with a
     * comma they are parsed as seperate state members by the preprocessor.meaning
     * that the DECLARATOR+ element, indicating one or more DECLARATOR child elements
     * will ALWAYS appear only once, not one or more times. This isnt changed in the
     * IDL 2 XML DTD as that would cause the IDL 2 XML to be incompatible with
     * the idl grammar define din the corba 2.4.2 spec.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT STATEMEMBER ( (TYPEREF | STRUCT | UNION | ENUM | SEQUENCE | STRING |
     * WSTRING | FIXED), DECLARATOR+, ((placeTopic?,keyDescription) |
     * (multiPlaceTopic,keyDescription) | (multiPlaceTopic,valueField+) |
     * (placeTopic?,valueField+) | local), relation*)>
     * <!ATTLIST STATEMEMBER
     * PUBLIC           (true|false)    #REQUIRED
     * CompoRelation    (true|false)    #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31EB014F
     */
    public void processIDLStateMember(IdlStateMember object) throws Exception {
        String attributeDeclaratorName = IDLTraverser.getIDLObjectName(object);
        //get the parent class name
        String className = IDLTraverser.getIDLObjectFullyqualifiedName((IdlObject)object.idlDefinedIn());
        //determine is this statemember is a comporelation
        boolean isCompoRelation = false;
        Element compoRelationElement = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(), MappingXMLTraverser.compoRelationElementID, MappingXMLTraverser.compoRelationClassAttributeID, className);
        if(compoRelationElement !=null && compoRelationElement.hasAttributes()){
            NamedNodeMap attributes = compoRelationElement.getAttributes();
            Node attributeNode = attributes.getNamedItem(MappingXMLTraverser.compoRelationAttributeAttributeID  );
            if(attributeNode == null){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)compoRelationElement, "Missing required attribute with attribute name '"+
                                            MappingXMLTraverser.compoRelationClassAttributeID+"' for this element."));
            }
            String attribute = MappingXMLTraverser.getNodeValue(attributeNode);
            if(attribute.equals(attributeDeclaratorName)){
                isCompoRelation = true;
            }
        }
        write("<STATEMEMBER PUBLIC=\""+object.isPublic()+"\" CompoRelation=\""+isCompoRelation+"\" typeHasVarLength=\""+isTypeVariableInLength(object.type())+"\">");
        indent++;
        idlTraverser.traverseIDLStateMember(object);
        Document sourceXML = rootXML;
        boolean mappingFound = false;
        Element classMappingElement = MappingXMLTraverser.getXMLElementWithAttributeValue(sourceXML.getDocumentElement(), MappingXMLTraverser.classMappingElementID, MappingXMLTraverser.classMappingNameAttributeID, className);
        if(classMappingElement!=null){
            Element monoAttribute = MappingXMLTraverser.getXMLElementWithAttributeValue(classMappingElement, MappingXMLTraverser.monoAttributeElementID, MappingXMLTraverser.monoAttributeNameAttributeID, attributeDeclaratorName);
            if(!mappingFound && monoAttribute != null){
                xmlTraverser.traverseXMLMappingMonoAttributeElement(monoAttribute);
                mappingFound = true;
            }
            Element multiAttribute = MappingXMLTraverser.getXMLElementWithAttributeValue(classMappingElement, MappingXMLTraverser.multiAttributeElementID, MappingXMLTraverser.multiAttributeNameAttributeID, attributeDeclaratorName);
            if(!mappingFound && multiAttribute != null){
                xmlTraverser.traverseXMLMappingMultiAttributeElement(multiAttribute);
                mappingFound = true;
            }
            Element monoRelation = MappingXMLTraverser.getXMLElementWithAttributeValue(classMappingElement, MappingXMLTraverser.monoRelationElementID, MappingXMLTraverser.monoRelationNameAttributeID, attributeDeclaratorName);
            if(!mappingFound && monoRelation != null){
                xmlTraverser.traverseXMLMappingMonoRelationElement(monoRelation);
                mappingFound = true;
                generateKeyMapping(monoRelation, object);
            }
            Element multiRelation = MappingXMLTraverser.getXMLElementWithAttributeValue(classMappingElement, MappingXMLTraverser.multiRelationElementID, MappingXMLTraverser.multiRelationNameAttributeID, attributeDeclaratorName);
            if(!mappingFound && multiRelation != null){
                xmlTraverser.traverseXMLMappingMultiRelationElement(multiRelation);
                mappingFound = true;
            }
            Element local = MappingXMLTraverser.getXMLElementWithAttributeValue(classMappingElement, MappingXMLTraverser.localElementID, MappingXMLTraverser.localNameAttributeID, attributeDeclaratorName);
            if(local!= null && !mappingFound){
                //no need to traverse the local element, doesnt have child elements...
                processXMLMappingLocalElement(local);
                mappingFound = true;
            }
        }
        processXMLMappingRelation(className, attributeDeclaratorName);
        indent--;
        write("</STATEMEMBER>");
    }

    private void generateKeyMapping(Element monoRelationElement, IdlStateMember object) throws Exception{
        /* first perform the steps to get the vector of owner key fields */
        Vector ownerKeyDexcriptionElements = MappingXMLTraverser.getXMLDirectChildElements(monoRelationElement,
                                                                    MappingXMLTraverser.keyDescriptionElementID);
        if(ownerKeyDexcriptionElements.size() != 1){//should only be 1 key description
            String name = MappingXMLTraverser.getNodeValue(monoRelationElement.getAttributes(),
                                                                MappingXMLTraverser.monoRelationNameAttributeID);
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoRelationElement, "Expected one "+
                MappingXMLTraverser.keyDescriptionElementID+" child element, but found "+
                ownerKeyDexcriptionElements.size()+"."));
        }
        Element ownerKeyDexcriptionElement = (Element)ownerKeyDexcriptionElements.get(0);
        Vector ownerKeyFields = MappingXMLTraverser.getXMLDirectChildElements(ownerKeyDexcriptionElement,
                                                                                MappingXMLTraverser.keyFieldElementID);
        String contentValue = MappingXMLTraverser.getXMLElementAttributeValue(ownerKeyDexcriptionElement,
                        MappingXMLTraverser.keyDescriptionContentAttributeID);//must always be specified
        if(contentValue == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(ownerKeyDexcriptionElement,
                "Invalid element tree detected. Expected a '"+MappingXMLTraverser.keyDescriptionElementID+
                "' element with the '"+MappingXMLTraverser.keyDescriptionContentAttributeID+"' attribute specified."));
        }

        /*now we need to locate the key fields of the target object of this mono relation.*/
        Element targetClassMapping = findClassMappingXMLElementForIdlObject(object);
        Element targetMainTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(targetClassMapping,
                                                                                MappingXMLTraverser.mainTopicElementID);
        Vector targetKeyDescriptions = MappingXMLTraverser.getXMLDirectChildElements(targetMainTopicElement,
                                                                            MappingXMLTraverser.keyDescriptionElementID);
        if(targetKeyDescriptions.size() != 1){//should only be 1 key description
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)targetMainTopicElement, "Expected one "+
                MappingXMLTraverser.keyDescriptionElementID+" child element, but found "+targetKeyDescriptions.size()+
                "."));
        }
        Element targetKeyDexcriptionElement = (Element)targetKeyDescriptions.get(0);
        Vector targetKeyFields = MappingXMLTraverser.getXMLDirectChildElements(targetKeyDexcriptionElement,
                                                                                MappingXMLTraverser.keyFieldElementID);

        /*the key fields of the owner and the target should be of equal length*/
        if(ownerKeyFields.size() != targetKeyFields.size()){
                String name = MappingXMLTraverser.getNodeValue(monoRelationElement.getAttributes(),
                                                                MappingXMLTraverser.monoRelationNameAttributeID);
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)ownerKeyDexcriptionElement, true,
                            "Mismatch in number of keyfields detected while resolving this relationship mapping. "+
                            "The 'owner' object defined "+ownerKeyFields.size()+
                            " keyfields in its keyDescription and the 'target' object defined "+
                            targetKeyFields.size()+" keyfields in its keyDescription. The number of keyfields should "+
                            "be equal. The XML for the owner keyDescription is as follows:")+
                            MappingXMLTraverser.produceElementTrace((Node)targetKeyDexcriptionElement, true,
                            "\nThe XML for the target keyDescription is as follows:"));

        }
        if(contentValue.equals(MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID)){
            Element oidKeyField = (Element)ownerKeyFields.get(0);
            Vector oidKeyFieldText = MappingXMLTraverser.filterNodeList(oidKeyField.getChildNodes(),Node.TEXT_NODE);
            if(oidKeyFieldText.size() != 1){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)oidKeyField,
                                        "Expected 1 text child node. But found "+oidKeyFieldText.size()+"."));
            }
            Node oidKeyfieldTextNode = (Node)oidKeyFieldText.get(0);
            write("<oid oidField=\""+MappingXMLTraverser.getNodeValue(oidKeyfieldTextNode)+"\"/>");
        } else if(contentValue.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID)){
            Element nameKeyField = (Element)ownerKeyFields.get(0);
            Vector nameKeyFieldText = MappingXMLTraverser.filterNodeList(nameKeyField.getChildNodes(),Node.TEXT_NODE);
            if(nameKeyFieldText.size() != 1){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)nameKeyField,
                                        "Expected 1 text child node. But found "+nameKeyFieldText.size()+"."));
            }
            Node nameKeyfieldTextNode = (Node)nameKeyFieldText.get(0);

            Element oidKeyField = (Element)ownerKeyFields.get(1);
            Vector oidKeyFieldText = MappingXMLTraverser.filterNodeList(oidKeyField.getChildNodes(),Node.TEXT_NODE);
            if(oidKeyFieldText.size() != 1){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)oidKeyField,
                                        "Expected 1 text child node. But found "+oidKeyFieldText.size()+"."));
            }
            Node oidKeyfieldTextNode = (Node)oidKeyFieldText.get(0);
            write("<oid oidField=\""+MappingXMLTraverser.getNodeValue(oidKeyfieldTextNode)+"\" nameField=\""+
                                                    MappingXMLTraverser.getNodeValue(nameKeyfieldTextNode)+"\"/>");
        } else {//no oid
            /* Now we can iterate through the key field vectors and generate the keyMapping elements correctly */
            for(int count = 0; count < ownerKeyFields.size(); count++){
                Element anOwnerKeyField = (Element)ownerKeyFields.get(count);
                Element aTargetKeyField = (Element)targetKeyFields.get(count);
                Vector ownerKeyfieldChildNodes = MappingXMLTraverser.filterNodeList(anOwnerKeyField.getChildNodes(),Node.TEXT_NODE);
                Vector targetKeyfieldChildNodes = MappingXMLTraverser.filterNodeList(aTargetKeyField.getChildNodes(), Node.TEXT_NODE);
                if(ownerKeyfieldChildNodes.size() != 1){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)anOwnerKeyField,
                                        "Expected 1 text child node. But found "+ownerKeyfieldChildNodes.size()+"."));
                } else if(targetKeyfieldChildNodes.size() != ownerKeyfieldChildNodes.size()){
                    throw new Exception(MappingXMLTraverser.produceElementTrace((Node)anOwnerKeyField, true,
                        "Text child nodes size mismatch. Relation owner keyfield defined '"+ownerKeyfieldChildNodes.size()+
                        "' text elements, but relation target keyfield has '"+targetKeyfieldChildNodes.size()+
                        "' text elements. It's illegal to define relations between objects using inconsistent keyfield text "+
                        "element sizes. The XML for the owner keyField is as follows:")+
                        MappingXMLTraverser.produceElementTrace((Node)aTargetKeyField, true,
                        "\nThe XML for the target keyField is as follows:"));
                }

                Node ownerChildNode = (Node)ownerKeyfieldChildNodes.get(0);
                Node targetCildNode = (Node)targetKeyfieldChildNodes.get(0);
                //get the corresponding child node
                String targetAttributeName = getMonoAttributeNameWhichMapsKeyField(targetClassMapping, targetCildNode);
                write("<keyMapping ownerField=\""+MappingXMLTraverser.getNodeValue(ownerChildNode)+
                    "\" targetField=\""+MappingXMLTraverser.getNodeValue(targetCildNode)+"\" targetAttributeName=\""+
                    targetAttributeName+"\"/>");


            }
        }
    }

    private String getMonoAttributeNameWhichMapsKeyField(Element classMapping, Node targetCildNode) throws Exception{
        String fieldName = MappingXMLTraverser.getNodeValue(targetCildNode);
        String name = null;
        Vector monoAttributes = MappingXMLTraverser.getXMLDirectChildElements(classMapping, MappingXMLTraverser.monoAttributeElementID);
        for(int count = 0; count < monoAttributes.size(); count++){
            Element monoAttribute = (Element)monoAttributes.get(count);
            Vector valueFields = MappingXMLTraverser.getXMLDirectChildElements(monoAttribute, MappingXMLTraverser.valueFieldElementID);
            if(valueFields.size() != 1){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)monoAttribute, true,
                    "Invalid mono attribute definition. Invalid number of valueFields detected. Required 1 valueField, but detected '"+
                    valueFields.size()+"'"));
            }
            Element valueField = (Element)valueFields.get(0);
            Vector valueFieldTextNodes = MappingXMLTraverser.filterNodeList(valueField.getChildNodes(), Node.TEXT_NODE);
            if(valueFieldTextNodes.size() != 1){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)valueField, true,
                        "Invalid mono attribute definition. No text is contained within the valuefield element."));
            }
            Node aValueFieldTextNode = (Node)valueFieldTextNodes.get(0);
            if(MappingXMLTraverser.getNodeValue(aValueFieldTextNode).equals(fieldName)){
                name = MappingXMLTraverser.getNodeValue(monoAttribute.getAttributes(), MappingXMLTraverser.monoAttributeNameAttributeID);
            }
        }
        if(name == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)targetCildNode, false,
                "Unable to locate a mono attribute which maps the shown keyfield. All keyfields must be (at least) "+
                "mapped to one mono attribute."));
        }
        return name;
    }

    private Element findClassMappingXMLElementForIdlObject(IdlStateMember object) throws Exception{
        IdlObject childObject = object.type();
        if(!(childObject instanceof IdlIdent)){
            String ownerName = "UNKNOWN";
            if(object.idlDefinedIn() != null){
                ownerName = IDLTraverser.getIDLObjectFullyqualifiedName((IdlObject)object.idlDefinedIn());
            }
            throw new Exception("Error while processing a statemember of IDL object '"+ownerName+"'. Expected an IDL identifier" +
                        "object as statemember but was of type '"+object.getClass().getName()+"'. Statemember name = "+
                        IDLTraverser.getIDLObjectFullyqualifiedName(object)+"'");
        }
        IdlObject identObject = ((IdlIdent)childObject).internalObject();
        int[] types = new int[2];
        types[0] = idlType.VALUE;
        types[1] = idlType.TYPEDEF;
        IdlObject anObject = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootIDL, types,
                                            IDLTraverser.getIDLObjectFullyqualifiedName(identObject));
        //resolve type def (nested)
        while(anObject != null && anObject instanceof IdlTypeDef){
            anObject = (IdlObject)((IdlTypeDef)anObject).original();
            if(anObject instanceof IdlIdent){
                anObject = (IdlObject)((IdlIdent)anObject).original();
            }
        }

        if((anObject != null) && (anObject instanceof IdlValue) && ((IdlValue)anObject).forward()){
            String name = IDLTraverser.getIDLObjectFullyqualifiedName(anObject);
            anObject = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootIDL, name);
            if(anObject == null){
                throw new Exception ("Unable to locate an IDL valuetype implementation for forward valuetype "+
                                "declaration '"+name+"'. It's not allowed to declare forward valuetype declarations "+
                                "without implementations.");
            }
        }
        Element classMapping = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(),
                                                                    MappingXMLTraverser.classMappingElementID,
                                                                    MappingXMLTraverser.classMappingNameAttributeID,
                                                                    IDLTraverser.getIDLObjectFullyqualifiedName(anObject));
        if(classMapping == null){
            String name = IDLTraverser.getIDLObjectFullyqualifiedName(anObject);
            throw new Exception ("Unable to locate an IDL valuetype implementation for forward valuetype "+
                            "declaration '"+name+"'. It's not allowed to declare forward valuetype declarations "+
                            "without implementations.");
        }
        return classMapping;
    }


    /**
     * This method is responsible for processing an IDL string and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT STRING EMPTY>
     * <!ATTLIST STRING
     * LENGTH       CDATA   #IMPLIED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31ED014F
     */
    public void processIDLString(IdlString object) throws Exception {
        write("<STRING LENGTH=\""+object.max()+"\" fromIncludedIdl=\""+object._map+"\"/>");
    }

    /**
     * This method is responsible for processing an IDL struct and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT STRUCT  (MEMBER)+>
     * <!ATTLIST STRUCT
     * NAME     CDATA   #REQUIRED
     * FORWARD (true|false) #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31EF0372
     */
    public void processIDLStruct(IdlStruct object) throws Exception {
        if(!object.isForward()){//ignore non forward decs
            write("<STRUCT NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(object)+
                    "\" FORWARD=\""+object.isForward()+"\" fromIncludedIdl=\""+object._map+"\">");
            indent++;
            idlTraverser.traverseIDLStruct(object);
            indent--;
            write("</STRUCT>");
        }
    }

    /*
     * A type is variable-length if it is one of the following types:
     * - The type any.
     * - A bounded or unbounded string or wide string.
     * - A bounded or unbounded sequence.
     * - An object reference or reference to a transmissible pseudo-object.
     * - A valuetype.
     * - A struct or union that contains a member whose type is variable-length.
     * - An array with a variable-length element type.
     * - A typedef to a variable-length type.
     */
    private boolean isTypeVariableInLength(
        IdlObject object) throws Exception
    {
        boolean isVariable = false;

        if(object instanceof IdlArray){
            object = (IdlObject)((IdlArray)object).internal();
        }
        if(object instanceof IdlSimple)
        {
            isVariable = isSimpleTypeVariableInLength((IdlSimple)object);
        } else if(object instanceof IdlSequence ||
                 object instanceof IdlString ||
                 object instanceof IdlWString ||
                 object instanceof IdlValue ||
                 object instanceof IdlValueBox)
        {
            isVariable = true;
        } else if(object instanceof IdlIdent)
        {
            object = ((IdlIdent)object).internalObject();
            object = IDLTraverser.getIdlObjectWithFullyQualifiedName(
                rootIDL,
                IDLTraverser.getAllIdlTypes(),
                IDLTraverser.getIDLObjectFullyqualifiedName(object));
            isVariable = isTypeVariableInLength(object);
        } else if(object instanceof IdlTypeDef)
        {
            isVariable = isTypeVariableInLength((IdlObject)((IdlTypeDef)object).original());
        }
        else if(object instanceof IdlStruct)
        {
            isVariable = isStructVariableInLength((IdlStruct)object);
        }
        else if(object instanceof IdlUnion)
        {
            isVariable = isUnionVariableInLength((IdlUnion)object);
        }
        return isVariable;
    }

    private boolean
    isSimpleTypeVariableInLength(
        IdlSimple object) throws Exception
    {
        int type;

        type = object.primitive();
        if(type == IdlSimple.ANY ||
            type == IdlSimple.OBJECT ||
            type == IdlSimple.VALUEBASE)
        {
            return true;
        }
        return false;
    }

    private boolean isStructVariableInLength(IdlStruct object) throws Exception
    {
        Enumeration members;
        boolean isVariable = false;
        IdlObject type;

        members = object.members();
        while(members.hasMoreElements() && !isVariable)
        {
            isVariable = isTypeVariableInLength(((IdlStructMember)members.nextElement()).type());
        }
        return isVariable;
    }

    private boolean isUnionVariableInLength(IdlUnion object) throws Exception
    {
        Enumeration content;
        boolean isVariable = false;
        IdlObject type;

        isVariable = isTypeVariableInLength((IdlObject)object.discriminant());

        content = object.content();
        while(content.hasMoreElements() && !isVariable){
            IdlUnionMember childObject = (IdlUnionMember)content.nextElement();
            isVariable = isTypeVariableInLength(childObject.type());
        }
        return isVariable;
    }

    /**
     * This method is responsible for processing an IDL typedef and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT TYPEDEF ((TYPEREF | STRUCT | UNION | ENUM | SEQUENCE | STRING |
     * WSTRING | FIXED), DECLARATOR+)>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31F200A4
     */
    public void processIDLTypeDef(IdlTypeDef object) throws Exception {
        write("<TYPEDEF fromIncludedIdl=\""+object._map+"\">");
        indent++;
        idlTraverser.traverseIDLTypeDef(object);
        indent--;
        write("</TYPEDEF>");
    }

    /**
     * This method is responsible for processing an IDL type spec and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT TYPEREF EMPTY>
     * <!ATTLIST TYPEREF
     *  TYPE        CDATA   #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF31FD0299
     */
    public void processIDLTypeSpec(IdlObject object) throws Exception {
        if(object instanceof IdlSimple){
            int type = ((IdlSimple)object).primitive();
            if(type == IdlSimple.ANY){
                write("<TYPEREF TYPE=\"any\"/>");
            } else if(type == IdlSimple.BOOLEAN){
                write("<TYPEREF TYPE=\"boolean\"/>");
            } else if(type == IdlSimple.CHAR){
                write("<TYPEREF TYPE=\"char\"/>");
            } else if(type == IdlSimple.DOUBLE){
                write("<TYPEREF TYPE=\"double\"/>");
            } else if(type == IdlSimple.FLOAT){
                write("<TYPEREF TYPE=\"float\"/>");
            } else if(type == IdlSimple.LONG){
                write("<TYPEREF TYPE=\"long\"/>");
            } else if(type == IdlSimple.LONGDOUBLE){
                write("<TYPEREF TYPE=\"long double\"/>");
            } else if(type == IdlSimple.LONGLONG){
                write("<TYPEREF TYPE=\"long long\"/>");
            } else if(type == IdlSimple.OBJECT){
                throw new Exception("\tThe 'Object' type is not allowed within DLRL objects");
            } else if(type == IdlSimple.OCTET){
                write("<TYPEREF TYPE=\"octet\"/>");
            } else if(type == IdlSimple.SHORT){
                write("<TYPEREF TYPE=\"short\"/>");
            } else if(type == IdlSimple.TYPECODE){
                write("<TYPEREF TYPE=\"TypeCode\"/>");
            } else if(type == IdlSimple.ULONG){
                write("<TYPEREF TYPE=\"unsigned long\"/>");
            } else if(type == IdlSimple.ULONGLONG){
                write("<TYPEREF TYPE=\"unsigned long long\"/>");
            } else if(type == IdlSimple.USHORT){
                write("<TYPEREF TYPE=\"unsigned short\"/>");
            } else if(type == IdlSimple.VALUEBASE){
                throw new Exception("\tThe 'ValueBase' type is not allowed within DLRL objects");
            } else if(type == IdlSimple.VOID){
                write("<TYPEREF TYPE=\"void\"/>");
            } else if(type == IdlSimple.WCHAR){
                write("<TYPEREF TYPE=\"wchar\"/>");
            }
        } else if(object instanceof IdlIdent){
            IdlObject identObject = ((IdlIdent)object).internalObject();
            //try and find a corresponding template def or valuetype and increase the index accordingingly
            //write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectName(identObject)+"\" INDEX=\""+index+"\"/>");
            int[] types = new int[2];
            types[0] = idlType.VALUE;
            types[1] = idlType.TYPEDEF;
            IdlObject anObject = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootIDL, types, IDLTraverser.getIDLObjectFullyqualifiedName(identObject));
            //resolve type def (nested)
            while(anObject != null && anObject instanceof IdlTypeDef){
                anObject = (IdlObject)((IdlTypeDef)anObject).original();
                if(anObject instanceof IdlIdent){
                    anObject = (IdlObject)((IdlIdent)anObject).original();
                }
            }

            if((((IdlIdent)object).idlDefinedIn() !=null) && (((IdlIdent)object).idlDefinedIn() instanceof IdlStateMember) && (anObject != null) && (anObject instanceof IdlValue)){
                if(((IdlValue)anObject).forward()){
                    Element templateDef = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(),
                                                                                MappingXMLTraverser.templateDefElementID,
                                                                                MappingXMLTraverser.templateDefNameAttributeID,
                                                                                IDLTraverser.getIDLObjectFullyqualifiedName(anObject));
                    if(templateDef != null){
                        write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\" INDEX=\""+collectionIndex+"\"/>");
                        collectionIndex++;
                    } else {
                        anObject = IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootIDL, IDLTraverser.getIDLObjectFullyqualifiedName(anObject));

                        if(anObject != null){
                            Element classMapping = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(),
                                                                                        MappingXMLTraverser.classMappingElementID,
                                                                                        MappingXMLTraverser.classMappingNameAttributeID,
                                                                                        IDLTraverser.getIDLObjectFullyqualifiedName(anObject));
                            if(classMapping != null){//we are refering to a DLRL object
                                write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\" INDEX=\""+relationIndex+"\"/>");
                                relationIndex++;
                            } else {
                                write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\"/>");
                            }
                        } else {
                            write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\"/>");
                        }
                    }
                } else {
                    Element classMapping = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(),
                                                                                MappingXMLTraverser.classMappingElementID,
                                                                                MappingXMLTraverser.classMappingNameAttributeID,
                                                                                IDLTraverser.getIDLObjectFullyqualifiedName(anObject));
                    if(classMapping != null){//we are refering to a DLRL object
                        write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\" INDEX=\""+relationIndex+"\"/>");
                        relationIndex++;
                    } else {
                        write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\"/>");
                    }
                }
            } else {
                write("<TYPEREF TYPE=\""+IDLTraverser.getIDLObjectFullyqualifiedName(identObject)+"\"/>");
            }
        } else {
            throw new Exception("Unexpected type: "+object.getClass().getName());
        }
    }

    /**
     * This method is responsible for processing an IDL union and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT UNION ((TYPEREF | ENUM | DECLARATOR), CASE+)>
     * <!ATTLIST UNION
     * NAME     CDATA   #REQUIRED
     * FORWARD (true|false) #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF320A0122
     */
    public void processIDLUnion(IdlUnion object) throws Exception {
        if(!object.isForward()){//ignore non forward decs
            boolean isDefaultDefined = isDefaultCaseForUnionDefined(object);
            boolean isDefaultAllowed = isDefaultCaseForUnionAllowed(object);
            if(isDefaultDefined && !isDefaultAllowed){
                throw new Exception("Default case in union '"+
                    IDLTraverser.getIDLObjectFullyqualifiedName(object)+
                    "' not allowed. The set of case labels completely covers the possible values of the discriminant.");
            }
            String defaultCaseValueString = "";//either remains "" or gets filled in...
            if(isDefaultAllowed){
                long defaultCaseValue = getDefaultCaseValue(object);
                defaultCaseValueString = " defaultCaseValue=\""+defaultCaseValue+"\"";
            }
            write("<UNION NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(object)+"\" FORWARD=\""+
                    object.isForward()+"\" fromIncludedIdl=\""+object._map+"\" defaultCaseAllowed=\""+isDefaultAllowed+
                    "\" hasDefaultCase=\""+isDefaultDefined+"\""+defaultCaseValueString+">");
            indent++;
            idlTraverser.traverseIDLUnion(object);
            indent--;
            write("</UNION>");
        }
    }

    public long getDefaultCaseValue(IdlUnion object) throws Exception{
        IdlObject discriminator = (IdlObject)object.discriminant();
        discriminator = IDLTraverser.resolveTypedefAndIdlIdent(discriminator);

        if(discriminator instanceof IdlEnum){
            String[] members = ((IdlEnum)discriminator).members();
            for(long count = 0; count < members.length; count++){
                if(!isCaseWithValueDefined(object, count)){
                    return count;
                }
            }
        } else if(discriminator instanceof IdlSimple){
            //for integer and char there are so many options, we just allow a default branch always
            int type = ((IdlSimple)discriminator).primitive();
            if(type == IdlSimple.boolean_type.primitive()){
                if(isCaseWithValueDefined(object, 1)){
                    return 0;
                } else {
                    return 1;
                }
            } else if(  type == IdlSimple.char_type.primitive() || type == IdlSimple.long_type.primitive() ||
                        type == IdlSimple.longlong_type.primitive() || type == IdlSimple.short_type.primitive() ||
                        type == IdlSimple.ulong_type.primitive() || type == IdlSimple.ulonglong_type.primitive() ||
                        type == IdlSimple.ushort_type.primitive()){
                int retVal = -2147483648;
                while(isCaseWithValueDefined(object, retVal)){
                    retVal++;
                    if(retVal == -2147483648){//wont ever happen, obviously, but hey funnah
                        throw new Exception("Wow you have done the (obviously not so) impossible and defined a union "+
                            "with like ultra loads of labels. My advice, leave the office, dont pass go, do not collect"+
                            " a bonus of $2000 and go directly to jail.");
                    }
                }
                return (long)retVal;
            }
        }
        throw new Exception(IDLTraverser.getIDLObjectFullyqualifiedName(object)+": Attempted to determine the "+
            "discriminant value for the default case, but failed to identify the type of the discriminant. "+
            "Only enum, integer, char or boolean types supported");
    }

    private boolean isDefaultCaseForUnionAllowed(IdlUnion object){
        IdlObject discriminator = (IdlObject)object.discriminant();
        discriminator = IDLTraverser.resolveTypedefAndIdlIdent(discriminator);

        if(discriminator instanceof IdlSimple){
            //for integer and char there are so many options, we just allow a default branch always
            int type = ((IdlSimple)discriminator).primitive();
            if(type == IdlSimple.char_type.primitive() || type == IdlSimple.long_type.primitive() || type == IdlSimple.longlong_type.primitive() ||
                                                type == IdlSimple.short_type.primitive() || type == IdlSimple.ulong_type.primitive() ||
                                                type == IdlSimple.ulonglong_type.primitive() || type == IdlSimple.ushort_type.primitive()){
                return true;
            } else if(type == IdlSimple.boolean_type.primitive() && (!isCaseWithValueDefined(object, 1) &&
                                                                     !isCaseWithValueDefined(object, 0))){
                //for boolean we only allow a default case if only the TRUE case or only the FALSE case is defined
                return true;
            }
        } else if(discriminator instanceof IdlEnum){
            String[] members = ((IdlEnum)discriminator).members();
            for(int count = 0; count < members.length; count++){
                if(!isCaseWithValueDefined(object, count)){
                    return true;
                }
            }
        }
        return false;
    }

    private boolean isCaseWithValueDefined(IdlObject object, long value){
        java.util.Enumeration content = object.content();
        while(content.hasMoreElements()){
            IdlUnionMember childObject = (IdlUnionMember)content.nextElement();
            if((value == childObject.getValue()) && !childObject.isDefault()){//compare, ignore default members
                return true;
            }
        }
        return false;
    }

    private boolean isDefaultCaseForUnionDefined(IdlUnion object){
        java.util.Enumeration content = object.content();
        while(content.hasMoreElements()){
            IdlUnionMember childObject = (IdlUnionMember)content.nextElement();
            if(childObject.isDefault()){
                return true;
            }
        }
        return false;
    }

    public void processIDLUnionDiscriminant(IdlObject object) throws Exception{
        idlTraverser.traverseIDLUnionDiscriminant(object);
    }

    /**
     * This method is responsible for processing an IDL union member and the Mapping
     * XML needed for this object (if any). The output is written to the output stream
     * by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT CASE  (((TYPEREF  | STRUCT | UNION | ENUM | SEQUENCE | STRING |
     * WSTRING | FIXED), DECLARATOR) | CASE)>
     * <!ATTLIST CASE
     * VALUE        CDATA   #IMPLIED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF320B02D8
     */
    public void processIDLUnionMember(IdlUnionMember object) throws Exception {
        if(!object.isDefault()){
            write("<CASE VALUE=\""+object.getValue()+"\">");
        } else {
            write("<CASE isDefault=\"true\">");
        }
        indent++;
        idlTraverser.traverseIDLUnionMember(object);
        indent--;
        write("</CASE>");
    }
    public void processIDLUnionBranch(Vector cases) throws Exception{
        write("<BRANCH>");
        indent++;
        for(int caseCount = 0; caseCount < cases.size(); caseCount++){
            IdlUnionMember member = (IdlUnionMember)cases.get(caseCount);
            if(!member.isDefault()){
                write("<CASE VALUE=\""+member.getValue()+"\"/>");
            } else {
                write("<CASE isDefault=\"true\"/>");
            }
        }
        idlTraverser.traverseIDLUnionMember((IdlUnionMember)cases.get(0));
        indent--;
        write("</BRANCH>");
    }
    /**
     * This method is used to determine what type of valuetype needs to be processed.
     * A forward valuetype, a valuetype def or an abstract value type. No output is
     * generated by this method. (boxed valuetype are handled in the
     * processIDLValueBox method)
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF320D00A6
     */
    public void processIDLValue(IdlValue object) throws Exception {
        idlTraverser.traverseIDLValue(object);
    }

    /**
     * This method is responsible for processing an IDL abstract valuetype and the
     * Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT VALUEABSTRACTDEF  (mainTopic, extensionTopic?,(VALUEINHERITS |
     * INTERFACEINHERITS | TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST |
     * EXCEPTION | ATTRIBUTE | OPERATION | associationDef)*)>
     * <!ATTLIST VALUEABSTRACTDEF
     * NAME     CDATA       #REQUIRED
     * ABSTRACT (true)      #REQUIRED
     * TRUNCATABLE  (true|false)    #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF320E01EE
     */
    public void processIDLValueAbstractDef(IdlValue object) throws Exception {
        // suppress any valueabstractdefs from DDS
        if (isFromDDS(object))
            return;

        String objectNameQualified = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        write("<VALUEABSTRACTDEF NAME=\""+objectNameQualified+"\" ABSTRACT=\"true\" TRUNCATABLE=\""+object.isTruncatable()+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        Document sourceXML = rootXML;
        Element classMappingElement = MappingXMLTraverser.getXMLElementWithAttributeValue(sourceXML.getDocumentElement(), MappingXMLTraverser.classMappingElementID, MappingXMLTraverser.classMappingNameAttributeID, objectNameQualified);
        if(classMappingElement==null){
            throw new Exception("No class mapping found for abstract valuetype "+objectNameQualified);
        }
        Element mainTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(classMappingElement, MappingXMLTraverser.mainTopicElementID);
        if(mainTopicElement==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)classMappingElement,
                        "Missing required main topic element."));
        }
        processXMLTopic(mainTopicElement, MappingXMLTraverser.mainTopicElementID);                  //mainTopic
        Element extensionTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(classMappingElement, MappingXMLTraverser.extensionTopicElementID);
        //only process extension topic if found
        if(extensionTopicElement!= null){
            processXMLTopic(extensionTopicElement, MappingXMLTraverser.extensionTopicElementID);                    //extensionTopic
        }

        idlTraverser.traverseIDLValueAbstractDef(object);
        indent--;
        write("</VALUEABSTRACTDEF>");
    }

    /**
     * This method is responsible for processing an IDL boxed valuetype and the
     * Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT VAlUEBOXDEF (mainTopic,extensionTopic?, (TYPEREF | STRUCT | UNION |
     * ENUM | SEQUENCE | STRING | WSTRING | FIXED), associationDef*)>
     * <!ATTLIST VAlUEBOXDEF
     * NAME     CDATA       #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF321000F4
     */
    public void processIDLValueBox(IdlValueBox object) throws Exception {
        String objectNameQualified = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        write("<VAlUEBOXDEF NAME=\""+ objectNameQualified+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        Document sourceXML = rootXML;
        Element classMappingElement = MappingXMLTraverser.getXMLElementWithAttributeValue(sourceXML.getDocumentElement(), MappingXMLTraverser.classMappingElementID, MappingXMLTraverser.classMappingNameAttributeID, objectNameQualified);
        if(classMappingElement==null){
            throw new Exception("No class mapping found for boxed valuetype "+objectNameQualified);
        }
        Element mainTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(classMappingElement, MappingXMLTraverser.mainTopicElementID);
        if(mainTopicElement==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)classMappingElement,
                        "Missing required main topic element."));
        }
        processXMLTopic(mainTopicElement, MappingXMLTraverser.mainTopicElementID);                  //mainTopic

        Element extensionTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(classMappingElement, MappingXMLTraverser.extensionTopicElementID);
        //only process extension topic if found
        if(extensionTopicElement!= null){
            processXMLTopic(extensionTopicElement, MappingXMLTraverser.extensionTopicElementID);                    //extensionTopic
        }

        idlTraverser.traverseIDLValueBox(object);
        indent--;
        write("</VAlUEBOXDEF>");
    }

    /**
     * This method is responsible for processing an IDL valuetype def and the Mapping
     * XML needed for this object (if any). The output is written to the output stream
     * by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT VALUEDEF (mainTopic,extensionTopic?, (VALUEINHERITS |
     * INTERFACEINHERITS | TYPEDEF | STRUCT | UNION | ENUM | NATIVE | CONST |
     * EXCEPTION | ATTRIBUTE | OPERATION | STATEMEMBER | INIT | associationDef)*)>
     * <!ATTLIST VALUEDEF
     * NAME     CDATA       #REQUIRED
     * CUSTOM       (true|false)    #REQUIRED
     * TRUNCATABLE  (true|false)    #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF32120039
     */
    public void processIDLValueDef(IdlValue object) throws Exception {
        // suppress any valuedefs from DDS
        if (isFromDDS(object))
            return;

        //reset relation & collection counters. they are increased each type a relation (mono/multi)
        //to a DLRL object is detected
        collectionIndex = 0;
        relationIndex = 0;
        String objectNameQualified = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        write("<VALUEDEF NAME=\""+objectNameQualified+"\" CUSTOM=\""+object.isCustom()+"\" TRUNCATABLE=\""+object.isTruncatable()+"\" fromIncludedIdl=\""+object._map+"\">");
        indent++;
        Document sourceXML = rootXML;
        Element classMappingElement = MappingXMLTraverser.getXMLElementWithAttributeValue(sourceXML.getDocumentElement(),MappingXMLTraverser.classMappingElementID, MappingXMLTraverser.classMappingNameAttributeID, objectNameQualified);
        if(classMappingElement==null){
            //throw new Exception("No class mapping found for valuetype "+objectNameQualified);
        } else {
            String targetImplName = MappingXMLTraverser.getNodeValue(classMappingElement.getAttributes(),
                                                MappingXMLTraverser.classMappingTargetClassNameAttributeID);
            String targetImplPath = MappingXMLTraverser.getNodeValue(classMappingElement.getAttributes(),
                                                MappingXMLTraverser.classMappingTargetClassIncludeFilePathNameAttributeID);
            if(targetImplName != null)
            {
                write("<TARGET_IMPL_CLASS NAME=\""+targetImplName+"\"/>");
                if(targetImplPath != null)
                {
                    write("<TARGET_IMPL_INCLUDE_PATH NAME=\""+targetImplPath+"\"/>");
                }
            }
            Element mainTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(classMappingElement, MappingXMLTraverser.mainTopicElementID);
            if(mainTopicElement==null){
                throw new Exception(MappingXMLTraverser.produceElementTrace((Node)classMappingElement,
                        "Missing required main topic element."));
            }
            generateDCPSFields(classMappingElement);
            processXMLTopic(mainTopicElement, MappingXMLTraverser.mainTopicElementID);                  //mainTopic
            Element extensionTopicElement = MappingXMLTraverser.getXMLFirstChildElementWithNameIgnoreCase(classMappingElement, MappingXMLTraverser.extensionTopicElementID);
            //only process extension topic if found
            if(extensionTopicElement!= null){
                processXMLTopic(extensionTopicElement, MappingXMLTraverser.extensionTopicElementID);                    //extensionTopic
            }
        }
        idlTraverser.traverseIDLValueDef(object);
        indent--;
        write("</VALUEDEF>");
    }

    /**
     * @param object
     * @return whether the IDLObject is owned by module DDS
     */
    private final boolean isFromDDS(IdlObject object) {
        idlObject definedIn = object.idlDefinedIn();
        return definedIn instanceof IdlModule
                && IDLTraverser.getIDLObjectName((IdlObject) definedIn)
                        .equalsIgnoreCase("DDS");
    }

    private final void generateDCPSFields(Element classMappingElement) throws Exception{
        Vector dcpsFields = new Vector();
        NodeList keyFieldNodes = classMappingElement.getElementsByTagName(MappingXMLTraverser.keyFieldElementID);
        NodeList valueFieldNodes = classMappingElement.getElementsByTagName(MappingXMLTraverser.valueFieldElementID);
        NodeList multiPlaceTopicNodes = classMappingElement.getElementsByTagName(MappingXMLTraverser.multiPlaceTopicElementID);
        NodeList validityFieldNodes = classMappingElement.getElementsByTagName(MappingXMLTraverser.validityFieldElementID);
        fillDCPSfieldsSet(dcpsFields, keyFieldNodes);
        fillDCPSfieldsSet(dcpsFields, multiPlaceTopicNodes);
        fillDCPSfieldsSet(dcpsFields, valueFieldNodes);
        fillDCPSfieldsSet(dcpsFields, validityFieldNodes);
        searchAndCorrectInnerForeignKeyUsage(dcpsFields);
        for(int count = 0; count < dcpsFields.size(); count++){
            DCPSField aField = (DCPSField)dcpsFields.get(count);
            if(aField.isKeyFieldOid){
                write("<DCPSField name=\""+aField.name+".systemId\" type=\"long\" keyType=\""+aField.getDCPSFieldKeyType(aField.keyType)
                    +"\" topic=\""+aField.topic+"\" immutable=\""+aField.immutable+"\"/>");
                write("<DCPSField name=\""+aField.name+".localId\" type=\"long\" keyType=\""+aField.getDCPSFieldKeyType(aField.keyType)
                    +"\" topic=\""+aField.topic+"\" immutable=\""+aField.immutable+"\"/>");
                write("<DCPSField name=\""+aField.name+".serial\" type=\"long\" keyType=\""+aField.getDCPSFieldKeyType(aField.keyType)
                    +"\" topic=\""+aField.topic+"\" immutable=\""+aField.immutable+"\"/>");
            } else {
                write("<DCPSField name=\""+aField.name+"\" type=\""+aField.type+"\" keyType=\""+aField.getDCPSFieldKeyType(aField.keyType)
                    +"\" topic=\""+aField.topic+"\" immutable=\""+aField.immutable+"\"/>");
            }
        }
    }

    private final void searchAndCorrectInnerForeignKeyUsage(Vector dcpsFields) throws Exception{
        DCPSField aField, tempField;
        int count;
        int index;
        String tempName;

        for(count = 0; count < dcpsFields.size(); count++){
            aField = (DCPSField)dcpsFields.get(count);
            if(aField.keyType != DCPSField._NORMAL){
                index = 0;
                tempName = aField.name;
                while(index != -1){
                    index = tempName.indexOf('.');
                    if(index == 0 || index == tempName.length()){
                        throw new Exception("Invalid position of '.' in mapped DCPS field with name '"+aField.name+
                                                                "'. Was processing following part: '"+tempName+"'");
                    } else if(index != -1){
                        tempField = findDCPSFieldWithName(dcpsFields, tempName.substring(0, index));
                        if(tempField != null){
                            tempField.immutable = true;
                        }
                        tempName = tempName.substring(index+1, tempName.length());
                    }
                }
            }
        }
    }

    private final DCPSField findDCPSFieldWithName(Vector dcpsFields, String name){
        DCPSField aField = null;
        int count;
        for(count = 0; count < dcpsFields.size(); count++){
            aField = (DCPSField)dcpsFields.get(count);
            if(aField.name.equals(name)){
                return aField;
            }
        }
        return null;
    }

    private final void fillDCPSfieldsSet(Vector dcpsFields, NodeList fieldNodes) throws Exception{
        for(int count = 0; count < fieldNodes.getLength(); count++){
            Element anElement = null;
            String value = null;
            Iterator iterator = null;
            DCPSField containedField = null;
            DCPSField aField = null;
            DCPSField newField = null;
            String topicName = null;
            int typeSuggestion = -1;
            Element holderElement;
            String holderElementName;
            NodeList tempChildren = null;
            Element multiPlaceTopicElement;
            Element topicElement;
            boolean isKeyFieldOid = false;
            String type = "UNKNOWN";
            String topicTypeName = null;

            anElement = (Element)fieldNodes.item(count);

            /* we need to process the index field of the multiplacetopic, if present */
            if(anElement.getTagName().equals(MappingXMLTraverser.multiPlaceTopicElementID)){
                if(anElement.hasAttribute(MappingXMLTraverser.multiPlaceTopicIndexAttributeID)){
                    topicName = anElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                    topicTypeName = anElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                    if(topicTypeName == null)
                    {
                        topicTypeName = topicName;
                    }
                    value = anElement.getAttribute(MappingXMLTraverser.multiPlaceTopicIndexAttributeID);
                    typeSuggestion = DCPSField._KEY;
                }
            /* we also need to process the validity field */
            } else if(anElement.getTagName().equals(MappingXMLTraverser.validityFieldElementID)){
                String validityField = anElement.getAttribute(MappingXMLTraverser.validityFieldNameAttributeID);
                if(validityField == null){
                    throw new Exception(MappingXMLTraverser.produceElementTrace(anElement,
                    "No (required) attribute '"+MappingXMLTraverser.validityFieldNameAttributeID+"' found for this element."));
                }
                value = validityField;
                typeSuggestion = DCPSField._NORMAL;
                /* now determine the topic is belongs too */
                holderElement = (Element)anElement.getParentNode();
                tempChildren = holderElement.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
                if(tempChildren.getLength() != 0){
                    topicElement = (Element)tempChildren.item(0);
                    topicName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                    topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                }
                if(topicName == null){//ie no place topic found
                    //fetch the main topic
                    //goto the classmapping and get the main topic element
                    tempChildren = ((Element)holderElement.getParentNode()).getElementsByTagName(
                                                                            MappingXMLTraverser.mainTopicElementID);
                    if(tempChildren.getLength() != 1){
                        throw new Exception(MappingXMLTraverser.produceElementTrace(holderElement.getParentNode(),
                            "Invalid element tree detected. Expected a "+
                            MappingXMLTraverser.mainTopicElementID+" as a child element."));
                    }
                    topicElement = (Element)tempChildren.item(0);
                    topicName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                    topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);

                }
                if(topicTypeName == null)
                {
                    topicTypeName = topicName;
                }
            } else {
                /* now we need to process value and key fields. Note that if a keyfields is an OID we will take
                 * special steps to ensure it's inserted correctly.
                 */
                //key field or value field
                value = MappingXMLTraverser.getNodeValue((Node)anElement);
                Vector childNodes = MappingXMLTraverser.filterNodeList(anElement.getChildNodes(), Node.TEXT_NODE);
                if(childNodes.size() == 0){
                    throw new Exception(MappingXMLTraverser.produceElementTrace(anElement, "No text child nodes found. "+
                                                                            "Expected one or more (text) child nodes"));
                }
                for(int nodeCount = 0; nodeCount < childNodes.size(); nodeCount++){
                    Node childNode = (Node)childNodes.get(nodeCount);
                    if(value == null){
                        value = MappingXMLTraverser.getNodeValue(childNode);
                    } else {
                        value = value +MappingXMLTraverser.getNodeValue(childNode);
                    }
                }

                holderElement = (Element)anElement.getParentNode();
                holderElementName = holderElement.getTagName();
                //the first 'getParentNode' gets the keyDescription, second 'getParentNode' gets the topic element
                //or another element
                if(anElement.getTagName().equals(MappingXMLTraverser.keyFieldElementID)){
                    String contentValue = MappingXMLTraverser.getXMLElementAttributeValue(holderElement,
                                    MappingXMLTraverser.keyDescriptionContentAttributeID);//must always be specified
                    if(contentValue == null){
                            throw new Exception(MappingXMLTraverser.produceElementTrace(holderElement,
                                "Invalid element tree detected. Expected a '"+
                                MappingXMLTraverser.keyDescriptionElementID+"' element with the '"+
                                MappingXMLTraverser.keyDescriptionContentAttributeID+"' attribute specified."));
                    }
                    /* oid mapped objects get special treatment, we need to determine if we need to transform the field
                     * we are inserting into the struct member fields, instead of the struct itself
                     * (incase of DDS.DLRLOid). so we hacked this into this already shitty part of the code.
                     */
                    if(!contentValue.equals(MappingXMLTraverser.keyDescriptionContentAttributeNoOidID)){//ie simple or fulloid
                        int position = -1;
                        NodeList tmpKeyFields = holderElement.getElementsByTagName(MappingXMLTraverser.keyFieldElementID);
                        for(int tmpCount = 0; tmpCount < tmpKeyFields.getLength(); tmpCount++){
                            Element tmpKeyfield = (Element)tmpKeyFields.item(tmpCount);
                            if(tmpKeyfield == anElement){
                                position = tmpCount;
                            }
                        }
                        if(position == 0 && contentValue.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID)){
                            isKeyFieldOid = false;
                        } else {
                            isKeyFieldOid = true;
                        }
                    }
                    holderElement = (Element)holderElement.getParentNode();
                    holderElementName = holderElement.getTagName();
                    if(holderElementName.equals(MappingXMLTraverser.mainTopicElementID) ||
                            holderElementName.equals(MappingXMLTraverser.multiPlaceTopicElementID) ||
                            holderElementName.equals(MappingXMLTraverser.placeTopicElementID) ||
                            holderElementName.equals(MappingXMLTraverser.extensionTopicElementID) ){
                        typeSuggestion = DCPSField._KEY;
                        topicName = holderElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                        topicTypeName = holderElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                        if(topicTypeName == null)
                        {
                            topicTypeName = topicName;
                        }
                    } else if(holderElementName.equals(MappingXMLTraverser.multiRelationElementID)){
                        tempChildren = holderElement.getElementsByTagName(MappingXMLTraverser.multiPlaceTopicElementID);
                        if(tempChildren.getLength() != 1){
                            throw new Exception(MappingXMLTraverser.produceElementTrace(holderElement,
                                "Invalid element tree detected. Expected a "+
                                MappingXMLTraverser.multiPlaceTopicElementID+" as a child element."));
                        }
                        multiPlaceTopicElement = (Element)tempChildren.item(0);
                        topicName = multiPlaceTopicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                        if(multiPlaceTopicElement.hasAttribute(MappingXMLTraverser.multiPlaceTopicIndexAttributeID)){
                            typeSuggestion = DCPSField._FOREIGN_KEY;//its a map or list
                        } else {
                            typeSuggestion = DCPSField._KEY;//its a set
                        }
                        topicTypeName = multiPlaceTopicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                        if(topicTypeName == null)
                        {
                            topicTypeName = topicName;
                        }
                    } else {
                        if(!holderElementName.equals(MappingXMLTraverser.monoRelationElementID)){
                            throw new Exception(MappingXMLTraverser.produceElementTrace(holderElement,
                                "Invalid tree element detected at this location. Found '"+
                                holderElementName+"' but expected '"+MappingXMLTraverser.monoRelationElementID+"'."));
                        }
                        typeSuggestion = DCPSField._FOREIGN_KEY;
                        tempChildren = holderElement.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
                        if(tempChildren.getLength() != 0){
                            topicElement = (Element)tempChildren.item(0);//place or multiplace
                            topicName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                            topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                            if(topicTypeName == null)
                            {
                                topicTypeName = topicName;
                            }
                        } else {
                            //goto the classmapping and get the main topic element
                            tempChildren = ((Element)(holderElement.getParentNode())).getElementsByTagName(
                                                                                MappingXMLTraverser.mainTopicElementID);
                            if(tempChildren.getLength() != 1){
                                throw new Exception(MappingXMLTraverser.produceElementTrace(holderElement.getParentNode(),
                                    "Invalid element tree detected. Expected a "+
                                            MappingXMLTraverser.mainTopicElementID+" as a child element."));
                            }
                            topicElement = (Element)tempChildren.item(0);//place or multiplace
                            topicName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                            topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                            if(topicTypeName == null)
                            {
                                topicTypeName = topicName;
                            }
                        }
                    }
                } else {
                    typeSuggestion = DCPSField._NORMAL;
                    tempChildren = holderElement.getElementsByTagName(MappingXMLTraverser.multiPlaceTopicElementID);
                    if(tempChildren.getLength() == 0){//maybe a place topic can be found?
                        tempChildren = holderElement.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
                    }
                    if(tempChildren.getLength() != 0){
                        topicElement = (Element)tempChildren.item(0);//place or multiplace
                        topicName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                        topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                    }

                    if(topicName == null){
                        //fetch the main topic
                        //goto the classmapping and get the main topic element
                        tempChildren = ((Element)holderElement.getParentNode()).getElementsByTagName(
                                                                                MappingXMLTraverser.mainTopicElementID);
                        if(tempChildren.getLength() != 1){
                            throw new Exception(MappingXMLTraverser.produceElementTrace(holderElement.getParentNode(),
                                "Invalid element tree detected. Expected a "+
                                MappingXMLTraverser.mainTopicElementID+" as a child element."));
                        }
                        topicElement = (Element)tempChildren.item(0);//place or multiplace
                        topicName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
                        topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
                    }
                    if(topicTypeName == null)
                    {
                        topicTypeName = topicName;
                    }
                }
            }

            if(value != null){//aka we found a valid dcps field!
                //try and find the dcps field
                for(int vectorCount = 0; (vectorCount < dcpsFields.size()) && (containedField == null); vectorCount++){
                    aField = (DCPSField)dcpsFields.get(vectorCount);
                    if(aField.name.equals(value) && aField.topic.equals(topicName)){
                        containedField = aField;
                    }
                }

                if(containedField == null){
                    newField = new DCPSField();
                    newField.name = value;
                    newField.type = determineTopicAttributeFinalIdlType(topicTypeName, value);
                    newField.topic = topicName;
                    newField.isKeyFieldOid = isKeyFieldOid;
                    newField.keyType = typeSuggestion;
                    dcpsFields.add(newField);
                } else {
                    if(isKeyFieldOid && !containedField.isKeyFieldOid){//overwrite a false value if needed
                        containedField.isKeyFieldOid = isKeyFieldOid;
                    }
                    containedField.setNewType(typeSuggestion);
                }
            }//otherwise, dont do anything
        }

    }

    public String determineTopicAttributeFinalIdlType(
        String fullyQualifiedTopicName,
        String attributeName) throws Exception
    {
        IdlObject object = null;
        Object attribute;
        int type;
        String retVal = IDLTraverser.UNKNOWN_TYPE_TXT;

        try
        {
            object = IDLTraverser.getIdlObjectWithFullyQualifiedName(
                rootDCPSIDL,
                IDLTraverser.getAllIdlTypes(),
                fullyQualifiedTopicName);
        }
        catch (Exception e)
        {
        }
        if(object != null)
        {
            attribute = IDLTraverser.getIdlObjectAttribute(
                rootDCPSIDL,
                object,
                attributeName);
            if(attribute instanceof IdlObject)
            {
                attribute = IDLTraverser.resolveTypedefAndIdlIdent((IdlObject)attribute);
            }
            if(attribute instanceof String)
            {
                return IDLTraverser.ENUM_TXT;
            } else
            {
                type = IDLTraverser.getAttributeType((IdlObject)attribute);
                retVal = IDLTraverser.attributeTypeToString(type);
            }
        }
        return retVal;
    }

    private class DCPSField {
        public static final int _NORMAL = 0;
        public static final int _SHARED_KEY = 1;
        public static final int _KEY = 2;
        public static final int _FOREIGN_KEY = 3;

        public String name = null;
        public String type = "UNKNOWN";
        public String topic = null;
        public int keyType = _NORMAL;
        public boolean immutable = false;
        public boolean isKeyFieldOid = false;//indicates if this dcps field is DDS::DLRLOid and was found in a keyfield. not the nicest addition, but i wont tell if you wont

        public DCPSField(){}

        public boolean equals(Object o) {
            if(((DCPSField)o).name.equals(this.name) && ((DCPSField)o).topic.equals(this.topic)){
                return true;
            }
            return false;
        }

        public void setNewType(int typeSuggestion){
            //ignore typeSuggestion == _NORMAL, this never results in a change (as normal is default and weaker then all other types)
            //another with a shared key results in the keyType being shared key
            //key and foreign key combination results in a shared key
            //new suggestion of key and foreign key are always accepted (assuming the above two options are always done first and exclude this one)
            if(typeSuggestion == _SHARED_KEY || keyType == _SHARED_KEY){
                keyType = _SHARED_KEY;
            } else if(((typeSuggestion == _FOREIGN_KEY) && (keyType == _KEY)) || ((keyType == _FOREIGN_KEY) && (typeSuggestion == _KEY))){
                keyType = _SHARED_KEY;
            } else if((typeSuggestion == _KEY) || (typeSuggestion == _FOREIGN_KEY)){
                keyType = typeSuggestion;
            }
        }

        public String getDCPSFieldKeyType(int keyType){
            switch (keyType){
                case _SHARED_KEY:
                    return "_SHARED_KEY";
                case _KEY:
                    return "_KEY";
                case _FOREIGN_KEY:
                    return "_FOREIGN_KEY";
                case _NORMAL:
                    return "_NORMAL";
                default:
                    return  "???";
            }
        }

    };

    /**
     * This method is responsible for processing an IDL forward valuetype and the
     * Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT VALUEFORWARDDEF EMPTY>
     * <!ATTLIST VALUEFORWARDDEF
     * NAME     CDATA       #REQUIRED
     * ABSTRACT (true|false)    #REQUIRED
     * pattern      (List | StrMap | IntMap | Set)  #REQUIRED
     * itemType                 CDATA       #REQUIRED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF32130152
     */
    public void processIDLValueForwardDef(IdlValue object) throws Exception {
        // suppress any valueforwarddefs from DDS
        if (isFromDDS(object))
            return;

        String objectNameQualified = IDLTraverser.getIDLObjectFullyqualifiedName(object);
        Element templateDefElement = MappingXMLTraverser.getXMLElementWithAttributeValue(rootXML.getDocumentElement(),
                                                                        MappingXMLTraverser.templateDefElementID,
                                                                        MappingXMLTraverser.templateDefNameAttributeID,
                                                                        objectNameQualified);
        boolean templateDefErrorFound = true;
        if(templateDefElement !=null && templateDefElement.hasAttributes()){
            NamedNodeMap attributes = templateDefElement.getAttributes();
            Node patternNode = attributes.getNamedItem(MappingXMLTraverser.templateDefPatternAttributeID    );
            Node itemTypeNode = attributes.getNamedItem(MappingXMLTraverser.templateDefItemTypeAttributeID      );
            if(patternNode != null && itemTypeNode != null){
                String pattern = MappingXMLTraverser.getNodeValue(patternNode);
                String itemType = MappingXMLTraverser.getNodeValue(itemTypeNode);
                if(pattern != null && itemType != null){
                    write("<VALUEFORWARDDEF NAME=\""+ objectNameQualified+"\" ABSTRACT=\""+object.abstract_value()+"\" pattern=\""+pattern+"\" itemType=\""+itemType+"\" fromIncludedIdl=\""+object._map+"\"/>");
                    templateDefErrorFound = false;
                }
            }
        } else if (null != IDLTraverser.getIdlNonForwardValueWithFullyQualifiedName(rootIDL, objectNameQualified)) {
            templateDefErrorFound = false;
        } else {
            templateDefErrorFound = false; // TODO ID:194. Can be removed once IDL has been validated for XML.
        }
        if(templateDefErrorFound){
            throw new Exception("No template def found in XML mapping for forward valuetype '"+objectNameQualified+"'");
        }
    }

    /**
     * This method is responsible for processing an IDL valuetype inheritence and the
     * Mapping XML needed for this object (if any). The output is written to the
     * output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT VALUEINHERITS EMPTY>
     * <!ATTLIST VALUEINHERITS
     * NAME     CDATA       #REQUIRED>
     *
     * @param inheritanceList The IdlValue array containing the valuetypes that are
     * apart of the inheritancelist
     * @throws Exception
     * @roseuid 40BF321402C9
     */
    public void processIDLValueInheritance(IdlValue[] inheritanceList) throws Exception {
        for(int count = 0; count < inheritanceList.length; count++){
            IdlValue parentValue = (IdlValue)inheritanceList[count];
            write("<VALUEINHERITS NAME=\""+IDLTraverser.getIDLObjectFullyqualifiedName(parentValue)+"\"/>");
        }
    }

    /**
     * This method is responsible for processing an IDL WString and the Mapping XML
     * needed for this object (if any). The output is written to the output stream by
     * calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT WSTRING EMPTY>
     * <!ATTLIST WSTRING
     * LENGTH       CDATA   #IMPLIED>
     *
     * @param object The source IDL object containing the data for output to the
     * merged XML file
     * @throws Exception
     * @roseuid 40BF3216001A
     */
    public void processIDLWString(IdlWString object) throws Exception {
        write("<WSTRING LENGTH=\"" +object.max()+"\"/ fromIncludedIdl=\""+object._map+"\">");
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param associationNode
     * @throws Exception
     * @roseuid 40DC2C15024F
     */
    public void processXMLMappingAssociationDefElement(Element associationNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param classMappingNode
     * @throws Exception
     * @roseuid 40DC2C0C00D7
     */
    public void processXMLMappingClassMappingElement(Element classMappingNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param compoRelationNode
     * @throws Exception
     * @roseuid 40DC2C17024F
     */
    public void processXMLMappingCompoRelationDefElement(Element compoRelationNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param enumDefNode
     * @throws Exception
     * @roseuid 40DC2C1901C2
     */
    public void processXMLMappingEnumDefElement(Element enumDefNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param extensionTopicNode
     * @throws Exception
     * @roseuid 40DC2C0A02CB
     */
    public void processXMLMappingExtensionTopicElement(Element extensionTopicNode) throws Exception {
        // processXMLTopic(extensionTopicNode, MappingXMLTraverser.extensionTopicElementID);
    }

    /**
     * This method will generate an XML description element, however it will not check
     * if the correct number of keyField child elements are specified. It will simply
     * process any keyField child elements found. An incorrect number of keyfields
     * should not occur because the XML input should then havent been succesfully
     * validated by the MappingXMLContentValidator.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT keyDescription (keyField*)>
     * <!ATTLIST keyDescription
     * content (FullOid | SimpleOid | NoOid)    #REQUIRED>
     *
     * @param keyDescriptionNode
     * @throws Exception
     * @roseuid 40BF321802D9
     */
    public void processXMLMappingKeyDescriptionElement(Element keyDescriptionNode) throws Exception {
        String contentValue = MappingXMLTraverser.getXMLElementAttributeValue(keyDescriptionNode,
                                    MappingXMLTraverser.keyDescriptionContentAttributeID);//must always be specified
        if(contentValue==null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)keyDescriptionNode, false,
              "No value provided for the required "+MappingXMLTraverser.keyDescriptionContentAttributeID+" attribute."));
        }
        Vector keyFields = MappingXMLTraverser.getXMLDirectChildElements(keyDescriptionNode,
                                                                                MappingXMLTraverser.keyFieldElementID);
        if(keyFields.size() == 0){
            write("<"+MappingXMLTraverser.keyDescriptionElementID+" "+
                            MappingXMLTraverser.keyDescriptionContentAttributeID+"=\""+contentValue+"\"/>");
        } else {
            write("<"+MappingXMLTraverser.keyDescriptionElementID+" "+
                                        MappingXMLTraverser.keyDescriptionContentAttributeID+"=\""+contentValue+"\">");
            indent++;
            for(int count = 0; count < keyFields.size(); count++){
                Element aKeyfield = (Element)keyFields.get(count);
                String keyFieldName = MappingXMLTraverser.getKeyFieldName(aKeyfield);
                if( (contentValue.equals(MappingXMLTraverser.keyDescriptionContentAttributeSimpleOidID) && count == 0) ||
                    (contentValue.equals(MappingXMLTraverser.keyDescriptionContentAttributeFullOidID) && count == 1)){
                    write("<"+MappingXMLTraverser.keyFieldElementID+">");
                    indent++;
                    write(keyFieldName+".systemId");
                    indent--;
                    write("</"+MappingXMLTraverser.keyFieldElementID+">");
                    write("<"+MappingXMLTraverser.keyFieldElementID+">");
                    indent++;
                    write(keyFieldName+".localId");
                    indent--;
                    write("</"+MappingXMLTraverser.keyFieldElementID+">");
                    write("<"+MappingXMLTraverser.keyFieldElementID+">");
                    indent++;
                    write(keyFieldName+".serial");
                    indent--;
                    write("</"+MappingXMLTraverser.keyFieldElementID+">");
                } else {
                    write("<"+MappingXMLTraverser.keyFieldElementID+">");
                    indent++;
                    write(keyFieldName);
                    indent--;
                    write("</"+MappingXMLTraverser.keyFieldElementID+">");
                }
            }
            indent--;
            write("</"+MappingXMLTraverser.keyDescriptionElementID+">");
        }
    }

    public void processXMLMappingValidityFieldElement(Element node) throws Exception{
        String name = MappingXMLTraverser.getXMLElementAttributeValue(node,
                                    MappingXMLTraverser.validityFieldNameAttributeID);//must always be specified
        if(name == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace((Node)node, false,
              "No value provided for the required "+MappingXMLTraverser.validityFieldNameAttributeID+" attribute."));
        }
        TopicInfoHolder holder = getValidityFieldTopicInfo(node, name);
        String topicTypeName = holder.topicNameFQ;
        int type = IDLTraverser.BOOLEAN;
        if(!holder.isDefault){
            type = getStructMemberType(topicTypeName, name);
        }
        String validValue = getValidityFieldValidValue(node, type);
        String invalidValue = getValidityFieldInvalidValue(node, type);
        write("<"+MappingXMLTraverser.validityFieldElementID+" "+MappingXMLTraverser.validityFieldNameAttributeID+"=\""+name+
            "\" "+MappingXMLTraverser.validityFieldValidAttributeID+"=\""+validValue+"\" "+
                MappingXMLTraverser.validityFieldInvalidAttributeID+"=\""+invalidValue+"\"/>");
    }

    /* only limited subset supported so far, throws exception if it doesnt match that subset */
    private int getStructMemberType(String topicNameFQ, String attributeName) throws Exception{
        int typeID = -1;
        int[] types = new int[1];
        types[0] = idlType.STRUCT;
        IdlObject object = IDLTraverser.getIdlObjectWithFullyQualifiedName(rootDCPSIDL, types, topicNameFQ);
        if(object == null){
            throw new Exception("No IDL Struct with name '"+topicNameFQ+"' found in the DCPS IDL.");
        }
        Object attribute = IDLTraverser.getIdlObjectAttribute(rootDCPSIDL, object, attributeName);
        if(attribute == null){
            throw new Exception("Failed to locate attribute '"+attributeName+"' in IDL Struct '"+topicNameFQ+"'.");
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
        if(attribute instanceof IdlSimple){
            int type = ((IdlSimple)attribute).primitive();
            switch (type){
                case IdlSimple.BOOLEAN:
                    typeID = IDLTraverser.BOOLEAN;
                    break;
                case IdlSimple.DOUBLE :
                    typeID = IDLTraverser.DOUBLE;
                    break;
                case IdlSimple.FLOAT :
                    typeID = IDLTraverser.FLOAT;
                    break;
                case IdlSimple.LONG :
                    typeID = IDLTraverser.LONG;
                    break;
                case IdlSimple.LONGLONG :
                    typeID = IDLTraverser.LONGLONG;
                    break;
                case IdlSimple.OCTET :
                    typeID = IDLTraverser.OCTET;
                    break;
                case IdlSimple.SHORT :
                    typeID = IDLTraverser.SHORT;
                    break;
                case IdlSimple.ULONG :
                    typeID = IDLTraverser.ULONG;
                    break;
                case IdlSimple.ULONGLONG :
                    typeID = IDLTraverser.ULONGLONG;
                    break;
                case IdlSimple.USHORT :
                    typeID = IDLTraverser.USHORT;
                    break;
                default:
                    throw new Exception("Attribute '"+attributeName+"' in IDL Struct '"+topicNameFQ+"' is of an illegal simple type "+
                    "(allowed types: boolean, double, float, long, long long, octet, short, unsigned long, unsigned long long, unsigned short).");
            }
        }
        if(typeID == -1){
            throw new Exception("Attribute '"+attributeName+"' in IDL Struct '"+topicNameFQ+"' is of an unsupported type ");
        }
        return typeID;
    }

    private class TopicInfoHolder{
        private String topicNameFQ;
        private boolean isDefault;
    }

    private TopicInfoHolder getValidityFieldTopicInfo(Element node, String name) throws Exception{
        String topicTypeName = null;
        Element topicElement;
        Element holderElement;
        NodeList tempChildren;
        TopicInfoHolder holder = new TopicInfoHolder();

        holderElement = (Element)node.getParentNode();
        //validity field can only be defined in a place- or main topic
        tempChildren = holderElement.getElementsByTagName(MappingXMLTraverser.placeTopicElementID);
        if(tempChildren.getLength() != 0){//i.e. we found a place topic
            topicElement = (Element)tempChildren.item(0);
            topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
            if(topicTypeName == null){
                topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
            }
            if(topicTypeName == null){
                 throw new Exception(MappingXMLTraverser.produceElementTrace(topicElement, false,
                     "Missing required attributes."));
            }
        }
        if(topicTypeName == null){
            //fetch the main topic
            //goto the classmapping and get the main topic element
            tempChildren = ((Element)holderElement.getParentNode()).getElementsByTagName(
                                                                    MappingXMLTraverser.mainTopicElementID);
            if(tempChildren.getLength() != 1){
                throw new Exception("Invalid element tree detected. Expected a "+
                    MappingXMLTraverser.mainTopicElementID+" as a child element of element "+
                                                ((Element)(holderElement.getParentNode())).getTagName());
            }
            topicElement = (Element)tempChildren.item(0);
            topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementTypeNameAttributeID);
            if(topicTypeName == null){
                topicTypeName = topicElement.getAttribute(MappingXMLTraverser.topicElementNameAttributeID);
            }
            if(topicTypeName == null){
                 throw new Exception(MappingXMLTraverser.produceElementTrace(topicElement, false,
                     "Missing required attributes."));
            }
            holder.isDefault = MappingXMLTraverser.isElementDefault(topicElement);
            holder.topicNameFQ = topicTypeName;
        }
        return holder;
    }


    private String getValidityFieldValidValue(Element node, int type) throws Exception{
        switch (type){
            case IDLTraverser.BOOLEAN:
                return "true";
            case IDLTraverser.LONG:
            case IDLTraverser.LONGLONG:
            case IDLTraverser.SHORT:
            case IDLTraverser.OCTET:
            case IDLTraverser.ULONG:
            case IDLTraverser.ULONGLONG:
            case IDLTraverser.USHORT:
                return "1";
            default:
                throw new Exception(MappingXMLTraverser.produceElementTrace(node, false,
                     "This attribute is defined with an invalid type in the DCPS IDL."));
        }
    }

    private String getValidityFieldInvalidValue(Element node, int type) throws Exception{
        switch (type){
            case IDLTraverser.BOOLEAN:
                return "false";
            case IDLTraverser.LONG:
            case IDLTraverser.LONGLONG:
            case IDLTraverser.SHORT:
            case IDLTraverser.OCTET:
            case IDLTraverser.ULONG:
            case IDLTraverser.ULONGLONG:
            case IDLTraverser.USHORT:
                return "0";
            default:
                throw new Exception(MappingXMLTraverser.produceElementTrace(node, false,
                     "This attribute is defined with an invalid type in the DCPS IDL."));
        }
    }




    /**
     * This method is responsible for processing a keyfield element. The output is
     * written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method(excluding
     * the child element):
     * <!ELEMENT keyField  (#PCDATA)>
     *
     * @param keyFieldNode
     * @throws Exception
     * @roseuid 40BF321A0172
     */
    public void processXMLMappingKeyFieldElement(Element keyFieldNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is responsible for processing a keyfield text child element. The
     * output is written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method(onlythe
     * child element):
     * <!ELEMENT keyField  (#PCDATA)>
     *
     * @param keyFieldNode The text node to be processed
     * @throws Exception
     * @roseuid 40DC28B303DA
     */
    public void processXMLMappingKeyFieldTextElement(Node keyFieldNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is responsible for processing a local element. The output is
     * written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT local EMPTY>
     * @param localNode
     * @throws Exception
     * @roseuid 40BF321F02D9
     */
    public void processXMLMappingLocalElement(Element localNode) throws Exception {
        write("<local/>");
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param mainTopicNode
     * @throws Exception
     * @roseuid 40DC2C06021F
     */
    public void processXMLMappingMainTopicElement(Element mainTopicNode) throws Exception {
        //processXMLTopic(mainTopicNode, MappingXMLTraverser.mainTopicElementID);
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param monoAttributeNode
     * @throws Exception
     * @roseuid 40DC2C0E01A2
     */
    public void processXMLMappingMonoAttributeElement(Element monoAttributeNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param monoRelationNode
     * @throws Exception
     * @roseuid 40DC2C100125
     */
    public void processXMLMappingMonoRelationElement(Element monoRelationNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param multiAttributeNode
     * @throws Exception
     * @roseuid 40DC2C1200B8
     */
    public void processXMLMappingMultiAttributeElement(Element multiAttributeNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method calls the processXMLTopic method, see documentation of that method.
     * @param multiPlaceTopicNode
     * @throws Exception
     * @roseuid 40DC2B9A019F
     */
    public void processXMLMappingMultiPlaceTopicElement(Element multiPlaceTopicNode) throws Exception {
         processXMLTopic(multiPlaceTopicNode, MappingXMLTraverser.multiPlaceTopicElementID);
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param multiRelationNode
     * @throws Exception
     * @roseuid 40DC2C130397
     */
    public void processXMLMappingMultiRelationElement(Element multiRelationNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method calls the processXMLTopic method, see documentation of that method.
     * @param placeTopicNode
     * @throws Exception
     * @roseuid 40DC2C0801C1
     */
    public void processXMLMappingPlaceTopicElement(Element placeTopicNode) throws Exception {
         processXMLTopic(placeTopicNode, MappingXMLTraverser.placeTopicElementID);
    }

    /**
     * This method is responsible for processing a association element. The output is
     * written to the output stream by calling the write(..) method. if the class and
     * attribute of both relation elements is the same then they are ignored.
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT associationDef (relation,relation)>
     *
     * <!ELEMENT relation EMPTY>
     * <!ATTLIST relation
     * class    CDATA   #REQUIRED
     * attribute    CDATA   #REQUIRED>
     *
     * @param objectName The name of the class part of the relation
     * @param attributeName The name of the attribute part of the relation
     * @throws Exception
     * @roseuid 40BF322602E9
     */
    private void processXMLMappingRelation(String objectName, String attributeName) throws Exception {
        Vector validAssociationDefNodes = new Vector();
        NodeList associationDefElements = MappingXMLTraverser.getXMLAllChildElements(rootXML.getDocumentElement(), MappingXMLTraverser.associationDefElementID);
        for(int count = 0; count < associationDefElements.getLength(); count++){
            Node aNode = associationDefElements.item(count);
            boolean relationFound = false;
            if(aNode.getNodeType() == Node.ELEMENT_NODE){
                Vector relationNodes = MappingXMLTraverser.getXMLDirectChildElements((Element)aNode, MappingXMLTraverser.relationElementID);
                for(int relationCount = 0; relationCount < relationNodes.size(); relationCount++){
                    Node relationNode = (Node)relationNodes.get(relationCount);
                    if(!relationNode.hasAttributes()){
                        throw new Exception(MappingXMLTraverser.produceElementTrace(relationNode, "Invalid relation node specified "+
                                    MappingXMLTraverser.relationElementAttributeAttributeID+" and "+
                                    MappingXMLTraverser.relationElementClassAttributeID+" attributes must be specified!"));
                    }
                    NamedNodeMap attributes = relationNode.getAttributes();
                    Node attributeNode = attributes.getNamedItem(MappingXMLTraverser.relationElementAttributeAttributeID);
                    Node classNode = attributes.getNamedItem(MappingXMLTraverser.relationElementClassAttributeID);
                    if(attributeNode == null){
                        throw new Exception(MappingXMLTraverser.produceElementTrace(relationNode, true, "Invalid relation node specified. The attribute with name "
                            +MappingXMLTraverser.relationElementAttributeAttributeID+" must be specified."));
                    }
                    if(classNode == null){
                        throw new Exception(MappingXMLTraverser.produceElementTrace(relationNode, true, "Invalid relation node specified. The attribute with name "
                            +MappingXMLTraverser.relationElementClassAttributeID+" must be specified."));
                    }
                    String currentAttributeValue = MappingXMLTraverser.getNodeValue(attributeNode);
                    String classValue = MappingXMLTraverser.getNodeValue(classNode);
                    if(classValue!=null && classValue.equals(objectName) && currentAttributeValue!=null && currentAttributeValue.equals(attributeName)){
                        relationFound = true;
                    }
                }
                if(relationFound){
                    for(int processCounter = 0; processCounter < relationNodes.size(); processCounter++){
                        Node relationNode = (Node)relationNodes.get(processCounter);
                        NamedNodeMap attributes = relationNode.getAttributes();
                        Node attributeNode = attributes.getNamedItem(MappingXMLTraverser.relationElementAttributeAttributeID    );
                        Node classNode = attributes.getNamedItem(MappingXMLTraverser.relationElementClassAttributeID        );
                        if(attributeNode != null && classNode != null){
                            String currentAttributeValue    = MappingXMLTraverser.getNodeValue(attributeNode);
                            String classValue               = MappingXMLTraverser.getNodeValue(classNode);
                            if(!(classValue.equals(objectName) && currentAttributeValue.equals(attributeName))){
                                write("<"+MappingXMLTraverser.relationElementID+" "+MappingXMLTraverser.relationElementClassAttributeID+"=\""+classValue+"\" "+MappingXMLTraverser.relationElementAttributeAttributeID+"=\""+currentAttributeValue+"\"/>");
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param relationNode
     * @throws Exception
     * @roseuid 40DC2C1C01C2
     */
    public void processXMLMappingRelationElement(Element relationNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is not implemented. The contents represented by this Mapping XML
     * element is integrated into other elements generated by this class. To find out
     * into which elements this element is integrated into see the IDL and Mapping XML
     * to XML describtion appendix.
     * @param templateDefNode
     * @throws Exception
     * @roseuid 40DC2C1A0397
     */
    public void processXMLMappingTemplateDefElement(Element templateDefNode) throws Exception {
        if(model.getVerbose()){
            System.out.println("Not implemented");
        }
    }

    /**
     * This method is responsible for processing a value element. The output is
     * written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method (excluding
     * the child element):
     * <!ELEMENT value (#PCDATA)>
     *
     * @param sourceValueElement The source XML element containing the source data
     * @throws Exception
     * @roseuid 40BF322102CA
     */
    public void processXMLMappingValueElement(Element sourceValueElement) throws Exception {
        write("<"+MappingXMLTraverser.valueElementID+">");
        indent++;
        xmlTraverser.traverseXMLMappingValueElement(sourceValueElement);
        indent--;
        write("</"+MappingXMLTraverser.valueElementID+">");
    }

    /**
     * This method is responsible for processing a value field element. The output is
     * written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method(excluding
     * the child element):
     * <!ELEMENT valueField  (#PCDATA)>
     *
     * @param sourceValueFieldElement The source XML element containing the source data
     * @param valueFieldNode
     * @throws Exception
     * @roseuid 40BF321D0039
     */
    public void processXMLMappingValueFieldElement(Element valueFieldNode) throws Exception {
        write("<"+MappingXMLTraverser.valueFieldElementID+">");
        indent++;
        xmlTraverser.traverseXMLMappingValueFieldElement(valueFieldNode);
        indent--;
        write("</"+MappingXMLTraverser.valueFieldElementID+">");
    }

    /**
     * This method is responsible for processing a value field text child element. The
     * output is written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method(only the
     * child element):
     * <!ELEMENT valueField  (#PCDATA)>
     *
     * @param valueFieldNode The text node to be processed
     * @throws Exception
     * @roseuid 40DC2B100071
     */
    public void processXMLMappingValueFieldTextElement(Node valueFieldNode) throws Exception {
        if(MappingXMLTraverser.getNodeValue(valueFieldNode) == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(valueFieldNode, true,
                                                                        "Invalid node value for text child node."));
        }
        write(MappingXMLTraverser.getNodeValue(valueFieldNode));

    }

    /**
     * This method is responsible for processing a value text child element. The
     * output is written to the output stream by calling the write(..) method.
     *
     * The following part of the merged XML DTD is handled by this method(only the
     * child element):
     * <!ELEMENT value (#PCDATA)>
     *
     * @param valueTextNode The text node to be processed
     * @throws Exception
     * @roseuid 40DC2A7F0356
     */
    public void processXMLMappingValueTextElement(Node valueTextNode) throws Exception {
        if(MappingXMLTraverser.getNodeValue(valueTextNode) == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(valueTextNode, true,
                                                                        "Invalid node value for text child node."));
        }
        write(MappingXMLTraverser.getNodeValue(valueTextNode));
    }

    /**
     * This method process an XML topic. It will also work correctly if multiple or no
     * keydescription child elements are specified. There is no check to see if the
     * indexField attribute is correctly specified. For instance of the indexField
     * attribute is required for the multiPlaceTopic element, but it isnt found then
     * NO error is given. However if the name attribute is missing then an error is
     * given as the name attribute is always required. Also note that if the
     * indexField is missing, then the input should never have been validated and been
     * provided as input for the merged XML generator(unless we are dealing with a set,
     * then the index field may not be present
     *
     * The following part of the merged XML DTD is handled by this method:
     * <!ELEMENT ANY_TOPIC_ID_NAME (keyDescription)>
     * <!ATTLIST ANY_TOPIC_ID_NAME
     * name     CDATA   #REQUIRED
     * indexField   CDATA   #IMPLIED>
     *
     * Will work for: mainTopic, extensionTopic, placeTopic
     * <!ELEMENT placeTopic (keyDescription)>
     * <!ATTLIST placeTopic
     * name     CDATA   #REQUIRED>
     *
     * <!ELEMENT extensionTopic (keyDescription)>
     * <!ATTLIST extensionTopic
     * name     CDATA   #REQUIRED>
     *
     * <!ELEMENT mainTopic (keyDescription)>
     * <!ATTLIST mainTopic
     * name     CDATA   #REQUIRED>
     *
     * <!ELEMENT multiPlaceTopic (keyDescription)>
     * <!ATTLIST multiPlaceTopic
     * name     CDATA   #REQUIRED
     * indexField    CDATA  #IMPLIED>
     *
     * @param sourceTopicElement The source XML element used to generate the correct
     * topic ouput.
     * @param topicID The string identifying the specific topic name to be ouput. The
     * string content is not validated and may not be NULL.
     * @throws Exception
     * @roseuid 40BF32170191
     */
    private void processXMLTopic(Element sourceTopicElement, String topicID) throws Exception {
        //get main topic element attribute 'name' value
        String nameValue = MappingXMLTraverser.getXMLElementAttributeValue(sourceTopicElement, MappingXMLTraverser.topicElementNameAttributeID);            //must always be specified
        String typeNameValue = MappingXMLTraverser.getXMLElementAttributeValue(sourceTopicElement, MappingXMLTraverser.topicElementTypeNameAttributeID);//doesnt have to be specified
        if(typeNameValue == null){
            typeNameValue = nameValue;
        }
        if(nameValue == null){
            throw new Exception(MappingXMLTraverser.produceElementTrace(sourceTopicElement, "No value provided for the required "+MappingXMLTraverser.topicElementNameAttributeID+" attribute."));
        }
        String indexFieldValue = MappingXMLTraverser.getXMLElementAttributeValue(sourceTopicElement, MappingXMLTraverser.topicElementIndexFieldAttributeID);//doesnt have to be specified
        Vector keyDescriptions = MappingXMLTraverser.getXMLDirectChildElements(sourceTopicElement, MappingXMLTraverser.keyDescriptionElementID);

        if(keyDescriptions.size() == 0){
            if(indexFieldValue == null){
                write("<"+topicID+" "+MappingXMLTraverser.topicElementNameAttributeID+"=\""+nameValue+"\" "+MappingXMLTraverser.topicElementTypeNameAttributeID+"=\""+typeNameValue+"\"/>");
            } else {
                write("<"+topicID+" "+MappingXMLTraverser.topicElementNameAttributeID+"=\""+nameValue+"\" "+MappingXMLTraverser.topicElementTypeNameAttributeID+"=\""+typeNameValue+"\" "+MappingXMLTraverser.topicElementIndexFieldAttributeID+"=\""+indexFieldValue+"\"/>");
            }
        } else {
            if(indexFieldValue == null){
                write("<"+topicID+" "+MappingXMLTraverser.topicElementNameAttributeID+"=\""+nameValue+"\" "+MappingXMLTraverser.topicElementTypeNameAttributeID+"=\""+typeNameValue+"\">");
            } else if(indexFieldValue !=null){
                write("<"+topicID+" "+MappingXMLTraverser.topicElementNameAttributeID+"=\""+nameValue+"\" "+MappingXMLTraverser.topicElementTypeNameAttributeID+"=\""+typeNameValue+"\" "+MappingXMLTraverser.topicElementIndexFieldAttributeID+"=\""+indexFieldValue+"\">");
            }
            indent++;
            for(int count = 0; count < keyDescriptions.size(); count++){
                Element keyDescriptionElement = (Element)keyDescriptions.get(count);
                processXMLMappingKeyDescriptionElement(keyDescriptionElement);
            }
            indent--;
            write("</"+topicID+">");
        }
    }

    private String[] getTopicModuleName(String topicNameWithModuleDef){
        StringTokenizer topicTokenizer = new StringTokenizer(topicNameWithModuleDef, "::");
        int tokenCount = topicTokenizer.countTokens();
        String tokens[] = new String[tokenCount];
        for(int count = 0; count < tokenCount; count++){
            String token = topicTokenizer.nextToken();
            tokens[count] = token;
        }
        return tokens;
    }

    /**
     * Writes the string as ouput to the ouputstream. Ensures the file is "pretty
     * printed" (it takes tabs into account by counting the required indent).
     *
     * @param text
     * @throws Exception
     * @roseuid 40BF3238000C
     */
    private void write(String text) throws Exception {
        //for(int count = 0; count < indent; count++){
       //   out.write('\t');
       // }
      out.write(text.toString());
      //  out.newLine();
    }
}