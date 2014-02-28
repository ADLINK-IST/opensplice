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
#include "idl_genCxxHelper.h"
#include "idl_genLanguageHelper.h"
#include "os_heap.h"
#include "os_stdlib.h"

/* Specify a list of all C++ keywords */
static const char *cxx_keywords[74] = {
    /* QAC EXPECT 5007; Bypass qactools error */
    "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "class",
    "compl", "const", "const_cast", "continue", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit",
    "export", "extern", "false", "float", "for", "friend",
    "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "not", "not_eq", "operator", "or",
    "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
    "return", "short", "signed", "sizeof", "static", "static_cast",
    "struct", "switch", "template", "this", "throw", "true",
    "try", "typedef", "typeid", "typename", "union", "unsigned",
    "using", "virtual", "void", "volatile", "wchar_t", "while",
    "xor", "xor_eq"
};

/* Translate an IDL identifier into  a C++ language identifier.
   The IDL specification states that all identifiers that match
   a C++ keyword must be prepended by "_cxx_".
*/
c_char *
idl_cxxId(
    const char *identifier)
{
    c_long i;
    char *cxxId;

    /* search through the C++ keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < (c_long)(sizeof(cxx_keywords)/sizeof(c_char *)); i++) {
	/* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
	if (strcmp(cxx_keywords[i], identifier) == 0) {
	    /* If a keyword matches the specified identifier, prepend _cxx_ */
	    /* QAC EXPECT 5007; will not use wrapper */
	    cxxId = os_malloc((size_t)((int)strlen(identifier)+1+5));
	    snprintf(cxxId, (size_t)((int)strlen(identifier)+1+5), "_cxx_%s", identifier);
	    return cxxId;
	}
    }
    /* No match with a keyword is found, thus return the identifier itself */
    cxxId = os_strdup(identifier);
    return cxxId;
    /* QAC EXPECT 2006; performance is selected above rules here */
}

/* Build a textual presenation of the provided scope stack taking the
   C++ keyword identifier translation into account. Further the function
   equals "idl_scopeStack".
*/
c_char *
idl_scopeStackCxx(
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
        Id = idl_cxxId(idl_scopeElementName(idl_scopeIndexed(scope, si)));
        scopeStack = os_realloc(scopeStack, (size_t)(
                         (int)strlen(scopeStack)+
                         (int)strlen(scopeSepp)+
                         (int)strlen(Id)+1));
        os_strcat(scopeStack, Id);
        si++;
        while (si < sz) {
            /* Translate the scope name to a C++ identifier */
            Id = idl_cxxId(idl_scopeElementName(idl_scopeIndexed(scope, si)));
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
           os_strcat(scopeStack, Id);
           si++;
        }
        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a C++ identifier */
            Id = idl_cxxId(name);
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
            os_strcat(scopeStack, Id);
        }
    } else {
	/* The stack is empty */
	if (name) {
	    /* A user identifier is specified */
	    scopeStack = os_strdup(idl_cxxId(name));
	} else {
	    /* make the stack represenation empty */
	    scopeStack = os_strdup("");
	}
    }
    /* return the scope stack representation */
    return scopeStack;
}

static c_char *
standaloneTypeFromTypeSpec(
    idl_typeBasic t)
{
    c_char *typeName;

    typeName = NULL;
    switch (idl_typeBasicType(t)) {
    case idl_short:
        typeName = os_strdup("::DDS::Short");
    break;
    case idl_ushort:
        typeName = os_strdup("::DDS::UShort");
    break;
    case idl_long:
        typeName = os_strdup("::DDS::Long");
    break;
    case idl_ulong:
        typeName = os_strdup("::DDS::ULong");
    break;
    case idl_longlong:
        typeName = os_strdup("::DDS::LongLong");
    break;
    case idl_ulonglong:
        typeName = os_strdup("::DDS::ULongLong");
    break;
    case idl_float:
        typeName = os_strdup("::DDS::Float");
    break;
    case idl_double:
        typeName = os_strdup("::DDS::Double");
    break;
    case idl_char:
        typeName = os_strdup("::DDS::Char");
    break;
    case idl_string:
        typeName = os_strdup("char *");
    break;
    case idl_boolean:
        typeName = os_strdup("::DDS::Boolean");
    break;
    case idl_octet:
        typeName = os_strdup("::DDS::Octet");
    break;
    default:
        /* No processing required, empty statement to satisfy QAC */
    break;
    }
    return typeName;
}

static c_char *
corbaTypeFromTypeSpec(
    idl_typeBasic t)
{
    c_char *typeName;

    typeName = NULL;
    switch (idl_typeBasicType(t)) {
    case idl_short:
        typeName = os_strdup("::DDS::Short");
    break;
    case idl_ushort:
        typeName = os_strdup("::DDS::UShort");
    break;
    case idl_long:
        typeName = os_strdup("::DDS::Long");
    break;
    case idl_ulong:
        typeName = os_strdup("::DDS::ULong");
    break;
    case idl_longlong:
        typeName = os_strdup("::DDS::LongLong");
    break;
    case idl_ulonglong:
        typeName = os_strdup("::DDS::ULongLong");
    break;
    case idl_float:
        typeName = os_strdup("::DDS::Float");
    break;
    case idl_double:
        typeName = os_strdup("::DDS::Double");
    break;
    case idl_char:
        typeName = os_strdup("::DDS::Char");
    break;
    case idl_string:
        typeName = os_strdup("char *");
    break;
    case idl_boolean:
        typeName = os_strdup("::DDS::Boolean");
    break;
    case idl_octet:
        typeName = os_strdup("::DDS::Octet");
    break;
    default:
        /* No processing required, empty statement to satisfy QAC */
    break;
    }
    return typeName;
}

/* Return the C++ specific type identifier for the
   specified type specification
*/
c_char *
idl_corbaCxxTypeFromTypeSpec(
    idl_typeSpec typeSpec)
{
    c_char *typeName;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
        if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
           typeName = standaloneTypeFromTypeSpec(idl_typeBasic(typeSpec));
        } else {
            typeName = corbaTypeFromTypeSpec(idl_typeBasic(typeSpec));
        }
    } else if ((idl_typeSpecType(typeSpec) == idl_tseq) ||
	(idl_typeSpecType(typeSpec) == idl_tarray)) {
	/* sequence does not have an identification */
	typeName = os_strdup ("");
	printf ("idl_corbaCxxTypeFromTypeSpec: Unexpected type handled\n");
        assert(0);
    } else {
        /* if a user type is specified build it from its scope and its name.
	   The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
           idl_tunion.
	*/
        typeName = idl_scopeStackCxx(
            idl_typeUserScope(idl_typeUser(typeSpec)),
            "::",
            idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
}

c_char *
idl_genCxxConstantGetter(void)
{
    return NULL;
}

const c_char*
idl_isocppCxxStructMemberSuffix()
{
  return idl_getIsISOCpp() && idl_getIsISOCppTypes() ? "_" : "";
}
