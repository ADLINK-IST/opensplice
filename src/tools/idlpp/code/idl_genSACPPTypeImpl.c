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
   This module generates type definitions related to
   an IDL input file.
   For implementation consideration see end of file.
*/

#include "idl_program.h"
/**
 * @file
 * This module generates Standalone C++ data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSACPPTypeImpl.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_dll.h"

#include "os.h"
#include <ctype.h>
#include "c_typebase.h"

#define MAX_BUFFER (256)

struct idl_genSACPPData {
    /** indentation level */
    c_long indent_level;
    char buffer[MAX_BUFFER];
};

static struct idl_genSACPPData idl_genSACPPData;

static c_ulong
idl_genArrayTotalDimension(
    idl_typeArray typeArray)
{
    c_ulong result = 0;
    idl_typeSpec ptr;

    result = (c_ulong)idl_typeArraySize(typeArray);
    ptr = idl_typeArrayType(typeArray);
    while (idl_typeSpecType(ptr) == idl_tarray) {
        result = result * (c_ulong)idl_typeArraySize(idl_typeArray(ptr));
        ptr = idl_typeArrayType(idl_typeArray(ptr));
    }

    return result;
}
static void
idl_arrayTypeImpl(
    idl_typeArray typeArray,
    const char *name,
    struct idl_genSACPPData *arg)
{
    idl_typeSpec actualType;
    idl_typeSpec subType;
    c_ulong totalDim;

