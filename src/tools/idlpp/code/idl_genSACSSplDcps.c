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

#include "idl_program.h"
/**
 * @file
 * This module generates Standalone C# marshalers
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSACSSplDcps.h"
#include "idl_genSACSHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genCHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_walk.h"
#include "idl_dll.h"
#include "c_base.h"
#include "c_metabase.h"

#include "vortex_os.h"
#include <ctype.h>
#include "c_typebase.h"

/** indentation level */
static c_long indent_level = 0;

/** static variables for union generation */
struct SACSUnionMetaDescriptionSplDcps_s {
    c_type gUnionType;
    c_bool gUnionGenDefault;
    idl_scope gUnionScope;
    c_char *fullyScopedName;
};
typedef struct SACSUnionMetaDescriptionSplDcps_s SACSUnionMetaDescriptionSplDcps;

static void
idl_generateDatabaseRepresentation (
    idl_typeStruct typeSpec,
    const c_char *structDBName,
    SACSTypeUserData *csUserData);


static void
idl_generateUnionDatabaseRepresentation (
    c_type unionType,
    const c_char *unionDBName);

static void
idl_generateMarshaler (
    idl_typeStruct typeSpec,
    const c_char *structDBName,
    const c_char *fullyScopedName,
    SACSTypeUserData *csUserData);

static void
idl_generateUnionMarshaler (
    c_type unionType,
    const c_char *structName,
    SACSTypeUserData *csUserData,
    SACSUnionMetaDescriptionSplDcps *umd);

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
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    /* Generate inclusion of standard Vortex OpenSplice type definition files */
    idl_fileOutPrintf(idl_fileCur(), "using DDS;\n");
    idl_fileOutPrintf(idl_fileCur(), "using DDS.OpenSplice.CustomMarshalers;\n");
    idl_fileOutPrintf(idl_fileCur(), "using DDS.OpenSplice.Database;\n");
    idl_fileOutPrintf(idl_fileCur(), "using DDS.OpenSplice.Kernel;\n");
    idl_fileOutPrintf(idl_fileCur(), "using System;\n");
    idl_fileOutPrintf(idl_fileCur(), "using System.Runtime.InteropServices;\n");
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
    SACSTypeUserData *csUserData = (SACSTypeUserData *) userData;
    OS_UNUSED_ARG(scope);

    OS_UNUSED_ARG(scope);

    /* Generate the C# code that opens the namespace. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "namespace %s\n", idl_CsharpId(name, csUserData->customPSM, FALSE));
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
}

/** @brief callback function called on module termination in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        module <module-name> {
            <module-contents>
   =>   };
   @endverbatim
 *
 */
static void
idl_moduleClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    /* Decrease the indentation level back to its original size. */
    indent_level--;

    /* Generate the C# code that closes the namespace. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
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
    char *scopedStructName;
    idl_action action;
    SACSTypeUserData *csUserData = (SACSTypeUserData *) userData;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);

    scopedStructName = idl_scopeStackCsharp(
            idl_typeUserScope(idl_typeUser(structSpec)),
            ".",
            idl_typeSpecName(idl_typeSpec(structSpec)));
    if (idl_isPredefined(scopedStructName)) {
        /* return idl_abort to indicate that the structure does not need to be processed. */
        action = idl_abort;
    } else {
        /* Translate the name of the struct into a valid C# identifier. */
        c_type structType = idl_typeSpecDef(idl_typeSpec(structSpec));
        c_char *structDBName = idl_CsharpScopeStackFromCType(structType, csUserData->customPSM, TRUE, FALSE);

        /* Generate the Database representation of the datatype. */
        idl_generateDatabaseRepresentation(structSpec, structDBName, csUserData);

        /* Generate the Marshaler for the datatype. */
        idl_generateMarshaler(structSpec, structDBName, scopedStructName, csUserData);

        /* return idl_explore to indicate that the rest of the structure needs to be processed. */
        action = idl_explore;
        os_free(structDBName);
    }
    os_free(scopedStructName);
    return action;
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
    char *scopedUnionName;
    idl_action action;
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescriptionSplDcps *umd = NULL;

    OS_UNUSED_ARG(name);

    scopedUnionName = idl_scopeStackCsharp(
            idl_typeUserScope(idl_typeUser(unionSpec)),
            ".",
            idl_typeSpecName(idl_typeSpec(unionSpec)));
    if (idl_isPredefined(scopedUnionName)) {
        /* return idl_abort to indicate that the structure does not need to be processed. */
        os_free(scopedUnionName);
        action = idl_abort;
    } else {
        umd = os_malloc(sizeof(SACSUnionMetaDescriptionSplDcps));
        memset(umd, 0, sizeof(SACSUnionMetaDescriptionSplDcps));
        umd->gUnionType = idl_typeSpecDef(idl_typeSpec(unionSpec));
        umd->gUnionScope = scope;
        umd->fullyScopedName = scopedUnionName;
        csUserData->typeStack = c_iterInsert(csUserData->typeStack, umd);
        /* return idl_explore to indicate that the rest of the structure needs to be processed. */
        action = idl_explore;
    }
    return action;
}

/** @brief callback function called on closure of a union in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
   =>   };
   @endverbatim
 *
 * The union is closed:
 * @verbatim
            } _u;
        };
   @endverbatim
 * @param name Name of the union
 */
static void
idl_unionClose (
    const char *name,
    void *userData)
{
    SACSTypeUserData *csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescriptionSplDcps *umd;
    c_char *unionDBName;

    OS_UNUSED_ARG(name);

    /* Translate the name of the struct into a valid C# identifier. */
    umd = (SACSUnionMetaDescriptionSplDcps *) c_iterTakeFirst(csUserData->typeStack);
    assert(umd->gUnionType);
    unionDBName = idl_CsharpScopeStackFromCType(umd->gUnionType, csUserData->customPSM, TRUE, FALSE);

    /* Generate the Database representation of the datatype. */
    idl_generateUnionDatabaseRepresentation(umd->gUnionType, unionDBName);

    /* Generate the Marshaler for the datatype. */
    idl_generateUnionMarshaler(umd->gUnionType, unionDBName, csUserData, umd);

    os_free(unionDBName);
    umd->gUnionType = NULL;
    umd->gUnionGenDefault = FALSE;
    os_free(umd->fullyScopedName);
    os_free(umd);
}


/** @brief callback function called when no default case is defined in an union
 *   for which not all possible label values are specified
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param labelVal Default value for the label case (lowest possible not used index)
 * @param typeSpec Specifies the type of the union switch
 */
static void
idl_artificialDefaultLabelOpenClose(
    idl_scope scope,
    idl_labelVal labelVal,
    idl_typeSpec typeSpec,
    void *userData)
{
    SACSTypeUserData *csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescriptionSplDcps *umd;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(labelVal);
    OS_UNUSED_ARG(typeSpec);

    umd = (SACSUnionMetaDescriptionSplDcps *) c_iterObject(csUserData->typeStack, 0);
    umd->gUnionGenDefault = TRUE;
}


/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
 */
static idl_programControl idl_genSACSLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genSACSLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program idl_genSACSSplDcps;

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSACSSplDcpsProgram(
        void *userData)
{
    idl_genSACSSplDcps.idl_getControl                   = idl_getControl;
    idl_genSACSSplDcps.fileOpen                         = idl_fileOpen;
    idl_genSACSSplDcps.fileClose                        = NULL;
    idl_genSACSSplDcps.moduleOpen                       = idl_moduleOpen;
    idl_genSACSSplDcps.moduleClose                      = idl_moduleClose;
    idl_genSACSSplDcps.structureOpen                    = idl_structureOpen;
    idl_genSACSSplDcps.structureClose                   = NULL;
    idl_genSACSSplDcps.structureMemberOpenClose         = NULL;
    idl_genSACSSplDcps.enumerationOpen                  = NULL;
    idl_genSACSSplDcps.enumerationClose                 = NULL;
    idl_genSACSSplDcps.enumerationElementOpenClose      = NULL;
    idl_genSACSSplDcps.unionOpen                        = idl_unionOpen;
    idl_genSACSSplDcps.unionClose                       = idl_unionClose;
    idl_genSACSSplDcps.unionCaseOpenClose               = NULL;
    idl_genSACSSplDcps.unionLabelsOpenClose             = NULL;
    idl_genSACSSplDcps.unionLabelOpenClose              = NULL;
    idl_genSACSSplDcps.typedefOpenClose                 = NULL;
    idl_genSACSSplDcps.boundedStringOpenClose           = NULL;
    idl_genSACSSplDcps.sequenceOpenClose                = NULL;
    idl_genSACSSplDcps.constantOpenClose                = NULL;
    idl_genSACSSplDcps.artificialDefaultLabelOpenClose  = idl_artificialDefaultLabelOpenClose;
    idl_genSACSSplDcps.userData                         = userData;

    return &idl_genSACSSplDcps;
}

static c_char *
idl_cTypeToCSharpDatabaseRepresentation(
        c_type memberType,
        c_bool fullyScoped)
{
    c_char *dbType = NULL;

    memberType = c_typeActualType(memberType);
    switch (c_baseObjectKind(memberType))
    {
    case M_PRIMITIVE:
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
        case P_CHAR:
        case P_OCTET:
            dbType = os_strdup("byte");
            break;
        case P_SHORT:
            dbType = os_strdup("short");
            break;
        case P_USHORT:
            dbType = os_strdup("ushort");
            break;
        case P_LONG:
            dbType = os_strdup("int");
            break;
        case P_ULONG:
            dbType = os_strdup("uint");
            break;
        case P_LONGLONG:
            dbType = os_strdup("long");
            break;
        case P_ULONGLONG:
            dbType = os_strdup("ulong");
            break;
        case P_FLOAT:
            dbType = os_strdup("float");
            break;
        case P_DOUBLE:
            dbType = os_strdup("double");
            break;
        default:
            /* Unsupported structure member type */
            assert(FALSE);
        }
        break;
    case M_ENUMERATION:
        dbType = os_strdup("uint");
        break;
    case M_STRUCTURE:
    case M_UNION:
        dbType = idl_CsharpScopeStackFromCType(memberType, FALSE, FALSE, fullyScoped);
        if (idl_isPredefined(dbType)) {
            c_char *tmp = os_strdup(idl_translateIfPredefined(dbType));
            os_free(dbType);
            dbType = tmp;
        } else {
            os_free(dbType);
            dbType = idl_CsharpScopeStackFromCType(memberType, FALSE, TRUE, fullyScoped);
        }
        break;
    case M_COLLECTION:
        switch (c_collectionTypeKind(memberType))
        {
        case OSPL_C_STRING:
        case OSPL_C_SEQUENCE:
            dbType = os_strdup("IntPtr");
            break;
        case OSPL_C_ARRAY:
            while (c_baseObjectKind(memberType) == M_COLLECTION &&
                    c_collectionTypeKind(memberType) == OSPL_C_ARRAY)
            {
                memberType = c_typeActualType(c_collectionTypeSubType(memberType));
            }
            dbType = idl_cTypeToCSharpDatabaseRepresentation(memberType, TRUE);
            break;
        default:
            /* Unsupported structure member type */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported structure member type */
        assert(FALSE);
        break;
    }

    return dbType;
}

static c_ulonglong
idl_determineDatabaseFlatArraySize(c_type arrayType)
{
    c_ulonglong nrElements = 1;

    assert(c_baseObjectKind(arrayType) == M_COLLECTION &&
                c_collectionTypeKind(arrayType) == OSPL_C_ARRAY);

    while (c_baseObjectKind(arrayType) == M_COLLECTION &&
            c_collectionTypeKind(arrayType) == OSPL_C_ARRAY)
    {
        nrElements *= c_collectionTypeMaxSize(arrayType);
        arrayType = c_typeActualType(c_collectionTypeSubType(arrayType));
    }

    return nrElements;
}

static c_ulonglong
idl_determineDatabaseSize(
    c_type type)
{
    c_ulonglong size;

    if (c_baseObjectKind(type) == M_COLLECTION && c_collectionTypeKind(type) == OSPL_C_ARRAY) {
        c_ulonglong nrElements = 1;

        while (c_baseObjectKind(type) == M_COLLECTION && c_collectionTypeKind(type) == OSPL_C_ARRAY) {
            nrElements *= c_collectionTypeMaxSize(type);
            type = c_typeActualType(c_collectionTypeSubType(type));
        }
        size = nrElements * type->size;
    } else {
        size = c_typeActualType(type)->size;
    }
    return size;
}

static c_type
idl_getArrayElementType(
     c_type arrayType)
{
    assert(c_baseObjectKind(arrayType) == M_COLLECTION && c_collectionTypeKind(arrayType) == OSPL_C_ARRAY);

    while (c_baseObjectKind(arrayType) == M_COLLECTION && c_collectionTypeKind(arrayType) == OSPL_C_ARRAY) {
        arrayType = c_typeActualType(c_collectionTypeSubType(arrayType));
    }

    return arrayType;
}

static c_char *
idl_genAttrElemVarName(
    const c_char *item,
    c_ulong index,
    c_ulong level)
{
    c_char *name;
    os_size_t size;

    size = 8 + strlen(item) + 2 *10 + 1;
    name = os_malloc(size);
    snprintf(name, size, "attr%uElem%u%s", index, level, item);

    return name;
}


static c_char *
idl_genAttrArrVarName(
    const c_char *item,
    c_ulong index,
    c_ulong level)
{
    c_char *name;
    os_size_t size;

    size = 7 + strlen(item) + 2 *10 + 1;
    name = os_malloc(size);
    snprintf(name, size, "attr%uVar%u%s", index, level, item);

    return name;
}

static c_char *
idl_genAttrSeqVarName(
    const c_char *item,
    c_ulong index,
    c_ulong level)
{
    c_char *name;
    os_size_t size;

    size = 7 + strlen(item) + 2 *10 + 1;
    name = os_malloc(size);
    snprintf(name, size, "attr%uSeq%u%s", index, level, item);

    return name;
}

static c_char *
idl_genAttrColVarName(
    const c_char *item,
    c_ulong index,
    c_ulong level)
{
    c_char *name;
    os_size_t size;

    size = 7 + strlen(item) + 2 *10 + 1;
    name = os_malloc(size);
    snprintf(name, size, "attr%uCol%u%s", index, level, item);

    return name;
}

static c_char *
idl_genTypeNameFromCType(c_type type)
{
    idl_typeSpec typeSpec = idl_makeTypeSpec(type);
    c_char *tName = idl_CsharpTypeFromTypeSpec(typeSpec, FALSE);
    idl_freeTypeSpec(typeSpec);

    return tName;
}

static c_type
idl_genGetContainerType(c_type collStartType, c_ulong dimension)
{
    c_type containerType = collStartType;
    c_ulong i;

    for (i = 0; i < dimension; i++) {
        assert(c_baseObjectKind(containerType) == M_COLLECTION);
        containerType = c_typeActualType(c_collectionTypeSubType(containerType));
    }
    return containerType;
}

static void
idl_generateDatabaseRepresentation (
    idl_typeStruct typeSpec,
    const c_char *structDBName,
    SACSTypeUserData *csUserData)
{
    /* Get the meta-data of this datatype from the database. */
    c_type structType = idl_typeSpecDef(idl_typeSpec(typeSpec));
    c_ulong i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %s\n", structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Sequential)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public struct %s\n",
        structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrMembers; i++) {
        c_char *dbType;

        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_typeActualType(c_memberType(structMember));
        c_char *memberName = idl_CsharpId(
                c_specifierName(structMember),
                csUserData->customPSM,
                FALSE);

        /* Now find and generate the corresponding database representation. */
        dbType = idl_cTypeToCSharpDatabaseRepresentation(memberType, TRUE);

        /* generate the the database representation for the attribute.  */
        idl_printIndent(indent_level);
        if (c_baseObjectKind(memberType) == M_COLLECTION && c_collectionTypeKind(memberType) == OSPL_C_ARRAY) {
            idl_fileOutPrintf(
                    idl_fileCur(),
                    "[MarshalAs(UnmanagedType.ByValArray, SizeConst=%"PA_PRIu64")]\n",
                    idl_determineDatabaseFlatArraySize(memberType));
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "public %s[] %s;\n", dbType, memberName);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "public %s %s;\n", dbType, memberName);
        }

        os_free(dbType);
        os_free(memberName);
    }

    /* Decrease the indentation level and generate the closing bracket. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
}


static void
idl_generateUnionDatabaseRepresentation (
    c_type unionType,
    const c_char *unionDBName)
{
    /* Get the meta-data of this datatype from the database. */
    c_ulong i, nrMembers = c_unionUnionCaseCount((c_union)unionType);
    c_ulonglong maxSize = 0;

    for (i = 0; i < nrMembers; i++) {
        c_ulonglong size;

        c_unionCase unionCase = c_unionUnionCase(unionType, i);
        c_type unionCaseType = c_typeActualType(c_unionCaseType(unionCase));

        size = idl_determineDatabaseSize(unionCaseType);
        maxSize = (size > maxSize) ? size : maxSize;
    }


    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %s\n", unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Explicit, Pack=1)]\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public struct %s\n", unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    {
        c_char *dbType;
        c_type unionDiscrType = c_typeActualType(c_unionUnionSwitchType((c_union)unionType));

        /* Now find and generate the corresponding database representation. */
        dbType = idl_cTypeToCSharpDatabaseRepresentation(unionDiscrType, TRUE);

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "[FieldOffset(0)]\n");

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "public %s _d;\n", dbType);

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "[FieldOffset(%"PA_PRIuSIZE")]\n", sizeof(void *));

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "[MarshalAs(UnmanagedType.ByValArray, SizeConst=%"PA_PRIu64")]\n", maxSize);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "public byte[] _u;\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "public void Init()\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    _u = new byte[%"PA_PRIu64"];\n", maxSize);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");

        os_free(dbType);
    }

    /* Decrease the indentation level and generate the closing bracket. */
    indent_level--;

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
}


