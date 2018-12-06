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
 * This module generates Standalone Cxx11 data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genCxxType.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"

#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_dll.h"
#include "idl_walk.h"

#include "vortex_os.h"
#include <ctype.h>
#include "c_base.h"
#include "c_metabase.h"
#include "c_typebase.h"

/** indentation level */
static c_long indent_level = 0;

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program idl_genCxxType;

struct SACPPScopeStack_s {
    c_type type;
    c_bool unionHasArtificialDefault;
};
typedef struct SACPPScopeStack_s SACPPScopeStack;

/** @brief callback function called on opening the IDL input file.
 *
 * Generate standard file header consisting of:
 * - inclusion of Splice type definition files
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

    /* Store fileName in uppercase. */
    /* Generate inclusion of corresponding header file */
    idl_fileOutPrintf(idl_fileCur(), "#include \"%s.h\"\n", name);
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
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

static void
idl_printArrayIndex(
        c_type arrType,
        int index)
{
    c_type elementType = c_typeActualType(c_collectionTypeSubType(arrType));

    idl_fileOutPrintf(idl_fileCur(), "[i%d]", index);
    if (c_baseObjectKind(elementType) == M_COLLECTION && c_collectionTypeKind(elementType) == OSPL_C_ARRAY) {
        idl_printArrayIndex(elementType, index + 1);
    }
}

static void
idl_generateArrayCopyBody(
        c_type outerType,
        c_type currentDimType,
        int index)
{
    c_type elementType = c_typeActualType(c_collectionTypeSubType(currentDimType));

    idl_printIndent(++indent_level);
    idl_fileOutPrintf(idl_fileCur(), "for (DDS::ULong i%d = 0; i%d < %d; i%d++) {\n",
            index, index, c_collectionTypeMaxSize(currentDimType), index);
    if (c_baseObjectKind(elementType) == M_COLLECTION && c_collectionTypeKind(elementType) == OSPL_C_ARRAY) {
        idl_generateArrayCopyBody(outerType, elementType, index + 1);
    } else {
        idl_printIndent(indent_level + 1);
        idl_fileOutPrintf(idl_fileCur(), "to");
        idl_printArrayIndex(outerType, 0);
        idl_fileOutPrintf(idl_fileCur(), " = from");
        idl_printArrayIndex(outerType, 0);
        idl_fileOutPrintf(idl_fileCur(), ";\n");
    }
    idl_printIndent(indent_level--);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
}

