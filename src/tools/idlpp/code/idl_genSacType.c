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
 * This module generates Standalone C data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSacType.h"
#include "idl_genSacHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genCHelper.h"
#include "idl_genFileHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_dll.h"
#include "idl_keyDef.h"

#include "vortex_os.h"
#include <ctype.h>
#include "c_typebase.h"

/** indentation level */
static c_long indent_level = 0;
	/** enumeration element index */
static c_ulong enum_element = 0;
static char *enum_def = NULL;

static void idl_arrayDimensions(idl_typeArray typeArray);

/** @brief generate name which will be used as a macro to prevent multiple inclusions
 *
 * From the specified scope create a macro which will
 * be used to prevent multiple inclusions of the generated
 * header file. The scope file or basename characters are
 * translated into uppercase characters and the append string is
 * appended to the macro.
 */
static c_char *
idl_macroFromBasename(
    idl_scope scope,
    const char *append)
{
    return idl_genIncludeGuardFromFilename(scope, append);
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
    c_ulong i;
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    /* Generate multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s\n", idl_macroFromBasename(scope, "DCPS_H"));
    idl_fileOutPrintf(idl_fileCur(), "#define %s\n", idl_macroFromBasename(scope, "DCPS_H"));
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* Generate inclusion of standard OpenSpliceDDS type definition files */
    idl_fileOutPrintf(idl_fileCur(), "#include <dds_primitive_types.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* Generate code for inclusion of application specific include files */
    for (i = 0; i < idl_depLength(idl_depDefGet()); i++) {
        idl_fileOutPrintf(idl_fileCur(), "#include \"%sDcps.h\"\n", idl_depGet(idl_depDefGet(), i));
    }
    if (idl_depLength(idl_depDefGet()) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* Setup dll stuff */
    idl_fileOutPrintf(idl_fileCur(), "%s\n", idl_dllGetHeader());
    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/** @brief callback function called on closing the IDL input file.
 *
 * Generate standard file footer consisting of:
 * - mutiple inclusion prevention closure
 */
static void
idl_fileClose (
    void *userData)
{
    OS_UNUSED_ARG(userData);
    /* Generate closure of multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");
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
    c_char *scopedName = idl_scopeStack(scope, "_", name);

    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    if (idl_definitionExists("definition", scopedName)) {
        return idl_abort;
    }

    idl_definitionAdd("definition", scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_defined\n",
	scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#define _%s_defined\n",
	scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#ifdef __cplusplus\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct %s;\n", scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#else /* __cplusplus */\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "typedef struct %s %s;\n",
        scopedName,
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#endif /* __cplusplus */\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_defined */\n",scopedName);
    /* define the prototype of the function allocate the structure */
    idl_fileOutPrintf(
        idl_fileCur(),
        "%s %s *%s__alloc (void);\n\n",
        idl_dllGetMacro(),
        scopedName,
        scopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct %s {\n",scopedName);
    os_free(scopedName);
    indent_level++;

    /* return idl_explore to indicate that the rest of the structure needs to be processed */
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
idl_structureClose (
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    indent_level--;
    idl_printIndent(indent_level); idl_fileOutPrintf (idl_fileCur(), "};\n\n");
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
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    if (idl_typeSpecType(typeSpec) == idl_tbasic ||
        idl_typeSpecType(typeSpec) == idl_ttypedef ||
        idl_typeSpecType(typeSpec) == idl_tenum ||
        idl_typeSpecType(typeSpec) == idl_tstruct ||
        idl_typeSpecType(typeSpec) == idl_tunion) {
        /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s;\n",
            idl_scopedSacTypeIdent(typeSpec),
            idl_languageId(name));
    } else {
        if (idl_typeSpecType(typeSpec) == idl_tarray) {
            /* generate code for an array mapping */
            idl_printIndent(indent_level);
            if (idl_typeSpecType(idl_typeArrayActual (idl_typeArray(typeSpec))) != idl_tseq) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s",
                    idl_scopedSacTypeIdent(idl_typeArrayActual(idl_typeArray(typeSpec))),
                    idl_languageId (name));
            } else {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s",
                    idl_sequenceIdent(idl_typeSeq(idl_typeArrayActual(idl_typeArray(typeSpec)))),
                    idl_languageId (name));
            }
            idl_arrayDimensions(idl_typeArray(typeSpec));
            idl_fileOutPrintf (idl_fileCur(), ";\n");
        } else {
            if (idl_typeSpecType(typeSpec) == idl_tseq) {
                /* generate code for a sequence mapping */
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s;\n",
                    idl_sequenceIdent(idl_typeSeq(typeSpec)),
                    idl_languageId (name));
            } else {
                printf("idl_structureMemberOpenClose: Unsupported structure member type (member name = %s, type name = %s)\n",
                    name, idl_scopedTypeName(typeSpec));
            }
        }
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
    c_char *scopedName = idl_scopeStack(scope, "_", name);

    OS_UNUSED_ARG(userData);

    if (idl_definitionExists("definition", scopedName)) {
        os_free(scopedName);
        return idl_abort;
    }
    idl_definitionAdd("definition", scopedName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "#ifndef _%s_defined\n",
        scopedName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "#define _%s_defined\n",
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#ifdef __cplusplus\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),
        "struct %s;\n",
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#else /* __cplusplus */\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "typedef struct %s %s;\n",
        scopedName,
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#endif /* __cplusplus */\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "#endif /* _%s_defined */\n",
        scopedName);
    /* define the prototype of the function allocate the union */
    idl_fileOutPrintf(
        idl_fileCur(),
        "%s *%s__alloc (void);\n",
        scopedName,
        scopedName);
    /* open the struct */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "struct %s {\n",
        scopedName);
    os_free(scopedName);
    indent_level++;
    /* generate code for the switch */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
        idl_printIndent(indent_level);
         idl_fileOutPrintf(
            idl_fileCur(),
            "%s _d;\n",
            idl_scopedSacTypeIdent(idl_typeUnionSwitchKind(unionSpec)));
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tenum) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s _d;\n",
            idl_scopedSacTypeIdent(idl_typeUnionSwitchKind(unionSpec)));
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) {
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec))))) {
        case idl_tbasic:
        case idl_tenum:
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s _d;\n",
                idl_scopedSacTypeIdent(idl_typeUnionSwitchKind(unionSpec)));
	    break;
        default:
            printf ("idl_unionOpen: Unsupported switckind\n");
        }
    } else {
        printf ("idl_unionOpen: Unsupported switckind\n");
    }
    /* open the union */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "union {\n");
    indent_level++;

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
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

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
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    if (idl_typeSpecType(typeSpec) == idl_tbasic ||
        idl_typeSpecType(typeSpec) == idl_ttypedef ||
        idl_typeSpecType(typeSpec) == idl_tenum ||
        idl_typeSpecType(typeSpec) == idl_tstruct ||
        idl_typeSpecType(typeSpec) == idl_tunion) {
        /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s;\n",
            idl_scopedSacTypeIdent(typeSpec),
            idl_languageId(name));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        /* generate code for an array mapping */
        idl_printIndent(indent_level);
        if (idl_typeSpecType(idl_typeArrayActual (idl_typeArray(typeSpec))) != idl_tseq) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s",
                idl_scopedSacTypeIdent(idl_typeArrayActual(idl_typeArray(typeSpec))),
                idl_languageId (name));
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s",
                idl_sequenceIdent(idl_typeSeq(idl_typeArrayActual(idl_typeArray(typeSpec)))),
                idl_languageId (name));
        }
        idl_arrayDimensions(idl_typeArray(typeSpec));
        idl_fileOutPrintf(idl_fileCur(), ";\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        /* generate code for a sequence mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s;\n",
            idl_sequenceIdent(idl_typeSeq(typeSpec)),
            idl_languageId(name));
    } else {
        printf("idl_unionCaseOpenClose: Unsupported union case type (case name = %s, type = %s)\n",
            name, idl_scopedTypeName(typeSpec));
    }
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
    c_char *scopedName = idl_scopeStack(scope, "_", name);

    OS_UNUSED_ARG(userData);

    if (idl_definitionExists("definition", scopedName)) {
        os_free(scopedName);
        return idl_abort;
    }
    idl_definitionAdd("definition", scopedName);
    if (enum_def == NULL) {
        enum_def = os_malloc(512);
    }
    snprintf(enum_def, 512,	"typedef enum %s_e %s;",
	   scopedName, scopedName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "enum %s_e {\n", scopedName);
    enum_element = idl_typeEnumNoElements(enumSpec);
    os_free(scopedName);
    indent_level++;

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
    idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "};\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s\n\n", enum_def);
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
    OS_UNUSED_ARG(userData);

    enum_element--;
    if (enum_element == 0) {
        idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "%s\n",
	    idl_scopeStack(scope, "_", name));
    } else {
        idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "%s,\n",
	    idl_scopeStack(scope, "_", name));
    }
}