static void
idl_determineFullyScopedName(
        c_type structType)
{
    c_char *fullyScopedName = c_metaScopedName(c_metaObject(structType));

    /* Generate the code to obtain the fully-scoped name of the datatype. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
            idl_fileCur(),
            "public static readonly string fullyScopedName = \"%s\";\n",
            fullyScopedName);
    os_free(fullyScopedName);
}

static c_longlong
determineNrEventualElements(c_type collType)
{
    c_longlong nrEventualElements = 1;

    while (c_baseObjectKind(collType) == M_COLLECTION && c_collectionTypeKind(collType) == OSPL_C_ARRAY) {
        nrEventualElements *= c_collectionTypeMaxSize(collType);
        collType = c_typeActualType(c_collectionTypeSubType(collType));
    }
    return nrEventualElements;
}

static void
idl_CreateAttributes(
        c_type structType,
        const c_char *structName,
        SACSTypeUserData *csUserData)
{
    c_bool isPredefined = FALSE;
    c_ulong i, j, nrMembers = c_structureMemberCount((c_structure) structType);
    c_longlong nrElements;

    OS_UNUSED_ARG(structName);

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrMembers; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_memberType(structMember);
        c_char *memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, FALSE, TRUE);
        if (idl_isPredefined(memberTypeName)) {
            isPredefined = TRUE;
        } else {
            isPredefined = FALSE;
            os_free(memberTypeName);
            memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, TRUE, TRUE);
        }

        /* Dereference possible typedefs first. */
        while (c_baseObjectKind(memberType) == M_TYPEDEF && !isPredefined) {
            os_free(memberTypeName);
            memberType = c_typeDef(memberType)->alias;
            memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, FALSE, TRUE);
            if (idl_isPredefined(memberTypeName)) {
                isPredefined = TRUE;
            } else {
                isPredefined = FALSE;
                os_free(memberTypeName);
                memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, TRUE, TRUE);
            }
        }

        switch(c_baseObjectKind(memberType))
        {
        case M_COLLECTION:
            /* Iterate to the element type of the collection. */
            for (j = 0; c_baseObjectKind(memberType) == M_COLLECTION; j++) {
                if (c_collectionTypeKind(memberType) == OSPL_C_SEQUENCE ||
                        c_collectionTypeKind(memberType) == OSPL_C_ARRAY) {
                    c_type subType = c_typeActualType(c_collectionTypeSubType(memberType));
                    c_char *subTypeName = idl_CsharpScopeStackFromCType(subType, FALSE, FALSE, TRUE);
                    c_char *subTypeDBName = idl_cTypeToCSharpDatabaseRepresentation(subType, TRUE);
                    nrElements = determineNrEventualElements(subType);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "private IntPtr attr%dCol%dType = IntPtr.Zero;\n", i, j);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "private static readonly int attr%dCol%dSize = %"PA_PRId64" * Marshal.SizeOf(typeof(%s));\n", i, j, nrElements, subTypeDBName);
                    if (idl_isPredefined(subTypeName)) isPredefined = TRUE;
                    os_free(subTypeDBName);
                    os_free(subTypeName);
                }
                memberType = c_typeActualType(c_collectionTypeSubType(memberType));
                os_free(memberTypeName);
                memberTypeName = idl_CsharpScopeStackFromCType(memberType, csUserData->customPSM, TRUE, TRUE);
            }
            if (c_baseObjectKind(memberType) != M_STRUCTURE && c_baseObjectKind(memberType) != M_UNION) {
                break;
            } else {
            /* No break statement here!!
             * This break-through is intentional, since in case the sequence
             * type is a structure, its Marshaler must also be cached.
             */
            }
        case M_STRUCTURE:
        case M_UNION:
            if (!isPredefined) {

                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "private %sMarshaler attr%dMarshaler;\n", memberTypeName, i);
            }
            break;
        default:
            /* Fine: constructor doesn't need to do anything in particular here. */
            break;
        }
        os_free(memberTypeName);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n");
}

static void
idl_CreateInitEmbeddedMarshalers(
        c_type structType,
        const c_char *structName)
{
    c_bool isPredefined;
    c_ulong i, nrMembers = c_structureMemberCount((c_structure) structType);
    OS_UNUSED_ARG(structName);

    OS_UNUSED_ARG(structName);

    /* Open the constructor itself and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
            "public override void InitEmbeddedMarshalers(IDomainParticipant participant)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrMembers; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_memberType(structMember);
        c_char *memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, FALSE, TRUE);

        if (idl_isPredefined(memberTypeName)) {
            isPredefined = TRUE;
        } else {
            isPredefined = FALSE;
            os_free(memberTypeName);
            memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, TRUE, TRUE);
        }

        /* Dereference possible typedefs/arrays/sequences first. */
        while ( !isPredefined &&
                ( c_baseObjectKind(memberType) == M_TYPEDEF ||
                ( c_baseObjectKind(memberType) == M_COLLECTION &&
                        ( c_collectionTypeKind(memberType) == OSPL_C_ARRAY ||
                          c_collectionTypeKind(memberType) == OSPL_C_SEQUENCE) ) ) ) {
            os_free(memberTypeName);
            if (c_baseObjectKind(memberType) == M_TYPEDEF) {
                memberType = c_typeDef(memberType)->alias;
            } else if (c_baseObjectKind(memberType) == M_COLLECTION) {
                memberType = c_collectionTypeSubType(memberType);
            }
            memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, FALSE, TRUE);
            if (idl_isPredefined(memberTypeName)) {
                isPredefined = TRUE;
            } else {
                isPredefined = FALSE;
                os_free(memberTypeName);
                memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, TRUE, TRUE);
            }
        }

        switch(c_baseObjectKind(memberType))
        {
        case M_STRUCTURE:
        case M_UNION:
            if (!isPredefined) {
                os_char *prevTypeName = memberTypeName;
                memberTypeName = idl_CsharpId(prevTypeName, FALSE, FALSE);
                os_free(prevTypeName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (attr%dMarshaler == null) {\n",i);
                indent_level++;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "attr%dMarshaler = DatabaseMarshaler.GetMarshaler(participant, typeof(%s)) as %sMarshaler;\n",
                        i, memberTypeName, memberTypeName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (attr%dMarshaler == null) {\n", i);
                indent_level++;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "attr%dMarshaler = new %sMarshaler();\n",
                        i, memberTypeName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "DatabaseMarshaler.Add(participant, typeof(%s), attr%dMarshaler);\n",
                        memberTypeName, i);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "attr%dMarshaler.InitEmbeddedMarshalers(participant);\n", i);
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            break;
        default:
            /* Fine: constructor doesn't need to do anything in particular here. */
            break;
        }
        os_free(memberTypeName);
    }

    /* Decrease the indent level back to its original value and close the constructor. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static c_char *
idl_CreateCSharpArrayIterationIndex(
        c_type collStartType,
        c_ulong dimension)
{
#define IDL_INDEX_BUFFER_LENGTH 32

    c_ulong i;
    c_type nextType;
    c_type currentType = c_typeActualType(collStartType);

    /* maxResultLength = '\0' + dimension * ('[' + ']' + nrDecimals(MaxInt) + 'i') */
    c_ulong maxResultLength = 1 + dimension * (2 + 10 + 1);
    c_char *result = os_malloc(maxResultLength);
    c_char *format = NULL;
    c_char postfix[IDL_INDEX_BUFFER_LENGTH];

    assert (result != NULL);

    /* If no dimension specified, return an empty string. */
    result[0] = '\0';
    if (dimension < 1) return result;

    /* Assert that startType is always a collection type and has at least
     * the specified number of dimensions. */
    maxResultLength -= 1;
    os_strncat(result, "[i0", maxResultLength);
    maxResultLength -= 3;
    for (i = 1; i < dimension; i++) {
        /* Iterate to the subtype. */
        nextType = c_typeActualType(c_collectionTypeSubType(currentType));

        /* Assert currentType and nextType are collection types. */
        switch (c_collectionTypeKind(currentType))
        {
        case OSPL_C_SEQUENCE:
            format = "][i%d";
            (void)snprintf (postfix, IDL_INDEX_BUFFER_LENGTH, format, i);
            break;
        case OSPL_C_ARRAY:
            if (c_collectionTypeKind(nextType) == OSPL_C_ARRAY)
            {
                format = ",i%d";
                (void)snprintf (postfix, IDL_INDEX_BUFFER_LENGTH, format, i);
            }
            else
            {
                format = "][i%d";
                (void)snprintf (postfix, IDL_INDEX_BUFFER_LENGTH, format, i);
            }
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
        }
        currentType = nextType;
        os_strncat (result, postfix, maxResultLength);
    }
    os_strncat(result, "]", maxResultLength);

    return result;
#undef IDL_INDEX_BUFFER_LENGTH
}

static void
idl_genColTypeInitialization(
        const c_char *colTypeName,
        const c_char *idlFieldName,
        c_ulong index,
        c_ulong dimension)
{
    /* Locate the database type-definition of the sequence. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) {\n", colTypeName);
    indent_level++;
    idl_printIndent(indent_level);
    if (dimension == 0) {
        idl_fileOutPrintf(idl_fileCur(),
                "IntPtr memberOwnerType = DDS.OpenSplice.Database.c.resolve(c.getBase(typePtr), fullyScopedName);\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "IntPtr specifier = DDS.OpenSplice.Database.c.metaResolveSpecifier(memberOwnerType, \"%s\");\n",
                idlFieldName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(),
                "DDS.OpenSplice.Database.c_collectionType colType = (DDS.OpenSplice.Database.c_collectionType) Marshal.PtrToStructure(\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "        attr%dCol%dType, typeof(DDS.OpenSplice.Database.c_collectionType));\n", index, dimension -1);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "IntPtr specifierType = colType.subType;\n");
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
            "%s = DDS.OpenSplice.Database.c.typeActualType(specifierType);\n",
            colTypeName);
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
}

static c_char *
idl_CreateDatabaseArrayIterationIndex(
        c_type collStartType,
        c_ulong dimension)
{
    c_ulong i, j;
    c_type nextType;
    c_type currentType = c_typeActualType(collStartType);
    c_char *indexStr;
    c_char *prevIndexStr = os_strdup("");

    /* If no dimension specified, return an empty string. */
    if (dimension < 1) return prevIndexStr;

    /* Assert that startType is always a collection type and has at least
     * the specified number of dimensions. */
    for (i = 0; i < (dimension - 1); i++) {
        /* Iterate to the subtype. */
        nextType = c_typeActualType(c_collectionTypeSubType(currentType));

        /* Assert currentType and nextType are collection types. */
        switch (c_collectionTypeKind(currentType))
        {
        case OSPL_C_SEQUENCE:
            (void)os_asprintf (&indexStr, "%si%d][", prevIndexStr, i);
            break;
        case OSPL_C_ARRAY:
            if (c_collectionTypeKind(nextType) == OSPL_C_ARRAY) {
                c_type iteratorType = nextType;
                (void)os_asprintf (&indexStr, "%si%d", prevIndexStr, i);
                for (j = i; j < dimension - 1; j++) {
                    os_free(prevIndexStr);
                    prevIndexStr = indexStr;
                    (void)os_asprintf (&indexStr, "%s*%d", prevIndexStr, c_collectionTypeMaxSize(iteratorType));
                    iteratorType = c_typeActualType(c_collectionTypeSubType(iteratorType));
                    if (c_baseObjectKind(iteratorType) != M_COLLECTION ||
                            c_collectionTypeKind(iteratorType) != OSPL_C_ARRAY) break;
                }
                os_free(prevIndexStr);
                prevIndexStr = indexStr;
                (void)os_asprintf (&indexStr, "%s + ", prevIndexStr);
            }
            else {
                (void)os_asprintf (&indexStr, "%s][i%d", prevIndexStr, i);
            }
            break;
        default:
            /* Unsupported Collection type. */
            indexStr = NULL;
            assert(FALSE);
            break;
        }
        currentType = nextType;
        os_free(prevIndexStr);
        prevIndexStr = indexStr;
    }
    (void)os_asprintf (&indexStr, "[%si%d]", prevIndexStr, i);
    os_free(prevIndexStr);

    return indexStr;
}

static void
idl_CreateArrayMemberWriteInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *bufName,
        c_bool isSeqOfArray)
{
    c_ulong maxSize;
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *databaseArrayBrackets = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension);
    c_char *collTypeName;

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        collTypeName = idl_CsharpScopeStackFromCType(collType, FALSE, TRUE, TRUE);
        idl_printIndent(indent_level);
        if (idl_isPredefined((collTypeName))) {
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = from.%s%s;\n", fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
        } else {
            if (isSeqOfArray) {
                idl_fileOutPrintf(idl_fileCur(),
                        "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, from.%s%s, %s);\n",
                        index, fieldName, cSharpArrayBrackets, bufName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "%s = new IntPtr(%s.ToInt64() + attr%dCol%dSize);\n",
                        bufName, bufName, index, dimension);
            } else {
                idl_fileOutPrintf(idl_fileCur(),
                        "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, from.%s%s, ref to.%s%s);\n",
                        index, fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (result != V_COPYIN_RESULT.OK) return result;\n");
        }
        os_free(collTypeName);
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        if (isSeqOfArray) {
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.WriteInt32(%s, 0, (int) from.%s%s);\n",
                    bufName, fieldName, cSharpArrayBrackets);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%s = new IntPtr(%s.ToInt64() + 4);\n", bufName, bufName);
        } else {
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (uint) from.%s%s;\n",
                    fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
        }
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        idl_printIndent(indent_level);
        switch (c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (from.%s%s ? (byte) 1 : (byte) 0);\n",
                    fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
            break;
        case P_CHAR:
            idl_fileOutPrintf(idl_fileCur(), "to.%s%s = (byte) (from.%s%s);\n",
                    fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
            break;
        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = from.%s%s;\n",
                    fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION:
        /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "if (from.%s%s == null) return V_COPYIN_RESULT.INVALID;\n", fieldName, cSharpArrayBrackets);
        maxSize = c_collectionTypeMaxSize(collType);
        /* Check for bounds if a maximum bound was specified. */
        if (maxSize > 0)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (from.%s%s.Length > %u) return V_COPYIN_RESULT.INVALID;\n", fieldName, cSharpArrayBrackets, maxSize);
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "if (!Write(c.getBase(typePtr), ref to.%s%s, from.%s%s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n",
                fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
        break;
    default:
        /* Unsupported array type. */
        assert(FALSE);
        break;
    }

    os_free(databaseArrayBrackets);
    os_free(cSharpArrayBrackets);
}

static void
idl_CreateSequenceMemberWriteInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *bufName)
{
    c_ulong maxSize;
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *collTypeName;

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        collTypeName = idl_scopeStackFromCType(collType);
        idl_printIndent(indent_level);
        if (idl_isPredefined((collTypeName))) {
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(%s, 0, from.%s%s);\n", bufName, fieldName, cSharpArrayBrackets);
        } else {
            idl_fileOutPrintf(idl_fileCur(),
                               "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, from.%s%s, %s);\n",
                               index, fieldName, cSharpArrayBrackets, bufName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (result != V_COPYIN_RESULT.OK) return result;\n");
        }
        os_free(collTypeName);
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "Marshal.WriteInt32(%s, (int) from.%s%s);\n",
                bufName, fieldName, cSharpArrayBrackets);
        break;
    case M_PRIMITIVE:
        switch(c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.WriteByte(%s, from.%s%s ? (byte) 1 : (byte) 0);\n",
                    bufName, fieldName, cSharpArrayBrackets);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.WriteByte(%s, (byte) from.%s%s);\n",
                    bufName, fieldName, cSharpArrayBrackets);
            break;
        /* Other primitives should have been handled by the caller already, using Marshal.Copy. */
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION:
        /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "if (from.%s%s == null) return V_COPYIN_RESULT.INVALID;\n", fieldName, cSharpArrayBrackets);
        maxSize = c_collectionTypeMaxSize(collType);
        /* Check for bounds if a maximum bound was specified. */
        if (maxSize > 0)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (from.%s%s.Length > %u) return V_COPYIN_RESULT.INVALID;\n", fieldName, cSharpArrayBrackets, maxSize);
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "IntPtr stringElementPtr = IntPtr.Zero;\n");
        idl_printIndent(indent_level);

        idl_fileOutPrintf(idl_fileCur(), "if (!Write(c.getBase(typePtr), ref stringElementPtr, from.%s%s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n",
                fieldName, cSharpArrayBrackets);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "Marshal.WriteIntPtr(%s, stringElementPtr);\n", bufName);
        break;
    default:
        /* Unsupported array type. */
        assert(FALSE);
        break;
    }

    os_free(cSharpArrayBrackets);
}

