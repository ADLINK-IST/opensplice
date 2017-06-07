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
import org.omg.dds.type.Optional;
import org.omg.dds.type.TypeKind;


/**
 * Objects of this type are immutable.
 */
public interface TypeDescriptor extends Serializable, DDSObject
{
    public boolean isConsistent();

    /**
     * @return the kind
     */
    public TypeKind getKind();

    /**
     * @return the name
     */
    public String getName();

    /**
     * @return the baseType
     */
    public DynamicType getBaseType();

    /**
     * @return the discriminatorType
     */
    public DynamicType getDiscriminatorType();

    /**
     * @return the bound, an unmodifiable list
     */
    public List<Integer> getBound();

    /**
     * @return the elementType
     */
    @Optional
    public DynamicType getElementType();

    /**
     * @return the keyElementType
     */
    @Optional
    public DynamicType getKeyElementType();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this descriptor and apply the given kind.
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withKind(TypeKind kind);

    /**
     * Copy this descriptor and apply the given name.
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withName(String name);

    /**
     * Copy this descriptor and apply the given base type.
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withBaseType(DynamicType baseType);

    /**
     * Copy this descriptor and apply the given discriminator type.
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withDiscriminatorType(
            DynamicType discriminatorType);

    /**
     * Copy this descriptor and apply the given bound(s).
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withBound(int... bound);

    /**
     * Copy this descriptor and apply the given element type.
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withElementType(DynamicType elementType);

    /**
     * Copy this descriptor and apply the given key element type.
     * 
     * @return  a new descriptor
     */
    public TypeDescriptor withKeyElementType(DynamicType keyElementType);
}
