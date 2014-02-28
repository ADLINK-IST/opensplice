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

#include "os.h"
#include <ctype.h>
#include "c_typebase.h"

    /** indentation level */
static c_long indent_level = 0;
    /** enumeration element index */
static c_long enum_element = 0;
    /** enumeration enum name */
static char *enum_enumName = NULL;

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
    /* Generate inclusion of standard OpenSplice DDS type definition files */
    idl_fileOutPrintf(idl_fileCur(), "using DDS;\n");
    idl_fileOutPrintf(idl_fileCur(), "using DDS.OpenSplice.CustomMarshalers;\n");
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
idl_cTypeToCSharp(
        c_type memberType,
        const c_char *memberTypeName,
        SACSSplDcpsUserData *csUserData)
{
    c_longlong nrElements;
    c_char *dbType = NULL, *dbTypeTmp, *memberTypeNameTmp;
    c_long dbTypeSize;

    switch (c_baseObjectKind(memberType))
    {
    case M_PRIMITIVE:
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
        case P_CHAR:
        case P_OCTET:
            dbType = os_strdup("char");
            break;
        case P_SHORT:
        case P_USHORT:
            dbType = os_strdup("short");
            break;
        case P_LONG:
        case P_ULONG:
            dbType = os_strdup("int");
            break;
        case P_LONGLONG:
        case P_ULONGLONG:
            dbType = os_strdup("long");
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
        dbType = os_strdup("int");
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
        case C_STRING:
        case C_SEQUENCE:
            dbType = os_strdup("IntPtr");
            break;
        case C_ARRAY:
            nrElements = (c_longlong) c_collectionTypeMaxSize(memberType);
            memberType = c_typeActualType(c_collectionTypeSubType(memberType));
            while (c_baseObjectKind(memberType) == M_COLLECTION &&
                    c_collectionTypeKind(memberType) == C_ARRAY)
            {
                nrElements *= (c_longlong) c_collectionTypeMaxSize(memberType);
                memberType = c_typeActualType(c_collectionTypeSubType(memberType));
            }
            /* Generate a tag to make sure the C# array uses an inline representation. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                    idl_fileCur(),
                    "[MarshalAs(UnmanagedType.ByValArray, SizeConst=%lld)]\n",
                    nrElements);
            memberTypeNameTmp = idl_scopeStackFromCType(memberType);
            dbTypeTmp = idl_cTypeToCSharp(memberType, memberTypeNameTmp, csUserData);
            dbTypeSize = strlen(dbTypeTmp) + 2 /* '[' & ']' */ + 1 /* '\0' */;
            dbType = os_malloc(dbTypeSize);
            snprintf(dbType, dbTypeSize, "%s[]", dbTypeTmp);
            os_free(dbTypeTmp);
            os_free(memberTypeNameTmp);
            break;
        default:
            /* Unsupported structure member type */
            assert(FALSE);
        }
        break;
    default:
        /* Unsupported structure member type */
        assert(FALSE);
    }

    return dbType;
}

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
        }
        break;
    default:
        /* This part of the code should never be reached for other types. */
        assert(FALSE);
    }

    return baseType;
}

