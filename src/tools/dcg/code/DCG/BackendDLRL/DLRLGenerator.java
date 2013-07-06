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

import DCG.Core.Generator;
import DCG.Core.MainModel;
import DCG.Core.AbstractSyntaxTreeHolder;
import java.util.HashMap;
import java.util.HashSet;
import org.w3c.dom.Document;
import java.util.Vector;
import java.io.File;
import java.util.Iterator;
import java.io.FileOutputStream;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import DCG.DCGUtilities.MappingXMLTraverser;
import DCG.DCGUtilities.StreamHandlerUtil;

/**
 * This class is the entry point into the BackendDLRL for outside classes. It acts
 * as a manager for the tasks that need to be executed within the Backend DLRL.
 * The steps taken by the DLRLGenerator class are further specified in the
 * executeGeneration method.
 */
public class DLRLGenerator extends Generator {

    /**
     * Field identifying the DCPS IDL file. This must be set at creation.
     */
    private String dcpsIdlFileId;

    /**
     * Field identifying the DLRL IDL file. This must be set at creation.
     */
    private String dlrlIdlFileId;

    /**
     * Field identifying the Mapping XML file. This must be set at creation.
     */
    private String mappingXmlFileId;

    private int oidType;

    private boolean isDefaultMappingUsed = false;

    public static final int FULL_OID = 0;
    public static final int SIMPLE_OID = 1;

    public static final int JAVA_LANG = 0;
    public static final int SACPP_LANG = 1;
    private int language = -1;

    private String macroName = "";
    private String macroHeaderFileName = "";

    /**
     * Specialized constructor
     *
     * @param model The main model that manages this generator
     * @param sourceFileIds A HashMap containing the source files identifiers
     * (Strings) and a value set to true if the file is required(Boolean)
     * @param templateGroupIds A HashSet containing the identifiers of the template
     * groups (String objects)
     * @param dlrlIdllFileId The file identifier for the DLRL IDL source file
     * @param mappingXmlFileId The file identifier for the Mapping XML source file
     * @param dcpsIdlFileId The file identifier for the DCPS IDL source file
     * @roseuid 408CC69C03C5
     */
    public DLRLGenerator(
        MainModel model,
        HashMap sourceFileIds,
        HashSet templateGroupIds,
        String dlrlIdlFileId,
        String mappingXmlFileId,
        String dcpsIdlFileId,
        int oidType,
        int language,
        String macroName,
        String macroHeaderFileName) throws Exception
    {
        super(model, sourceFileIds, templateGroupIds);
        if(model.getVerbose())
        {
            System.out.println("- Notification: DLRL generator is IDL version 2.4.2 compliant.");
            System.out.println("- Notification: DLRL generator is XML version 1.0 compliant.");
        }
        if(language != JAVA_LANG && language != SACPP_LANG)
        {
            throw new Exception("Error detected. This backend does not support the language you requested!");
        }
        this.dlrlIdlFileId = dlrlIdlFileId;
        this.mappingXmlFileId = mappingXmlFileId;
        this.dcpsIdlFileId = dcpsIdlFileId;
        this.oidType = oidType;
        this.language = language;
        this.macroName = macroName;
        this.macroHeaderFileName = macroHeaderFileName;
        DCG.DCGUtilities.IDLTraverser.verbose = model.getVerbose();
    }

    public boolean isDefaultMappingUsed(
        )
    {
        return isDefaultMappingUsed;
    }

