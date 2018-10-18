/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
/*
   This module generates Splice type definitions related to
   an IDL input file.
*/

/**
 * @file
 * This module generates a MATLAB script that outputs a Simulink Bus object
 * related to an IDL input file.
*/

#include <stdlib.h>

#include "os_version.h"
#include "idl_program.h"
#include "idl_scope.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"
#include "idl_map.h"
#include "idl_keyDef.h"
#include "idl_genMetaHelper.h"
#include "idl_genLabview.h"

#include <ctype.h>
#include "c_typebase.h"
#include "os_iterator.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_abstract.h"
#include "ut_collection.h"

/*
 * START: code copied from osplo/src/tools/labview/include/labview_cico.h
 */
typedef enum {
	CICO_STRUCT_DEF,
	CICO_PRIMITIVE,
	CICO_STRING,
	CICO_STRUCT,
	CICO_ARRAY,
	CICO_SEQUENCE,
} Cico_Code_t;

typedef enum {
	CICO_PRIM_BOOLEAN,
	CICO_PRIM_CHAR,
	CICO_PRIM_OCTET,
	CICO_PRIM_SHORT,
	CICO_PRIM_USHORT,
	CICO_PRIM_LONG,
	CICO_PRIM_ULONG,
	CICO_PRIM_LONGLONG,
	CICO_PRIM_ULONGLONG,
	CICO_PRIM_FLOAT,
	CICO_PRIM_DOUBLE,
} Cico_Primitive_t;

/*
 * END: copied code
 */

typedef struct entry_info {
	Cico_Code_t code;
	int value;
	const char *ref_qname;
} entry_info_t;

typedef struct copy_info {
	unsigned int nflds;
	unsigned int nentries;
	entry_info_t *entries;
} copy_info_t;

static idl_streamOut topicTypeStream;
static idl_streamOut copyInfoStream;
static copy_info_t *current_copy_info;

static const char *codeToStr(Cico_Code_t code) {
	switch (code) {
	case CICO_PRIMITIVE:
		return "PRIMITIVE";
	case CICO_STRING:
		return "STRING";
	case CICO_STRUCT:
		return "STRUCT";
	case CICO_ARRAY:
		return "ARRAY";
	case CICO_SEQUENCE:
		return "SEQUENCE";
	case CICO_STRUCT_DEF:
	default:
		return "";
	}
}


static c_char
*labviewQualifiedElement(
        idl_scope scope,
        const char *name)
{
    size_t slen = NULL;
    idl_scopeType currentScopeType = NULL;
    char *scopeStack = os_strdup ("");
    if (idl_scopeStackSize(scope) != 0) {
        int si;
        for (si = 0; si < idl_scopeStackSize(scope); si++) {
            char *seName = idl_scopeElementName(idl_scopeIndexed(scope, si));

            /* 2 for scope separator, 1 for null byte */
            slen = strlen (scopeStack) + 2 + strlen (seName) +1;
            scopeStack = os_realloc(scopeStack, slen);
            if (strlen(scopeStack) != 0) {
                os_strcat(scopeStack, "__");
            }
            os_strcat (scopeStack, seName);
        }
        if(strlen(name) != 0) {
            /* 2 for scope separator, 1 for null byte */
            slen = strlen (scopeStack) + 2 + strlen (name) + 1;
            scopeStack = os_realloc(scopeStack, slen);
            if (strlen(scopeStack) != 0) {
                currentScopeType = idl_scopeElementType(idl_scopeCur(scope));
                if(currentScopeType == idl_tStruct) {
                    os_strcat(scopeStack, "_");
                } else {
                    os_strcat(scopeStack, "__");
                }
            }
            os_strcat (scopeStack, name);
        }
    } else {
    	os_free(scopeStack);
    	scopeStack = os_strdup(name);
    }
    return scopeStack;
}

/* @brief callback function called on opening the IDL input file.
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
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* TODO: delete these */

    idl_fileOutPrintf(idl_fileCur(), "<!-- LabVIEW extract of %s. This document is processed by LabVIEW. -->\n", name);
    idl_fileOutPrintf(idl_fileCur(),
    		"<topic-types>\n"
    		"  <typedefs>\n");

    return idl_explore;
}

