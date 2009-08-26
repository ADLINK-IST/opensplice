/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "idl_genSACSHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_scope.h"
#include "idl_tmplExp.h"
#include "idl_typeSpecifier.h"
#include "idl_keyDef.h"
#include "idl_genMetaHelper.h"

#include <os_iterator.h>
#include <os_heap.h>
#include <os_stdlib.h>
#include <string.h>


/* Allocate a metaData structure used for generation of typeDescriptors outside
   the scope of the idl_walk.
 */
idl_metaCsharp *
idl_metaCsharpNew(c_type type, c_char *descriptor)
{
    idl_metaCsharp *idl_metaElmnt = os_malloc(sizeof(idl_metaCsharp));
    idl_metaElmnt->type = type;
    idl_metaElmnt->descriptor = descriptor;
    return idl_metaElmnt;
}

/* Add a copy of the specified metadata for datatypes that serve as topics
   (i.e. have a keylist). This list if metadata will be used for later processing,
   when the type's lock is available again.
 */
void
idl_metaCharpAddType(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec,
        os_iter *metaList)
{
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        c_type type = idl_typeSpecDef(typeSpec);
        idl_metaCsharp *metaElmnt = idl_metaCsharpNew(type, NULL);
        *metaList = os_iterAppend(*metaList, metaElmnt);
    }
}


void
idl_metaCsharpSerialize2XML(
        idl_metaCsharp *metaElmnt,
        void *args)
{
    if (!metaElmnt->descriptor)
        metaElmnt->descriptor = idl_genXMLmeta(metaElmnt->type);
}

/* Specify a list of all C# keywords */
static const char *Csharp_keywords[91] = {
    /* C# keywords */
    "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char",
    "checked", "class", "const", "continue", "decimal", "default", "delegate",
    "do", "double", "else", "enum", "event", "explicit", "extern", "finally",
    "fixed", "float", "for", "foreach", "goto", "if", "implicit", "in",
    "int", "interface", "internal", "is", "lock", "long", "namespace", "new",
    "object", "operator", "out", "override", "params", "private", "protected",
    "public", "readonly", "ref", "return", "sbyte", "sealed", "short", "sizeof",
    "stackalloc", "static", "string", "struct", "switch", "this", "throw",
    "try", "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using",
    "virtual", "void", "volatile", "while",
    /* C# constants */
    "true", "false", "null",
    /* C# contextual keywords */
    "get", "partial", "set", "getClass", "value", "where", "yield",
    /* Operations of the C# 'Object' class */
    "Equals", "Finalize", "GetHashCode", "GetType", "MemberwiseClone",
    "ReferenceEquals", "ToString"
};

/* Translate an IDL identifier into a C# language identifier.
   The IDL specification often states that all identifiers that
   match a native keyword must be prepended by "_".
*/
c_char *
idl_CsharpId(
    const char *identifier)
{
    c_long i;
    char *CsharpId;

    /* search through the Java keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < (c_long)(sizeof(Csharp_keywords)/sizeof(c_char *)); i++) {
    /* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
        if (strcmp (Csharp_keywords[i], identifier) == 0) {
            /* If a keyword matches the specified identifier, prepend _ */
            /* QAC EXPECT 5007; will not use wrapper */
            CsharpId = os_malloc((size_t)((int)strlen(identifier)+1+1));
            snprintf(CsharpId, (size_t)((int)strlen(identifier)+1+1), "_%s", identifier);
            return CsharpId;
        }
    }

    return os_strdup(identifier);
    /* QAC EXPECT 2006; performance is selected above rules here */
}

static c_char *
idl_scopeCsharpElementName (
    idl_scopeElement scope)
{
    c_char *scopeName;
    c_char *scopeCsharpName;

    scopeName = idl_scopeElementName(scope);
    if ((idl_scopeElementType(scope) == idl_tStruct) ||
            (idl_scopeElementType(scope) == idl_tUnion)) {
        scopeCsharpName = os_malloc(strlen(scopeName) + 8);
        sprintf(scopeCsharpName, "%sPackage", scopeName);
    } else {
        scopeCsharpName = scopeName;
    }
    return scopeCsharpName;
}

