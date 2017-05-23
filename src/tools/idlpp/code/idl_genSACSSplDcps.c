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

static void
idl_generateDatabaseRepresentation (
    idl_typeStruct typeSpec,
    const c_char *structName,
    SACSSplDcpsUserData *csUserData);

static void
idl_generateMarshaler (
    idl_typeStruct typeSpec,
    const c_char *structName,
    SACSSplDcpsUserData *csUserData);

/* @brief callback function called on opening the IDL input file.
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

    /* Generate inclusion of standard OpenSplice DDS type definition files */
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
    SACSSplDcpsUserData *csUserData = (SACSSplDcpsUserData *) userData;
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
    SACSSplDcpsUserData *csUserData = (SACSSplDcpsUserData *) userData;

    OS_UNUSED_ARG(scope);

    scopedStructName = idl_scopeStackCsharp(
            idl_typeUserScope(idl_typeUser(structSpec)),
            ".",
            idl_typeSpecName(idl_typeSpec(structSpec)));
    if (idl_isPredefined(scopedStructName)) {
        /* return idl_abort to indicate that the structure does not need to be processed. */
        action = idl_abort;
    } else {
        /* Translate the name of the struct into a valid C# identifier. */
        c_char *structName = idl_CsharpId(name, csUserData->customPSM, FALSE);

        /* Generate the Database representation of the datatype. */
        idl_generateDatabaseRepresentation(structSpec, structName, csUserData);

        /* Generate the Marshaler for the datatype. */
        idl_generateMarshaler(structSpec, structName, csUserData);

        /* return idl_explore to indicate that the rest of the structure needs to be processed. */
        action = idl_explore;
        os_free(structName);
    }
    os_free(scopedStructName);
    return action;
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
        SACSSplDcpsUserData *userData)
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
    idl_genSACSSplDcps.unionOpen                        = NULL;
    idl_genSACSSplDcps.unionClose                       = NULL;
    idl_genSACSSplDcps.unionCaseOpenClose               = NULL;
    idl_genSACSSplDcps.unionLabelsOpenClose             = NULL;
    idl_genSACSSplDcps.unionLabelOpenClose              = NULL;
    idl_genSACSSplDcps.typedefOpenClose                 = NULL;
    idl_genSACSSplDcps.boundedStringOpenClose           = NULL;
    idl_genSACSSplDcps.sequenceOpenClose                = NULL;
    idl_genSACSSplDcps.constantOpenClose                = NULL;
    idl_genSACSSplDcps.artificialDefaultLabelOpenClose  = NULL;
    idl_genSACSSplDcps.userData                         = userData;

    return &idl_genSACSSplDcps;
}

