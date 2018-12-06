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
#include "idl_genISOCxx2Helper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genCxxHelper.h"
#include "idl_genCHelper.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "c_misc.h"
#include "c_metabase.h"
#include <math.h>

/* Specify a list of all C++ keywords */
static const char *ISOCxx2_keywords[] = {
    /* QAC EXPECT 5007; Bypass qactools error */
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand",
    "bitor", "bool", "break", "case", "catch", "char", "char16_t",
    "char32_t", "class", "compl", "concept", "const", "constexpr",
    "const_cast", "continue", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit",
    "export", "extern", "false", "float", "for", "friend",
    "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
    "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
    "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
    "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
    "true", "try", "typedef", "typeid", "typename", "union", "unsigned",
    "using", "virtual", "void", "volatile", "wchar_t", "while",
    "xor", "xor_eq",
    "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
};

/* Translate an IDL identifier into  a C++11 language identifier.
   The IDL specification states that all identifiers that match
   a C++11 keyword must be prepended by "_cxx_".
*/
c_char *
idl_ISOCxx2Id(
    const char *identifier)
{
    size_t i;
    char *cxxId;

    /* search through the C++ keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < sizeof(ISOCxx2_keywords)/sizeof(c_char *); i++) {
    /* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
        if (strcmp(ISOCxx2_keywords[i], identifier) == 0) {
            /* If a keyword matches the specified identifier, prepend _cxx_ */
            /* QAC EXPECT 5007; will not use wrapper */
            cxxId = os_malloc(strlen(identifier)+1+5);
            snprintf(cxxId, strlen(identifier)+1+5, "_cxx_%s", identifier);
            return cxxId;
        }
    }
    /* No match with a keyword is found, thus return the identifier itself */
    cxxId = os_strdup(identifier);
    return cxxId;
    /* QAC EXPECT 2006; performance is selected above rules here */
}

/* Build a textual representation of the provided scope stack taking the
   C++ keyword identifier translation into account. Further the function
   equals "idl_scopeStack".
*/
c_char *
idl_scopeStackISOCxx2(
    idl_scope scope,
    const char *scopeSepp,
    const char *name)
{
    c_long si;
    c_long sz;
    c_char *scopeStack;
    c_char *Id;

    si = 0;
    sz = idl_scopeStackSize(scope);
    if (si < sz) {
        /* The scope stack is not empty */
        /* Copy the first scope element name */
        scopeStack = os_strdup(scopeSepp);/* start with the seperator */
        Id = idl_ISOCxx2Id(idl_scopeElementName(idl_scopeIndexed(scope, si)));
        scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
        os_strcat(scopeStack, Id);
        os_free(Id);
        si++;
        while (si < sz) {
            /* Translate the scope name to a C++ identifier */
            Id = idl_ISOCxx2Id(idl_scopeElementName(idl_scopeIndexed(scope, si)));
            /* allocate space for the current scope stack + the separator
               and the next scope name
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
           /* Concatenate the separator */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat(scopeStack, scopeSepp);
           /* Concatenate the scope name */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat(scopeStack, Id);
           os_free(Id);
           si++;
        }
        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a C++ identifier */
            Id = idl_ISOCxx2Id(name);
            /* allocate space for the current scope stack + the separator
               and the user identifier
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
            /* Concatenate the separator */
            /* QAC EXPECT 5007; will not use wrapper */
            os_strcat(scopeStack, scopeSepp);
            /* Concatenate the user identifier */
            /* QAC EXPECT 5007; will not use wrapper */
            os_strcat(scopeStack, Id);
            os_free(Id);
    }
    } else {
    /* The stack is empty */
    if (name) {
        /* A user identifier is specified */
        scopeStack = idl_ISOCxx2Id(name);
    } else {
        /* make the stack represenation empty */
        scopeStack = os_strdup("");
    }
    }
    /* return the scope stack representation */
    return scopeStack;
}

/* Return the C++ specific type identifier for the
   specified type specification
*/
c_char *
idl_ISOCxx2TypeFromTypeSpec(
        idl_typeSpec typeSpec)
{
    c_type t = idl_typeSpecDef(typeSpec);
    return idl_ISOCxx2TypeFromCType(t);
}

static c_char *
ISOCxx2FromfullyScopedName(c_char *fullyScopedName)
{
    c_char *savePtr = NULL;
    c_char *name, *cxxName, *typeName = NULL;
    size_t totalLen = strlen(fullyScopedName) + 1;
    size_t len = 0;

    name = os_strtok_r(fullyScopedName, ":", &savePtr);
    if (name) {
        cxxName = idl_ISOCxx2Id(name);
        if (strlen(cxxName) != strlen(name)) {
            totalLen += (strlen(cxxName) - strlen(name));
        }
        typeName = os_malloc(totalLen);

        len = (size_t)snprintf(typeName, totalLen, "%s", cxxName);
        assert(len < totalLen);
        os_free(cxxName);
        name = os_strtok_r(NULL, ":", &savePtr);
        while (name != NULL) {
            cxxName = idl_ISOCxx2Id(name);
            if (strlen(cxxName) != strlen(name)) {
                totalLen += (strlen(cxxName) - strlen(name));
                typeName = os_realloc(typeName, totalLen);
            }
            assert(typeName[len] == '\0');
            len += (size_t)snprintf(typeName + len, totalLen - len, "::%s", cxxName);
            assert(len < totalLen);
            os_free(cxxName);
            name = os_strtok_r(NULL, ":", &savePtr);
        }
    } else {
        typeName = os_malloc(1);
        typeName[0] = '\0';
    }

    return typeName;
}