static void
idl_CreateArrayMemberWrite(
        c_type collStartType,
        c_type collType,
        c_ulong index,
        c_ulong dimension,
        const c_char *idlFieldName,
        const c_char *csFieldName,
        const c_char *bufName,
        c_bool isSeqOfArray)
{
    c_ulong arrLength, seqMaxLength;
    c_char *seqLengthName, *colTypeName, *nextBufTypeSize, *iterationIndex;
    c_type subType, actualType, arrElemType;
    c_metaKind actualTypeKind;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
    case M_UNION:
        idl_CreateArrayMemberWriteInnerLoopBody(
                collStartType, collType, collKind, index, dimension, csFieldName, bufName, isSeqOfArray);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case OSPL_C_STRING:
            /* Handle strings. */
            idl_CreateArrayMemberWriteInnerLoopBody(
                    collStartType, collType, collKind, index, dimension, csFieldName, bufName, isSeqOfArray);
            break;
        case OSPL_C_ARRAY:
            /* Handle arrays. */
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            arrElemType = idl_getArrayElementType(collType);

            colTypeName = idl_genAttrColVarName("Type", index, dimension);
            idl_genColTypeInitialization(colTypeName, idlFieldName, index, dimension);

            if (c_baseObjectKind(arrElemType) == M_PRIMITIVE &&
                    c_primitiveKind(arrElemType) != P_BOOLEAN &&
                    c_primitiveKind(arrElemType) != P_CHAR) {
                c_char *primName = idl_genTypeNameFromCType(subType);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "System.Buffer.BlockCopy(from.%s, 0, to.%s, 0, (%"PA_PRIu64" * Marshal.SizeOf(typeof(%s))));\n",
                        csFieldName, csFieldName, idl_determineDatabaseFlatArraySize(collStartType), primName);
                os_free(primName);
            } else {
                /* Open a for loop to walk over the current dimension and increase the indent. */
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                        dimension, dimension, arrLength, dimension);
                indent_level++;

                /* Generate the the body to process the next dimension. */
                idl_CreateArrayMemberWrite(collStartType, actualType, index,
                        dimension + 1, idlFieldName, csFieldName, bufName, isSeqOfArray);

                /* Close the for loop and decrease the indent level back to its original. */
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            os_free(colTypeName);
            break;
        case OSPL_C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            actualTypeKind = c_baseObjectKind(actualType);

            seqLengthName = idl_genAttrSeqVarName("Length", index, dimension);
            colTypeName = idl_genAttrColVarName("Type", index, dimension);

            seqMaxLength = c_collectionTypeMaxSize(collType);
            iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            /* Check whether the sequence is valid. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s%s == null) return V_COPYIN_RESULT.INVALID;\n",
                    csFieldName, iterationIndex);

            /* Get its Length, and check it for validity. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "int %s = from.%s%s.Length;\n",
                    seqLengthName, csFieldName, iterationIndex);
            if (seqMaxLength == 0)
            {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "// Unbounded sequence: bounds check not required...\n");
            }
            else
            {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (%s > %u) return V_COPYIN_RESULT.INVALID;\n", seqLengthName, seqMaxLength);
            }

            idl_genColTypeInitialization(colTypeName, idlFieldName, index, dimension);

            /* Allocate a matching array in the database. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr %s = DDS.OpenSplice.Database.c.newSequence(%s, %s);\n",
                    bufName, colTypeName, seqLengthName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (%s == IntPtr.Zero) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n", bufName);
            if (dimension == 0 ||
                    c_collectionTypeKind(idl_genGetContainerType(collStartType, dimension - 1)) == OSPL_C_ARRAY) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "to.%s%s = %s;\n", csFieldName, iterationIndex, bufName);
            }

            switch(actualTypeKind)
            {
                case M_PRIMITIVE:
                    switch (c_primitiveKind(actualType))
                    {
                    case P_USHORT:
                    case P_ULONG:
                    case P_ULONGLONG:
                    {
                        const char *signedCounterPart;
                        if (c_primitiveKind(actualType) == P_USHORT) {
                            signedCounterPart = "Int16";
                        } else if (c_primitiveKind(actualType) == P_ULONG) {
                            signedCounterPart = "Int32";
                        } else {
                            signedCounterPart = "Int64";
                        }
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "Marshal.Copy((%s[]) (Array) from.%s%s, 0, %s, %s);\n",
                                signedCounterPart, csFieldName, iterationIndex, bufName, seqLengthName);
                        break;
                    }
                    case P_SHORT:
                    case P_LONG:
                    case P_LONGLONG:
                    case P_FLOAT:
                    case P_DOUBLE:
                    case P_OCTET:
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "Marshal.Copy(from.%s%s, 0, %s, %s);\n", csFieldName, iterationIndex, bufName, seqLengthName);
                        break;
                    default:
                        /* Intentionally fall through to the default handler. */
                        break;
                    }
                    if (c_primitiveKind(actualType) != P_BOOLEAN && c_primitiveKind(actualType) != P_CHAR) break;
                default:
                    nextBufTypeSize = idl_genAttrColVarName("Size", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "for (int i%u = 0; i%u < %s; i%u++) {\n",
                            dimension, dimension, seqLengthName, dimension);
                    indent_level++;
                    if (actualTypeKind == M_COLLECTION && c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                        subType = c_typeActualType(c_collectionTypeSubType(actualType));
                        if (c_collectionTypeKind(actualType) == OSPL_C_ARRAY) {
                            if (c_baseObjectKind(subType) == M_PRIMITIVE) {
                                os_free(iterationIndex);
                                iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension + 1);
                                idl_printIndent(indent_level);
                                idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(from.%s%s, 0, %s, %d);\n",
                                        csFieldName, iterationIndex, bufName, c_collectionTypeMaxSize(actualType));
                            } else {
                                idl_CreateArrayMemberWrite(
                                        collStartType, actualType, index, dimension + 1, idlFieldName, csFieldName, bufName, TRUE);
                            }
                        } else {
                            c_ulong seqBufNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
                            c_char *seqBufName = os_malloc(seqBufNameSize);
                            snprintf(seqBufName, seqBufNameSize, "attr%uSeq%uBuf", index, dimension + 1);
                            idl_CreateArrayMemberWrite(collStartType, actualType, index,
                                    dimension + 1, idlFieldName, csFieldName, seqBufName, isSeqOfArray);
                            if (c_collectionTypeKind(collType) == OSPL_C_SEQUENCE) {
                                if (c_baseObjectKind(subType) == M_PRIMITIVE) {
                                    idl_printIndent(indent_level);
                                    idl_fileOutPrintf(idl_fileCur(),
                                            "Marshal.WriteIntPtr(%s, %s);\n", bufName, seqBufName);
                                } else {
                                    idl_printIndent(indent_level);
                                    idl_fileOutPrintf(idl_fileCur(),
                                            "Marshal.WriteIntPtr(%s, new IntPtr(%s.ToInt64() - ((long) attr%dCol%dSize * (long) attr%dSeq%dLength)));\n",
                                            bufName, seqBufName, index, dimension + 1, index, dimension + 1);
                                }
                            }
                            os_free(seqBufName);
                        }
                    } else {
                        idl_CreateSequenceMemberWriteInnerLoopBody(
                                collStartType, actualType, actualTypeKind, index, dimension + 1,
                                csFieldName, bufName);
                    }
                    if (actualTypeKind != M_COLLECTION || c_collectionTypeKind(actualType) != OSPL_C_ARRAY) {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "%s = new IntPtr(%s.ToInt64() + %s);\n", bufName, bufName, nextBufTypeSize);
                    }
                    indent_level--;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
                    os_free(nextBufTypeSize);
                    break;
            }

            os_free(iterationIndex);
            os_free(colTypeName);
            os_free(seqLengthName);
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
}


static void
idl_CreateStructMemberWrite(
        c_type memberType,
        const char *idlMemberName,
        const char *csMemberName,
        c_ulong index,
        SACSTypeUserData *csUserData)
{
    c_ulong maxSize, dimension;
    c_ulonglong totalNrElements;
    c_char *bufName = NULL;
    c_bool isPredefined = FALSE;
    c_char *memberTypeName = idl_scopeStackFromCType(memberType);
    c_type arrType;

    OS_UNUSED_ARG(csUserData);

    if (idl_isPredefined(memberTypeName)) {
        isPredefined = TRUE;
    }

    /* Dereference possible typedefs first. */
    while (c_baseObjectKind(memberType) == M_TYPEDEF && !isPredefined) {
        os_free(memberTypeName);
        memberType = c_typeDef(memberType)->alias;
        memberTypeName = idl_scopeStackFromCType(memberType);
        if (idl_isPredefined(memberTypeName)) {
            isPredefined = TRUE;
        }
    }

    switch(c_baseObjectKind(memberType))
    {
    case M_STRUCTURE:
    case M_UNION:
        if (isPredefined)
        {
            /* Generate code to use the predefined Write operation in the base marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = from.%s;\n", csMemberName, csMemberName);
        }
        else
        {
            /* Otherwise generate code to use the dedicated marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                               "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, from.%s, ref to.%s);\n",
                               index,
                               csMemberName,
                               csMemberName);
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (result != V_COPYIN_RESULT.OK) return result;\n");
            idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(), "}\n");
        }
        break;
    case M_ENUMERATION:
        /* Generate code to handle an enum. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s = (uint) from.%s;\n",
                csMemberName,
                csMemberName);
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s = from.%s ? (byte) 1 : (byte) 0;\n",
                    csMemberName,
                    csMemberName);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = (byte) from.%s;\n", csMemberName, csMemberName);
            break;
        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = from.%s;\n", csMemberName, csMemberName);
            break;
        default:
            /* Unsupported primitive type. */
            assert(FALSE);
            break;
        }
        break;
    case M_COLLECTION:
        /* Generate code to handle Collection types. */
        switch (c_collectionTypeKind(memberType))
        {
        case OSPL_C_STRING:
            /* Generate code to handle strings. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s == null) return V_COPYIN_RESULT.INVALID;\n", csMemberName);
            /* Generate code to check for bounds if a maximum bound was specified. */
            maxSize = c_collectionTypeMaxSize(memberType);
            idl_printIndent(indent_level);
            if (maxSize > 0)
            {
                idl_fileOutPrintf(idl_fileCur(),
                        "if (from.%s.Length > %d) return V_COPYIN_RESULT.INVALID;\n",
                        csMemberName,
                        maxSize);
            }
            else
            {
                idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (!Write(c.getBase(typePtr), ref to.%s, from.%s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n",
                    csMemberName,
                    csMemberName);
            break;
        case OSPL_C_ARRAY:
            /* Check that the input is non-null. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s == null) return V_COPYIN_RESULT.INVALID;\n", csMemberName);

            /* Check that the input has the correct array size. */
            arrType = memberType;
            for (dimension = 0; c_baseObjectKind(arrType) == M_COLLECTION && c_collectionTypeKind(arrType) == OSPL_C_ARRAY; dimension++)
            {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (from.%s.GetLength(%d) != %d) return V_COPYIN_RESULT.INVALID;\n",
                        csMemberName, dimension, c_collectionTypeMaxSize(arrType));
                arrType = c_typeActualType(c_collectionTypeSubType(arrType));
            }

            /* Initialize the database sample to the correct Array size. */
            totalNrElements = idl_determineDatabaseFlatArraySize(memberType);
            bufName = idl_cTypeToCSharpDatabaseRepresentation(memberType, TRUE);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = new %s[%"PA_PRIu64"];\n", csMemberName, bufName, totalNrElements);

            /* Generate the body of the copying loop. */
            idl_CreateArrayMemberWrite(
                    memberType, memberType, index, 0, idlMemberName, csMemberName, csMemberName, FALSE);
            os_free(bufName);
            break;
        case OSPL_C_SEQUENCE:
            bufName = idl_genAttrSeqVarName("Buf", index, 0);

            idl_CreateArrayMemberWrite(
                    memberType, memberType, index, 0, idlMemberName, csMemberName, bufName, FALSE);

            arrType = c_typeActualType(c_collectionTypeSubType(memberType));
            if (c_baseObjectKind(arrType) == M_PRIMITIVE &&
                    c_primitiveKind(arrType) != P_BOOLEAN && c_primitiveKind(arrType) != P_CHAR) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),"to.%s = %s;\n", csMemberName, bufName);
            }
            os_free(bufName);
            break;

        default:
            /* Unsupported Collection type. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported Base type. */
        assert(FALSE);
        break;
    }
    os_free(memberTypeName);
}

static void
idl_CreateCopyIn(
        c_type structType,
        const c_char *structDBName,
        const c_char *fullyScopedName,
        SACSTypeUserData *csUserData)
{
    c_ulong i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Open the 1st CopyIn operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, System.IntPtr from, System.IntPtr to)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate code that obtains a C# object from a C pointer and then invokes the 2nd CopyIn. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandle = GCHandle.FromIntPtr(from);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s fromData = tmpGCHandle.Target as %s;\n", fullyScopedName, fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return CopyIn(typePtr, fromData, to);\n");
    /* Decrease the indent level back to its original value and close the 1st CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Generate code that creates a C# projection of the database, invoke the 3rd CopyIn with it, and marshal it into unmanaged memory. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, %s from, System.IntPtr to)\n", fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = new %s();\n", structDBName, structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "V_COPYIN_RESULT result = CopyIn(typePtr, from, ref nativeImg);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (result == V_COPYIN_RESULT.OK)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "Marshal.StructureToPtr(nativeImg, to, false);\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return result;\n");

    /* Decrease the indent level back to its original value and close the 2nd CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 2nd CopyIn operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, %s from, ref %s to)\n", fullyScopedName, structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate code that checks whether the C# input is a valid object. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (from == null) return V_COPYIN_RESULT.INVALID;\n");

    /* Now generate the code that loops through the attributes and generates the appropriate instructions. */
    for (i = 0; i < nrMembers; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_memberType(structMember);
        c_char *memberName = idl_CsharpId(
                        c_specifierName(structMember),
                        csUserData->customPSM,
                        FALSE);
        idl_CreateStructMemberWrite(memberType, c_specifierName(structMember), memberName, i, csUserData);
        os_free(memberName);
    }

    /* Generate code that returns true when everything went fine so far. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return V_COPYIN_RESULT.OK;\n");

    /* Decrease the indent level back to its original value and close the 3rd CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
idl_useRecycledArray(
        idl_typeSpec typeSpec,
        c_type currentType,
        c_type subType,
        const c_char *fieldName,
        const c_char *seqLengthName,
        const c_char *arrayBrackets,
        const c_char *arrayIndex,
        const c_char *arrayCtor)
{
    c_char *arrayNoIndex;

    switch (c_collectionTypeKind(currentType)) {
        case OSPL_C_SEQUENCE:
            arrayNoIndex = idl_sequenceCsharpIndexString(
                typeSpec, SACS_EXCLUDE_INDEXES, seqLengthName);
            break;
        case OSPL_C_ARRAY:
            arrayNoIndex = idl_arrayCsharpIndexString(
                typeSpec, SACS_EXCLUDE_INDEXES);
            break;
        default:
            assert(FALSE); /* Unsupported collection type. */
            arrayNoIndex = NULL;
            break;
    }

    if (arrayNoIndex != NULL) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s target = new %s%s;\n",
                arrayCtor, arrayNoIndex, arrayCtor, arrayIndex);
        if (c_collectionTypeKind(currentType) == OSPL_C_ARRAY &&
            c_baseObjectKind(subType) == M_COLLECTION &&
            c_collectionTypeKind(subType) == OSPL_C_ARRAY)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Rectangular array: recycling not yet possible...\n");
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "initObjectSeq(to.%s%s, target);\n",
                    fieldName, arrayBrackets);
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "to.%s%s = target;\n",
                fieldName, arrayBrackets);
        os_free(arrayNoIndex);
    }
}

static void
idl_CreateArrayInitialization(
        c_type currentType,
        const c_char *fieldName,
        const c_char *seqLengthName,
        const c_char *arrayBrackets,
        SACSTypeUserData *csUserData)
{
    c_char *arrayIndex = NULL, *arrayCtor;
    c_type subType = c_typeActualType(c_collectionTypeSubType(currentType));
    idl_typeSpec typeSpec = idl_makeTypeCollection(c_collectionType(currentType));
    c_char *subTypeName;

    /* Resolve the typename of the current collection. */
    arrayCtor = idl_CsharpTypeFromTypeSpec(typeSpec, csUserData->customPSM);

   /* Resolve the index representation for the current collection. */
    switch (c_collectionTypeKind(currentType))
    {
    case OSPL_C_SEQUENCE:
        arrayIndex = idl_sequenceCsharpIndexString(
                typeSpec, SACS_INCLUDE_INDEXES, seqLengthName);
        break;
    case OSPL_C_ARRAY:
        arrayIndex = idl_arrayCsharpIndexString(typeSpec, SACS_INCLUDE_INDEXES);
        break;
    default:
        /* Unsupported collection type. */
        assert(FALSE);
        break;
    }

    /* The initialization code for the current collection depends on its subtype. */
    switch (c_baseObjectKind(subType))
    {
    /* Primitive kind, no need to recycle elements. */
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "to.%s%s = new %s%s;\n",
                fieldName, arrayBrackets, arrayCtor, arrayIndex);
        break;
    /* Object kind, try to recycle elements as much as possible. */
    case M_STRUCTURE:
    case M_UNION:
        subTypeName = idl_scopeStackFromCType(subType);
        if (idl_isPredefined(subTypeName)) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s%s = new %s%s;\n",
                    fieldName, arrayBrackets, arrayCtor, arrayIndex);
        } else {
            idl_useRecycledArray(typeSpec, currentType, subType, fieldName, seqLengthName, arrayBrackets, arrayIndex, arrayCtor);
        }
        os_free(subTypeName);
        break;
    case M_COLLECTION:
        idl_useRecycledArray(typeSpec, currentType, subType, fieldName, seqLengthName, arrayBrackets, arrayIndex, arrayCtor);
        break;
    default:
        /* Unsupported collection type. */
        assert(FALSE);
        break;
    }
    os_free(arrayIndex);
    os_free(arrayCtor);
    idl_typeArrayFree(idl_typeArray(typeSpec));
}

