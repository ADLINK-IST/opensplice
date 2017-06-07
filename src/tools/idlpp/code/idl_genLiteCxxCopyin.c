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
/**
 * @file
 * This module generates Lite C++ CopyIn functions. It handles input types
 * that match the IDL/C++ mapping as specified by the OMG and writes the
 * data into data types as applicable for Lite.
 */

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genLiteCxxCopyin.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genLiteHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"

#include "c_typebase.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"

/** Text indentation level (4 spaces per indent) */
static c_long loopIndent;
    /** Index for array loop variables, incremented for each array dimension */
static c_long varIndex;

static c_char *
idl_seqAllocBuffer(
    idl_typeSeq typeSeq,
    const c_char *length,
    char *buffer,
    size_t bsize)
{
    char* tmp_string;
    idl_typeSpec nextType = idl_typeSeqType(typeSeq);

    tmp_string = idl_scopedLiteTypeName(nextType);
    snprintf(buffer, bsize, "(%s *)dds_alloc(%s * sizeof(%s))",
            tmp_string,
            length,
            tmp_string);

    return buffer;
}

#if 0
static void
idl_arrayDimensions (
    idl_typeArray typeArray,
    os_boolean resolveTypedefs);
#endif

static void
idl_arrayElements (
    idl_scope scope,
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_bool indirect,
    c_long indent,
    void *userData);

#if 0
static void
idl_arrayElementsIndirect (
    idl_scope scope,
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_long indent,
    void *userData);
#endif

static void idl_seqElements (idl_scope scope, const char *name, idl_typeDef typeDef, c_long indent, void *userData);
static void idl_seqLoopCopy (idl_scope scope, idl_typeSpec typeSpec, const char *from, const char *to, c_long loop_index, c_long indent, void *userData);

/** @brief callback function called on opening the IDL input file.
 *
 * Generate code to include standard include files.
 *
 * @param scope Current scope (not used)
 * @param name Name of the IDL input file (not used)
 * @return Next action for this file (idl_explore)
 */
static idl_action
idl_fileOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf (idl_fileCur(), "\n");

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
 * No action is required for IDL modules.
 *
 * @param scope Current scope (and scope of the module definition)
 * @param name Name of the defined module
 * @return Next action for this module (idl_explore)
 */
static idl_action
idl_moduleOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

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
 * For structures a copyIn routine named __<scope-elements>_<structure-name>__copyIn
 * will be prepared. The signature of this copyIn routine is:
 * @verbatim
    void copyIn (,
        const void *from,
        void *to);
    @endverbatim
 *
 * The copyIn routine signature is generated based upon the input
 * parameters which specify the scope and it's name.
 *
 * @param scope Current scope (and scope of the structure definition)
 * @param name Name of the structure
 * @param structSpec Specification of the struct holding the amount of members (not used)
 * @return Next action for this struct (idl_explore)
 */
static idl_action
idl_structureOpen (
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf (idl_fileCur(), "void\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n",
            idl_scopeStack (scope, "_", name));

    idl_fileOutPrintf (idl_fileCur(), "    const void *_from,\n");
    idl_fileOutPrintf (idl_fileCur(), "    void *_to)\n");

    idl_scopeStack(scope, "_", name);
    idl_fileOutPrintf (idl_fileCur(), "{\n");

    idl_fileOutPrintf (idl_fileCur(), "    const struct %s *from;\n",
                       idl_scopeStackCxx (scope, "::", name));

    idl_fileOutPrintf (idl_fileCur(), "    %s *to;\n\n",
                       idl_scopeStack(scope, "_", name));

    idl_fileOutPrintf (idl_fileCur(), "    from = (const struct %s *)_from;\n",
                       idl_scopeStackCxx (scope, "::", name));

    idl_fileOutPrintf (idl_fileCur(), "    to = (%s *)_to;\n\n",
                       idl_scopeStack(scope, "_", name));


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
 * The function finalizes the copyIn routine for the structure by
 * generating the closing bracket ('}').
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

    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
}

/** @brief Generate copy statement for elements of basic type.
 *
 * The function generates for the provided basic type a copyIn
 * statement. The identification of the source and destination
 * elements must be provided by the caller.
 *
 * Basic type elements.
 *
 * @param scope Scope of the element
 * @param name Name of the element
 * @param typeBasic Specification of the type of the element
 * @param from_id Identification of the source
 * @param Identification of the destination
 */
