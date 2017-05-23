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
package org.opensplice.dds.sub;

/**
 * OpenSplice-specific extension of {@link org.omg.dds.sub.Sample} with support
 * for obtaining the key value in case the sample is marked as invalid. As
 * {@link org.omg.dds.sub.Sample#getData()} returns null in case the sample is
 * invalid, there is no reliable way to get the key value associated with it.
 * <p>
 * Typically one would be able to get the key value by means of
 * {@link org.omg.dds.sub.DataReader#getKeyValue(org.omg.dds.core.InstanceHandle)}
 * , but this may fail in case the corresponding instance has already been
 * removed from the DataReader.
 * <p>
 * Therefore OpenSplice ensures the key value is always maintained with the
 * Sample, even if it is invalid.
 * 
 * @param <TYPE>
 *            The concrete type of the data encapsulated by this Sample.
 */
public interface Sample<TYPE> extends org.omg.dds.sub.Sample<TYPE> {
    /**
     * @return The key value associated with the Sample. Only key attributes of
     *         the data can be accessed.
     */
    public TYPE getKeyValue();
}
