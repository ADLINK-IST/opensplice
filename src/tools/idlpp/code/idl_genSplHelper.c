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
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"

#include "os_stdlib.h"
#include "os_heap.h"

/* Print a BOUNDSCHECK out of range error message to the default output file
*/
void
idl_boundsCheckFail(
        enum idl_boundsCheckFailKind kind,
        idl_scope scope,
        idl_typeSpec type,
        const c_char* name) {
    c_char *scopeName;

    /* Create scoped name for type */
    scopeName = idl_scopeStack(scope, "::", NULL);

    switch(kind) {

    /* Value is a case */
    case CASE:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Case '%s.%s' of type '%s' is out of range.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;

    /* Value is a discriminator */
    case DISCRIMINATOR:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Discriminator '%s.%s' of type '%s' is out of range.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;

    /* Value is a member */
    case MEMBER:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Member '%s.%s' of type '%s' is out of range.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;

    /* Value is an element */
    case ELEMENT:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Element of '%s.%s' of type '%s' is out of range.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;
    }

    os_free(scopeName);
}

/* Print a BOUNDSCHECK NULL error message to the default output file
*/
void
idl_boundsCheckFailNull(
        enum idl_boundsCheckFailKind kind,
        idl_scope scope,
        idl_typeSpec type,
        const c_char* name) {
    c_char *scopeName;

    /* Create scoped name for type */
    scopeName = idl_scopeStack(scope, "::", NULL);

    switch(kind) {

    /* Value is a case */
    case CASE:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Case '%s.%s' of type '%s' is NULL.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;

    /* Value is a discriminator */
    case DISCRIMINATOR:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Discriminator '%s.%s' of type '%s' is NULL.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;

    /* Value is a member */
    case MEMBER:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Member '%s.%s' of type '%s' is NULL.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;

    /* Value is an element */
    case ELEMENT:
        idl_fileOutPrintf(idl_fileCur(), "OS_REPORT (OS_ERROR, \"copyIn\", 0,"\
            "\"Element of '%s.%s' of type '%s' is NULL.\");\n",
            scopeName,
            name,
            idl_typeSpecName(type));
        break;
    }

    os_free(scopeName);
}

/* Print the specified indentation the the default output file,
   the indent is specified in units of 4 space characters
*/
void
idl_printIndent (
    c_long indent)
{
    c_long i;

    for (i = 0; i < indent; i++) {
        idl_fileOutPrintf (idl_fileCur(), "    ");
    }
}

/* Return the type name from its type specification */
c_char *
idl_typeFromTypeSpec (
    const idl_typeSpec typeSpec)
{
    return idl_typeSpecName(typeSpec);
}

/* Return the scoped type name where for the user types,
   scopes are separated by "_" chracters.
*/
c_char *
idl_scopedTypeName (
    const idl_typeSpec typeSpec)
{
    const char *scopedTypeName;

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tseq) {
	/* Sequences map to c_sequence */
	scopedTypeName = "c_sequence";
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	/* For basic types take the corresponding type name supported by the splice database */
	/* QAC EXPECT 3416; No unexpected side effects here */
	if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
	    /* in case of bounded string the type name must become c_string 	*/
	    /* type name is of form "C_STRING<xx>"				*/
	    scopedTypeName = "c_string";
	} else {
	    scopedTypeName = idl_typeSpecName(idl_typeSpec(typeSpec));
	}
    } else {
	/* Build a name for user types from scope and its own name */
	scopedTypeName = idl_scopeStack (
            idl_typeUserScope(idl_typeUser(typeSpec)),
            "_",
            idl_typeSpecName(idl_typeSpec(typeSpec)));
    }

    return os_strdup(scopedTypeName);
}

/* Return the scoped type name where for the user types,
   scopes are separated by "_" chracters.
*/
c_char *
idl_scopedSplTypeName (
    const idl_typeSpec typeSpec)
{
    char scopedTypeName[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tseq) {
	/* Sequences map to c_sequence */
	os_strncpy (scopedTypeName, "c_sequence", sizeof(scopedTypeName));
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	/* For basic types take the corresponding type name supported by the splice database */
	/* QAC EXPECT 3416; No unexpected side effects here */
	if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
	    /* in case of bounded string the type name must become c_string 	*/
	    /* type name is of form "C_STRING<xx>"				*/
	   os_strncpy (scopedTypeName, "c_string", sizeof(scopedTypeName));
	} else {
	   os_strncpy (scopedTypeName, idl_typeSpecName(idl_typeSpec(typeSpec)), sizeof(scopedTypeName));
	}
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
	idl_typeSpecType(typeSpec) == idl_tunion) {
	snprintf (scopedTypeName, (size_t)sizeof(scopedTypeName), "struct _%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
	snprintf (scopedTypeName, (size_t)sizeof(scopedTypeName), "enum _%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
    } else {
	snprintf (scopedTypeName, (size_t)sizeof(scopedTypeName), "_%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
    }

    return os_strdup(scopedTypeName);
}

/* Return the scoped type specification where for the user types,
   scopes are separated by "_" chracters. 
   IDL strings (bounded and unbounded) are mapped on: c_string, other
       basic types are mapped on corresponding splice types
   IDL structures are identified by: struct <scoped-struct-name>
   IDL unions are identified by: struct <scoped-union-name> becuase
   the union mapping is:
       struct <union-name> {
           <tag-type> _d;
	   union {
		<union-case-specifications>
	   } _u;
	}
    IDL enumerations are identified by: enum <scoped-enum-name>
    IDL typedefs are formed by the scoped type name
    IDL sequences are mapped on: c_sequence
*/
c_char *
idl_scopedTypeIdent (
    const idl_typeSpec typeSpec)
{
    c_char scopedTypeIdent[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	/* QAC EXPECT 3416; No unexpected side effects here */
	if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "c_string");
	} else {
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	        idl_typeSpecName(idl_typeSpec(typeSpec)));
	}
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "enum %s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "struct %s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tunion) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "struct %s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "c_sequence");
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC messages */
    }

    return os_strdup(scopedTypeIdent);
}

/* Return the scoped type specification where for the user types,
   scopes are separated by "_" chracters. 
   IDL strings (bounded and unbounded) are mapped on: c_string, other
       basic types are mapped on corresponding splice types
   IDL structures are identified by: struct <scoped-struct-name>
   IDL unions are identified by: struct <scoped-union-name> becuase
   the union mapping is:
       struct <union-name> {
           <tag-type> _d;
	   union {
		<union-case-specifications>
	   } _u;
	}
    IDL enumerations are identified by: enum <scoped-enum-name>
    IDL typedefs are formed by the scoped type name
    IDL sequences are mapped on: c_sequence
*/

c_char *
idl_scopedSplTypeIdent (
    const idl_typeSpec typeSpec)
{
    c_char scopedTypeIdent[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	/* QAC EXPECT 3416; No unexpected side effects here */
	if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "c_string");
	} else {
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	        idl_typeSpecName(idl_typeSpec(typeSpec)));
	}
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "enum _%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "struct _%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tunion) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "struct _%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "_%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "c_sequence");
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC messages */
    }

    return os_strdup(scopedTypeIdent);
}