static void
idl_basicMemberType (
    idl_scope scope,
    const char *name,
    idl_typeBasic typeBasic,
    const char *from_id,
    const char *to_id)
{
    c_ulong maxlen;
    c_char* cid;

    OS_UNUSED_ARG(scope);

    cid = idl_cxxId(name);

    switch (idl_typeBasicType(typeBasic)) {
    case idl_short:
    case idl_ushort:
    case idl_long:
    case idl_ulong:
    case idl_longlong:
    case idl_ulonglong:
    case idl_float:
    case idl_double:
    case idl_char:
    case idl_boolean:
    case idl_octet:
        idl_fileOutPrintf (idl_fileCur(), "    %s%s = (%s)(%s%s);\n",
                to_id,
                cid,
                idl_getLiteBasicTypeName(typeBasic),
                from_id,
                cid);
        break;
    case idl_string:
        maxlen = idl_typeBasicMaxlen(typeBasic);
        if (maxlen == 0) {
#if 0
            idl_fileOutPrintf(idl_fileCur(), "    %s%s = dds_string_dup(%s%s);\n",
                    to_id,
                    cid,
                    from_id,
                    cid);
#else
            idl_fileOutPrintf(idl_fileCur(), "    %s%s = (char *)((%s%s).in());\n",
                    to_id,
                    cid,
                    from_id,
                    cid);
#endif

        } else {
            /* TODO: bounded strings */
        }
        break;
    default:
        printf ("idl_basicMemberType: Unexpected basic type\n");
        break;
    }

    os_free(cid);
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
 * Depending on the type of the structure member, a copy strategy is used:
 * - If the type is \b idl_tbasic, \b idl_basicMemberType is called to generate the
 *   copy code.
 * - If the type is \b idl_typedef the copy strategy depends on the referred type.
 *   When this type is \b idl_tarray or \b idl_tseq a type specific copy routine
 *   is called. When type is something else, this routine is called recursively with
 *   with the referred type.
 * - If the type is \b idl_tenum, the element is immediately assigned.
 * - If the type is \b idl_tstruct or \b idl_tunion, the copyIn routine of that struct
 *   or union type is called.
 * - If the type is \b idl_tarray the array copy context is set and the service
 *   to copy arrays is called.
 * - If the type is \b idl_tseq, the sequence copy context is setup and the
 *   service to copy sequences is called.
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
    c_char buffer[256];
    c_ulong maxlen;
    c_char *scopedName;
    c_char* cid;
    c_long indent = 1;

    cid = idl_cxxId(name);

    /* Expected types: idl_tbasic, idl_ttypedef, idl_tenum, idl_tstruct, idl_tunion, idl_tarray, idl_tseq */

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        idl_basicMemberType (scope, name, idl_typeBasic(typeSpec), "from->", "to->");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        /* QAC EXPECT 3416; No side effect here */
        if ((idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tstruct) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tseq)) {
            idl_fileOutPrintf (idl_fileCur(), "    {\n");
            IDL_PRINTLINE (indent);
            idl_fileOutPrintf (idl_fileCur(), "        extern void __%s__copyIn(const %s *, %s *);\n",
                    idl_scopedTypeName (typeSpec),
                    idl_corbaCxxTypeFromTypeSpec(typeSpec),
                    idl_scopedLiteTypeName (typeSpec));
            idl_fileOutPrintf (idl_fileCur(), "        __%s__copyIn(&from->%s, &to->%s);\n",
                    idl_scopedTypeName (typeSpec),
                    cid,
                    cid);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
        } else {
            /* Calls itself for the actual type in case of typedef */
            /* QAC EXPECT 3670; We wan't to use recursion, the recursion is finite */
            idl_structureMemberOpenClose (
                    scope,
                    name,
                    idl_typeDefActual(idl_typeDef(typeSpec)),
                    userData);

        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        idl_fileOutPrintf (idl_fileCur(), "    to->%s = (%s)from->%s;\n",
            cid,
            idl_scopedTypeName (typeSpec),
            cid);
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        IDL_PRINTLINE (indent);
        idl_fileOutPrintf (idl_fileCur(), "        extern void __%s__copyIn(const void *, void *);\n",
            idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "        __%s__copyIn(&from->%s, &to->%s);\n",
            idl_scopedTypeName (typeSpec),
            cid,
            cid);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tunion) {
        /* TODO: support unions */
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        c_char dest[256];
        c_char src[256];
        varIndex = 0;
        snprintf(dest, sizeof(dest), "to->%s", name);
        snprintf(src, sizeof(src), "from->%s", name);
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        IDL_PRINTLINE (indent);
#if 0
        {
            idl_typeSpec subType = idl_typeArrayActual(idl_typeArray(typeSpec));
            if (idl_typeSpecType(subType) == idl_tseq) {
                c_char sname[32];
                snprintf(sname, sizeof(sname), "_%s", cid);
                idl_fileOutPrintf (idl_fileCur(), "        typedef const %s _SrcType",
                                   idl_scopeStack(idl_typeUserScope(idl_typeUser(subType)), "::", sname));
                //idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
                idl_fileOutPrintf (idl_fileCur(), ";\n");
                idl_fileOutPrintf (idl_fileCur(), "        _SrcType *src = &from->%s;\n", cid);
            }
        }
#endif

        idl_arrayElements (scope, idl_typeArray(typeSpec), src, dest, FALSE, indent, userData);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");

        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
        idl_typeSpec nextType;

        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedLiteTypeName(nextType);

        snprintf (type_name, sizeof(type_name), "_%s_seq", name);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));

        if (maxlen == 0) {
            idl_fileOutPrintf (idl_fileCur(), "    {\n");
#if 0
            IDL_PRINTLINE (indent);
            idl_fileOutPrintf (idl_fileCur(), "        const %s *src = &from->%s;\n", idl_scopeStackCxx (scope, "::", type_name), cid);
            idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
            idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");
            idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(typeSpec));
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->%s._maximum = (uint32_t)(*src).maximum();\n", cid);
            idl_fileOutPrintf (idl_fileCur(), "        to->%s._length  = length0;\n", cid);
            idl_fileOutPrintf (idl_fileCur(), "        to->%s._buffer  = (uint8_t *)dest0;\n", cid);
            idl_fileOutPrintf (idl_fileCur(), "        to->%s._release = false;\n", cid);
