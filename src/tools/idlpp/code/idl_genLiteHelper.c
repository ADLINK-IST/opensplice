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

#include "idl_genLiteHelper.h"

#include "os_heap.h"
#include "os_stdlib.h"

const char *
idl_getLiteBasicTypeName(
    idl_typeBasic typeBasic)
{
    const char *value = "";

    switch (idl_typeBasicType(typeBasic)) {
    case idl_short:
        value = "int16_t";
        break;
    case idl_ushort:
        value = "uint16_t";
        break;
    case idl_long:
        value = "int32_t";
        break;
    case idl_ulong:
        value = "uint32_t";
        break;
    case idl_longlong:
        value = "int64_t";
        break;
    case idl_ulonglong:
        value = "uint64_t";
        break;
    case idl_float:
        value = "float";
        break;
    case idl_double:
        value = "double";
        break;
    case idl_char:
        value = "char";
        break;
    case idl_string:
        value = "char *";
        break;
    case idl_boolean:
        value = "bool";
        break;
    case idl_octet:
        value = "uint8_t";
        break;
    }

    return value;
}

const c_char *
idl_scopedLiteSequenceBasicTypeName (
    const idl_typeBasic typeBasic)
{
    const char *value = "";

    switch (idl_typeBasicType(typeBasic)) {
    case idl_short:
        value = "short";
        break;
    case idl_ushort:
        value = "unsigned_short";
        break;
    case idl_long:
        value = "long";
        break;
    case idl_ulong:
        value = "unsigned_long";
        break;
    case idl_longlong:
        value = "long_long";
        break;
    case idl_ulonglong:
        value = "unsigned_long_long";
        break;
    case idl_float:
        value = "float";
        break;
    case idl_double:
        value = "double";
        break;
    case idl_char:
        value = "char";
        break;
    case idl_string:
        value = "string";
        break;
    case idl_boolean:
        value = "boolean";
        break;
    case idl_octet:
        value = "octet";
        break;
    }

    return value;
}

static c_char *
idl_scopedLiteSequenceName (
    const idl_typeSpec typeSpec,
    c_long level)
{
    const char *prefix;
    c_char *name;
    char seqName[256];
    idl_typeSpec nextType = idl_typeSeqType(idl_typeSeq(typeSpec));

    if (level == 0) {
        prefix = "DDS_";
    } else {
        prefix = "";
    }

    if (idl_typeSpecType(nextType) == idl_tseq) {
        name = idl_scopedLiteSequenceName(nextType, level+1);
        snprintf (seqName, sizeof(seqName), "%ssequence_%s", prefix, name);
        os_free(name);
    } else if (idl_typeSpecType(nextType) == idl_tbasic) {
        const char *basename =
            idl_scopedLiteSequenceBasicTypeName(idl_typeBasic(nextType));
        snprintf (seqName, sizeof(seqName), "%ssequence_%s", prefix, basename);
    } else {
        name = idl_scopeStack (
            idl_typeUserScope(idl_typeUser(nextType)),
            "_",
            idl_typeSpecName(idl_typeSpec(nextType))
        );

        if ((idl_typeSpecType(nextType) == idl_tstruct) ||
            (idl_typeSpecType(nextType) == idl_tunion)  ||
            (idl_typeSpecType(nextType) == idl_tenum))  {
            snprintf (seqName, sizeof(seqName), "%ssequence_%s", prefix, name);
        } else if (idl_typeSpecType(nextType) == idl_ttypedef) {
            snprintf (seqName, sizeof(seqName), "%s", name);
        } else {
            snprintf (seqName, sizeof(seqName), "%ssequence_%s", prefix, name);
        }
        os_free (name);
    }

    return os_strdup(seqName);
}

c_char *
idl_scopedLiteTypeName (
    const idl_typeSpec typeSpec)
{
    char scopedName[256];
    c_char *name;

    if (idl_typeSpecType(typeSpec) == idl_tseq) {
        name = idl_scopedLiteSequenceName(typeSpec, 0);
        snprintf (scopedName, sizeof(scopedName), "%s", name);
        os_free(name);
    } else if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            os_strncpy (scopedName, "char *", sizeof(scopedName));
        } else {
            os_strncpy (
                scopedName,
                idl_getLiteBasicTypeName(idl_typeBasic(typeSpec)),
                sizeof(scopedName)
            );
        }
    } else {
        name = idl_scopeStack (
            idl_typeUserScope(idl_typeUser(typeSpec)),
            "_",
            idl_typeSpecName(idl_typeSpec(typeSpec))
        );

        if (idl_typeSpecType(typeSpec) == idl_tstruct ||
                   idl_typeSpecType(typeSpec) == idl_tunion) {
            snprintf (scopedName, sizeof(scopedName), "struct %s", name);
        } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
            snprintf (scopedName, sizeof(scopedName), "enum %s", name);
        } else {
            snprintf (scopedName, sizeof(scopedName), "%s", name);
        }
        os_free (name);
    }

    return os_strdup(scopedName);
}
