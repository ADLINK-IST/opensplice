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
 * This module generates Splice data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSpliceLiteType.h"
#include "idl_genLanguageHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genFileHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_dll.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"

#include <ctype.h>
#include "os_stdlib.h"
#include "c_typebase.h"
#include "os_heap.h"

/** indentation level */
static c_long indent_level = 0;
/* test_mode ensures no dependencies to other idlpp generated files are generated */
static c_bool test_mode = FALSE;
static os_char* includeFile = NULL;
static c_bool useVoidPtrs = FALSE;

void
idl_genSpliceLiteTypeSetTestMode (
    c_bool val)
{
    test_mode = val;
}

void
idl_genSpliceLiteTypeSetIncludeFileName (
    os_char* val)
{
    os_free(includeFile);
    includeFile = os_strdup(val);
}

void
idl_genSpliceLiteTypeUseVoidPtrs (
    c_bool val)
{
    useVoidPtrs = val;
}

#if 0
static void idl_arrayDimensions(idl_typeArray typeArray, os_boolean resolveTypedefs);
#endif

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

/** @brief callback function called on opening the IDL input file.
 *
 * Generate standard file header consisting of:
 * - mutiple inclusion prevention
 * - inclusion of Splice type definition files
 * - inclusion of application specific include files related to other IDL files
 *
 * @param scope Current scope (not used)
 * @param name Name of the IDL input file
 * @return Next action for this file (idl_explore)
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
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s\n", idl_macroFromBasename(scope, "SPLTYPES_H"));
    idl_fileOutPrintf(idl_fileCur(), "#define %s\n", idl_macroFromBasename(scope, "SPLTYPES_H"));
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* Generate inclusion of standard OpenSpliceDDS type definition files */

    if(!test_mode)
    {
        idl_fileOutPrintf(idl_fileCur(), "#include \"%s\"\n", includeFile);
    }

    /* Generate code for inclusion of application specific include files */
    for (i = 0; i < idl_depLength(idl_depDefGet()); i++) {
        idl_fileOutPrintf(idl_fileCur(), "#include \"%sSpl%s.h\"\n", idl_depGet (idl_depDefGet(), i), test_mode ? "Type" : "Dcps");
    }
    if (idl_depLength(idl_depDefGet()) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    if(idl_dllGetHeader() != NULL)
    {
        idl_fileOutPrintf(idl_fileCur(), "%s\n\n", idl_dllGetHeader());
    }
    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/** @brief callback function called on closing the IDL input file.
 *
 * Generate standard file footer consisting of:
 * - mutiple inclusion prevention closure
 */
static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    /* Generate closure of multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#undef OS_API\n");
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
 * @return Next action for this module (idl_explore)
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
 * This function generates prototypes for the helper functions to
 * load the structures metadata, to determine the structures keys
 * and the scoped name of the struct if the struct is defined in
 * the global scope or within a module.
 * The name of the metadata load function is:
 * @verbatim
        __<scope-elements>_<structure-name>__load
   @endverbatim
 * The name of the key query function is:
 * @verbatim
        __<scope-elements>_<structure-name>__keys
   @endverbatim
 * The name of the name query function is:
 * @verbatim
        __<scope-elements>_<structure-name>__name
   @endverbatim
 * If the struct is defined within another struct or within a
 * union, no key nor its name can be queried. Such a structure
 * can not be communicated via Splice as a separate entity.
 * IDL structs are mapped onto C structs for Splice, therefor
 * a struct definition is opened:
 * @verbatim
        struct <scope-elements>_<name> {
   @endverbatim
 *
 * @param scope Current scope (and scope of the structure definition)
 * @param name Name of the structure
 * @param structSpec Specification of the struct holding the amount of members
 * @return Next action for this structure (idl_explore)
 */
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    char *fullyScopedName = idl_scopeStack(scope, "_", name);
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    if (idl_scopeStackSize(scope) == 0 || idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule) {
        if(!test_mode)
        {
            idl_fileOutPrintf(
                    idl_fileCur(),
                    "extern %s void __%s__copyIn(const void *from, void *to);\n\n",
                    idl_dllGetMacro(),
                    fullyScopedName);

            idl_fileOutPrintf(
                    idl_fileCur(),
                    "extern %s void __%s__copyOut(const void *_from, void *_to);\n",
                    idl_dllGetMacro(),
                    fullyScopedName);

            idl_fileOutPrintf(
                    idl_fileCur(),
                    "extern %s void __%s__copyOutSeq(void *_from[], void *_to, int n);\n\n",
                    idl_dllGetMacro(),
                    fullyScopedName);

            idl_fileOutPrintf(
                    idl_fileCur(),
                    "extern %s void * __%s__allocator();\n",
                    idl_dllGetMacro(),
                    fullyScopedName);

            idl_fileOutPrintf(
                    idl_fileCur(),
                    "extern %s void __%s__deallocator(void *p);\n",
                    idl_dllGetMacro(),
                    fullyScopedName);
        }

    }

    indent_level++;
    os_free(fullyScopedName);
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
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "\n\n");
}