    /**
     * Takes care of the execution process by following the following steps
     *
     * 1) It will check if the required source files (DLRL IDL and Mapping XML) are
     * present
     * 2) It will start validation of the Mapping XML file with the available source
     * files. (by calling an internal method)
     * 3) If no DCPS IDL file is found, it will initiate the topic model generator and
     * DCPS IDL file generator
     * 4) it will initiate the Merged XML file generator
     * 5) it will initiate the External XSLT Engine (by calling an internal method)
     *
     * @param abstractSyntaxTrees A HashMap containing the syntax trees. Key: The
     * source file ID to identify the syntax tree with a source file.
     * Value The syntax tree itself.
     * @param templateGroups A hashMap containing all groups of template files.
     * Key: Idenitifer for the template groups
     * Value: A template file (java.io.File)
     * @throws Exception
     * @roseuid 408CBC660111
     */
    public void executeGeneration(HashMap abstractSyntaxTrees, HashMap templateGroups) throws Exception {
        boolean fullOidDefault = false;
        Document mappingXML;
        Vector dlrlIDL;
        Vector dcpsIDL = null;
        File mergedXML = null;
        DCPSIDLGenerator2 dcpsIDLGenerator;
        DLRLXMLGenerator dlrlXMLGenerator = null;
        MappingXMLContentValidator contentValidator;
        TopicModelGenerator topicModelGenerator;
        OrbFileConverter orbFileConverter = null;
        File dlrlIdlFile;
        File dcpsIdlFile;
        int index;
        String baseName;
        File xsltIdlFile;

        /* Get the required input information and validate that it's present */
        mappingXML = (Document)((AbstractSyntaxTreeHolder)abstractSyntaxTrees.get(mappingXmlFileId)).getRoot();
        dlrlIDL = (Vector)((AbstractSyntaxTreeHolder)abstractSyntaxTrees.get(dlrlIdlFileId)).getRoot();
        dlrlIdlFile = getModel().getFileForSourceFileID(dlrlIdlFileId);
        dcpsIdlFile = getModel().getFileForSourceFileID(dcpsIdlFileId);
        /* dcps idl may be null..*/
        if(abstractSyntaxTrees.get(dcpsIdlFileId) != null){
            dcpsIDL = (Vector)((AbstractSyntaxTreeHolder)abstractSyntaxTrees.get(dcpsIdlFileId)).getRoot();
        }

        if(mappingXML == null){
            throw new Exception("Missing the required mapping XML file, aborting...");
        }
        if(dlrlIDL == null){
            throw new Exception("Missing the required DLRL IDL file, aborting...");
        }
        /* create all needed sub generators and validators */
        dcpsIDLGenerator = new DCPSIDLGenerator2(
            mappingXML,
            dlrlIDL,
            dlrlIdlFile,
            getModel());
        dlrlXMLGenerator = new DLRLXMLGenerator(
            mappingXML,
            dlrlIDL,
            dcpsIDL,
            getModel());
        contentValidator = new MappingXMLContentValidator(
            mappingXML,
            dlrlIDL,
            dcpsIDL,
            getModel().getVerbose());
        if(language == SACPP_LANG)
        {
            orbFileConverter = new OrbFileConverter(
                getModel(),
                mappingXML,
                dlrlIDL);
        }
        if(oidType == FULL_OID){
            fullOidDefault = true;
        }//else let it remain false
        topicModelGenerator = new TopicModelGenerator(
            mappingXML,
            dlrlIDL,
            dcpsIDL,
            getModel().getVerbose(),
            getModel().getOutputDirectoryPath(),
            fullOidDefault);

        try {
            /* annotate the mapping xml with missing mapping information. This information is derived from the
             * dlrl idl and validated against the dcps idl (to see if no conflicting topics are created).
             * generated information is annotated with a special tag to indicate the mapping information was generated
             * and not provided by the user
             */
            topicModelGenerator.generateMissingMappingXMLInformation(getModel().getValidateOnly());
            isDefaultMappingUsed = topicModelGenerator.isDefaultMappingUsed();

            /* validate the mapping XML, this mapping XML may contain annotated information, this should naturally not
             * be validated directly.
             */
            contentValidator.validateMappingXMLContent();

            /* only generate something if the user didnt indicate he just wanted validation of the input files */
            if(!getModel().getValidateOnly())
            {
                /***************************************************************************** ****************************/
                /***************************** now we actually output information for the user ****************************/
                /***************************************************************************** ****************************/
                if(getModel().getInvokeIdlPP() && (dcpsIdlFile != null))
                {
                    if(language == SACPP_LANG)
                    {
                        invokeIdlPP(dcpsIdlFile, null, new String[]{"-S", "-l", "cpp"});
                    } else
                    {
                        //must be SAJ_LANG
                        invokeIdlPP(dcpsIdlFile, null, new String[]{"-S", "-l", "java"});
                    }
                }
                /* If we annotated the mapping xml, then we need to generate a DCPS idl file which can be given as input for
                 * idlpp, this generation only take annotated information into account
                 */
                dcpsIDLGenerator.writeDcpsIdl() ;
                if(language == SACPP_LANG)
                {
                    invokeIdlPP(dcpsIDLGenerator.getGeneratedDCPSFile(), dlrlIdlFile, new String[]{"-S", "-l", "cpp"});
                } else
                {
                    //must be SAJ_LANG
                    invokeIdlPP(dcpsIDLGenerator.getGeneratedDCPSFile(), dlrlIdlFile, new String[]{"-S", "-l", "java"});
                }
                /* generate a big XML file containing all information for the XSLT engine to generate the DLRL files with */
                mergedXML = dlrlXMLGenerator.generateDLRLMergedXMLFile(dlrlIdlFile, dcpsIdlFile, dcpsIDLGenerator.getGeneratedDCPSFile());

                if(language == SACPP_LANG)
                {
                     /* generate the orb ready DLRL idl file */
                    orbFileConverter.convertDlrlIdl(dlrlIdlFile);
                    if (dcpsIDLGenerator.getGeneratedDCPSFile() != null)
                    {
                        invokeIdlPP(orbFileConverter.getOutputFile(), null, new String[]{"-S", "-l", "cpp"});
                    }
                    invokeCppgen(orbFileConverter.getOutputFile());
                }

                /* start the actual generation process of the DLRL output files */
                startXSLTEngine(mergedXML, templateGroups);

                if(language == SACPP_LANG)
                {
                    /* need to call cppgen also for the generate file by the xslt templates */
                    baseName = dlrlIdlFile.getName();
                    index = baseName.indexOf('.');
                    if(index != -1){
                        baseName = baseName.substring(0, index);
                    }
                    xsltIdlFile = new File(getModel().getOutputDirectoryPath()+File.separator+"ptdlrltmp"+File.separator+baseName+"Dlrl.idl");
                    invokeCppgen(xsltIdlFile);

                    if(!getModel().getVerbose())
                    {
                        //must delete xslt file first, so that then we delete the file generate by the file converter the
                        //directory is deleted correctly as well, could seperate the deleteGeneratedFile operation
                        //but hey life aint perfect and it just doesnt matter that much...
                        delete(xsltIdlFile);
                        orbFileConverter.deleteGeneratedFile();

                    }
                }
                /* Delete the intermediate files which were generated */
                cleanupIntermediateFiles(mergedXML);
            }
        } catch(Exception e){
            /* Delete the intermediate files which were (possibly) generated before propagating */
            if(!getModel().getVerbose())
            {
                if(mergedXML != null){
                    cleanupIntermediateFiles(mergedXML);
                }
                dcpsIDLGenerator.deleteGeneratedFile();
                if(language == SACPP_LANG)
                {
                    orbFileConverter.deleteGeneratedFile();
                }
            }
            throw e;
        }
    }

