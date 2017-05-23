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

package org.omg.dds.core.policy;

import java.util.List;


public interface DataRepresentation
extends QosPolicy.ForTopic,
        QosPolicy.ForDataReader,
        QosPolicy.ForDataWriter,
        RequestedOffered<DataRepresentation>
{
    // -----------------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------------

    public List<Short> getValue();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     *
     * @return  a new policy
     */
    public DataRepresentation withValue(List<Short> value);

    /**
     * Copy this policy and override the value of the property.
     *
     * @return  a new policy
     */
    public DataRepresentation withValue(short... value);



    // -----------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------

    public static final class Id
    {
        public static final short XCDR_DATA_REPRESENTATION = 0;
        public static final short XML_DATA_REPRESENTATION = 1;
    }
}
