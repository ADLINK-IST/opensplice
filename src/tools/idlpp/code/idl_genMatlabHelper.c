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

#include "idl_genMatlabHelper.h"
#include "idl_genSimulinkHelper.h"

#include "idl_tmplExp.h"

#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_string.h"
#include "os_errno.h"

#include <stdio.h>
#include <stdarg.h>


/* Specify a list of all C keywords */
static const char *matlab_keywords[] = {
    /* MATLAB keywords - output from command: iskeyword */
	    "break",
	    "case",
	    "catch",
	    "classdef",
	    "continue",
	    "else",
	    "elseif",
	    "end",
	    "for",
	    "function",
	    "global",
	    "if",
	    "otherwise",
	    "parfor",
	    "persistent",
	    "return",
	    "spmd",
	    "switch",
	    "try",
	    "while",
};

static idl_typeSpec
stripTypedefs(
		idl_typeSpec typeSpec)
{
	/* navigate through any obscuring typedefs */
	while(idl_ttypedef == idl_typeSpecType(typeSpec)) {
		typeSpec = idl_typeDefActual(idl_typeDef(typeSpec));
	}
	return typeSpec;
}

int
idl_matlabIsSequence(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tseq:
		return 1;
	case idl_tbasic:
	case idl_tenum:
	case idl_tstruct:
	case idl_tunion:
		return 0;
	case idl_tarray:
		return idl_matlabIsSequence(idl_typeArrayActual(idl_typeArray(typeSpec)));
	case idl_ttypedef:
		return idl_matlabIsSequence(idl_typeDefActual(idl_typeDef(typeSpec)));
	}
	return 0;
}

int
idl_matlabIsArray(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tseq:
		return idl_matlabIsArray(idl_typeSeqActual(idl_typeSeq(typeSpec)));
	case idl_tbasic:
	case idl_tenum:
	case idl_tstruct:
	case idl_tunion:
		return 0;
	case idl_tarray:
		return 1;
	case idl_ttypedef:
		return idl_matlabIsArray(idl_typeDefActual(idl_typeDef(typeSpec)));
	}
	return 0;
}


idl_typeSpec
idl_matlabFindArrayWrappedType(idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tarray:
		return idl_matlabFindArrayWrappedType(idl_typeArrayType(idl_typeArray(typeSpec)));
	case idl_ttypedef:
		return idl_matlabFindArrayWrappedType(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return typeSpec;
}

idl_typeSpec
idl_matlabFindSequenceWrappedType(idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tseq:
		return idl_typeSeqType(idl_typeSeq(typeSpec));
	case idl_ttypedef:
		return idl_matlabFindSequenceWrappedType(idl_typeDefActual(idl_typeDef(typeSpec)));
	case idl_tarray:
		return idl_matlabFindSequenceWrappedType(idl_typeArrayActual(idl_typeArray(typeSpec)));
	default:
		break;
	}
	return typeSpec;
}

int
idl_matlabIsArrayOfSequences(
		idl_typeSpec typeSpec)
{
	if(idl_matlabIsArray(typeSpec)) {
		idl_typeSpec wrapped = idl_matlabFindArrayWrappedType(typeSpec);
		return idl_matlabIsSequence(wrapped);
	} else {
		return 0;
	}
}

int
idl_scopeIsInStruct(
		idl_scope scope)
{
	return idl_tStruct == idl_scopeElementType(idl_scopeCur(scope));
}

