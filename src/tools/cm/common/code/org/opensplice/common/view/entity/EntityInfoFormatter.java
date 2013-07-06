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
