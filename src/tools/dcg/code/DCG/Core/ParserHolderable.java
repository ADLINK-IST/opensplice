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


/**
 * This interface defines the methods that must be implemented by every parser 
 * front-end that wants to be integrated into the DCG program
 */
public interface ParserHolderable {
    
    /**
     * This method must be implemented in each specific frontend parser to allow it to 
     * be compatible with the rest of the DCG program.
     * 
     * @param targetFile The target file to be parsed
     * @param targetValidationFile The optional validation file for the target file to 
     * be parsed. May be NULL
     * @return java.lang.Object
     * @roseuid 4062987C02E0
     */
    public Object parseFile(java.io.File targetFile, java.io.File targetValidationFile, java.util.Vector includePaths) throws Exception;
}
