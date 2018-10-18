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
#include "idl_genC99Type.h"
#include "idl_genC99Helper.h"
#include "idl_genSplHelper.h"
#include "idl_genCHelper.h"
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
 * From the specified basename create a macro which will
 * be used to prevent multiple inclusions of the generated
 * header file. The basename characters are translated
 * into uppercase characters and the append string is
 * appended to the macro.
 */
static c_char *
idl_macroFromBasename(
    const char *basename,
    const char *append)
{
    static c_char macro[200];
    size_t i;

    for (i = 0; i < strlen(basename); i++) {
        macro[i] = (c_char) toupper(basename[i]);
        macro[i+1] = '\0';
    }
    os_strncat(macro, append, sizeof(macro)-strlen(append));

    return macro;
}

char *
idl_memberDeclaration(
    const idl_typeSpec typeSpec,
    const char *name)
{
    size_t len;
    c_char *result;
    char *stname = idl_scopedC99TypeIdent(typeSpec);

    if ((idl_typeSpecType(typeSpec) == idl_tbasic) && (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string)) {
        c_ulong maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
        if (maxlen > 0) {
            len = strlen(name) + 32;
            result = os_malloc(len);
            snprintf(result, len, "char %s[%d]", name, maxlen + 1);
        } else {
            len = strlen(name) + strlen(stname) + 2;
            result = os_malloc(len);
            snprintf(result, len, "%s %s", stname, name);
        }
    } else {
        len = strlen(name) + strlen(stname) + 2;
        result = os_malloc(len);
        snprintf(result, len, "%s %s", stname, name);
    }
    os_free(stname);

    return result;
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
    c_ulong i;
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* Generate multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s\n", idl_macroFromBasename(name, "DCPS_H"));
    idl_fileOutPrintf(idl_fileCur(), "#define %s\n", idl_macroFromBasename(name, "DCPS_H"));
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* Generate inclusion of standard OpenSpliceDDS type definition files */
    idl_fileOutPrintf(idl_fileCur(), "#include <dds_primitive_types.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <stdint.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <stdbool.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");

    idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n");
    idl_fileOutPrintf(idl_fileCur(), "extern \"C\" {\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");

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

    idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");

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
    char *id;
    char *member;
    char *sequenceIdent;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    if (idl_typeSpecType(typeSpec) == idl_tbasic ||
        idl_typeSpecType(typeSpec) == idl_ttypedef ||
        idl_typeSpecType(typeSpec) == idl_tenum ||
        idl_typeSpecType(typeSpec) == idl_tstruct ||
        idl_typeSpecType(typeSpec) == idl_tunion) {
        /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */

        id = idl_languageId(name);
        member = idl_memberDeclaration(typeSpec, id);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s;\n", member);
        os_free(id);
        os_free(member);
    } else {
        if (idl_typeSpecType(typeSpec) == idl_tarray) {
            /* generate code for an array mapping */
            idl_printIndent(indent_level);
            if (idl_typeSpecType(idl_typeArrayActual (idl_typeArray(typeSpec))) != idl_tseq) {
                id = idl_languageId(name);
                member = idl_memberDeclaration(idl_typeArrayActual(idl_typeArray(typeSpec)), id);
                idl_fileOutPrintf(idl_fileCur(), "%s", member);
                os_free(id);
                os_free(member);
            } else {
                sequenceIdent = idl_c99SequenceIdent(idl_typeSeq(idl_typeArrayActual(idl_typeArray(typeSpec))));
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s",
                    sequenceIdent,
                    idl_languageId (name));
                os_free(sequenceIdent);
            }
            idl_arrayDimensions(idl_typeArray(typeSpec));
            idl_fileOutPrintf (idl_fileCur(), ";\n");
        } else {
            if (idl_typeSpecType(typeSpec) == idl_tseq) {
                /* generate code for a sequence mapping */
                sequenceIdent = idl_c99SequenceIdent(idl_typeSeq(typeSpec));
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s;\n",
                    sequenceIdent,
                    idl_languageId (name));
                os_free(sequenceIdent);
            } else {
                char *scopedTypeName = idl_scopedTypeName(typeSpec);
                printf("idl_structureMemberOpenClose: Unsupported structure member type (member name = %s, type name = %s)\n",
                    name, scopedTypeName);
                os_free(scopedTypeName);
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
    c_char *scopedTypeIdent;

    OS_UNUSED_ARG(userData);

    if (idl_definitionExists("definition", scopedName)) {
        os_free(scopedName);
        return idl_abort;
    }
    idl_definitionAdd("definition", scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_defined\n",
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#define _%s_defined\n",
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#ifdef __cplusplus\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(),"struct %s;\n",
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#else /* __cplusplus */\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "typedef struct %s %s;\n",
        scopedName,
        scopedName);
    idl_fileOutPrintf(idl_fileCur(), "#endif /* __cplusplus */\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_defined */\n",
        scopedName);
    /* define the prototype of the function allocate the union */
    idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void);\n",
        scopedName,
        scopedName);
    /* open the struct */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct %s {\n",
        scopedName);
    os_free(scopedName);
    indent_level++;
    /* generate code for the switch */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
        scopedTypeIdent = idl_scopedC99TypeIdent(idl_typeUnionSwitchKind(unionSpec));
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s _d;\n",
                scopedTypeIdent);
        os_free(scopedTypeIdent);
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tenum) {
        scopedTypeIdent = idl_scopedC99TypeIdent(idl_typeUnionSwitchKind(unionSpec));
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s _d;\n",
                scopedTypeIdent);
        os_free(scopedTypeIdent);
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) {
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec))))) {
        case idl_tbasic:
        case idl_tenum:
            scopedTypeIdent = idl_scopedC99TypeIdent(idl_typeUnionSwitchKind(unionSpec));
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "%s _d;\n",
                    scopedTypeIdent);
            os_free(scopedTypeIdent);
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
    char *id;
    char *member;
    c_char *sequenceIdent;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    if (idl_typeSpecType(typeSpec) == idl_tbasic ||
        idl_typeSpecType(typeSpec) == idl_ttypedef ||
        idl_typeSpecType(typeSpec) == idl_tenum ||
        idl_typeSpecType(typeSpec) == idl_tstruct ||
        idl_typeSpecType(typeSpec) == idl_tunion) {
        /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */

         id = idl_languageId(name);
         member = idl_memberDeclaration(typeSpec, id);
         idl_printIndent(indent_level);
         idl_fileOutPrintf(idl_fileCur(), "%s;\n", member);
         os_free(id);
         os_free(member);
#if 0
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s;\n",
            idl_scopedC99TypeIdent(typeSpec),
            idl_languageId(name));