    actualType = idl_typeArrayActual(typeArray);
    subType = idl_typeArrayType(typeArray);
    /* determine total buffer len, by multiplying all dimensions */
    totalDim = idl_genArrayTotalDimension(typeArray);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"template <>\n");
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"%s_slice *DDS_DCPS_ArrayHelper<%s, %s_slice, %s_uniq_>::alloc()\n",
        name, name, name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    idl_printIndent(arg->indent_level+1);
    idl_fileOutPrintf(idl_fileCur(),"return %s_alloc();\n", name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"template <>\n");
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"void DDS_DCPS_ArrayHelper<%s, %s_slice, %s_uniq_>::copy(%s_slice *to, const %s_slice *from)\n",
        name, name, name, name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    idl_printIndent(arg->indent_level+1);
    idl_fileOutPrintf(idl_fileCur(),"%s_copy(to, from);\n", name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"template <>\n");
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"void DDS_DCPS_ArrayHelper<%s, %s_slice, %s_uniq_>::free(%s_slice *ptr)\n",
        name, name, name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    idl_printIndent(arg->indent_level+1);
    idl_fileOutPrintf(idl_fileCur(),"%s_free(ptr);\n", name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    /* Generate implementation _alloc() method */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"%s_slice *%s_alloc()\n", name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    arg->indent_level++;
    if ((idl_typeSpecType(actualType) == idl_tbasic) &&
        (idl_typeBasicType(idl_typeBasic(actualType)) == idl_string)) {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"::DDS::String_mgr *ret = (::DDS::String_mgr *) new ::DDS::String_mgr[%d];\n",totalDim);
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"for (::DDS::ULong i = 0; i < %d; i++) {\n", totalDim);
        idl_printIndent(arg->indent_level+1);
        idl_fileOutPrintf(idl_fileCur(),"ret[i] = (::DDS::String)NULL;\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"}\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"return (%s_slice *)ret;\n");
    } else {
        if (idl_typeSpecType(actualType) == idl_tseq) {
            /* \TODO */
        } else {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s *ret = (%s *) new %s[%d];\n",
                idl_corbaCxxTypeFromTypeSpec(actualType),
                idl_corbaCxxTypeFromTypeSpec(actualType),
                idl_corbaCxxTypeFromTypeSpec(actualType),
                totalDim);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"return (%s_slice *)ret;\n", name);
        }
    }
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    /* Generate implementation _free() method:
     * Implementation is independent of the actual type.
     */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"void %s_free(%s_slice *s)\n", name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    arg->indent_level++;
    if ((idl_typeSpecType(actualType) == idl_tbasic) &&
        (idl_typeBasicType(idl_typeBasic(actualType)) == idl_string)) {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"if (s) {\n");
        arg->indent_level++;
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"::DDS::String_mgr *base = (::DDS::String_mgr *)s;\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"for (::DDS::ULong i = 0; i < %d; i++) {\n", totalDim);
        idl_printIndent(arg->indent_level+1);
        idl_fileOutPrintf(idl_fileCur(),"base[i] = (::DDS::String)NULL;\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"}\n");
        arg->indent_level--;
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"}\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"delete [] s;\n");
    } else {
        if (idl_typeSpecType(actualType) == idl_tseq) {
            /* \TODO */
        } else {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"delete [] s;\n");
        }
    }
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    /* Generate implementation _copy() method */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"void %s_copy(%s_slice *to, const %s_slice *from)\n",
        name, name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    arg->indent_level++;
    if ((idl_typeSpecType(actualType) == idl_tbasic) &&
        (idl_typeBasicType(idl_typeBasic(actualType)) == idl_string)) {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"::DDS::String_mgr *sv = (::DDS::String_mgr *)from;\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"::DDS::String_mgr *tv = (::DDS::String_mgr *)to;\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"for (::DDS::ULong i = 0; i < %d; i++) {\n", totalDim);
        idl_printIndent(arg->indent_level+1);
        idl_fileOutPrintf(idl_fileCur(),"tv[i] = ::DDS::string_dup (sv[i]);\n");
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"}\n");

    } else {
        if (idl_typeSpecType(actualType) == idl_tseq) {
            /* \TODO */
        } else {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s *sv = (%s *)from;\n",
                idl_corbaCxxTypeFromTypeSpec(actualType),
                idl_corbaCxxTypeFromTypeSpec(actualType));
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s *tv = (%s *)to;\n",
                idl_corbaCxxTypeFromTypeSpec(actualType),
                idl_corbaCxxTypeFromTypeSpec(actualType));
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"for (::DDS::ULong i = 0; i < %d; i++) {\n", totalDim);
            idl_printIndent(arg->indent_level+1);
            idl_fileOutPrintf(idl_fileCur(),"tv[i] = sv[i];\n");
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"}\n");
        }
    }
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");

    /* Generate implementation _dup() method */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"%s_slice *%s_dup(const %s_slice *from)\n",
        name, name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"{\n");
    arg->indent_level++;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"%s_slice *to = %s_alloc();\n", name, name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"%s_copy(to, from);\n", name);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"return to;\n");
    arg->indent_level--;
    idl_fileOutPrintf(idl_fileCur(),"}\n\n");
}

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
        macro[i] = toupper(basename[i]);
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;

    /* First initialize userData */
    arg->indent_level = 0;

    /* Generate inclusion of type definition file */
    idl_fileOutPrintf(idl_fileCur(), "#include \"%s.h\"\n", name);
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/* @brief callback function called on closing the IDL input file.
 *
 * Generate standard file footer consisting of:
 * - mutiple inclusion prevention closure
 */
static void
idl_fileClose (
    void *userData)
{
    /* \TODO: deinit userData */
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
 * This maps on namespaces in c++
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
    return idl_explore;
}

