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

import java.io.File;

/**
 * This object is responsible for the correctly processing a source file. It 
 * offers functionality for complete processing of a file and allows objects to 
 * access the used parser, syntax tree, file identification string and other 
 * relevant information.
 */
public class SourceFileHandler {
    
    /**
     * Indicates if the source file is processed.
     */
    private boolean isProcessed = false;
    
    /**
     * Contains the actual source file
     */
    private java.io.File sourceFile;
    
    /**
     * The String identification of the source file handled by this source file handler
     */
    private String sourceFileIDName;
    
    /**
     * The actual source validation file
     */
    private java.io.File validationFile;
    
    /**
     * The identifier for the validation file used by this source file handler
     */
    private String validationFileIDName;

	private AbstractSyntaxTreeHolder astHolder;
	private ParserHolderable parserHolder;
    
    /**
     * The (specialized) constructor
     * 
     * @param parser The parser used by this source file handler
     * @param sourceFileKey The identifier of the source file managed by this source 
     * file handler
     * @param sourceFileValidationFileKey The identifier of the source validation file
     * @param sourceFileIDName
     * @param validationFileIDName
     * @roseuid 4067DCE90224
     */
	public SourceFileHandler(ParserHolderable parser, String sourceFileIDName, String validationFileIDName){
		this.parserHolder = parser;
		this.astHolder = new AbstractSyntaxTreeHolder();
		this.sourceFileIDName = sourceFileIDName;
		this.validationFileIDName = validationFileIDName;
	}
    
    /**
     * Returns true if this handler contains the source file for the specified source 
     * file identifier
     * 
     * @param fileID The file identifier for the source file
     * @return boolean
     * @roseuid 406C26F302E0
     */
	public boolean containsSourceFileForFileID(String fileID) {
		if(fileID.equals(sourceFileIDName) && sourceFile !=null){
			return true;
		} else if (fileID.equals(validationFileIDName) && validationFile !=null){
			return true;
		} else {
			return false;
		}
	}
    
    /**
     * Returns the abstract syntax tree associated with this source file handler
     * @return DCG.Core.AbstractSyntaxTreeHolder
     * @roseuid 40692E9602A9
     */
	public AbstractSyntaxTreeHolder getASTHolder(){
		return astHolder;
	}
    
    /**
     * Returns the parser associated with this source file handler
     * @return DCG.Core.ParserHolderable
     * @roseuid 40692E700009
     */
	public ParserHolderable getParserHolder(){
		return parserHolder;
	}
    
    /**
     * Returns the source file handled by this source file handler
     * @return java.io.File
     * @roseuid 40692C670158
     */
	public java.io.File getSourceFile(){
		return sourceFile;
	}
    
    /**
     * Returns the source file identifier name
     * @return java.lang.String
     * @roseuid 40683493005C
     */
	public String getSourceFileIDName(){
		return sourceFileIDName;
	}

    /**
     * Returns the source validation file handled by this source file handler
     * @return java.io.File
     * @roseuid 406D66A90320
     */
	public java.io.File getSourceValidationFile(){
		return validationFile;
	}
    
    /**
     * Returns the source validation file identifier name
     * @return java.lang.String
     * @roseuid 406BD3EC02D6
     */
	public String getSourceValidationFileIDName(){
		return validationFileIDName;
	}
    
    /**
     * Returns true if this source file handler has already successfully processed the 
     * source file, false if not
     * @return boolean
     * @roseuid 406D32E7039C
     */
	public boolean hasBeenProcessed(){
		return isProcessed;
	}
    
    /**
     * Responsible for the correct processing of the source file managed by this 
     * source file handler. Will set the handler isProcessed attribute to true if the 
     * file has been processed successfully
     * @roseuid 40692CBE0330
     */
	public void processSourceFile(java.util.Vector includePaths) throws Exception{
		astHolder.setRoot(parserHolder.parseFile(sourceFile, validationFile, includePaths));
		setProcessed(true);	
	}
    
    /**
     * Sets the source or source validation file depending on the file identifier 
     * provided to the specified value. Will do nothing if the file identifier isn't 
     * found in this handler
     * 
     * @param fileID The file identifier string
     * @param file The source or source validation file to be set
     * @roseuid 406BD2A70360
     */
	public void setFileForFileID(String fileID, java.io.File file){
		if(fileID.equals(sourceFileIDName)){
			sourceFile = file;
		} else if (fileID.equals(validationFileIDName)){
			validationFile = file;
		} 
	}
    
    /**
     * Sets the processed status of the source file handler
     * 
     * @param isProcessed The boolean value to be set (true if the source file handler 
     * has processed it's file successfully, false if not)
     * @roseuid 406D33090075
     */
	public void setProcessed(boolean isProcessed){
		this.isProcessed = isProcessed;
	}
}