#endif
#if 1
            if (((idl_typeSpecType(nextType) == idl_tbasic) &&
                 (idl_typeBasicType(idl_typeBasic(nextType)) != idl_string)) ||
                (idl_typeSpecType(nextType) == idl_tenum)) {
                 IDL_PRINTLINE (indent);
                 idl_fileOutPrintf (idl_fileCur(), "        const %s *src = &from->%s;\n\n", idl_scopeStackCxx (scope, "::", type_name), cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._maximum = (uint32_t)(*src).maximum();\n", cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._length  = (uint32_t)(*src).length();\n", cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._buffer  = (uint8_t *)(*src).get_buffer();\n", cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._release = false;\n", cid);
            } else {
                 IDL_PRINTLINE (indent);
                 idl_fileOutPrintf (idl_fileCur(), "        const %s *src = &from->%s;\n", idl_scopeStackCxx (scope, "::", type_name), cid);
                 idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
                 idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");
                 idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
#if 0
                 idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(typeSpec));
#else
                 idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s;\n",
                         idl_seqAllocBuffer(idl_typeSeq(typeSpec), "length0", buffer, sizeof(buffer)));
#endif
                 idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._maximum = (uint32_t)(*src).maximum();\n", cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._length  = length0;\n", cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._buffer  = (uint8_t *)dest0;\n", cid);
                 idl_fileOutPrintf (idl_fileCur(), "        to->%s._release = false;\n", cid);
            }
#endif
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
        } else {
            /* TODO: bounded sequences */
        }

    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    /* QAC EXPECT 5103; Code is clearly separated in a number of cases of which each is maintainable */

    os_free(cid);
}

#if 0
/** @brief Generate the array dimensions.
 *
 * Generate the dimensions of the array:
 * @verbatim
   [<size-dimension-first>][<size-dimension-first+1>]..[<size-dimension-last>]
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 */
static void
idl_arrayDimensions (
    idl_typeArray typeArray,
    os_boolean resolveTypedefs)
{
    idl_typeSpec subType;

    idl_fileOutPrintf (idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
    subType = idl_typeArrayType(typeArray);
    while(resolveTypedefs && idl_typeSpecType(subType) == idl_ttypedef)
    {
        subType = idl_typeDefResolveFully(subType);
    }
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(subType) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayDimensions (idl_typeArray(subType), resolveTypedefs);
    }
}
#endif

/** @brief Generate the loop variables for the array copy.
 *
 * Generate the loop variables for the array copy. Per dimension a
 * loop variable named i# is required, where # represents the dimension.
 * (I.e. first dimension handled by i0, second by i1, etc.)
 * @verbatim
   int i0; // dimension-first
   int i1; // dimension-first+1
   ..
   int i#; // dimension-last
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param indent Indentation of the output
 */
static void
idl_arrayLoopVariables (
    idl_typeArray typeArray,
    c_long indent)
{
    loopIndent++;
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    int i%d;\n", loopIndent-1);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopVariables(idl_typeArray(idl_typeArrayType(typeArray)), indent);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    loopIndent--;
}

/** @brief Generate copy loop for an array.
 *
 * Generate the for statement for copying an array.
 * The variable \b loopIndent indicates the actual loop variable for this dimension.
 * The variable indent indicates the positional index for the loop.
 * @verbatim
   for (i0 = 0; i0 < <size-dimension-first>; i0++) {
       for (i1 = 0; i1 < <size-dimension-first+1>; i1++) {
           ..
               for (i# = 0; i# < <size-dimension-last>; i#++) {
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param indent Indentation of the output
 */
static void
idl_arrayLoopCopyOpen (
    idl_typeArray typeArray,
    c_long indent)
{
    loopIndent++;
    idl_printIndent(loopIndent + indent);
    idl_fileOutPrintf(idl_fileCur(), "for (i%d = 0; i%d < %d; i%d++) {\n",
    loopIndent-1,
    loopIndent-1,
    idl_typeArraySize(typeArray),
    loopIndent-1);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyOpen(idl_typeArray(idl_typeArrayType(typeArray)), indent);
    }
}

/** @brief Generate array index for copying array elements.
 *
 * Generate array index to the default output file.
 *
 * @verbatim
    [i0][i1]..[i#]
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 */
static void
idl_arrayLoopCopyIndex (
    idl_typeArray typeArray)
{
    varIndex++;
    idl_fileOutPrintf(idl_fileCur(), "[i%d]", varIndex-1);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyIndex(idl_typeArray(idl_typeArrayType(typeArray)));
    }
    varIndex--;
}

/** @brief Generate array index for copying array elements.
 *
 * Generate array index to string.
 *
 * @verbatim
    [i0][i1]..[i#]
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 */
static void
idl_arrayLoopCopyIndexString (
    c_char *indexString,
    idl_typeArray typeArray)
{
    c_char arrIndex[16];

    varIndex++;
    snprintf(arrIndex, sizeof(arrIndex), "[i%d]", varIndex-1);
    os_strncat(indexString, arrIndex, sizeof(arrIndex));
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyIndexString(indexString, idl_typeArray(idl_typeArrayType(typeArray)));
    }
    varIndex--;
}

/** @brief Determine the number of dimensions of the identified array
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @return The number of dimensions of the array
 */
static c_long
idl_indexSize(
    idl_typeArray typeArray)
{
    c_long return_val = 1;

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        return_val =  idl_indexSize (idl_typeArray(idl_typeArrayType(typeArray)))+1;
    }
    return return_val;
}

