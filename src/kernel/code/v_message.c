/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "v_kernel.h"
#include "v_message.h"
#include "v_public.h"

/**
 * Compares two sequence-/serial numbers according to RFC 1982.
 *
 * @param i1 The first comparand
 * @param i2 The second comparand
 * @return
 *      C_EQ if i1 == i2,
 *      C_LT if (i1 < i2 && i2 - i1 < 2^31) || (i1 > i2 && i1 - i2 > 2^31),
 *      C_GT if (i1 < i2 && i2 - i1 > 2^31) || (i1 > i2 && i1 - i2 < 2^31)
 */

c_equality
seqNrCompare(
    v_message m1,
    v_message m2)
{
    int is1imp = (v_messageStateTest(m1,L_IMPLICIT) != 0);
    int is2imp = (v_messageStateTest(m2,L_IMPLICIT) != 0);
    if (is1imp && !is2imp) {
        return C_GT;
    } else if(!is1imp && is2imp) {
        return C_LT;
    } else if (m1->sequenceNumber == m2->sequenceNumber) {
        return C_EQ;
    } else if (m1->sequenceNumber < m2->sequenceNumber) {
        return C_LT;
    } else {
        return C_GT;
    }
}

#define CMP_TO_EQ(c) \
    (((c) == OS_EQUAL) ? C_EQ : ((c) == OS_LESS) ? C_LT : C_GT)

c_equality
v_messageCompare (
    v_message m1,
    v_message m2)
{
    c_equality eq;

    if (m1 == m2) return C_EQ;
    if ((eq = CMP_TO_EQ(os_timeWCompare(m1->writeTime, m2->writeTime))) == C_EQ &&
        (eq = v_gidCompare(m1->writerGID, m2->writerGID)) == C_EQ)
    {
        eq = seqNrCompare(m1, m2);
    }
    return eq;
}


c_equality
v_messageCompareAllocTime (
    v_message m1,
    v_message m2)
{
    c_equality eq;

    if (m1 == m2) return C_EQ;
    if ((eq = CMP_TO_EQ(os_timeECompare(m1->allocTime, m2->allocTime))) == C_EQ) {
        /* if message from the same writer then verify sequence number and message validity. */
        if (v_gidCompare(m1->writerGID, m2->writerGID) == C_EQ)
        {
            eq = seqNrCompare(m1, m2);
        }
    }
    return eq;
}


c_equality
v_messageCompareNoTime (
    v_message m1,
    v_message m2)
{
    c_equality eq;

    if (m1 == m2) return C_EQ;
    if ((eq = v_gidCompare(m1->writerGID, m2->writerGID)) == C_EQ) {
        eq = seqNrCompare(m1, m2);
    }
    return eq;
}

c_bool
v_messageCheckDuplicate (
    v_message m1,
    v_message m2)
{
    if (m1 == m2) return TRUE;
    if (m1->sequenceNumber == m2->sequenceNumber) {
        if (v_gidCompare(m1->writerGID, m2->writerGID) == C_EQ) {
            if (!v_messageStateTest(m1,L_IMPLICIT) &&
                !v_messageStateTest(m2,L_IMPLICIT))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

