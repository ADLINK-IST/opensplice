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
package DCG.Core;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

/**
 * The MainModel class acts as the heart of the DCG program. It is responsible for
 * allowing the different components to work together and is the central access
 * class for every component. Outside controllers (like the command line
 * interface) need to communicate with this class in order to successfully use the
 * DCG program.
 */
public class MainModel {

    /**
     * A static final String representing the operating error mode.
     */
    public static final String errorMode = "ERROR_MODE";

    /**
     * A String containing the current operating mode of the DCG program.
     */
    private String mode = errorMode;

	/**
	 * The path of the output directory as specified by the user.
	 */
    private java.io.File outputDirectoryPath;

    private java.util.Vector includePaths = null;

    private boolean verbose = false;

    private boolean invokeIdlPP = false;

    private boolean validateOnly = false;

    /**
     * A HashMap containing the modes registered to this MainModel.
     *
     * key: modeName (String object)
     * value: A HashSet containing the (unique) file IDs combination used to identify
     * this mode (String objects)
     */
    private HashMap registeredModes = new HashMap();

    /**
     * A HashMap containing the various template groups.
     * Key : template group identifier (String object)
     * value : A HashSet containing the template files (java.io.File)
     */
    private HashMap templateGroups = new HashMap();

	private HashSet generators = new HashSet();
	public HashSet sourceFileHandlers = new HashSet();

    /**
     * The default constructor
     * @roseuid 405B001B02F0
     */
    public MainModel() {
		//do nothing
    }

