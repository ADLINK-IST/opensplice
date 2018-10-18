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
#include "idl_genCHelper.h"
#include "idl_genLanguageHelper.h"

#include "os_abstract.h"
#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"

static os_iter definitions = NULL;

/* Specify a list of all C keywords */
static const char *c_keywords[] = {
    "asm", "auto", "break", "case", "char", "const", "continue", "default",
    "do", "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while", "true", "false"
};

/* Translate an IDL identifier into a C language identifier.
 * The IDL specification states that all identifiers that match
 * a C keyword must be prepended by "_c_".
 */
c_char *
idl_cId (
    const char *identifier)
{
    c_long i;
    char *cId;

    /* search through the C keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < (c_long)(sizeof(c_keywords)/sizeof(c_char *)); i++) {
	/* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
	if (strcmp(c_keywords[i], identifier) == 0) {
	    /* If a keyword matches the specified identifier, prepend _c_ */
	    /* QAC EXPECT 5007; will not use wrapper */
	    cId = os_malloc(strlen(identifier)+1+3);
	    snprintf(cId, strlen(identifier)+1+3, "_c_%s", identifier);
	    return cId;
	}
    }
    /* No match with a keyword is found, thus return the identifier itself */
    cId = os_strdup(identifier);
    return cId;
    /* QAC EXPECT 2006; performance is selected above rules here */
}

/* Build a textual presenation of the provided scope stack taking the
 * C++ keyword identifier translation into account. Further the function
 * equals "idl_scopeStack".
 */
c_char *
idl_scopeStackC(
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
        scopeStack = os_strdup(idl_cId(idl_scopeElementName(idl_scopeIndexed(scope, si))));
        si++;
        while (si < sz) {
            /* Translate the scope name to a C identifier */
            Id = idl_cId(idl_scopeElementName(idl_scopeIndexed(scope, si)));
            /* allocate space for the current scope stack + the separator
             * and the next scope name
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
            /* Concatenate the separator */
            /* QAC EXPECT 5007; will not use wrapper */
            os_strcat(scopeStack, scopeSepp);
            /* Concatenate the scope name */
            /* QAC EXPECT 5007; will not use wrapper */
            os_strcat(scopeStack, Id);
            si++;
        }
        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a C identifier */
            Id = idl_cId(name);
            /* allocate space for the current scope stack + the separator
             * and the user identifier
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
            /* Concatenate the separator */
            /* QAC EXPECT 5007; will not use wrapper */
            os_strcat(scopeStack, scopeSepp);
            /* Concatenate the user identifier */
            /* QAC EXPECT 5007; will not use wrapper */
            os_strcat(scopeStack, Id);
        }
    } else {
	/* The stack is empty */
	if (name) {
	    /* A user identifier is specified */
	    scopeStack = os_strdup(idl_cId(name));
	} else {
	    /* make the stack represenation empty */
	    scopeStack = os_strdup("");
	}
    }
    /* return the scope stack representation */
    return scopeStack;
}

/* Return the C specific type identifier for the
 * specified type specification
 */
