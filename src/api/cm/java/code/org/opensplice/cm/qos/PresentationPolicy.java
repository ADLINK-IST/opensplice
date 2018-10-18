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
package org.opensplice.cm.qos;

/**
 * Specifies how the samples representing changes to data instances are 
 * presented to the subscribing application. This policy affects the 
 * application's ability to:
 * - Specify and receive coherent changes
 * - See the relative order of changes.
 * 
 * @date Jan 10, 2005 
 */
public class PresentationPolicy {
    /**
     * The kind of presentation. It determines the largest
     * scope spanning the entities for which the order and coherency of changes 
     * can be preserved. Default value is INSTANCE.
     */
    public PresentationKind access_scope;
    
    /**
     * Controls whether coherent access is supported within the access_scope.
     */
    public boolean coherent_access;
    
    /**
     * Controls whether ordered access is supported within the access_scope.
     */
    public boolean ordered_access;
    
    public static final PresentationPolicy DEFAULT = new PresentationPolicy(PresentationKind.INSTANCE, false, false);
    
    /**
     * Constructs a new PresentationPolicy
     *  
     *
     * @param _access_scope The access scope for the presentation.
     * @param _coherent_access Determines whether coherent access is supported.
     * @param _ordered_access Determines whether ordered access is supported.
     */
    public PresentationPolicy(
            PresentationKind _access_scope, 
            boolean _coherent_access, 
            boolean _ordered_access)
    {
        access_scope = _access_scope;
        coherent_access = _coherent_access;
        ordered_access = _ordered_access;
    }
    
    public PresentationPolicy copy(){
        return new PresentationPolicy(
                this.access_scope,
                this.coherent_access,
                this.ordered_access);
    }
}