int
idl_matlabIsStruct(idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tstruct:
		return 1;
	case idl_tarray:
		return idl_matlabIsStruct(idl_typeArrayType(idl_typeArray(typeSpec)));
	case idl_tseq:
		return idl_matlabIsStruct(idl_typeSeqType(idl_typeSeq(typeSpec)));
	case idl_ttypedef:
		return idl_matlabIsStruct(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return 0;
}

int
idl_matlabIsEnum(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tenum:
		return 1;
	case idl_tarray:
		return idl_matlabIsEnum(idl_typeArrayType(idl_typeArray(typeSpec)));
	case idl_tseq:
		return idl_matlabIsEnum(idl_typeSeqType(idl_typeSeq(typeSpec)));
	case idl_ttypedef:
		return idl_matlabIsEnum(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return 0;
}

int
idl_matlabIsNotMultiValued(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tarray:
	case idl_tseq:
		return 0;
	case idl_ttypedef:
		return idl_matlabIsNotMultiValued(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return 1;
}

int
idl_matlabIsString(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tbasic:
		return idl_string == idl_typeBasicType(idl_typeBasic(typeSpec));
	case idl_tarray:
		return idl_matlabIsString(idl_typeArrayType(idl_typeArray(typeSpec)));
	case idl_ttypedef:
		return idl_matlabIsString(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return 0;

}

c_ulong
idl_matlabStringBound(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tbasic:
		if(idl_string == idl_typeBasicType(idl_typeBasic(typeSpec))) {
			return idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
		} else {
			return 0;
		}
	case idl_tarray:
		return idl_matlabStringBound(idl_typeArrayType(idl_typeArray(typeSpec)));
	case idl_ttypedef:
		return idl_matlabStringBound(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return 0;
}

c_ulong
idl_matlabSequenceBound(
		idl_typeSpec typeSpec)
{
	switch(idl_typeSpecType(typeSpec)) {
	case idl_tseq:
		return idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
	case idl_tarray:
		return idl_matlabStringBound(idl_typeArrayType(idl_typeArray(typeSpec)));
	case idl_ttypedef:
		return idl_matlabStringBound(idl_typeDefActual(idl_typeDef(typeSpec)));
	default:
		break;
	}
	return 0;
}

static void
idl_matlabNumel(
		idl_typeSpec typeSpec,
		c_ulong *numel)
{
	idl_type type = idl_typeSpecType(typeSpec);

	switch(type) {
	case idl_tarray:
		idl_matlabNumel(idl_typeArrayType(idl_typeArray(typeSpec)), numel);
		*numel *= idl_typeArraySize(idl_typeArray(typeSpec));
		break;
	case idl_ttypedef:
		idl_matlabNumel(idl_typeDefRefered(idl_typeDef(typeSpec)), numel);
		break;
	default:
		break;
	}
}

int
idl_matlabShouldBeCellArray(
		idl_typeSpec typeSpec)
{
	const int NO = 0;
	const int YES = 1;
	const int IF_WRAPPED = 2;
	const int IF_SEQUENCE_WRAPPED = 3;

	idl_type type = idl_typeSpecType(typeSpec);
	switch(type) {
	case idl_tbasic:
		return idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string
				? IF_WRAPPED : NO;
	case idl_tenum:
	case idl_tstruct:
	case idl_tunion:
		return NO;
	case idl_ttypedef:
		return idl_matlabShouldBeCellArray(idl_typeDefActual(idl_typeDef(typeSpec)));
	case idl_tseq:
	{
		idl_typeSpec wrappedType = idl_typeSeqType(idl_typeSeq(typeSpec));
		int wrappedTypeDecision = idl_matlabShouldBeCellArray(wrappedType);
		if(wrappedTypeDecision == YES
				|| wrappedTypeDecision == IF_SEQUENCE_WRAPPED
				|| wrappedTypeDecision == IF_WRAPPED) {
			return YES;
		} else {
			return NO;
		}
	}
	case idl_tarray:
	{
		idl_typeSpec wrappedType = idl_typeArrayActual(idl_typeArray(typeSpec));
		int wrappedTypeDecision = idl_matlabShouldBeCellArray(wrappedType);
		if(wrappedTypeDecision == YES || wrappedTypeDecision == IF_WRAPPED) {
			return YES;
		} else {
			return IF_SEQUENCE_WRAPPED;
		}
	}
	}

	return NO;
}

static int
is_string(idl_typeSpec typeSpec)
{
	typeSpec = stripTypedefs(typeSpec);
	return idl_tbasic == idl_typeSpecType(typeSpec)
			&& idl_string == idl_typeBasicType(idl_typeBasic(typeSpec));

}

static int
forcesSequenceToCell(idl_typeSpec typeSpec)
{
	idl_type type;

	typeSpec = stripTypedefs(typeSpec);
	type = idl_typeSpecType(typeSpec);
	return is_string(typeSpec) || idl_tseq == type || idl_tarray == type;
}

c_char *
idl_matlabFieldInitialization(
		idl_typeSpec typeSpec)
{
	char buffer[256];
	c_char *mlType = idl_matlabTypeName(typeSpec);
	char *dims_string = idl_matlabFormattedDimensions(typeSpec);
	char *init_string;
	idl_type type;

	typeSpec = stripTypedefs(typeSpec);

	type = idl_typeSpecType(typeSpec);
	if(idl_tseq == type) {
		if(forcesSequenceToCell(idl_typeSeqType(idl_typeSeq(typeSpec)))) {
			init_string = "SEQUENCE";
		} else {
			init_string = mlType;
			os_free((void*)dims_string);
			dims_string = os_strdup("0");
		}
	} else if(idl_tarray == type) {
		idl_typeSpec wrappedType = idl_typeArrayActual(idl_typeArray(typeSpec));
		if(is_string(wrappedType)) {
			init_string = "STRING";
		} else if(idl_matlabIsSequence(wrappedType)) {
			init_string = "SEQUENCE";
		} else {
			init_string = mlType;
		}
	} else if(is_string(typeSpec)) {
		init_string = "STRING";
	} else {
		init_string = mlType;
	}

	os_sprintf(buffer, "Vortex.init_field('%s', %s)", init_string, dims_string);

	os_free((void*)mlType);
	os_free((void*)dims_string);
	return os_strdup(buffer);
}
/* Return the matlab type name for the given typeSpec */
c_char *
idl_matlabTypeName(
		idl_typeSpec typeSpec
		)
{
	idl_type type = idl_typeSpecType(typeSpec);
	switch(type) {
	case idl_tbasic:
		{
			idl_typeBasic typeBasic = idl_typeBasic(typeSpec);
			idl_basicType basicType = idl_typeBasicType(typeBasic);
			switch(basicType) {
			case idl_boolean:
				return os_strdup("logical");
			case idl_string:
				return os_strdup("char");
			case idl_char:
				return os_strdup("int8");
			case idl_octet:
				return os_strdup("uint8");
			case idl_short:
				return os_strdup("int16");
			case idl_ushort:
				return os_strdup("uint16");
			case idl_long:
				return os_strdup("int32");
			case idl_ulong:
				return os_strdup("uint32");
			case idl_longlong:
				return os_strdup("int64");
			case idl_ulonglong:
				return os_strdup("uint64");
			case idl_float:
				return os_strdup("single");
			case idl_double:
				return os_strdup("double");
			}
		}
		break;
	case idl_tstruct:
	case idl_tenum:
	{
		idl_typeUser typeUser = idl_typeUser(typeSpec);
		return idl_scopeStackMatlab(idl_typeUserScope(typeUser), ".", idl_typeSpecName(typeSpec));
	}
	case idl_tarray:
	{
		idl_typeArray typeArray = idl_typeArray(typeSpec);
		idl_typeSpec actualType = idl_typeArrayActual(typeArray);
		return idl_matlabTypeName(actualType);
	}
	case idl_tseq:
	{
		idl_typeSeq typeSeq = idl_typeSeq(typeSpec);
		return idl_matlabTypeName(idl_typeSeqActual(typeSeq));
	}
	case idl_ttypedef:
	{
		idl_typeDef typeDef = idl_typeDef(typeSpec);
		return idl_matlabTypeName(idl_typeDefActual(typeDef));
	}
	case idl_tunion:
		printf("idl_genMatlabHelper.c:idl_matlabTypeName: Union types are unsupported"
                " for MATLAB binding.\n");
		exit(1);
	}
	return os_strdup(""); /* won't get here, but keep the compiler happy */
}

static c_char *
idl_matlabTypeInfoKind(
		idl_typeSpec typeSpec)
{
	idl_type type = idl_typeSpecType(typeSpec);
	switch(type) {
	case idl_tbasic:
		{
			idl_typeBasic typeBasic = idl_typeBasic(typeSpec);
			idl_basicType basicType = idl_typeBasicType(typeBasic);
			switch(basicType) {
			case idl_boolean:
				return os_strdup("dds_boolean");
			case idl_string:
			{
				c_ulong maxlen = idl_typeBasicMaxlen(typeBasic);
				if(maxlen == 0) {
					return os_strdup("dds_string");
				} else {
					return os_strdup("dds_bstring");
				}
			}
			case idl_char:
				return os_strdup("dds_char");
			case idl_octet:
				return os_strdup("dds_octet");
			case idl_short:
				return os_strdup("dds_short");
			case idl_ushort:
				return os_strdup("dds_ushort");
			case idl_long:
				return os_strdup("dds_long");
			case idl_ulong:
				return os_strdup("dds_ulong");
			case idl_longlong:
				return os_strdup("dds_longlong");
			case idl_ulonglong:
				return os_strdup("dds_ulonglong");
			case idl_float:
				return os_strdup("dds_float");
			case idl_double:
				return os_strdup("dds_double");
			}
		}
		break;
	case idl_tstruct:
		return os_strdup("dds_struct");
	case idl_tenum:
		return os_strdup("dds_enum");
	case idl_tarray:
	{
		idl_typeArray typeArray = idl_typeArray(typeSpec);
		return idl_matlabTypeInfoKind(idl_typeArrayActual(typeArray));
	}
	case idl_tseq:
	{
		idl_typeSeq typeSeq = idl_typeSeq(typeSpec);
		c_ulong maxSize = idl_typeSeqMaxSize(typeSeq);
		if(maxSize == 0) {
			return os_strdup("dds_sequence");
		} else {
			return os_strdup("dds_bsequence");
		}
	}
	case idl_ttypedef:
	{
		idl_typeDef typeDef = idl_typeDef(typeSpec);
		return idl_matlabTypeInfoKind(idl_typeDefActual(typeDef));
	}
	case idl_tunion:
        printf("idl_genMatlabHelper.c:idl_matlabTypeInfoKind: Union types are unsupported"
                " for MATLAB binding.\n");
		exit(1);
	}
	return os_strdup(""); /* won't get here, but keep the compiler happy */
}


static c_ulong
idl_matlabNDimensions(
		idl_typeSpec typeSpec)
{
	idl_type type = idl_typeSpecType(typeSpec);
	if(type == idl_tarray) {
		idl_typeArray typeArray = idl_typeArray(typeSpec);
		idl_typeSpec wrappedTypeSpec = idl_typeArrayType(typeArray);
		idl_type wrappedType = idl_typeSpecType(wrappedTypeSpec);
		if(wrappedType == idl_tarray) {
			return 1 + idl_matlabNDimensions(wrappedTypeSpec);
		} else {
			return 1;
		}
	} else if(type == idl_ttypedef) {
		return idl_matlabNDimensions(idl_typeDefRefered(idl_typeDef(typeSpec)));
	} else {
		return 1;
	}
}

static void
idl_matlabGetDimensions(
		idl_typeSpec typeSpec,
		c_ulong dimensions[])
{
	if(idl_typeSpecType(typeSpec) == idl_tarray) {
		idl_typeArray typeArray = idl_typeArray(typeSpec);
		idl_typeSpec wrappedTypeSpec = idl_typeArrayType(typeArray);
		idl_type wrappedType = idl_typeSpecType(wrappedTypeSpec);

		dimensions[0] = idl_typeArraySize(typeArray);
		if(wrappedType == idl_tarray) {
			idl_matlabGetDimensions(wrappedTypeSpec, &dimensions[1]);
		}
	} else if(idl_typeSpecType(typeSpec) == idl_ttypedef) {
		idl_matlabGetDimensions(idl_typeDefRefered(idl_typeDef(typeSpec)), dimensions);
	} else {
		dimensions[0] = 1;
	}
}

c_char *
idl_matlabFormattedDimensions(
		idl_typeSpec typeSpec)
{
	c_ulong ndims = idl_matlabNDimensions(typeSpec);
	c_ulong *dims = (c_ulong *)os_malloc(ndims * sizeof(c_ulong));
	c_char *formatted_dims = NULL;

	idl_matlabGetDimensions(typeSpec, dims);
	formatted_dims = simulink_formatDimensions(dims, ndims);
	os_free((void*)dims);

	return formatted_dims;
}

/* Return matlab values for 'type info' entries */
void
idl_streamOutTypeInfoEntries(
		idl_streamOut so,
		const char *name,
		idl_typeSpec typeSpec)
{
#define TYPE_INFO_FORMAT \
	"      case '%s'\n"\
	"        info = struct('datatype', '%s', ...\n"\
	"          'kind', ml_type_info_kind.%s, ...\n"\
	"          'bound', %" PA_PA_PRIu32 ", ...\n"\
	"          'numel', %" PA_PA_PRIu32 ", ...\n"\
	"          'ndims', %" PA_PA_PRIu32 ", ...\n"\
	"          'size', %s);\n"

	c_char *typeInfoKind = idl_matlabTypeInfoKind(typeSpec);
	c_char *mlTypeName = idl_matlabTypeName(typeSpec);
	c_ulong numel = 1;
	c_ulong ndims = idl_matlabNDimensions(typeSpec);
	c_char *dims_ml_str = idl_matlabFormattedDimensions(typeSpec);

	c_ulong bound = idl_matlabIsString(typeSpec) ? idl_matlabStringBound(typeSpec) :
			idl_matlabIsSequence(typeSpec) ? idl_matlabSequenceBound(typeSpec) : 0;

	idl_matlabNumel(typeSpec, &numel);

	idl_streamOutPrintf(so, TYPE_INFO_FORMAT,
			name, mlTypeName, typeInfoKind, bound, numel, ndims, dims_ml_str);

	if(idl_matlabIsSequence(typeSpec)) {
		int i = 1;
		char *wrapped_name = (char *)os_malloc(strlen(name) + sizeof(" wrapped ") + 10);
		idl_typeSpec currentTypeSpec = typeSpec;
		do {
			idl_typeSpec wrappedTypeSpec = idl_matlabFindSequenceWrappedType(currentTypeSpec);
			c_char *wrapped_mlTypeName = idl_matlabTypeName(wrappedTypeSpec);
			c_char *wrapped_typeInfoKind = idl_matlabTypeInfoKind(wrappedTypeSpec);
			c_char *wrapped_dims_ml_str = idl_matlabFormattedDimensions(wrappedTypeSpec);
			c_ulong wrapped_bound = idl_matlabIsString(wrappedTypeSpec) ? idl_matlabStringBound(wrappedTypeSpec) :
					idl_matlabIsSequence(wrappedTypeSpec) ? idl_matlabSequenceBound(wrappedTypeSpec) : 0;
			c_ulong wrapped_numel = 1;
			idl_matlabNumel(wrappedTypeSpec, &wrapped_numel);
			os_sprintf(wrapped_name,"%s wrapped %d", name, i++);
			idl_streamOutPrintf(so, TYPE_INFO_FORMAT,
					wrapped_name,
					wrapped_mlTypeName,
					wrapped_typeInfoKind,
					wrapped_bound,
					wrapped_numel,
					idl_matlabNDimensions(wrappedTypeSpec),
					wrapped_dims_ml_str);
			os_free((void*)wrapped_mlTypeName);
			os_free((void*)wrapped_dims_ml_str);

			if(idl_matlabIsSequence(wrappedTypeSpec)) {

			}
			currentTypeSpec = wrappedTypeSpec;
		} while (idl_matlabIsSequence(currentTypeSpec));
		os_free((void*)wrapped_name);
	}
	os_free((void*)mlTypeName);
	os_free((void*)typeInfoKind);
	os_free((void*)dims_ml_str);

}
/* Translate an IDL identifier into a MATLAB language identifier.
 * All identifiers that match a MATLAB keyword must be append by "_".
 */
c_char *
idl_matlabId(
    const char *identifier)
{
    size_t i;
    char *matlabId;
    char *packageEnding;

    /* search through the Java keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < sizeof(matlab_keywords) / sizeof(c_char *); i++) {
            /* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
            if (strcmp(matlab_keywords[i], identifier) == 0) {
                    /* If a keyword matches the specified identifier, append _ */
                    /* QAC EXPECT 5007; will not use wrapper */
                    matlabId = os_malloc(strlen(identifier) + 1 + 1);
                    snprintf(matlabId, strlen(identifier) + 1 + 1, "%s_", identifier);
                    return matlabId;
            }
    }
    /* Check for reserved endings
      * - <type>Package
      */
    packageEnding = strstr("Package", identifier);
    if ((packageEnding && strcmp(packageEnding, "Package") == 0)) {
            matlabId = os_malloc(strlen(identifier) + 1 + 1);
            snprintf(matlabId, strlen(identifier) + 1 + 1, "%s_", identifier);
    } else {
            /* No match with a keyword is found, thus return the identifier itself */
            matlabId = os_strdup(identifier);
    }
    return matlabId;
    /* QAC EXPECT 2006; performance is selected above rules here */
}


static c_char *
idl_scopeMatlabElementName (
    idl_scopeElement scope)
{
    c_char *scopeName;
    c_char *scopeMatlabName;

	scopeName = idl_scopeElementName(scope);
	if ((idl_scopeElementType(scope) == idl_tStruct)
			|| (idl_scopeElementType(scope) == idl_tUnion)) {
		scopeMatlabName = os_malloc(strlen(scopeName) + 8);
		sprintf(scopeMatlabName, "%sPackage", scopeName);
	} else {
		scopeMatlabName = os_strdup(scopeName);
	}
	return scopeMatlabName;
}

/* Build a textual presentation of the provided scope stack taking the
 * Matlab keyword identifier translation into account. Further the function
 * equals "idl_scopeStack".
 */
c_char *
idl_scopeStackMatlab (
    idl_scope scope,
    const char *scopeSepp,
    const char *name)
{
    c_long si;
    c_long sz;
    c_char *scopeStack = NULL;
    c_char *Id;
    c_bool isFsPath;

    assert (scopeSepp != NULL);
    isFsPath = (strcmp(scopeSepp,".") != 0);
    scopeStack = os_strdup ("");

    for (si = 0, sz = idl_scopeStackSize(scope); si < sz; si++) {
        size_t slen;
        c_char *mElName = idl_scopeMatlabElementName(idl_scopeIndexed(scope, si));
        /* Translate the scope name to a C identifier */
        Id = idl_matlabId(mElName);
        os_free(mElName);
        /* allocate space for the current scope stack + the separator
         * and the next scope name
         */
        /* QAC EXPECT 5007; will not use wrapper */
        slen = strlen (scopeStack) + strlen (scopeSepp) + strlen (Id)
        		+ (isFsPath ? 1 : 0) /* add room for the + prefix */;
        scopeStack = os_realloc(scopeStack, slen + 1);
        /* Concatenate the separator */
        /* QAC EXPECT 5007; will not use wrapper */
        if (strlen(scopeStack)) {
            os_strcat(scopeStack, scopeSepp);
        }
        if (isFsPath) {
        	os_strcat(scopeStack, "+");
        }
        /* Concatenate the scope name */
        /* QAC EXPECT 5007; will not use wrapper */
        os_strcat (scopeStack, Id);
        os_free(Id);
    }

    if (name) {
        /* A user identifier is specified */
        /* Translate the user identifier to a Java identifier */
        Id = idl_matlabId(name);
        /* allocate space for the current scope stack + the separator
         * and the user identifier
         */
        /* QAC EXPECT 5007; will not use wrapper */
        scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
        /* Concatenate the separator */
        /* QAC EXPECT 5007; will not use wrapper */
        if (strlen(scopeStack)) {
            os_strcat(scopeStack, scopeSepp);
        }
        /* Concatenate the user identifier */
        /* QAC EXPECT 5007; will not use wrapper */
        os_strcat (scopeStack, Id);
        os_free(Id);
    }

    /* return the scope stack representation */
    return scopeStack;
}

static int
idl_createDir (
    os_char* fname)
{
    char *pathName;
    os_result statRes;
    struct os_stat_s statbuf;
    char* outdir;
    os_char* stackScope;
    os_char* token;
    const os_char* osSep;

    /* to allow nested modules which may potentially lead to very long paths,
     * pathName is not allocated fixed-size on stack
     * but dynamically on heap
     */
    pathName = os_malloc(1);
    pathName[0] = '\0';
    outdir = idl_dirOutCur();
    if(outdir){
        pathName = os_realloc(pathName, strlen(outdir) + strlen(os_fileSep()) + 1);
        os_sprintf(pathName, "%s%s", outdir, os_fileSep());
    }
    /* make sure that we replace the os file seperator with a simple character
     * like '/', this will allow us to parse easier later.
     */
    osSep = os_fileSep();
    stackScope = os_str_replace (fname, osSep, "/", 0);

    if(stackScope[0] != '\0'){ /* strlen(stackScope) > 0 */
        do
        {
            token = strchr(stackScope, '/');
            /* make sure this is not the last part of the file name, for example
             * for the file name org/opensplice/foo.java we only want to create
             * directories for org and opensplice, the foo.java part is not a
             * directory. So we can simply state if no '/' char can be
             * found then we must be at the end part of the file name!
             */
            if(token)
            {
                *token = '\0';
                token++;
                /* reallocate pathName to include stackScope */
                pathName = os_realloc(pathName, strlen(pathName) + strlen(stackScope) + 1);
                os_strcat (pathName, stackScope);
                stackScope = token;
                statRes = os_stat (pathName, &statbuf);
                if (statRes == os_resultFail) {
                    /* @todo                                                      */
                    /* Assume the file does not exist. On some platforms          */
                    /* a check to see if errno == ENOENT would be more conclusive */
                    /* That fails on WIN32 however because stat is not fully      */
                    /* compatible. Only an os_stat implementation can solve that. */

                    os_mkdir(pathName, 0777);
                    statRes = os_stat(pathName, &statbuf);
                } else {
                    if (!OS_ISDIR(statbuf.stat_mode)) {
                        printf ("File %s already exists, but is not a directory\n", pathName);
                        os_free(pathName);
                        return 0;
                    }
                }
                if (statRes == os_resultFail) {
                    printf ("Error when creating directory %s\n", pathName);
                    os_free(pathName);
                    return 0;
                }
                /* reallocate pathName to include os_fileSep() */
                pathName = os_realloc(pathName, strlen(pathName) + strlen(stackScope) + strlen(os_fileSep()) + 1);
                os_strcat (pathName, os_fileSep());
            }
        } while(token);
    }
    /* free the memory allocated to pathName */
    os_free(pathName);
    return 1;
}

void
idl_openMatlabPackage (
    idl_scope scope,
    const char *name)
{
    os_char *fname = NULL;
    os_char *package_file = NULL;

    package_file = idl_scopeStackMatlab(scope, os_fileSep(), name);
    fname = os_malloc(strlen (package_file) + strlen (".m") + 1);
    os_strcpy(fname, package_file);
    os_strcat(fname, ".m");
    if (idl_createDir(fname)) {
	    idl_fileSetCur(idl_fileOutNew (fname, "w"));
        if (idl_fileCur() == NULL) {
            idl_fileOpenError(fname);
        }
    } else {
	    idl_fileSetCur(NULL);
    }
    os_free(package_file);
    os_free(fname);
}

void
idl_closeMatlabPackage (
    void)
{
    if (idl_fileCur()) {
	idl_fileOutFree(idl_fileCur());
    }
}
