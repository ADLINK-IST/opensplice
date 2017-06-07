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

import java.util.HashSet;

import org.opensplice.config.meta.MetaConfiguration;
import org.opensplice.config.meta.MetaValue;
import org.opensplice.config.meta.MetaValueBoolean;
import org.opensplice.config.meta.MetaValueDouble;
import org.opensplice.config.meta.MetaValueEnum;
import org.opensplice.config.meta.MetaValueFloat;
import org.opensplice.config.meta.MetaValueInt;
import org.opensplice.config.meta.MetaValueLong;
import org.opensplice.config.meta.MetaValueNatural;
import org.opensplice.config.meta.MetaValueSize;
import org.opensplice.config.meta.MetaValueString;
import org.w3c.dom.Attr;
import org.w3c.dom.Text;

public class DataValue extends DataNode {
    private Object value;
    private HashSet<DataValue> DVdependencies = null;

    public DataValue(MetaValue metadata, Attr parent, Object value) throws DataException {
        super(metadata, parent);
        this.setValue(value);
    }

    public DataValue(MetaValue metadata, Text text) throws DataException {
        super(metadata, text);
        this.setValue(this.node.getNodeValue());
    }

    public Object getValue(){
        return this.value;
    }

    public void addDataValueDependency(DataValue dv) {
        if (DVdependencies == null) {
            DVdependencies = new HashSet<DataValue>();
        }
        DVdependencies.add(dv);
    }

    public HashSet<DataValue> getDataValueDependencies() {
        return DVdependencies;
    }

