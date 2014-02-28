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

/**
 * @file
 * This module generates Standalone Java classes
 * related to an IDL input file.
*/
#include "os_heap.h"
#include "os_stdlib.h"

#include <ctype.h>

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genSajHolder.h"
#include "idl_genSplHelper.h"
#include "idl_genJavaHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"

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
    c_long i;

    for (i = 0; i < (c_long)strlen(basename); i++) {
        macro[i] = toupper (basename[i]);
        macro[i+1] = '\0';
    }
    os_strncat(macro, append, (size_t)((int)sizeof(macro)-(int)strlen(append)));

    return macro;
}

static os_equality
defName(
    void *iterElem,
    void *arg)
{
    if (strcmp((char *)iterElem, (char *)arg) == 0) {
        return OS_EQ;
    }
    return OS_NE;
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
    int nameLen = strlen(idl_javaId(name)) + strlen("Holder") + 1;
    char *holderName;

    holderName = os_malloc(nameLen);
    snprintf(holderName, nameLen, "%sHolder", idl_javaId(name));

    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, holderName);
    if (idl_fileCur() == NULL) {
        return idl_abort;
    }
    if (idl_scopeStackSize(scope) > 0) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "package %s;\n",
            idl_scopeStackJava(scope, ".", NULL));
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    idl_fileOutPrintf(idl_fileCur(), "public final class %s\n{\n\n", holderName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s value = null;\n\n",
        idl_scopeStackJava(scope, ".", idl_javaId(name)));
    idl_fileOutPrintf(idl_fileCur(), "    public %s () { }\n\n", holderName);

    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s (%s initialValue)\n",
        holderName,
        idl_scopeStackJava(scope, ".", idl_javaId(name)));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        value = initialValue;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* close file */
    idl_closeJavaPackage();

    /* return idl_abort to indicate that the rest of the structure does not need to be processed */
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
    int nameLen = strlen(idl_javaId(name)) + strlen("Holder") + 1;
    char *holderName;

    holderName = os_malloc(nameLen);
    snprintf(holderName, nameLen, "%sHolder", idl_javaId(name));

    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, holderName);
    if (idl_fileCur() == NULL) {
        return idl_abort;
    }
    if (idl_scopeStackSize(scope) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "package %s;\n",
	    idl_scopeStackJava(scope, ".", NULL));
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    idl_fileOutPrintf(idl_fileCur(), "public final class %s\n{\n\n", holderName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s value = null;\n\n",
        idl_scopeStackJava(scope, ".", idl_javaId(name)));
    idl_fileOutPrintf(idl_fileCur(), "    public %s () { }\n\n", holderName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s (%s initialValue)\n",
        holderName,
        idl_scopeStackJava(scope, ".", idl_javaId(name)));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        value = initialValue;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* close file */
    idl_closeJavaPackage();

    /* return idl_abort to indicate that the rest of the union does not need to be processed */
    return idl_abort;
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
    int nameLen = strlen(idl_javaId(name)) + strlen("Holder") + 1;
    char *holderName;

    holderName = os_malloc(nameLen);
    snprintf(holderName, nameLen, "%sHolder", idl_javaId(name));

    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, holderName);
    if (idl_fileCur() == NULL) {
        return idl_abort;
    }
    if (idl_scopeStackSize(scope) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava(scope, ".", NULL));
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    idl_fileOutPrintf(idl_fileCur(), "public final class %s\n{\n\n", holderName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s value = null;\n\n",
        idl_scopeStackJava(scope, ".", idl_javaId(name)));
    idl_fileOutPrintf(idl_fileCur(), "    public %s () { }\n\n", holderName);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s (%s initialValue)\n",
        holderName,
        idl_scopeStackJava(scope, ".", idl_javaId(name)));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        value = initialValue;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* close file */
    idl_closeJavaPackage();

    /* return idl_abort to indicate that the rest of the enum does not need to be processed */
    return idl_abort;
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
    int nameLen = strlen(idl_javaId(name)) + strlen("Holder") + 1;
    char *holderName;


    if (idl_typeSpecType(idl_typeDefRefered (defSpec)) == idl_tseq) {
        holderName = os_malloc(nameLen);
        snprintf(holderName, nameLen, "%sHolder", idl_javaId(name));
        /* Open file for used scope, if needed create the directories */
        idl_openJavaPackage(scope, holderName);
        if (idl_fileCur() == NULL) {
            return;
        }
        if (idl_scopeStackSize(scope) > 0) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "package %s;\n",
                idl_scopeStackJava(scope, ".", NULL));
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
        idl_fileOutPrintf(idl_fileCur(), "public final class %s\n{\n\n", holderName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s%s value = null;\n\n",
            idl_corbaJavaTypeFromTypeSpec(idl_typeSeqActual(idl_typeSeq(idl_typeDefRefered(defSpec)))),
            idl_sequenceIndexString (idl_typeSeq (idl_typeDefRefered (defSpec))));
        idl_fileOutPrintf(idl_fileCur(), "    public %s () { }\n\n", holderName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s (%s%s initialValue)\n",
            holderName,
            idl_corbaJavaTypeFromTypeSpec(idl_typeSeqActual(idl_typeSeq(idl_typeDefRefered(defSpec)))),
            idl_sequenceIndexString(idl_typeSeq (idl_typeDefRefered (defSpec))));
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_fileOutPrintf(idl_fileCur(), "        value = initialValue;\n");
        idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        /* close file */
        idl_closeJavaPackage();
    } else if (idl_typeSpecType(idl_typeDefRefered (defSpec)) == idl_tarray) {
        holderName = os_malloc(nameLen);
        snprintf(holderName, nameLen, "%sHolder", idl_javaId(name));
        /* Open file for used scope, if needed create the directories */
        idl_openJavaPackage(scope, holderName);
        if (idl_fileCur() == NULL) {
            return;
        }
        if (idl_scopeStackSize(scope) > 0) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "package %s;\n",
                idl_scopeStackJava(scope, ".", NULL));
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
        idl_fileOutPrintf(idl_fileCur(), "public final class %s\n{\n\n", holderName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s%s value = null;\n\n",
            idl_corbaJavaTypeFromTypeSpec(idl_typeArrayActual(idl_typeArray(idl_typeDefRefered(defSpec)))),
            idl_arrayJavaIndexString (idl_typeArray (idl_typeDefRefered (defSpec))));
        idl_fileOutPrintf(idl_fileCur(), "    public %s () { }\n\n", holderName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s (%s%s initialValue)\n",
            holderName,
            idl_corbaJavaTypeFromTypeSpec(idl_typeArrayActual(idl_typeArray(idl_typeDefRefered(defSpec)))),
            idl_arrayJavaIndexString(idl_typeArray (idl_typeDefRefered (defSpec))));
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_fileOutPrintf(idl_fileCur(), "        value = initialValue;\n");
        idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        /* close file */
        idl_closeJavaPackage();
    }
}

static void
idl_sequenceOpenClose(
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
}

static void
idl_constantOpenClose(
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSajLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    return &idl_genSajLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSajHolder = {
    idl_getControl,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    NULL, /* idl_structureClose */
    NULL, /* idl_structureMemberOpenClose */
    idl_enumerationOpen,
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    idl_unionOpen,
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
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
idl_genSajHolderProgram(
    void)
{
    return &idl_genSajHolder;
}
