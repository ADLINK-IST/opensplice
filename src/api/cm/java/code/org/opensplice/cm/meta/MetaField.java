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
package org.opensplice.cm.meta;

import java.util.ArrayList;

/**
 * Abstract base class for all fields in a Splice database type.
 * 
 * @date May 24, 2004
 */
public abstract class MetaField {
    /**
     * The name of the field.
     */
    protected String name;

    /**
     * The name of the type of the field.
     */
    protected String typeName;

    /**
     * Constructs new MetaField. This constructor is used by all subclasses of
     * MetaField.
     * 
     * @param _name
     *            The name of the field.
     * @param _typeName
     *            The name of the type of the field.
     */
    protected MetaField(String _name, String _typeName){
        name = _name;
        typeName = _typeName;
    }

    /**
     * Provides access to the name of the field.
     * 
     * @return The name of the field.
     */
    public String getName(){
        return name;
    }

    /**
     * Provides access to the name of the type of the field.
     * 
     * @return The name of the type of the field.
     */
    public String getTypeName(){
        return typeName;
    }

    /**
     * Creates a String representation of the field.
     * 
     * @return The String representation of the field.
     */
    @Override
    public String toString(){
        return typeName + " " + name;
    }

    /**
     * Provides access to all fields within this field.
     * 
     * @return The array of subfields.
     */
    public MetaField[] getFields(){
        return null;
    }

    /**
     * Provides access to the subfield with the specified name.
     * 
     * @param name
     *            The name of the field to look up.
     * @return The field associated with the supplied name or null if it cannot
     *         be found.
     */
    public MetaField getField(String fieldName){
        return null;
    }

    /**
     * Provides access to all fieldnames of subfields of this field.
     * 
     * @return The list of subfields of this field.
     */
    public ArrayList<String> getFieldNames() {
        ArrayList<String> result = new ArrayList<String>();
        result.add(name);
        return result;
    }
}