    private void cleanupIntermediateFiles(File mergedXML){
        File outputDir;
        File[] files;
        File aFile;
        String fileName;

        if (!(getModel().getVerbose())) {
            delete(mergedXML);
        }
        outputDir = getModel().getOutputDirectoryPath();
        files = outputDir.listFiles();
        for (int count = 0; count < files.length; count++) {
            aFile = files[count];
            if (aFile.isFile()) {
                fileName = aFile.getName();
                if ((fileName.toLowerCase()).endsWith(".xsl.out")) {
                    delete(aFile);
                }
            }
        }
    }

    private void delete(File file) {
        if (!file.delete()){
            if(getModel().getVerbose()){
                System.out.println("- Notification: Could not delete "+ file.getAbsolutePath());
            }
        }
    }

    /**
     * Returns the DCPS IDL file identifier string.
     *
     * @return java.lang.String
     */
    public String getDCPSIDLFileIdentifier() {
        return dcpsIdlFileId;
    }

    /**
     * Returns the DLRL IDL file identifier string.
     *
     * @return java.lang.String
     */
    public String getDLRLIDLFileIdentifier() {
        return dlrlIdlFileId;
    }

    /**
     * Returns the Mapping XML file identifier string.
     *
     * @return java.lang.String
     */
    public String getMappingXMLFileIdentifier() {
        return mappingXmlFileId;
    }

    /**
     * This method is responsible for merging the various template groups this
     * generator can be registed to into one template group and removing any multiple
     * references to a template file found. This method is required because the
     * generator may be registered to various template groups in the future (this
     * might happen if other backends are introduced and template groups are split
     * up).
     * The returning hash set will contain only one reference to the found template
     * files.
     *
     * @param templateGroups A hashMap containing all groups of template files.
     * Key: Idenitifer for the template groups
     * Value: A template file (java.io.File)
     * @return java.util.HashSet
     */
    private HashSet mergeTemplateGroups(HashMap templateGroups) {
        HashSet mergedTemplateFiles = new HashSet();
        Iterator groupIterator = templateGroups.values().iterator();
        while(groupIterator.hasNext()){
            HashSet aFileGroup = (HashSet)groupIterator.next();
            Iterator fileGroupIterator = aFileGroup.iterator();
            while(fileGroupIterator.hasNext()){
                File aTemplateFile = (File)fileGroupIterator.next();
                if(!(mergedTemplateFiles.contains(aTemplateFile))){
                    mergedTemplateFiles.add(aTemplateFile);
                }
            }
        }
        return mergedTemplateFiles;
    }

