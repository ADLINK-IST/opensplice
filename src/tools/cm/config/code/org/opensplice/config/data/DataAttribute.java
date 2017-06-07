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
package org.opensplice.config.data;

import org.w3c.dom.Attr;

import org.opensplice.config.meta.MetaAttribute;

public class DataAttribute extends DataNode {
    private DataValue value;
    
    public DataAttribute(MetaAttribute metadata, Attr node, Object value) throws DataException {
        super(metadata, node);
        
        if(!node.getNodeName().equals(metadata.getName())){
            throw new DataException("Metadata and data do not match.");
        }
        this.value = new DataValue(metadata.getValue(), node, value);
        this.value.setParent(this);
        this.value.setOwner(this.getOwner());
    }
    
    public DataAttribute(MetaAttribute metadata, Attr node) throws DataException {
        super(metadata, node);
        
        if(!node.getNodeName().equals(metadata.getName())){
            throw new DataException("Metadata and data do not match.");
        }
        Object defaultValue = metadata.getValue().getDefaultValue();
        this.value = new DataValue(metadata.getValue(), node, defaultValue);
        this.value.setParent(this);
        this.value.setOwner(this.getOwner());
    }
    
    @Override
    public void setOwner(DataConfiguration owner) {
        super.setOwner(owner);
        this.value.setOwner(owner);
    }
    
    public DataValue getDataValue(){
        return this.value;
    }
    
    public void testSetValue(Object value) throws DataException{
        this.value.testSetValue(value);
    }

    public void setValue(Object value) throws DataException{
        DataValue tmpValue = this.value;
        this.value.setValue(value);
        tmpValue.setParent(null);
        tmpValue.setOwner(null);
        this.value.setParent(this);
        this.value.setOwner(this.owner);
    }
    
    public Object getValue(){
        return this.value.getValue();
    }
}
