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

package org.omg.dds.type;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;


@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target(value = {
        // "Enum" type should be considered a bit set type:
        ElementType.TYPE,
        // Object of EnumSet or BitSet type should be serialized as bit set:
        ElementType.FIELD,  // literal field
        ElementType.METHOD  // Java Bean property
    })
public @interface BitSet
{
    /**
     * When this annotation annotates an enum class, don't set this member.
     * But if you do, set it to the type of the enumeration itself.
     * 
     * When it annotates an object of type java.util.EnumSet of
     * java.util.BitSet, it indicates the BitSet-annotated enum class that
     * defines the members of the bit set.
     */
    @SuppressWarnings("rawtypes")
    public Class<? extends Enum> elementType() default Enum.class;
}