static void
idl_generateDatabaseRepresentation (
    idl_typeStruct typeSpec,
    const c_char *structName,
    SACSSplDcpsUserData *csUserData)
{
    /* Get the meta-data of this datatype from the database. */
    c_type structType = idl_typeSpecDef(idl_typeSpec(typeSpec));
    c_long i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region __%s\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Sequential)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public sealed class __%s\n",
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
        c_type memberType = c_memberType(structMember);
        c_char *memberName = idl_CsharpId(
                c_specifierName(structMember),
                csUserData->customPSM,
                FALSE);

        /* Dereference possible typedefs first. */
        while (c_baseObjectKind(memberType) == M_TYPEDEF) {
            memberType = c_typeDef(memberType)->alias;
        }

        /* Now find and generate the corresponding database representation. */
        memberTypeName = idl_scopeStackFromCType(memberType);
        dbType = idl_cTypeToCSharp(memberType, memberTypeName, csUserData);

        /* generate the the database representation for the attribute.  */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "public %s %s;\n",
            dbType,
            memberName);

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
idl_determineOffsets(
        c_type structType,
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
{
    c_long i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Generate the code to obtain the C# 'Type' class of the datatype. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
            idl_fileCur(),
            "private static readonly Type type = typeof(__%s);\n",
            structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
            idl_fileCur(),
            "public static readonly int Size = Marshal.SizeOf(type);\n\n");

    /* Loop over the attributes of the datatype and process each attribute. */
    for (i = 0; i < nrMembers; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(structType, i);
        c_char *memberName = idl_CsharpId(
                c_specifierName(structMember),
                csUserData->customPSM,
                FALSE);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
                idl_fileCur(),
                "private static readonly int offset_%s = (int)Marshal.OffsetOf(type, \"%s\");\n",
                memberName,
                memberName);
        os_free(memberName);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n");
}