/** @brief Generate copy body for an array element.
 *
 * Generate the body for copying an array element.
 *
 * Depending on the actual type of the array, one of the following strategies will
 * be followed.
 * - If the actual type is \b idl_tstruct or \b idl_tunion call the copy routine
 *   for the struct or union.
 * - If the actual type is \b idl_ttypedef depending on the actual type of the typedef,
 *   for \b idl_tstruct, \b idl_tunion, \b idl_tarray and \b idl_tseq call the copy
 *   routine for the type. If the actual type is \b idl_tbasic (only if it is a string),
 *   the string is duplicated.
 * - If the actual type is \b idl_tbasic (only a string is expected) a loop
 *   to copy the strings is created.
 * - If the actual type is \b idl_tseq (only a string is expected) a loop
 *   to copy the sequence is created.
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param from Identification of the source to copy from
 * @param to Identification of the destination to copy to
 * @param indent Indentation of the output
 */
static void
idl_arrayLoopCopyBody(
    idl_typeArray typeArray,
    idl_typeSpec typeSpec,
    const char *from,
    const char *to,
    c_long indent,
    idl_scope scope,
    void *userData)
{
    c_char buffer[256];
    idl_typeSpec nextType;
    c_ulong maxlen;
    c_long total_indent;

    loopIndent++;
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tstruct:
    case idl_tunion:
        varIndex = 0;
        IDL_PRINTLINE (loopIndent + indent);
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf(idl_fileCur(), "extern void __%s__copyIn(const void *, void *)\n\n",
                idl_scopedTypeName(typeSpec));

        idl_printIndent (loopIndent + indent);
        idl_fileOutPrintf (idl_fileCur(),"__%s__copyIn(&(%s)",
                idl_scopedTypeName(typeSpec), from);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ", &(%s)", to);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ");\n");
        break;
    case idl_ttypedef:
        {
            idl_typeSpec actualType = idl_typeDefActual(idl_typeDef(typeSpec));

            switch (idl_typeSpecType(actualType)) {
            case idl_tbasic:
                if (idl_typeBasicType(idl_typeBasic(actualType)) == idl_string) {
                    maxlen = idl_typeBasicMaxlen(idl_typeBasic(actualType));
                    if (maxlen == 0) {
                        IDL_PRINTLINE (loopIndent + indent);
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf(idl_fileCur(), "(%s)", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf(idl_fileCur(), " = dds_string_dup(((%s))", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf(idl_fileCur(), ");\n");
#if 0
                        idl_fileOutPrintf(idl_fileCur(), " = (char *)(%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf(idl_fileCur(), ".in();\n");
#endif
                    } else {
                        /* TODO: support bounded strings */
                    }
                } else {
                    assert (0);
                }
                break;
            case idl_tstruct:
            case idl_tunion:
                IDL_PRINTLINE (loopIndent + indent-1);
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "extern void __%s__copyIn(const void *, void *)\n\n",
                    idl_scopedTypeName(typeSpec));
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(),"__%s__copyIn(&(%s)",
                    idl_scopedTypeName(typeSpec),
                    from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ", &(%s)",
                    to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ");\n");
                indent--;
                break;
            break;
            case idl_tseq:
            case idl_tarray:
                IDL_PRINTLINE (loopIndent + indent-1);
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "extern void __%s__copyIn(",
                    idl_scopedTypeName(typeSpec));
                idl_fileOutPrintf(idl_fileCur(), "const %s *,",
                    idl_scopeStack(idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec)));
                idl_fileOutPrintf(idl_fileCur(), "%s *);\n\n",
                    idl_scopedLiteTypeName(typeSpec));
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(),"__%s__copyIn((%s *)&(%s)",
                    idl_scopedTypeName(typeSpec),
                    idl_corbaCxxTypeFromTypeSpec(typeSpec),
                from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ", (%s *)&(%s)",
                    idl_scopedLiteTypeName(typeSpec),
                to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ");\n");
                break;
            default:
                printf ("idl_arrayLoopCopyBody: Unexpected type\n");
                /* QAC EXPECT 3416; No side effect here */
                assert (0);
                break;
            }
        }
        break;
    case idl_tbasic:
        /* This may only be string */
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
            if (maxlen == 0) {
                IDL_PRINTLINE (loopIndent + indent);
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "(%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), " = dds_string_dup((%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ");\n");
#if 0
                idl_fileOutPrintf(idl_fileCur(), " = (char *)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ".in();\n");
#endif
            } else {
                /* TODO: support bounded strings */
            }
        } else {
            /* QAC EXPECT 3416; No side effect here */
            printf ("idl_arrayLoopCopyBody: Unexpected type\n");
            assert (0);
        }

        break;
    case idl_tseq:
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));

        total_indent = indent+idl_indexSize(typeArray);

        if (maxlen == 0) {
             char source[256];

             snprintf(source, sizeof(source), "(%s)", from);
             idl_arrayLoopCopyIndexString(source, typeArray);

             IDL_PRINTLINE (total_indent);
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    uint32_t length0;\n");
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    %s *dest0;\n\n",
                     idl_scopedLiteTypeName(nextType));
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    length0 = (uint32_t)(%s)", from);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(), ".length();\n");
             idl_printIndent (total_indent);
#if 0
             idl_fileOutPrintf (idl_fileCur(), "    dest0 = %s_allocbuf(length0)\n", idl_scopedLiteTypeName(typeSpec));
#else
             idl_fileOutPrintf (idl_fileCur(), "    dest0 = %s;\n",
                     idl_seqAllocBuffer(idl_typeSeq(typeSpec), "length0", buffer, sizeof(buffer)));
#endif
             idl_seqLoopCopy (scope, nextType, source, "dest0", 1, loopIndent+1, userData);
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(), "._maximum = (uint32_t)(%s)", from);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(), ".maximum();\n");
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(), "._length  = (uint32_t)(%s)", from);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(),  ".length();\n");
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(), "._buffer = (uint8_t *)dest0;\n");
             idl_printIndent (total_indent);
             idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
             idl_arrayLoopCopyIndex(typeArray);
             idl_fileOutPrintf (idl_fileCur(), "._release = false;\n");