static void
idl_generateArrayStatusCheck(
        c_type collStartType,
        c_type collType,
        c_type *eventualElementType,
        c_ulong dimension,
        const c_char *fieldName,
        SACSTypeUserData *csUserData)
{
    c_ulong i;
    c_char *arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

    /* Check if this dimension is part of a prior rectangular array by checking
     * whether its parent is also an array.
     */
    c_type parentType = collStartType;
    for (i = 1; i < dimension; i++)
    {
        parentType = c_typeActualType(c_collectionTypeSubType(parentType));
    }
    if (parentType == collType ||
            (c_baseObjectKind(parentType) != M_COLLECTION ||
             c_collectionTypeKind(parentType) != OSPL_C_ARRAY))
    {
        /* If no prior rectangular dimensions, then check the current dimensions. */
        c_ulong curDim;
        os_size_t dimensionCheckSize = 15 + strlen(fieldName) + strlen (arrayBrackets) + 1;
        c_char *dimensionCheck = os_malloc(dimensionCheckSize);
        snprintf(dimensionCheck, dimensionCheckSize,
                "to.%s%s == null", fieldName, arrayBrackets);
        *eventualElementType = c_typeActualType(collType);
        for (i = 0; c_baseObjectKind(*eventualElementType) == M_COLLECTION &&
                    c_collectionTypeKind(*eventualElementType) == OSPL_C_ARRAY; i++)
        {
            c_char *prevdimensionCheck = dimensionCheck;
            curDim = c_collectionTypeMaxSize(*eventualElementType);
            dimensionCheckSize = strlen(prevdimensionCheck) + 26 + strlen(fieldName) +
                    strlen(arrayBrackets) + 2 * 10 /* MAX_INT */ + 1;
            dimensionCheck = os_malloc(dimensionCheckSize);
            snprintf(dimensionCheck, dimensionCheckSize,
                    "%s || to.%s%s.GetLength(%d) != %d",
                    prevdimensionCheck, fieldName, arrayBrackets, i, curDim);
            os_free(prevdimensionCheck);
            *eventualElementType = c_typeActualType(c_collectionTypeSubType(*eventualElementType));
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "if (%s) {\n", dimensionCheck);
        indent_level++;
        idl_CreateArrayInitialization(collType, fieldName, "", arrayBrackets, csUserData);
        indent_level--;
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        os_free(dimensionCheck);
    }
    else
    {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "// Not the start of a rectangular array: initialization not required...\n");
    }
    os_free(arrayBrackets);
}

static void
idl_CreateArrayMemberReadInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *bufName,
        c_bool isSeqOfArray)
{
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *databaseArrayBrackets = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_CsharpScopeStackFromCType(collType, FALSE, FALSE, TRUE);

    OS_UNUSED_ARG(index);
    OS_UNUSED_ARG(bufName);

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        idl_printIndent(indent_level);
        if (idl_isPredefined(colTypeName)) {
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = from.%s%s;\n",
                    fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
        } else {
            /* Get the CSharp specific name of the member type. */
            if (isSeqOfArray) {
                idl_fileOutPrintf(idl_fileCur(),
                        "attr%dMarshaler.CopyOut(%s, ref to.%s%s);\n",
                        index, bufName, fieldName, cSharpArrayBrackets);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "%s = new IntPtr(%s.ToInt64() + attr%dCol%dSize);\n",
                        bufName, bufName, index, dimension);
            } else {
                c_char *colTypeDbName = idl_CsharpScopeStackFromCType(collType, FALSE, TRUE, TRUE);
                idl_fileOutPrintf(idl_fileCur(),
                        "%sMarshaler.CopyOut(ref from.%s%s, ref to.%s%s);\n",
                        colTypeDbName, fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
                os_free(colTypeDbName);
            }
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        if (isSeqOfArray) {
            c_char *targetEnumType = idl_CsharpScopeStackFromCType(collType, FALSE, FALSE, TRUE);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (%s) Marshal.ReadInt32(%s);\n",
                    fieldName, cSharpArrayBrackets, targetEnumType, bufName);
            os_free(targetEnumType);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%s = new IntPtr(%s.ToInt64() + 4);\n", bufName, bufName);
        } else {
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (%s) from.%s%s;\n",
                    fieldName, cSharpArrayBrackets, colTypeName, fieldName, databaseArrayBrackets);
        }
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        idl_printIndent(indent_level);
        switch (c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (from.%s%s != 0 ? true : false);\n",
                    fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
            break;
        case P_CHAR:
            idl_fileOutPrintf(idl_fileCur(), "(to.%s%s) = (char) (from.%s%s);\n",
                    fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
            break;
        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = from.%s%s;\n",
                    fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION: /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s%s = ReadString(from.%s%s);\n",
                fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
    os_free(colTypeName);
    os_free(databaseArrayBrackets);
    os_free(cSharpArrayBrackets);
}

static void
idl_CreateSequenceMemberReadInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *bufName,
        SACSTypeUserData *csUserData)
{

    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_CsharpScopeStackFromCType(collType, csUserData->customPSM, FALSE, TRUE);

    OS_UNUSED_ARG(index);

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        idl_printIndent(indent_level);
        if (idl_isPredefined(colTypeName)) {
            /* Function below returns const string, so no need to free it afterward. */
            const c_char *predefinedTypeName = idl_translateIfPredefined(colTypeName);
            /* Now remove the preceeding 'DDS.' from the typename, so start reading at character position 4. */
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = Read%s(%s, 0);\n",
                    fieldName, cSharpArrayBrackets, &predefinedTypeName[4], bufName);
        } else {
            c_char *colDbTypeName = idl_CsharpScopeStackFromCType(collType, csUserData->customPSM, TRUE, TRUE);
            /* Get the CSharp specific name of the member type. */
            idl_fileOutPrintf(idl_fileCur(),
                    "%sMarshaler.StaticCopyOut(%s, ref to.%s%s);\n",
                    colDbTypeName, bufName, fieldName, cSharpArrayBrackets);
            os_free(colDbTypeName);
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s%s = (%s) Marshal.ReadInt32(%s);\n",
                fieldName, cSharpArrayBrackets, colTypeName, bufName);
        break;
    case M_PRIMITIVE:
        switch (c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = Marshal.ReadByte(%s) != 0 ? true : false;\n",
                    fieldName, cSharpArrayBrackets, bufName);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (char) Marshal.ReadByte(%s);\n",
                    fieldName, cSharpArrayBrackets, bufName);
            break;
        /* Other primitives should have been handled by the caller already, using Marshal.Copy. */
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION: /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "IntPtr stringElementPtr = Marshal.ReadIntPtr(%s);\n",
                bufName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s%s = ReadString(stringElementPtr);\n",
                fieldName, cSharpArrayBrackets);
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
    os_free(colTypeName);
    os_free(cSharpArrayBrackets);
}

static void
idl_CreateArrayMemberRead(
        c_type collStartType,
        c_type collType,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *bufName,
        c_bool isSeqOfArray,
        SACSTypeUserData *csUserData)
{
    c_ulong arrLength;
    c_ulong seqLengthNameSize, nextBufTypeSizeLen;
    c_char *seqLengthName, *nextBufTypeSize, *arrayBrackets, *nextBuf;
    c_type subType, actualType, eventualElementType;
    c_metaKind actualTypeKind;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
    case M_UNION:
        idl_CreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                index, dimension, fieldName, bufName, isSeqOfArray);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case OSPL_C_STRING:
            /* Handle strings. */
            idl_CreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                    index, dimension, fieldName, bufName, isSeqOfArray);
            break;
        case OSPL_C_ARRAY:
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            eventualElementType = actualType;
            idl_generateArrayStatusCheck(collStartType, collType, &eventualElementType, dimension, fieldName, csUserData);
            if (c_baseObjectKind(eventualElementType) == M_PRIMITIVE &&
                    c_primitiveKind(eventualElementType) != P_BOOLEAN &&
                    c_primitiveKind(eventualElementType) != P_CHAR) {
                c_char *primName = idl_genTypeNameFromCType(eventualElementType);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "System.Buffer.BlockCopy(from.%s, 0, to.%s, 0, (%"PA_PRIu64" * Marshal.SizeOf(typeof(%s))));\n",
                        fieldName, fieldName, idl_determineDatabaseFlatArraySize(collStartType), primName);
                os_free(primName);
            } else {
                if (c_baseObjectKind(actualType) == M_COLLECTION && c_collectionTypeKind(actualType) == OSPL_C_SEQUENCE) {
                    c_char *bufIndex = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension + 1);
                    size_t bufNameLength = strlen(bufName) + strlen(bufIndex) + 1;
                    nextBuf = os_malloc(bufNameLength);
                    snprintf(nextBuf, bufNameLength, "%s%s", bufName, bufIndex);
                    os_free(bufIndex);
                } else {
                    nextBuf = os_strdup(bufName);
                }
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                        dimension, dimension, arrLength, dimension);
                indent_level++;
                idl_CreateArrayMemberRead(collStartType, actualType, index,
                        dimension + 1, fieldName, nextBuf, isSeqOfArray, csUserData);
                os_free(nextBuf);
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            break;
        case OSPL_C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            actualTypeKind = c_baseObjectKind(actualType);

            seqLengthNameSize = 13 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqLengthName = os_malloc(seqLengthNameSize);
            snprintf(seqLengthName, seqLengthNameSize, "attr%uSeq%uLength", index, dimension);

            arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "int %s = DDS.OpenSplice.Database.c.arraySize(%s);\n",
                    seqLengthName, bufName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (to.%s%s == null || to.%s%s.Length != %s) {\n",
                    fieldName, arrayBrackets, fieldName, arrayBrackets, seqLengthName);
            indent_level++;
            idl_CreateArrayInitialization(collType, fieldName, seqLengthName, arrayBrackets, csUserData);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            switch(actualTypeKind)
            {
                case M_PRIMITIVE:
                    switch (c_primitiveKind(actualType))
                    {
                    case P_USHORT:
                    case P_ULONG:
                    case P_ULONGLONG:
                    {
                        const char *signedCounterPart;
                        if (c_primitiveKind(actualType) == P_USHORT) {
                            signedCounterPart = "short";
                        } else if (c_primitiveKind(actualType) == P_ULONG) {
                            signedCounterPart = "int";
                        } else {
                            signedCounterPart = "long";
                        }
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "if(%s > 0) Marshal.Copy(%s, (%s[]) (Array) to.%s%s, 0, %s);\n",
                                seqLengthName, bufName, signedCounterPart, fieldName, arrayBrackets, seqLengthName);
                        break;
                    }
                    case P_SHORT:
                    case P_LONG:
                    case P_LONGLONG:
                    case P_FLOAT:
                    case P_DOUBLE:
                    case P_OCTET:
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "if(%s > 0) Marshal.Copy(%s, to.%s%s, 0, %s);\n",
                                seqLengthName, bufName, fieldName, arrayBrackets, seqLengthName);
                        break;
                    default:
                        /* Intentionally break through to use the default handler. */
                        break;
                    }
                    if (c_primitiveKind(actualType) != P_BOOLEAN && c_primitiveKind(actualType) != P_CHAR) break;
                default:
                    nextBufTypeSizeLen = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
                    nextBufTypeSize = os_malloc(nextBufTypeSizeLen);
                    snprintf(nextBufTypeSize, nextBufTypeSizeLen, "attr%uCol%uSize", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %s; i%d++) {\n",
                            dimension, dimension, seqLengthName, dimension);
                    indent_level++;
                    if (actualTypeKind == M_COLLECTION && c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                        subType = c_typeActualType(c_collectionTypeSubType(actualType));
                        if (c_collectionTypeKind(actualType) == OSPL_C_ARRAY) {
                            if (c_baseObjectKind(subType) == M_PRIMITIVE) {
                                os_free(arrayBrackets);
                                arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension + 1);
                                idl_printIndent(indent_level);
                                idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(%s, to.%s%s, 0, %d);\n",
                                        bufName, fieldName, arrayBrackets, c_collectionTypeMaxSize(actualType));
                            } else {
                                idl_CreateArrayMemberRead(collStartType, actualType, index,
                                                        dimension + 1, fieldName, bufName, TRUE, csUserData);
                            }
                        } else {
                            c_ulong seqBufNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
                            c_char *seqBufName = os_malloc(seqBufNameSize);
                            snprintf(seqBufName, seqBufNameSize, "attr%uSeq%uBuf", index, dimension + 1);
                            if (c_collectionTypeKind(actualType) == OSPL_C_SEQUENCE) {
                                idl_printIndent(indent_level);
                                idl_fileOutPrintf(idl_fileCur(),
                                        "IntPtr %s = Marshal.ReadIntPtr(%s);\n", seqBufName, bufName);
                            }
                            idl_CreateArrayMemberRead(collStartType, actualType, index,
                                    dimension + 1, fieldName, seqBufName, isSeqOfArray, csUserData);
                            os_free(seqBufName);
                        }
                    } else {
                        idl_CreateSequenceMemberReadInnerLoopBody(
                                collStartType, actualType, actualTypeKind, index, dimension + 1,
                                fieldName, bufName, csUserData);
                    }
                    if (actualTypeKind != M_COLLECTION || c_collectionTypeKind(actualType) != OSPL_C_ARRAY) {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "%s = new IntPtr(%s.ToInt64() + %s);\n", bufName, bufName, nextBufTypeSize);
                    }
                    indent_level--;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
                    os_free(arrayBrackets);
                    os_free(nextBufTypeSize);
                    os_free(seqLengthName);
                    break;
            }
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
}

static void
idl_CreateStructMemberRead(
        c_type memberType,
        const char *memberName,
        c_ulong index,
        SACSTypeUserData *csUserData)
{
    os_size_t bufNameLength;
    c_char *bufName = NULL;
    c_bool isPredefined = FALSE;
    c_char *memberTypeName = idl_CsharpScopeStackFromCType(memberType, FALSE, FALSE, TRUE);

    if (idl_isPredefined(memberTypeName)) {
        isPredefined = TRUE;
    }

    /* Dereference possible typedefs first. */
    while (c_baseObjectKind(memberType) == M_TYPEDEF && !isPredefined) {
        os_free(memberTypeName);
        memberType = c_typeDef(memberType)->alias;
        memberTypeName = idl_CsharpScopeStackFromCType(memberType, csUserData->customPSM, FALSE, TRUE);
        if (idl_isPredefined(memberTypeName)) {
            isPredefined = TRUE;
        }
    }

    switch(c_baseObjectKind(memberType))
    {
    case M_STRUCTURE:
    case M_UNION:
        if (isPredefined)
        /* Predefined types can use specialized Read operations. */
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s = from.%s;\n", memberName, memberName);
        }
        else
        /* Other types should use their corresponding Marshaler. */
        {
            /* Get the Database specific name of the member type. */
            c_char *dbTypeName = idl_CsharpScopeStackFromCType(memberType, csUserData->customPSM, TRUE, TRUE);

            /* Invoke the Marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%sMarshaler.CopyOut(ref from.%s, ref to.%s);\n",
                    dbTypeName, memberName, memberName);
            os_free(dbTypeName);
        }
        break;
    case M_ENUMERATION:
        /* Enums are read as Int32 types. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s = (%s) from.%s;\n",
                memberName, memberTypeName, memberName);
        break;
    case M_PRIMITIVE:
        /* Handle primitive. */
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = from.%s != 0 ? true : false;\n", memberName, memberName);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = (char) from.%s;\n", memberName, memberName);
            break;
        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = from.%s;\n", memberName, memberName);
            break;
        default:
            /* Unsupported primitive type */
            assert(FALSE);
        }
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(memberType))
        {
        case OSPL_C_STRING:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s = ReadString(from.%s);\n", memberName, memberName);
            break;
        case OSPL_C_ARRAY:
            bufNameLength = 5 + strlen(memberName) + 1; /* "from." + memberName + '\0'. */
            bufName = os_malloc(bufNameLength);
            snprintf(bufName, bufNameLength, "from.%s", memberName);
            idl_CreateArrayMemberRead(
                    memberType, memberType, index, 0, memberName, bufName, FALSE, csUserData);
            os_free(bufName);
            break;
        case OSPL_C_SEQUENCE:
            bufNameLength = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            bufName = os_malloc(bufNameLength);
            snprintf(bufName, bufNameLength, "attr%dSeq%dBuf", index, 0);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr %s = from.%s;\n", bufName, memberName);

            idl_CreateArrayMemberRead(
                    memberType, memberType, index, 0, memberName, bufName, FALSE, csUserData);

            os_free(bufName);
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
        }
        break;
    default:
        /* Unsupported Base type. */
        assert(FALSE);
    }
    os_free(memberTypeName);
}