static c_char *
idl_cTypeToCSharpDatabaseRepresentation(
        c_type memberType,
        const c_char *memberTypeName,
        SACSSplDcpsUserData *csUserData)
{
    c_char *dbType = NULL, *memberTypeNameTmp;

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
        if (idl_isPredefined(memberTypeName)) {
            dbType = os_strdup(idl_translateIfPredefined(memberTypeName));
        } else {
            dbType = idl_CsharpId(memberTypeName, csUserData->customPSM, TRUE);
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
            memberTypeNameTmp = idl_scopeStackFromCType(memberType);
            dbType = idl_cTypeToCSharpDatabaseRepresentation(memberType, memberTypeNameTmp, csUserData);
            os_free(memberTypeNameTmp);
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

#if 0
static const c_char *
idl_cTypeToBaseType(
        c_type memberType,
        const char *memberTypeName)
{
    const c_char *baseType = NULL;

    switch (c_baseObjectKind(memberType))
    {
    case M_PRIMITIVE:
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
            baseType = os_strdup("Boolean");
            break;
        case P_CHAR:
            baseType = os_strdup("Char");
            break;
        case P_OCTET:
            baseType = os_strdup("Byte");
            break;
        case P_SHORT:
            baseType = os_strdup("Int16");
            break;
        case P_USHORT:
            baseType = os_strdup("UInt16");
            break;
        case P_LONG:
            baseType = os_strdup("Int32");
            break;
        case P_ULONG:
            baseType = os_strdup("UInt32");
            break;
        case P_LONGLONG:
            baseType = os_strdup("Int64");
            break;
        case P_ULONGLONG:
            baseType = os_strdup("UInt64");
            break;
        case P_FLOAT:
            baseType = os_strdup("Single");
            break;
        case P_DOUBLE:
            baseType = os_strdup("Double");
            break;
        default:
            /* Unsupported structure member type */
            assert(FALSE);
        }
        break;
    case M_STRUCTURE:
        if (strcmp(memberTypeName, "DDS.Duration_t") == 0) {
            baseType = "Duration";
        } else if (strcmp(memberTypeName, "DDS.Time_t") == 0) {
            baseType = "Time";
        } else if (strcmp(memberTypeName, "DDS.InstanceHandle_t") == 0) {
            baseType = "InstanceHandle";
        } else {
            /* This part of the code should never be reached for non-predefined types. */
            assert(FALSE);
        }
        break;
    case M_COLLECTION:
        switch(c_collectionTypeKind(memberType))
        {
        case C_STRING:
            baseType = "String";
            break;
        default:
            /* Other collection types should never reach this part of the code. */
            assert(FALSE);
            break;
        }
        break;
    default:
        /* This part of the code should never be reached for other types. */
        assert(FALSE);
        break;
    }

    return baseType;
}
#endif

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


static void
idl_generateDatabaseRepresentation (
    idl_typeStruct typeSpec,
    const c_char *structName,
    SACSSplDcpsUserData *csUserData)
{
    /* Get the meta-data of this datatype from the database. */
    c_type structType = idl_typeSpecDef(idl_typeSpec(typeSpec));
    c_ulong i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region __%s\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Sequential)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public struct __%s\n",
        structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrMembers; i++) {
        c_char *dbType;
        c_char *memberTypeName;

        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_type memberType = c_typeActualType(c_memberType(structMember));
        c_char *memberName = idl_CsharpId(
                c_specifierName(structMember),
                csUserData->customPSM,
                FALSE);

        /* Now find and generate the corresponding database representation. */
        memberTypeName = idl_scopeStackFromCType(memberType);
        dbType = idl_cTypeToCSharpDatabaseRepresentation(memberType, memberTypeName, csUserData);

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
        os_free(memberTypeName);
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

static void
idl_CreateAttributes(
        c_type structType,
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
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
        c_char *memberTypeName = idl_scopeStackFromCType(memberType);
        c_char *memberName = idl_CsharpId(
                        c_specifierName(structMember),
                        csUserData->customPSM,
                        FALSE);
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
        case M_COLLECTION:
            /* Iterate to the element type of the collection. */
            nrElements = 1;
            for (j = 0; c_baseObjectKind(memberType) == M_COLLECTION; j++) {
                /* For sequences, cache the database type for CopyIn. */
                if (c_collectionTypeKind(memberType) == OSPL_C_SEQUENCE) {
                    c_char *subTypeName;
                    c_char *subTypeCSName;

                    c_type subType = c_typeActualType(c_collectionTypeSubType(memberType));
                    while (c_baseObjectKind(subType) == M_COLLECTION && c_collectionTypeKind(subType) == OSPL_C_ARRAY) {
                        nrElements *= (c_longlong) c_collectionTypeMaxSize(subType);
                        subType = c_typeActualType(c_collectionTypeSubType(subType));
                    }
                    subTypeName = idl_scopeStackFromCType(subType);
                    subTypeCSName = idl_cTypeToCSharpDatabaseRepresentation(subType, subTypeName, csUserData);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "private IntPtr attr%dSeq%dType = IntPtr.Zero;\n", i, j);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "private static readonly int attr%dSeq%dSize = %"PA_PRId64" * Marshal.SizeOf(typeof(%s));\n", i, j, nrElements, subTypeCSName);
                    os_free(subTypeCSName);
                    os_free(subTypeName);
                }
                memberType = c_typeActualType(c_collectionTypeSubType(memberType));
                os_free(memberTypeName);
                memberTypeName = idl_scopeStackFromCType(memberType);
            }
            if (c_baseObjectKind(memberType) != M_STRUCTURE) {
                break;
            } else {
            /* No break statement here!!
             * This break-through is intentional, since in case the sequence
             * type is a structure, its Marshaler must also be cached.
             */
            }
        case M_STRUCTURE:
            if (!idl_isPredefined(memberTypeName)) {

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
        os_free(memberName);
        os_free(memberTypeName);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n");
}

