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

#ifndef IDL_GENSACSHELPER_H_
#define IDL_GENSACSHELPER_H_

#include "c_typebase.h"

#include "idl_scope.h"
#include "idl_program.h"

typedef enum {
    SACS_EXCLUDE_INDEXES,
    SACS_INCLUDE_INDEXES
} SACS_INDEX_POLICY;

typedef struct {
    c_type type;
    c_char *descriptor;
} idl_metaCsharp;

idl_metaCsharp *
idl_metaCsharpNew(c_type type, c_char *descriptor);

void
idl_metaCharpAddType(
        idl_scope scope,
        const c_char *name,
        idl_typeSpec typeSpec,
        os_iter *metaList);

void
idl_metaCsharpSerialize2XML(
        void *_metaElmnt,
        void *args);

void
idl_CsharpRemovePrefix (
        const c_char *name,
        c_char *prefix);

c_char *idl_CsharpId(
        const c_char *identifier,
        c_bool customPSM,
        c_bool isCType);

void
idl_toCsharpScopingOperator(c_char *scopedName);

c_char *
idl_scopeStackFromCType(c_type dataType);

c_char *idl_scopeStackCsharp(
        idl_scope scope,
        const c_char *scopeSepp,
        const c_char *name);

c_char *idl_CsharpTypeFromTypeSpec(
        idl_typeSpec typeSpec,
        c_bool customPSM);

c_char *idl_genCsharpConstantGetter(void);

c_char *idl_sequenceCsharpIndexString(
        idl_typeSpec typeSpec,
        SACS_INDEX_POLICY policy,
        const char *seqLengthName);

c_char * idl_arrayCsharpIndexString (
        idl_typeSpec typeSpec,
        SACS_INDEX_POLICY policy);

const c_char *
idl_translateIfPredefined(
    const c_char *scopedName);

c_bool
idl_isPredefined(
        const c_char *scopedName);

#endif /* IDL_GENSACSHELPER_H_ */
