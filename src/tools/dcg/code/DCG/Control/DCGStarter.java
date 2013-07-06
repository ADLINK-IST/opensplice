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
package DCG.Control;

import DCG.FrontendXML.XMLParserHolder;
import DCG.FrontendIDL.IDLParserHolder;
import DCG.FrontendIDL.IDLErrorHandler;
import DCG.Core.MainModel;
import DCG.Core.Generator;
import DCG.BackendDLRL.DLRLGenerator;
import java.util.Vector;
import java.util.HashMap;
import java.util.HashSet;

/**
 * This class is responsible for correctly initializing all specialized layers and
 * the Core layer. New front ends, backends, source files, source validation
 * files, template groups and controller classes need to be defined here.
 */
public class DCGStarter {

    /**
     * String to identify the DCPS.IDL file (source file)
     */
    private String dcpsIdlKey = "DCPS_IDL";

    /**
     * String to identify the DCPS-leading mode
     */
    private String dcpsLeadingMode = "DCPS-Leading";

    /**
     * String to identify the DLRL.IDL file (source file)
     */
    private String dlrlIdlKey = "DLRL_IDL";

    /**
     * String to identify the DLRL-leading mode
     */
    private String dlrlLeadingMode = "DLRL-Leading";

    /**
     * String to identify the Mapping.XML file (source file)
     */
    private String mappingXmlKey = "MAPPING_XML";

    private org.xml.sax.ErrorHandler xmlErrorHandler = null;
    private IDLErrorHandler idlErrorHandler = null;

    private java.util.Vector includePaths = new java.util.Vector();
    private java.io.File dlrlFile = null;
    private java.io.File dcpsFile = null;
    private java.io.File mappingFile = null;
    private java.io.File outputDir = null;
    private String macroName = "";
    private String macroHeaderFileName = "";
    private boolean verbose = false;
    private boolean invokeIdlPP = false;
    private String language = null;
    public static final String JAVA_LANG = "JAVA";
    public static final String SACPP_LANG = "SACPP";
    public static boolean usagePrinted = false;
    public static final String SIMPLE_OID = "SimpleOid";
    public static final String FULL_OID = "FullOid";
    public String oid = null;
    public boolean validateOnly = false;
    private DLRLGenerator dlrlGenerator = null;
    private static boolean winOs = false;

    /**
     * The central access point to the Core layer. This model will be used by all
     * classes this DCGStarter initializes
     */
    private MainModel model;

    /**
     * Currently only one template group is required.
     */
    private String xsltOneTemplateId = "XSLT-1";


    public static void main(String[] args)
    {
       DCGStarter starter;

       try{
          starter = new DCGStarter(args);
       } catch (Exception e){
          System.err.print(e.getMessage());
          System.exit(1);
       }
    }