    public void testSetValue(Object value) throws DataException {
        try{
            if (!isEnvironmentVar(value)) {
                if(this.metadata instanceof MetaValueBoolean){
                    if(value instanceof String){
                        this.value = Boolean.parseBoolean((String)value);
                    } else if(!(value instanceof Boolean)){
                        throw new NumberFormatException();
                    }
                } else if(this.metadata instanceof MetaValueDouble){
                    Double tempValue;
                    MetaValueNatural mv = (MetaValueNatural)this.metadata;

                    if(value instanceof String){
                        tempValue = Double.parseDouble((String)value);
                    } else if(!(value instanceof Double)){
                        throw new NumberFormatException();
                    } else {
                        tempValue = (Double)value;
                    }
                    Object min = mv.getMinValue();
                    Object max = mv.getMaxValue();

                    if(tempValue.compareTo((Double)min) < 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + "<" + min);
                    } else if(tempValue.compareTo((Double)max) > 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + ">" + max);
                    }
                } else if(this.metadata instanceof MetaValueEnum){
                    if(value instanceof String){
                        String tmp;
                        String[] values = ((MetaValueEnum)this.metadata).getPosValues();
                        boolean valid   = false;

                        for(int i=0; i<values.length && !valid; i++){
                            tmp = values[i];

                            if(tmp.equalsIgnoreCase((String)value)){
                                valid = true;
                            }
                        }

                        if(!valid){
                            throw new DataException("Illegal Enum value found: " + value);
                        }
                    } else {
                        throw new NumberFormatException();
                    }
                } else if(this.metadata instanceof MetaValueFloat){
                    Float tempValue;
                    MetaValueNatural mv = (MetaValueNatural)this.metadata;

                    if(value instanceof String){
                        tempValue = Float.parseFloat((String)value);
                    } else if(!(value instanceof Float)){
                        throw new NumberFormatException();
                    } else {
                        tempValue = (Float)value;
                    }
                    Object min = mv.getMinValue();
                    Object max = mv.getMaxValue();

                    if(tempValue.compareTo((Float)min) < 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + " < " + min);
                    } else if(tempValue.compareTo((Float)max) > 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + " > " + max);
                    }
                } else if(this.metadata instanceof MetaValueInt){
                    Integer tempValue;
                    MetaValueNatural mv = (MetaValueNatural)this.metadata;

                    if(value instanceof String){
                        tempValue = Integer.parseInt((String)value);
                    } else if(!(value instanceof Integer)){
                        throw new NumberFormatException();
                    } else {
                        tempValue = (Integer)value;
                    }
                    Object min = mv.getMinValue();
                    Object max = mv.getMaxValue();

                    if(tempValue.compareTo((Integer)min) < 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + "<" + min);
                    } else if(tempValue.compareTo((Integer)max) > 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + ">" + max);
                    }
                } else if(this.metadata instanceof MetaValueLong){
                    Long tempValue;
                    MetaValueNatural mv = (MetaValueNatural)this.metadata;

                    if(value instanceof String){
                        tempValue = Long.parseLong((String)value);
                    } else if(!(value instanceof Long)){
                        throw new NumberFormatException();
                    } else {
                        tempValue = (Long)value;
                    }
                    Object min = mv.getMinValue();
                    Object max = mv.getMaxValue();

                    if(tempValue.compareTo((Long)min) < 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + "<" + min);
                    } else if(tempValue.compareTo((Long)max) > 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + ">" + max);
                    }
                } else if(this.metadata instanceof MetaValueSize){
                    Long tempValue;
                    MetaValueNatural mv = (MetaValueNatural)this.metadata;

                    if(value instanceof String){
                        tempValue = MetaConfiguration.createLongValuefromSizeValue((String)value);
                    } else if(!(value instanceof Long)){
                        throw new NumberFormatException();
                    } else {
                        tempValue = (Long)value;
                    }
                    Object min = mv.getMinValue();
                    Object max = mv.getMaxValue();

                    if(tempValue.compareTo((Long)min) < 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + "<" + min);
                    } else if(tempValue.compareTo((Long)max) > 0){
                        throw new DataException("<" + this.node.getParentNode().getNodeName() + "> Value: " + tempValue
                                + ">" + max);
                    }
                } else if(this.metadata instanceof MetaValueString){
                    if(value instanceof String){
                        String strValue = (String)value;

                        if(strValue.startsWith("${") && strValue.endsWith("}")){
                            /*Environment variable, this is ok*/
                        } else {
                            int length = strValue.length();
                            int maxLength = ((MetaValueString)this.metadata).getMaxLength();

                            if((maxLength != 0) && (length > maxLength)){
                                throw new DataException("<" + this.node.getParentNode().getNodeName() + "> String length: "
                                        + length + ">" + maxLength);
                            }
                        }
                        /*
                         * check if we got a domain service name element of service
                         * tag name element
                         */
                        if (this.getOwner() != null && this.getOwner().getServiceNames().contains(this)) {
                            if (!this.getValue().equals(value)) {
                                for (DataValue dv : this.getOwner().getServiceNames()) {
                                    if (dv.getValue().equals(value)) {
                                        throw new DataException("Name [" + value + "] is already used by another service");
                                    }
                                }
                            }
                        }
                    } else if(value == null){
                        throw new DataException("Null pointer string");
                    } else {
                        throw new NumberFormatException();
                    }
                } else {
                    throw new DataException("Found unknown metadata." +
                            this.metadata.getClass().toString().substring(
                                this.metadata.getClass().toString().lastIndexOf(
                                        '.') + 10));
                }
            }
        } catch(NumberFormatException nfe){
            if(value == null){
                throw new DataException("Found null pointer for data.");
            }
            throw new DataException("Expected '"
                    + this.metadata
                            .getClass()
                            .toString()
                            .substring(
                                    this.metadata.getClass().toString()
                                            .lastIndexOf('.') + 10)
                    + "', but found '"
                    + value.getClass()
                            .toString()
                            .substring(
                                    value.getClass().toString()
                                            .lastIndexOf('.') + 1) + "'.");

        }
    }

    public boolean isEnvironmentVar(Object value) {
        boolean result = false;
        if (value instanceof String && ((String) value).startsWith("${") && ((String) value).endsWith("}")) {
            result = true;
        }
        return result;
    }

    public void setValue(Object value) throws DataException {
        this.testSetValue(value);

        this.value = value;
        /*testSetValue succeeded*/
        if(this.node instanceof Attr){
            ((Attr)this.node).setValue(this.value.toString());
        } else if(this.node instanceof Text){
            ((Text)this.node).replaceWholeText(this.value.toString());
        }

        if (DVdependencies != null) {
            for (DataValue dv : DVdependencies) {
                Object oldValue = dv.getValue();
                dv.value = value;
                /* testSetValue succeeded */
                if (dv.node instanceof Attr) {
                    ((Attr) dv.node).setValue(dv.value.toString());
                } else if (dv.node instanceof Text) {
                    ((Text) dv.node).replaceWholeText(dv.value.toString());
                }
                dv.getOwner().notifyValueChanged(dv, oldValue, value);
            }
        }

    }

    public void resetValue(){
        try {
            this.setValue(((MetaValue)this.metadata).getDefaultValue());
        } catch (DataException e) {
            assert false: "Default value cannot be applied.";
        }
    }
}
