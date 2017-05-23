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

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.type.TypeKind;


public abstract class DynamicTypeFactory implements DDSObject
{
    // -----------------------------------------------------------------------
    // Singleton Access
    // -----------------------------------------------------------------------

    /**
     * @param env       Identifies the Service instance to which the
     *                  object will belong.
     */
    public static DynamicTypeFactory getInstance(ServiceEnvironment env)
    {
        return env.getSPI().getTypeFactory();
    }



    // -----------------------------------------------------------------------
    // Instance Methods
    // -----------------------------------------------------------------------

    public abstract DynamicType getPrimitiveType(TypeKind kind);

    public abstract DynamicType createType(TypeDescriptor descriptor);
    public abstract DynamicType createStringType(int bound);
    public abstract DynamicType createWStringType(int bound);
    /**
     * Create unbounded sequence
     * @param elementType
     * @return A new sequence type for unbounded sequence.
     */
    public abstract DynamicType createSequenceType(
            DynamicType elementType);
    
    /**
     * Create bounded sequence
     * @param elementType
     * @param bound
     * @return A new sequence type for bounded sequence. 
     */
    public abstract DynamicType createSequenceType(
            DynamicType elementType, int bound);
    
    public abstract DynamicType createArrayType(
            DynamicType elementType, int... bound);
    
    public abstract DynamicType createMapType(
            DynamicType keyElementType, DynamicType elementType, int bound);
    
    public abstract DynamicType createBitSetType(int bound);

    /**
     * Load a type from the specified URI. If multiple types are defined
     * only the first one is returned.
     */
    public abstract DynamicType loadType(String documentUrl);
    public abstract java.util.Collection<DynamicType> loadTypes(String documentUrl);
    
    public abstract DynamicType loadType(String documentUrl, String name);
    public abstract java.util.Collection<DynamicType> loadTypes(String documentUrl, String... names);

    public abstract TypeDescriptor newTypeDescriptor();
    public abstract MemberDescriptor newMemberDescriptor();
    public abstract AnnotationDescriptor newAnnotationDescriptor();
}
