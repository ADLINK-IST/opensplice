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
 * This module generates Standalone C# data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSACSType.h"
#include "idl_genSACSHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genCHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_dll.h"
#include "idl_map.h"


#include "vortex_os.h"
#include <ctype.h>
#include "c_typebase.h"


/** indentation level */
static c_long indent_level = 0;
    /** enumeration element index */
static c_ulong enum_element = 0;
    /** enumeration enum name */
static char *enum_enumName = NULL;


struct SACSUnionMetaDescription_s {
    idl_map unionCaseMap;
    char *unionSwitchType;
    char *union1stCaseValue;
    idl_typeSpec union1stCaseType;
    char *union1stCaseIdent;
    os_iter labelIter;
    os_iter labelsUsedIter;
    c_bool caseIsDefault;
    c_ulong unionCaseOffset;
};
typedef struct SACSUnionMetaDescription_s SACSUnionMetaDescription;



/** @brief function to find out whether the elements of an array
 * require initialization.
 *
 * idl_arrayElementsNeedInitialization is a local support function to find out
 * whether the specified array is of a type for which all elements need to be
 * initialized individually. If the array is of a primitive type, this is never
 * necessary, but if the array contains a reference type and no underlying
 * sequences (which will be initialized to 0 elements) then for loops will
 * need to be created to explicitly initialize all of these attributes.
 *
 * @param typeSpec Specifies the attribute type that needs to be investigated.
 */
static int
idl_arrayElementsNeedInitialization(
    idl_typeArray typeArray)
{
    int initRequired = FALSE;

    /* Obtain the type of the array. */
    idl_typeSpec typeSpec = idl_typeArrayType(typeArray);

    /* Resolve potential typedefs. */
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        initRequired = idl_arrayElementsNeedInitialization(idl_typeArray(typeSpec));
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
               idl_typeSpecType(typeSpec) == idl_tunion  ||
               idl_typeSpecType(typeSpec) == idl_tenum   ||
               (idl_typeSpecType(typeSpec) == idl_tbasic &&
                idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_string)) {
        initRequired = TRUE;
    }

    return initRequired;
}

/** @brief generate initialization of array elements.
 *
 * idl_arrayElementInit generates for-loops that initialize
 * each attribute of an array explicitly.
 *
 * @param typeArray Specifies the type of the array
 */
static void
idl_arrayElementInit(
    idl_typeSpec typeSpec,
    const char *elementName,
    int dimCount,
    int indent,
    c_bool customPSM)
{
    c_char *tName;

    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        idl_fileOutPrintf(
            idl_fileCur(), "%*sfor(int i%d = 0; i%d < %d; i%d++) {\n",
                indent, "", dimCount, dimCount, idl_typeArraySize(idl_typeArray(typeSpec)), dimCount);
        idl_arrayElementInit(idl_typeArrayType(idl_typeArray(typeSpec)), elementName, dimCount + 1, indent + 4, customPSM);
        idl_fileOutPrintf(idl_fileCur(), "%*s}\n", indent, "");
    } else {
        int j;

        idl_fileOutPrintf(idl_fileCur(), "%*s_%s[", indent, "", elementName);
        for (j = 1; j < dimCount; j++) {
            if (j == 1) {
                idl_fileOutPrintf(idl_fileCur(), "i%d", j);
            } else {
                idl_fileOutPrintf(idl_fileCur(), ",i%d", j);
            }
        }
        if (idl_typeSpecType(typeSpec) == idl_tbasic && idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            idl_fileOutPrintf(idl_fileCur(), "] = \"\";\n");
        } else if (idl_typeSpecType(typeSpec) == idl_tunion || idl_typeSpecType(typeSpec) == idl_tstruct ) {
            tName = idl_CsharpTypeFromTypeSpec(typeSpec, customPSM);
            idl_fileOutPrintf(idl_fileCur(), "] = new %s();\n", tName);
            os_free(tName);
        } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
            tName = idl_CsharpTypeFromTypeSpec(typeSpec, customPSM);
            idl_fileOutPrintf(idl_fileCur(), "] = (%s)(0);\n", tName);
            os_free(tName);
        }
    }
}


