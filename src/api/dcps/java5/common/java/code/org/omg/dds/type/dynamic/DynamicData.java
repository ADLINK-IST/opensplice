/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.type.dynamic;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.List;

import org.omg.dds.core.DDSObject;


public interface DynamicData extends DDSObject, Cloneable
{
    public DynamicType getType();

    /**
     * @return  an unmodifiable list.
     */
    public List<MemberDescriptor> getDescriptors();

    public int getMemberIdByName(String name);
    public int getMemberIdAtIndex(int index);


    public void clearAllValues();
    public void clearNonkeyValues();
    public void clearValue(int id);
    public void clearValue(String name);
    
    public <T> void setValue (int id, T value, Class<T> type);
    public <T> void setValue (String name, T value, Class<T> type);
    
    public <T> T getValue (int id, Class<T> type);
    public <T> T getValue (String name, Class<T> type);

    public DynamicData loanValue(int id);
    public void returnLoanedValue(DynamicData value);


    public int getInt32Value(int id);
    /**
     * @return  this
     */
    public DynamicData setInt32Value(int id, int value);

    public short getInt16Value(int id);
    /**
     * @return  this
     */
    public DynamicData setInt16Value(int id, short value);

    public long getInt64Value(int id);
    /**
     * @return  this
     */
    public DynamicData setInt64Value(int id, long value);

    public BigInteger getBigIntegerValue(int id);
    /**
     * @return  this
     */
    public DynamicData setBigIntegerValue(int id, BigInteger value);

    public float getFloat32Value(int id);
    /**
     * @return  this
     */
    public DynamicData setFloat32Value(int id, float value);

    public double getFloat64Value(int id);
    /**
     * @return  this
     */
    public DynamicData setFloat64Value(int id, double value);

    public BigDecimal getBigDecimalValue(int id);
    /**
     * @return  this
     */
    public DynamicData setBigDecimalValue(int id, BigDecimal value);

    public char getCharValue(int id);
    /**
     * @return  this
     */
    public DynamicData setCharValue(int id, char value);

    public byte getByteValue(int id);
    /**
     * @return  this
     */
    public DynamicData setByteValue(int id, byte value);

    public boolean getBooleanValue(int id);
    /**
     * @return  this
     */
    public DynamicData setBooleanValue(int id, boolean value);

    public String getStringValue(int id);
    /**
     * @return  this
     */
    public DynamicData setStringValue(int id, CharSequence value);

    public DynamicData getComplexValue(int id);
    public DynamicData getComplexValue(String name);

    /**
     * @return  this
     */
    public DynamicData setComplexValue(int id, DynamicData value);


    public int getInt32Values(
            int[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setInt32Values(
            int id, int[] value, int offset, int length);

    public int getInt16Values(
            short[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setInt16Values(
            int id, short[] value, int offset, int length);

    public int getInt64Values(
            long[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setInt64Values(
            int id, long[] value, int offset, int length);

    public List<BigInteger> getBigIntegerValues(
            List<BigInteger> value, int id);
    /**
     * @return  this
     */
    public DynamicData setBigIntegerValues(int id, List<BigInteger> value);

    public int getFloat32Values(
            float[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setFloat32Values(
            int id, float[] value, int offset, int length);

    public int getFloat64Values(
            double[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setFloat64Values(
            int id, double[] value, int offset, int length);

    public List<BigDecimal> getBigDecimalValues(
            List<BigDecimal> value, int id);

    /**
     * @return  this
     */
    public DynamicData setBigDecimalValues(int id, List<BigDecimal> value);

    public int getCharValues(
            char[] value, int offset, int length, int id);
    public StringBuilder getCharValues(StringBuilder value, int id);
    /**
     * @return  this
     */
    public DynamicData setCharValues(
            int id, char[] value, int offset, int length);
    /**
     * @return  this
     */
    public DynamicData setCharValues(int id, CharSequence value);

    public int getByteValues(
            byte[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setByteValues(
            int id, byte[] value, int offset, int length);

    public int getBooleanValues(
            boolean[] value, int offset, int length, int id);
    /**
     * @return  this
     */
    public DynamicData setBooleanValues(
            int id, boolean[] value, int offset, int length);

    public void getStringValues(List<String> value, int id);
    /**
     * @return  this
     */
    public DynamicData setStringValues(
            int id, String[] value, int offset, int length);
    /**
     * @return  this
     */
    public DynamicData setStringValues(int id, List<String> value);


    public DynamicData clone();
}
