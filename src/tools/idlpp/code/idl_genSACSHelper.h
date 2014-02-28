/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
