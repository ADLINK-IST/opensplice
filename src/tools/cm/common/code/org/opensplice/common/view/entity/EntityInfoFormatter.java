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
/**
 * Contains all SPLICE DDS C&M Tooling view components that are related to
 * C&M API
 */
package org.opensplice.common.view.entity;

import java.io.StringWriter;
import java.util.HashMap;

import org.opensplice.cm.meta.MetaType;

/**
 * Abstract base class that contains basic functionality for formatting 
 * Objects to be shown in a JEditorPane.
 * 
 * @date Jun 25, 2004
 */
public abstract class EntityInfoFormatter {
    /**
     * Constructs a new formatter.
     * 
     */
    public EntityInfoFormatter(){
        typeCache = new HashMap<MetaType, String>();
    }
    
    /**
     * Function to create a String representation of the supplied that can
     * be displayed in a JEditorPane. Currently plain text and HTML are 
     * supported.
     * 
     * @param o The object to format.
     * @return A String representation of the supplied object that can be displayed
     *         in a JEditorPane.
     */  
    public String getValue(Object o){
        return null;
    }
    
    /**
     *  Clears the complete cache of representations of MetaType objects.
     */
    public void clearCache(){
        typeCache.clear();
    }
    
    /**
     * Provides access to the current formatted object.
     * 
     * @return The current selected object.
     */
    public Object getCurrentObject(){
        return curObject;
    }
    
    /**
     * String representation for a tab in the formatter.
     */
    protected String tab;
    
    /**
     * String representation for a separator between lines in the formatter.
     */
    protected String separator;
    
    /**
     * String representation for a newline in the formatter.
     */
    protected String newLine;
    
    /**
     * Current value of the formatter.
     */
    protected String curValue;
    
    /**
     * Writer that is used to write the output to.
     */
    protected StringWriter writer;
    
    /**
     * The current object.
     */
    protected Object curObject;
    
    /**
     * Cache for type information. (<MetaType, String>)
     */
    protected HashMap<MetaType, String> typeCache;
}
