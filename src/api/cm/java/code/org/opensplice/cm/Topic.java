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
package org.opensplice.cm;

import org.opensplice.cm.meta.MetaType;

/**
 * Represents a Topic in SPLICE-DDS.
 */
public interface Topic extends Entity {
    /**
     * Provides access to the type name of the topic.
     * 
     * @return The type name of the topic.
     */
    public String getTypeName();

    /**
     * Provides access to the key list.
     *
     * @return Comma separated list of keys.
     */
    public String getKeyList();
    
    /**
     * Resolves the data type of the topic.
     * 
     * @return The dataType of the topic.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Topic is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported by this API.
     */
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException;
}