#if 0
            if (((idl_typeSpecType(nextType) == idl_tbasic) &&
                 (idl_typeBasicType(idl_typeBasic(nextType)) != idl_string)) ||
                (idl_typeSpecType(nextType) == idl_tenum)) {
                IDL_PRINTLINE (total_indent);
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._maximum = (uint32_t)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ".maximum();\n");
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._length  = (uint32_t)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ".length();\n");
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._buffer  = (uint8_t *)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ".get_buffer();\n");
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._release = false;\n");
            } else {
                char source[256];

                snprintf(source, sizeof(source), "(%s)", from);
                idl_arrayLoopCopyIndexString(source, typeArray);

                IDL_PRINTLINE (total_indent);
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    uint32_t length0;\n");

                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    %s *dest0;\n\n",
                        idl_scopedLiteTypeName(nextType));
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    length0 = (uint32_t)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ".length();\n", from);
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    dest0 = %s_allocbuf(length0)\n", idl_scopedLiteTypeName(typeSpec));
                idl_seqLoopCopy (scope, nextType, source, "dest0", 1, loopIndent, userData);
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._maximum = (uint32_t)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ".maximum();\n");
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._length  = (uint32_t)(%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(),  ".length();\n", to);
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._buffer = (uint8_t *)dest0;\n", to);
                idl_printIndent (total_indent);
                idl_fileOutPrintf (idl_fileCur(), "    (%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "._release = false;\n");
            }
#endif
        } else {
            /* TODO: bounded sequences */
        }
        break;
    default:
        /* QAC EXPECT 3416; No side effect here */
        assert (0);
        break;
    }
    loopIndent--;
    /* QAC EXPECT 5101, 5103: Complexity is limited, by independent cases, per case the number of lines is lower  */
}

/** @brief Generate array loop copy closing code.
 *
 * For each dimension a closing bracket '}' will be generated at the
 * correct indent level.
 *
 * @verbatim
               }
           ..
       }
   }
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param indent Indentation of the loop.
 */
static void
idl_arrayLoopCopyClose (
    idl_typeArray typeArray,
    c_long indent)
{
    idl_printIndent (loopIndent + indent);
    loopIndent--;
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyClose (idl_typeArray(idl_typeArrayType(typeArray)), indent);
    }
}

/** @brief Generate copy loops for array.
 *
 * For each array dimension generate a loop copy action.
 * An array copy action is construct of:
 * - a loop variable (idl_arrayLoopVariables)
 * - a loop statement (idl_arrayLoopCopyOpen)
 * - copying the array element (idl_arrayLoopCopyBody)
 * - closing the loop statement (idl_arrayLoopCopyClose)
 *
 * This results in the following pattern for copying arrays per element:
 * @verbatim
   int i0;
   int i1;
   ..
   int i#;

   for (i0 = 0; i0 < <size-dimension-first>; i0++) {
       for (i1 = 0; i1 < <size-dimension-first+1>; i1++) {
           ..
               for (i# = 0; i# < <size-dimension-last>; i#++) {
                    // copy array element
                    to[i0][i1]..[i#] = from[i0][i1]..[i#].in() // for string
                    __<type>__copyIn (&from[i0][i1]..[i#], &to[i0][i1]..[i#]) // for struct, union
                    .. // for sequence, see sequence copy construction
               }
           }
       ..
   }
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param from Identification of the source to copy from
 * @param to Identification of the destination to copy to
 * @param indent Indentation of the output
 */
static void
idl_arrayLoopCopy (
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_long indent,
    idl_scope scope,
    void *userData)
{
    loopIndent = 0;
    idl_arrayLoopVariables(typeArray, indent);
    idl_arrayLoopCopyOpen(typeArray, indent);
    idl_arrayLoopCopyBody(typeArray, idl_typeArrayActual(typeArray), from, to, indent, scope, userData);
    idl_arrayLoopCopyClose(typeArray, indent);
}

/** @brief Generate code for copying an array.
 *
 * This function is the top level function for copying an array type.
 * This function determines the strategy to use, copying by plain
 * memory copy (for plain types), or copying element for element (for
 * types that contain references).
 *
 * The copy strategy depends on the actual type of the array:
 * - If the actual type is \b idl_tbasic, the depending if it is a
 *   string or another basic type, an element copy or a plain memory copy
 *   is used respectively.
 * - If the actual type is \b idl_tenum, a plain memory copy is used.
 * - If the actual type is \b idl_tstruct or \b idl_tunion, an element
 *   copy is used.
 * - If the actual type is \b idl_ttypedef, the strategy depends on the
 *   actual type of the typedef.
 * - If the actual type is \b idl_tseq, the array will be copied on
 *   element basis.
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param from Identification of the source to copy from
 * @param to Identification of the destination to copy to
 * @param indirect Indentifies if the provided to and from are pointers of not
 * @param indent Indentation of the output
 */
static void
idl_arrayElements (
    idl_scope scope,
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_bool indirect,
    c_long indent,
    void *userData)
{
    idl_typeSpec subType = idl_typeArrayActual(typeArray);

    if (indirect) {
        if (idl_isContiguous(idl_typeSpecDef(subType))) {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, from);
        } else {
            char *dest;
            char *src;

            src = os_malloc(strlen(from) + 2);
            os_sprintf(src, "*%s", from);
            dest = os_malloc(strlen(to) + 2);
            os_sprintf(dest, "*%s", to);
            idl_arrayLoopCopy (typeArray, src, dest, indent, scope, userData);
            os_free(src);
            os_free(dest);
        }
    } else {

        if (idl_isContiguous(idl_typeSpecDef(subType))) {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (&(%s), &(%s), sizeof (%s));\n", to, from, from);
        } else {
            idl_arrayLoopCopy (typeArray, from, to, indent, scope, userData);
        }
    }
}


