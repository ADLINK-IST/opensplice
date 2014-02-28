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
 * Represents a structure field in a type.
 * 
 * @date Jun 14, 2004
 */
public class MetaStruct extends MetaField{
    /**
     * The list of members of the structure.(<MetaField>)
     */
    private final ArrayList<MetaField> members;

    /**
     * Creates a new structure field.
     * 
     * @param name
     *            The name of the structure.
     * @param typeName
     *            The type name of the structure.
     * @param _members
     *            The members of the structure.
     */
    public MetaStruct(String name, String typeName, ArrayList<MetaField> _members) {
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
        MetaField result = null;

        for (int i = 0; i < members.size(); i++) {
            result = (members.get(i));
            if (result instanceof MetaUnion) {
                if (fieldName.equals(result.getName())) {
                    return result;
                }
                result = result.getField(fieldName);
                if (result != null) {
                    return result;
                }
            } else if (fieldName.equals(result.getName())) {
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