/** @brief generate initialization of array elements.
 *
 * idl_arrayInitializeElements is a local support function to initialize
 * the elements of non-primitive array types, if appropriate.
 *
 * @param typeSpec Type of the attribute that might need to be initialized.
 * @param elementName Name of the attribute that might need to be initialized.
 */
static void
idl_arrayInitializeElements(
    idl_typeSpec typeSpec,
    const char *elementName,
    c_bool customPSM)
{
    int dimCount = 1;
    int indent = (indent_level + 1) * 4;

    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if ( idl_typeSpecType(typeSpec) == idl_tarray) {
        if (idl_arrayElementsNeedInitialization(idl_typeArray(typeSpec))) {
            idl_arrayElementInit(typeSpec, elementName, dimCount, indent, customPSM);
        }
    }
}






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
    SACSTypeUserData *csUserData = (SACSTypeUserData *) userData;
    idl_action action;

    scopedStructName = idl_scopeStackCsharp(
            idl_typeUserScope(idl_typeUser(structSpec)),
            ".",
            idl_typeSpecName(idl_typeSpec(structSpec)));
    if (idl_isPredefined(scopedStructName)) {
        /* return idl_abort to indicate that the structure does not need to be processed. */
        action = idl_abort;
    } else {
        char *structName;

        /* Add the metadata of this struct to a list for later processing. */
        idl_metaCharpAddType(scope, name, idl_typeSpec(structSpec), &csUserData->idlpp_metaList);

        /* Generate the C# code that opens a sealed class. */
        structName = idl_CsharpId(name, csUserData->customPSM, FALSE);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "#region %s\n", structName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Sequential)]\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "public sealed class %s\n",
            structName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");

        /* Increase the indentation level. */
        indent_level++;

        /* return idl_explore to indicate that the rest of the structure needs to be processed. */
        action = idl_explore;
        os_free(structName);
    }
    os_free(scopedStructName);
    return action;
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
idl_structureClose (
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    /* Decrease the indentation level back to its original size. */
    indent_level--;

    /* Generate the C# code that closes the class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "};\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");
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
idl_structureMemberOpenClose (
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    c_bool isPredefined = FALSE;
    char *tpName = idl_CsharpTypeFromTypeSpec(typeSpec, csUserData->customPSM);
    char *str_no_idx = NULL;
    char *str_idx = NULL;
    OS_UNUSED_ARG(scope);

    OS_UNUSED_ARG(scope);

    /* Dereference possible typedefs first. */
    while (idl_typeSpecType(typeSpec) == idl_ttypedef && !isPredefined) {
        if (!idl_isPredefined(tpName)) {
            typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
            os_free (tpName);
            tpName = idl_CsharpTypeFromTypeSpec(typeSpec, csUserData->customPSM);
        } else {
            isPredefined = TRUE;
        }
    }

    idl_printIndent(indent_level);
    switch (idl_typeSpecType(typeSpec))
    {
    case idl_tbasic:
        /* generate code for a standard empty ("") string.  */
        if (idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_string) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "public %s %s = string.Empty;\n",
                tpName,
                idl_CsharpId(name, csUserData->customPSM, FALSE));
        } else {
            /* generate code for a standard primitive type. */
            idl_fileOutPrintf(
                idl_fileCur(),
                "public %s %s;\n",
                tpName,
                idl_CsharpId(name, csUserData->customPSM, FALSE));
        }
        break;

    case idl_tenum:
        /* generate code for a standard enum. */
        idl_fileOutPrintf(
            idl_fileCur(),
            "public %s %s;\n",
            tpName,
            idl_CsharpId(name, csUserData->customPSM, FALSE));
        break;

    case idl_tstruct:
    case idl_tunion:
    {
        /* generate code for a standard mapping of a struct or union user-type mapping */
        idl_fileOutPrintf(
            idl_fileCur(),
            "public %s %s = new %s();\n",
            tpName,
            idl_CsharpId(name, csUserData->customPSM, FALSE),
            tpName);
        break;
    }
    case idl_tarray:
    {
        /* Initialize to 2 ('[]') so that every other dimension just has to add 1 ','. */
        str_no_idx = idl_arrayCsharpIndexString (
            typeSpec, SACS_EXCLUDE_INDEXES);
        str_idx = idl_arrayCsharpIndexString (
            typeSpec, SACS_INCLUDE_INDEXES);
        assert (str_no_idx && str_idx);

        /* generate code for an array mapping */
        idl_fileOutPrintf(
            idl_fileCur(),
            "public %s%s %s = new %s%s;\n",
            tpName,
            str_no_idx,
            idl_CsharpId(name, csUserData->customPSM, FALSE),
            tpName,
            str_idx);
        break;
    }

    case idl_tseq:
    {
        /* Initialize to 2 ('[]') so that every other dimension just has to add 2 '[]'. */
        str_no_idx = idl_sequenceCsharpIndexString (
            typeSpec, SACS_EXCLUDE_INDEXES, NULL);
        str_idx = idl_sequenceCsharpIndexString (
            typeSpec, SACS_INCLUDE_INDEXES, NULL);
        assert (str_no_idx && str_idx);

        /* generate code for a sequence mapping */
        idl_fileOutPrintf(
            idl_fileCur(),
            "public %s%s %s = new %s%s;\n",
            tpName,
            str_no_idx,
            idl_CsharpId(name, csUserData->customPSM, FALSE),
            tpName,
            str_idx);
        break;
    }

    case idl_ttypedef:
        /* This state should only be reachable for predefined types. */
        assert(isPredefined);
        idl_fileOutPrintf(
                idl_fileCur(),
                "public %s %s;\n",
                tpName,
                idl_CsharpId(name, csUserData->customPSM, FALSE));
        break;

    default:
        printf("idl_structureMemberOpenClose: Unsupported structure member type (member name = %s, type name = %s)\n",
            name, idl_scopedTypeName(typeSpec));
        break;
    }

    os_free(tpName);
    if (str_idx) {
        os_free(str_idx);
    }
    if (str_no_idx) {
        os_free(str_no_idx);
    }
}

