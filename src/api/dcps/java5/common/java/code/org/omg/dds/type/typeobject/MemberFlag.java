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

package org.omg.dds.type.typeobject;

import org.omg.dds.type.BitBound;
import org.omg.dds.type.BitSet;
import org.omg.dds.type.Value;


@BitSet
@BitBound(16)
public enum MemberFlag
{
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    @Value(0)
    IS_KEY          ((short)  0),

    @Value(1)
    IS_OPTIONAL     ((short)  1),

    @Value(2)
    IS_SHAREABLE    ((short)  2),

    @Value(3)
    IS_UNION_DEFAULT((short)  3),
    ;



    // -----------------------------------------------------------------------
    // Fields
    // -----------------------------------------------------------------------

    public final short value;



    // -----------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------

    private MemberFlag(short value) {
        this.value = value;
    }
}
