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
#include "idl_genSacHelper.h"
#include "idl_genCHelper.h"
#include "idl_tmplExp.h"

#include "os_heap.h"
#include "os_stdlib.h"

static c_char *idl_scopedSacSequenceTypeIdent (const idl_typeSpec typeSpec);

/** @todo Correct comment */

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
idl_scopedSacTypeIdent (
    const idl_typeSpec typeSpec)
{
    c_char scopedTypeIdent[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic ||
	idl_typeSpecType(typeSpec) == idl_tstruct ||
	idl_typeSpecType(typeSpec) == idl_tunion ||
	idl_typeSpecType(typeSpec) == idl_tenum) {
	/* QAC EXPECT 3416; No unexpected side effects here */
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sacTypeFromTypeSpec(idl_typeSpec(typeSpec)));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_scopeStack (
                idl_typeUserScope(idl_typeUser(typeSpec)),
                "_",
                idl_typeSpecName(idl_typeSpec(typeSpec))));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), idl_sequenceIdent(idl_typeSeq(typeSpec)));
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC messages */
    }

    return os_strdup(scopedTypeIdent);
}

/* Return the scoped actual type specification where for the user types,
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
static c_char *
idl_scopedSacSequenceElementTypeIdent (
    const idl_typeSpec typeSpec)
{
    c_char scopedTypeIdent[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
	case idl_short:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_short");
	    break;
	case idl_ushort:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_unsigned_short");
	    break;
	case idl_long:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_long");
	    break;
	case idl_ulong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_unsigned_long");
	    break;
	case idl_longlong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_long_long");
	    break;
	case idl_ulonglong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_unsigned_long_long");
	    break;
	case idl_float:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_float");
	    break;
	case idl_double:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_double");
	    break;
	case idl_char:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_char");
	    break;
	case idl_string:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_string");
	    break;
	case idl_boolean:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_boolean");
	    break;
	case idl_octet:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "DDS_octet");
	    break;
	default:
	   os_strncpy (scopedTypeIdent, "", (size_t)sizeof(scopedTypeIdent));
	}
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
	idl_typeSpecType(typeSpec) == idl_tunion ||
	idl_typeSpecType(typeSpec) == idl_tenum) {
	/* QAC EXPECT 3416; No unexpected side effects here */
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sacTypeFromTypeSpec(idl_typeSpec(typeSpec)));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	switch (idl_typeSpecType(idl_typeDefRefered (idl_typeDef(typeSpec)))) {
	case idl_tarray:
	case idl_tseq:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
		idl_sacTypeFromTypeSpec(typeSpec));
	    break;
	case idl_ttypedef:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	        idl_scopedSacSequenceTypeIdent(idl_typeDefRefered (idl_typeDef(typeSpec))));
	    break;
	default:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
		idl_sacTypeFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec))));
	}
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_scopedSacSequenceTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec))));
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sequenceIdent(idl_typeSeq(typeSpec)));
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC messages */
    }

    return os_strdup(scopedTypeIdent);
}

/* Return the scoped actual type specification where for the user types,
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
static c_char *
idl_scopedSacSequenceSubElementTypeIdent (
    const idl_typeSpec typeSpec)
{
    c_char scopedTypeIdent[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
	case idl_short:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "short");
	    break;
	case idl_ushort:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "unsigned_short");
	    break;
	case idl_long:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "long");
	    break;
	case idl_ulong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "unsigned_long");
	    break;
	case idl_longlong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "long_long");
	    break;
	case idl_ulonglong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "unsigned_long_long");
	    break;
	case idl_float:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "float");
	    break;
	case idl_double:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "double");
	    break;
	case idl_char:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "char");
	    break;
	case idl_string:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "string");
	    break;
	case idl_boolean:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "boolean");
	    break;
	case idl_octet:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "octet");
	    break;
	default:
	   os_strncpy (scopedTypeIdent, "", (size_t)sizeof(scopedTypeIdent));
	}
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
	idl_typeSpecType(typeSpec) == idl_tunion ||
	idl_typeSpecType(typeSpec) == idl_tenum) {
	/* QAC EXPECT 3416; No unexpected side effects here */
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sacTypeFromTypeSpec(idl_typeSpec(typeSpec)));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	switch (idl_typeSpecType(idl_typeDefRefered (idl_typeDef(typeSpec)))) {
	case idl_tarray:
	case idl_tseq:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
		idl_sacTypeFromTypeSpec(typeSpec));
	    break;
	case idl_ttypedef:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	        idl_scopedSacSequenceTypeIdent(idl_typeDefRefered (idl_typeDef(typeSpec))));
	    break;
	default:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
		idl_sacTypeFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec))));
	}
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_scopedSacSequenceTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec))));
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sequenceIdent(idl_typeSeq(typeSpec)));
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC messages */
    }

    return os_strdup(scopedTypeIdent);
}