static void
idl_moduleClose(
    void *userData)
{
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    char *memberName;
    char *scopedName;
    char *seqName;

    switch (idl_typeSpecType(typeSpec)) {
    case idl_tbasic:
    case idl_tenum:
    case idl_ttypedef:
    case idl_tstruct:
    case idl_tunion:
    case idl_tseq:
        /* nothing todo */
    break;
    case idl_tarray:
        /* generate implementation code for the array mapping */
        memberName = idl_cxxId(name);
        snprintf(arg->buffer,MAX_BUFFER,"_%s_array",memberName);
        seqName = os_strdup(arg->buffer);
        scopedName = idl_scopeStackCxx(scope, "::", seqName);
        idl_arrayTypeImpl(idl_typeArray(typeSpec), scopedName,arg);
        snprintf(arg->buffer, MAX_BUFFER, "_%s_array %s;\n", memberName, memberName);
        os_free(seqName);
        os_free(scopedName);
        os_free(memberName);
    break;
    default:
        printf("idl_structureMemberOpenClose: Unsupported structure member type (member name = %s, type name = %s)\n",
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    char *seqName;

    switch (idl_typeSpecType(typeSpec)) {
    case idl_tbasic:
    case idl_tenum:
    case idl_ttypedef:
    case idl_tstruct:
    case idl_tunion:
    break;
    case idl_tarray:
        /* generate code for an array mapping */
        snprintf(arg->buffer,MAX_BUFFER,"_%s_array", name);
        seqName = os_strdup(arg->buffer);
        idl_arrayTypeImpl(idl_typeArray(typeSpec),seqName,arg);
        os_free(seqName);
    break;
    case idl_tseq:
    break;
    default:
        printf("idl_unionCaseOpenClose: Unsupported union member type (member name = %s, type name = %s)\n",
            name, idl_scopedTypeName(typeSpec));
    }
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    idl_typeSpec actualType;
    idl_typeSpec refType;
    const char *refName;
    char *scopedName;

    actualType = idl_typeDefActual(defSpec);
    refType = idl_typeDefRefered(defSpec);
    refName = idl_typeSpecName(refType);
    scopedName = idl_corbaCxxTypeFromTypeSpec(idl_typeSpec(defSpec));
    if (idl_typeSpecType(refType) == idl_tarray) { /* ex. typedef long myArr[22]; */
        idl_arrayTypeImpl(idl_typeArray(refType), scopedName, arg);
    } else {
        if (idl_typeSpecType(actualType) == idl_tarray) {
            /* Generate implementation _alloc() method */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s_slice *%s_alloc()\n", scopedName, scopedName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"{\n");
            idl_printIndent(arg->indent_level+1);
            idl_fileOutPrintf(idl_fileCur(),"return (%s_slice *)%s_alloc();\n",
                scopedName, refName);
            idl_printIndent(arg->indent_level+1);
            idl_fileOutPrintf(idl_fileCur(),"}\n\n");

            /* Generate implementation _free() method:
             * Implementation is independent of the actual type.
             */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"void %s_free(%s_slice *s)\n", scopedName, scopedName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"{\n");
            idl_printIndent(arg->indent_level+1);
            idl_fileOutPrintf(idl_fileCur(),"%s_free(s);\n", refName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"}\n\n");

            /* Generate implementation _copy() method */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"void *%s_copy(%s_slice *to, const %s_slice *from)\n",
                scopedName, scopedName, scopedName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"{\n");
            idl_printIndent(arg->indent_level+1);
            idl_fileOutPrintf(idl_fileCur(),"%s_copy(to, from);\n", refName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"}\n\n");

            /* Generate implementation _dup() method */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s_slice *%s_dup(const %s_slice *from)\n",
                scopedName, scopedName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"{\n");
            arg->indent_level++;
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s_slice *to = %s_alloc();\n", scopedName, scopedName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"%s_copy(to, from);\n", refName);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"return to;\n");
            arg->indent_level--;
            idl_fileOutPrintf(idl_fileCur(),"}\n\n");
        }
    }
    os_free(scopedName);
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSACPPLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    return &idl_genSACPPLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSacppTypeImpl = {
    idl_getControl,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    idl_moduleClose,
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* dl_enumerationElementOpenClose,*/
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
    &idl_genSACPPData  /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSacppTypeImplProgram(
    void)
{
    return &idl_genSacppTypeImpl;
}

/*
 * Implementation Considerations:
 *
 * - Mapping structures:
 * The members of a structure have a type, which can be arrays, sequences, strings
 * or unions. For these kind of types additional type definitions have to be generated.
 * These additional typedefs can be generated within the scope of the structure, but not
 * all compilers can handle this. Therefore we will generate these type-defs at the module
 * level of the structure. The structure definition it self will be built on heap and written
 * to the output file in the idl_structureClose() callback.
 *
 */