/** @brief Function that generates index for addressing sequence elements.
 *
 * Generate code for addressing sequenece elements by means of the index
 * variables i0 .. in.
 *
 * The index is organized as follows:
 * @verbatim
   [i0][i1]..[in]
   @endverbatim
 *
 * @param indent Sequence recursion level
 * @return image addressing the current sequence element
 */
static char *
idl_seqIndex (
    c_long indent)
{
    c_char index[64];
    c_char is[64];
    c_long i;

    index[0] = 0;
    for (i = 0; i < indent; i++) {
        snprintf (is, sizeof(is), "[j%d]", i);
        /* Remaining size is (total size) - (current string size) - (sizeof('\0')). */
        os_strncat(index, is, sizeof(index) - strlen(index) - 1);
    }
    return os_strdup(index);
}

/** @brief Function that generates code for copying sequences.
 *
 * Generate code for copying sequence types.
 *
 * The general pattern for copying sequences is as follows:
 * @verbatim
   unsigned int i0;
   long length0;
   <scoped-sequence-element-type-name> *dest0;

   length0 = (long)(*src0).length();
   dest0 = (<scoped-sequence>_allocbuf();
   // Body to copy sequence contents from <source-id> to dest0
   <destination-id> = dest0;
   @endverbatim
 *
 * Note that in C++, nested sequences are always anonymous. Thus sequence copy routines in
 * C++ need to nest the copy actions (in contrast with C).
 * The variable names used "i0", "length0", "dst0" and "src0", suggest otherwise.
 *
 * This routine only generates the copying code for copying the sequence contents.
 * The context around it as depicted above is expected to be generated by the
 * calling function.
 * Motivation for this is that are many variations on the pattern depending on
 * the situation, for instance if it is an array of an sequence, an array copy loop
 * must be coded around it.
 *
 * The body to copy the sequence depends on the actual type of the sequence, however
 * the framework for copying is always the same:
 * @verbatim
   for (i0 = 0; i0 < (unsigned int)length0; i0++) {
       // Copy sequence element (type specific)
   }
   @endverbatim
 *
 * The element copyIn strategy is as follows:
 * - If the sequence element type is \b idl_tenum, direct assignment is used
 * - If the sequence element type is \b idl_tbasic, depending on whether it is
 *   a string or not, elements are duplicated via c_newString or otherwise
 *   immediately assigned
 * - If the sequence element type is \b idl_tstruct or \b idl_tunion the copyIn
 *   function for that struct or union is called.
 * - If the sequence element type is \b idl_ttypedef the strategy depends on the
 *   actual type of the typedef. For structs unions and arrays the copyIn routine
 *   for that type is called. For basic string type, the string is duplicated
 *   via c_stringNew. For other basic types and enumeration it is just assigned.
 * - If the sequence element type is \b idl_tseq, a new sequence copy operation
 *   is setup for the specific sequence type.
 *
 * Structure types (not unions) that do not contain any reference types are
 * copied via a plain memory copy because the sequence elements are located in
 * consequtive memory with the same memory map for C CORBA as well as Lite.
 * These are identified using idl_isContiguous().
 *
 * @param typeSpec The specification of the actual type of the sequence
 * @param from Specifies the identification of the source
 * @param to Specifies the identification of the destination
 * @param loop_index Specifies the index if the variables when sequence copy
 * routines are nested. The top level sequence starts with index 0.
 * @param indent Specifies the indentation level
 */
