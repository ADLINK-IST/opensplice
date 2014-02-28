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
 * Supplies information about metadata of objects in the Splice database.
 */
package org.opensplice.cm.meta;

import java.util.ArrayList;

/**
 * Represents a class field in a Splice database type.
 * 
 * @date Jun 14, 2004
 */
public class MetaClass extends MetaField{
    /**
     * The list of all members of this class.
     */
    private final ArrayList<MetaField> members;

    /**
     * Constructs a new class
     * 
     * @param name
     *            The name of the class.
     * @param typeName
     *            The type name of the class.
     * @param _members
     *            The members of the class.
     */
    public MetaClass(String name, String typeName, ArrayList<MetaField> _members) {
        super(name, typeName);
        members = _members;
    }

    @Override
    public MetaField[] getFields(){
        MetaField[] result = new MetaField[members.size()];

        for(int i=0; i<members.size(); i++){
            result[i] = (members.get(i));
        }
        return result;
    }

    @Override
    public String toString(){
        StringBuffer buf = new StringBuffer();
        buf.append(super.toString());
        for (int i = 0; i < members.size(); i++) {
            buf.append("\n" + members.get(i).toString());
        }
        return buf.toString();
    }

    @Override
    public MetaField getField(String fieldName){
        MetaField result;

        for(int i=0; i<members.size(); i++){
            result = (members.get(i));

            if(fieldName.equals(result.getName())){
                return result;
            }
        }
        return null;
    }

    @Override
    public ArrayList<String> getFieldNames() {
        MetaField field;
        ArrayList<String> result = new ArrayList<String>();
        ArrayList<String> names;

        for(int i=0; i<members.size(); i++){
            field = (members.get(i));

            names = field.getFieldNames();

            for(int j=0; j<names.size(); j++){
                result.add(name + "." + (names.get(j)));
            }
        }
        return result;
    }
}
