/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "kernelModuleI.h"
#include "c_typebase.h"
#include "v_networkHashValue.h"

v_networkHashValue
v_networkHashValueCalculate(
    const char * key1,
    const char * key2)
{
    v_networkHashValue result = {0xa0, 0x22, 0x8d, 0x07};
    const char *currentPtr;

    assert(key1);

#define NW_ROT_CHAR(val, rot) ((c_octet) (((val) << (rot)) + ((val) >> (8-(rot)))))
    currentPtr = key1;
    while (*currentPtr != '\0') {
        /* gcc2.96 gave internal compile errrors (with optimisation enabled)
         * when compiling the NW_ROT_CHAR macro twice in same command :
         * these assignments are deliberatly split over 2 lines as workaround.
         */
        result.h1 = (c_octet) (NW_ROT_CHAR(result.h1, 1) + NW_ROT_CHAR(*currentPtr, 4));
        result.h2 = (c_octet) (NW_ROT_CHAR(result.h2, 2) + NW_ROT_CHAR(*currentPtr, 7));
        result.h3 = (c_octet) (NW_ROT_CHAR(result.h3, 3) + NW_ROT_CHAR(*currentPtr, 1));
        result.h4 = (c_octet) (NW_ROT_CHAR(result.h4, 4) + NW_ROT_CHAR(*currentPtr, 5));
        currentPtr++;
    }

    currentPtr = key2;
    while (currentPtr && *currentPtr != '\0') {
        result.h1 = (c_octet) (NW_ROT_CHAR(result.h1, 4) + NW_ROT_CHAR(*currentPtr, 7));
        result.h2 = (c_octet) (NW_ROT_CHAR(result.h2, 3) + NW_ROT_CHAR(*currentPtr, 1));
        result.h3 = (c_octet) (NW_ROT_CHAR(result.h3, 2) + NW_ROT_CHAR(*currentPtr, 5));
        result.h4 = (c_octet) (NW_ROT_CHAR(result.h4, 1) + NW_ROT_CHAR(*currentPtr, 4));
        currentPtr++;
    }
#undef NW_ROT_CHAR
    return result;
}

c_equality
v_networkHashValueCompare(
    v_networkHashValue * value1,
    v_networkHashValue * value2)
{
    if (value1->h1 < value2->h1) return C_LT;
    if (value1->h1 > value2->h1) return C_GT;
    if (value1->h2 < value2->h2) return C_LT;
    if (value1->h2 > value2->h2) return C_GT;
    if (value1->h3 < value2->h3) return C_LT;
    if (value1->h3 > value2->h3) return C_GT;
    if (value1->h4 < value2->h4) return C_LT;
    if (value1->h4 > value2->h4) return C_GT;
    return C_EQ;
}