static void
idl_seqLoopCopy (
    idl_scope scope,
    idl_typeSpec typeSpec,
    const char *from,
    const char *to,
    c_long loop_index,
    c_long indent,
    void *userData)
{
    c_char buffer[256];
    c_char destin[32];
    c_char length[32];
    c_char *scopedName;
    idl_typeSpec nextType;
    c_ulong maxlen;

    if (idl_isContiguous(idl_typeSpecDef(typeSpec))) {
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        IDL_PRINTLINE (indent);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    const %s *buf%d;\n",
                idl_scopedLiteTypeName(typeSpec),loop_index-1);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    buf%d = (const %s *)(%s)%s.get_buffer();\n",
                loop_index-1, idl_scopedLiteTypeName(typeSpec), from, idl_seqIndex (loop_index-1));

        if(idl_typeSpecType (typeSpec) == idl_tenum){
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s,buf%d,length%d* sizeof(*%s));\n",
                to, loop_index-1, loop_index-1, to);
        } else {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s,buf%d,length%d* sizeof(*%s));\n",
                to, loop_index-1, loop_index-1, to);
        }

        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        return;
    }

    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    IDL_PRINTLINE (indent);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    uint32_t j%d;\n", loop_index-1);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    for (j%d = 0; j%d < length%d; j%d++) {\n",
        loop_index-1,
        loop_index-1,
        loop_index-1,
        loop_index-1);

    indent++;
    switch (idl_typeSpecType (typeSpec)) {
    case idl_tenum:
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s[j%d] = (%s)(%s)%s;\n",
                to,
                loop_index-1,
                idl_scopedLiteTypeName(typeSpec),
                from,
                idl_seqIndex(loop_index));
        break;
    case idl_tbasic:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
            idl_printIndent (indent);
            if( maxlen != 0) {
                /* TODO: bounded strings */
            } else {
                idl_fileOutPrintf (idl_fileCur(), "    %s[j%d] = dds_string_dup((%s)%s);\n",
                        to,
                        loop_index-1,
                        from,
                        idl_seqIndex(loop_index));
#if 0
                idl_fileOutPrintf (idl_fileCur(), "    %s[j%d] = (%s)%s.in();\n",
                        to,
                        loop_index-1,
                        from,
                        idl_seqIndex(loop_index));
#endif
            }
        }
        break;
    case idl_tstruct:
    case idl_tunion:
        varIndex = 0;
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    extern void __%s__copyIn(const void *, void *);\n",
            idl_scopedTypeName(typeSpec));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    __%s__copyIn(&(%s)%s, (%s *)&%s[j%d]);\n",
            idl_scopedTypeName(typeSpec),
            from,
            idl_seqIndex(loop_index),
            idl_scopedLiteTypeName(typeSpec),
            to,
            loop_index-1);
        break;
    case idl_ttypedef:
        idl_printIndent (indent);
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
        case idl_tstruct:
        case idl_tunion:
        case idl_tarray:
        case idl_tseq:
            idl_fileOutPrintf (idl_fileCur(), "    extern void __%s__copyIn(const %s *, %s *);\n",
                idl_scopedTypeName (typeSpec),
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                idl_scopedTypeName (typeSpec));
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    __%s__copyIn(&(%s)%s, (%s *)&%s[j%d]);\n",
                idl_scopedTypeName(typeSpec),
                from,
                idl_seqIndex(loop_index),
                idl_scopedLiteTypeName(typeSpec),
                to,
                loop_index-1);
            break;
        case idl_tenum:
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    %s[j%d] = (%s)(%s)%s;\n",
                    to,
                    loop_index-1,
                    idl_scopedLiteTypeName(typeSpec),
                    from,
                    idl_seqIndex(loop_index));
            break;
        case idl_tbasic:
            if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                maxlen = idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec))));
                if(maxlen == 0){
                    idl_fileOutPrintf (idl_fileCur(), "    %s[j%d] = dds_string_dup((%s)%s);\n",
                            to,
                            loop_index-1,
                            from,
                            idl_seqIndex(loop_index));
                } else {
                    /* TODO: support bounded strings */
                }
            } else {
                printf ("idl_seqLoopCopy: Unexpected type\n");
                assert (0);
            }
            break;
        default:
            printf ("idl_seqLoopCopy: Unexpected type\n");
            assert (0);
            break;
        }
        break;
    case idl_tseq:
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedLiteTypeName(nextType);

        snprintf (destin, sizeof(destin), "dest%d", loop_index);

        IDL_PRINTLINE (indent);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    uint32_t length%d;\n", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s *dest%d;\n\n", scopedName, loop_index);
        free(scopedName);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    length%d = (uint32_t)(%s)%s.length();\n", loop_index, from, idl_seqIndex(loop_index));
#if 0
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    dest%d = (%s *)%s_allocbuf(length%d);\n",
                loop_index, scopedName, idl_scopedLiteTypeName(typeSpec), loop_index);
#else
        idl_printIndent (indent);
        sprintf(length, "length%d", loop_index);
        idl_fileOutPrintf (idl_fileCur(), "    dest%d = %s;\n",
                loop_index,
                idl_seqAllocBuffer(idl_typeSeq(typeSpec), length, buffer, sizeof(buffer)));
#endif
        idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+1, userData);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    dest%d[j%d]._length = length%d;\n",
                loop_index-1, loop_index-1, loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    dest%d[j%d]._maximum = (uint32_t)(%s)%s.maximum();\n",
                loop_index-1, loop_index-1, from, idl_seqIndex(loop_index));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    dest%d[j%d]._buffer = dest%d;\n",
                loop_index-1, loop_index-1, loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    dest%d[j%d]._release = FALSE;\n",
                loop_index-1, loop_index-1);
        break;
    default:
        assert (0);
        break;
    }
    indent--;
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    }\n");
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "}\n");
}

/** @brief Function that set up the sequence copy context for a typedef.
 *
 * Generate code to setup the sequence copy context for a typedef.
 *
 * @todo Because it is only used by a typedef it would be more clear
 * to integrate it with the typedef. If it can be used for more contexts
 * the others should be removed and call this function.
 *
 * @param scope Current scope
 * @param name Specifies the name of the type
 * @param typeSeq Specifies the type of the sequence
 * @param indent Sequence recursion level
 */
static void
idl_seqElements (
    idl_scope scope,
    const char *name,
    idl_typeDef typeDef,
    c_long indent,
    void *userData)
{
    c_ulong maxlen;
    c_char buffer[256];
    c_char type_name[256];
    c_char *scopedName;
    idl_typeSeq typeSeq;
    idl_typeSpec nextType;
    char* tmp_string;

    OS_UNUSED_ARG(indent);

    typeSeq = idl_typeSeq(idl_typeDefActual(typeDef));
    nextType = idl_typeSeqType(idl_typeSeq(typeSeq));
    scopedName = idl_scopedLiteTypeName(nextType);

    snprintf (type_name, sizeof(type_name), "%s", name);
    maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSeq));

    if (maxlen == 0) {
        IDL_PRINTLINE (indent);
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        tmp_string = idl_scopeStackCxx (scope, "::", type_name);
        idl_fileOutPrintf (idl_fileCur(), "        const %s *src = from;\n", tmp_string);
        free(tmp_string);

        idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");
        idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
#if 0
        idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(idl_typeSpec(typeDef)));