static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(),
    		"  </typedefs>\n"
    		"</topic-types>\n"
    		);
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
	char *qname;
	const c_char *keys;

	OS_UNUSED_ARG(userData);

	qname = labviewQualifiedElement(scope, name);

	/* alloc'd here, release on structure close */
	current_copy_info = (copy_info_t *)os_malloc(sizeof(copy_info_t));
	current_copy_info->entries = NULL;
	current_copy_info->nflds = 0;
	current_copy_info->nentries = 0;

    idl_fileOutPrintf(idl_fileCur(),
    	    "    <struct name=\"%s\">\n"
    		, qname);

	keys = idl_keyResolve(idl_keyDefDefGet(), scope, name);
	copyInfoStream = idl_streamOutNew (0);

	if(keys != NULL) {
        c_type type = idl_typeSpecDef(idl_typeSpec(structSpec));
        c_char *descriptor = idl_genXMLmeta(type, FALSE);
        c_char *scopedTypeName = idl_scopeStack(scope, "::", name);
        c_char *cp;

		/* It's a topic type declaration */
		topicTypeStream = idl_streamOutNew (0);

		idl_streamOutPrintf(topicTypeStream,
				"      <type>\n"
				"        <name>%s</name>\n"
				"        <viname>%s</viname>\n"
				"        <keylist>%s</keylist>\n"
				"        <descriptor>"
				,
				scopedTypeName, qname, keys
				);
		/* XML encode and write the descriptor */
		for(cp = descriptor; *cp; cp++) {
			if(*cp == '<') {
				idl_streamOutPut(topicTypeStream, '&');
				idl_streamOutPut(topicTypeStream, 'l');
				idl_streamOutPut(topicTypeStream, 't');
				idl_streamOutPut(topicTypeStream, ';');
			} else if (*cp == '>') {
				idl_streamOutPut(topicTypeStream, '&');
				idl_streamOutPut(topicTypeStream, 'g');
				idl_streamOutPut(topicTypeStream, 't');
				idl_streamOutPut(topicTypeStream, ';');
			} else {
				(void) idl_streamOutPut(topicTypeStream, *cp);
			}
		}
		idl_streamOutPrintf(topicTypeStream,
				"</descriptor>\n"
				"      </type>\n"
				);

		os_free(descriptor);
		os_free(scopedTypeName);
	} else {
		topicTypeStream = NULL;
	}

    os_free(qname);

    return idl_explore;
}

static void print_copy_info(
		copy_info_t* ci) {
	unsigned int i;

	idl_streamOutPrintf(copyInfoStream,
			"          <entry><code>STRUCT_DEF</code><value>%d</value></entry>\n",
			ci->nflds);
	for (i = 0; i < ci->nentries; i++) {
		if(ci->entries[i].code == CICO_STRUCT) {
			idl_streamOutPrintf(copyInfoStream,
					"          <entry><code>%s</code><value>%d</value></entry>\n"
					"          <fixup><name>%s</name><index>%d</index></fixup>\n",
					codeToStr(ci->entries[i].code),
					ci->entries[i].value,
					ci->entries[i].ref_qname,
					i + 1
					);

		} else {
			idl_streamOutPrintf(copyInfoStream,
					"          <entry><code>%s</code><value>%d</value></entry>\n",
					codeToStr(ci->entries[i].code),
					ci->entries[i].value
					);
		}
	}
}

static void free_copy_info(copy_info_t *ci)
{
	if(ci->entries) {
		unsigned int i;
		for(i = 0; i < ci->nentries; i++) {
			if(ci->entries[i].code == CICO_STRUCT) {
				os_free((void *)ci->entries[i].ref_qname);
			}
		}
		os_free(ci->entries);
	}
	os_free(ci);
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

    /* print copy info */
	idl_streamOutPrintf(copyInfoStream,
			"      <table>\n"
			);

	print_copy_info(current_copy_info);

	idl_streamOutPrintf(copyInfoStream,
			"      </table>\n"
			);
	idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet (idl_stream(copyInfoStream)));
	idl_streamOutFree(copyInfoStream);
	copyInfoStream = NULL;

    if(topicTypeStream) {
    	/* print type info, if it's a topic type */
    	idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet (idl_stream(topicTypeStream)));

    	idl_streamOutFree(topicTypeStream);
    }
    topicTypeStream = NULL;

    idl_fileOutPrintf(idl_fileCur(), "    </struct>\n");

    free_copy_info(current_copy_info);
    current_copy_info = NULL;

}