static void
idl_CreateAttributes(
        c_type structType,
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
{
    c_bool isPredefined = FALSE;
    c_long i, j, nrMembers = c_structureMemberCount((c_structure) structType);

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
            for (j = 0; c_baseObjectKind(memberType) == M_COLLECTION; j++) {
                /* For sequences, cache the database type for CopyIn. */
                if (c_collectionTypeKind(memberType) == C_SEQUENCE) {
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(),
                            "private IntPtr attr%dSeq%dType = IntPtr.Zero;\n", i, j);
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
                        "private DatabaseMarshaler attr%dMarshaler;\n", i);
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
    c_bool isPredefined = FALSE;
    c_long i, nrMembers = c_structureMemberCount((c_structure) structType);

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
        }

        /* Dereference possible typedefs/arrays/sequences first. */
        while ( !isPredefined &&
                ( c_baseObjectKind(memberType) == M_TYPEDEF ||
                ( c_baseObjectKind(memberType) == M_COLLECTION &&
                        ( c_collectionTypeKind(memberType) == C_ARRAY ||
                          c_collectionTypeKind(memberType) == C_SEQUENCE) ) ) ) {
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
                        "attr%dMarshaler = DatabaseMarshaler.GetMarshaler(participant, typeof(%s));\n",
                        i, memberTypeName);
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

static void
idl_CreateSampleReaderAlloc(
        c_type structType,
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
{
    /* Open the SampleReaderAlloc operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override object[] SampleReaderAlloc(int length)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate the code that instantiates an array of the specified type and length. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return new %s[length];\n", structName);

    /* Decrease the indent level back to its original value and close the operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static c_char *
idl_CreateArrayIterationIndex(
        c_type collStartType,
        c_long dimension)
{
#define IDL_INDEX_BUFFER_LENGTH 32

    c_long i;
    c_type nextType;
    c_type currentType = c_typeActualType(collStartType);

    /* maxResultLength = '\0' + dimension * ('[' + ']' + MaxInt + 'i') */
    c_long maxResultLength = 1 + dimension * (2 + 10 + 1);
    c_char *result = os_malloc(maxResultLength);
    c_char *format = NULL;
    c_char postfix[IDL_INDEX_BUFFER_LENGTH];

    assert (result != NULL);

    /* If no dimension specified, return an empty string. */
    result[0] = '\0';
    if (dimension < 1) return result;

    /* Assert that startType is always a collection type and has at least
     * the specified number of dimensions. */
    os_strncat(result, "[i0", maxResultLength);
    for (i = 1; i < dimension; i++) {
        /* Iterate to the subtype. */
        nextType = c_typeActualType(c_collectionTypeSubType(currentType));

        /* Assert currentType and nextType are collection types. */
        switch (c_collectionTypeKind(currentType))
        {
        case C_SEQUENCE:
            format = "][i%d";
            (void)snprintf (postfix, IDL_INDEX_BUFFER_LENGTH, format, i);
            break;
        case C_ARRAY:
            if (c_collectionTypeKind(nextType) == C_ARRAY)
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
idl_CreateArrayMemberWriteInnerLoopBody(
        c_type collStartType,
        c_type collType,
        c_metaKind collKind,
        c_long index,
        c_long dimension,
        const c_char *fieldName,
        const c_char *bufName,
        const c_char *cursorName)
{
    c_long maxSize;
    c_char *arrayBrackets = idl_CreateArrayIterationIndex(collStartType, dimension);
    switch(collKind)
    {
    case M_STRUCTURE:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "if (!attr%dMarshaler.CopyIn(basePtr, from.%s%s, %s, %s)) return false;\n",
                index, fieldName, arrayBrackets, bufName, cursorName);
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "Write(%s, %s, (uint) from.%s%s);\n",
                bufName, cursorName, fieldName, arrayBrackets);
        break;
    case M_PRIMITIVE:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "Write(%s, %s, from.%s%s);\n",
                bufName, cursorName, fieldName, arrayBrackets);
        break;
    case M_COLLECTION:
        /* Assert that this collection is always of type string!! */
        assert(c_collectionTypeKind(collType) == C_STRING);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "if (from.%s%s == null) return false;\n", fieldName, arrayBrackets);
        maxSize = c_collectionTypeMaxSize(collType);
        /* Check for bounds if a maximum bound was specified. */
        if (maxSize > 0)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (from.%s%s.Length > %d) return false;\n", fieldName, arrayBrackets, maxSize);
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "Write(basePtr, %s, %s, ref from.%s%s);\n",
                bufName, cursorName, fieldName, arrayBrackets);
        break;
    default:
        /* Unsupported array type. */
        assert(FALSE);
    }

    /* Increase the cursor to the next element. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s += %d;\n", cursorName, c_typeSize(collType));
    os_free(arrayBrackets);
}

static void
idl_CreateArrayMemberWrite(
        c_type collStartType,
        c_type collType,
        c_long index,
        c_long dimension,
        const c_char *fieldName,
        const c_char *bufName,
        const c_char *cursorName)
{
    c_long arrLength, seqMaxLength;
    c_long seqLengthNameSize, seqBufNameSize, seqTypeNameSize, nextCursorNameSize;
    c_char *seqLengthName, *seqBufName, *seqTypeName, *nextCursorName, *iterationIndex;
    c_type subType, actualType;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_CreateArrayMemberWriteInnerLoopBody(
                collStartType, collType, collKind, index, dimension,
                        fieldName, bufName, cursorName);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case C_STRING:
            /* Handle strings. */
            idl_CreateArrayMemberWriteInnerLoopBody(
                    collStartType, collType, collKind, index, dimension,
                            fieldName, bufName, cursorName);
            break;
        case C_ARRAY:
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
                    dimension + 1, fieldName, bufName, cursorName);

            /* Close the for loop and decrease the indent level back to its original. */
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            break;
        case C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);

            seqLengthNameSize = 13 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqLengthName = os_malloc(seqLengthNameSize);
            snprintf(seqLengthName, seqLengthNameSize, "attr%dSeq%dLength", index, dimension);

            seqBufNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqBufName = os_malloc(seqBufNameSize);
            snprintf(seqBufName, seqBufNameSize, "attr%dSeq%dBuf", index, dimension);

            seqTypeNameSize = 11 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqTypeName = os_malloc(seqTypeNameSize);
            snprintf(seqTypeName, seqTypeNameSize, "attr%dSeq%dType", index, dimension);

            nextCursorNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            nextCursorName = os_malloc(nextCursorNameSize);
            snprintf(nextCursorName, nextCursorNameSize, "attr%dCursor%d", index, dimension + 1);

            seqMaxLength = c_collectionTypeMaxSize(collType);
            iterationIndex = idl_CreateArrayIterationIndex(collStartType, dimension);

            /* Check whether the sequence is valid. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s%s == null) return false;\n",
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
                idl_fileOutPrintf(idl_fileCur(), "if (%s > %d) return false;\n", seqLengthName, seqMaxLength);
            }

            /* Locate the database type-definition of the sequence. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (%s == IntPtr.Zero) {\n", seqTypeName);
            indent_level++;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr memberOwnerType = DDS.OpenSplice.Database.c.resolve(basePtr, fullyScopedName);\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr specifier = DDS.OpenSplice.Database.c.metaResolveSpecifier(memberOwnerType, \"%s\");\n",
                    fieldName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr specifierType = DDS.OpenSplice.Database.c.specifierType(specifier);\n");
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
                    seqBufName, seqTypeName, seqLengthName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "Write(%s, %s, %s);\n",
                    bufName, cursorName, seqBufName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s += %d;\n",
                    cursorName, c_typeSize(collType));
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "int %s = 0;\n", nextCursorName);

            /* Open a for loop to walk over the current dimension and increase the indent. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %s; i%d++) {\n",
                    dimension, dimension, seqLengthName, dimension);
            indent_level++;

            /* Generate the the body to process the next dimension. */
            idl_CreateArrayMemberWrite(collStartType, actualType, index,
                    dimension + 1, fieldName, seqBufName, nextCursorName);

            /* Close the for loop and decrease the indent level back to its original. */
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");

            os_free(iterationIndex);
            os_free(nextCursorName);
            os_free(seqTypeName);
            os_free(seqBufName);
            os_free(seqLengthName);
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
        }
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
    }
}

