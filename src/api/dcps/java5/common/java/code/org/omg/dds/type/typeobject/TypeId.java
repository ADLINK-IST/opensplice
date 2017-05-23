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

import org.omg.dds.type.TypeKind;


/**
 * Every type has an ID. Those of the primitive types are predefined.
 */
public final class TypeId {
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    public static final int NO_TYPE_ID =
        TypeKind.Values.NO_TYPE_VALUE;
    public static final int BOOLEAN_TYPE_ID =
        TypeKind.Values.BOOLEAN_TYPE_VALUE;
    public static final int BYTE_TYPE_ID =
        TypeKind.Values.BYTE_TYPE_VALUE;
    public static final int INT_16_TYPE_ID =
        TypeKind.Values.INT_16_TYPE_VALUE;
    public static final int UINT_16_TYPE_ID =
        TypeKind.Values.UINT_16_TYPE_VALUE;
    public static final int INT_32_TYPE_ID =
        TypeKind.Values.INT_32_TYPE_VALUE;
    public static final int UINT_32_TYPE_ID =
        TypeKind.Values.UINT_32_TYPE_VALUE;
    public static final int INT_64_TYPE_ID =
        TypeKind.Values.INT_64_TYPE_VALUE;
    public static final int UINT_64_TYPE_ID =
        TypeKind.Values.UINT_64_TYPE_VALUE;
    public static final int FLOAT_32_TYPE_ID =
        TypeKind.Values.FLOAT_32_TYPE_VALUE;
    public static final int FLOAT_64_TYPE_ID =
        TypeKind.Values.FLOAT_64_TYPE_VALUE;
    public static final int FLOAT_128_TYPE_ID =
        TypeKind.Values.FLOAT_128_TYPE_VALUE;
    public static final int CHAR_8_TYPE_ID =
        TypeKind.Values.CHAR_8_TYPE_VALUE;
    public static final int CHAR_32_TYPE_ID =
        TypeKind.Values.CHAR_32_TYPE_VALUE;



    // -----------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------

    private TypeId() {
        // empty
    }
}
