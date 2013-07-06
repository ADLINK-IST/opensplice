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
package org.opensplice.common.controller;

/**
 * Represents a result for assignment of a value to a certain field.
 * 
 * @date Nov 25, 2004
 */
public class AssignmentResult{
    private boolean valid;
    private String errorMessage;
    
    /**
     * Constructs a new AssignMentResult.
     * 
     * @param _success true if validation succeeded, false otherwise.
     * @param _errorMessage null when validation succeeded, the failure
     *                      reason otherwise.
     */
    public AssignmentResult(boolean _success, String _errorMessage){
        valid = _success;
        errorMessage = _errorMessage;
    }
    
    /**
     * Provides access to errorMessage.
     * 
     * @return Returns the errorMessage.
     */
    public String getErrorMessage() {
        return errorMessage;
    }
    /**
     * Provides access to valid.
     * 
     * @return Returns the valid.
     */
    public boolean isValid() {
        return valid;
    }
}