/** @brief Generate a string representaion the literal value of a label
 * in metadata terms.
 *
 * @param labelVal Specifies the kind and the value of the label
 * @return String representing the image of \b labelVal
 */
#define maxLabelNameSize 100

static c_char *
idl_valueFromLabelVal (
    idl_labelVal labelVal,
    SACSUnionMetaDescription *umd,
    c_bool customPSM)
{
    c_char *labelName;

    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
        labelName = idl_labelCsharpEnumVal(umd->unionSwitchType, idl_labelEnum(labelVal), customPSM);
    } else {
        labelName = os_malloc(maxLabelNameSize);
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
        case V_CHAR:
            snprintf(labelName, maxLabelNameSize, "%u",
                idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
        break;
        case V_SHORT:
            snprintf(labelName, maxLabelNameSize, "%d",
                idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
        break;
        case V_USHORT:
            snprintf(labelName, maxLabelNameSize, "%u",
                idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
        break;
        case V_LONG:
            snprintf(labelName, maxLabelNameSize, "%d",
                idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
        break;
        case V_ULONG:
            snprintf(labelName, maxLabelNameSize, "%u",
                idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
        break;
        case V_LONGLONG:
            snprintf(labelName, maxLabelNameSize, "%"PA_PRId64"",
                idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
        break;
        case V_ULONGLONG:
            snprintf(labelName, maxLabelNameSize, "%"PA_PRIu64"",
                idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
        break;
        case V_BOOLEAN:
            /* QAC EXPECT 3416; No side effect here */
        if (idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
            snprintf(labelName, maxLabelNameSize, "true");
        } else {
            snprintf(labelName, maxLabelNameSize, "false");
        }
        break;
        default:
        break;
        }
    }
    return labelName;
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
    char *unionTypeName, *discrTypeName;
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescription *umd = NULL;

    if (idl_definitionExists("definition", idl_scopeStack (scope, ".", name))) {
        return idl_abort;
    }
    idl_definitionAdd("definition", idl_scopeStack (scope, ".", name));

    unionTypeName = idl_CsharpId(name, csUserData->customPSM, FALSE);
    discrTypeName = idl_CsharpTypeFromTypeSpec(idl_typeUnionSwitchKind(unionSpec), csUserData->customPSM);

    umd = os_malloc(sizeof(SACSUnionMetaDescription));
    memset(umd, 0, sizeof(SACSUnionMetaDescription));
    umd->unionCaseMap = idl_mapNew(NULL, 1, 1);
    umd->unionCaseOffset = (idl_typeSpecDef(idl_typeUnionSwitchKind(unionSpec))->size > 4) ? 8 : 4;
    umd->unionSwitchType =  os_strdup(discrTypeName);
    umd->labelsUsedIter = os_iterNew(NULL);
    csUserData->typeStack = c_iterInsert(csUserData->typeStack, umd);

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %s\n", unionTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Sequential)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public sealed class %s\n", unionTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Generate the discriminator and its getter property. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "private %s _d;\n", discrTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "private _Fields _u;\n\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public %s Discriminator\n", discrTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    idl_printIndent(++indent_level);
    idl_fileOutPrintf(idl_fileCur(), "get { return _d; }\n");
    idl_printIndent(--indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    os_free(discrTypeName);
    os_free(unionTypeName);

    /* return idl_explore to indicate that the rest of the union needs to be processed */
    return idl_explore;
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
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescription *umd;
    char *unionTypeName;
    char *labelImage;
    char *tName;
    idl_mapIter mapIter;

    unionTypeName = idl_CsharpId(name, csUserData->customPSM, FALSE);
    umd = (SACSUnionMetaDescription *) c_iterTakeFirst(csUserData->typeStack);

    assert(umd->union1stCaseType);
    assert(umd->union1stCaseValue);
    assert(umd->union1stCaseIdent);

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public %s () {\n", unionTypeName);

    idl_printIndent(indent_level);
    if (idl_typeSpecType(umd->union1stCaseType) == idl_tbasic) {
        /* generate code for a standard empty ("") string.  */
        if (idl_typeBasicType(idl_typeBasic (umd->union1stCaseType)) == idl_string) {
            idl_fileOutPrintf(idl_fileCur(), "    _u.%s = \"\";\n", umd->union1stCaseIdent);
        } else if (idl_typeBasicType(idl_typeBasic (umd->union1stCaseType)) == idl_boolean) {
            idl_fileOutPrintf(idl_fileCur(), "    _u.%s = false;\n", umd->union1stCaseIdent);
        } else if (idl_typeBasicType(idl_typeBasic (umd->union1stCaseType)) == idl_char) {
            idl_fileOutPrintf(idl_fileCur(), "    _u.%s = '\\0';\n", umd->union1stCaseIdent);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "    _u.%s = 0;\n", umd->union1stCaseIdent);
        }
    } else if (idl_typeSpecType(umd->union1stCaseType) == idl_tseq ||
            idl_typeSpecType(umd->union1stCaseType) == idl_tarray) {
        char *str_idx = idl_arrayCsharpIndexString (umd->union1stCaseType, SACS_INCLUDE_INDEXES);
        assert (str_idx);

        tName = idl_CsharpTypeFromTypeSpec(umd->union1stCaseType, csUserData->customPSM);
        idl_fileOutPrintf(idl_fileCur(), "    _u.%s = new %s%s", umd->union1stCaseIdent, tName, str_idx);
        idl_fileOutPrintf(idl_fileCur(), ";\n");
        idl_arrayInitializeElements(umd->union1stCaseType, umd->union1stCaseIdent, csUserData->customPSM);
        os_free(tName);
        os_free(str_idx);
    } else if (idl_typeSpecType(umd->union1stCaseType) == idl_tstruct ||
            idl_typeSpecType(umd->union1stCaseType) == idl_tunion) {
        tName = idl_CsharpTypeFromTypeSpec(umd->union1stCaseType, csUserData->customPSM);
        idl_fileOutPrintf(idl_fileCur(), "    _u.%s = new %s();\n", umd->union1stCaseIdent, tName);
        os_free(tName);
    } else if (idl_typeSpecType(umd->union1stCaseType) == idl_tenum) {
        tName = idl_CsharpTypeFromTypeSpec(umd->union1stCaseType, csUserData->customPSM);
        idl_fileOutPrintf(idl_fileCur(), "    _u.%s = (%s)0;\n", umd->union1stCaseIdent, tName);
        os_free(tName);
    }

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "    _d = (%s)%s;\n", umd->unionSwitchType, umd->union1stCaseValue);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Sequential)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct _Fields\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    mapIter = idl_mapFirst(umd->unionCaseMap);
    while (idl_mapIterObject(mapIter)) {
        char *tName = idl_mapIterObject(mapIter);
        char *fName = idl_mapIterKey(mapIter);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    public %s %s;\n", tName, fName);
        idl_mapIterNext(mapIter);
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");

    idl_mapIterFree(mapIter);
    idl_mapFree(umd->unionCaseMap);

    indent_level--;
    idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "};\n");
    idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "#endregion\n\n");

    os_free(unionTypeName);
    os_free(umd->union1stCaseValue);
    os_free(umd->union1stCaseIdent);
    if (umd->unionSwitchType) {
        os_free(umd->unionSwitchType);
    }

    labelImage = os_iterTakeFirst(umd->labelsUsedIter);
    while (labelImage) {
        os_free(labelImage);
        labelImage = os_iterTakeFirst(umd->labelsUsedIter);
    }
    os_iterFree(umd->labelsUsedIter);
    os_free(umd);
}


/** @brief callback function called on definition of a union case in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
   =>           <union-case-1>;
            case label2.1; .. case label2.n;
   =>           ...        ...
            case labeln.1; .. case labeln.n;
   =>           <union-case-n>;
            default:
   =>           <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param name Name of the union case
 * @param typeSpec Specifies the type of the union case
 */
static void
idl_unionCaseOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescription *umd;
    char *caseTypename;
    char *caseField;
    c_ulong nrElements, i;
    char *labelImage;

    OS_UNUSED_ARG(scope);

    caseTypename = idl_unionCaseTypeFromTypeSpec(typeSpec, csUserData->customPSM);
    caseField = idl_CsharpId(name, csUserData->customPSM, FALSE);

    umd = (SACSUnionMetaDescription *) c_iterObject(csUserData->typeStack, 0);
    idl_mapAdd(umd->unionCaseMap, os_strdup(caseField), os_strdup(caseTypename));

    nrElements = os_iterLength(umd->labelIter);
    labelImage = os_iterObject(umd->labelIter, 0);

    if (!umd->union1stCaseType) {
        umd->union1stCaseType = typeSpec;
        umd->union1stCaseValue = os_strdup(os_iterObject(umd->labelIter, 0));
        umd->union1stCaseIdent = os_strdup(caseField);
    }

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public %s %s ()\n", caseTypename, caseField);

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level);
    if (umd->caseIsDefault == FALSE) {
        idl_fileOutPrintf(idl_fileCur(), "    if (_d != (%s)%s", umd->unionSwitchType, labelImage);
        for (i = 1; i < nrElements; i++) {
            labelImage = os_iterObject(umd->labelIter, i);
            idl_fileOutPrintf(idl_fileCur(), " &&\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "        _d != (%s)%s", umd->unionSwitchType, labelImage);
        }
        idl_fileOutPrintf(idl_fileCur(), ") {\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "        throw new System.InvalidOperationException();\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
    } else {
        if(os_iterLength(umd->labelsUsedIter)) {
            labelImage = os_iterObject(umd->labelsUsedIter, 0);
            idl_fileOutPrintf(idl_fileCur(), "    if (_d == (%s)%s", umd->unionSwitchType, labelImage);
                nrElements = os_iterLength(umd->labelsUsedIter);
            for (i = 1; i < nrElements; i++) {
                labelImage = os_iterObject(umd->labelsUsedIter, i);
                idl_fileOutPrintf(idl_fileCur(), " ||\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "        _d == (%s)%s", umd->unionSwitchType, labelImage);
            }
            idl_fileOutPrintf(idl_fileCur(), ") {\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "        throw new System.InvalidOperationException();\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
        }
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "    return _u.%s;\n", caseField);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public void %s (%s val)\n", caseField, caseTypename);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "    _u.%s = val;\n", caseField);
    labelImage = os_iterObject(umd->labelIter, 0);
    idl_printIndent(indent_level);
    assert(labelImage);
    idl_fileOutPrintf(idl_fileCur(), "    _d = (%s)%s;\n", umd->unionSwitchType, labelImage);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    if (nrElements > 1) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "public void %s (%s d, %s val)\n", caseField, umd->unionSwitchType, caseTypename);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_printIndent(indent_level);
        if (umd->caseIsDefault == FALSE) {
            labelImage = os_iterTakeFirst(umd->labelIter);
            idl_fileOutPrintf(idl_fileCur(), "    if (d != (%s)%s", umd->unionSwitchType, labelImage);
            os_free(labelImage);
            labelImage = os_iterTakeFirst(umd->labelIter);
            while (labelImage) {
                idl_fileOutPrintf(idl_fileCur(), " &&\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "        d != (%s)%s", umd->unionSwitchType, labelImage);
                os_free(labelImage);
                labelImage = os_iterTakeFirst (umd->labelIter);
            }
            idl_fileOutPrintf(idl_fileCur(), ") {\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "        throw new System.InvalidOperationException();\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
        } else {
            if(os_iterLength(umd->labelsUsedIter)) {
                labelImage = os_iterTakeFirst(umd->labelsUsedIter);
                idl_fileOutPrintf(idl_fileCur(), "    if (d == (%s)%s", umd->unionSwitchType, labelImage);
                os_free(labelImage);
                labelImage = os_iterTakeFirst(umd->labelsUsedIter);
                while (labelImage) {
                    idl_fileOutPrintf(idl_fileCur(), " ||\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "        d == (%s)%s", umd->unionSwitchType, labelImage);
                    os_free(labelImage);
                    labelImage = os_iterTakeFirst(umd->labelsUsedIter);
                }
                idl_fileOutPrintf(idl_fileCur(), ") {\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "        throw new System.InvalidOperationException();\n");
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "    }\n");
            }
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    _u.%s = val;\n", caseField);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    _d = d;\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    }

    os_free(caseTypename);
    os_free(caseField);

    labelImage = os_iterTakeFirst(umd->labelIter);
    while (labelImage) {
        os_free(labelImage);
        labelImage = os_iterTakeFirst(umd->labelIter);
    }
    os_iterFree(umd->labelIter);
    umd->labelIter = NULL;
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelOpenClose(
    idl_scope ownScope,
    idl_labelVal labelVal,
    void *userData)
{
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescription *umd;

    OS_UNUSED_ARG(ownScope);

    umd = (SACSUnionMetaDescription *) c_iterObject(csUserData->typeStack, 0);
    if (idl_labelValType(labelVal) != idl_ldefault) {
        umd->caseIsDefault = FALSE;
        umd->labelIter = os_iterAppend(umd->labelIter, idl_valueFromLabelVal(labelVal, umd, csUserData->customPSM));
        umd->labelsUsedIter = os_iterAppend(umd->labelsUsedIter, idl_valueFromLabelVal(labelVal, umd, csUserData->customPSM));
    } else {
        umd->caseIsDefault = TRUE;
        umd->labelIter = os_iterAppend(umd->labelIter,
                idl_valueFromLabelVal(idl_labelDefaultAlternative(idl_labelDefault(labelVal)), umd, csUserData->customPSM));
    }
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelsOpenClose(
    idl_scope scope,
    idl_labelSpec labelSpec,
    void *userData)
{
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescription *umd;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(labelSpec);

    umd = (SACSUnionMetaDescription *) c_iterObject(csUserData->typeStack, 0);
    umd->labelIter = os_iterNew(NULL);
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
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    SACSUnionMetaDescription *umd;
    char *caseTypename;
    char *labelImage;

    OS_UNUSED_ARG(scope);

    umd = (SACSUnionMetaDescription *) c_iterObject(csUserData->typeStack, 0);
    caseTypename = idl_unionCaseTypeFromTypeSpec(typeSpec, csUserData->customPSM);

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public void Default ()\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "    _d = (%s)%s;\n", umd->unionSwitchType,
            idl_valueFromLabelVal(labelVal, umd, csUserData->customPSM));
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public void Default (%s d)\n", caseTypename);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    idl_printIndent(indent_level);
    if(os_iterLength(umd->labelsUsedIter)) {
        labelImage = os_iterTakeFirst(umd->labelsUsedIter);
        idl_fileOutPrintf(idl_fileCur(), "    if (d == (%s)%s", umd->unionSwitchType, labelImage);
        labelImage = os_iterTakeFirst(umd->labelsUsedIter);
        while (labelImage) {
            idl_fileOutPrintf(idl_fileCur(), " ||\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "        d == (%s)%s", umd->unionSwitchType, labelImage);
            labelImage = os_iterTakeFirst(umd->labelsUsedIter);
        }
        idl_fileOutPrintf(idl_fileCur(), ") {\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "        throw new System.InvalidOperationException();\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "    _d = d;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    os_free(caseTypename);
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
    char *enumName;
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;

    if (idl_definitionExists("definition", idl_scopeStack(scope, ".", name))) {
        return idl_abort;
    }
    idl_definitionAdd("definition", idl_scopeStack(scope, ".", name));

    enumName = idl_CsharpId(name, csUserData->customPSM, FALSE);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %s\n", enumName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public enum %s\n", enumName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    enum_element = idl_typeEnumNoElements(enumSpec);

    /* Hold on to the name of the enum, until we looped though all the labels. */
    enum_enumName = os_strdup(name);

    indent_level++;
    os_free(enumName);

    /* return idl_explore to indicate that the rest of the enumeration needs to be processed */
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
idl_enumerationClose (
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "#endregion\n\n");

    /* Release the enum name since we no longer need to reference it. */
    os_free(enum_enumName);
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
    char *nameCopy, *labelName;
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    OS_UNUSED_ARG(scope);

    OS_UNUSED_ARG(scope);

    /* If the element if prefixed with the name of the enum, remove the prefix. */
    nameCopy = os_strdup(name);
    idl_CsharpRemovePrefix(enum_enumName, nameCopy);

    /* Translate the remaining label into its C# representation. */
    labelName = idl_CsharpId(nameCopy, csUserData->customPSM, FALSE);
    enum_element--;
    if (enum_element == 0) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s\n", labelName);
    } else {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s,\n", labelName);
    }
    os_free(labelName);
    os_free(nameCopy);
}


static void
idl_constantOpenClose (
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    SACSTypeUserData* csUserData = (SACSTypeUserData *) userData;
    char *constName = idl_constSpecName(constantSpec);
    char *constTypeName = idl_CsharpTypeFromTypeSpec(
            idl_constSpecTypeGet(constantSpec),
            csUserData->customPSM);
    char *constSpecImage = idl_constSpecImage(constantSpec);

    OS_UNUSED_ARG(scope);

    idl_printIndent(indent_level);
    idl_fileOutPrintf(
            idl_fileCur(),
            "public struct %s { public static readonly %s value = (%s) %s; }\n\n",
            constName,
            constTypeName,
            constTypeName,
            constSpecImage);

    os_free(constSpecImage);
    os_free(constTypeName);
    os_free(constName);
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSACSLoadControl = {
    idl_inline
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
static struct idl_program idl_genSACSType;

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSACSTypeProgram(
    SACSTypeUserData *userData)
{
    idl_genSACSType.idl_getControl                  = idl_getControl;
    idl_genSACSType.fileOpen                        = idl_fileOpen;
    idl_genSACSType.fileClose                       = NULL;
    idl_genSACSType.moduleOpen                      = idl_moduleOpen;
    idl_genSACSType.moduleClose                     = idl_moduleClose;
    idl_genSACSType.structureOpen                   = idl_structureOpen;
    idl_genSACSType.structureClose                  = idl_structureClose;
    idl_genSACSType.structureMemberOpenClose        = idl_structureMemberOpenClose;
    idl_genSACSType.enumerationOpen                 = idl_enumerationOpen;
    idl_genSACSType.enumerationClose                = idl_enumerationClose;
    idl_genSACSType.enumerationElementOpenClose     = idl_enumerationElementOpenClose;
    idl_genSACSType.unionOpen                       = idl_unionOpen;
    idl_genSACSType.unionClose                      = idl_unionClose;
    idl_genSACSType.unionCaseOpenClose              = idl_unionCaseOpenClose;
    idl_genSACSType.unionLabelsOpenClose            = idl_unionLabelsOpenClose;
    idl_genSACSType.unionLabelOpenClose             = idl_unionLabelOpenClose;
    idl_genSACSType.typedefOpenClose                = NULL;
    idl_genSACSType.boundedStringOpenClose          = NULL;
    idl_genSACSType.sequenceOpenClose               = NULL;
    idl_genSACSType.constantOpenClose               = idl_constantOpenClose;
    idl_genSACSType.artificialDefaultLabelOpenClose = idl_artificialDefaultLabelOpenClose;
    idl_genSACSType.userData                        = userData;

    return &idl_genSACSType;
}
