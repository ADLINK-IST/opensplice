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
#include "idl_genSACSHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_scope.h"
#include "idl_tmplExp.h"
#include "idl_typeSpecifier.h"
#include "idl_keyDef.h"
#include "idl_genMetaHelper.h"

#include <ctype.h>
#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"
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
        void *_metaElmnt,
        void *args)
{
    idl_metaCsharp  *metaElmnt = _metaElmnt;
    c_ulong nrElements;
    if (!metaElmnt->descriptor)
        metaElmnt->descriptor = idl_cutXMLmeta(idl_genXMLmeta(metaElmnt->type), &nrElements);
}

void
idl_CsharpRemovePrefix (
    const c_char *prefix,
    c_char *name)
{
    unsigned int i, prefixMatch;

    /* if the name is not prefixed, do nothing */
    prefixMatch = TRUE;
    for (i = 0; i < strlen(prefix) && prefixMatch; i++) {
        if (toupper(prefix[i]) != toupper(name[i])) {
            prefixMatch = FALSE;
        }
    }

    if (prefixMatch) {
        c_char *p;
        unsigned int j = 0, newLength = 0;

        p = name + strlen(prefix);
        newLength = strlen(name) - strlen(prefix);

        /* (for loop over new element length)  */
        for (i = 0, j = 0; i <= newLength; i++, j++) {
            if (j == 0 && p[i] == '_') {
                /* On first underscore, skip. */
                name[j] = p[++i];
            } else {
                name[j] = p[i];
            }
        }
        name[j] = '\0';
    }
}

void
toPascalCase(c_char *name)
{
    unsigned int i, j, nrUnderScores;

    /* Determine number of '_' characters. */
    nrUnderScores = 0;
    for (i = 0; i < strlen(name); i++) {
        if (name[i] == '_') nrUnderScores++;
    }

    /* Now go to UpperCase when necessary.
     * (for loop includes '\0' terminator.)  */
    for (i = 0, j = 0; i <= strlen(name); i++, j++) {
        /* Start out with capital. */
        if (i == 0) {
            name[j] = toupper(name[i]);
        } else if (name[i] == '_') {
            /* On underscore, start new capital. */
            name[j] = toupper(name[++i]);
        } else {
            /* If underscores mark the occurrence of new words, then go to
             * lower-case for all the other characters.
             * In the other case, the name could already be in camelCase,
             * so copy the character as is.
             */
            if (nrUnderScores > 0) {
                name[j] = tolower(name[i]);
            } else {
                name[j] = name[i];
            }
        }
    }
}


