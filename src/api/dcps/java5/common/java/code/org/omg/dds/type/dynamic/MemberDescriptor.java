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
import java.util.List;

import org.omg.dds.core.DDSObject;


/**
 * Objects of this type are immutable.
 */
public interface MemberDescriptor extends Serializable, DDSObject
{
    public abstract boolean isConsistent();

    /**
     * @return the name
     */
    public String getName();

    /**
     * @return the id
     */
    public int getId();

    /**
     * @return the type
     */
    public DynamicType getType();

    /**
     * @return the defaultValue
     */
    public String getDefaultValue();

    /**
     * @return the index
     */
    public int getIndex();

    /**
     * @return the label
     */
    public List<Integer> getLabel();

    /**
     * @return the defaultLabel
     */
    public boolean isDefaultLabel();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this descriptor and apply the given name.
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withName(String name);

    /**
     * Copy this descriptor and apply the given ID.
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withId(int id);

    /**
     * Copy this descriptor and apply the given type.
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withType(DynamicType type);

    /**
     * Copy this descriptor and apply the given default value.
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withDefaultValue(String defaultValue);

    /**
     * Copy this descriptor and apply the given index.
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withIndex(int index);

    /**
     * Copy this descriptor and apply the given label(s).
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withLabel(int... label);

    /**
     * Copy this descriptor and apply the given default label.
     * 
     * @return  a new descriptor
     */
    public MemberDescriptor withDefaultLabel(boolean defaultLabel);
}
