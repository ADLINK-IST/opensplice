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
/*
   This module generates Splice type definitions related to
   an IDL input file.
*/

/**
 * @file
 * This module generates a MATLAB script that outputs a Simulink Bus object
 * related to an IDL input file.
*/

#include "os_version.h"
#include "idl_program.h"
#include "idl_scope.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"
#include "idl_map.h"
#include "idl_keyDef.h"
#include "idl_genMetaHelper.h"
#include "idl_genSimulinkHelper.h"

#include <ctype.h>
#include "c_typebase.h"
#include "os_iterator.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_abstract.h"
#include "ut_collection.h"

#define MAX_UNBOUNDED_STR_DEFAULT 256

static int busIdx;
static int enumIdx;
static int busElemIdx;
static c_ulong enumNumElements;
static c_ulong enumElemIdx;
static char *enumScopeStack;
static const c_char *enumFileAnnotation;

static void
simulink_arrayDimensions(
    idl_typeSpec typeSpec,
    char* dimensions)
{
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        idl_typeArray typeArray = idl_typeArray(typeSpec);
        c_ulong n = simulink_arrayNDimensions(typeArray);
        c_ulong *dims = (c_ulong *)os_malloc(n * sizeof(c_ulong));
        c_char *formatted = NULL;
        simulink_arrayGetDimensions(typeArray, dims);
        formatted = simulink_formatDimensions(dims, n);
        os_free((void*)dims);
        strcpy(dimensions, formatted);
        os_free(formatted);
    }
}

/** @brief callback function called on opening the IDL input file.
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

    simulink_propertiesTable = simulink_createTable();
    simulink_nameTable = simulink_createTable();

    /*sizeof(“.properties”) includes the null-byte */
    simulink_propertiesFileName = (char *)os_malloc(strlen(name) + sizeof(".properties"));
    os_sprintf(simulink_propertiesFileName, "%s.properties", name);

    simulink_readProperties(simulink_propertiesFileName, simulink_propertiesTable);

    busIdx = 1;
    enumIdx = 1;
    return idl_explore;
}

static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    if(simulink_propertiesFileName != NULL) {
    	simulink_writeProperties(simulink_propertiesTable, simulink_propertiesFileName);
    	os_free((void*)simulink_propertiesFileName);
    }
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
    c_ulong showWarning = OS_TRUE;
    char *scopeStack = simulinkScope(scope);
    const c_char *keyAnnotation = simulinkKeyListAnnotation(scope, name);
    const c_char *idlFileAnnotation = simulinkIdlFileAnnotation(scope);
    const char *busName = simulink_getClassNameFromName(scope, name, showWarning);

    OS_UNUSED_ARG(userData);

    busElemIdx = 1;

    idl_fileOutPrintf(idl_fileCur(), "busses{1,%d} = '%s';\n", busIdx, busName);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d} = Simulink.Bus;\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.Description = '%s%s%s';\n",
    		busIdx, scopeStack, keyAnnotation, idlFileAnnotation);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.DataScope = 'Auto';\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.HeaderFile = '';\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.Alignment = -1;\n", busIdx);
    os_free((void*)scopeStack);
    os_free((void*)keyAnnotation);
    os_free((void*)idlFileAnnotation);

    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        c_type type = idl_typeSpecDef(idl_typeSpec(structSpec));
        char *typedesc = idl_genXMLmeta(type, FALSE);

        idl_fileOutPrintf(idl_fileCur(), "busses{3,%d} = '%s';\n", busIdx, typedesc);
        os_free(typedesc);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "busses{3,%d} = '';\n", busIdx);
    }
    return idl_explore;
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

    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.Elements = saveVarsTmp;\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "clear saveVarsTmp;\n\n");
    busIdx++;
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
    char* simulinkType = NULL;
    char* description = "";
    char dimensions[256] = "1";

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);
    /* Dereference possible typedefs first. */
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if(!simulink_containSeqType(typeSpec)) {
        simulink_recordStringBound(typeSpec, scope, name);
    }

	description = simulinkTypeAnnotation(typeSpec);
	if ((idl_typeSpecType(typeSpec) == idl_tbasic)) {
        simulinkType = idl_SimulinkTypeFromTypeSpec(typeSpec);
        if (idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_string) {
            c_ulong maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
            os_sprintf(dimensions, "%d", maxlen ? maxlen : simulink_getStrMaxDimension());
        }
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_ulong showWarning = OS_FALSE;
    	simulinkType = simulinkSeqClassRef(idl_typeSeq(typeSpec), showWarning);
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        simulinkType = idl_SimulinkTypeFromTypeSpec(typeSpec);
        simulink_arrayDimensions(typeSpec, dimensions);
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        simulinkType = simulinkGetDataType(typeSpec);
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        simulinkType = simulinkGetDataType(typeSpec);
    } else if (idl_typeSpecType (typeSpec) == idl_tunion) {
        printf("idl_genSimulink.c:idl_structureMemberOpenClose: %s Union types are unsupported"
                " for Simulink binding.\n",
                name);
        exit(1);
    } else {
        printf("idl_genSimulink.c:idl_structureMemberOpenClose: Unexpected type %d\n",
            idl_typeSpecType(typeSpec));
    }

    if (!simulinkType) {
        simulinkType = os_strdup("");
    }

    /* Set the Simulink bus properties. */
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d) = Simulink.BusElement;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Name = '%s';\n", busElemIdx, name);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Dimensions = %s;\n", busElemIdx, dimensions);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DataType = '%s';\n", busElemIdx, simulinkType);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Description = '%s';\n", busElemIdx, description);

    /* Other Simulink bus properties that we don't modify. */
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Complexity = 'real';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Min = [];\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Max = [];\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DimensionsMode = 'Fixed';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).SamplingMode = 'Sample based';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).SampleTime = -1;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DocUnits = '';\n", busElemIdx);

    simulink_setStrMaxDimension(MAX_UNBOUNDED_STR_DEFAULT);
    busElemIdx++;
    os_free(simulinkType);
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
    c_ulong showWarning = OS_TRUE;
    const char *enumName = simulink_getClassNameFromName(scope, name, showWarning);
    OS_UNUSED_ARG(userData);
    enumFileAnnotation = simulinkIdlFileAnnotation(scope);

    idl_fileOutPrintf(idl_fileCur(), "enums{%d} = '%s';\n", enumIdx, enumName);
    enumIdx++;
    enumScopeStack = simulinkScope(scope);
    enumNumElements = idl_typeEnumNoElements (enumSpec);
    enumElemIdx = 0;
    idl_fileOutPrintf(idl_fileCur(), "Simulink.defineIntEnumType('%s',{", enumName);

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
    c_ulong i;
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "},[");
    for (i = 0; i < enumNumElements; i++) {
        idl_fileOutPrintf(idl_fileCur(), "%d", i);
        if (i != enumNumElements - 1) {
            idl_fileOutPrintf(idl_fileCur(), ";");
        }
    }
    idl_fileOutPrintf(idl_fileCur(), "]");
    idl_fileOutPrintf(idl_fileCur(), ", 'Description', '%s%s'", enumScopeStack, enumFileAnnotation);
    os_free((void*)enumScopeStack);
    os_free((void*)enumFileAnnotation);

    idl_fileOutPrintf(idl_fileCur(),");\n\n");
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
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "'%s'", name);
    if (enumElemIdx != enumNumElements - 1) {
        idl_fileOutPrintf(idl_fileCur(), ",");
    }
    enumElemIdx++;
}