idl_typeSpec actualArrayType(idl_typeArray typeArray) {
	idl_typeSpec arrayActual = idl_typeArrayType(typeArray);
	/* This while loop may be overkill, but doc and code is unclear
	 * as to whehter idl_typeDefActual goes through all typedefs
	 */
	while(idl_typeSpecType(arrayActual) == idl_ttypedef) {
		arrayActual = idl_typeDefActual(idl_typeDef(arrayActual));
	}

	return arrayActual;
}

static void print_field_type(idl_typeSpec typeSpec) {
	idl_type memberType = idl_typeSpecType(typeSpec);
	switch (memberType) {
	case idl_tbasic: {
		idl_typeBasic typeBasic = idl_typeBasic(typeSpec);
		idl_basicType basicType = idl_typeBasicType(typeBasic);
		switch (basicType) {
		case idl_short:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>int16</numeric>");
			break;
		case idl_ushort:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>uint16</numeric>");
			break;
		case idl_long:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>int32</numeric>");
			break;
		case idl_ulong:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>uint32</numeric>");
			break;
		case idl_longlong:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>int64</numeric>");
			break;
		case idl_ulonglong:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>uint64</numeric>");
			break;
		case idl_float:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>float</numeric>");
			break;
		case idl_double:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>double</numeric>");
			break;
		case idl_char:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>int8</numeric>");
			break;
		case idl_octet:
			idl_fileOutPrintf(idl_fileCur(), "<numeric>uint8</numeric>");
			break;
		case idl_boolean:
			idl_fileOutPrintf(idl_fileCur(), "<boolean/>");
			break;
		case idl_string:
			idl_fileOutPrintf(idl_fileCur(), "<string/>");
			break;
		}
		break;
	}
	case idl_tenum: {
		char* enumQname;
		enumQname = labviewQualifiedElement(
				idl_typeUserScope(idl_typeUser(typeSpec)),
				idl_typeSpecName(typeSpec));
		idl_fileOutPrintf(idl_fileCur(), "<enum>%s</enum>", enumQname);
		os_free(enumQname);
		break;
	}
	case idl_tstruct: {
		char* structQname;
		structQname = labviewQualifiedElement(
				idl_typeUserScope(idl_typeUser(typeSpec)),
				idl_typeSpecName(typeSpec));
		idl_fileOutPrintf(idl_fileCur(), "<struct>%s</struct>", structQname);
		os_free(structQname);
		break;
	}
	case idl_tarray: {
		idl_typeArray typeArray = idl_typeArray(typeSpec);
		idl_typeSpec containedTypeSpec;
		int ndims = 1;
		for (containedTypeSpec = actualArrayType(typeArray);
				idl_typeSpecType(containedTypeSpec) == idl_tarray;
				containedTypeSpec = idl_typeArrayType(idl_typeArray(containedTypeSpec))) {
			ndims++;
		}
		idl_fileOutPrintf(idl_fileCur(), "<array dims=\"%d\">", ndims);
		print_field_type(containedTypeSpec);
		idl_fileOutPrintf(idl_fileCur(), "</array>");
		break;
	}
	case idl_tseq: {
		idl_typeSpec containedTypeSpec = idl_typeSeqType(idl_typeSeq(typeSpec));
		idl_fileOutPrintf(idl_fileCur(), "<sequence>");
		print_field_type(containedTypeSpec);
		idl_fileOutPrintf(idl_fileCur(), "</sequence>");
		break;
	}
	case idl_ttypedef: {
		print_field_type(idl_typeDefActual(idl_typeDef(typeSpec)));
		break;
	}
	case idl_tunion:
	default:
		break;
	}
}

static entry_info_t *grow_entries() {
	const unsigned int INCR = 10;
	unsigned int idx;
	if(current_copy_info->nentries == 0) {
		current_copy_info->entries = (entry_info_t *)os_malloc(INCR * sizeof(entry_info_t));
	} else if(current_copy_info->nentries % INCR == 0) {
		current_copy_info->entries = (entry_info_t *)os_realloc(current_copy_info->entries,
				(current_copy_info->nentries + INCR) * sizeof(entry_info_t));
	}
	idx = current_copy_info->nentries++;
	return &(current_copy_info->entries[idx]);
}