static void
idl_generateArrayFunctionBodies(
        c_type arrType,
        c_char *scopeName,
        c_char *memberName,
        c_bool isMember)
{
    c_char *typedefBaseName;
    const char *baseNameTmplt;
    size_t baseNameLength;
    c_type elementType = c_collectionTypeSubType(arrType);

    /* Obtain baseName for typedef (either derived from member name, or from typedef name) */
    if (isMember) {
        baseNameTmplt = "%s::_%s";
    } else {
        baseNameTmplt = "%s%s";
    }
    baseNameLength = strlen(baseNameTmplt) + strlen(scopeName) + strlen(memberName) + 1;
    typedefBaseName = os_malloc(baseNameLength);
    snprintf(typedefBaseName, baseNameLength, baseNameTmplt, scopeName, memberName);

    if (c_baseObjectKind(elementType) == M_TYPEDEF) {
        elementType = c_typeActualType(elementType);
        while (c_baseObjectKind(elementType) == M_COLLECTION &&
                c_collectionTypeKind(elementType) == OSPL_C_ARRAY) {
            elementType = c_typeActualType(c_collectionTypeSubType(elementType));
        }
    }

    idl_fileOutPrintf(idl_fileCur(), "#if DDS_USE_EXPLICIT_TEMPLATES\n");
    switch(c_baseObjectKind(elementType)) {
    case M_COLLECTION:
        switch (c_collectionTypeKind(elementType)) {
        case OSPL_C_SEQUENCE:
        case OSPL_C_STRING:
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_VArray_var< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_Array_forany< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_VLArray_out < %s, %s_slice, %s_var, %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName);
            break;
        case OSPL_C_ARRAY:
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_MArray_var< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_MArray_forany< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
            if (idl_CxxIsRefType(elementType)) {
                idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_VLArray_out < %s, %s_slice, %s_var, %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName);
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_PRIMITIVE:
    case M_ENUMERATION:
        idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_FArray_var< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
        idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_Array_forany< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
        break;
    case M_STRUCTURE:
    case M_UNION:
        if (idl_CxxIsRefType(elementType)) {
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_VArray_var< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_VLArray_out < %s, %s_slice, %s_var, %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_FArray_var< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
        }
        idl_fileOutPrintf(idl_fileCur(), "template class DDS_DCPS_Array_forany< %s, %s_slice, struct %s_uniq_>;\n", typedefBaseName, typedefBaseName, typedefBaseName);
        break;
    case M_TYPEDEF:
        /* Typedef should have already been dereferenced at start of this switch statement. */
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
    idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");
    idl_fileOutPrintf(idl_fileCur(), "template <>\n");
    idl_fileOutPrintf(idl_fileCur(), "%s_slice* DDS_DCPS_ArrayHelper < %s, %s_slice, %s_uniq_>::alloc ()\n", typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    return %s_alloc ();\n", typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    idl_fileOutPrintf(idl_fileCur(), "template <>\n");
    idl_fileOutPrintf(idl_fileCur(), "void DDS_DCPS_ArrayHelper < %s, %s_slice, %s_uniq_>::copy (%s_slice *to, const %s_slice* from)\n", typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    %s_copy (to, from);\n", typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    idl_fileOutPrintf(idl_fileCur(), "template <>\n");
    idl_fileOutPrintf(idl_fileCur(), "void DDS_DCPS_ArrayHelper < %s, %s_slice, %s_uniq_>::free (%s_slice *ptr)\n", typedefBaseName, typedefBaseName, typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    %s_free(ptr);\n", typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    idl_fileOutPrintf(idl_fileCur(), "%s_slice * %s_alloc ()\n", typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    %s_slice * ret = new %s_slice[%d];\n", typedefBaseName, typedefBaseName, c_collectionTypeMaxSize(arrType));
    idl_fileOutPrintf(idl_fileCur(), "    return ret;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    idl_fileOutPrintf(idl_fileCur(), "void %s_free (%s_slice * s)\n", typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    delete [] s;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    idl_fileOutPrintf(idl_fileCur(), "void %s_copy(%s_slice * to, const %s_slice * from)\n", typedefBaseName, typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_generateArrayCopyBody(arrType, arrType, 0);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    idl_fileOutPrintf(idl_fileCur(), "%s_slice * %s_dup(const %s_slice * from)\n", typedefBaseName, typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    %s_slice * to = %s_alloc ();\n", typedefBaseName, typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "    %s_copy (to, from);\n", typedefBaseName);
    idl_fileOutPrintf(idl_fileCur(), "    return to;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    os_free(typedefBaseName);
}

static void
idl_generateSequenceFunctionBodies(
        c_type seqType,
        c_char *scopeName,
        c_char *memberName,
        int recursionIndex,
        c_bool isMember)
{
    c_char *typedefBaseName, *subElementBaseName, *elementTypeName;
    const char *baseNameTmplt, *subElementTemplate;
    size_t baseNameLength, subElementLen;
    c_type elementType = c_typeActualType(c_collectionTypeSubType(seqType));
    c_ulong maxBound = c_collectionTypeMaxSize(seqType);

    /* Obtain baseName for typedef (either derived from member name, or from typedef name) */
    if (isMember) {
        baseNameTmplt = "%s::_%s_seq";
    } else {
        baseNameTmplt = "%s%s";
    }
    baseNameLength = strlen(baseNameTmplt) + strlen(scopeName) + strlen(memberName) + 1;
    typedefBaseName = os_malloc(baseNameLength);
    snprintf(typedefBaseName, baseNameLength, baseNameTmplt, scopeName, memberName);

    elementTypeName = idl_CxxTypeFromCType(elementType, typedefBaseName);

    if (recursionIndex == 0) idl_fileOutPrintf(idl_fileCur(), "#if DDS_USE_EXPLICIT_TEMPLATES\n");
    switch (c_baseObjectKind(elementType)) {
    case M_COLLECTION:
        switch (c_collectionTypeKind(elementType)) {
        case OSPL_C_STRING:
            if (!maxBound) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSUStrSeqT<struct %s_uniq_>;\n",
                    typedefBaseName);
            } else {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSBStrSeq<%u>;\n",
                    maxBound);
            }
            break;
        case OSPL_C_SEQUENCE:
            subElementTemplate = "%s_sub";
            subElementLen = strlen(subElementTemplate) + strlen(typedefBaseName) + 1;
            subElementBaseName = os_malloc(subElementLen);
            snprintf(subElementBaseName, subElementLen, subElementTemplate, typedefBaseName);
            idl_generateSequenceFunctionBodies(
                    elementType,
                    "",
                    subElementBaseName,
                    recursionIndex + 1,
                    FALSE);
            if (!maxBound) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSUVLSeq<%s, struct %s_uniq_>;\n",
                    subElementBaseName,
                    typedefBaseName);
            } else {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSBVLSeq<%s, %u>;\n",
                    subElementBaseName,
                    maxBound);
            }
            os_free(subElementBaseName);
            break;
        case OSPL_C_ARRAY:
            /* Array should be identified by its memberType. */
            idl_generateArrayFunctionBodies(
                    elementType,
                    scopeName,
                    elementTypeName,
                    isMember);
            break;
        default:
            /* Invalid collection. */
            assert(0);
            break;
        }
        break;
    case M_PRIMITIVE:
    case M_ENUMERATION:
        if (!maxBound) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "template class DDS_DCPSUFLSeq<%s, struct %s_uniq_>;\n",
                elementTypeName,
                typedefBaseName);
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "template class DDS_DCPSBFLSeq<%s, struct %s_uniq_, %u>;\n",
                elementTypeName,
                typedefBaseName,
                maxBound);
        }
        break;
    case M_STRUCTURE:
    case M_UNION:
        if (idl_CxxIsRefType(elementType)) {
            if (!maxBound) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSUVLSeq<%s, struct %s_uniq_>;\n",
                    elementTypeName,
                    typedefBaseName);
            } else {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSBVLSeq<%s, %u>;\n",
                    elementTypeName,
                    maxBound);
            }
        } else {
            if (!maxBound) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSUFLSeq<%s, struct %s_uniq_>;\n",
                    elementTypeName,
                    typedefBaseName);
            } else {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "template class DDS_DCPSBFLSeq<%s, struct %s_uniq_, %u>;\n",
                    elementTypeName,
                    typedefBaseName,
                    maxBound);
            }
        }
        break;
    case M_TYPEDEF:
        /* Typedef should have already been dereferenced at start of this function. */
        assert(0);
        break;
    default:
        /* Unexpected sequence type. */
        assert(0);
        break;
    }
    if (recursionIndex == 0) idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");
    os_free(elementTypeName);
    os_free(typedefBaseName);
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
    c_char *cxxName = idl_cxxId(name);
    c_char *scopeName = idl_scopeStackCxx (scope, "::", cxxName);
    c_type structType = idl_typeSpecDef(idl_typeSpec(structSpec));
    c_ulong i, nrMembers = c_structureMemberCount((c_structure) structType);

    OS_UNUSED_ARG(userData);

    for (i = 0; i < nrMembers; i++) {
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_memberType(structMember);

        if (c_baseObjectKind(memberType) == M_COLLECTION) {
            c_char *memberName = idl_cxxId(c_specifierName(structMember));
            if (c_collectionTypeKind(memberType) == OSPL_C_ARRAY) {
                idl_generateArrayFunctionBodies(memberType, scopeName, memberName, TRUE);
            } else if (c_collectionTypeKind(memberType) == OSPL_C_SEQUENCE) {
                idl_generateSequenceFunctionBodies(memberType, scopeName, memberName, 0, TRUE);
            }
            os_free(memberName);
        }
    }

    os_free(scopeName);
    os_free(cxxName);

    /* return idl_abort to indicate that the rest of the structure doesn't need to be processed. */
    return idl_abort;
}