    /**
     * Sets the DCPS IDL file identifier string.
     *
     * @param dcpsIdlFileId The DCPS IDL File identifier string.
     * @roseuid 408CC05D03A2
     */
    public void setDCPSIDLFileIdentifier(String dcpsIdlFileId) {
        this.dcpsIdlFileId = dcpsIdlFileId;
    }

    /**
     * Sets the DLRL IDL file identifier string.
     *
     * @param dlrlIdlFileId The DLRL IDL File identifier string.
     * @roseuid 408CC03F0085
     */
    public void setDLRLIDLFileIdentifier(String dlrlIdlFileId) {
        this.dlrlIdlFileId = dlrlIdlFileId;
    }

    /**
     * Sets the Mapping XML file identifier string.
     *
     * @param mappingXmlFileId The Mapping XML File identifier string.
     * @roseuid 408CC051017F
     */
    public void setMappingXMLFileIdentifier(String mappingXmlFileId) {
        this.mappingXmlFileId = mappingXmlFileId;
    }

    /**
     * Starts the XSLT engine and will process each template file found (after they
     * are merged into one hash set by the mergeTemplateGroups(..) method).
     *
     * @param sourceXMLFile The source XML file that will be used for the XSLT
     * generation process
     * @param templateGroups A hashMap containing all groups of template files.
     * Key: Idenitifer for the template groups
     * Value: A template file (java.io.File)
     * @throws Exception
     */
    private void startXSLTEngine(java.io.File sourceXMLFile, HashMap templateGroups) throws Exception {
        //create XSLT transformer factory
        TransformerFactory tFactory = TransformerFactory.newInstance();
        if(getModel().getVerbose()){
            System.out.println("- Notification: Using the following engine for XSLT transformation: "+tFactory.getClass().getName());
        }
        //get all unique template files
        HashSet allUniqueTemplateFiles = mergeTemplateGroups(templateGroups);
        //for each template file, start XSLT generattion
        Iterator templateIterator = allUniqueTemplateFiles.iterator();
        while(templateIterator.hasNext()){
            File aTemplateFile = (File)templateIterator.next();
            //create an xslt transformer
            Transformer transformer = tFactory.newTransformer(new StreamSource(aTemplateFile.toURI().toURL().toString()));
            //start XSLT transformation with as source the sourceXMLFile, result the output path
            File outputResult = getModel().getOutputDirectoryPath();
            outputResult.mkdirs();
            transformer.setParameter("output.dir", outputResult.getAbsolutePath());
            transformer.setParameter("import.macro", macroName);
            transformer.setParameter("import.macro.header", macroHeaderFileName);
            transformer.transform(new StreamSource(sourceXMLFile.toURI().toURL().toString()), new StreamResult(new FileOutputStream(outputResult.getAbsolutePath()+File.separator+aTemplateFile.getName()+".out")));
        }
    }

    public void invokeCppgen(
        File outputFile) throws Exception
    {
        Vector commandList = new Vector();
        Vector includePaths;
        Process process;
        int exitVal;
        StreamHandlerUtil errorHandler;
        StreamHandlerUtil outputHandler;

        if(outputFile != null)
        {

            commandList.add("cppgen");
            if(macroName.length() > 0)
            {
                if(macroHeaderFileName.length() > 0)
                {
                    commandList.add("-import_export="+macroName+","+macroHeaderFileName);
                } else
                {
                    commandList.add("-import_export="+macroName);
                }

            }
            if(getModel().getVerbose())
            {
                commandList.add("-v");
            }
            includePaths = getModel().getIncludePaths();
            for(int count = 0; count < includePaths.size(); count++)
            {
                File path = (File)includePaths.get(count);
                commandList.add("-I"+path.getAbsolutePath());
            }
            commandList.add("-output="+getModel().getOutputDirectoryPath().getAbsolutePath());
            commandList.add(outputFile.getAbsolutePath());
            if(getModel().getVerbose())
            {
                StringBuffer buffer = new StringBuffer();
                for(int count = 0; count < commandList.size(); count++)
                {
                    buffer.append(commandList.get(count));
                    if((count+1) < commandList.size()){
                        buffer.append(" ");
                    }
                }
                System.out.println("- Notification: Invoking cppgen with the following command:\n\n\t"+buffer.toString()+"\n");
            }
            String[] commandsArray = new String[commandList.size()];
            for(int count = 0; count < commandList.size(); count++)
            {
                commandsArray[count] = (String)commandList.get(count);
            }
            process = Runtime.getRuntime().exec(commandsArray);
            
            errorHandler = new StreamHandlerUtil(process.getErrorStream(), null);
            outputHandler = new StreamHandlerUtil(process.getInputStream(), null);
            errorHandler.start();
            outputHandler.start();
            exitVal = process.waitFor();
            errorHandler.finished();
            outputHandler.finished();

            if(exitVal != 0)
            {
                throw new Exception("Invokation of cppgen has failed! Received exit code '"+exitVal+"'.");
            }
            if(getModel().getVerbose())
            {
                System.out.println("- Notification: Done invoking cppgen.");
            }
        }
    }