static void record_field_info(idl_typeSpec typeSpec) {
	idl_type memberType = idl_typeSpecType(typeSpec);
	switch (memberType) {
	case idl_tbasic: {
		entry_info_t *entry = grow_entries();
		idl_typeBasic typeBasic = idl_typeBasic(typeSpec);
		idl_basicType basicType = idl_typeBasicType(typeBasic);
		switch (basicType) {
		case idl_short:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_SHORT;
			break;
		case idl_ushort:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_USHORT;
			break;
		case idl_long:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_LONG;
			break;
		case idl_ulong:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_ULONG;
			break;
		case idl_longlong:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_LONGLONG;
			break;
		case idl_ulonglong:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_ULONGLONG;
			break;
		case idl_float:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_FLOAT;
			break;
		case idl_double:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_DOUBLE;
			break;
		case idl_char:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_CHAR;
			break;
		case idl_octet:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_OCTET;
			break;
		case idl_boolean:
			entry->code = CICO_PRIMITIVE;
			entry->value = CICO_PRIM_BOOLEAN;
			break;
		case idl_string:
			entry->code = CICO_STRING;
			entry->value = (int)idl_typeBasicMaxlen(typeBasic);
			break;
		}
		break;
	}
	case idl_tenum: {
		entry_info_t *entry = grow_entries();
		entry->code = CICO_PRIMITIVE;
		entry->value = CICO_PRIM_LONG;
		break;
		break;
	}
	case idl_tstruct: {
		entry_info_t *entry = grow_entries();
		entry->code = CICO_STRUCT;
		entry->value = -1;
		entry->ref_qname = idl_scopeStack(
				idl_typeUserScope(idl_typeUser(typeSpec)),
				"::",
				idl_typeSpecName(typeSpec));
/*
				labviewQualifiedElement(
				idl_typeUserScope(idl_typeUser(typeSpec)),
				idl_typeSpecName(typeSpec));
*/
		break;
	}
	case idl_tarray: {
		entry_info_t *entry = grow_entries();
		entry->code = CICO_ARRAY;
		entry->value = (int)idl_typeArraySize(idl_typeArray(typeSpec));
		record_field_info(idl_typeArrayType(idl_typeArray(typeSpec)));
		break;
	}
	case idl_tseq: {
		entry_info_t *entry = grow_entries();
		entry->code = CICO_SEQUENCE;
		entry->value = (int)idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
		record_field_info(idl_typeSeqType(idl_typeSeq(typeSpec)));
		break;
	}
	case idl_ttypedef: {
		record_field_info(idl_typeDefActual(idl_typeDef(typeSpec)));
		break;
	}
	case idl_tunion:
	default:
		break;
	}
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

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSpec);
    OS_UNUSED_ARG(userData);


    idl_fileOutPrintf(idl_fileCur(),
    	    "      <field name=\"%s\">"
    		, name);

	print_field_type(typeSpec);

    idl_fileOutPrintf(idl_fileCur(),
    		"</field>\n"
    		);

    record_field_info(typeSpec);
	current_copy_info->nflds++;


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
	char *qname;

	OS_UNUSED_ARG(enumSpec);
	OS_UNUSED_ARG(userData);

	qname = labviewQualifiedElement(scope, name);

    idl_fileOutPrintf(idl_fileCur(),
    	    "    <enum name=\"%s\">\n"
    		, qname);

    os_free(qname);
    /* return idl_explore to indicate that the rest of the structure needs to be processed */
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


    idl_fileOutPrintf(idl_fileCur(),
    	    "    </enum>\n"
    		);
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
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(),
    	    "      <value>%s</value>\n"
    		, name);

}

static void
idl_sequenceOpenClose(
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSeq);
    OS_UNUSED_ARG(userData);
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
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);
    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genLabVIEWLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genLabVIEWLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genLabVIEW = {
    idl_getControl,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    idl_enumerationOpen,
    idl_enumerationClose,
    idl_enumerationElementOpenClose,
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    NULL, /* idl_typedefOpenClose */
    NULL, /* idl_boundedStringOpenClose */
	idl_sequenceOpenClose, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genLabVIEWProgram(
    void)
{
    return &idl_genLabVIEW;
}