/** @brief generate dimension of an array
 *
 * arrayDimensions is a local support function to generate
 * the array dimensions of an array
 *
 * @param typeArray Specifies the type of the array
 */
static void
idl_arrayDimensions(
    idl_typeArray typeArray)
{
    idl_fileOutPrintf(idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        idl_arrayDimensions(idl_typeArray(idl_typeArrayType(typeArray)));
    }
}

/** @brief generate dimension of an array slice
 *
 * arraySliceDimensions is a local support function to generate
 * the array dimensions of an array slice
 *
 * @param typeArray Specifies the type of the array
 */
static void
idl_arraySliceDimensions(
    idl_typeArray typeArray)
{
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray &&
        idl_typeSpecType(idl_typeArrayType(idl_typeArray(idl_typeArrayType(typeArray)))) == idl_tarray) {
        idl_arraySliceDimensions(idl_typeArray(idl_typeArrayType(typeArray)));
    }
    idl_fileOutPrintf(idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
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
    c_char *scopedName = idl_scopeStack(scope, "_", name);

    if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tbasic) {
        /* generate code for a standard mapping or a basic type mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "typedef %s %s;\n",
            idl_scopedSacTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec))),
            scopedName);
        /* define the prototype of the function allocate the typedefed type */
        if (idl_typeBasicType(idl_typeBasic(idl_typeDefRefered(defSpec))) == idl_string) {
            /* if it is a string, then accept a length */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s *%s__alloc (DDS_unsigned_long len);\n\n",
                idl_dllGetMacro(),
                scopedName,
                scopedName);
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s *%s__alloc (void);\n\n",
                idl_dllGetMacro(),
                scopedName,
                scopedName);
        }
    } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_ttypedef) {
        idl_typedefOpenClose(scope, name, idl_typeDef(idl_typeDefRefered (defSpec)), userData);
    } else {
        if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tenum) ||
            (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct) ||
            (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion)) {
            /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef %s %s;\n",
                idl_scopedSacTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec))),
                scopedName);
            /* define the prototype of the function allocate the typedefed type */
            /* if it is a string, then accept a length */
            idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void);\n\n", scopedName, scopedName);
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tarray) {
            /* generate code for an array mapping */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef %s %s",
                idl_scopedSacTypeIdent(idl_typeArrayActual (idl_typeArray(idl_typeDefRefered(defSpec)))),
                scopedName);
            idl_arrayDimensions(idl_typeArray(idl_typeDefRefered(defSpec)));
            idl_fileOutPrintf(idl_fileCur(), ";\n");
            /* define the prototype of the function allocate the typedefed type */
            if (idl_typeSpecType(idl_typeArrayType(idl_typeArray(idl_typeDefRefered(defSpec)))) == idl_tarray) {
                /* This is a multi dimensional array, therefore define a slice */
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "typedef %s %s_slice",
                    idl_scopedSacTypeIdent(idl_typeArrayActual (idl_typeArray(idl_typeDefRefered(defSpec)))),
                    scopedName);
                idl_arraySliceDimensions(idl_typeArray(idl_typeDefRefered(defSpec)));
                idl_fileOutPrintf(idl_fileCur(), ";\n");
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s_slice *%s__alloc (void);\n\n",
                    idl_dllGetMacro(),
                    scopedName,
                    scopedName);
            } else {
                /* This is a one dimensional array, nevertheless define a slice */
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "typedef %s %s_slice;\n",
                    idl_scopedSacTypeIdent(idl_typeArrayActual (idl_typeArray(idl_typeDefRefered(defSpec)))),
                    scopedName);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s_slice *%s__alloc (void);\n\n",
                    idl_dllGetMacro(),
                    scopedName,
                    scopedName);
            }
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
            char *sequenceElementIdent = idl_sequenceElementIdent(idl_typeSeqType(idl_typeSeq(idl_typeDefRefered(defSpec))));
            char *sequenceIdent = idl_sequenceIdent(idl_typeSeq(idl_typeDefRefered(defSpec)));
            /* generate code for a sequence mapping */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef %s %s;\n",
                sequenceIdent,
                scopedName);
            /* define the prototype of the function allocate the typedefed type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s *%s__alloc (void);\n",
                idl_dllGetMacro(),
                scopedName,
                scopedName);
            /* define the prototype of the function allocate the sequence buffer */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s *%s_allocbuf (DDS_unsigned_long len);\n\n",
                idl_dllGetMacro(),
                sequenceElementIdent,
                scopedName);
            os_free(sequenceElementIdent);
            os_free(sequenceIdent); 
        } else {
            printf ("idl_typedefOpenClose: Unsupported typedef type (typename = %s, type = %s)\n",
                name, idl_scopedTypeName(idl_typeSpec(defSpec)));
        }
    }
    os_free(scopedName);
}