#else
        idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s;\n",
                idl_seqAllocBuffer(typeSeq, "length0", buffer, sizeof(buffer)));
#endif
        idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
        idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
        idl_fileOutPrintf (idl_fileCur(), "        to->_length  = length0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        to->_buffer  = dest0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");


#if 0
        if (idl_typeSpecType(nextType) == idl_tbasic) {
            if (idl_typeBasicType(idl_typeBasic(nextType)) == idl_string) {
                idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
                idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");
                idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
                idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(idl_typeSpec(typeDef)));
                idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
                idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
                idl_fileOutPrintf (idl_fileCur(), "        to->_length  = (uint32_t)(*src).length();\n");
                idl_fileOutPrintf (idl_fileCur(), "        to->_buffer = dest0;\n");
                idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
                idl_fileOutPrintf (idl_fileCur(), "        to->_length  = (uint32_t)(*src).length();\n");
                idl_fileOutPrintf (idl_fileCur(), "        to->_buffer  = (*src).get_buffer();\n");
                idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");
            }
        } else if (idl_typeSpecType(nextType) == idl_tenum) {
            idl_fileOutPrintf (idl_fileCur(), "\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_length  = (uint32_t)(*src).length();\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_buffer  = (*src).get_buffer();\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");
        } else if (idl_typeSpecType(nextType) == idl_tstruct) {
            idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
            idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");
            idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(idl_typeSpec(typeDef)));
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_length  = length0;\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_buffer  = dest0;\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");
        } else if (idl_typeSpecType(nextType) == idl_ttypedef) {
            idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
            idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");

            idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(idl_typeSpec(typeDef)));
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_length  = length0;\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_buffer  = dest0;\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");
        } else if (idl_typeSpecType(nextType) == idl_tseq) {
            idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", idl_scopedLiteTypeName(idl_typeSpec(typeSeq)));
            idl_fileOutPrintf (idl_fileCur(), "        uint32_t length0;\n\n");

            idl_fileOutPrintf (idl_fileCur(), "        length0 = (uint32_t)(*src).length();\n");
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = %s_allocbuf(length0);\n", idl_scopedLiteTypeName(idl_typeSpec(typeDef)));
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->_maximum = (uint32_t)(*src).maximum();\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_length  = length0;\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_buffer  = dest0;\n");
            idl_fileOutPrintf (idl_fileCur(), "        to->_release = false;\n");
        } else {
            /* Do nothing, only to prevent dangling else-ifs QAC reports */
        }
#endif

        idl_fileOutPrintf (idl_fileCur(), "    }\n");
    } else {
        /* TODO: bounded sequences */
    }
}

/** @brief callback function called on definition of a named type in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   typedef <type-name> <name>;
   @endverbatim
 *
 * For typedef's the action to copy the data is dependend on the actual type.
 * - If the actual type is \b idl_tstruct or \b idl_tunion, a named copy routines
 *   is generated. The copyIn routines calls the copyIn routine of the struct or union.
 * - If the actual type is \b idl_tarray, a named copyIn routine is generated.
 *   For that copyIn routine, array copy code is generated.
 * - If the actual type is \b idl_tseq, a named copyIn routine is generated.
 *   For that copyIn routine, sequence copy code is generated.
 *
 * Type definitions of basic type are immediately copied from within the generated
 * struct or union copyIn routine. No named copy routine is required.
 *
 * @param scope Current scope
 * @param name Specifies the name of the type
 * @param defSpec Specifies the type of the named type
 */
static void
idl_typedefOpenClose (
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    switch (idl_typeSpecType(idl_typeDefActual(defSpec))) {
    case idl_tstruct:
    case idl_tunion:
        idl_fileOutPrintf (idl_fileCur(), "void\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n",
                idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "    const void *from,\n");
        idl_fileOutPrintf (idl_fileCur(), "    void *to)\n");
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    extern void __%s__copyIn(const void *, void *);\n",
                idl_scopedTypeName (idl_typeDefActual(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    __%s__copyIn(from, to);\n",
                idl_scopedTypeName (idl_typeDefActual(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tarray:
        idl_fileOutPrintf (idl_fileCur(), "void\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n",
                idl_scopedTypeName (idl_typeSpec(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    const %s *from,\n",
                idl_corbaCxxTypeFromTypeSpec(idl_typeSpec(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    %s *to)\n",
                idl_scopedTypeName (idl_typeSpec(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_arrayElements (scope, idl_typeArray(idl_typeDefActual(defSpec)), "from", "to", TRUE, 0, userData);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tseq:
        idl_fileOutPrintf (idl_fileCur(), "void\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n",
                idl_scopedTypeName (idl_typeSpec(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    const %s *from,\n",
                idl_corbaCxxTypeFromTypeSpec(idl_typeSpec(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    %s *to)\n",
                idl_scopedTypeName (idl_typeSpec(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_seqElements (scope, name, defSpec, 0, userData);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    default:
        break;
    }
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
 */
static idl_programControl idlControl = {
        idl_prior
    };

/** @brief return the program control structure for the CORBA C++ CopyIn generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idlControl;
}


/**
 * Specifies the callback table for the Lite C++ CopyIn generation functions.
 */
static struct idl_program
idl_genLiteCxxCopyin = {
    idl_getControl,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/** @brief return the callback table for the Lite C++ CopyIn generation functions.
 */
idl_program
idl_genLiteCxxCopyinProgram (
    void)
{
    return &idl_genLiteCxxCopyin;
}
