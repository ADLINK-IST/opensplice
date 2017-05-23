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

package org.omg.dds.topic;

import java.io.Serializable;

import org.omg.dds.core.DDSObject;
import org.omg.dds.type.Extensibility;
import org.omg.dds.type.Nested;


@Extensibility(Extensibility.Kind.EXTENSIBLE_EXTENSIBILITY)
@Nested
public interface BuiltinTopicKey extends Cloneable, Serializable, DDSObject
{
    /**
     * Get a copy of the value.
     */
    public int[] getValue();


    // -----------------------------------------------------------------------

    /**
     * Overwrite the state of this object with that of the given object.
     */
    public void copyFrom(BuiltinTopicKey src);


    // --- From Object: ------------------------------------------------------

    public BuiltinTopicKey clone();
}