/* Build a textual presentation of the provided scope stack taking the
   C# keyword identifier translation into account. Further the function
   equals "idl_scopeStack".
*/
c_char *
idl_scopeStackCsharp (
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
        scopeStack = os_strdup(idl_CsharpId(idl_scopeCsharpElementName(idl_scopeIndexed(scope, si))));
        si++;
        while (si < sz) {
            /* Translate the scope name to a C identifier */
            Id = idl_CsharpId(idl_scopeCsharpElementName(idl_scopeIndexed(scope, si)));
            /* allocate space for the current scope stack + the separator
               and the next scope name
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, (size_t)(
                             (int)strlen(scopeStack)+
                             (int)strlen(scopeSepp)+
                             (int)strlen(Id)+1));
           /* Concatenate the separator */
           /* QAC EXPECT 5007; will not use wrapper */
           strcat(scopeStack, scopeSepp);
           /* Concatenate the scope name */
           /* QAC EXPECT 5007; will not use wrapper */
           strcat (scopeStack, Id);
           si++;
        }
        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a C identifier */
            Id = idl_CsharpId(name);
            /* allocate space for the current scope stack + the separator
               and the user identifier
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, (size_t)(
                             (int)strlen(scopeStack)+
                             (int)strlen(scopeSepp)+
                             (int)strlen(Id)+1));
           /* Concatenate the separator */
           /* QAC EXPECT 5007; will not use wrapper */
           strcat(scopeStack, scopeSepp);
           /* Concatenate the user identifier */
           /* QAC EXPECT 5007; will not use wrapper */
           strcat (scopeStack, Id);
        }
     } else {
    /* The stack is empty */
        if (name) {
            /* A user identifier is specified */
            scopeStack = os_strdup(idl_CsharpId(name));
        } else {
            /* make the stack represenation empty */
            scopeStack = os_strdup("");
        }
    }
    /* return the scope stack representation */
    return scopeStack;
}

/* Return the C# specific type identifier for the
   specified type specification
*/
c_char *
idl_CsharpTypeFromTypeSpec (
    idl_typeSpec typeSpec)
{
    c_char *typeName;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
        switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
            case idl_short:
                typeName = os_strdup("short");
                break;
            case idl_ushort:
                typeName = os_strdup("ushort");
                break;
            case idl_long:
                typeName = os_strdup("int");
                break;
            case idl_ulong:
                typeName = os_strdup("uint");
                break;
            case idl_longlong:
                typeName = os_strdup("long");
                break;
            case idl_ulonglong:
                typeName = os_strdup("ulong");
                break;
            case idl_float:
                typeName = os_strdup("float");
                break;
            case idl_double:
                typeName = os_strdup("double");
                break;
            case idl_char:
                typeName = os_strdup("char");
                break;
            case idl_string:
                typeName = os_strdup("string");
                break;
            case idl_boolean:
                typeName = os_strdup("bool");
                break;
            case idl_octet:
                typeName = os_strdup("byte");
                break;
            default:
                /* No processing required, empty statement to satisfy QAC */
                break;
            /* QAC EXPECT 2016; Default case must be empty here */
        }
        /* QAC EXPECT 3416; No side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        return idl_CsharpTypeFromTypeSpec (idl_typeSeqActual(idl_typeSeq (typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        return idl_CsharpTypeFromTypeSpec (idl_typeArrayActual(idl_typeArray (typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        return idl_CsharpTypeFromTypeSpec (idl_typeDefActual(idl_typeDef (typeSpec)));
    } else {
    /* if a user type is specified build it from its scope and its name.
       The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
       idl_tunion.
     */
        typeName = idl_scopeStackCsharp(
                idl_typeUserScope(idl_typeUser(typeSpec)),
                ".",
                idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
}

