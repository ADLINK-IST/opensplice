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

#include "idl_genPython.h"

#include "idl_genMetaHelper.h"
#include "idl_keyDef.h"
#include "idl_tmplExp.h"
#include "ut_stack.h"
#include "ut_collection.h"
#include "idl_constSpecifier.h"

#include "os_abstract.h"

#include <stdio.h>
#include <os_stdlib.h>
#include <os_heap.h>

typedef struct file_stack_entry_t {
    idl_fileOut cur_file;
    ut_set includes;
    idl_scope scope;
    idl_streamOut moduleContents;
} file_stack_entry_t;

typedef struct union_case_t union_case_t;
typedef struct union_case_label_t {
    union_case_t *union_case;
    c_bool is_default;
    char *value;
} union_case_label_t;

struct union_case_t {
    const char *name;
    const char *ddstype;
    const char *pytype;
    const char *pydefault;
    const char *pack_string;
    const char *serialize;
    const char *deserialize;
    const char *checker;
    c_ulong nlabels;
    union_case_label_t *labels;
    union_case_label_t *default_label;
};

typedef struct union_info_t {
    const char *name;
    const char *d_ddstype;
    const char *d_pytype;
    const char *d_pydefault;
    const char *d_pack_string;
    const char *d_pack_discriminator;
    const char *d_deserialize;
    const char *d_checker;
    c_ulong ncases;
    union_case_t *cases;
    union_case_t *artificial_case;
} union_info_t;
static ut_stack fileStack = NULL;
static int unionCaseLabelNext = 0;
static int unionCaseNext = 0;
static union_case_t *unionCase = NULL;
static union_info_t *unionInfo = NULL;
static ut_set includes = NULL;
static idl_scope fileScope = NULL;
static const char *baseModuleDir = NULL;
static idl_streamOut moduleContents = NULL;
static idl_streamOut fieldInits = NULL;
static idl_streamOut fieldNames = NULL;
static idl_streamOut fieldPackString = NULL;
static idl_streamOut fieldDeserialize = NULL;
static idl_streamOut fieldPackValues = NULL;
static int idlEnumFieldCount = 0;

static void union_case_label_free(union_case_label_t *c)
{
    os_free((void*)c->value);
}

static void union_case_free(union_case_t *c, void *arg)
{
    c_ulong i;

    OS_UNUSED_ARG(arg);

    os_free((void*)c->ddstype);
    os_free((void*)c->deserialize);
    os_free((void*)c->name);
    os_free((void*)c->pack_string);
    os_free((void*)c->pydefault);
    os_free((void*)c->pytype);
    os_free((void*)c->serialize);
    os_free((void*)c->checker);

    for(i = 0; i < c->nlabels; i++) {
        union_case_label_free(&c->labels[i]);
    }
    if(c->default_label) {
        union_case_label_free(c->default_label);
        os_free(c->default_label);
    }
    if(c->labels) {
        os_free((void*)c->labels);
    }
}

static void union_info_free(union_info_t *ui, void *arg)
{
    c_ulong i;

    OS_UNUSED_ARG(arg);

    os_free((void*)ui->name);
    os_free((void*)ui->d_ddstype);
    os_free((void*)ui->d_pytype);
    os_free((void*)ui->d_pydefault);
    os_free((void*)ui->d_pack_string);
    os_free((void*)ui->d_deserialize);
    os_free((void*)ui->d_pack_discriminator);
    os_free((void*)ui->d_checker);

    for(i = 0; i < ui->ncases; i++) {
        union_case_free(&ui->cases[i],NULL);
    }
    os_free(ui->cases);
    if(ui->artificial_case) {
        union_case_free(ui->artificial_case, NULL);
        os_free(ui->artificial_case);
    }

    os_free(ui);
}

static void
idl_genPythonMkpath(const char *dir_path)
{
    c_char *outCur = idl_dirOutCur();
    char *full_path_name;

    if(outCur) {
        full_path_name = os_malloc(strlen(outCur) + strlen(os_fileSep()) + strlen(dir_path) + 1);
        os_sprintf(full_path_name, "%s%s%s", outCur, os_fileSep(), dir_path);
    } else {
        full_path_name = os_strdup(dir_path);
    }

    if(os_mkpath(full_path_name, 0777) == os_resultFail) {
        printf("Error: Could not create path %s\n", full_path_name);
        os_free(full_path_name);
        exit(-1);
    }
    os_free(full_path_name);
}

static os_equality includesCompare (void *o1, void *o2, void *args)
{
    char *s1 = (char *)o1;
    char *s2 = (char *)o2;
    int cmp = strcmp(s1, s2);

    OS_UNUSED_ARG(args);

    if(cmp < 0) {
        return OS_LT;
    } else if(cmp == 0) {
        return OS_EQ;
    } else {
        return OS_GT;
    }
}

static void
includesFree(void *o, void *arg)
{
    OS_UNUSED_ARG(arg);
    os_free(o);
}

static ut_set
includesNew()
{
    return ut_setNew(includesCompare, NULL, includesFree, NULL);

}

static void
includesAdd(char *include)
{
    if(!ut_contains(ut_collection(includes), include)) {
        char *to_insert = os_strdup(include); /* make a copy for the set */
        (void)ut_setInsert(includes, to_insert);
    }
}

static os_int32 includesPrint_action(void *o, void *arg)
{
    char *include = (char *)o;
    idl_fileOut fileOut = (idl_fileOut) arg;
    idl_fileOutPrintf(fileOut, "import %s\n", include);

    return 1;
}

static void
includesPrint(idl_fileOut fileOut)
{
    if(ut_count(ut_collection(includes)) > 0) {
        ut_walk(ut_collection(includes), includesPrint_action, (void *)fileOut);
        idl_fileOutPrintf(fileOut, "\n");
    }
}

static char *
idl_genPythonPyQName(idl_typeSpec typeSpec) {
    idl_typeUser typeUser = idl_typeUser(typeSpec);
    idl_scope scope = idl_typeUserScope(typeUser);

    if(idl_scopeEqual(scope, fileScope)) {
        return os_strdup(idl_typeSpecName(typeSpec));
    } else {
        char *include = idl_scopeStack(idl_typeUserScope(typeUser), ".", NULL);
        char *ddsQname = idl_scopeStack(idl_typeUserScope(typeUser), ".", idl_typeSpecName(typeSpec));
        const char *basename = idl_scopeBasename(scope);
        char *pyQname = os_malloc(strlen(basename) + 1 + strlen(ddsQname) + 1);
        char *pyInclude = os_malloc(strlen(basename) + 1 + strlen(include) + 1);

        os_sprintf(pyQname, "%s.%s", basename, ddsQname);
        if(strlen(include) == 0) {
            os_sprintf(pyInclude, "%s", basename);
        } else {
            os_sprintf(pyInclude, "%s.%s", basename, include);
        }
        includesAdd(pyInclude);
        os_free(pyInclude);
        os_free(include);
        os_free(ddsQname);
        os_free((void*)basename);
        return pyQname;
    }
}

static void
idl_genPythonPushCurFile(idl_scope scope, const char *file_path)
{
    idl_fileOut newOut;
    file_stack_entry_t *entry = (file_stack_entry_t *)os_malloc(sizeof(file_stack_entry_t));

    if(fileStack == NULL) {
        fileStack = ut_stackNew(UT_STACK_DEFAULT_INC);
    }

    newOut = idl_fileOutNew(file_path, "w");
    if(newOut == NULL) {
        printf("Error: Could not open \"%s\" for write.\n", file_path);
        exit(-1);
    }

    entry->cur_file = idl_fileCur();
    entry->includes = includes;
    entry->scope = fileScope;
    entry->moduleContents = moduleContents;
    ut_stackPush(fileStack, entry);

    idl_fileSetCur(newOut);
    includes = includesNew();
    fileScope = idl_scopeDup(scope);
    moduleContents = idl_streamOutNew(0);
}

static void
idl_genPythonPopCurFile()
{
    file_stack_entry_t *entry = (file_stack_entry_t *)((fileStack && !ut_stackIsEmpty(fileStack)) ?  ut_stackPop(fileStack) : NULL);
    if(entry) {
        idl_fileOut curFile = entry->cur_file;

        /* close current file */
        if(idl_fileCur()) {
            idl_fileOutFree(idl_fileCur());
        }
        idl_fileSetCur(curFile);

        if(includes != NULL) {
            ut_setFree(includes);
        }
        includes = entry->includes;

        if(fileScope) {
            c_char *basename = idl_scopeBasename(fileScope);
            if(basename) {
                os_free((void*)basename);
            }
            idl_scopeFree(fileScope);
        }
        fileScope = entry->scope;

        if (moduleContents) {
            idl_streamOutFree(moduleContents);
        }
        moduleContents = entry->moduleContents;

        os_free(entry);
    }

    /* free the stack, if it's empty */
    if(fileStack && ut_stackIsEmpty(fileStack)) {
        ut_stackFree(fileStack);
        fileStack = NULL;
    }
}

