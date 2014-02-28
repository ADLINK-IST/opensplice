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
#include "idl_genCHelper.h"
#include "idl_genLanguageHelper.h"

#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"

static os_iter definitions = NULL;

/* Specify a list of all C keywords */
static const char *c_keywords[33] = {
    "asm", "auto", "break", "case", "char", "const", "continue", "default",
    "do", "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while"
};

/* Translate an IDL identifier into a C language identifier.
   The IDL specification states that all identifiers that match
   a C keyword must be prepended by "_c_".
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
	    cId = os_malloc((size_t)((int)strlen(identifier)+1+3));
	    snprintf(cId, (size_t)((int)strlen(identifier)+1+3), "_c_%s", identifier);
	    return cId;
	}
    }
    /* No match with a keyword is found, thus return the identifier itself */
    cId = os_strdup(identifier);
    return cId;
    /* QAC EXPECT 2006; performance is selected above rules here */
}

/* Build a textual presenation of the provided scope stack taking the
   C++ keyword identifier translation into account. Further the function
   equals "idl_scopeStack".
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
            /* Translate the user identifier to a C identifier */
            Id = idl_cId(name);
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
   specified type specification
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
	   The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
           idl_tunion.
	*/
        typeName = idl_scopeStackC(
            idl_typeUserScope(idl_typeUser(typeSpec)),
            "_",
            idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
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
    void *iterElem,
    void *arg)
{
    if (strcmp((char *)iterElem, (char *)arg) == 0) {
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