static void
idl_sequenceOpenClose(
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    c_bool prototypeExists = FALSE;
    char *sequenceElementName;
    char *sequenceName;
    char * sequenceScopedName;

    OS_UNUSED_ARG(userData);

    sequenceElementName = idl_sequenceElementIdent(idl_typeSeqType(typeSeq));
    sequenceName = idl_sequenceIdent(typeSeq);

    if (idl_definitionExists("definition", sequenceName)) {
        prototypeExists = TRUE;
    } else {
        idl_definitionAdd("definition", sequenceName);
        if ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tbasic) &&
             ((idl_typeBasicType(idl_typeBasic(idl_typeSeqType(typeSeq))) == idl_octet) ||
              (idl_typeBasicType(idl_typeBasic(idl_typeSeqType(typeSeq))) == idl_string))) {
            prototypeExists = TRUE;
        }
    }

    if (!prototypeExists) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "/* Definition for sequence of %s */\n",
            sequenceElementName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "#ifndef _%s_defined\n",
            sequenceName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "#define _%s_defined\n",
            sequenceName);
        if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tstruct) {
            /* In case struct is refered, it can be a sequence definition within	*/
            /* within the structure with a reference to the struct itself. In that	*/
            /* case the struct or union is not defined yet, and a forward 		*/
            /* definition is required.						*/
            idl_fileOutPrintf(
                idl_fileCur(),
                "#ifndef _%s_defined\n",
                sequenceElementName);
            idl_fileOutPrintf(
                idl_fileCur(),
                "#define _%s_defined\n",
                sequenceElementName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef struct %s %s;\n",
                sequenceElementName,
                sequenceElementName);
            idl_fileOutPrintf(
                idl_fileCur(),
                "#endif /* _%s_defined */\n",
                sequenceElementName);
        } else if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tunion) {
            /* In case struct is refered, it can be a sequence definition within	*/
            /* within the structure with a reference to the struct itself. In that	*/
            /* case the struct or union is not defined yet, and a forward 		*/
            /* definition is required.						*/
            idl_fileOutPrintf(
                idl_fileCur(),
                "#ifndef _%s_defined\n",
                sequenceElementName);
            idl_fileOutPrintf(
                idl_fileCur(),
                "#define _%s_defined\n",
                sequenceElementName);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef struct %s %s;\n",
                sequenceElementName,
                sequenceElementName);
            idl_fileOutPrintf(idl_fileCur(),
                "#endif /* _%s_defined */\n",
                sequenceElementName);
        }
        idl_fileOutPrintf(idl_fileCur(), "typedef struct {\n");
        idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long _maximum;\n");
        idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long _length;\n");
        idl_fileOutPrintf(idl_fileCur(), "    %s *_buffer;\n", sequenceElementName);
        idl_fileOutPrintf(idl_fileCur(), "    DDS_boolean _release;\n");
        idl_fileOutPrintf(idl_fileCur(), "} %s;\n", sequenceName);

        idl_fileOutPrintf(idl_fileCur(),
                "%s %s *%s__alloc (void);\n",
                idl_dllGetMacro(),
                sequenceName,
                sequenceName);
        idl_fileOutPrintf(idl_fileCur(),
                "%s %s *%s_allocbuf (DDS_unsigned_long len);\n",
                idl_dllGetMacro(),
                sequenceElementName,
                sequenceName);

        idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_defined */\n", sequenceName);
    }
    sequenceScopedName = idl_sequenceIdentScoped(scope, typeSeq);

    idl_fileOutPrintf(idl_fileCur(),
            "#define %s__alloc %s__alloc\n",
            sequenceScopedName,
            sequenceName);
    idl_fileOutPrintf(idl_fileCur(),
            "#define %s_allocbuf %s_allocbuf\n\n",
            sequenceScopedName,
            sequenceName);

    os_free(sequenceScopedName);
    os_free(sequenceElementName);
    os_free(sequenceName);
}

static void
idl_constantOpenClose (
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "#define %-30s %s\n\n",
	idl_scopeStack(scope, "_", idl_constSpecName(constantSpec)),
	idl_constSpecImage(constantSpec));
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSacLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genSacLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSacType = {
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
    idl_unionOpen,
    idl_unionClose,
    idl_unionCaseOpenClose,
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    idl_sequenceOpenClose,
    idl_constantOpenClose,
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSacTypeProgram(
    void)
{
    return &idl_genSacType;
}