static void
idl_CreateStructMemberWrite(
        c_type memberType,
        const c_char * memberName,
        c_long index,
        SACSSplDcpsUserData *csUserData)
{
    c_long cursorNameLength, maxSize;
    c_char *cursorName;
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
        {
            /* Generate code to use the predefined Write operation in the base marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(to, offset + offset_%s, from.%s);\n",
                    memberName,
                    memberName);
        }
        else
        {
            /* Otherwise generate code to use the dedicated marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (!attr%dMarshaler.CopyIn(basePtr, from.%s, to, offset + offset_%s)) return false;\n",
                    index,
                    memberName,
                    memberName);
        }
        break;
    case M_ENUMERATION:
        /* Generate code to handle an enum. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "Write(to, offset + offset_%s, (uint) from.%s);\n",
                memberName,
                memberName);
        break;
    case M_PRIMITIVE:
        /* Generate code to handle a primitive. */
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
        case P_CHAR:        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(to, offset + offset_%s, from.%s);\n",
                    memberName,
                    memberName);
            break;
        default:
            /* Unsupported primitive type. */
            assert(FALSE);
        }
        break;
    case M_COLLECTION:
        /* Generate code to handle Collection types. */
        switch (c_collectionTypeKind(memberType))
        {
        case C_STRING:
            /* Generate code to handle strings. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (from.%s == null) return false;\n", memberName);
            /* Generate code to check for bounds if a maximum bound was specified. */
            maxSize = c_collectionTypeMaxSize(memberType);
            idl_printIndent(indent_level);
            if (maxSize > 0)
            {
                idl_fileOutPrintf(idl_fileCur(),
                        "if (from.%s.Length > %d) return false;\n",
                        memberName,
                        maxSize);
            }
            else
            {
                idl_fileOutPrintf(idl_fileCur(), "// Unbounded string: bounds check not required...\n");
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "Write(basePtr, to, offset + offset_%s, ref from.%s);\n",
                    memberName,
                    memberName);
            break;
        case C_SEQUENCE:
        case C_ARRAY:
            /* Generate code to handle arrays or sequences. */
            cursorNameLength = 12 /* template */ + 10 /* MAX_INT */ + 1 /* '\0' */;
            cursorName = os_malloc(cursorNameLength);
            snprintf(cursorName, cursorNameLength, "attr%dCursor0", index);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "int %s = offset + offset_%s;\n",
                    cursorName,
                    memberName);
            idl_CreateArrayMemberWrite(
                    memberType, memberType, index, 0, memberName, "to", cursorName);
            os_free(cursorName);
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
idl_CreateCopyIn(
        c_type structType,
        const c_char *structName,
        SACSSplDcpsUserData *csUserData)
{
    c_long i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Open the 1st CopyIn operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override bool CopyIn(System.IntPtr basePtr, System.IntPtr from, System.IntPtr to)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate code that obtains a C# object from a C pointer and then invokes the 2nd CopyIn. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandle = GCHandle.FromIntPtr(from);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "object fromData = tmpGCHandle.Target;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return CopyIn(basePtr, fromData, to, 0);\n");

    /* Decrease the indent level back to its original value and close the 1st CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 2nd CopyIn operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override bool CopyIn(System.IntPtr basePtr, object untypedFrom, System.IntPtr to, int offset)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate code that checks whether the C# input is a valid object. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s from = untypedFrom as %s;\n", structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (from == null) return false;\n");

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
    idl_fileOutPrintf(idl_fileCur(), "return true;\n");

    /* Decrease the indent level back to its original value and close the 2nd CopyIn operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
idl_CreateArrayInitialization(
        c_type currentType,
        const c_char *fieldName,
        const c_char *seqLengthName,
        const c_char *arrayBrackets,
        SACSSplDcpsUserData *csUserData)
{
    c_char *arrayNoIndex = NULL, *arrayIndex = NULL, *arrayCtor;
    c_type subType = c_typeActualType(c_collectionTypeSubType(currentType));
    idl_typeSpec typeSpec = idl_makeTypeCollection(c_collectionType(currentType));

    /* Resolve the typename of the current collection. */
    arrayCtor = idl_CsharpTypeFromTypeSpec(typeSpec, csUserData->customPSM);

   /* Resolve the index representation for the current collection. */
    switch (c_collectionTypeKind(currentType))
    {
    case C_SEQUENCE:
        arrayIndex = idl_sequenceCsharpIndexString(
                typeSpec, SACS_INCLUDE_INDEXES, seqLengthName);
        break;
    case C_ARRAY:
        arrayIndex = idl_arrayCsharpIndexString(typeSpec, SACS_INCLUDE_INDEXES);
        break;
    default:
        /* Unsupported collection type. */
        assert(FALSE);
    }

    /* The initialization code for the current collection depends on its subtype. */
    switch (c_baseObjectKind(subType))
    {
    /* Primitive kind, no need to recycle elements. */
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "dataTo.%s%s = new %s%s;\n",
                fieldName, arrayBrackets, arrayCtor, arrayIndex);
        break;
    /* Object kind, try to recycle elements as much as possible. */
    case M_STRUCTURE:
    case M_COLLECTION:
        switch (c_collectionTypeKind(currentType)) {
            case C_SEQUENCE:
                arrayNoIndex = idl_sequenceCsharpIndexString(
                    typeSpec, SACS_EXCLUDE_INDEXES, seqLengthName);
                break;
            case C_ARRAY:
                arrayNoIndex = idl_arrayCsharpIndexString(
                    typeSpec, SACS_EXCLUDE_INDEXES);
                break;
            default:
                assert(FALSE); /* Unsupported collection type. */
                break;
        }

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s%s target = new %s%s;\n",
                arrayCtor, arrayNoIndex, arrayCtor, arrayIndex);
        if (c_collectionTypeKind(currentType) == C_ARRAY &&
            c_baseObjectKind(subType) == M_COLLECTION &&
            c_collectionTypeKind(subType) == C_ARRAY)
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "// Rectangular array: recycling not yet possible...\n");
        }
        else
        {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "initObjectSeq(dataTo.%s%s, target);\n",
                    fieldName, arrayBrackets);
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "dataTo.%s%s = target as %s%s;\n",
                fieldName, arrayBrackets, arrayCtor, arrayNoIndex);
        os_free(arrayNoIndex);
        break;
    default:
        /* Unsupported collection type. */
        assert(FALSE);
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
        c_long index,
        c_long dimension,
        const c_char *fieldName,
        const c_char *bufName,
        const c_char *cursorName,
        SACSSplDcpsUserData *csUserData)
{
    c_char *arrayBrackets = idl_CreateArrayIterationIndex(collStartType, dimension);
    c_char *colTypeName = idl_scopeStackFromCType(collType);

    switch(collKind)
    {
    case M_STRUCTURE:
        if (idl_isPredefined(colTypeName)) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "dataTo.%s%s = (%s) Read%s(%s, %s);\n",
                    fieldName, arrayBrackets, colTypeName,
                    idl_cTypeToBaseType(collType, colTypeName),
                    bufName, cursorName);
        } else {
            /* Get the CSharp specific name of the member type. */
            c_char *prevColTypeName = colTypeName;
            colTypeName = idl_CsharpId(prevColTypeName, csUserData->customPSM, FALSE);
            os_free(prevColTypeName);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "object elementObj = dataTo.%s%s;\n", fieldName, arrayBrackets);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "attr%dMarshaler.CopyOut(%s, ref elementObj, %s);\n",
                    index, bufName, cursorName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (dataTo.%s%s == null) {\n", fieldName, arrayBrackets);
            indent_level++;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "dataTo.%s%s = elementObj as %s;\n",
                    fieldName, arrayBrackets, colTypeName);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
        }
        break;
    case M_ENUMERATION:
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "dataTo.%s%s = (%s) ReadUInt32(%s, %s);\n",
                fieldName, arrayBrackets, colTypeName, bufName, cursorName);
        break;
    case M_PRIMITIVE:
    case M_COLLECTION: /* Assert that this collection is always of type string!! */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "dataTo.%s%s = Read%s(%s, %s);\n",
                fieldName, arrayBrackets,
                idl_cTypeToBaseType(collType, colTypeName),
                bufName, cursorName);
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s += %d;\n", cursorName, c_typeSize(collType));

    os_free(colTypeName);
    os_free(arrayBrackets);
}