#endif
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        /* generate code for an array mapping */
        idl_printIndent(indent_level);
        if (idl_typeSpecType(idl_typeArrayActual (idl_typeArray(typeSpec))) != idl_tseq) {
            id = idl_languageId(name);
            member = idl_memberDeclaration(idl_typeArrayActual(idl_typeArray(typeSpec)), id);
            idl_fileOutPrintf(idl_fileCur(), "%s", member);
            os_free(id);
            os_free(member);
#if 0
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s",
                idl_scopedC99TypeIdent(idl_typeArrayActual(idl_typeArray(typeSpec))),
                idl_languageId (name));
#endif
        } else {
            sequenceIdent = idl_c99SequenceIdent(idl_typeSeq(idl_typeArrayActual(idl_typeArray(typeSpec))));
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s",
                sequenceIdent,
                idl_languageId (name));
            os_free(sequenceIdent);
        }
        idl_arrayDimensions(idl_typeArray(typeSpec));
        idl_fileOutPrintf(idl_fileCur(), ";\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        /* generate code for a sequence mapping */
        sequenceIdent = idl_c99SequenceIdent(idl_typeSeq(typeSpec));
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s;\n",
            sequenceIdent,
            idl_languageId(name));
        os_free(sequenceIdent);
    } else {
        c_char *scopedTypeName = idl_scopedTypeName(typeSpec);
        printf("idl_unionCaseOpenClose: Unsupported union case type (case name = %s, type = %s)\n",
            name, scopedTypeName);
        os_free(scopedTypeName);
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

/* @brief generate dimension of an array
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

/* @brief generate dimension of an array slice
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
    c_char *scopedTypeIdent;
    char *member;
    c_ulong maxlen;

    if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tbasic) {
        /* generate code for a standard mapping or a basic type mapping */
        member = idl_memberDeclaration(idl_typeSpec(idl_typeDefRefered(defSpec)), scopedName);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "typedef %s;\n", member);
        os_free(member);

        /* define the prototype of the function allocate the typedefed type */
        if (idl_typeBasicType(idl_typeBasic(idl_typeDefRefered(defSpec))) == idl_string) {
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefRefered(defSpec)));
            if (maxlen > 0) {
                idl_fileOutPrintf(idl_fileCur(), "%s %s *%s__alloc (void);\n\n",
                     idl_dllGetMacro(),
                     scopedName,
                     scopedName);
            } else {
                /* if it is a string, then accept a length */
                idl_fileOutPrintf(idl_fileCur(), "%s %s *%s__alloc (uint32_t len);\n\n",
                        idl_dllGetMacro(),
                        scopedName,
                        scopedName);
            }
        } else {
            idl_fileOutPrintf(idl_fileCur(), "%s %s *%s__alloc (void);\n\n",
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
            scopedTypeIdent = idl_scopedC99TypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec)));
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "typedef %s %s;\n",
                scopedTypeIdent,
                scopedName);
            os_free(scopedTypeIdent);
            /* define the prototype of the function allocate the typedefed type */
            /* if it is a string, then accept a length */
            idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void);\n\n", scopedName, scopedName);
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tarray) {
            /* generate code for an array mapping */
            scopedTypeIdent = idl_scopedC99TypeIdent(idl_typeArrayActual (idl_typeArray(idl_typeDefRefered(defSpec))));
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "typedef %s %s",
                scopedTypeIdent,
                scopedName);
            idl_arrayDimensions(idl_typeArray(idl_typeDefRefered(defSpec)));
            idl_fileOutPrintf(idl_fileCur(), ";\n");
            /* define the prototype of the function allocate the typedefed type */
            if (idl_typeSpecType(idl_typeArrayType(idl_typeArray(idl_typeDefRefered(defSpec)))) == idl_tarray) {
                /* This is a multi dimensional array, therefore define a slice */
                idl_fileOutPrintf(idl_fileCur(), "typedef %s %s_slice",
                    scopedTypeIdent,
                    scopedName);
                idl_arraySliceDimensions(idl_typeArray(idl_typeDefRefered(defSpec)));
                idl_fileOutPrintf(idl_fileCur(), ";\n");
                idl_fileOutPrintf(idl_fileCur(), "%s %s_slice *%s__alloc (void);\n\n",
                    idl_dllGetMacro(),
                    scopedName,
                    scopedName);
            } else {
                /* This is a one dimensional array, nevertheless define a slice */
                idl_fileOutPrintf(idl_fileCur(), "typedef %s %s_slice;\n",
                    scopedTypeIdent,
                    scopedName);
                idl_fileOutPrintf(idl_fileCur(), "%s %s_slice *%s__alloc (void);\n\n",
                    idl_dllGetMacro(),
                    scopedName,
                    scopedName);
            }
            os_free(scopedTypeIdent);
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
            idl_typeSeq typeSeq = idl_typeSeq(idl_typeDefRefered(defSpec));
            idl_typeSpec typeSpec = idl_typeDefResolveFully(idl_typeSeqActual(typeSeq));
            c_ulong maxlen = 0;

            /* generate code for a sequence mapping */
            if ((idl_typeSpecType(typeSpec) == idl_tbasic) &&
                (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string)) {
                maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
            }

            if (maxlen > 0) {
                char *seqTypeName = idl_scopedC99TypeIdent(idl_typeSpec(defSpec));
                char *elemTypeName = idl_scopedC99TypeIdent(idl_typeSpec(typeSeq));

                idl_fileOutPrintf(idl_fileCur(),"/* Definition for sequence of %s */\n", elemTypeName);
                idl_fileOutPrintf(idl_fileCur(), "typedef struct %s {\n", seqTypeName);
                idl_fileOutPrintf(idl_fileCur(), "    uint32_t _maximum;\n");
                idl_fileOutPrintf(idl_fileCur(), "    uint32_t _length;\n");
                idl_fileOutPrintf(idl_fileCur(), "    %s *_buffer;\n", elemTypeName);
                idl_fileOutPrintf(idl_fileCur(), "    bool _release;\n");
                idl_fileOutPrintf(idl_fileCur(), "} %s;\n\n", seqTypeName);

                idl_fileOutPrintf(idl_fileCur(), "%s %s * %s__alloc();\n",
                        idl_dllGetMacro(),seqTypeName, seqTypeName);

                idl_fileOutPrintf(idl_fileCur(), "%s %s * %s_allocbuf(uint32_t len);\n\n",
                        idl_dllGetMacro(), elemTypeName, seqTypeName);

                os_free(seqTypeName);
                os_free(elemTypeName);

            } else {
                char *seqName = idl_c99SequenceIdent(idl_typeSeq(idl_typeDefRefered(defSpec)));
                char *elemTypeName = idl_c99SequenceElementIdent(idl_typeSeqType(idl_typeSeq(idl_typeDefRefered(defSpec))));

                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "typedef %s %s;\n", seqName, scopedName);
                /* define the prototype of the function allocate the typedefed type */
                idl_fileOutPrintf(idl_fileCur(), "%s %s *%s__alloc (void);\n",
                        idl_dllGetMacro(), scopedName, scopedName);
                /* define the prototype of the function allocate the sequence buffer */
                idl_fileOutPrintf(idl_fileCur(), "%s %s *%s_allocbuf (uint32_t len);\n\n",
                        idl_dllGetMacro(), elemTypeName, scopedName);

                os_free(seqName);
                os_free(elemTypeName);
            }
        } else {
            char * tname = idl_scopedTypeName(idl_typeSpec(defSpec));
            printf ("idl_typedefOpenClose: Unsupported typedef type (typename = %s, type = %s)\n",
                name, tname);
            os_free(tname);
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

    sequenceElementName = idl_c99SequenceElementIdent(idl_typeSeqType(typeSeq));
    sequenceName = idl_c99SequenceIdent(typeSeq);

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
        char *scopedTypeIdent = idl_scopedC99TypeIdent(idl_typeSeqType(typeSeq));
        idl_fileOutPrintf(idl_fileCur(), "/* Definition for sequence of %s */\n",
                scopedTypeIdent);
        idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_defined\n",
                sequenceName);
        idl_fileOutPrintf(idl_fileCur(), "#define _%s_defined\n",
                sequenceName);
        if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tstruct) {
            /* In case struct is refered, it can be a sequence definition within
             * within the structure with a reference to the struct itself. In that
             * case the struct or union is not defined yet, and a forward
             * definition is required.
             */
            idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_defined\n",
                    scopedTypeIdent);
            idl_fileOutPrintf(idl_fileCur(), "#define _%s_defined\n",
                    scopedTypeIdent);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "typedef struct %s %s;\n",
                    scopedTypeIdent,
                    scopedTypeIdent);
            idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_defined */\n",
                    scopedTypeIdent);
        } else if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tunion) {
            /* In case struct is refered, it can be a sequence definition within
             * within the structure with a reference to the struct itself. In that
             * case the struct or union is not defined yet, and a forward
             * definition is required.
             */
            idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_defined\n",
                    scopedTypeIdent);
            idl_fileOutPrintf(idl_fileCur(), "#define _%s_defined\n",
                    scopedTypeIdent);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "typedef struct %s %s;\n",
                    scopedTypeIdent,
                    scopedTypeIdent);
            idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_defined */\n",
                    scopedTypeIdent);
        }
        idl_fileOutPrintf(idl_fileCur(), "typedef struct {\n");
        idl_fileOutPrintf(idl_fileCur(), "    uint32_t _maximum;\n");
        idl_fileOutPrintf(idl_fileCur(), "    uint32_t _length;\n");
        idl_fileOutPrintf(idl_fileCur(), "    %s *_buffer;\n", sequenceElementName);
        idl_fileOutPrintf(idl_fileCur(), "    bool _release;\n");
        idl_fileOutPrintf(idl_fileCur(), "} %s;\n", sequenceName);

        idl_fileOutPrintf(idl_fileCur(), "%s %s *%s__alloc (void);\n",
                idl_dllGetMacro(),
                sequenceName,
                sequenceName);
        idl_fileOutPrintf(idl_fileCur(), "%s %s *%s_allocbuf (uint32_t len);\n",
                idl_dllGetMacro(),
                sequenceElementName,
                sequenceName);
        idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_defined */\n", sequenceName);

        os_free(scopedTypeIdent);
    }

    sequenceScopedName = idl_c99SequenceIdentScoped(scope, typeSeq);

    idl_fileOutPrintf(idl_fileCur(), "#define %s__alloc %s__alloc\n",
            sequenceScopedName,
            sequenceName);
    idl_fileOutPrintf(idl_fileCur(), "#define %s_allocbuf %s_allocbuf\n\n",
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
static idl_programControl idl_genC99LoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genC99LoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genC99Type = {
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
idl_genC99TypeProgram(
    void)
{
    return &idl_genC99Type;
}