static void
idl_CreateCopyOut(
        c_type structType,
        const c_char *structDBName,
        const c_char *fullyScopedName,
        SACSTypeUserData *csUserData)
{
    c_ulong i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Open the 1st CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, System.IntPtr to)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = (%s) Marshal.PtrToStructure(from, typeof(%s));\n",
            structDBName, structDBName, structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandleTo = GCHandle.FromIntPtr(to);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s toObj = tmpGCHandleTo.Target as %s;\n", fullyScopedName, fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref toObj);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "tmpGCHandleTo.Target = toObj;\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 2nd CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, ref %s to)\n", fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = (%s) Marshal.PtrToStructure(from, typeof(%s));\n",
            structDBName, structDBName, structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref to);\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 3rd CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public static void StaticCopyOut(System.IntPtr from, ref %s to)\n", fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = (%s) Marshal.PtrToStructure(from, typeof(%s));\n",
            structDBName, structDBName, structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref to);\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 4th CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public static void CopyOut(ref %s from, ref %s to)\n", structDBName, fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Cast the object to its proper type and if no yet allocated, allocate it. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (to == null) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "to = new %s();\n", fullyScopedName);
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");

    /* Now generate the code that loops through the attributes and generates the appropriate instructions. */
    for (i = 0; i < nrMembers; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_memberType(structMember);
        c_char *memberName = idl_CsharpId(
                        c_specifierName(structMember),
                        csUserData->customPSM,
                        FALSE);
        idl_CreateStructMemberRead(memberType, memberName, i, csUserData);
        os_free(memberName);
    }

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
idl_generateMarshaler (
    idl_typeStruct typeSpec,
    const c_char *structDBName,
    const c_char *fullyScopedName,
    SACSTypeUserData *csUserData)
{
    /* Get the meta-data of this datatype from the database. */
    c_type structType = idl_typeSpecDef(idl_typeSpec(typeSpec));

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %sMarshaler\n", structDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public sealed class %sMarshaler : DDS.OpenSplice.CustomMarshalers.FooDatabaseMarshaler<%s>\n",
        structDBName, fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Create the constructor of the Marshaler. */
    idl_determineFullyScopedName(structType);
    idl_CreateAttributes(structType, structDBName, csUserData);
    idl_CreateInitEmbeddedMarshalers(structType, structDBName);
    idl_CreateCopyIn(structType, structDBName, fullyScopedName, csUserData);
    idl_CreateCopyOut(structType, structDBName, fullyScopedName, csUserData);

    /* Decrease the indentation level and generate the closing bracket. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
}


/** @brief Generate a string representaion the literal value of a label
 * in metadata terms.
 *
 * @param labelVal Specifies the kind and the value of the label
 * @return String representing the image of \b labelVal
 */
static void
idl_printCaseLabel (
    c_literal label)
{
    switch (label->value.kind) {
    case V_CHAR:
        idl_fileOutPrintf(idl_fileCur(), "case %u:\n", label->value.is.Char);
        break;
    case V_BOOLEAN:
        idl_fileOutPrintf(idl_fileCur(), "case %d:\n", label->value.is.Boolean);
        break;
    default:
        {
            char *value = c_valueImage(label->value);
            idl_fileOutPrintf(idl_fileCur(), "case %s:\n", value);
            os_free(value);
        }
        break;
    }
}


static char *
idl_getCSharpLabelValue (
    c_literal label)
{
    char buf[16];
    char *value;

    switch (label->value.kind) {
    case V_CHAR:
        snprintf(buf, sizeof(buf), "'%c'", label->value.is.Char);
        value = os_strdup(buf);
        break;
    case V_BOOLEAN:
        snprintf(buf, sizeof(buf), "%s", label->value.is.Boolean ? "true" : "false");
        value = os_strdup(buf);
        break;
    default:
        value = c_valueImage(label->value);
        break;
    }
    return value;
}


/** @brief Return a C# typename corresponging to a case field the IDL type specification
 *
 * @param typeSpec IDL type specification
 * @param customPSM Use custom PSM or not
 */
static char *
idl_unionCaseTypeFromTypeSpec(
    idl_typeSpec typeSpec,
    c_bool customPSM)
{
    char typeName[512];
    char *tname;
    c_bool isPredefined = FALSE;

    tname = idl_CsharpTypeFromTypeSpec(typeSpec, customPSM);

    /* Dereference possible typedefs first. */
    while (idl_typeSpecType(typeSpec) == idl_ttypedef && !isPredefined) {
        if (!idl_isPredefined(tname)) {
            typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
            os_free (tname);
            tname = idl_CsharpTypeFromTypeSpec(typeSpec, customPSM);
        } else {
            isPredefined = TRUE;
        }
    }

    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        snprintf(typeName, sizeof(typeName), "%s", tname);
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        char *str_no_idx = idl_sequenceCsharpIndexString(typeSpec, SACS_EXCLUDE_INDEXES, NULL);
        assert (str_no_idx);
        snprintf (typeName, sizeof(typeName), "%s%s", tname, str_no_idx);
        os_free(str_no_idx);
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        char *str_no_idx = idl_arrayCsharpIndexString(typeSpec, SACS_EXCLUDE_INDEXES);
        assert (str_no_idx);
        snprintf(typeName, sizeof(typeName), "%s%s", tname,  str_no_idx);
        os_free(str_no_idx);
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        /* This state should only be reachable for predefined types. */
        assert(isPredefined);
        snprintf(typeName, sizeof(typeName), "%s", tname);
    } else {
        if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
            (idl_typeSpecType (typeSpec) == idl_tunion) ||
            (idl_typeSpecType (typeSpec) == idl_tenum)) {
            snprintf(typeName, sizeof(typeName), "%s", tname);
        } else {
            printf ("idl_unionCaseTypeFromTypeSpec: Unexpected type %d\n",
                idl_typeSpecType(typeSpec));
        }
    }

    os_free(tname);

    return os_strdup(typeName);
}

static c_char *
idl_CsharpTypeFromCType(
    c_type ctype,
    c_bool customPSM)
{
    c_char *tName;
    idl_typeSpec typeSpec = idl_makeTypeSpec(ctype);
    tName = idl_unionCaseTypeFromTypeSpec(typeSpec, customPSM);
    idl_freeTypeSpec(typeSpec);

    return tName;
}


static void
idl_UnionUseRecycledArray(
        idl_typeSpec typeSpec,
        c_type currentType,
        c_type subType,
        const c_char *dest,
        const c_char *seqLengthName,
        const c_char *arrayBrackets,
        const c_char *arrayIndex,
        const c_char *arrayCtor)
{
    c_char *arrayNoIndex;

    switch (c_collectionTypeKind(currentType)) {
        case OSPL_C_SEQUENCE:
            arrayNoIndex = idl_sequenceCsharpIndexString(
                typeSpec, SACS_EXCLUDE_INDEXES, seqLengthName);
            break;
        case OSPL_C_ARRAY:
            arrayNoIndex = idl_arrayCsharpIndexString(
                typeSpec, SACS_EXCLUDE_INDEXES);
            break;
        default:
            assert(FALSE); /* Unsupported collection type. */
            arrayNoIndex = NULL;
            break;
    }

    if (arrayNoIndex != NULL) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s target = new %s%s;\n",
                arrayCtor, arrayNoIndex, arrayCtor, arrayIndex);
        if (c_collectionTypeKind(currentType) == OSPL_C_ARRAY &&
            c_baseObjectKind(subType) == M_COLLECTION &&
            c_collectionTypeKind(subType) == OSPL_C_ARRAY)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Rectangular array: recycling not yet possible...\n");
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "initObjectSeq(%s%s, target);\n",
                    dest, arrayBrackets);
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s = target;\n",
                dest, arrayBrackets);
        os_free(arrayNoIndex);
    }
}

static void
idl_UnionCreateArrayInitialization(
        c_type currentType,
        const c_char *dstName,
        const c_char *seqLengthName,
        const c_char *arrayBrackets,
        SACSTypeUserData *csUserData)
{
    c_char *arrayIndex = NULL, *arrayCtor;
    c_type subType = c_typeActualType(c_collectionTypeSubType(currentType));
    idl_typeSpec typeSpec = idl_makeTypeCollection(c_collectionType(currentType));
    c_char *subTypeName;

    /* Resolve the typename of the current collection. */
    arrayCtor = idl_CsharpTypeFromTypeSpec(typeSpec, csUserData->customPSM);

   /* Resolve the index representation for the current collection. */
    switch (c_collectionTypeKind(currentType))
    {
    case OSPL_C_SEQUENCE:
        arrayIndex = idl_sequenceCsharpIndexString(
                typeSpec, SACS_INCLUDE_INDEXES, seqLengthName);
        break;
    case OSPL_C_ARRAY:
        arrayIndex = idl_arrayCsharpIndexString(typeSpec, SACS_INCLUDE_INDEXES);
        break;
    default:
        /* Unsupported collection type. */
        assert(FALSE);
        break;
    }

    /* The initialization code for the current collection depends on its subtype. */
    switch (c_baseObjectKind(subType))
    {
    /* Primitive kind, no need to recycle elements. */
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s = new %s%s;\n",
                dstName, arrayBrackets, arrayCtor, arrayIndex);
        break;
    /* Object kind, try to recycle elements as much as possible. */
    case M_STRUCTURE:
    case M_UNION:
        subTypeName = idl_scopeStackFromCType(subType);
        if (idl_isPredefined(subTypeName)) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s%s = new %s%s;\n",
                    dstName, arrayBrackets, arrayCtor, arrayIndex);
        } else {
            idl_UnionUseRecycledArray(typeSpec, currentType, subType, dstName, seqLengthName, arrayBrackets, arrayIndex, arrayCtor);
        }
        os_free(subTypeName);
        break;
    case M_COLLECTION:
        idl_UnionUseRecycledArray(typeSpec, currentType, subType, dstName, seqLengthName, arrayBrackets, arrayIndex, arrayCtor);
        break;
    default:
        /* Unsupported collection type. */
        assert(FALSE);
        break;
    }
    os_free(arrayIndex);
    os_free(arrayCtor);
    idl_freeTypeSpec(typeSpec);
}

static void
idl_generateUnionArrayStatusCheck(
        c_type collStartType,
        c_type collType,
        c_type *eventualElementType,
        c_ulong dimension,
        const c_char *dstName,
        SACSTypeUserData *csUserData)
{
    c_ulong i;
    c_char *arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    /* Check if this dimension is part of a prior rectangular array by checking
      * whether its parent is also an array.
      */
    c_type parentType = collStartType;
     for (i = 1; i < dimension; i++)
     {
         parentType = c_typeActualType(c_collectionTypeSubType(parentType));
     }

     if (parentType == collType ||
             (c_baseObjectKind(parentType) != M_COLLECTION ||
              c_collectionTypeKind(parentType) != OSPL_C_ARRAY))
     {
         /* If no prior rectangular dimensions, then check the current dimensions. */
         c_ulong curDim;
         os_size_t dimensionCheckSize = 15 + strlen(dstName) + strlen (arrayBrackets) + 1;
         c_char *dimensionCheck = os_malloc(dimensionCheckSize);
         snprintf(dimensionCheck, dimensionCheckSize, "%s%s == null", dstName, arrayBrackets);
         *eventualElementType = c_typeActualType(collType);
         for (i = 0; c_baseObjectKind(*eventualElementType) == M_COLLECTION &&
                     c_collectionTypeKind(*eventualElementType) == OSPL_C_ARRAY; i++)
         {
             c_char *prevdimensionCheck = dimensionCheck;
             curDim = c_collectionTypeMaxSize(*eventualElementType);
             dimensionCheckSize = strlen(prevdimensionCheck) + 26 + strlen(dstName) +
                     strlen(arrayBrackets) + 2 * 10 /* MAX_INT */ + 1;
             dimensionCheck = os_malloc(dimensionCheckSize);
             snprintf(dimensionCheck, dimensionCheckSize,
                     "%s || %s%s.GetLength(%d) != %d",
                     prevdimensionCheck, dstName, arrayBrackets, i, curDim);
             os_free(prevdimensionCheck);
             *eventualElementType = c_typeActualType(c_collectionTypeSubType(*eventualElementType));
         }
         idl_printIndent(indent_level);
         idl_fileOutPrintf(idl_fileCur(), "if (%s) {\n", dimensionCheck);
         indent_level++;
         idl_UnionCreateArrayInitialization(collType, dstName, "", arrayBrackets, csUserData);
         indent_level--;
         idl_printIndent(indent_level);
         idl_fileOutPrintf(idl_fileCur(), "}\n");
         os_free(dimensionCheck);
     } else {
         idl_printIndent(indent_level);
         idl_fileOutPrintf(idl_fileCur(),
                 "// Not the start of a rectangular array: initialization not required...\n");
     }
     os_free(arrayBrackets);
}

static void
idl_UnionCreateArrayMemberWriteInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *srcName,
        const c_char *dstName)
{
    c_ulong maxSize;
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *databaseArrayBrackets = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension);
    c_char *collTypeName;

    OS_UNUSED_ARG(fieldName);

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        collTypeName = idl_scopeStackFromCType(collType);

        idl_printIndent(indent_level);
        if (idl_isPredefined((collTypeName))) {
            idl_fileOutPrintf(idl_fileCur(), "Write(%s, 0, %s%s);\n", dstName, srcName, cSharpArrayBrackets);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                    "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, %s%s, %s);\n",
                    index, srcName, cSharpArrayBrackets, dstName);
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (result != V_COPYIN_RESULT.OK) return result;\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
        }
        os_free(collTypeName);
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "Marshal.WriteInt32(%s, (int) %s%s);\n", dstName, srcName, cSharpArrayBrackets);
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        idl_printIndent(indent_level);
        switch (c_primitiveKind(collType))
        {
        case P_CHAR:
            idl_fileOutPrintf(idl_fileCur(),"Write(%s, 0, (byte) (%s%s));\n",
                    dstName, srcName, cSharpArrayBrackets);
            break;
        case P_BOOLEAN:
            idl_fileOutPrintf(idl_fileCur(),"Write(%s, 0, (%s%s));\n",
                    dstName, srcName, cSharpArrayBrackets);
            break;
        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(%s, 0, %s%s);\n",
                    dstName, srcName, cSharpArrayBrackets);
            break;
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION:
        /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "if (%s%s == null) return V_COPYIN_RESULT.INVALID;\n", srcName, cSharpArrayBrackets);
        maxSize = c_collectionTypeMaxSize(collType);
        /* Check for bounds if a maximum bound was specified. */
        if (maxSize > 0)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (%s%s.Length > %u) return V_COPYIN_RESULT.INVALID;\n", srcName, cSharpArrayBrackets, maxSize);
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "if (!Write(c.getBase(typePtr), %s, 0, ref %s%s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n",
                dstName, srcName, cSharpArrayBrackets);
        break;
    default:
        /* Unsupported array type. */
        assert(FALSE);
        break;
    }

    os_free(databaseArrayBrackets);
    os_free(cSharpArrayBrackets);
}

static void
idl_UnionCreateSequenceMemberWriteInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *srcName,
        const c_char *dstName)
{
    c_ulong maxSize;
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *collTypeName;

    OS_UNUSED_ARG(fieldName);

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        collTypeName = idl_scopeStackFromCType(collType);
        idl_printIndent(indent_level);
        if (idl_isPredefined((collTypeName))) {
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(%s, 0, %s%s);\n", dstName, srcName, cSharpArrayBrackets);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                               "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, %s%s, %s);\n",
                               index, srcName, cSharpArrayBrackets, dstName);
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (result != V_COPYIN_RESULT.OK) return result;\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
        }
        os_free(collTypeName);
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "Marshal.WriteInt32(%s, (int) %s%s);\n", dstName, srcName, cSharpArrayBrackets);
        break;
    case M_PRIMITIVE:
        switch(c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.WriteByte(%s, from.%s()%s ? (byte) 1 : (byte) 0);\n",
                    dstName, fieldName, cSharpArrayBrackets);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.WriteByte(%s, (byte) from.%s()%s);\n",
                    dstName, fieldName, cSharpArrayBrackets);
            break;
        /* Other primitives should have been handled by the caller already, using Marshal.Copy. */
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION:
        /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "if (%s%s == null) return V_COPYIN_RESULT.INVALID;\n", srcName, cSharpArrayBrackets);
        maxSize = c_collectionTypeMaxSize(collType);
        /* Check for bounds if a maximum bound was specified. */
        if (maxSize > 0) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (%s%s.Length > %u) return V_COPYIN_RESULT.INVALID;\n", srcName, cSharpArrayBrackets, maxSize);
        } else {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
        }

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "if (!Write(c.getBase(typePtr), %s, 0, ref %s%s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n",
                dstName, srcName, cSharpArrayBrackets);

        break;
    default:
        /* Unsupported array type. */
        assert(FALSE);
        break;
    }

    os_free(cSharpArrayBrackets);
}