static const char*
idl_genPythonDDSType(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
            return os_strdup("short");
        case idl_ushort:
            return os_strdup("unsigned short");
        case idl_long:
            return os_strdup("long");
        case idl_ulong:
            return os_strdup("unsigned long");
        case idl_longlong:
            return os_strdup("long long");
        case idl_ulonglong:
            return os_strdup("unsigned long long");
        case idl_float:
            return os_strdup("float");
        case idl_double:
            return os_strdup("double");
        case idl_char:
            return os_strdup("char");
        case idl_string:
            return os_strdup("string");
        case idl_boolean:
            return os_strdup("boolean");
        case idl_octet:
            return os_strdup("octet");
        }
    }
    case idl_ttypedef: {
        return idl_genPythonDDSType(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)));
    }
    case idl_tenum:
    case idl_tunion:
    case idl_tstruct: {
        idl_typeUser typeUser = idl_typeUser(typeSpec);
        return idl_scopeStack(idl_typeUserScope(typeUser), "::", idl_typeSpecName(typeSpec));
    }
    case idl_tarray: {
#define FMT_DDS_TYPE_ARRAY "%s[%d]"
        const char *inner = idl_genPythonDDSType(scope, name, idl_typeArrayType(idl_typeArray(typeSpec)));
        c_ulong dim = idl_typeArraySize(idl_typeArray(typeSpec));
        char *buffer = os_malloc(strlen(FMT_DDS_TYPE_ARRAY) + strlen(inner) + 10 /*dim max*/ + 1);
        os_sprintf(buffer, FMT_DDS_TYPE_ARRAY, inner, dim);
        os_free((void *)inner);
        return buffer;
    }
    case idl_tseq: {
#define FMT_DDS_TYPE_SEQ "sequence<%s>"
#define FMT_DDS_TYPE_SEQ_BOUNDED "sequence<%s, %d>"
        const char *inner = idl_genPythonDDSType(scope, name, idl_typeSeqType(idl_typeSeq(typeSpec)));
        c_ulong size = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        char *buffer = size == 0 ? os_malloc(strlen(FMT_DDS_TYPE_SEQ) + strlen(inner) + 1)
                : os_malloc(strlen(FMT_DDS_TYPE_SEQ_BOUNDED) + strlen(inner) + 10 /*dim max*/ + 1);
        if(size == 0) {
            os_sprintf(buffer, FMT_DDS_TYPE_SEQ, inner);
        } else {
            os_sprintf(buffer, FMT_DDS_TYPE_SEQ_BOUNDED, inner, size);
        }
        os_free((void *)inner);
        return buffer;
    }
    }

    return os_strdup("<dds-unexpected>");
}

static const char*
idl_genPythonPyType(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
        case idl_ushort:
        case idl_long:
        case idl_ulong:
        case idl_longlong:
        case idl_ulonglong:
        case idl_octet:
            return os_strdup("int");
        case idl_float:
        case idl_double:
            return os_strdup("float");
        case idl_char:
            return os_strdup("str");
        case idl_string:
            return os_strdup("str");
        case idl_boolean:
            return os_strdup("bool");
        }
    }
    case idl_ttypedef: {
        return idl_genPythonPyType(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)));
    }
    case idl_tenum:
    case idl_tstruct:
    case idl_tunion:
        return idl_genPythonPyQName(typeSpec);
    case idl_tarray:
        return os_strdup("list");
    case idl_tseq:
        return os_strdup("list");
    }
    return os_strdup("<dds-unexpected>");
}

static const char*
idl_genPythonPyDefault(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
        case idl_ushort:
        case idl_long:
        case idl_ulong:
        case idl_longlong:
        case idl_ulonglong:
        case idl_octet:
            return os_strdup("0");
        case idl_float:
        case idl_double:
            return os_strdup("0.0");
        case idl_char:
            return os_strdup("'\\0'");
        case idl_string:
            return os_strdup("''");
        case idl_boolean:
            return os_strdup("False");
        }
    }
    case idl_ttypedef: {
        return idl_genPythonPyDefault(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)));
    }
    case idl_tenum: {
#define FMT_PYDEFAULT_ENUM "%s(0)"
        char *pyQName = idl_genPythonPyQName(typeSpec);
        char *buffer = os_malloc(strlen(FMT_PYDEFAULT_ENUM) - 2 + strlen(pyQName) + 1);
        os_sprintf(buffer, FMT_PYDEFAULT_ENUM, pyQName);
        os_free(pyQName);
        return buffer;
    }
    case idl_tstruct:
    case idl_tunion: {
#define FMT_PYDEFAULT_STRUCT "%s()"
        char *pyQName = idl_genPythonPyQName(typeSpec);
        char *buffer = os_malloc(strlen(FMT_PYDEFAULT_STRUCT) - 2 + strlen(pyQName) + 1);
        os_sprintf(buffer, FMT_PYDEFAULT_STRUCT, pyQName);
        os_free(pyQName);
        return buffer;
    }
    case idl_tarray: {
#define FMT_PYDEFAULT_ARRAY "[%s for _ in range(%d)]"
        const char *nestedDefault = idl_genPythonPyDefault(scope, name, idl_typeArrayType(idl_typeArray(typeSpec)));
        char *buffer = os_malloc(strlen(FMT_PYDEFAULT_ARRAY) + strlen(nestedDefault) + 10/*max int len*/ + 1);
        os_sprintf(buffer, FMT_PYDEFAULT_ARRAY, nestedDefault, idl_typeArraySize(idl_typeArray(typeSpec)));
        os_free((void*)nestedDefault);
        return buffer;
    }
    case idl_tseq:
        /* [] */
        return os_strdup("[]");
    }
    return os_strdup("<dds-unexpected>");
}

static idl_typeSpec
idl_genPythonArrayActual(idl_typeArray typeArray)
{
    idl_typeSpec typeSpec = idl_typeArrayActual(typeArray);
    /* may have navigated into a typedef. Find it's actual type */
    if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefActual(idl_typeDef(typeSpec));
    }
    /* there was a typedef, it may have refered to an array. recurse */
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        return idl_genPythonArrayActual(idl_typeArray(typeSpec));
    } else {
        return typeSpec;
    }
}

static int idl_genPythonArrayDims(idl_typeArray typeArray)
{
    int ndims = 1;
    idl_type type;

    idl_typeSpec typeSpec = idl_typeArrayType(typeArray);
    for(type = idl_typeSpecType(typeSpec); type == idl_tarray || type == idl_ttypedef; type = idl_typeSpecType(typeSpec)) {
        if(type == idl_tarray) {
            typeSpec = idl_typeArrayType(idl_typeArray(typeSpec));
            ndims += 1;
        } else {
            typeSpec = idl_typeDefActual(idl_typeDef(typeSpec));
        }
    }
    return ndims;
}

static c_ulong idl_genPythonArrayNumElements(idl_typeArray typeArray)
{
    c_ulong nele = idl_typeArraySize(typeArray);
    idl_type type;

    idl_typeSpec typeSpec = idl_typeArrayType(typeArray);
    for(type = idl_typeSpecType(typeSpec); type == idl_tarray || type == idl_ttypedef; type = idl_typeSpecType(typeSpec)) {
        if(type == idl_tarray) {
            nele *= idl_typeArraySize(idl_typeArray(typeSpec));
            typeSpec = idl_typeArrayType(idl_typeArray(typeSpec));
        } else {
            typeSpec = idl_typeDefActual(idl_typeDef(typeSpec));
        }
    }
    return nele;
}

static int
idl_genPythonPyPackNeedsAlign(
        idl_typeSpec typeSpec)
{
    switch(idl_typeSpecType(typeSpec)) {
    case idl_tstruct:
    case idl_tseq:
        return 1;
    case idl_tarray:
        return idl_genPythonPyPackNeedsAlign(idl_typeArrayActual(idl_typeArray(typeSpec)));
    case idl_ttypedef:
        return idl_genPythonPyPackNeedsAlign(idl_typeDefActual(idl_typeDef(typeSpec)));
    default:
        return 0;
    }
}

