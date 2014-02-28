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
 * This module generates Splice data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSpliceType.h"
#include "idl_genLanguageHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_dll.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"

#include <ctype.h>
#include "os_stdlib.h"
#include "c_typebase.h"

	/** indentation level */
static c_long indent_level = 0;
	/** enumeration element index */
static c_long enum_element = 0;
/* test_mode ensures no dependencies to other idlpp generated files are generated */
c_bool test_mode = FALSE;
os_char* includeFile = NULL;
c_bool useVoidPtrs = FALSE;

void
idl_genSpliceTypeSetTestMode (
    c_bool val)
{
    test_mode = val;
}

void
idl_genSpliceTypeSetIncludeFileName (
    os_char* val)
{
    includeFile = os_strdup(val);
}

void
idl_genSpliceTypeUseVoidPtrs (
    c_bool val)
{
    useVoidPtrs = val;
}

static void idl_arrayDimensions(idl_typeArray typeArray, os_boolean resolveTypedefs);

/** @brief generate name which will be used as a macro to prevent multiple inclusions
 *
 * From the specified basename create a macro which will
 * be used to prevent multiple inclusions of the generated
 * header file. The basename characters are translated
 * into uppercase characters and the append string is
 * appended to the macro.
 *
 * @param basename Basename of the IDL file passed (without extension).
 * @param append String to append to the basename
 * @return string of concatinated \b basename and \b append where basename is first
 * converted to uppercase characters
 */
