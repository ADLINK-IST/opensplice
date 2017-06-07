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


public enum TypeKind
{
    // -----------------------------------------------------------------------
    // Enumerated Constants
    // -----------------------------------------------------------------------

    /** sentinel indicating "null" value */
    NO_TYPE         (Values.NO_TYPE_VALUE),
    BOOLEAN_TYPE    (Values.BOOLEAN_TYPE_VALUE),
    BYTE_TYPE       (Values.BYTE_TYPE_VALUE),
    INT_16_TYPE     (Values.INT_16_TYPE_VALUE),
    UINT_16_TYPE    (Values.UINT_16_TYPE_VALUE),
    INT_32_TYPE     (Values.INT_32_TYPE_VALUE),
    UINT_32_TYPE    (Values.UINT_32_TYPE_VALUE),
    INT_64_TYPE     (Values.INT_64_TYPE_VALUE),
    UINT_64_TYPE    (Values.UINT_64_TYPE_VALUE),
    FLOAT_32_TYPE   (Values.FLOAT_32_TYPE_VALUE),
    FLOAT_64_TYPE   (Values.FLOAT_64_TYPE_VALUE),
    FLOAT_128_TYPE  (Values.FLOAT_128_TYPE_VALUE),
    CHAR_8_TYPE     (Values.CHAR_8_TYPE_VALUE),
    CHAR_32_TYPE    (Values.CHAR_32_TYPE_VALUE),

    ENUMERATION_TYPE(Values.ENUMERATION_TYPE_VALUE),
    BITSET_TYPE     (Values.BITSET_TYPE_VALUE),
    ALIAS_TYPE      (Values.ALIAS_TYPE_VALUE),

    ARRAY_TYPE      (Values.ARRAY_TYPE_VALUE),
    SEQUENCE_TYPE   (Values.SEQUENCE_TYPE_VALUE),
    STRING_TYPE     (Values.STRING_TYPE_VALUE),
    MAP_TYPE        (Values.MAP_TYPE_VALUE),

    UNION_TYPE      (Values.UNION_TYPE_VALUE),
    STRUCTURE_TYPE  (Values.STRUCTURE_TYPE_VALUE),
    ANNOTATION_TYPE (Values.ANNOTATION_TYPE_VALUE),
    ;



    // -----------------------------------------------------------------------
    // Compile-TIme Constants
    // -----------------------------------------------------------------------

    public static final class Values {
        /** sentinel indicating "null" value */
        public static final int NO_TYPE_VALUE           = 0x0000;
        public static final int PRIMITIVE_TYPE_VALUE    = 0x4000;
        public static final int CONSTRUCTED_TYPE_VALUE  = 0x8000;
        public static final int COLLECTION_TYPE_VALUE   = 0x0200;
        public static final int AGGREGATION_TYPE_VALUE  = 0x0100;
        public static final int ANNOTATION_TYPE_VALUE   = 0x0080;
        
        public static final int BOOLEAN_TYPE_VALUE     =  PRIMITIVE_TYPE_VALUE | 0x1;
        public static final int BYTE_TYPE_VALUE        =  PRIMITIVE_TYPE_VALUE | 0x2;
        public static final int INT_16_TYPE_VALUE      =  PRIMITIVE_TYPE_VALUE | 0x3;
        public static final int UINT_16_TYPE_VALUE     =  PRIMITIVE_TYPE_VALUE | 0x4;
        public static final int INT_32_TYPE_VALUE      =  PRIMITIVE_TYPE_VALUE | 0x5;
        public static final int UINT_32_TYPE_VALUE     =  PRIMITIVE_TYPE_VALUE | 0x6;
        public static final int INT_64_TYPE_VALUE      =  PRIMITIVE_TYPE_VALUE | 0x7;
        public static final int UINT_64_TYPE_VALUE     =  PRIMITIVE_TYPE_VALUE | 0x8;
        public static final int FLOAT_32_TYPE_VALUE    =  PRIMITIVE_TYPE_VALUE | 0x9;
        public static final int FLOAT_64_TYPE_VALUE    =  PRIMITIVE_TYPE_VALUE | 0xA;
        public static final int FLOAT_128_TYPE_VALUE   =  PRIMITIVE_TYPE_VALUE | 0xB;
        public static final int CHAR_8_TYPE_VALUE      =  PRIMITIVE_TYPE_VALUE | 0xC;
        public static final int CHAR_32_TYPE_VALUE     =  PRIMITIVE_TYPE_VALUE | 0xD;

        public static final int ENUMERATION_TYPE_VALUE =  CONSTRUCTED_TYPE_VALUE | 0x1;
        public static final int BITSET_TYPE_VALUE      =  CONSTRUCTED_TYPE_VALUE | 0x2;
        public static final int ALIAS_TYPE_VALUE       =  CONSTRUCTED_TYPE_VALUE | 0x3;

        public static final int ARRAY_TYPE_VALUE       =  CONSTRUCTED_TYPE_VALUE | COLLECTION_TYPE_VALUE | 0x0004;
        public static final int SEQUENCE_TYPE_VALUE    =  CONSTRUCTED_TYPE_VALUE | COLLECTION_TYPE_VALUE | 0x0005;
        public static final int STRING_TYPE_VALUE      =  CONSTRUCTED_TYPE_VALUE | COLLECTION_TYPE_VALUE | 0x0006;
        public static final int MAP_TYPE_VALUE         =  CONSTRUCTED_TYPE_VALUE | COLLECTION_TYPE_VALUE | 0x0007;

        public static final int UNION_TYPE_VALUE       = CONSTRUCTED_TYPE_VALUE | AGGREGATION_TYPE_VALUE | 0x0008;
        public static final int STRUCTURE_TYPE_VALUE   = CONSTRUCTED_TYPE_VALUE | AGGREGATION_TYPE_VALUE | 0x0009;
    }


    // -----------------------------------------------------------------------
    // Fields
    // -----------------------------------------------------------------------

    public final int value;



    // -----------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------

    private TypeKind(int value) {
        this.value = value;
    }
}
