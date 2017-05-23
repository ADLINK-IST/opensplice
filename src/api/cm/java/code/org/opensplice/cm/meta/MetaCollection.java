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


/**
 * Represents a collection field in a Splice database type.
 *
 * @date May 24, 2004
 */
public class MetaCollection extends MetaField{
    /**
     * The maximum size of the collection.
     */
    private final int maxSize;

    /**
     * The subtype of the collection.
     */
    private final MetaField subType;

    /**
     * Constructs a new collection type.
     *
     * @param _name
     *            The name of the collection.
     * @param _typeName
     *            The type name of the collection.
     * @param _maxSize
     *            The maximum size of the collection.
     * @param _subType
     *            The subtype within the collection.
     */
    public MetaCollection(String _name, String _typeName, int _maxSize, MetaField _subType) {
        super(_name, _typeName);
        maxSize = _maxSize;
        subType = _subType;

    }

    /**
     * Provides access to the maximum size of the collection.
     *
     * @return The maximum size of the collection.
     */
    public int getMaxSize(){
        return maxSize;
    }

    /**
     * Provides access to the size of a collection within this collection. This
     * can be used if the subtype of this collection also is a collection.
     *
     * @param depth
     *            The recursive depth of the collection (0 is this collection)
     * @return The maximum size of the collection associated with the supplied
     *         depth. If the depth is greater then the real depth, -1 is
     *         returned.
     */
    public int getRecursiveMaxSize(int depth){
        int recSize = maxSize;
        MetaField curSubType = this;

        if(depth == 0){
            return maxSize;
        }

        for(int i=0; i<depth; i++){
            if(curSubType instanceof MetaCollection){
                curSubType = ((MetaCollection)curSubType).getSubType();
            }
            else{
                return -1;
            }
        }

        if(curSubType instanceof MetaCollection){
            recSize = ((MetaCollection)curSubType).getMaxSize();
        }
        return recSize;
    }

    @Override
    public String toString(){
        String s = super.toString();

        return s;
    }

    @Override
    public MetaField getField(String fieldName){
        MetaField result;

        if(subType instanceof MetaStruct){
            result = subType.getField(fieldName);
        } else if (subType instanceof MetaUnion) {
            result = subType.getField(fieldName);
        } else if (subType instanceof MetaCollection) {
            result = ((MetaCollection) subType).getSubType().getField(fieldName);
        } else {
            result = null;
        }
        return result;
    }

    /**
     * Provides access to the subtype of the collection.
     *
     * @return The subtype of the collection.
     */
    public MetaField getSubType() {
        return subType;
    }
}