static void
idl_CreateArrayMemberRead(
        c_type collStartType,
        c_type collType,
        c_long index,
        c_long dimension,
        const c_char *fieldName,
        const c_char *bufName,
        const c_char *cursorName,
        SACSSplDcpsUserData *csUserData)
{
    c_long i, arrLength, dimensionCheckSize;
    c_long seqLengthNameSize, seqBufNameSize, seqTypeNameSize, nextCursorNameSize;
    c_char *seqLengthName, *seqBufName, *seqTypeName, *nextCursorName, *arrayBrackets;
    c_char *dimensionCheck;
    c_type subType, actualType, parentType;

    c_metaKind collKind = c_baseObjectKind(collType);
    switch(collKind)
    {
    case M_STRUCTURE:
    case M_ENUMERATION:
    case M_PRIMITIVE:
        idl_CreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                index, dimension, fieldName, bufName, cursorName, csUserData);
        break;
    case M_COLLECTION:
        /* Handle Collection type. */
        switch (c_collectionTypeKind(collType))
        {
        case C_STRING:
            /* Handle strings. */
            idl_CreateArrayMemberReadInnerLoopBody(collStartType, collType, collKind,
                    index, dimension, fieldName, bufName, cursorName, csUserData);
            break;
        case C_ARRAY:
            /* Handle arrays. */
            arrLength = c_collectionTypeMaxSize(collType);
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);
            arrayBrackets = idl_CreateArrayIterationIndex(collStartType, dimension);

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
                     c_collectionTypeKind(parentType) != C_ARRAY))
            {
                /* If no prior rectangular dimensions, then check the current dimensions. */
                c_long curDim;
                subType = c_typeActualType(collType);
                dimensionCheckSize = 15 + strlen(fieldName) + strlen (arrayBrackets) + 1;
                dimensionCheck = os_malloc(dimensionCheckSize);
                snprintf(dimensionCheck, dimensionCheckSize,
                        "dataTo.%s%s == null", fieldName, arrayBrackets);
                for (i = 0; c_baseObjectKind(subType) == M_COLLECTION &&
                            c_collectionTypeKind(subType) == C_ARRAY; i++)
                {
                    c_char *prevdimensionCheck = dimensionCheck;
                    curDim = c_collectionTypeMaxSize(subType);
                    dimensionCheckSize = strlen(prevdimensionCheck) + 26 + strlen(fieldName) +
                            strlen(arrayBrackets) + 2 * 10 /* MAX_INT */ + 1;
                    dimensionCheck = os_malloc(dimensionCheckSize);
                    snprintf(dimensionCheck, dimensionCheckSize,
                            "%s || dataTo.%s%s.GetLength(%d) != %d",
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
                    dimension + 1, fieldName, bufName, cursorName, csUserData);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            break;
        case C_SEQUENCE:
            /* Handle sequences. */
            subType = c_collectionTypeSubType(collType);
            actualType = c_typeActualType(subType);

            seqLengthNameSize = 13 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqLengthName = os_malloc(seqLengthNameSize);
            snprintf(seqLengthName, seqLengthNameSize, "attr%dSeq%dLength", index, dimension);

            seqBufNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqBufName = os_malloc(seqBufNameSize);
            snprintf(seqBufName, seqBufNameSize, "attr%dSeq%dBuf", index, dimension);

            seqTypeNameSize = 11 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            seqTypeName = os_malloc(seqTypeNameSize);
            snprintf(seqTypeName, seqTypeNameSize, "attr%dSeq%dType", index, dimension);

            nextCursorNameSize = 10 + 2 * 10 + 1; /* tmpl-size + 2 * MAX_INT + '\0'. */
            nextCursorName = os_malloc(nextCursorNameSize);
            snprintf(nextCursorName, nextCursorNameSize, "attr%dCursor%d", index, dimension + 1);

            arrayBrackets = idl_CreateArrayIterationIndex(collStartType, dimension);

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "IntPtr %s = ReadIntPtr(%s, %s);\n",
                    seqBufName, bufName, cursorName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "int %s = DDS.OpenSplice.Database.c.arraySize(%s);\n",
                    seqLengthName, seqBufName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "if (dataTo.%s%s == null || dataTo.%s%s.Length != %s) {\n",
                    fieldName, arrayBrackets, fieldName, arrayBrackets, seqLengthName);
            indent_level++;
            idl_CreateArrayInitialization(collType, fieldName, seqLengthName, arrayBrackets, csUserData);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s += %d;\n", cursorName, c_typeSize(collType));
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "int %s = 0;\n", nextCursorName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "for (int i%d = 0; i%d < %s; i%d++) {\n",
                    dimension, dimension, seqLengthName, dimension);
            indent_level++;
            idl_CreateArrayMemberRead(collStartType, actualType, index,
                    dimension + 1, fieldName, seqBufName, nextCursorName, csUserData);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            break;
        default:
            /* Unsupported Collection type. */
            assert(FALSE);
        }
        break;
    default:
        /* Unsupported Array type. */
        assert(FALSE);
    }
}