static void
idl_sequenceOpenClose(
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    c_ulong seqMax;
    c_char *wrappedTypeDimensions;
    c_ulong showWarning = OS_TRUE;
    const c_char *idlFileAnnotation = simulinkIdlFileAnnotation(scope);
    c_char *seqAnnotation = simulinkTypeAnnotation(idl_typeSpec(typeSeq));
    c_char *name = simulinkSeqClassName(typeSeq);
    idl_typeSpec wrappedTypeSpec = idl_typeSeqType(typeSeq);
    c_char* wrappedTypeName = simulinkSeqWrappedTypeNameRef(wrappedTypeSpec);
    c_char *wrappedTypeAnnotation = simulinkTypeAnnotation(wrappedTypeSpec);
    const c_char *busName = simulink_getClassNameFromName(scope, name, showWarning);
    if(!simulink_containSeqType(idl_typeSeqActual(typeSeq))) {
        simulink_recordStringBound(idl_typeSpec(typeSeq), scope, name);
    }
    seqMax = simulink_seqTypeBound(typeSeq, scope, name);
    wrappedTypeDimensions = simulink_sequenceTypeDimensions(wrappedTypeSpec, seqMax);

    OS_UNUSED_ARG(userData);

    /* TODO: check whether we've seen this class before */

    busElemIdx = 1;

    idl_fileOutPrintf(idl_fileCur(), "busses{1,%d} = '%s';\n", busIdx, busName);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d} = Simulink.Bus;\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.Description = '%s%s';\n",
    		busIdx, seqAnnotation, idlFileAnnotation);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.DataScope = 'Auto';\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.HeaderFile = '';\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.Alignment = -1;\n", busIdx);
    os_free((void*)idlFileAnnotation);

    idl_fileOutPrintf(idl_fileCur(), "busses{3,%d} = '';\n", busIdx);

    /* fields */
    /* Set the 'length' bus properties. */
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d) = Simulink.BusElement;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Name = 'length';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Dimensions = 1;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DataType = 'uint32';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Description = '';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Complexity = 'real';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Min = [];\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Max = [];\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DimensionsMode = 'Fixed';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).SamplingMode = 'Sample based';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).SampleTime = -1;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DocUnits = '';\n", busElemIdx);

    busElemIdx++;

    /* Set the 'buffer' bus properties. */
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d) = Simulink.BusElement;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Name = 'buffer';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Dimensions = %s;\n", busElemIdx, wrappedTypeDimensions);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DataType = '%s';\n", busElemIdx, wrappedTypeName);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Description = '%s';\n", busElemIdx, wrappedTypeAnnotation);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Complexity = 'real';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Min = [];\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).Max = [];\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DimensionsMode = 'Fixed';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).SamplingMode = 'Sample based';\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).SampleTime = -1;\n", busElemIdx);
    idl_fileOutPrintf(idl_fileCur(), "saveVarsTmp(%d).DocUnits = '';\n", busElemIdx);

    busElemIdx++;

    /* finish up*/
    idl_fileOutPrintf(idl_fileCur(), "busses{2,%d}.Elements = saveVarsTmp;\n", busIdx);
    idl_fileOutPrintf(idl_fileCur(), "clear saveVarsTmp;\n\n");
    busIdx++;

    simulink_setStrMaxDimension(MAX_UNBOUNDED_STR_DEFAULT);

    os_free((void *)name);
    os_free((void *)wrappedTypeDimensions);
    os_free((void *)wrappedTypeName);
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
static idl_programControl idl_genSimulinkLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genSimulinkLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSimulink = {
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
idl_genSimulinkProgram(
    void)
{
    return &idl_genSimulink;
}