static const char*
idl_genPythonPyPackString(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
            return os_strdup("'h'");
        case idl_ushort:
            return os_strdup("'H'");
        case idl_long:
            return os_strdup("'i'");
        case idl_ulong:
            return os_strdup("'I'");
        case idl_longlong:
            return os_strdup("'q'");
        case idl_ulonglong:
            return os_strdup("'Q'");
        case idl_float:
            return os_strdup("'f'");
        case idl_double:
            return os_strdup("'d'");
        case idl_char:
            return os_strdup("'c'");
        case idl_string:
            return os_strdup("'P'");
        case idl_boolean:
            return os_strdup("'?'");
        case idl_octet:
            return os_strdup("'B'");
        }
    }
    case idl_ttypedef: {
        return idl_genPythonPyPackString(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)));
    }
    case idl_tenum:
        return os_strdup("'i'");
    case idl_tstruct:
    case idl_tunion: {
#define FMT_PYPACK_STRUCT "%s._get_packing_fmt()"
        char *full_name = idl_genPythonPyQName(typeSpec);
        char *result = os_malloc(strlen(FMT_PYPACK_STRUCT) + strlen(full_name) + 1 - 2);
        os_sprintf(result, FMT_PYPACK_STRUCT, full_name);
        os_free(full_name);
        return result;
    }
    case idl_tarray: {
#define FMT_PYPACK_ARRAY "%s * %d"
        c_ulong nele = idl_genPythonArrayNumElements(idl_typeArray(typeSpec));
        idl_typeSpec typeSpecActual = idl_genPythonArrayActual(idl_typeArray(typeSpec));
        const char *nestedDefault = idl_genPythonPyPackString(scope, name, typeSpecActual);
        char *buffer = os_malloc(strlen(FMT_PYPACK_ARRAY) + strlen(nestedDefault) + 10/*max int len*/ + 1);
        os_sprintf(buffer, FMT_PYPACK_ARRAY, nestedDefault, nele);
        os_free((void*)nestedDefault);
        return buffer;
    }
    case idl_tseq:
        return os_strdup("ddsutil._pad_fmt('iiP?')");
    }
    return os_strdup("<dds-unexpected>");
}

static const char*
idl_genPythonPyDeserialize(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec,
        const char *data_var)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
        case idl_ushort:
        case idl_long:
        case idl_ulong:
        case idl_longlong:
        case idl_ulonglong:
        case idl_float:
        case idl_double:
        case idl_octet:
        case idl_boolean: {
#define FMT_PYDESERIALIZE_PRIM "%s.pop(0)"
            char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_PRIM) + strlen(data_var) + 1);
            os_sprintf(buffer, FMT_PYDESERIALIZE_PRIM, data_var);
            return buffer;
        }
        case idl_string: {
#define FMT_PYDESERIALIZE_STRING "ddsutil._ptr_to_bytes(%s.pop(0)).decode('ISO-8859-1')"
            char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_STRING) + strlen(data_var) + 1);
            os_sprintf(buffer, FMT_PYDESERIALIZE_STRING, data_var);
            return buffer;
        }
        case idl_char: {
#define FMT_PYDESERIALIZE_CHAR "%s.pop(0).decode('ISO-8859-1')"
            char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_CHAR) + strlen(data_var) + 1);
            os_sprintf(buffer, FMT_PYDESERIALIZE_CHAR, data_var);
            return buffer;

        }
        }
        break;
    }
    case idl_ttypedef: {
        return idl_genPythonPyDeserialize(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)), data_var);
    }
    case idl_tenum: {
#define FMT_PYDESERIALIZE_ENUM "%s(%s.pop(0))"
        char *pyQName = idl_genPythonPyQName(typeSpec);
        char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_ENUM) + strlen(pyQName) + strlen(data_var) + 1);
        os_sprintf(buffer, FMT_PYDESERIALIZE_ENUM, pyQName, data_var);
        os_free(pyQName);
        return buffer;
    }
    case idl_tunion:
    case idl_tstruct: {
#define FMT_PYDESERIALIZE_STRUCT "%s()._deserialize(%s)"
        char *pyQName = idl_genPythonPyQName(typeSpec);
        char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_STRUCT) + strlen(pyQName) + strlen(data_var)+ 1);
        os_sprintf(buffer, FMT_PYDESERIALIZE_STRUCT, pyQName, data_var);
        os_free(pyQName);
        return buffer;
    }
    case idl_tarray: {
#define FMT_PYDESERIALIZE_ARRAY "[%s for _ in range (%d)]"
        /* TODO: need to check if array type is struct or seq, and do like struct */
        idl_typeSpec nestedType = idl_typeArrayType(idl_typeArray(typeSpec));
        const char *inner = idl_genPythonPyDeserialize(scope, name, nestedType, data_var);
        char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_ARRAY) + strlen(inner) + 10/*max int len*/ + 1);
        os_sprintf(buffer, FMT_PYDESERIALIZE_ARRAY, inner, idl_typeArraySize(idl_typeArray(typeSpec)));
        os_free((void*)inner);
        return buffer;
    }
    case idl_tseq: {
#define FMT_PYDESERIALIZE_SEQ "ddsutil._deserialize_seq(%s, %s, lambda seq_data: %s)"
        idl_typeSpec nestedType = idl_typeSeqType(idl_typeSeq(typeSpec));
        const char *inner = idl_genPythonPyDeserialize(scope, name, nestedType, "seq_data");
        const char *packFmt = idl_genPythonPyPackString(scope, name, nestedType);
        char *buffer = os_malloc(strlen(FMT_PYDESERIALIZE_SEQ) + strlen(data_var) + strlen(packFmt) + strlen(inner) + 1);
        os_sprintf(buffer, FMT_PYDESERIALIZE_SEQ, data_var, packFmt, inner);
        os_free((void*)packFmt);
        os_free((void*)inner);
        return buffer;
    }
    }
    return os_strdup("<py-unexpected>");
}

static const char*
idl_genPythonPyPackField(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec,
        const char *arg_expr,
        const char *field_expr)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
        case idl_ushort:
        case idl_long:
        case idl_ulong:
        case idl_longlong:
        case idl_ulonglong:
        case idl_float:
        case idl_double:
        case idl_octet:
        case idl_boolean: {
#define PKG_FLD_BASIC "%s.append(%s)"
            char *result = os_malloc(strlen(PKG_FLD_BASIC) + strlen(arg_expr) + strlen(field_expr) + 1 - 2);
            os_sprintf(result, PKG_FLD_BASIC, arg_expr, field_expr);
            return result;
        }
        case idl_string: {
#define PKG_FLD_STRING "%s.append(ddsutil._bytes_to_ptr(%s.encode('ISO-8859-1')))"
            /* FIXME: Does this preserve the string data? Only if the string field is bytes, I think */
            char *result = os_malloc(strlen(PKG_FLD_STRING) + strlen(arg_expr) + strlen(field_expr) + 1 - 2);
            os_sprintf(result, PKG_FLD_STRING, arg_expr, field_expr);
            return result;
        }
        case idl_char: {
#define PKG_FLD_CHAR "%s.append(%s.encode('ISO-8859-1'))"
            char *result = os_malloc(strlen(PKG_FLD_CHAR) + strlen(arg_expr) + strlen(field_expr) + 1 - 2);
            os_sprintf(result, PKG_FLD_CHAR, arg_expr, field_expr);
            return result;
        }
        }
        break;
    }
    case idl_ttypedef: {
        return idl_genPythonPyPackField(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)), arg_expr, field_expr);
    }
    case idl_tenum: {
#define PACK_FLD_ENUM "%s.append(%s.value)"
        char *result = os_malloc(strlen(PACK_FLD_ENUM) + strlen(arg_expr) + strlen(field_expr) + 1 - 2);
        os_sprintf(result, PACK_FLD_ENUM, arg_expr, field_expr);
        return result;
    }
    case idl_tunion:
    case idl_tstruct: {
#define PACK_FLD_STRUCT "%s.extend(%s._get_packing_args())"
        char *result = os_malloc(strlen(PACK_FLD_STRUCT) - 2 + strlen(arg_expr) + strlen(field_expr) + 1);
        os_sprintf(result, PACK_FLD_STRUCT, arg_expr, field_expr);
        return result;
    }
    case idl_tarray: {
#define PACK_FLD_ARRAY "ddsutil._lin_map_array(%s, %d, lambda v: %s)"
        idl_typeSpec innerType = idl_genPythonArrayActual(idl_typeArray(typeSpec));
        int ndims = idl_genPythonArrayDims(idl_typeArray(typeSpec));
        const char *inner = idl_genPythonPyPackField(scope, name, innerType, arg_expr, "v");
        char *result = os_malloc(strlen(PACK_FLD_ARRAY) + strlen(field_expr) + strlen(inner) + 10/*dims*/ + 1);
        os_sprintf(result, PACK_FLD_ARRAY, field_expr, ndims, inner);
        os_free((void*)inner);
        return result;
    }
    case idl_tseq: {
#define PACK_FLD_SEQ "%s.extend(ddsutil._serialize_seq(%s, %s, lambda seq_args, v: %s))"
        idl_typeSpec nestedType = idl_typeSeqType(idl_typeSeq(typeSpec));
        const char *nestedPack = idl_genPythonPyPackField(scope, name, nestedType, "seq_args", "v");
        const char *packFmt = idl_genPythonPyPackString(scope, name, nestedType);
        char *buffer = os_malloc(strlen(PACK_FLD_SEQ) + strlen(arg_expr) + strlen(field_expr) + strlen(packFmt) + strlen(nestedPack)  + 1);
        os_sprintf(buffer, PACK_FLD_SEQ, arg_expr, field_expr, packFmt, nestedPack);
        os_free((void*)packFmt);
        os_free((void*)nestedPack);
        return buffer;
    }
    }
    return os_strdup("<dds-unexpected>");
}