c_char *
idl_corbaCTypeFromTypeSpec(
    idl_typeSpec typeSpec)
{
    c_char *typeName = NULL;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
        if (idl_getCorbaMode() == IDL_MODE_ORB_BOUND) {
            switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
                case idl_short:
                    typeName = os_strdup("CORBA_Short");
                    break;
                case idl_ushort:
                    typeName = os_strdup("CORBA_UShort");
                    break;
                case idl_long:
                    typeName = os_strdup("CORBA_Long");
                    break;
                case idl_ulong:
                    typeName = os_strdup("CORBA_ULong");
                    break;
                case idl_longlong:
                    typeName = os_strdup("CORBA_LongLong");
                    break;
                case idl_ulonglong:
                    typeName = os_strdup("CORBA_ULongLong");
                    break;
                case idl_float:
                    typeName = os_strdup("CORBA_Float");
                    break;
                case idl_double:
                    typeName = os_strdup("CORBA_Double");
                    break;
                case idl_char:
                    typeName = os_strdup("CORBA_Char");
                    break;
                case idl_string:
                    typeName = os_strdup("char *");
                    break;
                case idl_boolean:
                    typeName = os_strdup("CORBA_Boolean");
                    break;
                case idl_octet:
                    typeName = os_strdup("CORBA_Octet");
                    break;
                default:
                    /* No processing required, empty statement to satisfy QAC */
                    break;
                    /* QAC EXPECT 2016; Default case must be empty here */
            }
        } else {
            switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
                case idl_short:
                    typeName = os_strdup("DDS_short");
                    break;
                case idl_ushort:
                    typeName = os_strdup("DDS_unsigned_short");
                    break;
                case idl_long:
                    typeName = os_strdup("DDS_long");
                    break;
                case idl_ulong:
                    typeName = os_strdup("DDS_unsigned_long");
                    break;
                case idl_longlong:
                    typeName = os_strdup("DDS_long_long");
                    break;
                case idl_ulonglong:
                    typeName = os_strdup("DDS_unsigned_long_long");
                    break;
                case idl_float:
                    typeName = os_strdup("DDS_float");
                    break;
                case idl_double:
                    typeName = os_strdup("DDS_double");
                    break;
                case idl_char:
                    typeName = os_strdup("DDS_char");
                    break;
                case idl_string:
                    typeName = os_strdup("char *");
                    break;
                case idl_boolean:
                    typeName = os_strdup("DDS_boolean");
                    break;
                case idl_octet:
                    typeName = os_strdup("DDS_octet");
                    break;
                default:
                    /* No processing required, empty statement to satisfy QAC */
                    break;
                    /* QAC EXPECT 2016; Default case must be empty here */
            }
        }
        /* QAC EXPECT 3416; No side effects here */
    } else if ((idl_typeSpecType(typeSpec) == idl_tseq) ||
            (idl_typeSpecType(typeSpec) == idl_tarray)) {
        /* sequence does not have an identification */
        typeName = os_strdup("");
        printf("idl_corbaTypeFromTypeSpec: Unexpected type handled\n");
    } else {
        /* if a user type is specified build it from its scope and its name.
         * The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
         * idl_tunion.
         */
        typeName = idl_scopeStackC(
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
}

c_char *
idl_genCLiteralValueImage(
    c_value literal,
    c_type type)
{
    c_char * valueImg = NULL;
    c_char *val2;
    int i;

    if (c_baseObject(type)->kind != M_ENUMERATION) {
        switch (literal.kind) {
        case V_OCTET:
            valueImg = os_malloc (40);
            snprintf(valueImg, 40, "%x", literal.is.Octet);
            break;
        case V_FLOAT:
        case V_DOUBLE:
            val2 = os_malloc(45);
            valueImg = os_malloc(45);
            snprintf(val2, 45, "%40.17g", literal.is.Double);
            i = 0;
            while (val2[i] == ' ') {
                i++;
            }
            os_strncpy(valueImg, &val2[i], 40);
            os_free(val2);
            if ((strchr(valueImg, '.') == NULL) && (strchr(valueImg, 'E') == NULL)) {
                strcat(valueImg, ".0");
            }
            break;
        case V_STRING:
            valueImg = os_malloc(strlen(literal.is.String)+3);
            snprintf(valueImg, strlen(literal.is.String)+3, "\"%s\"", literal.is.String);
            break;
        case V_BOOLEAN:
            valueImg = os_malloc(10);
            if (literal.is.Boolean) {
                snprintf(valueImg, 10, "%s", "TRUE");
            } else {
                snprintf (valueImg, 10, "%s", "FALSE");
            }
            break;
        case V_LONGLONG:
            valueImg = os_malloc(40);
            switch (c_primitive(type)->kind) {
            case P_SHORT:
                snprintf(valueImg, 40, "%hu", (c_short)literal.is.LongLong);
                break;
            case P_USHORT:
                snprintf(valueImg, 40, "%huU", (c_ushort)literal.is.LongLong);
                break;
            case P_LONG:
                if ((c_long)literal.is.LongLong != INT_MIN) {
                    snprintf(valueImg, 40, "%u", (c_long)literal.is.LongLong);
                } else {
                    // We cannot represent minus 2^31 directly, due to the fact that it
                    // is constructed in two steps:
                    // 1) the specification of the unsigned literal.
                    // 2) the application of the minus sign.
                    // However, due to the inclusion of number 0 the positive range
                    // is one lower than the negative range and therefore the most
                    // negative number cannot be expressed as unsigned literal without
                    // overflow (with subsequent compilation warning). Therefore we
                    // construct it in 3 steps:
                    // 1) Specification of most positive value
                    // 2) Application of the minus sign
                    // 3) Subtraction of 1 to reach most negative value.
                    snprintf(valueImg, 40, "2147483647 - 1");
                }
                break;
            case P_ULONG:
                snprintf(valueImg, 40, "%uU", (c_ulong)literal.is.LongLong);
                break;
            case P_LONGLONG:
                if ((c_longlong)literal.is.LongLong != LLONG_MIN) {
                    snprintf(valueImg, 40, "%"PA_PRIu64"LL", (c_longlong)literal.is.LongLong);
                } else {
                    // We cannot represent minus 2^63 directly, due to the fact that it
                    // is constructed in two steps:
                    // 1) the specification of the unsigned literal.
                    // 2) the application of the minus sign.
                    // However, due to the inclusion of number 0 the positive range
                    // is one lower than the negative range and therefore the most
                    // negative number cannot be expressed as unsigned literal without
                    // overflow (with subsequent compilation warning). Therefore we
                    // construct it in 3 steps:
                    // 1) Specification of most positive value
                    // 2) Application of the minus sign
                    // 3) Subtraction of 1 to reach most negative value.
                    snprintf(valueImg, 40, "9223372036854775807LL - 1LL");
                }
                break;
            case P_ULONGLONG:
                snprintf(valueImg, 40, "%"PA_PRIu64"ULL", (c_ulonglong)literal.is.LongLong);
                break;
            case P_CHAR:
                snprintf(valueImg, 40, "%u", (unsigned char)literal.is.LongLong);
                break;
            case P_OCTET:
                snprintf(valueImg, 40, "%u", (unsigned char)literal.is.LongLong);
            break;
            case P_ADDRESS:
                snprintf(valueImg, 40, PA_ADDRFMT, (PA_ADDRCAST)literal.is.LongLong);
                break;
            case P_UNDEFINED:
            case P_BOOLEAN:
            case P_WCHAR:
            case P_FLOAT:
            case P_DOUBLE:
            case P_VOIDP:
            case P_MUTEX:
            case P_LOCK:
            case P_COND:
            case P_COUNT:
            case P_PA_UINT32:
            case P_PA_UINTPTR:
            case P_PA_VOIDP:
                /* Do nothing */
                break;
            }
            break;
        case V_SHORT:
            valueImg = os_malloc(40);
            snprintf(valueImg, 40, "%hu", literal.is.Short);
            break;
        case V_LONG:
            valueImg = os_malloc(40);
            if ((c_long)literal.is.LongLong != INT_MIN) {
                snprintf(valueImg, 40, "%u", literal.is.Long);
            } else {
                // We cannot represent minus 2^31 directly, due to the fact that it
                // is constructed in two steps:
                // 1) the specification of the unsigned literal.
                // 2) the application of the minus sign.
                // However, due to the inclusion of number 0 the positive range
                // is one lower than the negative range and therefore the most
                // negative number cannot be expressed as unsigned literal without
                // overflow (with subsequent compilation warning). Therefore we
                // construct it in 3 steps:
                // 1) Specification of most positive value
                // 2) Application of the minus sign
                // 3) Subtraction of 1 to reach most negative value.
                snprintf(valueImg, 40, "2147483647 - 1");
            }
            break;
        case V_USHORT:
            valueImg = os_malloc(40);
            snprintf(valueImg, 40, "%huU", literal.is.UShort);
            break;
        case V_ULONG:
            valueImg = os_malloc(40);
            snprintf(valueImg, 40, "%uU", literal.is.ULong);
            break;
        case V_ULONGLONG:
            valueImg = os_malloc(40);
            snprintf(valueImg, 40, "%" PA_PRIu64 "ULL", literal.is.ULongLong);
            break;
        case V_ADDRESS:
            valueImg = os_malloc(40);
            snprintf(valueImg, 40, PA_ADDRFMT, (PA_ADDRCAST)literal.is.Address);
            break;
        case V_CHAR:
            valueImg = os_malloc(40);
            snprintf(valueImg, 40, "%u", (unsigned char)literal.is.Char);
            break;
        case V_UNDEFINED:
        case V_WCHAR:
        case V_WSTRING:
        case V_FIXED:
        case V_VOIDP:
        case V_OBJECT:
        case V_COUNT:
            /* Invalid types for literal constants*/
            /* FALL THROUGH */
        default:
            valueImg = NULL;
            break;
        }
    } else {
        valueImg = os_strdup(c_metaObject(c_constant(c_enumeration(type)->elements[literal.is.Long]))->name);
    }

    return valueImg;
}

void
idl_definitionClean (
    void
    )
{
    char *def;

    if (definitions) {
        while ((def = os_iterTakeFirst(definitions))) {
	    os_free(def);
        }
        os_iterFree(definitions);
	definitions = NULL;
    }
}

void
idl_definitionAdd (
    char *class,
    char *name
    )
{
    char *def;

    def = os_malloc(strlen(class) + strlen (name) + 1);
    os_strcpy(def, class);
    os_strcat(def, name);
    if (definitions == NULL) {
	definitions = os_iterNew(NULL);
    }
    os_iterInsert(definitions, def);
}

static os_equality
defName(
    const void *iterElem,
    const void *arg)
{
    if (strcmp((const char *)iterElem, (const char *)arg) == 0) {
	return OS_EQ;
    }
    return OS_NE;
}

int
idl_definitionExists(
    char *class,
    char *name)
{
    char *def;

    def = os_malloc(strlen(class) + strlen (name) + 1);
    os_strcpy(def, class);
    os_strcat(def, name);
    if (definitions != NULL) {
        if (os_iterResolve(definitions, defName, def) != NULL) {
	    os_free(def);
	    return 1;
        }
    }
    os_free(def);
    return 0;
}

c_char *
idl_genCConstantGetter(void)
{
    return NULL;
}