c_char *
idl_sequenceCsharpIndexString (
    idl_typeSpec typeSpec,
    SACS_INDEX_POLICY policy,
    int *indexStrLen)
{
    c_char *sequenceString, *insertPos;
    const c_char *indexStr;

    /* Determine the index-size of the current sequence. */
    indexStr = (policy == SACS_INCLUDE_INDEXES ? "0" : "");

    /* Dereference possible typedefs first. */
    typeSpec = idl_typeSeqType(idl_typeSeq(typeSpec));
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tseq) {
        *indexStrLen += (2 + strlen(indexStr)); /* Current index + '[' and ']'. */
        sequenceString = idl_sequenceCsharpIndexString(typeSpec, SACS_EXCLUDE_INDEXES, indexStrLen);
        insertPos = strchr(sequenceString, '[');
        snprintf(insertPos - 2 - strlen(indexStr), 2 + strlen(indexStr), "[%s]", indexStr);
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        *indexStrLen += (2 + strlen(indexStr)); /* Current index + '[' and ']'. */
        sequenceString = idl_arrayCsharpIndexString(typeSpec, SACS_EXCLUDE_INDEXES, indexStrLen);
        insertPos = strchr(sequenceString, '[');
        snprintf(insertPos - 2 - strlen(indexStr), 2 + strlen(indexStr), "[%s]", indexStr);
    } else {
        *indexStrLen += strlen(indexStr);
        sequenceString = os_malloc(*indexStrLen + 1);
        memset(sequenceString, '.', *indexStrLen);
        snprintf(sequenceString + *indexStrLen - 2 - strlen(indexStr), 3 + strlen(indexStr), "[%s]", indexStr);
    }
    return sequenceString;
}

static c_char *
idl_indexStr(
    idl_typeArray typeArray,
    SACS_INDEX_POLICY policy)
{
    c_char *indexStr = os_malloc(12); /* > MAX_INT_SIZE. */
    if (policy == SACS_INCLUDE_INDEXES) {
        c_long size = idl_typeArraySize (typeArray);
        snprintf(indexStr, 12, "%d", size);
    } else {
        indexStr[0] = '\0';
    }
    return indexStr;
}

c_char *
idl_arrayCsharpIndexString (
    idl_typeSpec typeSpec,
    SACS_INDEX_POLICY policy,
    int *indexStrLen)
{
    c_char *arrayString, *insertPos, *indexStr;

    /* Determine the index-size of the current array. */
    indexStr = idl_indexStr(idl_typeArray(typeSpec), policy);

    /* Dereference possible typedefs first. */
    typeSpec = idl_typeArrayType(idl_typeArray(typeSpec));
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        *indexStrLen += (1 + strlen(indexStr)); /* Current index + ','. */
        arrayString = idl_arrayCsharpIndexString(typeSpec, policy, indexStrLen);
        insertPos = strchr(arrayString, '[');
        snprintf(insertPos - 1 - strlen(indexStr), 2 + strlen(indexStr), "[%s,", indexStr);
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        *indexStrLen += (2 + strlen(indexStr)); /* Current index + '[' and ']'. */
        arrayString = idl_sequenceCsharpIndexString(typeSpec, SACS_EXCLUDE_INDEXES, indexStrLen);
        insertPos = strchr(arrayString, '[');
        snprintf(insertPos - 2 - strlen(indexStr), 2 + strlen(indexStr), "[%s]", indexStr);
    } else {
        *indexStrLen += strlen(indexStr);           /* Current index. */
        arrayString = os_malloc(*indexStrLen + 1);  /* Allow for the '\0' terminator. */
        memset(arrayString, '.', *indexStrLen);
        snprintf(arrayString + *indexStrLen - 2 - strlen(indexStr), 3 + strlen(indexStr), "[%s]", indexStr);
    }
    return arrayString;
}