#if 0
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
 * This function generates prototypes for the helper functions to
 * load the unions metadata, to determine the unions keys and the
 * scoped name of the union if the union is defined in the global
 * scope or within a module.
 * The name of the metadata load function is:
 * @verbatim
        __<scope-elements>_<structure-name>__load
   @endverbatim
 * The name of the key query function is:
 * @verbatim
        __<scope-elements>_<union-name>__keys
   @endverbatim
 * The name of the name query function is:
 * @verbatim
        __<scope-elements>_<union-name>__name
   @endverbatim
 * If the union is defined within another union or within a
 * struct, no key nor its name can be queried. Such a union
 * can not be communicated via Splice as a separate entity.
 * IDL unions are mapped onto C structs for Splice in the
 * following manner:
 * @verbatim
        struct <union-name> {
            <switch-type> _d;
            union {
                <union-case-1>;
                ...        ...
                <union-case-m>;
            } _u;
        };
   @endverbatim
 * The union definition is opened:
 * @verbatim
        struct <scope-elements>_<name> {
   @endverbatim
 * Then depending on the type of the switch (integral type is required) any of the following:
 * @verbatim
        enum <enum-type> _d;		// for an enumeration
        <scope-elements>_<typedef-name> _d;	// for an typedeffed enum or basic type
        <basic-type-mapping> _d;		// for an integral basic type
   @endverbatim
 * Then open the union part:
 * @verbatim
            union {
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the union
 * @param unionSpec Specifies the number of union cases and the union switch type
 * @return Next action for this union (idl_explore)
 */
static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    char *fullyScopedName = idl_scopeStack(scope, "_", name);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "/* no union support yet */\n");
    idl_fileOutPrintf(idl_fileCur(), "#ifdef 0\n");
    /* open the struct */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct _%s {\n", fullyScopedName);
    indent_level++;
    /* generate code for the switch */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf (idl_fileCur(), "%s _d;\n", idl_scopedTypeName(idl_typeUnionSwitchKind(unionSpec)));
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tenum) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s _d;\n", idl_scopedSplTypeName(idl_typeUnionSwitchKind(unionSpec)));
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) {
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec))))) {
        case idl_tbasic:
        case idl_tenum:
            idl_printIndent(indent_level); idl_fileOutPrintf (idl_fileCur(), "%s _d;\n",
            idl_scopedSplTypeName(idl_typeUnionSwitchKind(unionSpec)));
        break;
        default:
            printf ("idl_unionOpen: Unsupported switchkind\n");
        }
    } else {
        printf ("idl_unionOpen: Unsupported switchkind\n");
    }

    /* open the union */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "union {\n");

    indent_level++;
    os_free(fullyScopedName);


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
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "} _u;\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");
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
 * If the type specification is idl_tbasic a standard mapping can be generated:
 * @verbatim
        string <name>;             => c_string <name>;
        string <name,length>;      => c_string <name>;
        char <name>;               => c_char <name>;
        octet <name>;              => c_octet <name>;
        short <name>;              => c_short <name>;
        unsigned short <name>;     => c_ushort <name>;
        long <name>;               => c_long <name>;
        unsigned long <name>;      => c_ulong <name>;
        long long <name>;          => c_longlong <name>;
        unsigned long long <name>; => c_ulonglong <name>;
        float <name>;              => c_float <name>;
        double <name>;             => c_double <name>;
        boolean <name>;            => c_bool <name>;
   @endverbatim
 * If the type specification is a user defined idl_ttypedef, idl_tenum,
 * idl_tstruct or idl_tunion a scoped name mapping will be generated.
 * @verbatim
    <typedef-name> <name>;     => enum <scope-elements>_<typedef-name> <name>;
    <enum-name> <name>;        => enum <scope-elements>_<enum-name> <name>;
    <struct-name> <name>;      => struct <scope-elements>_<structure-name> <name>;
    <union-name> <name>;       => struct <scope-elements>_<union-name> <name>;
   @endverbatim
 * If the type specification is idl_tarray then generate a scoped name
 * with the array specifiers:
 * @verbatim
        <other-usertype-name> <name>[n1]..[nn]; => <scope-elements>_<other-usertype-name> <name>[n1]..[nn];
        <basic-type> <name>[n1]..[nn];          => <basic-type-mapping> <name>[n1]..[nn];
        sequence<spec> <name>[n1]..[nn];        => c_array <name>[n1]..[nn];
        sequence<spec,length> <name>[n1]..[nn]; => c_array <name>[n1]..[nn];
   @endverbatim
 * If the type specification is idl_tseq then generate a mapping on c_sequence:
 * @verbatim
        sequence<spec> <name>;        => c_sequence <name>;
        sequence<spec,length> <name>; => c_sequence <name>;
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

    if (idl_typeSpecType(typeSpec) == idl_ttypedef ||
        idl_typeSpecType(typeSpec) == idl_tenum ||
        idl_typeSpecType(typeSpec) == idl_tstruct ||
        idl_typeSpecType(typeSpec) == idl_tunion ||
        idl_typeSpecType(typeSpec) == idl_tbasic) {
    /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s;\n",
            idl_scopedSplTypeIdent(typeSpec),
            idl_languageId(name));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        /* generate code for an array mapping */
        idl_printIndent(indent_level);
        if (idl_typeSpecType(idl_typeArrayActual (idl_typeArray(typeSpec))) != idl_tseq) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s",
                idl_scopedSplTypeIdent(idl_typeArrayActual (idl_typeArray(typeSpec))),
                idl_languageId (name));
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "c_array %s",
                idl_languageId(name));
        }
        idl_arrayDimensions(idl_typeArray(typeSpec), OS_FALSE);
        idl_fileOutPrintf(idl_fileCur(), ";\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        /* generate code for a sequence mapping */
        idl_printIndent(indent_level);
        if (idl_typeSeqMaxSize (idl_typeSeq(typeSpec)) == 0) {
            /* unbounded sequence */
            idl_fileOutPrintf(idl_fileCur(), "c_sequence %s", idl_languageId (name));
        } else {
            /* bounded sequence */
            idl_fileOutPrintf(idl_fileCur(), "c_sequence %s", idl_languageId (name));
        }
        idl_fileOutPrintf(idl_fileCur(), ";\n");
    } else {
        printf("idl_unionCaseOpenClose: Unsupported union case type (case name = %s, type = %s)\n",
            name, idl_scopedTypeName(typeSpec));
    }
}
#endif

#if 0
/* @brief generate dimension of an array
 *
 * arrayDimensions is a local support function to generate
 * the array dimensions of an array
 *
 * @param typeArray Specifies the type of the array
 */
static void
idl_arrayDimensions (
    idl_typeArray typeArray,
    os_boolean resolveTypedefs)
{
    idl_typeSpec subType;

    idl_fileOutPrintf(idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
    subType = idl_typeArrayType(typeArray);
    while(resolveTypedefs && idl_typeSpecType(subType) == idl_ttypedef)
    {
        subType = idl_typeDefResolveFully(subType);
    }
    if (idl_typeSpecType(subType) == idl_tarray) {
        idl_arrayDimensions (idl_typeArray(subType), resolveTypedefs);
    }
}
#endif

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSpliceLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 *
 * @return The program control structure for the splice type generation functions
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genSpliceLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSpliceLiteType = {
    idl_getControl,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    idl_structureClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    NULL,
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 *
 * @return The callback table for the splice type generation functions
 */
idl_program
idl_genSpliceLiteTypeProgram(
    void)
{
    return &idl_genSpliceLiteType;
}