/* Specify a list of all C# keywords */
static const c_char *Csharp_keywords[] = {
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
#define NR_CSHARP_KEYWORDS sizeof(Csharp_keywords)/sizeof(c_char *)

/* Translate an IDL identifier into a C# language identifier.
   The IDL specification often states that all identifiers that
   match a native keyword must be prepended by "_".
*/
c_char *
idl_CsharpId(
    const c_char *identifier,
    c_bool customPSM,
    c_bool isCType)
{
    c_ulong i, j, nrScopes, scopeNr, totalLength;
    c_char *cSharpId;
    c_char **scopeList;

    if (strcmp("", identifier) == 0) {
        return os_strdup("");
    }

    nrScopes = 1;
    /* Determine the amount of scopes by looking for the number of '.' chars. */
    for (i = 0; i < strlen(identifier); i++) {
        if (identifier[i] == '.') nrScopes++;
    }

    scopeList = (c_char **) os_malloc(nrScopes * sizeof(c_char *));
    for (i = 0, scopeNr = 0; i < strlen(identifier); i++, scopeNr++) {
        j = i;
        while (identifier[i] != '.' && identifier[i] != '\0') i++;
        scopeList[scopeNr] = os_malloc(i - j + 1);  /* Account for '\0'. */
        scopeList[scopeNr][0] = '\0';
        os_strncat(scopeList[scopeNr], &identifier[j], i - j);

        /* In case of the custom PSM mode, first go to PascalCase. */
        if (customPSM) {
            toPascalCase(scopeList[scopeNr]);
        }

        /* If the identifier represents a cType, then prepend it with a
         * double underscore ('__') to distinguish it from the corresponding
         * C# representation of the same datatype.
         */
        if (isCType && scopeNr == nrScopes - 1) /* Actual datatype is always in last scope. */
        {
            totalLength = i - j + 2 + 1; /* Account for '__' and '\0'. */
            cSharpId = os_malloc(totalLength);
            cSharpId[0] = '\0';
            os_strncat(cSharpId, "__", 2);
            os_strncat(cSharpId, scopeList[scopeNr], strlen(scopeList[scopeNr]));
            os_free(scopeList[scopeNr]);
            scopeList[scopeNr] = cSharpId;
        }

        /* search through the C# keyword list */
        /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
        for (j = 0; j < NR_CSHARP_KEYWORDS; j++) {
        /* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
            if (strcmp (Csharp_keywords[j], scopeList[scopeNr]) == 0) {
                /* Determine Escape character. */
                char escChar = customPSM ? '@' : '_';

                /* If a keyword matches the specified identifier, prepend _ */
                /* QAC EXPECT 5007; will not use wrapper */
                c_char *EscCsharpId = os_malloc((size_t)((int)strlen(scopeList[scopeNr])+1+1));
                snprintf(
                        EscCsharpId,
                        (size_t)((int)strlen(scopeList[scopeNr])+1+1),
                        "%c%s",
                        escChar,
                        scopeList[scopeNr]);
                os_free(scopeList[scopeNr]);
                scopeList[scopeNr] = EscCsharpId;
            }
        }
    }

    /* Calculate the total length of the resulting CSharp identifier.
     * Initialize to 'nrScopes' first, to account for the '.' between each
     * scope element and the '\0' terminator at the end.
     */
    totalLength = nrScopes;
    for (i = 0; i < nrScopes; i++) {
        totalLength += strlen(scopeList[i]);
    }
    cSharpId = os_malloc(totalLength);

    /* Concatenate all scopes together. */
    snprintf(cSharpId, strlen(scopeList[0]) + 1, "%s", scopeList[0]);
    os_free(scopeList[0]);
    for (i = 1; i < nrScopes; i++) {
        os_strncat(cSharpId, ".", 1);
        os_strncat(cSharpId, scopeList[i], strlen(scopeList[i]));
        os_free(scopeList[i]);
    }
    os_free(scopeList);

    return cSharpId;
}

/* This operation replaces the database scoping operator '::' in the scoped
 * name of a database type description with the '.' scoping operator in C#.
 */
void
idl_toCsharpScopingOperator(c_char *scopedName)
{
    unsigned int i, j;

    for (i = 0, j = 0; i <= strlen(scopedName); i++, j++) {
        /* Start looking for an occurrence of the '::' operator.  */
        if (scopedName[i] == ':' && scopedName[i + 1] == ':') {
            /* Replace the '::' with a single '.' */
            scopedName[j] = '.';
            i++;
        } else {
            scopedName[j] = scopedName[i];
        }
    }
}

/* Build a textual presentation of the provided datatype, including
 * its scope stack.
 */
c_char *
idl_scopeStackFromCType(c_type dataType)
{
    c_char *scopedName = os_strdup(c_metaScopedName(c_metaObject(dataType)));
    idl_toCsharpScopingOperator(scopedName);
    return scopedName;
}

/* Build a textual presentation of the provided scope stack taking the
 * C# keyword identifier translation into account. Further the function
 * equals "idl_scopeStack".
 */