/** @brief callback function called on definition of a union in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the union
 * @param unionSpec Specifies the number of union cases and the union switch type
 */
static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    c_char *cxxName = idl_cxxId(name);
    c_char *scopeName = idl_scopeStackCxx (scope, "::", cxxName);
    c_type unionType = idl_typeSpecDef(idl_typeSpec(unionSpec));
    c_ulong i, nrBranches = c_unionUnionCaseCount(unionType);

    OS_UNUSED_ARG(userData);

    for (i = 0; i < nrBranches; i++) {
        /* Get the meta-data of the branches from the database. */
        c_unionCase branch = c_unionUnionCase(unionType, i);
        c_type branchType = c_unionCaseType(branch);
        if (c_baseObjectKind(branchType) == M_COLLECTION) {
            c_char *branchName = idl_cxxId(c_specifierName(branch));
            if (c_collectionTypeKind(branchType) == OSPL_C_ARRAY) {
                idl_generateArrayFunctionBodies(branchType, scopeName, branchName, TRUE);
            } else if (c_collectionTypeKind(branchType) == OSPL_C_SEQUENCE) {
                idl_generateSequenceFunctionBodies(branchType, scopeName, branchName, 0, TRUE);
            }
            os_free(branchName);
        }
    }

    os_free(scopeName);
    os_free(cxxName);

    /* return idl_abort to indicate that the rest of the union doesn't need to be processed. */
    return idl_abort;
}