/* Return the scoped actual type specification where for the user types,
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
static c_char *
idl_scopedSacSequenceTypeIdent (
    const idl_typeSpec typeSpec)
{
    c_char scopedTypeIdent[256];

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
	case idl_short:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "short");
	    break;
	case idl_ushort:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "unsigned_short");
	    break;
	case idl_long:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "long");
	    break;
	case idl_ulong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "unsigned_long");
	    break;
	case idl_longlong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "long_long");
	    break;
	case idl_ulonglong:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "unsigned_long_long");
	    break;
	case idl_float:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "float");
	    break;
	case idl_double:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "double");
	    break;
	case idl_char:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "char");
	    break;
	case idl_string:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "string");
	    break;
	case idl_boolean:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "boolean");
	    break;
	case idl_octet:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "octet");
	    break;
	default:
	   os_strncpy (scopedTypeIdent, "", (size_t)sizeof(scopedTypeIdent));
	}
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
	idl_typeSpecType(typeSpec) == idl_tunion ||
	idl_typeSpecType(typeSpec) == idl_tenum) {
	/* QAC EXPECT 3416; No unexpected side effects here */
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sacTypeFromTypeSpec(idl_typeSpec(typeSpec)));
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	switch (idl_typeSpecType(idl_typeDefRefered (idl_typeDef(typeSpec)))) {
	case idl_tarray:
	case idl_tseq:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
		idl_sacTypeFromTypeSpec(typeSpec));
	    break;
	case idl_ttypedef:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	        idl_scopedSacSequenceTypeIdent(idl_typeDefRefered (idl_typeDef(typeSpec))));
	    break;
	default:
	    snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
		idl_sacTypeFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec))));
	}
	/* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_scopedSacSequenceTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec))));
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	snprintf (scopedTypeIdent, (size_t)sizeof(scopedTypeIdent), "%s",
	    idl_sequenceIdent(idl_typeSeq(typeSpec)));
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC messages */
    }

    return os_strdup(scopedTypeIdent);
}

/* Return the standalone C specific type identifier for the
   specified type specification
*/
c_char *
idl_sacTypeFromTypeSpec (
    const idl_typeSpec typeSpec)
{
    c_char *typeName = NULL;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
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
	    typeName = os_strdup("DDS_string");
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
        /* QAC EXPECT 3416; No side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	/* sequence does not have an identification */
	typeName = os_strdup ("");
	printf ("idl_sacTypeFromTypeSpec: Unexpected sequence type handled\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
	typeName = os_strdup ("");
	printf ("idl_sacTypeFromTypeSpec: Unexpected array type handled\n");
    } else {
        /* if a user type is specified build it from its scope and its name.
	   The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
           idl_tunion.
	*/
        typeName = idl_scopeStackC (
            idl_typeUserScope(idl_typeUser(typeSpec)),
            "_",
            idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
}

static char *
idl_sequenceSubElementIdent (
    const idl_typeSpec typeSpec)
{
    char *sequenceType;
    char *sequenceName;
    int len;

    if (idl_typeSpecType(typeSpec) == idl_tseq) {
	sequenceType = idl_sequenceSubElementIdent (idl_typeSeqType (idl_typeSeq(typeSpec)));
        len = strlen(sequenceType) + strlen ("sequence_") + 1;
        sequenceName = os_malloc (len);
        snprintf (sequenceName, len, "%s%s", "sequence_", sequenceType);
    } else {
	sequenceName = idl_scopedSacSequenceSubElementTypeIdent(typeSpec);
    }
    return sequenceName;
}

char *
idl_sequenceElementIdent (
    const idl_typeSpec typeSpec)
{
    char *sequenceType;
    char *sequenceName;
    int len;

    if (idl_typeSpecType(typeSpec) == idl_tseq) {
	sequenceType = idl_sequenceSubElementIdent (idl_typeSeqType (idl_typeSeq(typeSpec)));
        len = strlen(sequenceType) + strlen ("DDS_sequence_") + 1;
        sequenceName = os_malloc (len);
        snprintf (sequenceName, len, "%s%s", "DDS_sequence_", sequenceType);
    } else {
	sequenceName = idl_scopedSacSequenceElementTypeIdent(typeSpec);
    }
    return sequenceName;
}

static char *
idl_sequenceSubIdent (
    const idl_typeSeq typeSeq)
{
    char *sequenceType;
    char *sequenceName;
    int len;

    if (idl_typeSpecType(idl_typeSeqType (typeSeq)) == idl_tseq) {
	sequenceType = idl_sequenceSubIdent (idl_typeSeq(idl_typeSeqType (typeSeq)));
        len = strlen(sequenceType) + strlen ("sequence_") + 1;
        sequenceName = os_malloc (len);
        snprintf (sequenceName, len, "%s%s", "sequence_", sequenceType);
    } else {
	sequenceType = idl_scopedSacSequenceTypeIdent(idl_typeSeqType (typeSeq));
        len = strlen(sequenceType) + strlen ("sequence_") + 1;
        sequenceName = os_malloc (len);
        snprintf (sequenceName, len, "%s%s", "sequence_", sequenceType);
    }
    return sequenceName;
}

char *
idl_sequenceIdent (
    const idl_typeSeq typeSeq)
{
    char *sequenceType;
    char *sequenceName;
    int len;

    if (idl_typeSpecType(idl_typeSeqType (typeSeq)) == idl_tseq) {
	sequenceType = idl_sequenceSubIdent (idl_typeSeq(idl_typeSeqType (typeSeq)));
        len = strlen(sequenceType) + strlen ("DDS_sequence_") + 1;
        sequenceName = os_malloc (len);
        snprintf (sequenceName, len, "%s%s", "DDS_sequence_", sequenceType);
    } else {
	sequenceType = idl_scopedSacSequenceTypeIdent(idl_typeSeqType (typeSeq));
        len = strlen(sequenceType) + strlen ("DDS_sequence_") + 1;
        sequenceName = os_malloc (len);
        snprintf (sequenceName, len, "%s%s", "DDS_sequence_", sequenceType);
    }
    return sequenceName;
}
