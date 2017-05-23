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

import java.util.List;
import java.util.Map;

import org.omg.dds.core.DDSObject;
import org.omg.dds.type.TypeKind;


public interface DynamicType extends DDSObject, Cloneable
{
    public TypeDescriptor getDescriptor();

    public String getName();

    public TypeKind getKind();

    public DynamicTypeMember getMember(int id);

    public DynamicTypeMember getMember(String name);

    public Map<Integer, DynamicTypeMember> getAllMembers();

    public Map<Integer, DynamicTypeMember> getAllMembers(int... id);

    public Map<String, DynamicTypeMember> getAllMembers(String... id);

    public Map<Integer, AnnotationDescriptor> getAllAnnotations();

    public AnnotationDescriptor getAnnotation(int id);

    public AnnotationDescriptor getAnnotation(String name);

    public Map<Integer, AnnotationDescriptor> getAnnotations(int... id);

    public Map<String, AnnotationDescriptor> getAnnotations(String... name);

    public void setAnnotation(AnnotationDescriptor descriptor);

    public void setAnnotations(AnnotationDescriptor... descriptors);

    public DynamicTypeMember addMember(MemberDescriptor descriptor);

    public List<DynamicTypeMember> addMembers(MemberDescriptor... descriptor);

    public DynamicType clone();
}