c_char *
idl_ISOCxx2TypeFromCType(
    c_type t)
{
    c_char *typeName = NULL;
    c_char *subTypeName;
    const c_char *template;
    size_t typeNameSize;
    c_ulong arrSize;

    switch (c_baseObjectKind(t)) {
    case M_PRIMITIVE:
        switch (c_primitiveKind(t))
        {
        case P_SHORT:
            typeName = os_strdup("int16_t");
            break;
        case P_USHORT:
            typeName = os_strdup("uint16_t");
            break;
        case P_LONG:
            typeName = os_strdup("int32_t");
            break;
        case P_ULONG:
            typeName = os_strdup("uint32_t");
            break;
        case P_LONGLONG:
            typeName = os_strdup("int64_t");
            break;
        case P_ULONGLONG:
            typeName = os_strdup("uint64_t");
            break;
        case P_FLOAT:
            typeName = os_strdup("float");
            break;
        case P_DOUBLE:
            typeName = os_strdup("double");
            break;
        case P_CHAR:
            typeName = os_strdup("char");
            break;
        case P_BOOLEAN:
            typeName = os_strdup("bool");
            break;
        case P_OCTET:
            typeName = os_strdup("uint8_t");
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION:
        switch (c_collectionTypeKind(t))
        {
        case OSPL_C_STRING:
            typeName = os_strdup("std::string");
            break;
        case OSPL_C_SEQUENCE:
            template = "std::vector<%s> ";
            subTypeName = idl_ISOCxx2TypeFromCType(c_collectionTypeSubType(t));
            typeNameSize = strlen(template) + strlen(subTypeName) + 1;
            typeName = os_malloc(typeNameSize);
            (void)snprintf(typeName, typeNameSize, template, subTypeName);
            os_free(subTypeName);
            break;
        case OSPL_C_ARRAY:
            template = "dds::core::array<%s, %d> ";
            subTypeName = idl_ISOCxx2TypeFromCType(c_collectionTypeSubType(t));
            arrSize = c_collectionTypeMaxSize(t);
            typeNameSize = strlen(template) + strlen(subTypeName) + (size_t) log10((double) arrSize) + 1;
            typeName = os_malloc(typeNameSize);
            (void)snprintf(typeName, typeNameSize, template, subTypeName, arrSize);
            os_free(subTypeName);
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_ENUMERATION:
    case M_STRUCTURE:
    case M_UNION:
    case M_TYPEDEF:
        subTypeName = c_metaScopedName(c_metaObject(t));
        typeName = ISOCxx2FromfullyScopedName(subTypeName);
        os_free(subTypeName);
        break;
    default:
        assert(0);
        break;
    }
    return typeName;
}

static c_char *
abstractEnumLabel(c_type t, c_ulong i)
{
    c_char *scopeName, *cxxScopeName, *cxxEnumTp, *cxxEnumLabel, *abstractLabel;
    const c_char *enumTmplt = "OSPL_ENUM_LABEL(%s,%s,%s)";
    size_t abstractLabelLen;

    scopeName = c_metaScopedName(c_metaObject(t)->definedIn);
    cxxScopeName = ISOCxx2FromfullyScopedName(scopeName);
    cxxEnumTp = idl_ISOCxx2Id(c_metaObject(t)->name);
    cxxEnumLabel = idl_ISOCxx2Id(c_metaObject(c_enumeration(t)->elements[i])->name);
    abstractLabelLen = strlen(enumTmplt) + strlen(cxxScopeName) + strlen(cxxEnumTp) + strlen(cxxEnumLabel) + 1;
    abstractLabel = os_malloc(abstractLabelLen);
    (void)snprintf(abstractLabel, abstractLabelLen, enumTmplt, cxxScopeName, cxxEnumTp, cxxEnumLabel);
    os_free(cxxEnumLabel);
    os_free(cxxEnumTp);
    os_free(cxxScopeName);
    os_free(scopeName);
    return abstractLabel;
}

c_char *
idl_ISOCxx2DefaultValueFromCType(
    c_type t)
{
    os_char *defValue = NULL;

    t = c_typeActualType(t);
    switch (c_baseObjectKind(t)) {
    case M_PRIMITIVE:
        switch (c_primitiveKind(t))
        {
        case P_SHORT:
        case P_USHORT:
        case P_LONG:
        case P_ULONG:
        case P_LONGLONG:
        case P_ULONGLONG:
        case P_CHAR:
        case P_OCTET:
            defValue = os_strdup("0");
            break;
        case P_FLOAT:
            defValue = os_strdup("0.0f");
            break;
        case P_DOUBLE:
            defValue = os_strdup("0.0");
            break;
        case P_BOOLEAN:
            defValue = os_strdup("false");
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_ENUMERATION:
        defValue = abstractEnumLabel(t, 0);
        break;
    case M_COLLECTION:
    case M_STRUCTURE:
    case M_UNION:
        defValue = NULL; /* These classes have their own default constructor. */
        break;
    case M_TYPEDEF:
        assert(0); /* Typedef has already been resolved: should not get here! */
        break;
    default:
        assert(0);
        break;
    }
    return defValue;
}

c_char *
idl_ISOCxx2ValueFromCValue(
    c_type t,
    c_value v)
{
    os_char *cxxValue = NULL;

    t = c_typeActualType(t);
    switch (c_baseObjectKind(t)) {
    case M_PRIMITIVE:
        switch (c_primitiveKind(t))
        {
        case P_LONG:
        case P_ULONG:
        case P_LONGLONG:
        case P_ULONGLONG:
        case P_OCTET:
        case P_FLOAT:
        case P_DOUBLE:
        case P_SHORT:
        case P_USHORT:
        case P_CHAR:
            /* Forward to default C++ literal representation. */
            cxxValue = idl_ISOCxx2ValueFromCValue(t, v);
            break;
        case P_BOOLEAN:
            if (v.is.Boolean) {
                cxxValue = os_strdup("true");
            } else {
                cxxValue = os_strdup("false");
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_ENUMERATION:
        cxxValue = abstractEnumLabel(t, (c_ulong) v.is.Long);
        break;
    case M_COLLECTION:
    case M_STRUCTURE:
    case M_UNION:
    case M_TYPEDEF:
        assert(0); /* None of these types should be expected here */
        break;
    default:
        assert(0);
        break;
    }
    return cxxValue;
}

c_char *
idl_genISOCxx2LiteralValueImage(
    c_value literal,
    c_type type)
{
    c_char * valueImg = NULL;

    if (c_baseObject(type)->kind != M_ENUMERATION) {
        valueImg = idl_genCLiteralValueImage(literal, type);
    } else {
        const char *ENUM_TEMPLATE = "OSPL_UNSCOPED_ENUM_LABEL(%s,%s)";
        char *cxxEnumTp = idl_ISOCxx2Id(c_metaObject(type)->name);
        char *cxxEnumLabel = idl_ISOCxx2Id(c_metaObject(c_enumeration(type)->elements[literal.is.Long])->name);
        size_t valLen = strlen(cxxEnumTp) + strlen(cxxEnumLabel) + strlen(ENUM_TEMPLATE) + 1;
        valueImg = os_malloc(valLen);
        snprintf(valueImg, valLen, ENUM_TEMPLATE, cxxEnumTp, cxxEnumLabel);
        os_free(cxxEnumTp);
        os_free(cxxEnumLabel);
    }
    return valueImg;
}

c_bool
idl_ISOCxx2IsRefType(
    c_type t)
{
    t = c_typeActualType(t);

    switch (c_baseObjectKind(t)) {
    case M_PRIMITIVE:
    case M_ENUMERATION:
        return FALSE;
    case M_COLLECTION:
    case M_STRUCTURE:
    case M_UNION:
        return TRUE;
    case M_TYPEDEF:
        assert(0); /* Typedef should have been resolved already. */
        break;
    default:
        assert(0);
        break;
    }
    return FALSE;
}

c_char *
idl_ISOCxx2InTypeFromCType(
    c_type t)
{
    os_char *inType = NULL;
    os_char *paramType;
    const char *template;
    size_t size;

    switch (c_baseObjectKind(t)) {
    case M_PRIMITIVE:
    case M_ENUMERATION:
        inType = idl_ISOCxx2TypeFromCType(t);
        break;
    case M_COLLECTION:
    case M_STRUCTURE:
    case M_UNION:
        template = "const %s&";
        paramType = idl_ISOCxx2TypeFromCType(t);
        size = strlen(template) + strlen(paramType) + 1;
        inType = os_malloc(size);
        (void)snprintf(inType, size, template, paramType);
        os_free(paramType);
        break;
    case M_TYPEDEF:
        switch(c_baseObjectKind(c_typeActualType(t))) {
        case M_PRIMITIVE:
        case M_ENUMERATION:
            inType = idl_ISOCxx2TypeFromCType(t);
            break;
        case M_COLLECTION:
        case M_STRUCTURE:
        case M_UNION:
            template = "const %s&";
            paramType = idl_ISOCxx2TypeFromCType(t);
            size = strlen(template) + strlen(paramType) + 1;
            inType = os_malloc(size);
            (void)snprintf(inType, size, template, paramType);
            os_free(paramType);
            break;
        default:
            assert(0);
            break;
        }
        break;
    default:
        assert(0);
        break;
    }
    return inType;
}

c_value
idl_ISOCxx2LowestUnionDefaultValue(
        c_type t)
{
    /* Forward to classic C++ handler: no C++11 override needed. */
    return idl_CxxLowestUnionDefaultValue(t);
}
