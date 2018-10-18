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
package org.opensplice.cm.status;

/**
 * Abstract base class for a communication status.
 * 
 * @date Oct 12, 2004 
 */
public abstract class Status {
    protected String state;
    
    /**
     * Creates a new Status from the supplied argument.
     *
     * @param _state The state of the Status.
     */
    public Status(String _state){
        state = _state;
    }
    
    /**
     * Provides access to the state.
     * 
     * @return The state.
     */
    public String getState(){
        return state;
    }
}