static const char*
idl_genPythonPyFieldChecker(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec)
{
    switch(idl_typeSpecType(typeSpec)) {
    case idl_tbasic: {
        switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
        case idl_short:
        	return os_strdup("ddsutil._short_checker");
        case idl_ushort:
        	return os_strdup("ddsutil._ushort_checker");
        case idl_long:
        	return os_strdup("ddsutil._long_checker");
        case idl_ulong:
        	return os_strdup("ddsutil._ulong_checker");
        case idl_longlong:
        	return os_strdup("ddsutil._longlong_checker");
        case idl_ulonglong:
        	return os_strdup("ddsutil._ulonglong_checker");
        case idl_float:
        	return os_strdup("ddsutil._float_checker");
        case idl_double:
        	return os_strdup("ddsutil._float_checker");
        case idl_octet:
        	return os_strdup("ddsutil._octet_checker");
        case idl_boolean:
    	    return os_strdup("ddsutil._bool_checker");
        case idl_string: {
#define FMT_CHECKER_BSTRING "ddsutil._bounded_str_checker(%d)"
    	    c_ulong bound = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
    	    if(bound == 0) {
    	    	return os_strdup("ddsutil._str_checker");
    	    } else {
    	    	char *result = os_malloc(strlen(FMT_CHECKER_BSTRING) + 10 + 1);
    	    	os_sprintf(result, FMT_CHECKER_BSTRING, bound);
    	    	return result;
    	    }
        }
        case idl_char:
    	    return os_strdup("ddsutil._char_checker");
        }
        break;
    }
    case idl_ttypedef: {
        return idl_genPythonPyFieldChecker(scope, name, idl_typeDefActual(idl_typeDef(typeSpec)));
    }
    case idl_tenum:
    case idl_tunion:
    case idl_tstruct: {
#define FMT_CHECKER_CLASS "ddsutil._class_checker(%s)"
    	char *checker_type = idl_genPythonPyQName(typeSpec);
        char *result = os_malloc(strlen(FMT_CHECKER_CLASS) + strlen(checker_type) + 1);
        os_sprintf(result, FMT_CHECKER_CLASS, checker_type);
        os_free((void*)checker_type);
        return result;
    }
    case idl_tarray: {
#define FMT_CHECKER_ARRAY "ddsutil._array_checker(%d,%s)"
        idl_typeSpec innerType = idl_typeArrayType(idl_typeArray(typeSpec));
        c_ulong size = idl_typeArraySize(idl_typeArray(typeSpec));
        const char *inner = idl_genPythonPyFieldChecker(scope, name, innerType);
        char *result = os_malloc(strlen(FMT_CHECKER_ARRAY) + 10/*size*/ + strlen(inner) + 1);
        os_sprintf(result, FMT_CHECKER_ARRAY, size, inner);
        os_free((void*)inner);
        return result;
    }
    case idl_tseq: {
#define FMT_CHECKER_SEQ "ddsutil._seq_checker(%d,%s)"
        idl_typeSpec innerType = idl_typeSeqType(idl_typeSeq(typeSpec));
        c_ulong size = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        const char *inner = idl_genPythonPyFieldChecker(scope, name, innerType);
        char *result = os_malloc(strlen(FMT_CHECKER_SEQ) + 10/*size*/ + strlen(inner) + 1);
        os_sprintf(result, FMT_CHECKER_SEQ, size, inner);
        os_free((void*)inner);
        return result;
    }
    }
    return os_strdup("<dds-unexpected>");

}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genPythonLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genPythonLoadControl;
}

/** @brief callback function called on opening the IDL input file.
 *
 * Generate standard file header consisting of:
 * - mutiple inclusion prevention
 * - inclusion of Splice type definition files
 * - inclusion of application specific include files related to other IDL files
 *
 * @param scope Current scope (not used)
 * @param name Name of the IDL input file
 */
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_pythonUserData *pythonUserData = (idl_pythonUserData *)userData;
    const char *init_py = "__init__.py";
    char *baseModule;
    idl_scope base_scope = idl_scopeDup(scope);

    OS_UNUSED_ARG(scope);
    /*idl_scopePush(base_scope, idl_scopeElementNew(name, idl_tFile));*/

    baseModuleDir = (pythonUserData && pythonUserData->baseModuleName) ? pythonUserData->baseModuleName : name;

    idl_genPythonMkpath(baseModuleDir);

    baseModule = os_malloc(strlen(baseModuleDir) + strlen(os_fileSep()) + strlen(init_py) + 1);
    os_sprintf(baseModule, "%s%s%s", baseModuleDir, os_fileSep(), init_py);
    idl_genPythonPushCurFile(base_scope, baseModule);
    idl_scopeFree(base_scope);

    os_free(baseModule);

    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/** @brief callback function called on closing the IDL input file.
 *
 * Generate standard file footer consisting of:
 * - mutiple inclusion prevention closure
 */
static void
idl_fileClose (
    void *userData)
{
    OS_UNUSED_ARG(userData);
    /* Generate closure of multiple inclusion prevention code */

    includesPrint(idl_fileCur());
    idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet(idl_stream(moduleContents)));

    idl_genPythonPopCurFile();

    baseModuleDir = NULL;

}

/** @brief callback function called on module definition in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   module <module-name> {
            <module-contents>
        };
   @endverbatim
 *
 * This fuction generates the prototype of the function that
 * is responsible for loading the metadata into the database.
 * The name of the function is:
 * @verbatim
        __<scope-basename>_<scope-elements>_<module-name>__load
   @endverbatim
 * For the Splice types, no further actions are required.
 *
 * @param scope Current scope (and scope of the module definition)
 * @param name Name of the defined module
 */
static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    c_char *modPath = idl_scopeStack(scope, os_fileSep(), name);
    size_t moduleDirSize = strlen(baseModuleDir) + strlen(os_fileSep()) + strlen(modPath);
    const char *init_py = "__init__.py";
    char *moduleDir = os_malloc(moduleDirSize + 1);
    char *moduleFile = os_malloc(moduleDirSize + strlen(os_fileSep()) + strlen(init_py) + 1);
    idl_scope moduleScope = idl_scopeDup(scope);

    idl_scopePush(moduleScope, idl_scopeElementNew(name, idl_tModule));

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    os_sprintf(moduleDir, "%s%s%s", baseModuleDir, os_fileSep(), modPath);
    idl_genPythonMkpath(moduleDir);

    os_sprintf(moduleFile, "%s%s%s", moduleDir, os_fileSep(), init_py);
    idl_genPythonPushCurFile(moduleScope, moduleFile);
    idl_scopeFree(moduleScope);

    os_free(modPath);
    os_free(moduleDir);
    os_free(moduleFile);


    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
}

static void
idl_moduleClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    includesPrint(idl_fileCur());
    idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet(idl_stream(moduleContents)));

    idl_genPythonPopCurFile();
}