static c_char *
idl_macroFromBasename(
    const char *basename,
    const char *append)
{
    static c_char macro[200];
    c_long i;

    for (i = 0; i < (c_long)strlen(basename); i++) {
        macro[i] = toupper(basename[i]);
        macro[i+1] = '\0';
    }
    os_strncat(macro, append, (size_t)sizeof(macro));

    return macro;
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
 * @return Next action for this file (idl_explore)
 */
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    c_long i;

    /* Generate multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s\n", idl_macroFromBasename(name, "SPLTYPES_H"));
    idl_fileOutPrintf(idl_fileCur(), "#define %s\n", idl_macroFromBasename(name, "SPLTYPES_H"));
    idl_fileOutPrintf(idl_fileCur(), "\n");
    if(!test_mode)
    {
        char *dot = strrchr(includeFile, '.');
        if (dot && !strcmp(dot, ".hpp"))
        {
            idl_fileOutPrintf(idl_fileCur(), "#include \"%s\"\n", includeFile);
        }
        else
        {
            idl_fileOutPrintf(idl_fileCur(), "#include \"%s.h\"\n", includeFile);
        }
        if(idl_dllGetHeader() != NULL)
        {
            idl_fileOutPrintf(idl_fileCur(), "%s\n", idl_dllGetHeader());
        }
    }

    /* Generate inclusion of standard OpenSpliceDDS type definition files */
    idl_fileOutPrintf(idl_fileCur(), "#include <c_base.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <c_misc.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <c_sync.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <c_collection.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <c_field.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* Generate code for inclusion of application specific include files */
    for (i = 0; i < idl_depLength(idl_depDefGet()); i++) {
        idl_fileOutPrintf(idl_fileCur(), "#include \"%sSpl%s.h\"\n", idl_depGet (idl_depDefGet(), i), test_mode ? "Type" : "Dcps");
    }
    if (idl_depLength(idl_depDefGet()) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/* @brief callback function called on closing the IDL input file.
 *
 * Generate standard file footer consisting of:
 * - mutiple inclusion prevention closure
 */
static void
idl_fileClose(
    void *userData)
{
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
 * @return Next action for this module (idl_explore)
 */
static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_fileOutPrintf(idl_fileCur(), "extern c_metaObject __%s_%s__load (c_base base);\n",
	idl_scopeBasename(scope),
	idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "\n");

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
    if (idl_scopeStackSize(scope) == 0 || idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule) {
	/* define the prototype of the function for metadata load */
        idl_fileOutPrintf(idl_fileCur(), "extern c_metaObject __%s__load (c_base base);\n",
	        idl_scopeStack(scope, "_", name));
	/* define the prototype of the function for querying the keys */
        idl_fileOutPrintf(idl_fileCur(), "extern const char * __%s__keys (void);\n",
	        idl_scopeStack(scope, "_", name));
	/* define the prototype of the function for querying scoped structure name */
        idl_fileOutPrintf(idl_fileCur(), "extern const char * __%s__name (void);\n",
	        idl_scopeStack(scope, "_", name));
        if(!test_mode)
        {
            /* define the prototype of the function for performing copyIn/copyOut. This needs to be
             * exported as well to ensure it can be access by other (ussually also OpenSplice
             * generated) packages
             */
            idl_fileOutPrintf(
                idl_fileCur(),
                "struct _%s ;\n",
                idl_scopeStack(scope, "_", name));

            if(useVoidPtrs)
            {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "extern %s c_bool __%s__copyIn(c_base base, void *from, void *to);\n",
                    idl_dllGetMacro(),
                    idl_scopeStack(scope, "_", name));
            } else
            {
                if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
                {
                  idl_fileOutPrintf(
                      idl_fileCur(),
                      "extern %s c_bool __%s__copyIn(c_base base, class %s *from, struct _%s *to);\n",
                      idl_dllGetMacro(),
                      idl_scopeStack(scope, "_", name),
                      idl_scopeStack(scope, "::", name),
                      idl_scopeStack(scope, "_", name));
                }
                else
                {
                  idl_fileOutPrintf(
                      idl_fileCur(),
                      "extern %s c_bool __%s__copyIn(c_base base, struct %s *from, struct _%s *to);\n",
                      idl_dllGetMacro(),
                      idl_scopeStack(scope, "_", name),
                      idl_scopeStack(scope, "::", name),
                      idl_scopeStack(scope, "_", name));
                }
            }
            idl_fileOutPrintf(
                idl_fileCur(),
                "extern %s void __%s__copyOut(void *_from, void *_to);\n",
                idl_dllGetMacro(),
                idl_scopeStack(scope, "_", name));
        }

    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "struct _%s {\n",
        idl_scopeStack(scope, "_", name));
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
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n\n");
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
        sequence<spec,length> <name>; => c_sequence <name,length>;
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
    if (idl_typeSpecType(typeSpec) == idl_ttypedef||
        idl_typeSpecType(typeSpec) == idl_tenum ||
        idl_typeSpecType(typeSpec) == idl_tstruct ||
        idl_typeSpecType(typeSpec) == idl_tunion ||
        idl_typeSpecType(typeSpec) == idl_tbasic)
    {
        os_boolean catsRequested = OS_FALSE;
        os_boolean stacRequested = OS_FALSE;
        idl_typeSpec baseTypeDereffered = NULL;
        idl_typeSpec typeDereffered;

        /* is a stac pragma defined for this member? */
        stacRequested = idl_stacDef_isStacDefined(scope, name, typeSpec, &baseTypeDereffered);
        if(!stacRequested)
        {
            /* Cats and stac def can not be defined over the same member as
             * one excepts string types and the other char (array) types
             */
            catsRequested = idl_catsDef_isCatsDefined(scope, name, typeSpec);
        }

        if(catsRequested)
        {
            assert(!stacRequested);
            typeDereffered = idl_typeDefResolveFully(typeSpec);
            idl_structureMemberOpenClose(scope, name, typeDereffered, userData);
        } else if(stacRequested)
        {
            assert(baseTypeDereffered);
            typeDereffered = idl_typeDefResolveFully(typeSpec);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "c_char %s", idl_languageId (name));
            /* potential array bounds */
            if((idl_typeSpecType(typeDereffered) == idl_tarray))
            {
                idl_arrayDimensions(idl_typeArray(typeDereffered), OS_TRUE);
            }
            idl_fileOutPrintf(idl_fileCur(), "[%d];\n", idl_typeBasicMaxlen(idl_typeBasic(baseTypeDereffered))+1);
        } else
        {
            /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s %s;\n",
                idl_scopedSplTypeIdent(typeSpec),
                idl_languageId(name));
        }
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        /* generate code for an array mapping */
        os_boolean catsRequested = OS_FALSE;
        os_boolean stacRequested = OS_FALSE;
        idl_typeSpec baseTypeDereffered = NULL;

        /* is a stac pragma defined for this member? */
        stacRequested = idl_stacDef_isStacDefined(scope, name, typeSpec, &baseTypeDereffered);
        if(!stacRequested)
        {
            /* Cats and stac def can not be defined over the same member as
             * one excepts string types and the other char (array) types
             */
            catsRequested = idl_catsDef_isCatsDefined(scope, name, typeSpec);
        }
        if(catsRequested)
        {
            assert(!stacRequested);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "c_string %s;\n", idl_languageId(name));
        } else if(stacRequested)
        {
            assert(!catsRequested);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "c_char %s",
                idl_languageId(name));
            idl_arrayDimensions(idl_typeArray(typeSpec), OS_TRUE);
            idl_fileOutPrintf(idl_fileCur(),"[%d]", idl_typeBasicMaxlen(idl_typeBasic(baseTypeDereffered))+1);
            idl_fileOutPrintf(idl_fileCur(), ";\n");
        } else
        {
            idl_printIndent(indent_level);
            if (idl_typeSpecType(idl_typeArrayActual (idl_typeArray(typeSpec))) != idl_tseq)
            {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "%s %s",
                    idl_scopedSplTypeIdent(idl_typeArrayActual(idl_typeArray(typeSpec))),
                    idl_languageId(name));
            } else
            {
                idl_fileOutPrintf(idl_fileCur(), "c_array %s", idl_languageId (name));
            }
            idl_arrayDimensions(idl_typeArray(typeSpec), OS_FALSE);
            idl_fileOutPrintf(idl_fileCur(), ";\n");
        }
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	/* generate code for a sequence mapping */
        idl_printIndent(indent_level);
        if (idl_typeSeqMaxSize (idl_typeSeq(typeSpec)) == 0) {
	    /* unbounded sequence */
            idl_fileOutPrintf(idl_fileCur(), "c_sequence %s", idl_languageId(name));
        } else {
	    /* bounded sequence */
            idl_fileOutPrintf (idl_fileCur(), "c_sequence %s", idl_languageId(name));
        }
        idl_fileOutPrintf (idl_fileCur(), ";\n");
    } else {
        printf ("idl_structureMemberOpenClose: Unsupported structure member type (member name = %s, type name = %s)\n",
            name, idl_scopedTypeName(typeSpec));
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
    if (idl_scopeStackSize(scope) == 0 || idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule) {
	/* define the prototype of the function for metadata load */
        idl_fileOutPrintf(idl_fileCur(), "extern c_metaObject __%s__load (c_base base);\n",
	    idl_scopeStack(scope, "_", name));
	/* define the prototype of the function for querying the keys */
        idl_fileOutPrintf(idl_fileCur(), "extern const char * __%s__keys (void);\n",
	    idl_scopeStack(scope, "_", name));
	/* define the prototype of the function for querying scoped union name */
        idl_fileOutPrintf(idl_fileCur(), "extern const char * __%s__name (void);\n",
	    idl_scopeStack(scope, "_", name));
    }
    /* open the struct */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct _%s {\n", idl_scopeStack(scope, "_", name));
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
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "} _u;\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n\n");
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
 * This function generates a prototype for the helper function to
 * load the enumerations metadata if the enumeration is defined
 * within the global scope or the scope of a module.
 * The name of the metadata load function is:
 * @verbatim
        __<scope-elements>_<enum-name>__load
   @endverbatim
 * The enum definition is opened:
 * @verbatim
        enum <scope-elements>_<enum-name> {
   @endverbatim
 * @param scope Current scope
 * @param name Name of the enumeration
 * @param enumSpec Specifies the number of elements in the enumeration
 * @return Next action for this enumeration (idl_explore)
 */
static idl_action
idl_enumerationOpen(
    idl_scope scope,
    const char *name,
    idl_typeEnum enumSpec,
    void *userData)
{
    if (idl_scopeStackSize(scope) == 0 || idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule) {
        idl_fileOutPrintf(idl_fileCur(), "extern c_metaObject __%s__load (c_base base);\n",
	    idl_scopeStack(scope, "_", name));
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "enum _%s {\n", idl_scopeStack (scope, "_", name));
    enum_element = idl_typeEnumNoElements(enumSpec);
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
 * The enumeration is closed:
 * @verbatim
        };
   @endverbatim
 *
 * @param name Name of the enumeration
 */
static void
idl_enumerationClose(
    const char *name,
    void *userData)
{
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n\n");
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
idl_enumerationElementOpenClose(
    idl_scope scope,
    const char *name,
    void *userData)
{
    enum_element--;
    if (enum_element == 0) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "_%s\n",
	    idl_scopeStack(scope, "_", name));
    } else {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "_%s,\n",
            idl_scopeStack (scope, "_", name));
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

/** @brief callback function called on definition of a named type in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   typedef <type-name> <name>;
   @endverbatim
 *
 * This function generates a prototype for the helper function to
 * load the typedef metadata if the typedef is defined
 * within the global scope or the scope of a module.
 * The name of the metadata load function is:
 * @verbatim
        __<scope-elements>_<name>__load
   @endverbatim
 * If the type specification is idl_tbasic a standard mapping will be applied:
 * @verbatim
	typedef <basic-type-mapping> <scope-elements>_<name>;
   @endverbatim
 * If the type specification is a user defined idl_ttypedef, idl_tenum,
 * idl_tstruct or idl_tunion a scoped name mapping will be generated.
 * @verbatim
	<typedef-name> <name>;     => typedef enum <scope-elements>_<typedef-name> <scope-elements>_<name>;
	<enum-name> <name>;        => typedef enum <scope-elements>_<enum-name> <scope-elements>_<name>;
	<struct-name> <name>;      => typedef struct <scope-elements>_<structure-name> <scope-elements>_<name>;
	<union-name> <name>;       => typedef struct <scope-elements>_<union-name> <scope-elements>_<name>;
   @endverbatim
 * If the type specification is idl_tarray then generate a scoped name
 * with the array specifiers:
 * @verbatim
        <other-usertype-name> <name>[n1]..[nn]; =>
            typedef <scope-elements>_<other-usertype-name> <scope-elements>_<name>[n1]..[nn];
        <basic-type> <name>[n1]..[nn];          =>
            typedef <basic-type-mapping> <scope-elements>_<name>[n1]..[nn];
        sequence<spec> <name>[n1]..[nn];        =>
            typedef c_array <scope-elements>_<name>[n1]..[nn];
        sequence<spec,length> <name>[n1]..[nn]; =>
            typedef c_array <scope-elements>_<name>[n1]..[nn];
   @endverbatim
 * If the type specification is idl_tseq then generate a mapping on c_sequence:
 * @verbatim
        sequence<spec> <name>;        => typedef c_sequence <scope-elements>_<name>;
        sequence<spec,length> <name>; => typedef c_sequence <scope-elements>_<name>;
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
    if (idl_scopeStackSize(scope) == 0 || idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "extern c_metaObject __%s__load (c_base base);\n",
            idl_scopeStack(scope, "_", name));
        if (idl_typeSpecType(idl_typeDefActual(defSpec)) == idl_tstruct ||
            idl_typeSpecType(idl_typeDefActual(defSpec)) == idl_tunion) {
            /* If this is a typedef of a struct or union			*/
            /* define the prototype of the function for querying the keys 	*/
            idl_fileOutPrintf(
                idl_fileCur(),
                "extern const char * __%s__keys (void);\n",
                idl_scopeStack(scope, "_", name));
	    /* define the prototype of the function for querying scoped structure name */
            idl_fileOutPrintf(
                idl_fileCur(),
                "extern const char * __%s__name (void);\n",
                idl_scopeStack(scope, "_", name));
        }
    }
    if (idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_ttypedef ||
        idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_tenum ||
        idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_tstruct ||
        idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_tunion ||
        idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_tbasic) {
        /* generate code for a standard mapping or a typedef, enum, struct or union user-type mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "typedef %s _%s;\n\n",
            idl_scopedSplTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec))),
            idl_scopeStack (scope, "_", name));
    } else if (idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_tarray) {
        /* generate code for an array mapping */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "typedef %s _%s",
            idl_scopedSplTypeIdent(idl_typeArrayActual (idl_typeArray(idl_typeDefRefered(defSpec)))),
            idl_scopeStack(scope, "_", name));
        idl_arrayDimensions(idl_typeArray(idl_typeDefRefered(defSpec)), OS_FALSE);
	    idl_fileOutPrintf(idl_fileCur(), ";\n\n");
    } else if (idl_typeSpecType(idl_typeSpec(idl_typeDefRefered(defSpec))) == idl_tseq) {
        /* generate code for a sequence mapping */
        idl_printIndent(indent_level);
        if (idl_typeSeqMaxSize (idl_typeSeq(idl_typeDefRefered(defSpec))) == 0) {
	    /* unbounded sequence */
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef c_sequence _%s",
                idl_scopeStack (scope, "_", name));
        } else {
	    /* bounded sequence */
            idl_fileOutPrintf(
                idl_fileCur(),
                "typedef c_sequence _%s",
                idl_scopeStack(scope, "_", name));
        }
        idl_fileOutPrintf (idl_fileCur(), ";\n\n");
    } else {
        printf("idl_typedefOpenClose: Unsupported typedef type (typename = %s, type = %s)\n",
            name, idl_scopedTypeName(idl_typeSpec(defSpec)));
    }
}

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
    return &idl_genSpliceLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSpliceType = {
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
idl_genSpliceTypeProgram(
    void)
{
    return &idl_genSpliceType;
}