/** @brief callback function called on definition of a named type in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   typedef <type-name> <name>;
   @endverbatim
 *
 * @param scope Current scope
 * @param name Specifies the name of the type
 * @param defSpec Specifies the type of the named type
 */
static void
idl_typedefOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    idl_typeSpec typedefDeref = idl_typeDefRefered(defSpec);
    c_type derefType = idl_typeSpecDef(idl_typeSpec(typedefDeref));

    OS_UNUSED_ARG(userData);

    if (c_baseObjectKind(derefType) == M_COLLECTION) {
        c_char *scopeName = idl_scopeStackCxx (scope, "::", "");
        c_char *cxxName = idl_cxxId(name);
        if (c_collectionTypeKind(derefType) == OSPL_C_ARRAY) {
            idl_generateArrayFunctionBodies(derefType, scopeName, cxxName, FALSE);
        } else if (c_collectionTypeKind(derefType) == OSPL_C_SEQUENCE) {
            idl_generateSequenceFunctionBodies(derefType, scopeName, cxxName, 0, FALSE);
        }
        os_free(cxxName);
        os_free(scopeName);
    }
}

/**
 * Standard control structure to specify that anonymous
 * type definitions are to be processed inline with the
 * type itself in contrast with the setting of idl_prior.
*/
static idl_programControl idl_genCxxLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    return &idl_genCxxLoadControl;
}

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genCxxTypeBodyProgram(
    CxxTypeUserData *userData)
{
    idl_genCxxType.idl_getControl                  = idl_getControl;
    idl_genCxxType.fileOpen                        = idl_fileOpen;
    idl_genCxxType.fileClose                       = NULL;
    idl_genCxxType.moduleOpen                      = idl_moduleOpen;
    idl_genCxxType.moduleClose                     = NULL;
    idl_genCxxType.structureOpen                   = idl_structureOpen;
    idl_genCxxType.structureClose                  = NULL;
    idl_genCxxType.structureMemberOpenClose        = NULL;
    idl_genCxxType.enumerationOpen                 = NULL;
    idl_genCxxType.enumerationClose                = NULL;
    idl_genCxxType.enumerationElementOpenClose     = NULL;
    idl_genCxxType.unionOpen                       = idl_unionOpen;
    idl_genCxxType.unionClose                      = NULL;
    idl_genCxxType.unionCaseOpenClose              = NULL;
    idl_genCxxType.unionLabelsOpenClose            = NULL;
    idl_genCxxType.unionLabelOpenClose             = NULL;
    idl_genCxxType.typedefOpenClose                = idl_typedefOpenClose;
    idl_genCxxType.boundedStringOpenClose          = NULL;
    idl_genCxxType.sequenceOpenClose               = NULL;
    idl_genCxxType.constantOpenClose               = NULL;
    idl_genCxxType.artificialDefaultLabelOpenClose = NULL;
    idl_genCxxType.userData                        = userData;

    return &idl_genCxxType;
}