/** @brief callback function called on structure definition in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   struct <structure-name> {
            <structure-member-1>;
            ...              ...
            <structure-member-n>;
        };
   @endverbatim
 *
 * @param scope Current scope (and scope of the structure definition)
 * @param name Name of the structure
 * @param structSpec Specification of the struct holding the amount of members
 */
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    const char* keylist;

    OS_UNUSED_ARG(userData);

    /* setup streams to collect field information we will apply on idl_structureClose */
    fieldInits = idl_streamOutNew(0);
    fieldNames = idl_streamOutNew(0);
    fieldPackString = idl_streamOutNew(0);
    fieldDeserialize = idl_streamOutNew(0);
    fieldPackValues = idl_streamOutNew(0);

    keylist = idl_keyResolve(idl_keyDefDefGet(), scope, name);
    if(keylist) {
        /* There's a #pragma keylist, so it's a topic class */
        c_char *typename = idl_scopeStack(scope,"::",name);
        c_type type = idl_typeSpecDef(idl_typeSpec(structSpec));
        c_char *metadescriptor = idl_genXMLmeta(type, FALSE);

        includesAdd("dds");
        includesAdd("struct");
        idl_streamOutPrintf(moduleContents,
                "class %s__TypeSupport(dds.TypeSupport):\n"
                "\n"
                "    _typename = '%s'\n"
                "    _keylist = '%s'\n"
                "    _metadescriptor = '%s'\n"
                "\n"
                "    def __init__(self):\n"
                "        super(%s__TypeSupport, self).__init__(self._typename,\n"
                "              self._keylist,\n"
                "              self._metadescriptor,\n"
                "              struct.calcsize(%s()._get_packing_fmt()))\n"
                "\n"
                "    def _serialize(self, o):\n"
                "        if not isinstance(o, %s):\n"
                "            raise TypeError('Incorrect data type')\n"
                "        result = o._serialize()\n"
                "        return result\n"
                "\n"
                "    def _deserialize(self, buf):\n"
                "        result = %s()\n"
                "        data = struct.unpack(result._get_packing_fmt(), buf)\n"
                "        result._deserialize(list(data))\n"
                "        return result\n"
                "\n",
                name,
                typename,
                keylist,
                metadescriptor,
				name,
				name,
				name,
				name
                );
        os_free(metadescriptor);
        os_free(typename);
    }

    includesAdd("ddsutil");
    idl_streamOutPrintf(moduleContents,
            "class %s(ddsutil.TopicDataClass):\n"
            "\n",
            name);

    if(keylist) {
        idl_streamOutPrintf(moduleContents,
                "    @staticmethod\n"
                "    def get_type_support():\n"
                "        ''' Return type support information required for dds.Topic constructor '''\n"
                "        return %s__TypeSupport()\n"
                "\n",
                name);

    }
    /* return idl_explore to indicate that the rest of the structure needs to be processed */
    return idl_explore;
}

/** @brief callback function called on end of a structure definition in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        struct <structure-name> {
            <structure-member-1>
            ...              ...
            <structure-member-n>
   =>   };
   @endverbatim
 *
 * The structure is closed:
 * @verbatim
        };
   @endverbatim
 *
 * @param name Name of the structure (not used)
 */
static void
idl_structureClose(
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idl_streamOutPrintf(moduleContents,
            "    _member_attributes = (%s)\n"
            "\n"
            ,
            idl_streamGet(idl_stream(fieldNames))
            );
    idl_streamOutPrintf(moduleContents,
            "    def __init__(self, **kwargs):\n"
            "        ''' Create a %s instance, optionally initializing fields by name '''\n"
            "        # init attributes to default value\n"
            "%s"
            "        # set values for attributes passed in\n"
            "        for key, value in kwargs.items():\n"
            "            if key not in self._member_attributes:\n"
            "                raise TypeError('Invalid argument name : %%s' %%(key))\n"
            "            setattr(self, key, value)\n"
            "\n"
            ,
            name,
            idl_streamGet(idl_stream(fieldInits))
            );
    idl_streamOutPrintf(moduleContents,
            "    _packing_fmt = None\n"
            "\n"
            "    @staticmethod\n"
            "    def _get_packing_fmt():\n"
            "        ''' Returns packing format of this class data '''\n"
            "        if %s._packing_fmt is not None:\n"
            "            return %s._packing_fmt\n"
            "        fmt = ''\n"
            "%s\n"
            "        %s._packing_fmt = ddsutil._pad_fmt(fmt)\n"
            "        return %s._packing_fmt\n"
            "\n"
            ,
            name,
			name,
            idl_streamGet(idl_stream(fieldPackString)),
			name,
			name
            );
    includesAdd("struct");
    idl_streamOutPrintf(moduleContents,
            "    def _serialize(self):\n"
            "        ''' Serialize data for writing '''\n"
            "        fmt = self._get_packing_fmt()\n"
            "        args = self._get_packing_args()\n"
            "        result = struct.pack(fmt, *args)\n"
            "        return result\n"
            "\n"
            );
    idl_streamOutPrintf(moduleContents,
            "    def _deserialize(self, data):\n"
            "        ''' Initialize values from data returned by struct.unpack()'''\n"
            "%s"
            "        return self;\n"
            "\n"
            ,
            idl_streamGet(idl_stream(fieldDeserialize))
            );
    idl_streamOutPrintf(moduleContents,
            "    def _get_packing_args(self):\n"
            "        ''' Return an list of data suitable for struct.pack '''\n"
            "        args = []\n"
            "%s"
            "        return args\n"
            "\n",
            idl_streamGet(idl_stream(fieldPackValues))
            );
    idl_streamOutFree(fieldNames);
    idl_streamOutFree(fieldInits);
    idl_streamOutFree(fieldPackString);
    idl_streamOutFree(fieldDeserialize);
    idl_streamOutFree(fieldPackValues);
}

/** @brief callback function called on definition of a structure member in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        struct <structure-name> {
   =>       <structure-member-1>;
   =>       ...              ...
   =>       <structure-member-n>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the structure member
 * @param typeSpec Type specification of the structure member
 */
static void
idl_structureMemberOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    const char *ddstype = idl_genPythonDDSType(scope, name, typeSpec);
    const char *pytype = idl_genPythonPyType(scope, name, typeSpec);
    const char *pydefault = idl_genPythonPyDefault(scope, name, typeSpec);
    const char *pydeserialize = idl_genPythonPyDeserialize(scope, name, typeSpec, "data");
    char *field_ref = os_malloc(strlen("self.") + strlen(name) + 1);
    const char *pypackfield;
    const char *pypackstring = idl_genPythonPyPackString(scope, name, typeSpec);
    const char *pychecker = idl_genPythonPyFieldChecker(scope, name, typeSpec);
    int alignpackstring = idl_genPythonPyPackNeedsAlign(typeSpec);

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSpec);
    OS_UNUSED_ARG(userData);

    os_sprintf(field_ref, "self.%s", name);
    pypackfield = idl_genPythonPyPackField(scope, name, typeSpec, "args", field_ref);
    os_free(field_ref);

    /* maintain values accumulated for idl_structureClose */
    idl_streamOutPrintf(fieldNames,
            "%s'%s'",
            idl_streamLength(idl_stream(fieldNames)) > 0
                ? ", " : "",
            name
            );
    idl_streamOutPrintf(fieldInits,
            "        self.%s = %s\n",
            name,
            pydefault
            );
    if(alignpackstring) {
        idl_streamOutPrintf(fieldPackString, "        fmt += ddsutil._align(fmt, %s)\n", pypackstring);
    } else {
        idl_streamOutPrintf(fieldPackString, "        fmt += %s\n", pypackstring);
    }
    idl_streamOutPrintf(fieldDeserialize,
            "        self.%s = %s\n",
            name,
            pydeserialize);
    idl_streamOutPrintf(fieldPackValues,
            "        %s\n",
            pypackfield);

    /* write member-specific code */
    idl_streamOutPrintf(moduleContents,
            "    @property\n"
            "    def %s(self):\n"
            "        '''\n"
            "        a DDS %s value\n"
            "        @rtype: %s\n"
            "        '''\n"
            "        return self._%s_\n"
            "\n",
            name,
            ddstype,
            pytype,
			name);
    idl_streamOutPrintf(moduleContents,
       		"    _%s_checker = %s\n"
            "    @%s.setter\n"
            "    def %s(self, value):\n"
            "        self._%s_checker(value)\n"
            "        self._%s_ = value\n"
            "\n",
			name,
			pychecker,
			name,
			name,
			name,
			name
            );

    os_free((void*)pytype);
    os_free((void*)pydefault);
    os_free((void*)pypackstring);
    os_free((void*)pydeserialize);
    os_free((void*)pypackfield);
    os_free((void*)ddstype);
}

/** @brief callback function called on definition of an enumeration.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   enum <enum-name> {
            <enum-element-1>;
            ...          ...
            <enum-element-n>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the enumeration
 * @param enumSpec Specifies the number of elements in the enumeration
 */
static idl_action
idl_enumerationOpen(
    idl_scope scope,
    const char *name,
    idl_typeEnum enumSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(enumSpec);
    OS_UNUSED_ARG(userData);

    includesAdd("enum");
    idl_streamOutPrintf(moduleContents,
            "class %s(enum.Enum):\n",
            name);
    idlEnumFieldCount = 0;

    /* return idl_explore to indicate that the rest of the enumeration needs to be processed */
    return idl_explore;
}

/** @brief callback function called on closure of an enumeration in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        enum <enum-name> {
            <enum-element-1>;
            ...          ...
            <enum-element-n>;
   =>   };
   @endverbatim
 *
 * @param name Name of the enumeration
 */
static void
idl_enumerationClose(
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    if(idlEnumFieldCount > 0) {
        idl_streamOutPrintf(moduleContents, "\n");
    }
}

/** @brief callback function called on definition of an enumeration element in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        enum <enum-name> {
   =>       <enum-element-1>,
   =>       ...          ...
   =>       <enum-element-n>
        };
   @endverbatim
 *
 * For the last element generate:
 * @verbatim
        <element-name>
   @endverbatim
 * For any but the last element generate:
 * @verbatim
    <element-name>,
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the enumeration element
 */