static void
idl_UnionCreateArrayMemberWriteInner(
        c_type collStartType,
        c_type collType,
        c_ulong index,
        c_ulong dimension,
        const c_char *idlFieldName,
        const c_char *csFieldName,
        const c_char *srcName,
        const c_char *dstName)
{
    c_ulong arrLength, seqMaxLength;
    c_char *seqLengthName, *seqTypeName, *nextBufTypeSize, *iterationIndex;
    c_char *curBufName, *elemSize;
    c_type subType, actualType, arrElemType;
    c_metaKind actualTypeKind;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_PRIMITIVE:
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_UNION:
        idl_UnionCreateArrayMemberWriteInnerLoopBody(
                collStartType, collType, collKind, index, dimension, csFieldName, srcName, dstName);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case OSPL_C_STRING:
            /* Handle strings. */
            idl_UnionCreateArrayMemberWriteInnerLoopBody(
                    collStartType, collType, collKind, index, dimension, csFieldName, srcName, dstName);
            break;
        case OSPL_C_ARRAY:
            /* Handle arrays. */
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            arrElemType = idl_getArrayElementType(collType);

            if (c_baseObjectKind(arrElemType) == M_PRIMITIVE &&
                    c_primitiveKind(arrElemType) != P_BOOLEAN && c_primitiveKind(arrElemType)!= P_CHAR) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "Marshal.Copy(%s, 0, %s, %d);\n", srcName, dstName, c_collectionTypeMaxSize(collType));
            } else {

                curBufName = idl_genAttrArrVarName("Buf", index, dimension);
                elemSize = idl_genAttrColVarName("Size", index, dimension);

                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = new IntPtr(%s.ToInt64());\n", curBufName, dstName);

                if (c_baseObjectKind(arrElemType) == M_COLLECTION && c_collectionTypeKind(arrElemType) == OSPL_C_SEQUENCE) {
                    c_char *arrTypeName = idl_genAttrColVarName("Type", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) {\n", arrTypeName);
                    indent_level++;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "IntPtr memberOwnerType = DDS.OpenSplice.Database.c.resolve(c.getBase(typePtr), fullyScopedName);\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "IntPtr specifier = DDS.OpenSplice.Database.c.metaResolveSpecifier(memberOwnerType, \"%s\");\n",
                            idlFieldName);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");

                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "%s = DDS.OpenSplice.Database.c.typeActualType(specifierType);\n", arrTypeName);
                    indent_level--;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
                    os_free(arrTypeName);
                }

                /* Open a for loop to walk over the current dimension and increase the indent. */
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                        dimension, dimension, arrLength, dimension);
                indent_level++;
                /* Generate the the body to process the next dimension. */
                idl_UnionCreateArrayMemberWriteInner(
                        collStartType, actualType, index, dimension + 1, idlFieldName, csFieldName, srcName, curBufName);
                idl_printIndent(indent_level);

                idl_fileOutPrintf(idl_fileCur(), "%s = new IntPtr(%s.ToInt64() + %s);\n",
                        curBufName, curBufName, elemSize);
                /* Close the for loop and decrease the indent level back to its original. */
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
                os_free(curBufName);
                os_free(elemSize);
            }
            break;
        case OSPL_C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            actualTypeKind = c_baseObjectKind(actualType);

            seqLengthName = idl_genAttrSeqVarName("Length", index, dimension);
            seqTypeName = idl_genAttrColVarName("Type", index, dimension);
            curBufName = idl_genAttrSeqVarName("Buf", index, dimension);

            seqMaxLength = c_collectionTypeMaxSize(collType);
            iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            /* Check whether the sequence is valid. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (%s%s == null) return V_COPYIN_RESULT.INVALID;\n",
                    srcName, iterationIndex);

            /* Get its Length, and check it for validity. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "int %s = %s%s.Length;\n", seqLengthName, srcName, iterationIndex);
            if (seqMaxLength == 0) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "// Unbounded sequence: bounds check not required...\n");
            } else {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (%s > %u) return V_COPYIN_RESULT.INVALID;\n", seqLengthName, seqMaxLength);
            }

            /* Locate the database type-definition of the sequence. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) {\n", seqTypeName);
            indent_level++;
            idl_printIndent(indent_level);
            if (dimension == 0) {
                idl_fileOutPrintf(idl_fileCur(),
                        "IntPtr memberOwnerType = DDS.OpenSplice.Database.c.resolve(c.getBase(typePtr), fullyScopedName);\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "IntPtr specifier = DDS.OpenSplice.Database.c.metaResolveSpecifier(memberOwnerType, \"%s\");\n",
                        idlFieldName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");
            } else {
                idl_fileOutPrintf(idl_fileCur(),
                        "DDS.OpenSplice.Database.c_collectionType colType = (DDS.OpenSplice.Database.c_collectionType) Marshal.PtrToStructure(\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "        attr%dCol%dType, typeof(DDS.OpenSplice.Database.c_collectionType));\n", index, dimension -1);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "IntPtr specifierType = colType.subType;\n");
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%s = DDS.OpenSplice.Database.c.typeActualType(specifierType);\n",
                    seqTypeName);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");

            /* Allocate a matching array in the database. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr %s = DDS.OpenSplice.Database.c.newSequence(%s, %s);\n",
                    curBufName, seqTypeName, seqLengthName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (%s == IntPtr.Zero) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n", curBufName);

            switch(actualTypeKind)
            {
                case M_PRIMITIVE:
                    switch (c_primitiveKind(actualType))
                    {
                    case P_USHORT:
                    case P_ULONG:
                    case P_ULONGLONG:
                    {
                        const char *signedCounterPart;
                        if (c_primitiveKind(actualType) == P_USHORT) {
                            signedCounterPart = "Int16";
                        } else if (c_primitiveKind(actualType) == P_ULONG) {
                            signedCounterPart = "Int32";
                        } else {
                            signedCounterPart = "Int64";
                        }
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "Marshal.Copy((%s[]) (Array) %s%s, 0, %s, %s);\n",
                                signedCounterPart, srcName, iterationIndex, curBufName, seqLengthName);
                        break;
                    }
                    case P_SHORT:
                    case P_LONG:
                    case P_LONGLONG:
                    case P_FLOAT:
                    case P_DOUBLE:
                    case P_OCTET:
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "Marshal.Copy(%s%s, 0, %s, %s);\n", srcName, iterationIndex, curBufName, seqLengthName);
                        break;
                    default:
                        /* Intentionally fall through to the default handler. */
                        break;
                    }
                    if (c_primitiveKind(actualType) != P_BOOLEAN && c_primitiveKind(actualType) != P_CHAR) {
                        if (dimension > 0) {
                            idl_printIndent(indent_level);
                            idl_fileOutPrintf(idl_fileCur(), "Marshal.WriteIntPtr(%s, %s);\n", dstName, curBufName);
                        }
                        break;
                    }
                default:
                    if (dimension > 0) {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(), "Marshal.WriteIntPtr(%s, %s);\n", dstName, curBufName);
                    }
                    nextBufTypeSize = idl_genAttrColVarName("Size", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "for (int i%u = 0; i%u < %s; i%u++) {\n",
                            dimension, dimension, seqLengthName, dimension);
                    indent_level++;
                    if (actualTypeKind == M_COLLECTION) {
                        if (c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                            subType = c_typeActualType(c_collectionTypeSubType(actualType));
                            if (c_collectionTypeKind(actualType) == OSPL_C_ARRAY && c_baseObjectKind(subType) == M_PRIMITIVE) {
                                os_free(iterationIndex);
                                iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension + 1);
                                idl_printIndent(indent_level);
                                idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(%s%s, 0, %s, %d);\n",
                                        srcName, iterationIndex, dstName, c_collectionTypeMaxSize(actualType));
                            } else {
                                idl_UnionCreateArrayMemberWriteInner(collStartType, actualType, index,
                                        dimension + 1, idlFieldName, csFieldName, srcName, curBufName);
                            }
                        } else {
                            idl_UnionCreateArrayMemberWriteInner(collStartType, actualType, index,
                                    dimension + 1, idlFieldName, csFieldName, srcName, curBufName);
                        }
                    } else {
                        idl_UnionCreateSequenceMemberWriteInnerLoopBody(
                                collStartType, actualType, actualTypeKind, index, dimension + 1,
                                csFieldName, srcName, curBufName);
                    }
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "%s = new IntPtr(%s.ToInt64() + %s);\n", curBufName, curBufName, nextBufTypeSize);
                    indent_level--;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
                    os_free(nextBufTypeSize);
                    break;
            }

            os_free(iterationIndex);
            os_free(seqTypeName);
            os_free(seqLengthName);
            os_free(curBufName);
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
}


static void
idl_UnionCreateArrayMemberWrite(
        c_type collStartType,
        c_type collType,
        c_ulong index,
        c_ulong dimension,
        const c_char *idlFieldName,
        const c_char *csFieldName,
        const c_char *srcName,
        const c_char *dstName)
{
    c_ulong arrLength;
    c_type subType, actualType, arrElemType;

    /* Handle arrays. */
    arrLength = c_collectionTypeMaxSize(collType);
    subType = c_collectionTypeSubType(collType);
    actualType = c_typeActualType(subType);
    arrElemType = idl_getArrayElementType(collType);

    if (c_baseObjectKind(arrElemType) == M_PRIMITIVE &&
            c_primitiveKind(arrElemType) != P_BOOLEAN && c_primitiveKind(arrElemType)!= P_CHAR) {
        c_char *primName = idl_genTypeNameFromCType(arrElemType);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "System.Buffer.BlockCopy(%s, 0, %s, 0, (%"PA_PRIu64" * Marshal.SizeOf(typeof(%s))));\n",
                srcName, dstName, idl_determineDatabaseFlatArraySize(collType), primName);
        os_free(primName);
    } else {
        c_char *curBufName = idl_genAttrArrVarName("Buf", index, dimension);
        c_char *elemSize = idl_genAttrColVarName("Size", index, dimension);
        c_char *handleName = idl_genAttrArrVarName("Handle", index, dimension);

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "GCHandle %s = GCHandle.Alloc(%s, GCHandleType.Pinned);\n", handleName, dstName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = %s.AddrOfPinnedObject();\n", curBufName, handleName);

        if (c_baseObjectKind(arrElemType) == M_COLLECTION && c_collectionTypeKind(arrElemType) == OSPL_C_SEQUENCE) {
            c_char *arrTypeName = idl_genAttrColVarName("Type", index, dimension);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) {\n", arrTypeName);
            indent_level++;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr memberOwnerType = DDS.OpenSplice.Database.c.resolve(c.getBase(typePtr), fullyScopedName);\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr specifier = DDS.OpenSplice.Database.c.metaResolveSpecifier(memberOwnerType, \"%s\");\n",
                    idlFieldName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s = DDS.OpenSplice.Database.c.typeActualType(specifierType);\n", arrTypeName);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            os_free(arrTypeName);
        }

        /* Open a for loop to walk over the current dimension and increase the indent. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                dimension, dimension, arrLength, dimension);
        indent_level++;
        /* Generate the the body to process the next dimension. */
        idl_UnionCreateArrayMemberWriteInner(collStartType, actualType, index, dimension + 1, idlFieldName, csFieldName, srcName, curBufName);

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s = new IntPtr(%s.ToInt64() + %s);\n",
                curBufName, curBufName, elemSize);
        /* Close the for loop and decrease the indent level back to its original. */

        indent_level--;

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s.Free();\n", handleName);
        os_free(curBufName);
        os_free(elemSize);
        os_free(handleName);
    }
}


static void
idl_UnionCreateSequenceMemberWrite(
        c_type collStartType,
        c_type collType,
        c_ulong index,
        c_ulong dimension,
        const c_char *idlFieldName,
        const c_char *csFieldName,
        const c_char *srcName,
        const c_char *dstName)
{
    c_ulong seqMaxLength;
    c_char *seqLengthName, *seqTypeName, *nextBufTypeSize, *iterationIndex;
    c_char *curBufName;
    c_type subType, actualType;
    c_metaKind actualTypeKind;

    /* Handle sequences. */
    subType = c_collectionTypeSubType(collType);
    actualType = c_typeActualType(subType);
    actualTypeKind = c_baseObjectKind(actualType);

    seqLengthName = idl_genAttrSeqVarName("Length", index, dimension);
    seqTypeName = idl_genAttrColVarName("Type", index, dimension);
    curBufName = idl_genAttrSeqVarName("Buf", index, dimension);

    seqMaxLength = c_collectionTypeMaxSize(collType);
    iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

    /* Check whether the sequence is valid. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (%s%s == null) return V_COPYIN_RESULT.INVALID;\n",
            srcName, iterationIndex);

    /* Get its Length, and check it for validity. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "int %s = %s%s.Length;\n", seqLengthName, srcName, iterationIndex);
    if (seqMaxLength == 0) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "// Unbounded sequence: bounds check not required...\n");
    } else {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "if (%s > %u) return V_COPYIN_RESULT.INVALID;\n", seqLengthName, seqMaxLength);
    }

    /* Locate the database type-definition of the sequence. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) {\n", seqTypeName);
    indent_level++;

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
            "IntPtr memberOwnerType = DDS.OpenSplice.Database.c.resolve(c.getBase(typePtr), fullyScopedName);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
            "IntPtr specifier = DDS.OpenSplice.Database.c.metaResolveSpecifier(memberOwnerType, \"%s\");\n",
            csFieldName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
            "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s = DDS.OpenSplice.Database.c.typeActualType(specifierType);\n", seqTypeName);

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");

    /* Allocate a matching array in the database. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = DDS.OpenSplice.Database.c.newSequence(%s, %s);\n",
            curBufName, seqTypeName, seqLengthName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n", curBufName);

    switch(actualTypeKind)
    {
    case M_PRIMITIVE:
        switch (c_primitiveKind(actualType))
        {
        case P_USHORT:
        case P_ULONG:
        case P_ULONGLONG:
        {
            const char *signedCounterPart;
            if (c_primitiveKind(actualType) == P_USHORT) {
                signedCounterPart = "Int16";
            } else if (c_primitiveKind(actualType) == P_ULONG) {
                signedCounterPart = "Int32";
            } else {
                signedCounterPart = "Int64";
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.Copy((%s[]) (Array) %s%s, 0, %s, %s);\n",
                    signedCounterPart, srcName, iterationIndex, curBufName, seqLengthName);
            break;
        }
        case P_SHORT:
        case P_LONG:
        case P_LONGLONG:
        case P_FLOAT:
        case P_DOUBLE:
        case P_OCTET:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Marshal.Copy(%s%s, 0, %s, %s);\n", srcName, iterationIndex, curBufName, seqLengthName);
            break;
        default:
            /* Intentionally fall through to the default handler. */
            break;
        }
        if (c_primitiveKind(actualType) != P_BOOLEAN && c_primitiveKind(actualType) != P_CHAR) {
            if (dimension > 0) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "Marshal.WriteIntPtr(%s, %s);\n", dstName, curBufName);
            }
            break;
        }
        default:
            nextBufTypeSize = idl_genAttrColVarName("Size", index, dimension);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "for (int i%u = 0; i%u < %s; i%u++) {\n",
                    dimension, dimension, seqLengthName, dimension);
            indent_level++;
            if (actualTypeKind == M_COLLECTION) {
                if (c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                    subType = c_typeActualType(c_collectionTypeSubType(actualType));
                    if (c_collectionTypeKind(actualType) == OSPL_C_ARRAY && c_baseObjectKind(subType) == M_PRIMITIVE) {
                        os_free(iterationIndex);
                        iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension + 1);
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(%s%s, 0, %s, %d);\n",
                                srcName, iterationIndex, dstName, c_collectionTypeMaxSize(actualType));
                    } else {
                        idl_UnionCreateArrayMemberWriteInner(collStartType, actualType, index,
                                dimension + 1, idlFieldName, csFieldName, srcName, curBufName);
                    }
                } else {
                    idl_UnionCreateArrayMemberWriteInner(collStartType, actualType, index,
                            dimension + 1, idlFieldName, csFieldName, srcName, curBufName);
                }
            } else {
                idl_UnionCreateSequenceMemberWriteInnerLoopBody(
                        collStartType, actualType, actualTypeKind, index, dimension + 1,
                        csFieldName, srcName, curBufName);
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%s = new IntPtr(%s.ToInt64() + %s);\n", curBufName, curBufName, nextBufTypeSize);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            os_free(nextBufTypeSize);
            break;
    }

    os_free(iterationIndex);
    os_free(seqTypeName);
    os_free(seqLengthName);
    os_free(curBufName);
}

static void
idl_UnionCreateArrayMemberReadInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *dstName,
        const c_char *srcName,
        SACSTypeUserData *csUserData)
{
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *databaseArrayBrackets = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_CsharpScopeStackFromCType(collType, csUserData->customPSM, FALSE, TRUE);

    OS_UNUSED_ARG(index);
    OS_UNUSED_ARG(fieldName);

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        if (idl_isPredefined(colTypeName)) {
            const c_char *predefinedTypeName = idl_translateIfPredefined(colTypeName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s%s = Read%s(%s, 0);\n",
                    dstName, cSharpArrayBrackets, &predefinedTypeName[4], srcName);
        } else {
            /* Get the CSharp specific name of the member type. */
            c_char *colTypeDBName = idl_CsharpScopeStackFromCType(collType, csUserData->customPSM, TRUE, TRUE);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%sMarshaler.StaticCopyOut(%s, ref %s%s);\n",
                    colTypeDBName, srcName, dstName, cSharpArrayBrackets);
            os_free(colTypeDBName);
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s = (%s) Marshal.ReadInt32(%s);\n",
                dstName, cSharpArrayBrackets, colTypeName, srcName);
        break;
    case M_PRIMITIVE:
        switch (c_primitiveKind(collType)) {
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s%s = ReadChar(%s, 0);\n",
                    dstName, cSharpArrayBrackets, srcName);
            break;
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s%s = ReadBoolean(%s, 0);\n",
                    dstName, cSharpArrayBrackets, srcName);
            break;
        default:
            assert(0);
        }
        break;
    case M_COLLECTION: /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s = ReadString(Marshal.ReadIntPtr(%s));\n",
                dstName, cSharpArrayBrackets, srcName);
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
    os_free(colTypeName);
    os_free(databaseArrayBrackets);
    os_free(cSharpArrayBrackets);
}

static void
idl_UnionCreateSequenceMemberReadInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *dstName,
        const c_char *srcName,
        SACSTypeUserData *csUserData)
{
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_CsharpScopeStackFromCType(collType, csUserData->customPSM, FALSE, TRUE);

    OS_UNUSED_ARG(index);
    OS_UNUSED_ARG(fieldName);

    switch(collKind)
    {
    case M_STRUCTURE:
    case M_UNION:
        idl_printIndent(indent_level);
        if (idl_isPredefined(colTypeName)) {
            /* Function below returns const string, so no need to free it afterward. */
            const c_char *predefinedTypeName = idl_translateIfPredefined(colTypeName);
            /* Now remove the preceeding 'DDS.' from the typename, so start reading at character position 4. */
            idl_fileOutPrintf(idl_fileCur(), "%s%s = Read%s(%s, 0);\n",
                    dstName, cSharpArrayBrackets, &predefinedTypeName[4], srcName);
        } else {
            /* Get the CSharp specific name of the member type. */
            c_char *colTypeDBName = idl_CsharpScopeStackFromCType(collType, csUserData->customPSM, TRUE, TRUE);
            idl_fileOutPrintf(idl_fileCur(), "%sMarshaler.StaticCopyOut(%s, ref %s%s);\n",
                    colTypeDBName, srcName, dstName, cSharpArrayBrackets);
            os_free(colTypeDBName);
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "%s%s = (%s) Marshal.ReadInt32(%s);\n",
                dstName, cSharpArrayBrackets, colTypeName, srcName);
        break;
    case M_PRIMITIVE:
        switch (c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s%s = Marshal.ReadByte(%s) != 0 ? true : false;\n",
                    dstName, cSharpArrayBrackets, srcName);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s%s = (char) Marshal.ReadByte(%s);\n",
                    dstName, cSharpArrayBrackets, srcName);
            break;
        /* Other primitives should have been handled by the caller already, using Marshal.Copy. */
        default:
            assert(0);
            break;
        }
        break;
    case M_COLLECTION: /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == OSPL_C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "IntPtr stringElementPtr = Marshal.ReadIntPtr(%s);\n", srcName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s = ReadString(stringElementPtr);\n", dstName, cSharpArrayBrackets);
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
    os_free(colTypeName);
    os_free(cSharpArrayBrackets);
}


