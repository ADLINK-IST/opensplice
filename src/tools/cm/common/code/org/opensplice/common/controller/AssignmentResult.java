/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