static void
idl_CreateStructMemberRead(
        c_type memberType,
        const char *memberName,
        c_long index,
        SACSSplDcpsUserData *csUserData)
{
    c_long cursorNameLength;
    c_char *cursorName;
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
                    "dataTo.%s = Read%s(from, offset + offset_%s);\n",
                    memberName,
                    idl_cTypeToBaseType(memberType, memberTypeName),
                    memberName);
        }
        else
        /* Other types should use their corresponding Marshaler. */
        {
            /* Get the CSharp specific name of the member type. */
            c_char *prevTypeName = memberTypeName;
            memberTypeName = idl_CsharpId(prevTypeName, csUserData->customPSM, FALSE);
            os_free(prevTypeName);

            /* Pick a holder for the read result. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "object attr%dVal = dataTo.%s;\n", index, memberName);

            /* Invoke the Marshaler. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "attr%dMarshaler.CopyOut(from, ref attr%dVal, offset + offset_%s);\n",
                    index, index, memberName);

            /* If the target did not yet instantiate its attribute, replace it by the holder. */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "if (dataTo.%s == null) {\n", memberName);
            indent_level++;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "dataTo.%s = attr%dVal as %s;\n", memberName, index, memberTypeName);
            indent_level--;
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "}\n");
        }
        break;
    case M_ENUMERATION:
        /* Enums are read as Int32 types. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(),
                "dataTo.%s = (%s) ReadUInt32(from, offset + offset_%s);\n",
                memberName, memberTypeName, memberName);
        break;
    case M_PRIMITIVE:
        /* Handle primitive. */
        switch (c_primitiveKind(memberType))
        {
        case P_BOOLEAN:
        case P_CHAR:        case P_OCTET:
        case P_SHORT:       case P_USHORT:
        case P_LONG:        case P_ULONG:
        case P_LONGLONG:    case P_ULONGLONG:
        case P_FLOAT:       case P_DOUBLE:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "dataTo.%s = Read%s(from, offset + offset_%s);\n",
                    memberName,
                    idl_cTypeToBaseType(memberType, memberTypeName),
                    memberName);
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
        case C_STRING:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "dataTo.%s = ReadString(from, offset + offset_%s);\n",
                    memberName, memberName);
            break;
        case C_SEQUENCE:
        case C_ARRAY:
            /* Handle arrays. */
            cursorNameLength = 12 /* template */ + 10 /* MAX_INT */ + 1 /* '\0' */;
            cursorName = os_malloc(cursorNameLength);
            snprintf(cursorName, cursorNameLength, "attr%dCursor0", index);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(),
                    "int %s = offset + offset_%s;\n",
                    cursorName,
                    memberName);
            idl_CreateArrayMemberRead(
                    memberType, memberType, index, 0, memberName, "from", cursorName, csUserData);
            os_free(cursorName);
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
    c_long i, nrMembers = c_structureMemberCount((c_structure) structType);

    /* Open the 1st CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, System.IntPtr to)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Generate a body that retrieves the C# object and invokes the appropriate CopyOut. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "GCHandle tmpGCHandleTo = GCHandle.FromIntPtr(to);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "object toObj = tmpGCHandleTo.Target;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "CopyOut(from, ref toObj, 0);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (toObj != tmpGCHandleTo.Target) tmpGCHandleTo.Target = toObj;\n");

    /* Decrease the indent level back to its original value and close the CopyOut operation. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Open the 2nd CopyOut operation and increase the indent. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public override void CopyOut(System.IntPtr from, ref object to, int offset)\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;

    /* Cast the object to its proper type and if no yet allocated, allocate it. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s dataTo = to as %s;\n", structName, structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (dataTo == null) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "dataTo = new %s();\n", structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "to = dataTo;\n");
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
        "public sealed class %sMarshaler : DDS.OpenSplice.CustomMarshalers.DatabaseMarshaler\n",
        structName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Create the constructor of the Marshaler. */
    idl_determineFullyScopedName(structType);
    idl_determineOffsets(structType, structName, csUserData);
    idl_CreateAttributes(structType, structName, csUserData);
    idl_CreateInitEmbeddedMarshalers(structType, structName, csUserData);
    idl_CreateSampleReaderAlloc(structType, structName, csUserData);
    idl_CreateCopyIn(structType, structName, csUserData);
    idl_CreateCopyOut(structType, structName, csUserData);

    /* Decrease the indentation level and generate the closing bracket. */
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
}

