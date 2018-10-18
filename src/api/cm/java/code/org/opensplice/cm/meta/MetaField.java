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
