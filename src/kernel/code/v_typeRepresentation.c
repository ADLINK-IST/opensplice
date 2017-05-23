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
#include "c_base.h"
#include "v__kernel.h"
#include "v__builtin.h"
#include "v__participant.h"
#include "v_typeRepresentation.h"

/**************************************************************
 * Protected functions
 **************************************************************/
v_typeRepresentation
v__typeRepresentationNew (
    v_kernel kernel,
    const struct v_typeInfo *info,
    os_boolean announce)
{
    v_typeRepresentation tr, found;
    v_message msg;

    assert(info != NULL);

    tr = v_typeRepresentation(c_new(v_kernelType(kernel,K_TYPEREPRESENTATION)));
    if (tr != NULL) {
        tr->typeName = c_keep(info->name);
        tr->dataRepresentationId = info->data_representation_id;
        tr->typeHash = info->type_hash;
        tr->metaData = c_keep(info->meta_data);
        tr->extentions = c_keep(info->extentions);

        found = v__addTypeRepresentation(kernel, tr);
        if (found == tr) {
            if (announce == TRUE) {
                msg = v_builtinCreateTypeInfo(kernel->builtin, tr);
                v_writeBuiltinTopic(kernel, V_TYPEINFO_ID, msg);
                c_free(msg);
            }
        } else if (found != NULL) {
            c_free(tr);
            tr = c_keep(found);
        } else {
            c_free(tr);
            tr = NULL;
        }
    }

    return tr;
}

/**************************************************************
 * Public functions
 **************************************************************/
v_typeHash
v_typeHashFromArray (
    const os_uchar *array,
    os_uint32 arrLen)
{
    v_typeHash hash;
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

v_typeRepresentation
v_typeRepresentationNew (
    v_participant participant,
    const os_char *typeName,
    v_dataRepresentationId_t dataRepresentationId,
    const v_typeHash typeHash,
    const os_uchar *metaData,
    os_uint32 metaDataLength,
    const os_uchar *extentions,
    os_uint32 extentionsLength)
{
    v_typeRepresentation tr = NULL;
    v_typeRepresentation found;
    c_base base = c_getBase(c_object(participant));
    v_kernel kernel = v_objectKernel(v_object(participant));
    struct v_typeInfo info;

    assert(typeName != NULL);
    assert(metaData != NULL);
    assert(metaDataLength != 0);
    assert((extentions != NULL) ? (extentionsLength != 0) : (extentionsLength == 0));

    if ((info.name = c_stringNew_s(base, typeName)) == NULL) {
        goto err_alloc_name;
    } else if ((info.meta_data = c_sequenceNew_s(c_octet_t(base), metaDataLength, metaDataLength)) == NULL) {
        goto err_alloc_meta_data;
    } else {
        info.data_representation_id = dataRepresentationId;
        info.type_hash = typeHash;
        memcpy(info.meta_data, metaData, metaDataLength);
        if (extentionsLength > 0) {
            if ((info.extentions = c_sequenceNew_s(c_octet_t(base), extentionsLength, extentionsLength)) == NULL) {
                goto err_alloc_extentions;
            }
            memcpy(info.extentions, extentions, extentionsLength);
        } else {
            info.extentions = NULL;
        }
    }

    tr = v__typeRepresentationNew(kernel, &info, TRUE);
    if (tr != NULL) {
        found = v__participantAddTypeRepresentation(participant, tr);
        if (found == tr) {
            /* OK */
        } else if (found != NULL) {
            if ((tr->dataRepresentationId != found->dataRepresentationId) ||
                (tr->typeHash.msb != found->typeHash.msb) ||
                (tr->typeHash.lsb != found->typeHash.lsb)) {
                /* Already a TypeRepresentation in participant which is not the one
                 * being added. */
                c_free(found);
                c_free(tr);
                tr = NULL;
            }
        } else {
            c_free(tr);
            tr = NULL;
        }
    }
err_alloc_extentions:
    c_free(info.meta_data);
err_alloc_meta_data:
    c_free(info.name);
err_alloc_name:
    return tr;

}