static void
idl_enumerationElementOpenClose (
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);


    idl_streamOutPrintf(moduleContents,
            "    %s = %d\n",
            name,
            idlEnumFieldCount++);
}
/** @brief callback function called on definition of a union in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the union
 * @param unionSpec Specifies the number of union cases and the union switch type
 */
static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    idl_typeSpec typeSpec = idl_typeUnionSwitchKind(unionSpec);

    OS_UNUSED_ARG(userData);

    unionInfo = os_malloc(sizeof(union_info_t));
    memset(unionInfo, 0, sizeof(union_info_t));
    unionInfo->name = os_strdup(name);
    unionInfo->d_ddstype = idl_genPythonDDSType(scope, name, typeSpec);
    unionInfo->d_pytype = idl_genPythonPyType(scope, name, typeSpec);
    unionInfo->d_pydefault = idl_genPythonPyDefault(scope, name, typeSpec);
    unionInfo->d_pack_string = idl_genPythonPyPackString(scope, name, typeSpec);
    unionInfo->d_pack_discriminator = idl_genPythonPyPackField(scope, name, typeSpec, "args", "self._d");
    unionInfo->d_deserialize = idl_genPythonPyDeserialize(scope, name, typeSpec, "data");
    unionInfo->d_checker = idl_genPythonPyFieldChecker(scope, name, typeSpec);
    unionInfo->ncases = idl_typeUnionNoCases(unionSpec);
    unionInfo->cases = os_malloc(unionInfo->ncases * sizeof(union_case_t));
    memset(unionInfo->cases, 0, unionInfo->ncases * sizeof(union_case_t));
    unionCaseNext = 0;

    return idl_explore;
}

static void idl_genPythonUnionLabelMapEntry(
        union_case_t *c,
        union_case_label_t *cl)
{
    idl_streamOutPrintf(moduleContents,
            "    %s: {\n",
            cl->is_default ? "'__default__'" : cl->value
            );
    idl_streamOutPrintf(moduleContents,
            "        'initializer': lambda: %s,\n"
            "        'case_attr_set': lambda o,v: setattr(o,'%s',v),\n"
            "        'to_str': lambda o: '%s={}'.format(o.%s),\n"
            "        'packing_fmt': lambda: %s,\n"
            "        'pack_case_fld': lambda args, o: %s,\n"
            "        'unpack_data': lambda data, o: o.set_%s(%s, o.discriminator),\n",
            c->pydefault,
            c->name,
            c->name, c->name,
            c->pack_string,
            c->serialize,
            c->name, c->deserialize
            );
    idl_streamOutPrintf(moduleContents,
            "    },\n"
            );
}

static void idl_genPythonUnionLabelArtificialMapEntry()
{
    idl_streamOutPrintf(moduleContents,
            "    %s: {\n"
            "        'initializer': lambda: None,\n"
            "        'case_attr_set': lambda o,v: None,\n"
            "        'to_str': lambda o: None,\n"
            "        'packing_fmt': lambda: '',\n"
            "        'pack_case_fld': lambda args, o: None,\n"
            "        'unpack_data': lambda data, o: None,\n"
            "    },\n",
            "'__default__'"
            );
}

/** @brief create a Python tuple containing the listed labels */
static char *idl_genPythonUnionLabelTuple(c_ulong num_labels, char **labels)
{
    c_ulong i;
    char *result;
    size_t total_label_size = 0;

    for(i = 0; i < num_labels; i++) {
        total_label_size += strlen(labels[i]);
    }

    /* allocate space for ([label,]*) */
    result = os_malloc(2 + total_label_size + num_labels + 1);
    os_strcpy(result, "(");
    for(i = 0; i < num_labels; i++) {
        os_strcat(result, labels[i]);
        /* Note: always appending a comma handles the pathological Python case of a single element tuple,
         * which is indistinguishable from a parenthesized expression. A trailing comma is
         * ugly, but essential in that case */
        if(i == 0 || i != num_labels - 1) {
            os_strcat(result, ",");
        }
    }
    os_strcat(result,")");

    return result;
}
/**
 * @brief return a Python tuple of all the
 */
static char *idl_genPythonUnionNonDefaultLabels(
        union_info_t *ui
        )
{
    c_ulong i, j, num_labels = 0, ii;
    char **labels = NULL;
    char *result;

    for(i = 0; i < ui->ncases; i++) {
        union_case_t *c = &ui->cases[i];
        if(!c->default_label) {
            /* only process cases without a default label */
            num_labels += c->nlabels;
        }
    }

    labels = os_malloc(num_labels * sizeof(char*));
    ii = 0;
    for(i = 0; i < ui->ncases; i++) {
        union_case_t *c = &ui->cases[i];
        if(!c->default_label) {
            /* only process cases without a default label */
            for(j = 0; j < c->nlabels; j++) {
                union_case_label_t *cl = &c->labels[j];
                labels[ii++] = cl->value;
            }
        }
    }

    result = idl_genPythonUnionLabelTuple(num_labels, labels);
    os_free(labels);
    return result;
}

/** @brief return a Python value that selects the given union case */
static char *idl_genPythonUnionCaseDefaultSelector(
        union_case_t *c
        )
{
    if(c->nlabels > 0) {
        return c->labels[0].value;
    } else {
        return c->default_label->value;
    }
}

static char *idl_genPythonUnionCaseAllSelectors(
        union_case_t *c
        )
{
    c_ulong i;
    c_ulong num_labels = c->nlabels;
    char **labels = os_malloc(num_labels * sizeof(char*));
    char *result;

    for(i = 0; i < num_labels; i++) {
        union_case_label_t *cl = &c->labels[i];
        labels[i] = cl->value;
    }

    result = idl_genPythonUnionLabelTuple(num_labels, labels);
    os_free(labels);
    return result;

}
/** @brief callback function called on closure of a union in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
   =>   };
   @endverbatim
 *
 * The union is closed:
 * @verbatim
            } _u;
        };
   @endverbatim
 * @param name Name of the union
 */