    public void invokeIdlPP(
        File sourceFile,/* may be null */
        File includePathToThisFile,/* may be null */
        String[] languageOptions) throws Exception
    {
        if(sourceFile != null)
        {
            Vector commandList = new Vector();
            Vector includePaths;
            Process process;
            int exitVal;
            StreamHandlerUtil errorHandler;
            StreamHandlerUtil outputHandler;
            File additionalIncludePath = null;
            File sourceFileIncludePath = null;

            if(includePathToThisFile != null)
            {
                additionalIncludePath = (new File(includePathToThisFile.getAbsolutePath())).getParentFile();
            }
            sourceFileIncludePath = (new File(sourceFile.getAbsolutePath())).getParentFile();

            commandList.add("idlpp");
            for (int count = 0; count < languageOptions.length; count++)
            {
                commandList.add(languageOptions[count]);
            }
            if(macroName.length() > 0)
            {
                if(macroHeaderFileName.length() > 0)
                {
                    commandList.add("-P");
                    commandList.add(macroName+","+macroHeaderFileName);
                } else
                {
                    commandList.add("-P");
                    commandList.add(macroName);
                }

            }
            if(sourceFileIncludePath != null)
            {
                commandList.add("-I"+sourceFileIncludePath.getAbsolutePath());
            }
            if(additionalIncludePath != null &&
                ((sourceFileIncludePath == null) || (additionalIncludePath.compareTo(sourceFileIncludePath) != 0)))
            {
                commandList.add("-I"+additionalIncludePath.getAbsolutePath());
            }
            includePaths = getModel().getIncludePaths();
            for(int count = 0; count < includePaths.size(); count++)
            {
                File path = (File)includePaths.get(count);
                if(additionalIncludePath == null || path.compareTo(additionalIncludePath) != 0)
                {
                    if(sourceFileIncludePath == null || path.compareTo(sourceFileIncludePath) != 0)
                    {
                        commandList.add("-I"+path.getAbsolutePath());
                    }
                }
            }
            commandList.add("-d");
            commandList.add(getModel().getOutputDirectoryPath().getAbsolutePath());
            commandList.add(sourceFile.getAbsolutePath());
            if(getModel().getVerbose())
            {
                StringBuffer buffer = new StringBuffer();
                for(int count = 0; count < commandList.size(); count++)
                {
                    buffer.append(commandList.get(count));
                    if((count+1) < commandList.size()){
                        buffer.append(" ");
                    }
                }
                System.out.println("- Notification: Invoking idlpp with the following command:\n\n\t"+buffer.toString()+"\n");
            }
            String[] commandsArray = new String[commandList.size()];
            for(int count = 0; count < commandList.size(); count++)
            {
                commandsArray[count] = (String)commandList.get(count);
            }
            process = Runtime.getRuntime().exec(commandsArray);            

            errorHandler = new StreamHandlerUtil(process.getErrorStream(), "idlpp");
            outputHandler = new StreamHandlerUtil(process.getInputStream(), "idlpp");
            errorHandler.start();
            outputHandler.start();
            exitVal = process.waitFor();
            errorHandler.finished();
            outputHandler.finished();

            if(exitVal != 0)
            {
                throw new Exception("Invokation of idlpp has failed! Received exit code '"+exitVal+"'.");
            }
            if(getModel().getVerbose())
            {
                System.out.println("- Notification: Done invoking idlpp.");
            }
        }
    }
}
