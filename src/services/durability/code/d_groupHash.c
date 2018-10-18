/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__groupHash.h"

#include "kernelModule.h"
#include "v_group.h"
#include "v_state.h"
#include "ut_md5.h"
#include "d__misc.h"
#include "d__configuration.h"


static const char*
v_groupFlushTypeImage(v_groupFlushType type)
{
#define _CASE(x) case x: return #x
    switch(type) {
    _CASE(V_GROUP_FLUSH_REGISTRATION);
    _CASE(V_GROUP_FLUSH_UNREGISTRATION);
    _CASE(V_GROUP_FLUSH_MESSAGE);
    _CASE(V_GROUP_FLUSH_TRANSACTION);
    }
#undef _CASE
    return "(unknown)";
}


void
d_groupHashCalculate(
    struct d_groupHash *groupHash,
    const c_iter list)
{
    struct v_groupFlushData *flushData;
    v_message message;
    c_iterIter local;
    ut_md5_state_t md5st;
    c_octet key[25];
    c_octet state;
    unsigned dataBE[6];
    int i;
    int item = 0;

    assert(groupHash);

    memset(&groupHash->hash, 0, 16);
    groupHash->flags = 1;
    groupHash->nrSamples = c_iterLength(list);
    if (groupHash->nrSamples > 0) {
        local = c_iterIterGet(list);
        flushData = c_iterNext(&local);

        ut_md5_init(&md5st);

        while (flushData) {
            assert(flushData->object);
            switch (flushData->flushType) {
            case V_GROUP_FLUSH_REGISTRATION:
            case V_GROUP_FLUSH_UNREGISTRATION:
                groupHash->nrSamples--;
                break;
            case V_GROUP_FLUSH_MESSAGE:
            case V_GROUP_FLUSH_TRANSACTION:
                message = (v_message)flushData->object;

                dataBE[0] = d_swap4uToBE(message->writerGID.systemId);
                dataBE[1] = d_swap4uToBE(message->writerGID.localId);
                dataBE[2] = d_swap4uToBE(message->writerGID.serial);
                dataBE[3] = d_swap4uToBE((unsigned)OS_TIMEW_GET_SECONDS(message->writeTime));
                dataBE[4] = d_swap4uToBE((unsigned)OS_TIMEW_GET_NANOSECONDS(message->writeTime));
                dataBE[5] = d_swap4uToBE(message->sequenceNumber);
                for (i = 0; i < 6; i++) {
                    memcpy(&key[i*4], &dataBE[i], 4);
                }
                if (flushData->instance) {
                    state = v_stateTest(flushData->instance->state, L_NOWRITERS) ? 1 : 0;
                } else {
                    /* Instance for message unknown, this occurs when the message is read from
                     * a store and therefore should be handled as if the instance is in the
                     * L_NOWRITERS state */
                    state = 1;
                }
                key[24] = state;
                ut_md5_append(&md5st, (const unsigned char *) &key, sizeof(key));

                d_trace(D_TRACE_GROUP_HASH, "%s: [%d] %s {%d.%d.%d}, %"PA_PRItime", %u, %u\n", OS_FUNCTION, item++,
                    v_groupFlushTypeImage(flushData->flushType),
                    message->writerGID.systemId,message->writerGID.localId,message->writerGID.serial,
                    OS_TIMEW_PRINT(message->writeTime),
                    message->sequenceNumber, state);
                break;
            }
            flushData = c_iterNext(&local);
        }
        ut_md5_finish(&md5st, groupHash->hash);
        if (groupHash->nrSamples == 0) {
            groupHash->flags = 0;
            memset(&groupHash->hash, 0, 16);
        }
    }
}

c_char *
d_groupHashToString(
    struct d_groupHash *groupHash)
{
    c_char *hashString;
    int i;

    if (groupHash->nrSamples > 0) {
        hashString = os_malloc(50); /* flags(1) + stringified(nrSamples(8) + hash(16))(48) + \0 */
        assert(groupHash->flags);
        hashString[0] = (char)groupHash->flags;
        sprintf(&hashString[1], "%08x", groupHash->nrSamples);
        for (i=0;i<16;i++) {
            sprintf(&hashString[9+(i*2)], "%02x", groupHash->hash[i]);
        }
    } else {
        hashString = os_malloc(1);
        hashString[0] = '\0';
    }
    d_trace(D_TRACE_GROUP_HASH, "%s: %s\n", OS_FUNCTION, hashString);

    return hashString;
}

/**
 * \brief              This operation translates a hashString to a d_groupHash
 *
 * \param[out] groupHash : The groupHash to store the translated hashString in.
 * \param[in] hashString : The hashString to be translated.
 *
 * \return            : TRUE when a non-empty valid hashString was supplied and
 *                      transformed.
 */
c_bool
d_groupHashFromString(
    struct d_groupHash *groupHash,
    c_char *hashString)
{
    c_bool result = FALSE;
    c_char buf[9];
    c_char *hex;
    int i;

    assert(groupHash);
    assert(hashString);

    d_trace(D_TRACE_GROUP_HASH, "%s: %s\n", OS_FUNCTION, hashString);

    groupHash->flags = (c_octet)hashString[0];
    if (groupHash->flags == 1) {
        memset(&buf, 0, 9);
        for (hex = &hashString[9], i=0; i<16; i++) {
            memcpy(&buf, &hex[i*2], 2);
            groupHash->hash[i] = (c_octet)strtol(buf, NULL, 16);
        }
        memcpy(&buf, &hashString[1], 8);
        groupHash->nrSamples = (c_ulong)strtol(buf, NULL, 16);
        result = TRUE;
    } else {
        groupHash->nrSamples = 0;
        memset(groupHash->hash, 0, 16);
    }

    return result;
}

c_bool
d_groupHashIsEqual(
    struct d_groupHash *groupHash1,
    struct d_groupHash *groupHash2)
{
    c_bool equality = TRUE;
    int i;

    assert(groupHash1);
    assert(groupHash2);

    if (groupHash1 == groupHash2) {
        equality = TRUE;
    } else if ((groupHash1->nrSamples != groupHash2->nrSamples) ||
               (groupHash1->flags != groupHash2->flags)) {
        equality = FALSE;
    } else {
        for (i=0; i<16; i++) {
            if (groupHash1->hash[i] != groupHash2->hash[i]) {
                equality = FALSE;
                break;
            }
        }
    }

    return equality;
}