static void
idl_unionClose (
    const char *name,
    void *userData)
{
    c_ulong i, j;
    char *nonDefaultLabelSelectors = idl_genPythonUnionNonDefaultLabels(unionInfo);

    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    includesAdd("ddsutil");

    /* output the union 'label_map' variable */
    idl_streamOutPrintf(moduleContents,
            "_%s__label_map = {\n",
            unionInfo->name
            );
    for(i = 0; i < unionInfo->ncases; i++) {
        union_case_t *c = &unionInfo->cases[i];
        for(j = 0; j < c->nlabels; j++) {
            union_case_label_t *cl = &c->labels[j];
            idl_genPythonUnionLabelMapEntry(c, cl);
        }
        if(c->default_label) {
            idl_genPythonUnionLabelMapEntry(c, c->default_label);
        }
    }
    if(unionInfo->artificial_case) {
        idl_genPythonUnionLabelArtificialMapEntry();
    }
    idl_streamOutPrintf(moduleContents,
            "}\n\n"
            );
    idl_streamOutPrintf(moduleContents,
            "_%s__packing_fmt, _%s__label_to_padded_packing_fmt = ddsutil._calc_union_formats(%s,_%s__label_map)\n\n",
            unionInfo->name,
            unionInfo->name,
            unionInfo->d_pack_string,
            unionInfo->name
            );
    /* output the union itself */
    idl_streamOutPrintf(moduleContents,
            "class %s(object):\n"
            "    '''\n"
            "    Implementation of DDS union %s, with a DDS %s discriminator\n"
            "    '''\n\n"
            ""
            "    def _label_switch(self, selector, *args):\n"
            "        v = _%s__label_map.get(self.discriminator, _%s__label_map.get('__default__'))\n"
            "        return v[selector](*args)\n\n"
            ""
            "    def __init__(self,d=None,v=None,**kwargs):\n"
            "        '''\n"
            "        %s()\n"
            "        %s(descriminator, value)\n",
            unionInfo->name,
            unionInfo->name,
            unionInfo->d_ddstype,
            unionInfo->name,
            unionInfo->name,
            unionInfo->name,
            unionInfo->name
            );

    for(i = 0; i < unionInfo->ncases; i++) {
        union_case_t *c = &unionInfo->cases[i];
        idl_streamOutPrintf(moduleContents,
                "        %s(%s=value)\n",
                unionInfo->name,
                c->name
                );
    }
    idl_streamOutPrintf(moduleContents,
            "        @raise ValueError: if discriminator or value is incorrect\n"
            "        @raise NameError: if a keyword argument is not a union case field\n"
            "        @raise RuntimeError: if the function invocation is not one of the above formats\n"
            "        '''\n"
            "        if d is None and v is None and len(kwargs) == 0:\n"
            "            self.discriminator = %s\n" /* d_pydefault */
            "        elif d is not None and len(kwargs) > 0 or len(kwargs) > 1:\n"
            "            raise RuntimeError('Incorrect call format')\n"
            "        elif d is not None:\n"
            "            if not isinstance(d, %s):\n" /* d_pytype */
            "                raise ValueError('First argument is not of type %s')\n" /* d_pytype */
            "            self._d = d\n"
            "            self._label_switch('case_attr_set', self, v)\n"
            "        elif len(kwargs) == 1:\n"
            "            cases = [p for p in dir(%s) if isinstance(getattr(%s,p),property) and p != 'discriminator']\n"
            "            for k, v in kwargs.items():\n"
            "                if k in cases:\n"
            "                    setattr(self, k, v)\n"
            "                else:\n"
            "                    raise NameError('Unknown case field: ' + k)\n"
            "        else:\n"
            "            raise RuntimeError('Incorrect call format')\n\n"
            "",
            unionInfo->d_pydefault,
            unionInfo->d_pytype,
            unionInfo->d_pytype,
            unionInfo->name, unionInfo->name
            );

    idl_streamOutPrintf(moduleContents,
            "    @property\n"
            "    def discriminator(self):\n"
            "        '''\n"
            "        Union discriminator - a DDS %s\n"
            "        @rtype: %s\n"
            "        @return: The current descriptor value\n"
            "        '''\n"
            "        return self._d\n\n"
            ""
    		"    _discriminator_checker = %s\n"
            "    @discriminator.setter\n"
            "    def discriminator(self, value):\n"
            "        '''\n"
            "        Set the union discriminator - a DDS %s\n"
            "        @param value: a new discriminator value\n"
            "        @type value: %s\n"
            "        '''\n"
            "        self._discriminator_checker(value)\n"
            "        self._d = value\n"
            "        self._v = self._label_switch('initializer')\n\n"
            "",
            unionInfo->d_ddstype,
            unionInfo->d_pytype,
			unionInfo->d_checker,
            unionInfo->d_ddstype,
            unionInfo->d_pytype
            );

    for(i = 0; i < unionInfo->ncases; i++) {
        union_case_t *c = &unionInfo->cases[i];
        c_char *defaultCaseSelector  = idl_genPythonUnionCaseDefaultSelector(c);
        const char *caseSelectors = idl_genPythonUnionCaseAllSelectors(c);
        const char *containment_condition = c->default_label ? "not in" : "in";
        const char *containment_condition_neg = c->default_label ? "in" : "not in";
        idl_streamOutPrintf(moduleContents,
                "    @property\n"
                "    def %s(self):\n"
                "        '''\n"
                "        Union case for discriminator %s %s\n"
                "        a DDS %s\n"
                "        @rtype: %s\n",
                c->name,
                containment_condition,
                c->default_label ? nonDefaultLabelSelectors : caseSelectors,
                c->ddstype,
                c->pytype);
        idl_streamOutPrintf(moduleContents,
                "        @return: an %s representing a DDS %s\n"
                "        @raise AttributeError: if discriminator does not select this case\n"
                "        '''\n",
                c->pytype, c->ddstype);
        idl_streamOutPrintf(moduleContents,
                "        if self.discriminator %s %s:\n"
                "            raise AttributeError('discriminator value does not select this attribute')\n"
                "        return self._v\n\n"
                "",
                containment_condition_neg,
                c->default_label ? nonDefaultLabelSelectors : caseSelectors
		);
        idl_streamOutPrintf(moduleContents,
        		"    _%s_checker = %s\n",
                c->name, c->checker);
        idl_streamOutPrintf(moduleContents,
                "    @%s.setter\n"
                "    def %s(self,value):\n"
                "        '''\n",
				c->name,
				c->name);
        idl_streamOutPrintf(moduleContents,
                "        Set the union case %s and union discriminator to %s\n"
                "        @param value: an DDS %s\n"
                "        @type value: %s'''\n",
                c->name,
                defaultCaseSelector,
                c->ddstype,
                c->pytype);
        idl_streamOutPrintf(moduleContents,
                "        if not isinstance(value,%s):\n"
                "            raise TypeError('Value is not of type %s')\n"
                "        self.set_%s(value, %s)\n\n"
                "",
                c->pytype,
                c->pytype,
                c->name,
                defaultCaseSelector
		);
        idl_streamOutPrintf(moduleContents,
                "    def set_%s(self, value, d):\n"
                "        '''\n"
                "        Set union case %s with an appropriate discriminator value\n",
				c->name,
				c->name);
        idl_streamOutPrintf(moduleContents,
                "        @param value: a DDS %s\n"
                "        @type value: %s\n"
                "        @param d: a DDS %s, whose value is %s %s\n"
                "        @type d: %s\n"
                "        '''\n",
                c->ddstype,
                c->pytype,
                unionInfo->d_ddstype,
                containment_condition,
                c->default_label ? nonDefaultLabelSelectors : caseSelectors,
				unionInfo->d_pytype);
        idl_streamOutPrintf(moduleContents,
                "        if d %s %s:\n"
                "            raise ValueError('Value 1 does not select case %s')\n",
                containment_condition_neg,
                c->default_label ? nonDefaultLabelSelectors : caseSelectors,
				c->name);
        idl_streamOutPrintf(moduleContents,
                "        if not isinstance(value,%s):\n"
                "            raise TypeError('Value 2 is not of type %s')\n",
                c->pytype,
                c->pytype);
        idl_streamOutPrintf(moduleContents,
                "        if not isinstance(d, %s):\n"
                "            raise TypeError('Value 2 is not of type %s')\n"
                "        self._d = d\n"
                "        self._v = value\n\n"
                "",
				unionInfo->d_pytype,
				unionInfo->d_pytype);
        os_free((void*)caseSelectors);
    }
    if(unionInfo->artificial_case) {
        c_char *defaultCaseSelector  = idl_genPythonUnionCaseDefaultSelector(unionInfo->artificial_case);
        idl_streamOutPrintf(moduleContents,
                "    def set_default(self, d=%s):\n"
                "        '''\n"
                "        Set union implicit default case with an appropriate discriminator value\n"
                "        @param d: a DDS %s, whose value is not in %s\n"
                "        @type d: %s\n"
                "        '''\n"
                "        if d in %s:\n"
                "            raise ValueError('Value 1 does not select implicit default case')\n"
                "        self._d = d\n"
                "        self._v = None\n\n"
                "",
                defaultCaseSelector,
                unionInfo->d_ddstype,
                nonDefaultLabelSelectors,
                unionInfo->d_pytype,
                nonDefaultLabelSelectors
                );

    }
    idl_streamOutPrintf(moduleContents,
            "    def __str__(self):\n"
            "        formatted = '%s(discrimitator={}'.format(self.discriminator)\n"
            "        s = self._label_switch('to_str',self)\n"
            "        if s is not None:\n"
            "            formatted += ', ' + s\n"
            "        formatted += ')'\n"
            "        return formatted\n\n"
            "",
			unionInfo->name);
    idl_streamOutPrintf(moduleContents,
            "    @staticmethod\n"
            "    def _get_packing_fmt():\n"
            "        ''' Returns packing format of this class data '''\n"
            "        return _%s__packing_fmt\n\n"
            "",
			unionInfo->name);
    idl_streamOutPrintf(moduleContents,
            "    def _get_case_fmt(self):\n"
            "        return _%s__label_to_padded_packing_fmt.get(self.discriminator, _%s__label_to_padded_packing_fmt.get('__default__'))\n\n"
            "",
			unionInfo->name,
			unionInfo->name);
    idl_streamOutPrintf(moduleContents,
            "    def _get_packing_args(self):\n"
            "        args = []\n"
            "        %s\n"
            "        fargs = []\n"
            "        self._label_switch('pack_case_fld', fargs, self)\n"
            "        args.extend(struct.pack(self._get_case_fmt(), *fargs))\n"
            "        return args\n\n"
            "",
			unionInfo->d_pack_discriminator);
    idl_streamOutPrintf(moduleContents,
            "    def _serialize(self):\n"
            "        ''' Serialize data for writing '''\n"
            "        fmt = self._get_packing_fmt()\n"
            "        args = self._get_packing_args()\n"
            "        result = struct.pack(fmt, *args)\n"
            "        return result\n\n"
            ""
            "    def _deserialize(self, data):\n"
            "        ''' Initialize values from data returned by struct.unpack()'''\n"
            "        self.discriminator = %s\n"
            "        ffmt = self._get_case_fmt()\n"
            "        ubuffer = bytes([data.pop(0) for _ in range (struct.calcsize(ffmt))])\n"
            "        udata = list(struct.unpack(ffmt, ubuffer))\n"
            "        self._label_switch('unpack_data', udata, self)\n"
            "        return self\n\n",
            unionInfo->d_deserialize
            );

    os_free(nonDefaultLabelSelectors);
    union_info_free(unionInfo, NULL);
    unionInfo = NULL;
}

