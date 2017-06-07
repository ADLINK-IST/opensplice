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
#include "u__types.h"
#include "v_state.h"
#include "v_instance.h"
#include "v_dataViewSample.h"

const c_char *
u_resultImage(
    u_result result)
{
    c_char *image = NULL;

#define _CASE_(o) case o: image = #o; break;
    switch (result) {
    _CASE_(U_RESULT_UNDEFINED);
    _CASE_(U_RESULT_OK);
    _CASE_(U_RESULT_INTERRUPTED);
    _CASE_(U_RESULT_NOT_INITIALISED);
    _CASE_(U_RESULT_OUT_OF_MEMORY);
    _CASE_(U_RESULT_INTERNAL_ERROR);
    _CASE_(U_RESULT_ILL_PARAM);
    _CASE_(U_RESULT_CLASS_MISMATCH);
    _CASE_(U_RESULT_DETACHING);
    _CASE_(U_RESULT_TIMEOUT);
    _CASE_(U_RESULT_OUT_OF_RESOURCES);
    _CASE_(U_RESULT_INCONSISTENT_QOS);
    _CASE_(U_RESULT_IMMUTABLE_POLICY);
    _CASE_(U_RESULT_PRECONDITION_NOT_MET);
    _CASE_(U_RESULT_ALREADY_DELETED);
    _CASE_(U_RESULT_NO_DATA);
    _CASE_(U_RESULT_UNSUPPORTED);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

const c_char *
u_kindImage(
    u_kind kind)
{
    c_char *image = NULL;

#define _CASE_(o) case o: image = #o; break;
    switch (kind) {
    _CASE_(U_UNDEFINED);
    _CASE_(U_ENTITY);
    _CASE_(U_PARTICIPANT);
    _CASE_(U_PUBLISHER);
    _CASE_(U_WRITER);
    _CASE_(U_SPLICED);
    _CASE_(U_SERVICE);
    _CASE_(U_SERVICEMANAGER);
    _CASE_(U_SUBSCRIBER);
    _CASE_(U_READER);
    _CASE_(U_NETWORKREADER);
    _CASE_(U_GROUPQUEUE);
    _CASE_(U_QUERY);
    _CASE_(U_DATAVIEW);
    _CASE_(U_PARTITION);
    _CASE_(U_TOPIC);
    _CASE_(U_CFTOPIC);
    _CASE_(U_GROUP);
    _CASE_(U_WAITSET);
    _CASE_(U_WAITSETENTRY);
    _CASE_(U_DOMAIN);
    _CASE_(U_LISTENER);
    _CASE_(U_STATUSCONDITION);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

u_typeHash
u_typeHashFromArray(
    const os_uchar *array,
    os_uint32 arrLen)
{
    u_typeHash hash;
#ifdef NDEBUG
    OS_UNUSED_ARG(arrLen);
#endif
    assert(array != NULL);
    assert(arrLen == 16);

#define UCHATOLL(arr) \
    ((c_ulonglong) ((arr)[0] & 0xFF) << 56 | (c_ulonglong) ((arr)[1] & 0xFF) << 48 | \
     (c_ulonglong) ((arr)[2] & 0xFF) << 40 | (c_ulonglong) ((arr)[3] & 0xFF) << 32 | \
     (c_ulonglong) ((arr)[4] & 0xFF) << 24 | (c_ulonglong) ((arr)[5] & 0xFF) << 16 | \
     (c_ulonglong) ((arr)[6] & 0xFF) << 8  | (c_ulonglong) ((arr)[7] & 0xFF))

    hash.msb = UCHATOLL(array);
    hash.lsb = UCHATOLL(&array[8]);
#undef UCHATOLL

    return hash;
}