static void
idl_UnionCreateArrayMemberRead(
        c_type collStartType,
        c_type collType,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *dstName,
        const c_char *srcName,
        SACSTypeUserData *csUserData)
{
    c_ulong arrLength;
    c_char *seqLengthName, *seqBufName, *nextBufTypeSize, *arrayBrackets;
    c_type subType, actualType, eventualElementType;
    c_metaKind actualTypeKind;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
    case M_UNION:
        idl_UnionCreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                index, dimension, fieldName, dstName, srcName, csUserData);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case OSPL_C_STRING:
            /* Handle strings. */
            idl_UnionCreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                    index, dimension, fieldName, dstName, srcName, csUserData);
            break;
        case OSPL_C_ARRAY:
            /* Handle arrays. */
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            eventualElementType = actualType;
            arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            idl_generateUnionArrayStatusCheck(collStartType, collType, &eventualElementType, dimension, dstName, csUserData);

            if (c_baseObjectKind(eventualElementType) == M_PRIMITIVE &&
                    c_primitiveKind(eventualElementType) != P_CHAR && c_primitiveKind(eventualElementType) != P_BOOLEAN) {
                idl_printIndent(indent_level);
                if (dimension == 0) {
                    c_char *primName = idl_genTypeNameFromCType(subType);
                    idl_fileOutPrintf(idl_fileCur(), "System.Buffer.BlockCopy(%s, 0, %s, 0, (%"PA_PRIu64" * Marshal.SizeOf(typeof(%s))));\n",
                            srcName, dstName, idl_determineDatabaseFlatArraySize(collStartType), primName);
                    os_free(primName);
                } else {
                    idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(%s,%s%s, 0, %d);\n",
                            srcName, dstName, arrayBrackets, c_collectionTypeMaxSize(collType));
                }
            } else {
                c_char *handleName = idl_genAttrArrVarName("Handle", index, dimension);
                c_char *bufName = idl_genAttrArrVarName("Buf", index, dimension);
                c_char *elemSize = idl_genAttrColVarName("Size", index, dimension);

                if (dimension == 0) {
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "GCHandle %s = GCHandle.Alloc(from._u, GCHandleType.Pinned);\n", handleName);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = %s.AddrOfPinnedObject();\n", bufName, handleName);
                } else {
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = new IntPtr(%s.ToInt64());\n", bufName, srcName);
                }

                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                        dimension, dimension, arrLength, dimension);
                indent_level++;

                idl_UnionCreateArrayMemberRead(collStartType, actualType, index,
                        dimension + 1, fieldName, dstName, bufName, csUserData);

                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "%s = new IntPtr(%s.ToInt64() + %s);\n", bufName, bufName, elemSize);

                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");

                if  (dimension == 0) {
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "%s.Free();\n", handleName);
                }

                os_free(bufName);
                os_free(handleName);
                os_free(elemSize);
            }

            os_free(arrayBrackets);
            break;
        case OSPL_C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            actualTypeKind = c_baseObjectKind(actualType);

            seqLengthName = idl_genAttrSeqVarName("Length", index, dimension);

            arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            if (dimension > 0) {
                seqBufName = idl_genAttrSeqVarName("Buf", index, dimension);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "IntPtr %s = Marshal.ReadIntPtr(%s);\n", seqBufName, srcName);
                srcName = seqBufName;
            } else {
                seqBufName = os_strdup(srcName);
            }

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "int %s = DDS.OpenSplice.Database.c.arraySize(%s);\n",
                    seqLengthName, srcName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (%s%s == null || %s%s.Length != %s) {\n",
                    dstName, arrayBrackets, dstName, arrayBrackets, seqLengthName);
            indent_level++;
            idl_UnionCreateArrayInitialization(collType, dstName, seqLengthName, arrayBrackets, csUserData);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            switch(actualTypeKind)
            {
                case M_PRIMITIVE:
                    switch (c_primitiveKind(actualType))
                    {
                    case P_USHORT:
                    case P_ULONG:
                    case P_ULONGLONG:
                    {
                        const char *signedCounterPart;
                        if (c_primitiveKind(actualType) == P_USHORT) {
                            signedCounterPart = "short";
                        } else if (c_primitiveKind(actualType) == P_ULONG) {
                            signedCounterPart = "int";
                        } else {
                            signedCounterPart = "long";
                        }
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "if(%s > 0) Marshal.Copy(%s, (%s[]) (Array) %s%s, 0, %s);\n",
                                seqLengthName, srcName, signedCounterPart, dstName, arrayBrackets, seqLengthName);
                        break;
                    }
                    case P_SHORT:
                    case P_LONG:
                    case P_LONGLONG:
                    case P_FLOAT:
                    case P_DOUBLE:
                    case P_OCTET:
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "if(%s > 0) Marshal.Copy(%s, %s%s, 0, %s);\n",
                                seqLengthName, srcName, dstName, arrayBrackets, seqLengthName);
                        break;
                    default:
                        /* Intentionally break through to use the default handler. */
                        break;
                    }
                    if (c_primitiveKind(actualType) != P_BOOLEAN && c_primitiveKind(actualType) != P_CHAR) break;
                default:
                    nextBufTypeSize = idl_genAttrColVarName("Size", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %s; i%d++) {\n",
                            dimension, dimension, seqLengthName, dimension);
                    indent_level++;
                    if (actualTypeKind == M_COLLECTION && c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                        idl_UnionCreateArrayMemberRead(collStartType, actualType, index,
                                dimension + 1, fieldName, dstName, seqBufName, csUserData);

                    } else {
                        idl_UnionCreateSequenceMemberReadInnerLoopBody(
                                collStartType, actualType, actualTypeKind, index, dimension + 1,
                                fieldName, dstName, srcName, csUserData);
                    }
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "%s = new IntPtr(%s.ToInt64() + %s);\n",
                            srcName, srcName, nextBufTypeSize);
                    indent_level--;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
                    os_free(nextBufTypeSize);
                    break;
            }
            os_free(arrayBrackets);
            os_free(seqLengthName);
            os_free(seqBufName);
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
        break;
    }
}

static void
idl_CreateUnionAttributes(
    c_type unionType,
    const c_char *unionName,
    SACSTypeUserData *csUserData)
{
    c_ulong i, j, nrCases = c_unionUnionCaseCount((c_union) unionType);
    c_longlong nrElements;
    c_collKind seqKind;

    OS_UNUSED_ARG(unionName);

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrCases; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_unionCase unionCase = c_unionUnionCase(unionType, i);
        c_type unionCaseType = c_memberType(unionCase);
        c_char *unionCaseTypeName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);
        c_char *unionCaseName = idl_CsharpId(c_specifierName(unionCase), csUserData->customPSM, FALSE);

        while (c_baseObjectKind(unionCaseType) == M_TYPEDEF) {
            os_free(unionCaseTypeName);
            unionCaseType = c_typeDef(unionCaseType)->alias;
            unionCaseTypeName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);
        }

        seqKind = OSPL_C_UNDEFINED;
        j = 0;

        switch(c_baseObjectKind(unionCaseType))
        {
        case M_COLLECTION:
            /* Iterate to the element type of the collection. */
            for (j = 0; c_baseObjectKind(unionCaseType) == M_COLLECTION; j++) {
                /* For sequences, cache the database type for CopyIn. */
                if (c_collectionTypeKind(unionCaseType) == OSPL_C_SEQUENCE) {
                    c_char *subTypeDBName;
                    c_type subType;

                    seqKind = OSPL_C_SEQUENCE;

                    subType = c_typeActualType(c_collectionTypeSubType(unionCaseType));
                    subTypeDBName = idl_cTypeToCSharpDatabaseRepresentation(subType, TRUE);

                    nrElements = determineNrEventualElements(subType);

                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "private IntPtr attr%dCol%dType = IntPtr.Zero;\n", i, j);
                    if (c_baseObjectKind(c_typeActualType(c_collectionTypeSubType(unionCaseType))) != M_PRIMITIVE ||
                            c_primitiveKind(c_typeActualType(c_collectionTypeSubType(unionCaseType))) == P_CHAR ||
                            c_primitiveKind(c_typeActualType(c_collectionTypeSubType(unionCaseType))) == P_BOOLEAN) {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "private static readonly int attr%dCol%dSize = %"PA_PRId64" * Marshal.SizeOf(typeof(%s));\n",
                                i, j, nrElements, subTypeDBName);
                    }
                    os_free(subTypeDBName);
                    unionCaseType = c_typeActualType(c_collectionTypeSubType(unionCaseType));
                    os_free(unionCaseTypeName);
                    unionCaseTypeName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);
                } else {
                    c_char *subTypeDBName;
                    c_type subType = c_typeActualType(c_collectionTypeSubType(unionCaseType));

                    seqKind = OSPL_C_ARRAY;

                    nrElements = determineNrEventualElements(subType);
                    if (c_baseObjectKind(subType) != M_PRIMITIVE ||
                            c_primitiveKind(subType) == P_CHAR || c_primitiveKind(subType) == P_BOOLEAN ) {
                        subTypeDBName = idl_cTypeToCSharpDatabaseRepresentation(unionCaseType, TRUE);

                        if (c_baseObjectKind(subType) == M_COLLECTION && c_collectionTypeKind(subType) == OSPL_C_SEQUENCE) {
                            idl_printIndent(indent_level);
                            idl_fileOutPrintf(idl_fileCur(),
                                    "private IntPtr attr%dCol%dType = IntPtr.Zero;\n", i, j);
                        }
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(),
                                "private static readonly int attr%dCol%uSize = %"PA_PRId64" * Marshal.SizeOf(typeof(%s));\n",
                                i, j, nrElements, subTypeDBName);
                        os_free(subTypeDBName);
                    }

                    unionCaseType = c_typeActualType(c_collectionTypeSubType(unionCaseType));
                    os_free(unionCaseTypeName);
                    unionCaseTypeName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);
                }
            }
            if (c_baseObjectKind(unionCaseType) != M_STRUCTURE && c_baseObjectKind(unionCaseType) != M_UNION) {
                break;
            } else {
            /* No break statement here!!
             * This break-through is intentional, since in case the sequence
             * type is a structure, its Marshaler must also be cached.
             */
            }
        case M_STRUCTURE:
        case M_UNION:
            if (!idl_isPredefined(unionCaseTypeName)) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "private %sMarshaler attr%dMarshaler;\n", unionCaseTypeName, i);
            } else {
                /* Generate type variable for collections of predefined types */
                switch (seqKind) {
                case OSPL_C_SEQUENCE:
                    idl_printIndent(indent_level);
                     idl_fileOutPrintf(idl_fileCur(),
                             "private IntPtr attr%uSeq%uType = IntPtr.Zero;\n", i, j);
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            /* Fine: constructor doesn't need to do anything in particular here. */
            break;
        }
        os_free(unionCaseName);
        os_free(unionCaseTypeName);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n");
}



static void
idl_CreateUnionInitEmbeddedMarshalers(
    c_type unionType,
    const c_char *unionName,
    SACSTypeUserData *csUserData)
{
    c_bool isPredefined;
    c_ulong i, nrCases = c_unionUnionCaseCount((c_union) unionType);

    OS_UNUSED_ARG(unionName);

    /* Open the constructor itself and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
            "public override void InitEmbeddedMarshalers(IDomainParticipant participant)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrCases; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_unionCase unionCase = c_unionUnionCase(unionType, i);
        c_type unionCaseType = c_unionCaseType(unionCase);
        c_char *unionCaseTypeName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);
        c_char *unionCaseName = idl_CsharpId(c_specifierName(unionCase), csUserData->customPSM, FALSE);

        if (idl_isPredefined(unionCaseName)) {
            isPredefined = TRUE;
        } else {
            isPredefined = FALSE;
        }

        /* Dereference possible typedefs/arrays/sequences first. */
        while ( !isPredefined &&
                ( c_baseObjectKind(unionCaseType) == M_TYPEDEF ||
                ( c_baseObjectKind(unionCaseType) == M_COLLECTION &&
                        ( c_collectionTypeKind(unionCaseType) == OSPL_C_ARRAY ||
                          c_collectionTypeKind(unionCaseType) == OSPL_C_SEQUENCE) ) ) ) {
            os_free(unionCaseTypeName);
            if (c_baseObjectKind(unionCaseType) == M_TYPEDEF) {
                unionCaseType = c_typeDef(unionCaseType)->alias;
            } else if (c_baseObjectKind(unionCaseType) == M_COLLECTION) {
                unionCaseType = c_collectionTypeSubType(unionCaseType);
            }
            unionCaseTypeName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);
            if (idl_isPredefined(unionCaseTypeName)) {
                isPredefined = TRUE;
            }
        }

        switch(c_baseObjectKind(unionCaseType))
        {
        case M_STRUCTURE:
        case M_UNION:
            if (!idl_isPredefined(unionCaseTypeName)) {
                os_char *prevTypeName = unionCaseTypeName;
                unionCaseTypeName = idl_CsharpId(prevTypeName, csUserData->customPSM, FALSE);
                os_free(prevTypeName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (attr%dMarshaler == null) {\n",i);
                indent_level++;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "attr%dMarshaler = DatabaseMarshaler.GetMarshaler(participant, typeof(%s)) as %sMarshaler;\n",
                        i, unionCaseTypeName, unionCaseTypeName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (attr%dMarshaler == null) {\n", i);
                indent_level++;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "attr%dMarshaler = new %sMarshaler();\n",
                        i, unionCaseTypeName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                        idl_fileCur(),
                        "DatabaseMarshaler.Add(participant, typeof(%s), attr%dMarshaler);\n",
                        unionCaseTypeName, i);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "attr%dMarshaler.InitEmbeddedMarshalers(participant);\n", i);
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            break;
        default:
            /* Fine: constructor doesn't need to do anything in particular here. */
            break;
        }
        os_free(unionCaseName);
        os_free(unionCaseTypeName);
    }

    /* Decrease the indent level back to its original value and close the constructor. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}



static void
idl_CreateUnionCaseWrite(
    c_unionCase unionCase,
    c_ulong index,
    SACSTypeUserData *csUserData)
{
    c_type unionCaseType = c_unionCaseType(unionCase);
    c_char *unionCaseName;
    c_ulong maxSize, dimension;
    c_char *varName, *handleName, *ptrName;
    c_ulong nrLabels, i;
    c_char *tName;
    c_char *bufName = NULL;
    c_bool isPredefined = FALSE;
    c_char *unionCaseTypeName;
    c_type arrType;

    unionCaseName = idl_CsharpId(c_specifierName(unionCase), csUserData->customPSM, FALSE);
    unionCaseTypeName = idl_scopeStackFromCType(unionCaseType);

    if (idl_isPredefined(unionCaseTypeName)) {
        isPredefined = TRUE;
    }

    /* Dereference possible typedefs first. */
    while (c_baseObjectKind(unionCaseType) == M_TYPEDEF && !isPredefined) {
        os_free(unionCaseTypeName);
        unionCaseType = c_typeDef(unionCaseType)->alias;
        unionCaseTypeName = idl_scopeStackFromCType(unionCaseType);
        if (idl_isPredefined(unionCaseTypeName)) {
            isPredefined = TRUE;
        }
    }

    nrLabels = c_arraySize(unionCase->labels);

    if (nrLabels > 0) {
        for (i = 0; i < nrLabels; i++) {
            c_literal label = c_literal(unionCase->labels[i]);
            idl_printIndent(indent_level);
            idl_printCaseLabel(label);
        }
    } else {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "default:\n");
    }

    indent_level++;

    varName = idl_genAttrElemVarName("Var", index, 0);
    handleName = idl_genAttrElemVarName("Handle", index, 0);
    ptrName = idl_genAttrElemVarName("Buf", index, 0);

    switch(c_baseObjectKind(unionCaseType))
    {
    case M_STRUCTURE:
    case M_UNION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");

        if (isPredefined) {
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "GCHandle %s = GCHandle.Alloc(to._u, GCHandleType.Pinned);\n", handleName);
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);
            /* Generate code to use the predefined Write operation in the base marshaler. */
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "Write(%s, 0, from.%s());\n", ptrName, unionCaseName);
        } else {

            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "%s %s = from.%s();\n", unionCaseTypeName, varName, unionCaseName);
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "GCHandle %s = GCHandle.Alloc(to._u, GCHandleType.Pinned);\n", handleName);
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);
            /* Otherwise generate code to use the dedicated marshaler. */
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(),
                    "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, %s, %s);\n",
                    index, varName, ptrName);
            idl_printIndent(indent_level+1);
            idl_fileOutPrintf(idl_fileCur(), "if (result != V_COPYIN_RESULT.OK) return result;\n");
        }
        idl_printIndent(indent_level+1);
        idl_fileOutPrintf(idl_fileCur(), "%s.Free();\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        break;
    case M_ENUMERATION:
        /* Generate code to handle an enum. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    GCHandle %s = GCHandle.Alloc(to._u, GCHandleType.Pinned);\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    BaseMarshaler.Write(%s, 0, (int)from.%s());\n", ptrName, unionCaseName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    %s.Free();\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    GCHandle %s = GCHandle.Alloc(to._u, GCHandleType.Pinned);\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    BaseMarshaler.Write(%s, 0, from.%s());\n", ptrName, unionCaseName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    %s.Free();\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        break;
    case M_COLLECTION:
        /* Generate code to handle Collection types. */
        switch (c_collectionTypeKind(unionCaseType))
        {
        case OSPL_C_STRING:
            /* Generate code to handle strings. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    string %s = from.%s();\n", varName, unionCaseName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    if (%s == null) return V_COPYIN_RESULT.INVALID;\n", varName);
            /* Generate code to check for bounds if a maximum bound was specified. */
            maxSize = c_collectionTypeMaxSize(unionCaseType);
            idl_printIndent(indent_level);
            if (maxSize > 0){
                idl_fileOutPrintf(idl_fileCur(),
                        "    if (%s.Length > %d) return V_COPYIN_RESULT.INVALID;\n", varName, maxSize);
            } else {
                idl_fileOutPrintf(idl_fileCur(), "    // Unbounded string: bounds check not required...\n");
            }

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    GCHandle %s = GCHandle.Alloc(to._u, GCHandleType.Pinned);\n", handleName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "    if (!Write(c.getBase(typePtr), %s , 0, ref %s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n", ptrName, varName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    %s.Free();\n", handleName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            break;
        case OSPL_C_ARRAY:

            tName = idl_CsharpTypeFromCType(unionCaseType, csUserData->customPSM);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "{\n");

            indent_level++;

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s %s = from.%s();\n", tName, varName, unionCaseName);

            /* Check that the input is non-null. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (%s == null) return V_COPYIN_RESULT.INVALID;\n", varName);

            /* Check that the input has the correct array size. */
            arrType = unionCaseType;
            for (dimension = 0; c_baseObjectKind(arrType) == M_COLLECTION && c_collectionTypeKind(arrType) == OSPL_C_ARRAY; dimension++)
            {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (%s.GetLength(%d) != %d) return V_COPYIN_RESULT.INVALID;\n",
                        varName, dimension, c_collectionTypeMaxSize(arrType));
                arrType = c_typeActualType(c_collectionTypeSubType(arrType));
            }

            /* Generate the body of the copying loop. */
            idl_UnionCreateArrayMemberWrite(
                    unionCaseType, unionCaseType, index, 0, c_specifierName(unionCase), unionCaseName, varName, "to._u");

            indent_level--;

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");

            os_free(tName);
            break;
        case OSPL_C_SEQUENCE:

            tName = idl_CsharpTypeFromCType(unionCaseType, csUserData->customPSM);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "{\n");

            indent_level++;

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s %s = from.%s();\n", tName, varName, unionCaseName);

            bufName = idl_genAttrSeqVarName("Buf", index, 0);

            idl_UnionCreateSequenceMemberWrite(
                    unionCaseType, unionCaseType, index, 0, c_specifierName(unionCase), unionCaseName, varName, bufName);

            arrType = c_typeActualType(c_collectionTypeSubType(unionCaseType));
            if (c_baseObjectKind(arrType) != M_PRIMITIVE ||
                    c_primitiveKind(arrType) == P_BOOLEAN || c_primitiveKind(arrType)== P_CHAR) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "%s = new IntPtr(%s.ToInt64() - ((long) attr%dCol0Size * (long) attr%dSeq0Length));\n",
                        bufName, bufName, index, index);
            }

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "GCHandle %s = GCHandle.Alloc(to._u, GCHandleType.Pinned);\n", handleName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "Marshal.WriteIntPtr(%s, %s);\n", ptrName, bufName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s.Free();\n", handleName);

            indent_level--;

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");

            os_free(bufName);
            os_free(tName);
            break;

        default:
            /* Unsupported Collection type. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* Unsupported Base type. */
        assert(FALSE);
        break;
    }

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "break;\n");

    indent_level--;

    os_free(unionCaseTypeName);
    os_free(unionCaseName);
    os_free(varName);
    os_free(handleName);
    os_free(ptrName);
}