/** @brief callback function called on definition of a union case in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
   =>           <union-case-1>;
            case label2.1; .. case label2.n;
   =>           ...        ...
            case labeln.1; .. case labeln.n;
   =>           <union-case-n>;
            default:
   =>           <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param name Name of the union case
 * @param typeSpec Specifies the type of the union case
 */
static void
idl_unionCaseOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    char *field_ref = os_malloc(strlen("o.") + strlen(name) + 1);

    OS_UNUSED_ARG(userData);

    os_sprintf(field_ref, "o.%s", name);

    unionCase->ddstype = idl_genPythonDDSType(scope, name, typeSpec);
    unionCase->pytype = idl_genPythonPyType(scope, name, typeSpec);
    unionCase->pydefault = idl_genPythonPyDefault(scope, name, typeSpec);
    unionCase->name = os_strdup(name);
    unionCase->pack_string = idl_genPythonPyPackString(scope, name, typeSpec);
    unionCase->deserialize = idl_genPythonPyDeserialize(scope, name, typeSpec, "data");
    unionCase->serialize = idl_genPythonPyPackField(scope, name, typeSpec, "args", field_ref);
    unionCase->checker = idl_genPythonPyFieldChecker(scope, name, typeSpec);

    os_free(field_ref);

    unionCase = NULL;
}

static c_char *
idl_valueFromLabelVal (
        idl_labelVal labelVal)
{
    static c_char labelName [1000];

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
        /* TODO: do something similar to idl_genPythonPyQName */
        snprintf (labelName, sizeof(labelName), "%s", idl_labelEnumImage(idl_labelEnum(labelVal)));
    } else {
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
            case V_CHAR:
                snprintf (labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
                break;
            case V_SHORT:
                snprintf (labelName, sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
                break;
            case V_USHORT:
                snprintf (labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
                break;
            case V_LONG:
                snprintf (labelName, sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
                break;
            case V_ULONG:
                snprintf (labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
                break;
            case V_LONGLONG:
                snprintf (labelName, sizeof(labelName), "%"PA_PRId64, idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
                break;
            case V_ULONGLONG:
                snprintf (labelName, sizeof(labelName), "%"PA_PRIu64, idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
                break;
            case V_BOOLEAN:
                /* QAC EXPECT 3416; No side effect here */
                if (idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
                    snprintf (labelName, sizeof(labelName), "True");
                } else {
                    snprintf (labelName, sizeof(labelName), "False");
                }
                break;
            default:
                break;
        }
    }
    return labelName;
}

/** @brief return a Python expression representative of the given label value */
static char *
idl_genPythonLabelValue(
        union_info_t *ui,
        idl_labelVal labelVal
        )
{
    idl_labelType labelType = idl_labelValType(labelVal);
    switch(labelType) {
    case idl_lenum: {
        c_char *value = idl_valueFromLabelVal(labelVal);
        char *result = os_malloc(strlen(ui->d_pytype) + 1 + strlen(value) + 1);
        os_sprintf(result, "%s.%s", ui->d_pytype, value);
        return result;
    }
    case idl_lvalue: {
        if(strcmp(ui->d_ddstype,"char") == 0) {
            c_char *value = idl_valueFromLabelVal(labelVal);
            char *result = os_malloc(strlen("chr()") + strlen(value) + 1);
            os_sprintf(result, "chr(%s)", value);
            return result;
        } else {
            return os_strdup(idl_valueFromLabelVal(labelVal));
        }
    }
    case idl_ldefault:
    default:
        return idl_genPythonLabelValue(ui, idl_labelDefaultAlternative(idl_labelDefault(labelVal)));
    }
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelOpenClose(
    idl_scope ownScope,
    idl_labelVal labelVal,
    void *userData)
{
    idl_labelType labelType = idl_labelValType(labelVal);
    union_case_label_t *case_label;

    OS_UNUSED_ARG(ownScope);
    OS_UNUSED_ARG(userData);

    if(labelType == idl_ldefault) {
        case_label = os_malloc(sizeof(union_case_label_t));
        unionCase->default_label = case_label;
    } else {
        case_label = &unionCase->labels[unionCaseLabelNext++];
    }
    case_label->value = idl_genPythonLabelValue(unionInfo, labelVal);
    case_label->is_default = labelType == idl_ldefault;
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelsOpenClose(
    idl_scope scope,
    idl_labelSpec labelSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    unionCase = &unionInfo->cases[unionCaseNext++];
    unionCaseLabelNext = 0;
    memset(unionCase, 0, sizeof(union_case_t));
    unionCase->nlabels = idl_labelSpecNoLabels(labelSpec);
    unionCase->labels = os_malloc(unionCase->nlabels * sizeof(union_case_label_t));
    memset(unionCase->labels, 0, unionCase->nlabels * sizeof(union_case_label_t));
}

/** @brief callback function called when no default case is defined in an union
 *   for which not all possible label values are specified
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param labelVal Default value for the label case (lowest possible not used index)
 * @param typeSpec Specifies the type of the union switch
 */
static void
idl_artificialDefaultLabelOpenClose(
    idl_scope scope,
    idl_labelVal labelVal,
    idl_typeSpec typeSpec,
    void *userData)
{
    union_case_t *union_case = NULL;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSpec);
    OS_UNUSED_ARG(userData);

    union_case = unionInfo->artificial_case = os_malloc(sizeof(union_case_t));

    union_case->ddstype = os_strdup("__NONE__");
    union_case->pytype = os_strdup("__NONE__");
    union_case->pydefault = os_strdup("None");
    union_case->name = os_strdup("__NONE__");
    union_case->pack_string = os_strdup("");
    union_case->deserialize = os_strdup("None");
    union_case->serialize = os_strdup("None");
    union_case->checker = os_strdup("None");
    union_case->nlabels = 0;
    union_case->labels = NULL;

    union_case->default_label = os_malloc(sizeof(union_case_label_t));
    union_case->default_label->value = idl_genPythonLabelValue(unionInfo, labelVal);
    union_case->default_label->is_default = TRUE;
}

/** @brief callback function called on definition of a named type in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   typedef <type-name> <name>;
   @endverbatim
 *
 * @param scope Current scope
 * @param name Specifies the name of the type
 * @param defSpec Specifies the type of the named type
 */
static void
idl_typedefOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(defSpec);
    OS_UNUSED_ARG(userData);

}

static void
idl_sequenceOpenClose (
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSeq);
    OS_UNUSED_ARG(userData);
}

/* constantOpenClose callback

   Generate dependencies for the following IDL construct:
   =>   const <type-name> <name>;
   For <type-name> determine if it is specified in the same file as
   this const definition self. If it is defined in another file, add
   a dependency to the dependency list.
*/
static void
idl_constantOpenClose(
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    idl_typeSpec typeSpec = idl_constSpecTypeGet(constantSpec);
    idl_type type = idl_typeSpecType(typeSpec);

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    if(type == idl_tbasic) {
        char *image = idl_constSpecImage(constantSpec);
        idl_basicType basicType = idl_typeBasicType(idl_typeBasic(typeSpec));
        if(basicType == idl_char) {
            char *new_image;
            new_image = os_malloc(strlen(image) + strlen("chr()") + 1);
            os_sprintf(new_image, "chr(%s)", image);
            os_free((void*)image);
            image = new_image;
        }
        idl_streamOutPrintf(moduleContents,
                "%s = %s\n\n",
                idl_constSpecName(constantSpec),
                image
                );
        os_free((void*)image);
    } else if(type == idl_tenum) {
        char *image = idl_constSpecImage(constantSpec);
        char *enumQname = idl_genPythonPyQName(typeSpec);

        idl_streamOutPrintf(moduleContents,
                "%s = %s.%s\n\n",
                idl_constSpecName(constantSpec),
                enumQname,
                image
                );
        os_free((void*)image);
        os_free((void*)enumQname);
    } else {
        printf("should not get here. idl_type == %d\n", type);
        exit(1);
    }
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genPythonType = {
    idl_getControl,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    idl_moduleClose,
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    idl_enumerationOpen,
    idl_enumerationClose,
    idl_enumerationElementOpenClose,
    idl_unionOpen,
    idl_unionClose,
    idl_unionCaseOpenClose,
    idl_unionLabelsOpenClose,
    idl_unionLabelOpenClose,
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    idl_sequenceOpenClose,
    idl_constantOpenClose,
    idl_artificialDefaultLabelOpenClose,
    NULL /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genPythonProgram (
        void)
{
    return &idl_genPythonType;
}