    /**
     * Responsible for initializing all generators, setting the modes they support and
     * registering them to the Core layer
     * @roseuid 40717A27005F
     */
    private void initGenerators() throws Exception{
        int oidType;
        int generatorLanguage = -1;
        HashMap sourceFileKeysForDlrlGen;
        HashSet templateIdsForDlrlGen;

        /*************** create generators used by this DCG*****************/
        //create DLRLGenerator

        //Create and fill the sourcefile keys map for the DLRL XML Generator
        //The key of this map is the source file id and the value is a boolean to indicate if the file is optional or not for this generator
        sourceFileKeysForDlrlGen =  new HashMap();
        sourceFileKeysForDlrlGen.put(dlrlIdlKey, new Boolean(true));
        sourceFileKeysForDlrlGen.put(mappingXmlKey, new Boolean(true));
        sourceFileKeysForDlrlGen.put(dcpsIdlKey, new Boolean(false));

        //create a set containing all supported template groups by this generator
        templateIdsForDlrlGen = new HashSet();
        templateIdsForDlrlGen.add(xsltOneTemplateId);

        if(oid != null && oid.equalsIgnoreCase(FULL_OID)){
            oidType = DLRLGenerator.FULL_OID;
        } else {
            oidType = DLRLGenerator.SIMPLE_OID;
        }
        if(language != null && language.equalsIgnoreCase(JAVA_LANG))
        {
            generatorLanguage = DLRLGenerator.JAVA_LANG;
        } else if(language != null && language.equalsIgnoreCase(SACPP_LANG))
        {
            generatorLanguage = DLRLGenerator.SACPP_LANG;
        }

        //create the generator itself, giving along the model, the source files it uses and the template groups it is interested in
        dlrlGenerator = new DLRLGenerator(
            model,
            sourceFileKeysForDlrlGen,
            templateIdsForDlrlGen,
            dlrlIdlKey,
            mappingXmlKey,
            dcpsIdlKey,
            oidType,
            generatorLanguage,
            macroName,
            macroHeaderFileName);
        //once the generator has been created, register the modes it can function under.
        //before setting a mode, the generator has to check if he can support the desired mode
        //a generator can support a mode if the required source files are in the defining set of file ids for the specific mode
        //(for example the defining set of file ids for the DCPS-leading mode is the DLRL.IDL and Mapping.XML file ids)
        if(dlrlGenerator.canSetSupportedMode(dlrlLeadingMode) )
        {
            dlrlGenerator.setSupportedMode(dlrlLeadingMode);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to set supported mode '"+dlrlLeadingMode+"' to DLRL Generator");
            }
        }
        if(dlrlGenerator.canSetSupportedMode(dcpsLeadingMode) )
        {
            dlrlGenerator.setSupportedMode(dcpsLeadingMode);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to set supported mode '"+dcpsLeadingMode+"' to DLRL Generator");
            }
        }

        //register the generator to the model
        model.registerGenerator(dlrlGenerator);
    }

    /**
     * Responsible for initializing all operating modes and registering them to the
     * Core layer
     * @roseuid 407179FF02F0
     */
    private void initModes()
    {
        HashSet dlrlLeadingModeFileIds;
        HashSet dcpsLeadingModeFileIds;

        //register DLRL leading mode to the main model
        dlrlLeadingModeFileIds =    new HashSet();
        dlrlLeadingModeFileIds.add(dlrlIdlKey);
        dlrlLeadingModeFileIds.add(mappingXmlKey);
        //Before registering a mode, a check needs to be made to see if the file id combination of this mode is unique
        if(model.canRegisterMode(dlrlLeadingModeFileIds, dlrlLeadingMode) )
        {
            model.registerMode(dlrlLeadingMode, dlrlLeadingModeFileIds);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to register mode '"+dlrlLeadingMode+"' with files '"+dlrlLeadingModeFileIds+"'");
            }
        }
        //register DCPS leading mode to the main model
        dcpsLeadingModeFileIds =    new HashSet();
        dcpsLeadingModeFileIds.add(dlrlIdlKey);
        dcpsLeadingModeFileIds.add(dcpsIdlKey);
        dcpsLeadingModeFileIds.add(mappingXmlKey);
        //Before registering a mode, a check needs to be made to see if the file id combination of this mode is unique
        if(model.canRegisterMode(dcpsLeadingModeFileIds, dcpsLeadingMode) )
        {
            model.registerMode(dcpsLeadingMode, dcpsLeadingModeFileIds);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to register mode '"+dcpsLeadingMode+"' with files '"+dcpsLeadingModeFileIds+"'");
            }
        }
    }

    /**
     * Responsible for initializing all source file handlers (with parser and AST) and
     * registering them to the Core layer
     * @roseuid 40717A1700DC
     */
    private void initSourceFileHandlers()
    {
        //DLRL IDL source file handler
        if(model.canCreateSourceFileHandler(dlrlIdlKey))
        {
            model.createDLRLSourceFileHandler(new IDLParserHolder(verbose, idlErrorHandler), dlrlIdlKey, null);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to create source file handler for file '"+dlrlIdlKey+"'");
            }
        }
        //DCPS IDL source file handler
        if(model.canCreateSourceFileHandler(dcpsIdlKey))
        {
            model.createDLRLSourceFileHandler(new IDLParserHolder(verbose, idlErrorHandler), dcpsIdlKey, null);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to create source file handler for file '"+dcpsIdlKey+"'");
            }
        }
        //Mapping XML source file handler
        if(model.canCreateSourceFileHandler(mappingXmlKey))
        {
            model.createDLRLSourceFileHandler(new XMLParserHolder(verbose, xmlErrorHandler), mappingXmlKey, null);
        } else
        {
            if(verbose)
            {
                System.out.println("--WARNING: Unable to create source file handler for file '"+mappingXmlKey+"'");
            }
        }
    }

    private String getFilenameFromArgs(String[] args, int[] index) throws Exception
    {
        int count = index[0];
        StringBuffer buffer = new StringBuffer("");
        String tmp = args[count];
        boolean beginFound = false;
        boolean endFound = false;

        //strip leading " or '
        while((tmp.startsWith("\"") || tmp.startsWith("'")) && tmp.length() > 0)
        {
            tmp = tmp.substring(1);
            beginFound = true;
        }
        //only if we found leading " or ' will we look for an ending ' or "
        if(beginFound)
        {
            while((tmp.endsWith("\"") || tmp.endsWith("'")) && tmp.length() > 0)
            {
                tmp = tmp.substring(0, tmp.length()-1);
                endFound = true;
            }
            buffer.append(tmp);
            if(!endFound)
            {
                //we will continue as long as we didnt find the ending " or ' or when or index exceeds the length
                for(count = count; count < args.length && !endFound; count++)
                {
                    tmp = args[count+1];
                    while((tmp.endsWith("\"") || tmp.endsWith("'")) && tmp.length() > 0)
                    {
                        tmp = tmp.substring(0, tmp.length()-1);
                        endFound = true;
                    }
                    buffer.append(" "+tmp);
                }
            }
            if(!endFound)
            {
                throw new Exception("Found an argument enclosed by ' or \" but could not locate an ending ' or \"");
            }
        } else
        {
            buffer.append(tmp);
        }
        index[0] = count;
        return buffer.toString();
    }

    private String stripQuotations(String arg)
    {
        return arg.replace("\"", "");
    }

    public void registerXMLErrorHandler(
        org.xml.sax.ErrorHandler xmlErrorHandler)
    {
        this.xmlErrorHandler = xmlErrorHandler;
    }

    public void registerIDLErrorHandler(
        IDLErrorHandler idlErrorHandler)
    {
        this.idlErrorHandler = idlErrorHandler;
    }

    private boolean checkOs(String osName)
    {
        boolean found = false;

        String os = System.getProperty("os.name");

	if (os.startsWith(osName))
	{
            found = true;
	}

        return found;
    }

    private void readArgs(String[] args) throws Exception
    {
        int[] indexArray = new int[1];
        String fileName;

        winOs = checkOs("Windows");

        for(int count = 0; count < args.length; count++)
        {
            String arg;

            arg = stripQuotations(args[count]);//shouldnt be neccesary to strip, but no time to properly fix it... this is needed due to ospldcg scripting.
            if(arg.equalsIgnoreCase("-dlrl"))
            {
                if(count+1 == args.length || stripQuotations(args[count+1]).startsWith("-"))
                {
                    throw new Exception("Expected the DLRL IDL file name to follow the '-dlrl' option. But could not locate a valid argument. Aborting..");
                }
                if(dlrlFile != null)
                {
                    throw new Exception("The '-dlrl' option was used twice, this is not allowed. Aborting..");
                }
                indexArray[0] = ++count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                dlrlFile = new java.io.File(fileName);
            } else if(arg.equalsIgnoreCase("-dcps"))
            {
                if(count+1 == args.length || stripQuotations(args[count+1]).startsWith("-"))
                {
                    throw new Exception("Expected the DCPS IDL file name to follow the '-dcps' option. But could not locate a valid argument. Aborting..");
                }
                if(dcpsFile != null)
                {
                    throw new Exception("The '-dcps' option was used twice, this is not allowed. Aborting..");
                }
                indexArray[0] = ++count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                dcpsFile = new java.io.File(fileName);
            } else if(arg.equalsIgnoreCase("-mapping"))
            {
                if(count+1 == args.length || stripQuotations(args[count+1]).startsWith("-"))
                {
                    throw new Exception("Expected the Mapping XML file name to follow the '-mapping' option. But could not locate a valid argument. Aborting..");
                }
                if(mappingFile != null)
                {
                    throw new Exception("The '-mapping' option was used twice, this is not allowed. Aborting..");
                }
                indexArray[0] = ++count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                mappingFile = new java.io.File(fileName);
            } else if(arg.equalsIgnoreCase("-o"))
            {
                if(count+1 >= args.length || stripQuotations(args[count+1]).startsWith("-")){
                    throw new Exception("Expected the output directory to follow the '-o' option. But could not locate a valid argument. Aborting..");
                }
                if(outputDir != null){
                    throw new Exception("The '-o' option was used twice, this is not allowed. Aborting..");
                }
                indexArray[0] = ++count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                outputDir = new java.io.File(fileName);
            } else if (arg.toLowerCase().startsWith("-o"))
            {
                if(outputDir != null)
                {
                    throw new Exception("The '-o' option was used twice, this is not allowed. Aborting..");
                }
                //remove the -O prefix...
                args[count] = arg.substring(2);
                indexArray[0] = count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                outputDir = new java.io.File(fileName);
            } else if(arg.equalsIgnoreCase("-i"))
            {
                if(count+1 >= args.length || stripQuotations(args[count+1]).startsWith("-")){
                    throw new Exception("Expected an include directory to follow the '-i' option. But could not locate a valid argument. Aborting..");
                }
                indexArray[0] = ++count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                includePaths.add(new java.io.File(fileName));
            } else if (arg.toLowerCase().startsWith("-i"))
            {
                args[count] = arg.substring(2);
                indexArray[0] = count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                includePaths.add(new java.io.File(fileName));
            } else if(arg.equalsIgnoreCase("-p"))
            {
                if(winOs == false)
                {
                    throw new Exception("The '-p' option can only be used in windows environments");
                }
                if(count+1 >= args.length || stripQuotations(args[count+1]).startsWith("-")){
                    throw new Exception("Expected the dll macro name to follow the '-p' option. But could not locate a valid argument. Aborting..");
                }
                count++;/* for the argument */
                macroName = stripQuotations(args[count]);
                int commaIndex = macroName.indexOf(",");
                if(commaIndex == 0)
                {
                    throw new Exception("Invalid argument provided for the '-p' option. No dll macro name found before the comma in '"+macroName+"'. Aborting..");
                }
                if(commaIndex != -1)
                {
                    macroHeaderFileName = macroName.substring(commaIndex+1, macroName.length());
                    if(macroHeaderFileName.length() == 0)
                    {
                        throw new Exception("Invalid argument provided for the '-p' option. No header file name found after the comma in '"+macroName+"'. Aborting..");
                    }
                    macroName = macroName.substring(0, commaIndex);
                }
            } else if(arg.equalsIgnoreCase("-gendcps"))
            {
                invokeIdlPP = true;
            } else if(arg.equalsIgnoreCase("-validate"))
            {
                validateOnly = true;
            }
            else if(arg.equalsIgnoreCase("-v"))
            {
                verbose = true;
            } else if(arg.equalsIgnoreCase("-l"))
            {
                if(count+1 >= args.length || stripQuotations(args[count+1]).startsWith("-")){
                    throw new Exception("Expected '"+JAVA_LANG+"' or '"+SACPP_LANG+"'to follow the '-l' option. But could not locate a valid argument. Aborting..");
                }
                if(language != null){
                    throw new Exception("The '-l' option was used twice, this is not allowed. Aborting..");
                }
                indexArray[0] = ++count;
                language = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
            } else if(arg.equalsIgnoreCase("-h") || arg.equalsIgnoreCase("-help"))
            {
                printUsage();
            } else if(arg.equalsIgnoreCase("-defaultoid"))
            {
                if(count+1 >= args.length|| stripQuotations(args[count+1]).startsWith("-"))
                {
                    throw new Exception("Expected 'SimpleOid' or 'FullOid' to follow the '-defaultoid' option. But could not locate a valid argument. Aborting..");
                }
                if(oid != null)
                {
                    throw new Exception("The '-defaultoid' option was used twice, this is not allowed. Aborting..");
                }
                indexArray[0] = ++count;
                oid = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
            } else if (arg.startsWith("-"))
            {
                throw new Exception("Illegal option '"+arg+"'! Aborting...");
            }
            else
            {
                if(dlrlFile != null)
                {
                    throw new Exception("A DLRL model file has already been specified for this command. No further files can be processed. Aborting..");
                }
                indexArray[0] = count;
                fileName = getFilenameFromArgs(args, indexArray);
                count = indexArray[0];
                dlrlFile = new java.io.File(fileName);
            }
        }
    }

    /**
     * The main initialize method for the DCG program
     *
     * @param args
     */
    private DCGStarter(String[] args) throws Exception
    {
        long initStart = System.currentTimeMillis();
        if (args == null || args.length == 0 || helpRequested(args))
        {
            System.out.print("\nOpenSplice (c) PrismTech Ltd.\n");
            printUsage();
        } else
        {
            System.out.print("\nOpenSplice (c) PrismTech Ltd.\n\n");
            startup(initStart, args);
        }
    }

    private boolean helpRequested(String[] args)
    {
        for(int count = 0; count < args.length; count++)
        {
            String arg = stripQuotations(args[count]);
            if(arg.equalsIgnoreCase("-h") || arg.equalsIgnoreCase("-help"))
            {
                return true;
            }
        }
        return false;
    }

    /* scdds2308: ES: Maintainted for backwards compatibility */
    public DCGStarter(
        java.io.File dlrlFile,
        java.io.File dcpsFile,
        java.io.File mappingFile,
        java.io.File outputDir,
        Vector includeDirPaths,
        boolean verbose,
        String language,
        String oid,
        boolean validateOnly) throws Exception
    {
        constructDCGStarter(
            dlrlFile,
            dcpsFile,
            mappingFile,
            outputDir,
            includeDirPaths,
            verbose,
            language,
            oid,
            validateOnly,
            "",
            "");
    }

    public DCGStarter(
        java.io.File dlrlFile,
        java.io.File dcpsFile,
        java.io.File mappingFile,
        java.io.File outputDir,
        Vector includeDirPaths,
        boolean verbose,
        String language,
        String oid,
        boolean validateOnly,
        String macroName,
        String macroHeaderFileName) throws Exception
    {
        constructDCGStarter(
            dlrlFile,
            dcpsFile,
            mappingFile,
            outputDir,
            includeDirPaths,
            verbose,
            language,
            oid,
            validateOnly,
            macroName,
            macroHeaderFileName);
    }

    private void constructDCGStarter(
        java.io.File dlrlFile,
        java.io.File dcpsFile,
        java.io.File mappingFile,
        java.io.File outputDir,
        Vector includeDirPaths,
        boolean verbose,
        String language,
        String oid,
        boolean validateOnly,
        String macroName,
        String macroHeaderFileName) throws Exception
    {
        long initStart = System.currentTimeMillis();
        System.out.print("\nOpenSplice (c) PrismTech Ltd.\n\n");
        this.dlrlFile = dlrlFile;
        this.dcpsFile = dcpsFile;
        this.mappingFile = mappingFile;
        this.outputDir = outputDir;
        if(includeDirPaths != null)
        {
            this.includePaths = includeDirPaths;
        }
        this.verbose = verbose;
        this.language = language;
        this.oid = oid;
        this.validateOnly = validateOnly;
        this.macroName = macroName;
        this.macroHeaderFileName = macroHeaderFileName;
        startup(initStart, null);
    }

    public boolean isDefaultMappingUsed(
        ) throws Exception
    {
        if(dlrlGenerator == null)
        {
            throw new Exception("Unable to verify mapping completeness, failure in DCG start up.");
        }
        return dlrlGenerator.isDefaultMappingUsed();
    }

    private void verifyInput() throws Exception
    {
        if(dlrlFile == null)
        {
            throw new Exception("No DLRL IDL file specified. Aborting...");
        } else
        {
            verifyFile(dlrlFile, "-dlrl");
        }
        if(mappingFile == null)
        {
            throw new Exception("No mapping XML file specified. Aborting...");
        } else {
            verifyFile(mappingFile, "-mapping");
        }
        if(dcpsFile == null)
        {
            if(verbose)
            {
                System.out.println("No dcps file specified (which is allowed).");
            }
        } else
        {
            verifyFile(dcpsFile, "-dcps");
        }
        if(outputDir == null)
        {
            outputDir = new java.io.File(".");
            if(verbose){
                System.out.println("No output directory specified. Defaulting to current file directory ("+outputDir.getAbsolutePath()+").");

            }
        }
        outputDir.mkdirs();
        if(!outputDir.isDirectory())
        {
            throw new Exception("The output dir path '"+outputDir.getAbsolutePath()+"' following option '-o' does not specify a directory! Aborting...");
        }
        if(language == null)
        {
            if(verbose){
                System.out.println("No target language specified. Defaulting to '"+JAVA_LANG+"'");
            }
            language = JAVA_LANG;
        } else if(!language.equalsIgnoreCase(JAVA_LANG) && !language.equalsIgnoreCase(SACPP_LANG))
        {
            throw new Exception("Invalid target language '"+language+"' detected");
        }
        if(oid == null)
        {
            if(verbose)
            {
                System.out.println("No oid type for default generated topics specified. Defaulting to '"+SIMPLE_OID+"'");
            }
            oid = SIMPLE_OID;
        } else if(!oid.equalsIgnoreCase(FULL_OID) && !oid.equalsIgnoreCase(SIMPLE_OID))
        {
            throw new Exception("Invalid oid type for default generated topics '"+oid+"' detected");
        }
        for(int count =0; count < includePaths.size(); count++)
        {
            Object file = includePaths.get(count);
            if(!(file instanceof java.io.File))
            {
                throw new Exception("The include paths contains a non java.io.File object!. The Vector may only contain File objects");
            }
            java.io.File includeDirPath = (java.io.File)includePaths.get(count);
            if(!includeDirPath.exists())
            {
                if(verbose)
                {
                    System.out.println("The include path '"+includeDirPath.getAbsolutePath()+"' provided does not exist! Ignoring path.");
                }
            } else if(!includeDirPath.isDirectory())
            {
                throw new Exception("The include path '"+includeDirPath.getAbsolutePath()+"' provided does not specify a directory! Aborting...");
            }
        }
    }

    private void verifyIncludePaths (String systemVariable) throws Exception
    {
        String osplHome = null;

        if (systemVariable.equals("OSPL_HOME"))
        {
           osplHome = System.getProperty (systemVariable);
        }
        if(osplHome == null)
        {
           osplHome = System.getenv (systemVariable);
        }
        if(osplHome == null)
        {
           throw new Exception("Unable to locate the enviroment variable '" + systemVariable + "'. " +
                               "Make sure this variable is set in your enviroment and points to your OpenSplice installation.");
        }
        String seperator = java.io.File.separator ;
        String inclPath = osplHome+seperator+"etc"+seperator+"idl";

        boolean alreadyIncluded = false;
        for (int i = 0; i < includePaths.size() && !alreadyIncluded; i++)
        {
            java.io.File aFile = (java.io.File) includePaths.get(i);
            if (aFile.getAbsolutePath().equalsIgnoreCase(inclPath))
            {
                alreadyIncluded = true;
            }
        }
        if (!alreadyIncluded)
        {
            includePaths.insertElementAt(new java.io.File(inclPath), 0);
        }
    }

    private void startup(long initStart, String[] args) throws Exception
    {
        try{
            if(args != null)
            {
                readArgs(args);
            }

            /* Check whether default include path was already included. */
            try
            {
                verifyIncludePaths ("OSPL_HOME");
            }
            catch (Exception e)
            {
                throw e;
            }

            verifyInput();
            model = new MainModel();
            model.setValidateOnly(validateOnly);
            model.setVerbose(verbose);
            model.setInvokeIdlPP(invokeIdlPP);
            initModes();
            initSourceFileHandlers();
            initGenerators();
            long[]  times = new long[3];

            if(model.canRegisterSourceFile(dlrlIdlKey))
            {
                model.registerSourceFileToHandler(dlrlIdlKey, dlrlFile);
            }
            if(model.canRegisterSourceFile(mappingXmlKey))
            {
                model.registerSourceFileToHandler(mappingXmlKey, mappingFile);
            }
            if(dcpsFile != null && model.canRegisterSourceFile(dcpsIdlKey))
            {
                model.registerSourceFileToHandler(dcpsIdlKey, dcpsFile);
            }
            registerTemplateFiles(language);

            model.setMode();
            model.setOutputDirectoryPath(outputDir);
            model.setIncludePaths(includePaths);
            times[0] = System.currentTimeMillis();
            if(model.canProcessDLRLSourceFile(dlrlIdlKey) )
            {
                model.processDLRLSourceFile(dlrlIdlKey);
            }
            if(model.canProcessDLRLSourceFile(mappingXmlKey) )
            {
                model.processDLRLSourceFile(mappingXmlKey);
            }
            if(dcpsFile != null && model.canProcessDLRLSourceFile(dcpsIdlKey) )
            {
                model.processDLRLSourceFile(dcpsIdlKey);
            }
            times[1] = System.currentTimeMillis();
            model.executeGenerationSequence();
            times[2] = System.currentTimeMillis();

            System.out.println("Total run time : "+((times[2]-initStart)/1000.0)+" seconds");
            System.out.println("\tInitialization time : "+((times[0]-initStart)/1000.0)+" seconds");
            System.out.println("\tParsing time : "+((times[1]-times[0])/1000.0)+" seconds");
            System.out.println("\tGeneration time : "+((times[2]-times[1])/1000.0)+" seconds");
        } catch (Exception e)
        {
            StringBuffer msg = new StringBuffer("Generation sequence failed...\n");
            msg.append("The following fatal error was reported:\n\n"+e.getMessage()+"\n");
            if(verbose)
            {
                e.printStackTrace();
            }
            msg.append("\nDLRL Code Generator is terminating...\n");
            throw new Exception(msg.toString());
        }
    }

    private void printUsage()
    {
        if(!usagePrinted)
        {

            String seperator = java.io.File.separator ;

            System.out.print("\nUsage: ospldcg [options] [FILE]");
            System.out.print("\nRun the OpenSplice DLRL code generator on FILE");
            System.out.print("\nAll options are case insensitive and may be used in random order.");
            System.out.print("\nSpaces in file names or paths should be enclosed by \\\" or \\'.");
            System.out.print("\nOptions are:");

            System.out.print("\n\t[-dcps <filename>]");
            System.out.print("\n\t\tThis option specifies the IDL input file containing the DCPS Topic Model");
            System.out.print("\n\t\tdefinition. Even when this option is specified, the DCG may still generate");
            System.out.print("\n\t\ta complementary topics IDL file if any additional topic information is required.");
            System.out.print("\n\t\tIn such a case the output file is generated in the output directory specified");
            System.out.print("\n\t\tby the '-o' option. The OpenSplice DCG will invoke the OpenSplice IDL");
            System.out.print("\n\t\tPre-Processor tool automatically for this generated file. The name of this");
            System.out.print("\n\t\tgenerated file is chosen equal to the name of the 'Dlrl' element in the ");
            System.out.print("\n\t\tcorresponding XML mapping file (that is specified by the '-mapping' option),");
            System.out.print("\n\t\tpost fixed with '.idl'.");

            System.out.print("\n\t[-dlrl <filename>]");
            System.out.print("\n\t\tThis option specifies the IDL file containing the DLRL Object Model");
            System.out.print("\n\t\tdefinition. It is equivalent  to and an alternate to specifying FILE.");

            System.out.print("\n\t[-mapping <filename>]");
            System.out.print("\n\t\tThis mandatory option specifies the XML mapping file containing mapping");
            System.out.print("\n\t\tinformation between the Object Model and the Topic Model definitions. If");
            System.out.print("\n\t\tany mapping information is missing the OpenSplice DCG will try to annotate");
            System.out.print("\n\t\tthe mapping file and output the result to the output directory specified");
            System.out.print("\n\t\tby the '-o' option. The name of this generated file is chosen equal to the");
            System.out.print("\n\t\tname of the 'Dlrl' element in the specified XML mapping input file by this");
            System.out.print("\n\t\t'-mapping' option, post fixed with '.xml'.");

            System.out.print("\n\t[-o <path>] or [-o<path>]");
            System.out.print("\n\t\tThis option specifies the output directory where the OpenSplice DCG should ");
            System.out.print("\n\t\tgenerate it's output files. If no output path is specified then the OpenSplice");
            System.out.print("\n\t\tDCG will generate it's output to the current file directory. If neccesary the ");
            System.out.print("\n\t\toutput directory will be created.");

            System.out.print("\n\t[-I <path>] or [-I<path>]");
            System.out.print("\n\t\tThis option specifies an IDL include file path.");
            System.out.print("\n\t\tThis option may be used zero or more times.");
            System.out.print("\n\t\tThe etc" + seperator + "idl path in the OSPL_HOME release direcory has already");
            System.out.print("\n\t\tbeen set by default.");

            System.out.print("\n\t[-defaultOid (FullOid | SimpleOid)]");
            System.out.print("\n\t\tThis option specifies the content value of the key descriptions that default");
            System.out.print("\n\t\tgenerated topics will use. This option is only applicable to DLRL classes that");
            System.out.print("\n\t\thave no corresponding topic definition. If not specified 'SimpleOid' will be used");
            System.out.print("\n\t\tby default. Be aware that selecting 'FullOid' is less efficient due to the");
            System.out.print("\n\t\taddition of an extra string key.");

            System.out.print("\n\t[-gendcps]");
            System.out.print("\n\t\tIf this option in combination with the '-dcps' option is specified then the");
            System.out.print("\n\t\tOpenSplice DCG will invoke the OpenSplice IDL Pre-processor for the file");
            System.out.print("\n\t\tlisted for the '-dcps' option. The same language options, include paths and");
            System.out.print("\n\t\toutput directory will be used for invoking of the OpenSplice IDL Pre-processor");
            System.out.print("\n\t\tas was used for the invoking of OpenSplice DCG.");
            System.out.print("\n\t\tUsing this option without specifying the '-dcps' option has no meaning.");

            System.out.print("\n\t[-validate]");
            System.out.print("\n\t\tIf this option is specified then the DCG will only validate the input files and");
            System.out.print("\n\t\twill not generate any output files.");
            if(winOs == true)
            {
                System.out.print("\n\t[-p <dll-macro-name>,[header-file]]");
                System.out.print("\n\t\tOnly applicable to C++. Sets export macro that will be prefixed to all ");
                System.out.print("\n\t\tfunctions in the generated code. This allows creating DLL's from generated code. ");
                System.out.print("\n\t\tOptionally a header file can be given that will be included in each generated file.");
            }
            System.out.print("\n\t[-v]");
            System.out.print("\n\t\tIf this option is specified additional messages will be printed to the default");
            System.out.print("\n\t\toutput stream.");

            System.out.print("\n\t[-l (JAVA | SACPP)]");
            System.out.print("\n\t\tThis option specifies the desired language for which the DCG should generate");
            System.out.print("\n\t\toutput. The JAVA language is the default if this option is not specified.");

            System.out.print("\n\t[-h] or [-help]");
            System.out.print("\n\t\tDisplay this information.");

            System.out.print("\n\n\tExample:");
            System.out.print("\n\t\tospldcg -mapping mapping.xml -dlrl objects.idl -dcps topics.idl -I" +
                                                        seperator + "opt" + seperator + "ospl" + seperator + "include");
            System.out.print("\n\t\t-I " + seperator + "opt" + seperator + "ospl2 -o\\\"" + seperator + "work" +
                                                                    seperator + "my name" + seperator + "output\\\"\n");
            usagePrinted = true;
        }//else do nothing
    }

    private void addTemplateFiles (java.util.Vector templateFiles, String language, String osplHome) throws Exception
    {
        String seperator = java.io.File.separator ;
        if(language != null && language.equalsIgnoreCase(JAVA_LANG))
        {
            String languageDirName = "SAJ";
            String basePath = osplHome+seperator+"etc"+seperator+"dcg"+seperator+languageDirName;
            templateFiles.add(new java.io.File(basePath+seperator+"typed_Object_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_ObjectImpl_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_ObjectHome_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_ObjectListener_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_Selection_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_SelectionListener_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_FilterCriterion_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_Set_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_StrMap_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_IntMap_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_List_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_enum_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_struct_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_union_template.xsl"));
        }
        else if(language != null && language.equalsIgnoreCase(SACPP_LANG))
        {
            String basePath = osplHome+seperator+"etc"+seperator+"dcg"+seperator+"idl";
            templateFiles.add(new java.io.File(basePath+seperator+"typed_dlrl_impl.xsl"));
            basePath = osplHome+seperator+"etc"+seperator+"dcg"+seperator+"CCPP";
            templateFiles.add(new java.io.File(basePath+seperator+"typed_dlrl_impl_header_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_dlrl_impl_ccpp_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_abstract_header_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_abstract_ccpp_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_impl_header_template.xsl"));
            templateFiles.add(new java.io.File(basePath+seperator+"typed_impl_ccpp_template.xsl"));
            basePath = osplHome+seperator+"etc"+seperator+"dcg"+seperator+"SACPP";
            templateFiles.add(new java.io.File(basePath+seperator+"typed_header_template.xsl"));
        }
        else
        {
            throw new Exception("Invalid target language '"+language+"' detected");
        }
    }

    private void checkTemplateFiles (java.util.Vector templateFiles) throws Exception
    {
        for(int count = 0; count < templateFiles.size(); count++)
        {
            java.io.File aFile = (java.io.File)templateFiles.get(count);
            String path = aFile.getAbsolutePath();
            if(!aFile.isFile())
            {
               throw new Exception("The XSLT template file '"+path+"' is not a file");
            } else if(!aFile.exists())
            {
               throw new Exception("The XSLT template file '"+path+"' does not exist");
            } else if(!aFile.canRead())
            {
               throw new Exception("The XSLT template file '"+path+"' can not be read");
            }
        }
    }

    private void registerTemplateFiles(String language) throws Exception
    {
        java.util.Vector templateFiles = new java.util.Vector();
        String osplHome;

        osplHome = System.getProperty("OSPL_HOME");
        if(osplHome == null)
        {
            osplHome = System.getenv ("OSPL_HOME");
            if(osplHome == null)
            {
                throw new Exception("Unable to locate the enviroment variable 'OSPL_HOME'. " +
                                "Make sure this variable is set in your enviroment and points to your OpenSplice installation.");
            }
        }
        addTemplateFiles (templateFiles, language, osplHome);
        checkTemplateFiles (templateFiles);

        for(int count = 0; count < templateFiles.size(); count++)
        {
            java.io.File aFile = (java.io.File)templateFiles.get(count);
            if(model.canSetTemplateSourceFile(xsltOneTemplateId, aFile))
            {
                model.setTemplateSourceFile(aFile, xsltOneTemplateId);
            }
        }
    }

    private void verifyFile(java.io.File aFile, String name) throws Exception
    {
        if(!aFile.isFile())
        {
            throw new Exception("The file ("+aFile.getAbsolutePath()+") provided for option '"+name+"' is not a file");
        } else if(!aFile.exists())
        {
            throw new Exception("The file ("+aFile.getAbsolutePath()+") provided for option '"+name+"' does not exist");
        } else if(!aFile.canRead())
        {
            throw new Exception("The file ("+aFile.getAbsolutePath()+") provided for option '"+name+"' can not be read");
        }
    }

}
