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

#include "os.h"
#include <ctype.h>
#include "c_typebase.h"

    /** indentation level */
static c_long indent_level = 0;
    /** enumeration element index */
static c_long enum_element = 0;
    /** enumeration enum name */
static char *enum_enumName = NULL;

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

    if (idl_definitionExists("definition", idl_scopeStack (scope, ".", name))) {
        return idl_abort;
    }
    idl_definitionAdd("definition", idl_scopeStack (scope, ".", name));

    unionTypeName = idl_CsharpId(name, csUserData->customPSM, FALSE);
    discrTypeName = idl_CsharpTypeFromTypeSpec(
            idl_typeUnionSwitchKind(unionSpec),
            csUserData->customPSM);

    /* Generate the C# code that opens a sealed class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "#region %s\n", unionTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "[StructLayout(LayoutKind.Explicit)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public sealed struct %s\n", unionTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;

    /* Generate the discriminator and its getter property. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),"[FieldOffset(0)]\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "private %s _d;\n",
        discrTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "public %s Discriminator\n",
        discrTypeName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    idl_printIndent(++indent_level);
    idl_fileOutPrintf(idl_fileCur(), "get { return _d; }\n");
    idl_printIndent(--indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n");

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
    indent_level--;
    idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "} _u;\n");
    indent_level--;
    idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "};\n\n");
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
    char *constTypeName = idl_CsharpTypeFromTypeSpec(
            idl_constSpecTypeGet(constantSpec),
            csUserData->customPSM);

    idl_printIndent(indent_level);
    idl_fileOutPrintf(
            idl_fileCur(),
            "public struct %s { public static readonly %s value = %s; }\n\n",
            idl_constSpecName(constantSpec),
            constTypeName,
            idl_constSpecImage(constantSpec));
    os_free(constTypeName);
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
    idl_genSACSType.unionLabelsOpenClose            = NULL;
    idl_genSACSType.unionLabelOpenClose             = NULL;
    idl_genSACSType.typedefOpenClose                = NULL;
    idl_genSACSType.boundedStringOpenClose          = NULL;
    idl_genSACSType.sequenceOpenClose               = NULL;
    idl_genSACSType.constantOpenClose               = idl_constantOpenClose;
    idl_genSACSType.artificialDefaultLabelOpenClose = NULL;
    idl_genSACSType.userData                        = userData;

    return &idl_genSACSType;
}