c_char *
idl_scopeStackCsharp (
    idl_scope scope,
    const c_char *scopeSepp,
    const c_char *name)
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
        scopeStack = idl_CsharpId(idl_scopeElementName(idl_scopeIndexed(scope, si)), FALSE, FALSE);
        si++;
        while (si < sz) {
            /* Translate the scope name to a C identifier */
            Id = idl_CsharpId(idl_scopeElementName(idl_scopeIndexed(scope, si)), FALSE, FALSE);
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
           os_strcat(scopeStack, scopeSepp);
           /* Concatenate the scope name */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat (scopeStack, Id);
           si++;
        }
        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a C# identifier */
            Id = idl_CsharpId(name, FALSE, FALSE);
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
           os_strcat(scopeStack, scopeSepp);
           /* Concatenate the user identifier */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat (scopeStack, Id);
        }
     } else {
    /* The stack is empty */
        if (name) {
            /* A user identifier is specified */
            scopeStack = idl_CsharpId(name, FALSE, FALSE);
        } else {
            /* make the stack representation empty */
            scopeStack = os_strdup("");
        }
    }
    /* return the scope stack representation */
    return scopeStack;
}

/* Specify a 2-Dimensional list where the 1st dimension specifies the names
 * of all IDL types that are already predefined in the API, and the 2nd dimension
 * specifies the corresponding names used to represent these datatypes in C#.
 */
static const c_char *Csharp_predefined[][2] = {
    /* Predefined DDS types */
    { "DDS.Duration_t",         "DDS.Duration"          },
    { "DDS.Time_t",             "DDS.Time",             },
    { "DDS.InstanceHandle_t",   "DDS.InstanceHandle"    }
};
#define NR_PREDEFINED_DATATYPES sizeof(Csharp_predefined)/(2*sizeof(c_char *))

/* This function determines whether the requested datatype is already predefined
 * in the C# API. The list of predefined datatypes is mentioned in the
 * Csharp_predefined string array above. If name and scope that are passed
 * as parameters match a name in the Csharp_predefined list, this function
 * will return TRUE, otherwise it will return FALSE.
 */
c_bool
idl_isPredefined(
    const c_char *scopedName)
{
    c_ulong i;
    c_bool predefined = FALSE;

    /* Iterate over the list of predefined C# datatypes and compare them to the
     * current C# datatype. If there is a match, indicate so and stop the loop.
     */
    for (i = 0; i < NR_PREDEFINED_DATATYPES && !predefined; i++) {
        if (strcmp(scopedName, Csharp_predefined[i][0]) == 0) {
            predefined = TRUE;
        }
    }

    return predefined;
}


/* This function translates an IDL typename that has been pre-defined in the
 * C# API already into the name of its C# representation. Otherwise it returns
 * just the original IDL type name.
 */
const c_char *
idl_translateIfPredefined(
    const c_char *scopedName)
{
    c_ulong i;

    /* Iterate over the list of predefined C# datatypes and compare them to the
     * current IDL datatype. If there is a match, translate the IDL datatype
     * name into its corresponding C# datatype name.
     */
    for (i = 0; i < NR_PREDEFINED_DATATYPES; i++) {
        if (strcmp(scopedName, Csharp_predefined[i][0]) == 0) {
            return Csharp_predefined[i][1];
        }
    }

    return NULL;
}

/* Return the C# specific type identifier for the
   specified type specification. The substPredefs
   parameter determines whether the function should
   attempt to translate IDL types that already have
   a predefined representation into this predefined
   representation.
*/
c_char *
idl_CsharpTypeFromTypeSpec (
    idl_typeSpec typeSpec,
    c_bool customPSM)
{
    c_char *typeName = NULL;

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
        typeName = idl_CsharpTypeFromTypeSpec (
                idl_typeSeqActual(idl_typeSeq (typeSpec)),
                customPSM);
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        typeName = idl_CsharpTypeFromTypeSpec (
                idl_typeArrayActual(idl_typeArray (typeSpec)),
                customPSM);
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        os_char *tmp = idl_scopeStackCsharp(
                idl_typeUserScope(idl_typeUser(typeSpec)),
                ".",
                idl_typeSpecName(typeSpec));
        if (idl_isPredefined(tmp)) {
            /* idl_translateIfPredefined will release old value when required. */
            typeName = os_strdup(idl_translateIfPredefined(tmp));
        } else {
            typeName = idl_CsharpTypeFromTypeSpec (
                    idl_typeDefRefered(idl_typeDef(typeSpec)),
                    customPSM);
        }
        os_free(tmp);
    } else {
        /* if a user type is specified, build it from its scope and its name.
           The type should be one of idl_tenum, idl_tstruct, idl_tunion.
         */
        typeName = idl_scopeStackCsharp(
                idl_typeUserScope(idl_typeUser(typeSpec)),
                ".",
                idl_typeSpecName(typeSpec));
        if (idl_isPredefined(typeName)) {
            os_char *tmp = os_strdup(idl_translateIfPredefined(typeName));
            os_free(typeName);
            typeName = tmp;
        }

        /* If a customPSM is specified, translate the typeName into PascalCase. */
        if (customPSM) {
            toPascalCase(typeName);
        }

    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefore the total complexity is low */
}