static void
idl_CreateUnionCopyIn(
    c_type unionType,
    const c_char *unionDBName,
    SACSTypeUserData *csUserData,
    SACSUnionMetaDescriptionSplDcps *umd)
{
    c_ulong i, nrCases = c_unionUnionCaseCount((c_union) unionType);
    c_type unionDiscrType = c_typeActualType(c_unionUnionSwitchType((c_union)unionType));

    /* Open the 1st CopyIn operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, System.IntPtr from, System.IntPtr to)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate code that obtains a C# object from a C pointer and then invokes the 2nd CopyIn. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandle = GCHandle.FromIntPtr(from);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s fromData = tmpGCHandle.Target as %s;\n", umd->fullyScopedName, umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return CopyIn(typePtr, fromData, to);\n");
    /* Decrease the indent level back to its original value and close the 1st CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Generate code that creates a C# projection of the database, invoke the 3rd CopyIn with it, and marshal it into unmanaged memory. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, %s from, System.IntPtr to)\n", umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = new %s();\n", unionDBName, unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "V_COPYIN_RESULT result = CopyIn(typePtr, from, ref nativeImg);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (result == V_COPYIN_RESULT.OK)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "Marshal.StructureToPtr(nativeImg, to, false);\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return result;\n");

    /* Decrease the indent level back to its original value and close the 2nd CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 2nd CopyIn operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, %s from, ref %s to)\n", umd->fullyScopedName, unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate code that checks whether the C# input is a valid object. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (from == null) return V_COPYIN_RESULT.INVALID;\n");

    idl_printIndent(indent_level);
    if ((c_baseObjectKind(unionDiscrType) == M_PRIMITIVE) &&
            (c_primitiveKind(unionDiscrType) == P_BOOLEAN)) {
        idl_fileOutPrintf(idl_fileCur(), "to._d = (from.Discriminator ? (byte)1 :(byte)0);\n");
    } else {
        char *dbType = idl_cTypeToCSharpDatabaseRepresentation(unionDiscrType, TRUE);
        idl_fileOutPrintf(idl_fileCur(), "to._d = (%s)from.Discriminator;\n", dbType);
        os_free(dbType);
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "to.Init();\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "switch (to._d)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    indent_level++;

    /* Now generate the code that loops through the attributes and generates the appropriate instructions. */
    for (i = 0; i < nrCases; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_unionCase unionCase = c_unionUnionCase(unionType, i);
        idl_CreateUnionCaseWrite(unionCase, i, csUserData);
    }

    /* create default case */
    if (umd->gUnionGenDefault) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "default:\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    break;\n");
    }

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");

    /* Generate code that returns true when everything went fine so far. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return V_COPYIN_RESULT.OK;\n");

    /* Decrease the indent level back to its original value and close the 3rd CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
idl_CreateUnionCaseRead(
    c_unionCase unionCase,
    c_ulong index,
    c_char *unionDiscrTypeName,
    SACSTypeUserData *csUserData)
{
    c_type unionCaseType = c_unionCaseType(unionCase);
    c_char *unionCaseName;
    c_ulong nrLabels, i;
    c_char *bufName = NULL;
    c_char *varName = NULL, *handleName = NULL, *ptrName = NULL;
    c_bool isPredefined = FALSE;
    c_char *unionCaseTypeName;
    c_char *tName;
    c_char *discr = os_strdup("");
    c_bool isArray;

    unionCaseName = idl_CsharpId(c_specifierName(unionCase), csUserData->customPSM, FALSE);
    unionCaseTypeName = idl_scopeStackFromCType(unionCaseType);

    if (idl_isPredefined(unionCaseTypeName)) {
        isPredefined = TRUE;
    }

    /* Dereference possible typedefs first. */
    while (c_baseObjectKind(unionCaseType) == M_TYPEDEF && !isPredefined) {
        os_free(unionCaseTypeName);
        unionCaseType = c_typeDef(unionCaseType)->alias;
        unionCaseTypeName = idl_scopeStackFromCType(unionCaseType);
        if (idl_isPredefined(unionCaseTypeName)) {
            isPredefined = TRUE;
        }
    }

    nrLabels = c_arraySize(unionCase->labels);

    if (nrLabels > 0) {
        for (i = 0; i < nrLabels; i++) {
            c_literal label = c_literal(unionCase->labels[i]);
            idl_printIndent(indent_level);
            idl_printCaseLabel(label);
        }
        if (nrLabels > 1) {
            os_size_t l = strlen(unionDiscrTypeName) + strlen("()from._d, ") + 1;
            os_free(discr);
            discr = os_malloc(l);
            snprintf(discr, l, "(%s)from._d, ", unionDiscrTypeName);
        }
    } else {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "default:\n");
    }

    isArray = (c_baseObjectKind(unionCaseType) == M_COLLECTION) && (c_collectionTypeKind(unionCaseType) == OSPL_C_ARRAY);

    varName = idl_genAttrElemVarName("Var", index, 0);
    handleName = idl_genAttrElemVarName("Handle", index, 0);
    ptrName = idl_genAttrSeqVarName("Buf", index, 0);

    indent_level++;

    if (!isArray) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    GCHandle %s = GCHandle.Alloc(from._u, GCHandleType.Pinned);\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    IntPtr %s = %s.AddrOfPinnedObject();\n", ptrName, handleName);
    }

    switch(c_baseObjectKind(unionCaseType))
    {
    case M_STRUCTURE:
    case M_UNION:
        if (isPredefined) {
            /* Predefined types can use specialized Read operations. */
            const c_char *predefinedTypeName = idl_translateIfPredefined(unionCaseTypeName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(Read%s(%s, 0));\n", unionCaseName, &predefinedTypeName[4], ptrName);
        } else {
            /* Other types should use their corresponding Marshaler. */
            /* Get the CSharp specific name of the member type. */
            c_char *unionCaseTypeDBName = idl_CsharpScopeStackFromCType(unionCaseType, csUserData->customPSM, TRUE, TRUE);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    %s %s = new %s();\n",
                    unionCaseTypeName, varName, unionCaseTypeName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    %sMarshaler.StaticCopyOut(%s, ref %s);\n",
                    unionCaseTypeDBName, ptrName, varName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(%s%s);\n", unionCaseName, discr, varName);
            os_free(unionCaseTypeDBName);
        }
        break;
    case M_ENUMERATION:
        /* Enums are read as Int32 types. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    to.%s((%s)BaseMarshaler.ReadInt32(%s, 0));\n", unionCaseName, unionCaseTypeName, ptrName);
        break;
    case M_PRIMITIVE:
        /* Handle primitive. */
        idl_printIndent(indent_level);
        switch (c_primitiveKind(unionCaseType))
        {
        case P_BOOLEAN:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadBoolean(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_CHAR:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadChar(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_OCTET:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadByte(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_SHORT:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadInt16(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_USHORT:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadUInt16(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_LONG:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadInt32(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_ULONG:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadUInt32(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_LONGLONG:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadInt64(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_ULONGLONG:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadUInt64(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_FLOAT:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadFloat(%s, 0));\n", unionCaseName, ptrName);
            break;
        case P_DOUBLE:
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(BaseMarshaler.ReadDouble(%s,0));\n", unionCaseName, ptrName);
            break;
        default:
            /* Unsupported primitive type. */
            assert(FALSE);
            break;
        }
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(unionCaseType))
        {
        case OSPL_C_STRING:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    to.%s(ReadString(Marshal.ReadIntPtr(%s)));\n", unionCaseName, ptrName);
            break;
        case OSPL_C_ARRAY:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "{\n");

            indent_level++;

            tName = idl_CsharpTypeFromCType(unionCaseType, csUserData->customPSM);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s %s = null;\n", tName, varName);
            nrLabels = c_arraySize(unionCase->labels);
            if (nrLabels > 0) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (");
                for (i = 0; i < nrLabels; i++) {
                    c_literal label = c_literal(unionCase->labels[i]);
                    char *value = idl_getCSharpLabelValue(label);
                    if (i == 0) {
                        idl_fileOutPrintf(idl_fileCur(), "to.Discriminator == (%s)%s", unionDiscrTypeName, value);
                    } else {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(), "\n    || to.Discriminator == (%s)%s", unionDiscrTypeName, value);
                    }
                    os_free(value);
                }
                idl_fileOutPrintf(idl_fileCur(),") {\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "    %s = to.%s();\n", varName, unionCaseName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),"}\n");
            }

            os_free(tName);

            idl_UnionCreateArrayMemberRead(
                    unionCaseType, unionCaseType, index, 0, unionCaseName, varName, "from._u", csUserData);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s(%s);\n", unionCaseName, varName);

            indent_level--;

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            break;
        case OSPL_C_SEQUENCE:
            indent_level++;

            tName = idl_CsharpTypeFromCType(unionCaseType, csUserData->customPSM);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s %s = null;\n", tName, varName);
            nrLabels = c_arraySize(unionCase->labels);
            if (nrLabels > 0) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (");
                for (i = 0; i < nrLabels; i++) {
                    c_literal label = c_literal(unionCase->labels[i]);
                    char *value = idl_getCSharpLabelValue(label);
                    if (i == 0) {
                        idl_fileOutPrintf(idl_fileCur(), "to.Discriminator == (%s)%s", unionDiscrTypeName, value);
                    } else {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(), "\n    || to.Discriminator == (%s)%s", unionDiscrTypeName, value);
                    }
                    os_free(value);
                }
                idl_fileOutPrintf(idl_fileCur(),") {\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "    %s = to.%s();\n", varName, unionCaseName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),"}\n");
            }
            os_free(tName);

            bufName = idl_genAttrSeqVarName("Buf", index, 0);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s = Marshal.ReadIntPtr(%s);\n", bufName, bufName);

            idl_UnionCreateArrayMemberRead(
                    unionCaseType, unionCaseType, index, 0, unionCaseName, varName, bufName, csUserData);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s(%s);\n", unionCaseName, varName);

            indent_level--;
            os_free(bufName);
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
        }
        break;
    default:
        /* Unsupported Base type. */
        assert(FALSE);
    }

    if (!isArray) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    %s.Free();\n", handleName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
    }

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "break;\n");

    indent_level--;

    os_free(unionCaseTypeName);
    os_free(unionCaseName);
    os_free(varName);
    os_free(handleName);
    os_free(ptrName);
    os_free(discr);
}

static void
idl_CreateUnionCopyOut(
    c_type unionType,
    const c_char *unionDBName,
    SACSTypeUserData *csUserData,
    SACSUnionMetaDescriptionSplDcps *umd)
{
    c_ulong i, nrCases = c_unionUnionCaseCount((c_union) unionType);
    c_type unionDiscrType = c_typeActualType(c_unionUnionSwitchType((c_union)unionType));
    idl_typeSpec unionDiscrSpec = idl_makeTypeSpec(unionDiscrType);
    c_char *unionDiscrTypeName = idl_CsharpTypeFromTypeSpec(unionDiscrSpec, csUserData->customPSM);
    idl_freeTypeSpec(unionDiscrSpec);

    /* Open the 1st CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, System.IntPtr to)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = (%s) Marshal.PtrToStructure(from, typeof(%s));\n",
            unionDBName, unionDBName, unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandleTo = GCHandle.FromIntPtr(to);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s toObj = tmpGCHandleTo.Target as %s;\n", umd->fullyScopedName, umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref toObj);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "tmpGCHandleTo.Target = toObj;\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 2nd CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, ref %s to)\n", umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = (%s) Marshal.PtrToStructure(from, typeof(%s));\n",
            unionDBName, unionDBName, unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref to);\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 3rd CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public static void StaticCopyOut(System.IntPtr from, ref %s to)\n", umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s nativeImg = (%s) Marshal.PtrToStructure(from, typeof(%s));\n",
            unionDBName, unionDBName, unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref to);\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 4th CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public static void CopyOut(ref %s from, ref %s to)\n", unionDBName, umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Cast the object to its proper type and if no yet allocated, allocate it. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (to == null) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "to = new %s();\n", umd->fullyScopedName);
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "switch (from._d)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    indent_level++;


    /* Now generate the code that loops through the attributes and generates the appropriate instructions. */
    for (i = 0; i < nrCases; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_unionCase unionCase = c_unionUnionCase(unionType, i);
        idl_CreateUnionCaseRead(unionCase, i, unionDiscrTypeName, csUserData);
    }

    /* create default case */
    if (umd->gUnionGenDefault) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "default:\n");
        idl_printIndent(indent_level);
        if ((c_baseObjectKind(unionDiscrType) == M_PRIMITIVE) && (c_primitiveKind(unionDiscrType) == P_BOOLEAN)) {
            idl_fileOutPrintf(idl_fileCur(), "    to.Default(((from._d != 0) ? true : false));\n");
        } else {
            idl_fileOutPrintf(idl_fileCur(), "    to.Default((%s)from._d);\n", unionDiscrTypeName);
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    break;\n");
    }

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    os_free(unionDiscrTypeName);
}

static void
idl_generateUnionMarshaler (
    c_type unionType,
    const c_char *unionDBName,
    SACSTypeUserData *csUserData,
    SACSUnionMetaDescriptionSplDcps *umd)
{
    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %sMarshaler\n", unionDBName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public sealed class %sMarshaler : DDS.OpenSplice.CustomMarshalers.FooDatabaseMarshaler<%s>\n",
        unionDBName, umd->fullyScopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Create the constructor of the Marshaler. */
    idl_determineFullyScopedName(unionType);
    idl_CreateUnionAttributes(unionType, unionDBName, csUserData);
    idl_CreateUnionInitEmbeddedMarshalers(unionType, unionDBName, csUserData);
    idl_CreateUnionCopyIn(unionType, unionDBName, csUserData, umd);
    idl_CreateUnionCopyOut(unionType, unionDBName, csUserData, umd);

    /* Decrease the indentation level and generate the closing bracket. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
}