static void
idl_CreateInitEmbeddedMarshalers(
        c_type structType,
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
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
        c_char *memberTypeName = idl_scopeStackFromCType(memberType);
        c_char *memberName = idl_CsharpId(
                        c_specifierName(structMember),
                        csUserData->customPSM,
                        FALSE);

        if (idl_isPredefined(memberTypeName)) {
            isPredefined = TRUE;
        } else {
            isPredefined = FALSE;
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
            memberTypeName = idl_scopeStackFromCType(memberType);
            if (idl_isPredefined(memberTypeName)) {
                isPredefined = TRUE;
            }
        }

        switch(c_baseObjectKind(memberType))
        {
        case M_STRUCTURE:
            if (!idl_isPredefined(memberTypeName)) {
                os_char *prevTypeName = memberTypeName;
                memberTypeName = idl_CsharpId(prevTypeName, csUserData->customPSM, FALSE);
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
        os_free(memberName);
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
}

static c_char *
idl_CreateDatabaseArrayIterationIndex(
        c_type collStartType,
        c_ulong dimension)
{
    c_ulong i;
    c_type nextType;
    c_type currentType = c_typeActualType(collStartType);

    /* maxResultLength = '\0' + '[' + ']' + (dimension -1 ) * (2 * nrDecimals(MaxInt) + 'i' + '*' + ' + ') + 'i' + nrDecimals(MaxInt) */
    c_ulong maxResultLength = 1 + 2 + (dimension - 1) * (2 * 10 + 1 + 1 + 3) + 1 + 10;
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
    os_strncat(result, "[", maxResultLength);
    maxResultLength -= 1;
    for (i = 0; i < (dimension - 1); i++) {
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
                format = "i%d*%d + ";
                (void)snprintf (postfix, IDL_INDEX_BUFFER_LENGTH, format, i, c_collectionTypeMaxSize(currentType));
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

    format = "i%d]";
    (void)snprintf (postfix, IDL_INDEX_BUFFER_LENGTH, format, i);
    os_strncat(result, postfix, maxResultLength);

    return result;

#undef IDL_INDEX_BUFFER_LENGTH
}

static void
idl_CreateArrayMemberWriteInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName)
{
    c_ulong maxSize;
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *databaseArrayBrackets = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension);
    c_char *collTypeName;

    switch(collKind)
    {
    case M_STRUCTURE:
        collTypeName = idl_scopeStackFromCType(collType);
        idl_printIndent(indent_level);
        if (idl_isPredefined((collTypeName))) {
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = from.%s%s;\n", fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                    "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, from.%s%s, ref to.%s%s);\n",
                    index, fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
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
                "to.%s%s = (uint) from.%s%s;\n",
                fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
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
        collTypeName = idl_scopeStackFromCType(collType);
        idl_printIndent(indent_level);
        if (idl_isPredefined((collTypeName))) {
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(%s, 0, from.%s%s);\n", bufName, fieldName, cSharpArrayBrackets);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_printIndent(indent_level + 1);
            idl_fileOutPrintf(idl_fileCur(),
                               "V_COPYIN_RESULT result = attr%dMarshaler.CopyIn(typePtr, from.%s%s, %s);\n",
                               index, fieldName, cSharpArrayBrackets, bufName);
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
        const c_char *fieldName,
        const c_char *bufName)
{
    c_ulong arrLength, seqMaxLength, nextBufTypeSizeLen;
    c_ulong seqLengthNameSize, seqTypeNameSize;
    c_char *seqLengthName, *seqTypeName, *nextBufTypeSize, *iterationIndex;
    c_type subType, actualType;
    c_metaKind actualTypeKind;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_CreateArrayMemberWriteInnerLoopBody(
                collStartType, collType, collKind, index, dimension, fieldName);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case OSPL_C_STRING:
            /* Handle strings. */
            idl_CreateArrayMemberWriteInnerLoopBody(
                    collStartType, collType, collKind, index, dimension, fieldName);
            break;
        case OSPL_C_ARRAY:
            /* Handle arrays. */
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);

            /* Open a for loop to walk over the current dimension and increase the indent. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                    dimension, dimension, arrLength, dimension);
            indent_level++;

            /* Generate the the body to process the next dimension. */
            idl_CreateArrayMemberWrite(collStartType, actualType, index,
                    dimension + 1, fieldName, bufName);

            /* Close the for loop and decrease the indent level back to its original. */
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            break;
        case OSPL_C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            actualTypeKind = c_baseObjectKind(actualType);

            seqLengthNameSize = 13 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqLengthName = os_malloc(seqLengthNameSize);
            snprintf(seqLengthName, seqLengthNameSize, "attr%uSeq%uLength", index, dimension);

            seqTypeNameSize = 11 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqTypeName = os_malloc(seqTypeNameSize);
            snprintf(seqTypeName, seqTypeNameSize, "attr%uSeq%uType", index, dimension);

            seqMaxLength = c_collectionTypeMaxSize(collType);
            iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            /* Check whether the sequence is valid. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s%s == null) return V_COPYIN_RESULT.INVALID;\n",
                    fieldName, iterationIndex);

            /* Get its Length, and check it for validity. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "int %s = from.%s%s.Length;\n",
                    seqLengthName, fieldName, iterationIndex);
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
                        fieldName);
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");
            } else {
                idl_fileOutPrintf(idl_fileCur(),
                        "DDS.OpenSplice.Database.c_collectionType colType = (DDS.OpenSplice.Database.c_collectionType) Marshal.PtrToStructure(\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "        attr%dSeq%dType, typeof(DDS.OpenSplice.Database.c_collectionType));\n", index, dimension -1);
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
                    bufName, seqTypeName, seqLengthName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (%s == IntPtr.Zero) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n", bufName);

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
                                signedCounterPart, fieldName, iterationIndex, bufName, seqLengthName);
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
                                "Marshal.Copy(from.%s%s, 0, %s, %s);\n", fieldName, iterationIndex, bufName, seqLengthName);
                        break;
                    default:
                        /* Intentionally fall through to the default handler. */
                        break;
                    }
                    if (c_primitiveKind(actualType) != P_BOOLEAN && c_primitiveKind(actualType) != P_CHAR) break;
                default:
                    nextBufTypeSizeLen = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
                    nextBufTypeSize = os_malloc(nextBufTypeSizeLen);
                    snprintf(nextBufTypeSize, nextBufTypeSizeLen, "attr%uSeq%uSize", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "for (int i%u = 0; i%u < %s; i%u++) {\n",
                            dimension, dimension, seqLengthName, dimension);
                    indent_level++;
                    if (actualTypeKind == M_COLLECTION && c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                        subType = c_typeActualType(c_collectionTypeSubType(actualType));
                        if (c_collectionTypeKind(actualType) == OSPL_C_ARRAY && c_baseObjectKind(subType) == M_PRIMITIVE) {
                            os_free(iterationIndex);
                            iterationIndex = idl_CreateCSharpArrayIterationIndex(collStartType, dimension + 1);
                            idl_printIndent(indent_level);
                            idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(from.%s%s, 0, %s, %d);\n",
                                    fieldName, iterationIndex, bufName, c_collectionTypeMaxSize(actualType));
                        } else {
                            c_ulong seqBufNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
                            c_char *seqBufName = os_malloc(seqBufNameSize);
                            snprintf(seqBufName, seqBufNameSize, "attr%uSeq%uBuf", index, dimension + 1);
                            idl_CreateArrayMemberWrite(collStartType, actualType, index,
                                    dimension + 1, fieldName, seqBufName);
                            if (c_collectionTypeKind(actualType) == OSPL_C_SEQUENCE) {
                                if (c_baseObjectKind(subType) == M_PRIMITIVE) {
                                    idl_printIndent(indent_level);
                                    idl_fileOutPrintf(idl_fileCur(),
                                            "Marshal.WriteIntPtr(%s, %s);\n", bufName, seqBufName);
                                } else {
                                    idl_printIndent(indent_level);
                                    idl_fileOutPrintf(idl_fileCur(),
                                            "Marshal.WriteIntPtr(%s, new IntPtr(%s.ToInt64() - ((long) attr%dSeq%dSize * (long) attr%dSeq%dLength)));\n",
                                            bufName, seqBufName, index, dimension + 1, index, dimension + 1);
                                }
                            }
                            os_free(seqBufName);
                        }
                    } else {
                        idl_CreateSequenceMemberWriteInnerLoopBody(
                                collStartType, actualType, actualTypeKind, index, dimension + 1,
                                fieldName, bufName);
                    }
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "%s = new IntPtr(%s.ToInt64() + %s);\n", bufName, bufName, nextBufTypeSize);
                    indent_level--;
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
                    os_free(nextBufTypeSize);
                    break;
            }

            os_free(iterationIndex);
            os_free(seqTypeName);
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
        const c_char * memberName,
        c_ulong index,
        SACSSplDcpsUserData *csUserData)
{
    c_ulong bufNameLength, maxSize, dimension;
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
        if (isPredefined)
        {
            /* Generate code to use the predefined Write operation in the base marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = from.%s;\n", memberName, memberName);
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
                               memberName,
                               memberName);
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
                memberName,
                memberName);
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s = from.%s ? (byte) 1 : (byte) 0;\n",
                    memberName,
                    memberName);
            break;
        case P_CHAR:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = (byte) from.%s;\n", memberName, memberName);
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
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s == null) return V_COPYIN_RESULT.INVALID;\n", memberName);
            /* Generate code to check for bounds if a maximum bound was specified. */
            maxSize = c_collectionTypeMaxSize(memberType);
            idl_printIndent(indent_level);
            if (maxSize > 0)
            {
                idl_fileOutPrintf(idl_fileCur(),
                        "if (from.%s.Length > %d) return V_COPYIN_RESULT.INVALID;\n",
                        memberName,
                        maxSize);
            }
            else
            {
                idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (!Write(c.getBase(typePtr), ref to.%s, from.%s)) return V_COPYIN_RESULT.OUT_OF_MEMORY;\n",
                    memberName,
                    memberName);
            break;
        case OSPL_C_ARRAY:
            /* Check that the input is non-null. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s == null) return V_COPYIN_RESULT.INVALID;\n", memberName);

            /* Check that the input has the correct array size. */
            arrType = memberType;
            for (dimension = 0; c_baseObjectKind(arrType) == M_COLLECTION && c_collectionTypeKind(arrType) == OSPL_C_ARRAY; dimension++)
            {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (from.%s.GetLength(%d) != %d) return V_COPYIN_RESULT.INVALID;\n",
                        memberName, dimension, c_collectionTypeMaxSize(arrType));
                arrType = c_typeActualType(c_collectionTypeSubType(arrType));
            }

            /* Initialize the database sample to the correct Array size. */
            totalNrElements = idl_determineDatabaseFlatArraySize(memberType);
            bufName = idl_cTypeToCSharpDatabaseRepresentation(memberType, memberTypeName, csUserData);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "to.%s = new %s[%"PA_PRIu64"];\n", memberName, bufName, totalNrElements);

            /* Generate the body of the copying loop. */
            idl_CreateArrayMemberWrite(
                    memberType, memberType, index, 0, memberName, memberName);
            os_free(bufName);
            break;
        case OSPL_C_SEQUENCE:
            bufNameLength = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            bufName = os_malloc(bufNameLength);
            snprintf(bufName, bufNameLength, "attr%dSeq%dBuf", index, 0);

            idl_CreateArrayMemberWrite(
                    memberType, memberType, index, 0, memberName, bufName);

            if (c_baseObjectKind(c_typeActualType(c_collectionTypeSubType(memberType))) != M_PRIMITIVE) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "to.%s = new IntPtr(%s.ToInt64() - ((long) attr%dSeq0Size * (long) attr%dSeq0Length));\n",
                        memberName, bufName, index, index);
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
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
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
    idl_fileOutPrintf(idl_fileCur(), "%s fromData = tmpGCHandle.Target as %s;\n", structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return CopyIn(typePtr, fromData, to);\n");
    /* Decrease the indent level back to its original value and close the 1st CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Generate code that creates a C# projection of the database, invoke the 3rd CopyIn with it, and marshal it into unmanaged memory. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, %s from, System.IntPtr to)\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "__%s nativeImg = new __%s();\n", structName, structName);
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
    idl_fileOutPrintf(idl_fileCur(), "public V_COPYIN_RESULT CopyIn(System.IntPtr typePtr, %s from, ref __%s to)\n", structName, structName);
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
        idl_CreateStructMemberWrite(memberType, memberName, i, csUserData);
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
        SACSSplDcpsUserData *csUserData)
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
idl_CreateArrayMemberReadInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_ulong index,
        c_ulong dimension,
        const c_char *fieldName,
        const c_char *bufName,
        SACSSplDcpsUserData *csUserData)
{
    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *databaseArrayBrackets = idl_CreateDatabaseArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_scopeStackFromCType(collType);

    OS_UNUSED_ARG(index);
    OS_UNUSED_ARG(bufName);

    switch(collKind)
    {
    case M_STRUCTURE:
        if (idl_isPredefined(colTypeName)) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = from.%s%s;\n",
                    fieldName, cSharpArrayBrackets, fieldName, databaseArrayBrackets);
        } else {
            /* Get the CSharp specific name of the member type. */
            c_char *prevColTypeName = colTypeName;
            colTypeName = idl_CsharpId(prevColTypeName, csUserData->customPSM, FALSE);
            os_free(prevColTypeName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%sMarshaler.CopyOut(ref from.%s%s, ref to.%s%s);\n",
                    colTypeName, fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s%s = (%s) from.%s%s;\n",
                fieldName, cSharpArrayBrackets, colTypeName, fieldName, databaseArrayBrackets);
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        idl_printIndent(indent_level);
        switch (c_primitiveKind(collType))
        {
        case P_BOOLEAN:
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = (from.%s%s != 0 ? true : false);\n",
                    fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
            break;
        case P_CHAR:
            idl_fileOutPrintf(idl_fileCur(), "(to.%s%s) = (char) (from.%s%s);\n",
                    fieldName, databaseArrayBrackets, fieldName, cSharpArrayBrackets);
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
        SACSSplDcpsUserData *csUserData)
{

    c_char *cSharpArrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_scopeStackFromCType(collType);

    OS_UNUSED_ARG(index);

    switch(collKind)
    {
    case M_STRUCTURE:
        idl_printIndent(indent_level);
        if (idl_isPredefined(colTypeName)) {
            /* Function below returns const string, so no need to free it afterward. */
            const c_char *predefinedTypeName = idl_translateIfPredefined(colTypeName);
            /* Now remove the preceeding 'DDS.' from the typename, so start reading at character position 4. */
            idl_fileOutPrintf(idl_fileCur(),
                    "to.%s%s = Read%s(%s, 0);\n",
                    fieldName, cSharpArrayBrackets, &predefinedTypeName[4], bufName);
        } else {
            /* Get the CSharp specific name of the member type. */
            c_char *prevColTypeName = colTypeName;
            colTypeName = idl_CsharpId(prevColTypeName, csUserData->customPSM, FALSE);
            os_free(prevColTypeName);
            idl_fileOutPrintf(idl_fileCur(),
                    "%sMarshaler.StaticCopyOut(%s, ref to.%s%s);\n",
                    colTypeName, bufName, fieldName, cSharpArrayBrackets);
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "to.%s%s = (%s) ReadInt32(%s);\n",
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
        SACSSplDcpsUserData *csUserData)
{
    c_ulong i, arrLength;
    os_size_t dimensionCheckSize;
    c_ulong seqLengthNameSize, nextBufTypeSizeLen;
    c_char *seqLengthName, *nextBufTypeSize, *arrayBrackets;
    c_char *dimensionCheck;
    c_type subType, actualType, parentType;
    c_metaKind actualTypeKind;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_CreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                index, dimension, fieldName, bufName, csUserData);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case OSPL_C_STRING:
            /* Handle strings. */
            idl_CreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                    index, dimension, fieldName, bufName, csUserData);
            break;
        case OSPL_C_ARRAY:
            /* Handle arrays. */
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension);

            /* Check if this dimension is part of a prior rectangular array by checking
             * whether its parent is also an array.
             */
            parentType = collStartType;
            for (i = 0; i < dimension; i++)
            {
                parentType = c_typeActualType(c_collectionTypeSubType(parentType));
            }
            if (parentType == collStartType ||
                    (c_baseObjectKind(parentType) != M_COLLECTION &&
                     c_collectionTypeKind(parentType) != OSPL_C_ARRAY))
            {
                /* If no prior rectangular dimensions, then check the current dimensions. */
                c_ulong curDim;
                subType = c_typeActualType(collType);
                dimensionCheckSize = 15 + strlen(fieldName) + strlen (arrayBrackets) + 1;
                dimensionCheck = os_malloc(dimensionCheckSize);
                snprintf(dimensionCheck, dimensionCheckSize,
                        "to.%s%s == null", fieldName, arrayBrackets);
                for (i = 0; c_baseObjectKind(subType) == M_COLLECTION &&
                            c_collectionTypeKind(subType) == OSPL_C_ARRAY; i++)
                {
                    c_char *prevdimensionCheck = dimensionCheck;
                    curDim = c_collectionTypeMaxSize(subType);
                    dimensionCheckSize = strlen(prevdimensionCheck) + 26 + strlen(fieldName) +
                            strlen(arrayBrackets) + 2 * 10 /* MAX_INT */ + 1;
                    dimensionCheck = os_malloc(dimensionCheckSize);
                    snprintf(dimensionCheck, dimensionCheckSize,
                            "%s || to.%s%s.GetLength(%d) != %d",
                            prevdimensionCheck, fieldName, arrayBrackets, i, curDim);
                    os_free(prevdimensionCheck);
                    subType = c_typeActualType(c_collectionTypeSubType(subType));
                }
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "if (%s) {\n", dimensionCheck);
                indent_level++;
                idl_CreateArrayInitialization(collType, fieldName, "", arrayBrackets, csUserData);
                indent_level--;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            else
            {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(),
                        "// Not the start of a rectangular array: initialization not required...\n");
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %d; i%d++) {\n",
                    dimension, dimension, arrLength, dimension);
            indent_level++;
            idl_CreateArrayMemberRead(collStartType, actualType, index,
                    dimension + 1, fieldName, bufName, csUserData);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            os_free(arrayBrackets);
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
                    snprintf(nextBufTypeSize, nextBufTypeSizeLen, "attr%uSeq%uSize", index, dimension);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %s; i%d++) {\n",
                            dimension, dimension, seqLengthName, dimension);
                    indent_level++;
                    if (actualTypeKind == M_COLLECTION && c_collectionTypeKind(actualType) != OSPL_C_STRING) {
                        subType = c_typeActualType(c_collectionTypeSubType(actualType));
                        if (c_collectionTypeKind(actualType) == OSPL_C_ARRAY && c_baseObjectKind(subType) == M_PRIMITIVE) {
                            os_free(arrayBrackets);
                            arrayBrackets = idl_CreateCSharpArrayIterationIndex(collStartType, dimension + 1);
                            idl_printIndent(indent_level);
                            idl_fileOutPrintf(idl_fileCur(), "Marshal.Copy(%s, to.%s%s, 0, %d);\n",
                                    bufName, fieldName, arrayBrackets, c_collectionTypeMaxSize(actualType));
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
                                    dimension + 1, fieldName, seqBufName, csUserData);
                            os_free(seqBufName);
                        }
                    } else {
                        idl_CreateSequenceMemberReadInnerLoopBody(
                                collStartType, actualType, actualTypeKind, index, dimension + 1,
                                fieldName, bufName, csUserData);
                    }
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "%s = new IntPtr(%s.ToInt64() + %s);\n", bufName, bufName, nextBufTypeSize);
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
        SACSSplDcpsUserData *csUserData)
{
    c_ulong bufNameLength;
    c_char *bufName = NULL;
    c_bool isPredefined = FALSE;
    c_char *memberTypeName = idl_scopeStackFromCType(memberType);

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
            /* Get the CSharp specific name of the member type. */
            c_char *prevTypeName = memberTypeName;
            memberTypeName = idl_CsharpId(prevTypeName, csUserData->customPSM, FALSE);
            os_free(prevTypeName);

            /* Invoke the Marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "%sMarshaler.CopyOut(ref from.%s, ref to.%s);\n",
                    memberTypeName, memberName, memberName);
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
            idl_CreateArrayMemberRead(
                    memberType, memberType, index, 0, memberName, memberName, csUserData);
            break;
        case OSPL_C_SEQUENCE:
            bufNameLength = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            bufName = os_malloc(bufNameLength);
            snprintf(bufName, bufNameLength, "attr%dSeq%dBuf", index, 0);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr %s = from.%s;\n", bufName, memberName);

            idl_CreateArrayMemberRead(
                    memberType, memberType, index, 0, memberName, bufName, csUserData);

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
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
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
    idl_fileOutPrintf(idl_fileCur(), "__%s nativeImg = (__%s) Marshal.PtrToStructure(from, typeof(__%s));\n",
            structName, structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandleTo = GCHandle.FromIntPtr(to);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s toObj = tmpGCHandleTo.Target as %s;\n", structName, structName);
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
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, ref %s to)\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "__%s nativeImg = (__%s) Marshal.PtrToStructure(from, typeof(__%s));\n",
            structName, structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref to);\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 3rd CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public static void StaticCopyOut(System.IntPtr from, ref %s to)\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "__%s nativeImg = (__%s) Marshal.PtrToStructure(from, typeof(__%s));\n",
            structName, structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(ref nativeImg, ref to);\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 4th CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public static void CopyOut(ref __%s from, ref %s to)\n", structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Cast the object to its proper type and if no yet allocated, allocate it. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (to == null) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "to = new %s();\n", structName);
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
    const c_char *structName,
    SACSSplDcpsUserData *csUserData)
{
    /* Get the meta-data of this datatype from the database. */
    c_type structType = idl_typeSpecDef(idl_typeSpec(typeSpec));

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %sMarshaler\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public sealed class %sMarshaler : DDS.OpenSplice.CustomMarshalers.FooDatabaseMarshaler<%s>\n",
        structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Create the constructor of the Marshaler. */
    idl_determineFullyScopedName(structType);
    idl_CreateAttributes(structType, structName, csUserData);
    idl_CreateInitEmbeddedMarshalers(structType, structName, csUserData);
    idl_CreateCopyIn(structType, structName, csUserData);
    idl_CreateCopyOut(structType, structName, csUserData);

    /* Decrease the indentation level and generate the closing bracket. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
}