c_char *
idl_sequenceCsharpIndexString (
    idl_typeSpec typeSpec,
    SACS_INDEX_POLICY policy,
    const char *seqLengthName)
{
    c_char *str, *postfix;
    const c_char *indexStr;
    int len;

    /* Determine the index-size of the current sequence. */
    if (policy == SACS_INCLUDE_INDEXES) {
        if (seqLengthName == NULL) {
            indexStr = "0";
        } else {
            indexStr = seqLengthName;
        }
    } else {
        indexStr = "";
    }

    /* Dereference possible typedefs first. */
    typeSpec = idl_typeSeqType(idl_typeSeq(typeSpec));
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tseq) {
        postfix = idl_sequenceCsharpIndexString (typeSpec, SACS_EXCLUDE_INDEXES, NULL);
        len = strlen (postfix) + strlen (indexStr) + 3; /* '['. ']' and '\0'*/
        str = os_malloc (len);
        assert (str);
        (void)snprintf (str, len, "[%s]%s", indexStr, postfix);
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        postfix = idl_arrayCsharpIndexString(typeSpec, SACS_EXCLUDE_INDEXES);
        len = strlen (postfix) + strlen (indexStr) + 3; /* '['. ']' and '\0'*/
        str = os_malloc (len);
        assert (str);
        (void)snprintf (str, len, "[%s]%s", indexStr, postfix);
    } else {
        postfix = NULL;
        len = strlen (indexStr) + 3;  /* '['. ']' and '\0'*/
        str = os_malloc (len);
        assert (str);
        (void)snprintf (str, len, "[%s]", indexStr);
    }

    if (postfix) {
        os_free (postfix);
    }

    return str;
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
    SACS_INDEX_POLICY policy)
{
    c_char *postfix, *str, *indexStr;
    int len;

    /* Determine the index-size of the current array. */
    indexStr = idl_indexStr(idl_typeArray(typeSpec), policy);

    /* Dereference possible typedefs first. */
    typeSpec = idl_typeArrayType(idl_typeArray(typeSpec));
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        postfix = idl_arrayCsharpIndexString(typeSpec, policy);
        len = strlen (postfix) + strlen (indexStr) + 3; /* '[', ',' and '\0' */
        str = os_malloc (len);
        assert (str);
        (void)snprintf (str, len, "[%s,%s", indexStr, postfix + 1);
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        postfix = idl_sequenceCsharpIndexString (typeSpec, SACS_EXCLUDE_INDEXES, NULL);
        len = strlen (postfix) + strlen (indexStr) + 3; /* '[', ']' and '\0'*/
        str = os_malloc (len);
        assert (str);
        (void)snprintf (str, len, "[%s]%s", indexStr, postfix);
    } else {
        postfix = NULL;
        len = strlen (indexStr) + 3; /* '[', ']' and '\0'*/
        str = os_malloc (len);
        assert (str);
        (void)snprintf (str, len, "[%s]", indexStr);
    }

    if (postfix) {
        os_free (postfix);
    }

    return str;
}