    /**
     * Returns true if a source file handler isnt yet present for this source file. If
     * this source file is registered to another source file handler as a validation
     * file id, then this method will still return true (Case sensitive)
     *
     * @param fileId The file identifier for which a source file has to be created
     * @return boolean
     * @roseuid 40724E2B0396
     */
	public boolean canCreateSourceFileHandler(String fileId){
		HashSet handlers = getDLRLSourceFileHandlers();
		Iterator handlerIterator = handlers.iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			if(aHandler.getSourceFileIDName().equals(fileId)){
				return false;
			}
		}
		return true;
	}

    public void setValidateOnly(
        boolean validateOnly)
    {
        this.validateOnly = validateOnly;
    }

    public boolean getValidateOnly(
        )
    {
        return this.validateOnly;
    }

    /**
     * Returns true if a source file can be processed. A source file can be processed
     * if a source file handler is present with a parser and abstract syntax tree
     * defined for the source file handler.Also checks if the source file is allocated
     * (meaning if the java.io.File has been set) and checks if a validation source
     * file is allocated, if required.
     *
     * @param key The file identifier to identify the source file
     * @return boolean
     * @roseuid 406834160193
     */
	public boolean canProcessDLRLSourceFile(String key){
		Iterator handlerIterator = getDLRLSourceFileHandlers().iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			//check if the source file id name is specified and match the key
			if(aHandler.getSourceFileIDName().equals(key)){
				//check if the source file is specified and
				//check if the parser holder is specified and
				//check if the ast holder is specified and
				//check if the source validation file  is specified only if the source validation
				//file id name is specified
				if(aHandler.getSourceFile() != null && aHandler.getParserHolder() != null && aHandler.getASTHolder() != null
						&& (aHandler.getSourceValidationFileIDName() != null && aHandler.getSourceValidationFile() != null ||
							aHandler.getSourceValidationFileIDName() == null) ){
					return true;
				}
			}
		}
		return false;
	}

    /**
     * Checks if the mode name isn't used already and if the set of file identifiers
     * to identify this mode isn't already registered to another mode. Also checks if
     * the mode name specified doesnt match the error mode name
     * @param modeFileIDs The (unique) set of file identifiers to define the mode
     *  - The HashSet containing the file identifiers defining the mode
     * @param modeName The (unique) mode name - The name of the to-be-registered mode
     * @return boolean
     * @roseuid 406C15210228
     */
	public boolean canRegisterMode(HashSet modeFileIDs, String modeName){
		//first check if the mode name is already known (case sensitive)
		if(registeredModes.keySet().contains(modeName) || modeName.equals(errorMode)){
			return false;
		} else {
			//if the mode name isnt present check if the set of mode file ids is already in existance
			//first get a hashset containing all hashsets of registered mode file ids
			Iterator i = registeredModes.values().iterator();
			while(i.hasNext()){
				HashSet registeredFileIds = (HashSet)i.next();
				if(collectionsAreIdentifical(registeredFileIds, modeFileIDs)){
					return false;
				}
			}
		}
		return true;
	}

    /**
     * Returns true if the source file handler for the specified source file id is
     * known within the Mainmodel
     *
     * @param sourceFileID The file identifier identifying the source file
     * @return boolean
     * @roseuid 406C1D670100
     */
	public boolean canRegisterSourceFile(String sourceFileID){
		HashSet handlers = getDLRLSourceFileHandlers();
		Iterator handlerIterator = handlers.iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			if(aHandler.getSourceFileIDName().equals(sourceFileID)){
				return true;
			} else if(aHandler.getSourceValidationFileIDName() != null && aHandler.getSourceValidationFileIDName().equals(sourceFileID)){
				return true;
			}
		}
		return false;
	}

    /**
     * Returns true if the template file isn't already contained within the template
     * group
     *
     * @param file The file identifying the template file location on disk
     * @param templateGroupId The template group id to register the file to
     * @return boolean
     * @roseuid 406C1F70014D
     */
	public boolean canSetTemplateSourceFile(String templateGroupId, java.io.File file){
		HashSet templateFiles = (HashSet)templateGroups.get(templateGroupId);
		if(templateFiles != null && templateFiles.contains(file)){
			return false;
		}
		return true;
	}

    /**
     * Helper method for comparing two hash sets. Will return true if both hash sets
     * contain the exact same elements. Elements must implement the compareTo method
     * 	and be comparable to eachother
     *
     * @param set1 The first hashset to be compared
     * @param set2 The second hash set to be compared
     * @return boolean
     * @roseuid 40764B83035E
     */
	private boolean collectionsAreIdentifical(HashSet set1, HashSet set2){
		if(set1.size() == set2.size()){
			Iterator set1Iterator = set1.iterator();
			while(set1Iterator.hasNext()){
				Object element = set1Iterator.next();
				if(!set2.contains(element)){
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
	}

    /**
     * Creates a source file handler
     *
     * @param parser The parser to parse the source file
     * @param sourceFileIdKey The identifier to identify the source file which is
     * managed by this source file handler
     * @param sourceFileValidatorFileKey An optional source file validation file. The
     * parser may require this file for successful parsing. May be NULL if the file is
     * not needed.
     * @roseuid 406435EF0072
     */
	public void createDLRLSourceFileHandler(ParserHolderable parser, String sourceFileIdKey, String sourceFileValidatorFileKey){
		SourceFileHandler handler = new SourceFileHandler(parser, sourceFileIdKey, sourceFileValidatorFileKey);
		sourceFileHandlers.add(handler);
	}

    /**
     * Ensures all generators that support the current operating mode start their
     * generation sequence.
     * @roseuid 406D137701E4
     */
	public void executeGenerationSequence() throws Exception{
		//Before generation is started the mode is set to ensure the correct operating mode is used
		setMode();
		HashSet modeGenerators = getAllRegisteredGeneratorsForMode(getMode());
		Iterator genIterator = modeGenerators.iterator();
		while(genIterator.hasNext()){
			Generator aGenerator = (Generator)genIterator.next();
			aGenerator.startGeneration();
		}
	}

    /**
     * Checks if the file id given is registered as a validation file in one of the
     * handlers
     *
     * @param fileID The file ID to check to see if it belong to a validation file ID
     * in one of the handlers
     * @return boolean
     * @roseuid 4076A90C020A
     */
	public boolean fileIdIsValidationFileInHandler(String fileID){
		Iterator handlerIterator =  getDLRLSourceFileHandlers().iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			if(aHandler.getSourceValidationFileIDName() != null && aHandler.getSourceValidationFileIDName().equals(fileID)){
				return true;
			}
		}
		return false;
	}

    /**
     * Returns a HashSet containing all registered generators
     * @return java.util.HashSet
     * @roseuid 406D1BA1004A
     */
	public HashSet getAllRegisteredGenerators(){
		return generators;
	}

    /**
     * Returns all generators supporting a specific operating mode
     *
     * @param mode The operating mode to be checked
     * @return java.util.HashSet
     * @roseuid 406D142E02E0
     */
	public HashSet getAllRegisteredGeneratorsForMode(String mode){
		HashSet generatorsForMode = new HashSet();
		Iterator genIterator = generators.iterator();
		while(genIterator.hasNext()){
			Generator aGenerator = (Generator)genIterator.next();
			if(aGenerator.getSupportedModes().contains(mode)){
				generatorsForMode.add(aGenerator);
			}
		}
		return generatorsForMode;
	}

    /**
     * Returns a HashMap containing all registered modes (String objects)
     * @return java.util.HashMap
     * @roseuid 406C26A103CD
     */
	public HashMap getAllRegisteredModes(){
		return registeredModes;
	}

    /**
     * Returns the abstract syntax tree for a specific source file.
     *
     * @param sourceFileId The source file id to find the AST for
     * @return DCG.Core.AbstractSyntaxTreeHolder
     * @roseuid 406D18C70004
     */
	public AbstractSyntaxTreeHolder getASTreeForSourceFileID(String sourceFileId){
		Iterator handlerIterator =  getDLRLSourceFileHandlers().iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			if(aHandler.getSourceFileIDName().equals(sourceFileId)){
				return aHandler.getASTHolder();
			}
		}
		return null;
	}

    public java.io.File getFileForSourceFileID(String sourceFileId){
        if(sourceFileId != null)
        {
            Iterator handlerIterator =  getDLRLSourceFileHandlers().iterator();
            while(handlerIterator.hasNext()){
                SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
                if(aHandler.getSourceFileIDName().equals(sourceFileId)){
                    return aHandler.getSourceFile();
                }
            }
        }
		return null;
	}
    /**
     * Returns a HashSet containing all known source file handlers
     * @return java.util.HashSet
     * @roseuid 406833D203E5
     */
	public HashSet getDLRLSourceFileHandlers() {
		return sourceFileHandlers;
	}

    /**
     * Returns the HashSet of file identifiers belong to the specified mode, or null
     * if the mode isn't defined
     *
     * @param modeName The (unique) mode name identifying the needed mode
     * @return java.util.HashSet
     * @roseuid 406C19A801D1
     */
	public HashSet getFileIdsForMode(String modeName){
		return (HashSet)registeredModes.get(modeName);
	}

    /**
     * Returns the current operating mode
     * @return java.lang.String
     * @roseuid 405AE3000175
     */
	public String getMode(){
		return mode;
	}

    /**
     * Returns all template files belong to a specific template group identifier as a
     * HashSet containing java.io.File objects
     *
     * @param templateGroupKey The key identifying the template group
     * @return java.util.HashSet
     * @roseuid 406D19CC007F
     */
	public HashSet getTemplateFilesForGroup(String templateGroupKey){
		return (HashSet)templateGroups.get(templateGroupKey);
	}


    /**
     * Processes a source file
     *
     * @param key The file identifier to identify the source file
     * @roseuid 4069265900B2
     */
	public void processDLRLSourceFile(String key) throws Exception{
		Iterator handlerIterator = getDLRLSourceFileHandlers().iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			if(aHandler.getSourceFileIDName().equals(key)){
				aHandler.processSourceFile(this.includePaths);
			}
		}
	}

    /**
     * Registers a generator to the MainModel
     *
     * @param generator The generator to be registered
     * @roseuid 4067DFB101ED
     */
	public void registerGenerator(Generator generator){
		generators.add(generator);
	}

    /**
     * Registers a mode to the MainModel
     *
     * @param modeName the (unique) mode name
     * @param modeFileIDs The (unique) set of file identifiers to define the mode
     * @roseuid 406C15690235
     */
	public void registerMode(String modeName, HashSet modeFileIDs){
		registeredModes.put(modeName, modeFileIDs);
	}

    /**
     * Registers the specified source file to the correct source file handler (either
     * as source file or as source validation file)
     *
     * @param fileID The String identifier of the source (validation) file
     * @param file The actual source or source validation file
     * @roseuid 406BD23800FD
     */
	public void registerSourceFileToHandler(String fileID, java.io.File file){
		Iterator handlerIterator =  getDLRLSourceFileHandlers().iterator();
		boolean registered = false;
		while(handlerIterator.hasNext() && !registered){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			String sourceFile = aHandler.getSourceFileIDName();
			String sourceValidationFile = aHandler.getSourceValidationFileIDName();
			if(sourceFile.equals(fileID)){
				aHandler.setFileForFileID(sourceFile, file);
				registered = true;
			} else if(sourceValidationFile !=null && sourceValidationFile.equals(fileID)){
				aHandler.setFileForFileID(sourceValidationFile, file);
				registered = true;
			}
		}
	}

    /**
     * Determines the current operating mode automatically. It checks which modes are
     * registered (an error mode will be set if no modes are registered) and checks if
     * exact number and type of source files are present. If so the found mode is set
     * to the current operating mode (please note that the source files do not need to
     * be processed yet, this method only checks if the source file handlers are
     * present.)
     * @roseuid 405AE2D901F2
     */
	public void setMode(){
		HashMap modes = getAllRegisteredModes();
		if(modes.size() == 0){
			mode = errorMode;
		} else {
			boolean modeSet = false;
			Iterator modeIterator = modes.keySet().iterator();
			while(modeIterator.hasNext() && !modeSet){
				String modeName = (String)modeIterator.next();
				HashSet modeFileIds = (HashSet)modes.get(modeName);
				HashSet handlers = getDLRLSourceFileHandlers();
				//there need to be the exact same number of source file handlers as there are
				//mode files defined for this mode
				if(modeFileIds.size()  == handlers.size()){
					boolean sourceFilesNotContained = false;
					Iterator handlerIterator = handlers.iterator();
					while(handlerIterator.hasNext() && !sourceFilesNotContained){
						SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
						if(!modeFileIds.contains(aHandler.getSourceFileIDName())){
							sourceFilesNotContained = true;
						}
					}
					if(!sourceFilesNotContained){
						mode = modeName;
						modeSet = true;
					}
				}
			}
			if(!modeSet){
				mode = errorMode;
			}
		}
	}

	/**
	* Sets the correct output directory path
	*
	* @param The specified output directory path
	*/
	public void setOutputDirectoryPath(java.io.File outputDirectoryPath){
		this.outputDirectoryPath = outputDirectoryPath;
	}

	/**
	* Gets the correct output directory path
	*/
	public java.io.File getOutputDirectoryPath(){
		return outputDirectoryPath;
	}

	public void setIncludePaths(java.util.Vector includePaths){
		this.includePaths = includePaths;
	}

	public java.util.Vector getIncludePaths(){
		return this.includePaths;
	}

    /**
     * Adds a template file to a specific template group set
     *
     * @param file The java.io.File to identify the actual template file
     * @param key The key to identify the template group this file belongs to
     * @roseuid 4064391A02F8
     */
	public void setTemplateSourceFile(java.io.File file, String key){
		if(templateGroups.get(key) !=null){
			((HashSet)templateGroups.get(key)).add(file);
		} else {
			HashSet newGroup = new HashSet();
			newGroup.add(file);
			templateGroups.put(key, newGroup);
		}
	}

    public void setVerbose(boolean verbose){
        this.verbose = verbose;
    }

    public boolean getVerbose(){
        return this.verbose;
    }

    public void setInvokeIdlPP(boolean invokeIdlPP){
        this.invokeIdlPP = invokeIdlPP;
    }

    public boolean getInvokeIdlPP(){
        return this.invokeIdlPP;
    }

    /**
     * Returns true if a source file has been processed. Contacts the
     * SourceFileHandler object to determine this (hasBeenProcessed()) method.
     *
     * @param fileId The file identifier to identify the correct source file handler.
     * @return boolean
     * @roseuid 406D380901E3
     */
	public boolean sourceFileHasBeenProcessed(String fileId) {
		Iterator handlerIterator =  getDLRLSourceFileHandlers().iterator();
		while(handlerIterator.hasNext()){
			SourceFileHandler aHandler = (SourceFileHandler)handlerIterator.next();
			if(aHandler.getSourceFileIDName().equals(fileId)){
				return aHandler.hasBeenProcessed();
			}
		}
		return false;
	}
}
