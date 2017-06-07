/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
