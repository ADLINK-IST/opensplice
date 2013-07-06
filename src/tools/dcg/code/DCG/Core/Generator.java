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
 * The abstract Generator class is used as a base for every Generator backend. It 
 * defines and implements basic functionality common to every generator backend 
 * and will allow simple integration of new generator backends into the DCG 
 * program. 
 * 
 * A new generator simply needs extend the Generator class and overwrite the 
 * executeGeneration(...) method call. All other components will then be able to 
 * work successfully with the new Generator.
 */
public abstract class Generator {
    
    /**
     * The main model which manages this generator. The generator needs to be able to 
     * communicate with the MainModel to retrieve needed information.
     */
    private MainModel model;
    
    /**
     * A HashMap containing as key the source file identifiers (String objects) and as 
     * value a boolean to indicate if the source file is a required source file (true) 
     * or an optional source file (false)
     * The source file identifiers do not need to be a subset of the source files 
     * managed by the MainModel. However if a required source file is not present (or 
     * not processed yet) in the MainModel the Generator will abort generation
     */
    private HashMap sourceFileIds = new HashMap();
    
    /**
     * A HashSet containing the modes (String objects) that are supported by this 
     * generator. These modes are a subset of the modes registered at the MainModel
     */
    private HashSet supportedModes = new HashSet();
    
    /**
     * A HashSet containing template group identifiers. A generator can be registered 
     * to any number of template groups. The template group identifier indicates which 
     * kind of template files this generator is interested in. For example a generator 
     * for c++ output files wishes to use XSLT templates to generate the output code, 
     * while a generator for a DCPS IDL file doesn't want to use any templates, or a 
     * completely different set of templates.
     * 
     * This allows the DCG to work with various backends which all have their own 
     * template demands.
     */
    private HashSet templateGroupIds = new HashSet();
    
    /**
     * The (specialized) constructor of the Generator
     * 
     * @param model The main model which manages this generator
     * @param  sourceFileIds A HashMap containing the source files
     * identifiers and a value to represent if the source file is required or not.
     * 
     * Key = File ID (String)
     * Value = required (Boolean) (true if required, false if optional)
     * 
     * @param templateGroupIds A HashSet containing the identifiers of the template 
     * groups (String objects)
     * @roseuid 4067E9B202F3
     */
	public Generator(MainModel model, HashMap sourceFileIds, HashSet templateGroupIds){
		this.model = model;
		this.sourceFileIds = sourceFileIds;
		this.templateGroupIds = templateGroupIds;
	}
    
    /**
     * Checks if this generator can support the mode. A generator can support a mode 
     * if all of the required source files for this generator are present if the mode 
     * is selected. The MainModel contains the relevant information about which files 
     * will be present if a mode is selected.
     * 
     * @param modeID The mode to be checked if it can be supported
     * @return boolean
     * @roseuid 406C18E20010
     */
	public boolean canSetSupportedMode(String modeID){
		HashSet modeFileIds = model.getFileIdsForMode(modeID);
		//cant set the mode, if the mode isnt known within the model
		if(modeFileIds == null){
			return false;
		} else {
			//check if the required source file ids is a subset of the mode file ids
			HashMap theSourceFileIds = getSourceFileIds();
			Iterator fileIdIterator = theSourceFileIds.keySet().iterator();
			while(fileIdIterator.hasNext()){
				String aFileId = (String)fileIdIterator.next();
				boolean required = ((Boolean)theSourceFileIds.get(aFileId)).booleanValue();
				//if the source file is required but the source file isnt contained within the
				//set of mode file ids then return false. Because the mode can not be supported
				//if a required file isnt present in the main model if that mode is set 
				if(required && !modeFileIds.contains(aFileId)){
					return false;
				}
			}
		}
		return true;
	}
    
    /**
     * Takes care of the actual generation process. Must be implemented by every 
     * specific generator which extends the Generator class
     * 
     * @param abstractSyntaxTrees A HashMap containing the syntax trees.
     * Key : The source file id to identify to which file this syntax tree belongs
     * value: the syntax tree itself
     * @param templateGroups A HashMap containing all groups of template files.
     * Key: identifier for the template group (String object)
     * value: The template files (java.io.File objects)
     * @roseuid 406D1C7D014C
     */
    public abstract void executeGeneration(HashMap abstractSyntaxTrees, HashMap templateGroups) throws Exception;
    
    /**
     * Returns the HashMap containing the source file identifiers
     * @return java.util.HashMap
     * @roseuid 406C196400C0
     */
	public HashMap getSourceFileIds(){
		return sourceFileIds;
	}

	/**
	 * Returns the main model used
	 */
	public MainModel getModel(){
		return model;
	}
    
    /**
     * Returns a HashSet of all supported modes of this generator
     * @return java.util.HashSet
     * @roseuid 406D1B7B003C
     */
	public HashSet getSupportedModes(){
		return supportedModes;
	}
    
    /**
     * Adds the mode to the list of supported modes
     * 
     * @param modeName The mode to be supported
     * @roseuid 406C19130062
     */
	public void setSupportedMode(String modeName){
		supportedModes.add(modeName);
	}
    
    /**
     * The main method called to start the generation process. It takes care of all 
     * preprocessing. 
     * First it will look up all needed abstract syntax trees for each source file 
     * (required or optional). If optional files are not present or have not yet been 
     * processed then they are ignored. If required files are not present or have not 
     * yet been processed then the generation process exits.
     * Secondly the template files are located based upon the template group key that 
     * has been registered to the generator. If the template group is not present or 
     * no files are defined for the template group, then this is ignored at this point 
     * (every implementation of a generator needs to deal with this in the 
     * executeGeneration method).
     * Finally the executegeneration method is called, with the following arguments:
     * 1) the AST's needed in a map (key value is the file id the AST belongs too, 
     * value is the AST itself) 
     * 2) a map containing the template files (key value is the template group id, and 
     * value is a HashSet containing the template files (java.io.File)
     * @roseuid 40629CF800F5
     */
	public void startGeneration() throws Exception{
		HashMap astHolders = new HashMap();
		HashMap templateGroups = new HashMap();
		boolean ableToContinue = true;
		Iterator sourceFilesIterator = sourceFileIds.keySet().iterator();
		while(sourceFilesIterator.hasNext() && ableToContinue){
			String aFileId = (String)sourceFilesIterator.next();
			boolean required = ((Boolean)sourceFileIds.get(aFileId)).booleanValue();
			boolean processed = model.sourceFileHasBeenProcessed(aFileId);
			if(processed){
				//get ast holders for all the source files it is interested in
				AbstractSyntaxTreeHolder holder = model.getASTreeForSourceFileID(aFileId);
				astHolders.put(aFileId, holder);
			} else if(required){
				ableToContinue = false;
			}
		}
		Iterator templateIterator = templateGroupIds.iterator();
		while(templateIterator.hasNext()){
			String templateGroupKey = (String)templateIterator.next();
			HashSet templateFiles = model.getTemplateFilesForGroup(templateGroupKey);
			templateGroups.put(templateGroupKey, templateFiles);
		}
		if(!ableToContinue){
			throw new Exception("Generator "+this.getClass().getName()+" is unable to generate ouput. Required files have not been processed. Minimal generation requirements have not been met");
		} else {
			//start the actual generation
			executeGeneration(astHolders, templateGroups);
		}
	}
}
