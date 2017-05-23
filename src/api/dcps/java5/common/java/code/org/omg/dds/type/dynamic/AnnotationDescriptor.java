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

import java.io.Serializable;
import java.util.Map;

import org.omg.dds.core.DDSObject;


/**
 * Objects of this type are immutable.
 */
public interface AnnotationDescriptor extends Serializable, DDSObject
{
    public String getValue(String key);

    public Map<String, String> getAllValue();

    public boolean isConsistent();

    /**
     * @return the type
     */
    public DynamicType getType();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this descriptor and apply the given value.
     * 
     * @return  a new descriptor
     */
    public AnnotationDescriptor withValue(String key, String value);

    /**
     * Copy this descriptor and apply the given type.
     * 
     * @return  a new descriptor
     */
    public AnnotationDescriptor withType(DynamicType type);
}
