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
/**
 * @file
 * This module generates CORBA C CopyOut functions. It handles input types
 * that match the OpenSpliceDDS database  format and  writes the data into
 * data types that match the IDL/C mapping as specified by the OMG.
 */

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCorbaCCopyout.h"
#include "idl_genCHelper.h"
#include "idl_genSacHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"

#include "c_typebase.h"
#include "os_stdlib.h"

#include <stdio.h>

	/** Text indentation level (4 spaces per indent) */
static c_long loopIndent;
	/** Index for array loop variables, incremented for each array dimension */
static c_long varIndex;
	/** Preset discriminator assignment action */
static c_char union_discriminator[256];

static void
idl_arrayDimensions (
    idl_typeArray typeArray,
    os_boolean resolveTypedefs);

static void
idl_arrayElements (
    idl_scope scope,
    const char* name,
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_long indent,
    os_boolean stacRequested,
    os_boolean catsRequested);

static void idl_seqLoopCopy (idl_typeSpec typeSpec, const char *from, const char *to, c_long loop_index, c_long indent);

/** @brief Generate a string representaion the literal value of a label
 * in metadata terms.
 *
 * @param labelVal Specifies the kind and the value of the label
 * @return String representing the image of \b labelVal
 */
static c_char *
idl_valueFromLabelVal (
    idl_labelVal labelVal)
{
    static c_char labelName [1000];

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
	snprintf (labelName, (size_t)sizeof(labelName), "_%s", idl_labelEnumVal(idl_labelEnum(labelVal)));
    } else {
	switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
	    case V_CHAR:
		snprintf (labelName, (size_t)sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
		break;
	    case V_SHORT:
		snprintf (labelName, (size_t)sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
		break;
	    case V_USHORT:
		snprintf (labelName, (size_t)sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
		break;
	    case V_LONG:
		snprintf (labelName, (size_t)sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
		break;
	    case V_ULONG:
		snprintf (labelName, (size_t)sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
		break;
	    case V_LONGLONG:
		snprintf (labelName, (size_t)sizeof(labelName), "%lld", idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
		break;
	    case V_ULONGLONG:
		snprintf (labelName, (size_t)sizeof(labelName), "%llu", idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
		break;
	    case V_BOOLEAN:
    		/* QAC EXPECT 3416; No side effect here */
		if (idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
		    snprintf (labelName, (size_t)sizeof(labelName), "TRUE");
		} else {
		    snprintf (labelName, (size_t)sizeof(labelName), "FALSE");
		}
		break;
	    default:
		break;
	}
    }
    return labelName;
}

/** @brief callback function called on opening the IDL input file.
 *
 * Generate code to include required include files
 * OpenSpliceDDS specific include files:
 * - dds_dcps_private.h
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
    idl_fileOutPrintf (idl_fileCur(), "#include <dds_dcps_private.h>\n\n");
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
 * For structures a copyOut routine named __<scope-elements>_<structure-name>__copyOut
 * will be prepared. The signature of this copyOut routine is:
 * @verbatim
    void copyOut (void *_from, void *_to);
    @endverbatim
 *
 * The copyOut routine signature is generated based upon the input
 * parameters which specify the scope and it's name.
 * This routine generates local variables which match the type of the source
 * and destination:
 * @verbatim
   void
   copyOut (void *_from, void *_to)
   {
       struct _<scope-elements>_<structure-name> *from = (_<scope-elements>_<structure-name> *)_from;
       <scope-elements>_<structure-name> *to = (<scope-elements>_<structure-name> *)to;
    @endverbatim
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
    idl_fileOutPrintf (idl_fileCur(), "void\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyOut(void *_from, void *_to)\n",
	idl_scopeStack (scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    struct _%s *from = (struct _%s *)_from;\n",
	idl_scopeStack (scope, "_", name),
	idl_scopeStack (scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "    %s *to = (%s *)_to;\n",
	idl_scopeStackC (scope, "_", name),
	idl_scopeStackC (scope, "_", name));

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
 * The function finalizes the copyOut routine for the structure by
 * generating the closing bracket ('}').
 *
 * @param name Name of the structure (not used)
 */
static void
idl_structureClose (
    const char *name,
    void *userData)
{
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
}

/** @brief Generate copy statement for elements of basic type.
 *
 * The function generates for the provided basic type a copyOut
 * statement. The identification of the source and destination
 * elements must be provided by the caller.
 *
 * Note that structures can be reused when part of a sequence.
 *
 * Basic type elements, apart from string types, are assigned
 * immediately. Strings are duplicated calling DDS_string_replace.
 * DDS_string_replace will first free the string if a string was
 * already assigned and duplicate the string from the database.
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
    const char *to_id,
    os_boolean stacRequested)
{
    c_long maxlen = 0;
    c_char *cIdName = idl_cId(name);

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
        idl_fileOutPrintf (idl_fileCur(), "    %s%s = (%s)%s%s;\n",
            to_id,
            cIdName,
            idl_corbaCTypeFromTypeSpec (idl_typeSpec(typeBasic)),
            from_id,
            cIdName);
        break;
    case idl_string:
        maxlen = idl_typeBasicMaxlen(typeBasic);
        idl_fileOutPrintf (idl_fileCur(), "    DDS_string_replace (%s%s ? %s%s : \"\", &%s%s);\n",
            from_id,
            cIdName,
            from_id,
            cIdName,
            to_id,
            cIdName);
        break;
    }

    os_free(cIdName);
}

/** @brief Generate copy statement for union case of basic type.
 *
 * The function generates for the provided basic type a copyOut
 * statement. The identification of the source and destination
 * elements must be provided by the caller.
 *
 * Note that unions can be reused when part of a sequence.
 *
 * Basic type elements, apart from string types, are assigned
 * immediately. Strings are duplicated calling DDS_string_replace.
 * DDS_string_replace will first free the string if a string was
 * already assigned and duplicate the string from the database.
 *
 * @param scope Scope of the element
 * @param name Name of the element
 * @param typeBasic Specification of the type of the element
 * @param from_id Identification of the source
 * @param Identification of the destination
 */
static void
idl_basicCaseType (
    idl_scope scope,
    const char *name,
    idl_typeBasic typeBasic,
    const char *from_id,
    const char *to_id)
{
    c_char *cIdName = idl_cId(name);
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
        idl_fileOutPrintf (idl_fileCur(), "        %s%s = (%s)%s%s;\n",
            to_id,
            cIdName,
            idl_corbaCTypeFromTypeSpec (idl_typeSpec(typeBasic)),
            from_id,
            cIdName);
        break;
    case idl_string:
        idl_fileOutPrintf (idl_fileCur(), "        DDS_string_replace (%s%s ? %s%s : \"\", &%s%s);\n",
            from_id,
            cIdName,
            from_id,
            cIdName,
            to_id,
            cIdName);
        break;
    }
    os_free(cIdName);
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
 * - If the type is \b idl_typedef the copy strategy depends on the refered type.
 *   When this type is \b idl_tarray or \b idl_tseq a type specific copy routine
 *   is called. When type is something else, this routine is called recursively with
 *   with the refered type.
 * - If the type is \b idl_tenum, the element is immediately assigned.
 * - If the type is \b idl_tstruct or \b idl_tunion, the copyOut routine of that struct
 *   or union type is called.
 * - If the type is \b idl_tarray the array copy context is set and the service
 *   to copy arrays is called.
 * - If the type is \b idl_tseq, the sequence copy context is setup and the
 *   service to copy sequences is called.
 *
 * Note that structures can be reused when part of a sequence.
 * For that reason for sequences, DDS_sequence_replacebuf is called to determine
 * if the current buffer (if any) is large enough to accommodate the number of
 * sequence elements in the database.
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
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        os_boolean stacRequested = OS_FALSE;

        /* is a stac pragma defined for this member? */
        stacRequested = idl_stacDef_isStacDefined(scope, name, typeSpec, NULL);
        idl_basicMemberType (scope, name, idl_typeBasic(typeSpec), "from->", "to->", stacRequested);
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        os_boolean catsRequested = OS_FALSE;
        os_boolean stacRequested = OS_FALSE;
        idl_typeSpec typeDereffered;

        /* is a stac pragma defined for this member? */
        stacRequested = idl_stacDef_isStacDefined(scope, name, typeSpec, NULL);
        if(!stacRequested)
        {
            /* Cats and stac def can not be defined over the same member as
             * one excepts string types and the other char (array) types
             */
            catsRequested = idl_catsDef_isCatsDefined(scope, name, typeSpec);
        }
        if(catsRequested || stacRequested)
        {
            typeDereffered = idl_typeDefResolveFully(typeSpec);
            idl_structureMemberOpenClose (
                scope,
                name,
                typeDereffered,
                userData);
        } else
        {
            /* QAC EXPECT 3416; No side effect here */
            if ((idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tarray) ||
                (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tseq))
            {
                idl_fileOutPrintf (idl_fileCur(), "    {\n");
                idl_fileOutPrintf (idl_fileCur(), "        extern void __%s__copyOut(void *, void *);\n",
                idl_scopedTypeName (typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "        __%s__copyOut(&from->%s, &to->%s);\n",
                        idl_scopedTypeName (typeSpec),
                        idl_cId(name),
                        idl_cId(name));
                idl_fileOutPrintf (idl_fileCur(), "    }\n");
            } else {
                    /* Calls itself for the actual type in case of typedef */
                    idl_structureMemberOpenClose (
                    scope,
                    name,
                    idl_typeDefActual(idl_typeDef(typeSpec)),
                    userData);
            }
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        idl_fileOutPrintf (idl_fileCur(), "    to->%s = (%s)from->%s;\n",
            idl_cId(name),
	    idl_corbaCTypeFromTypeSpec (typeSpec),
	    idl_cId(name));
        /* QAC EXPECT 3416; No side effect here */
    } else if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
	           (idl_typeSpecType(typeSpec) == idl_tunion)) {
	    idl_fileOutPrintf (idl_fileCur(), "    {\n");
	    idl_fileOutPrintf (idl_fileCur(), "        extern void __%s__copyOut(void *, void *);\n",
	    idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "        __%s__copyOut(&from->%s, &to->%s);\n",
            idl_scopedTypeName (typeSpec),
	    idl_cId(name),
	    idl_cId(name));
	    idl_fileOutPrintf (idl_fileCur(), "    }\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray)
    {
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
            c_char destination[256];
            c_char source[256];

            assert(!stacRequested);

            snprintf(destination, (size_t)sizeof(destination), "to->%s", name);
            snprintf(source, (size_t)sizeof(source), "from->%s", name);

            idl_fileOutPrintf (idl_fileCur(), "    {\n");
            idl_arrayElements (scope, name, idl_typeArray(typeSpec), source, destination, 1, stacRequested, catsRequested);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
        } else if(stacRequested)
        {
            c_char destin[256];
            assert(!catsRequested);
            snprintf(destin, (size_t)sizeof(destin), "to->%s", name);
            idl_fileOutPrintf (idl_fileCur(), "    {\n");
            idl_fileOutPrintf (idl_fileCur(), "        typedef c_char _DestType");
            idl_arrayDimensions (idl_typeArray(typeSpec), OS_TRUE);
            idl_fileOutPrintf (idl_fileCur(),"[%d]", idl_typeBasicMaxlen(idl_typeBasic(baseTypeDereffered))+1);
            idl_fileOutPrintf (idl_fileCur(), ";\n");
            idl_fileOutPrintf (idl_fileCur(), "        _DestType *src = &from->%s;\n\n", idl_cId(name));
            idl_arrayElements (scope, name, idl_typeArray(typeSpec), "src", destin, 1, stacRequested, catsRequested);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
        }
        else
        {
            c_char destin[256];

            snprintf(destin, (size_t)sizeof(destin), "to->%s", name);
            idl_fileOutPrintf (idl_fileCur(), "    {\n");
            idl_fileOutPrintf (idl_fileCur(), "        typedef %s _DestType", idl_scopedSplTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec))));
            idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
            idl_fileOutPrintf (idl_fileCur(), ";\n");
            idl_fileOutPrintf (idl_fileCur(), "        _DestType *src = &from->%s;\n\n", idl_cId(name));
            idl_arrayElements (scope, name, idl_typeArray(typeSpec), "src", destin, 1, stacRequested, catsRequested);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
	idl_typeSpec nextType = idl_typeSeqType(idl_typeSeq(typeSpec));

        snprintf (type_name, (size_t)sizeof(type_name), "_%s_seq", name);
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        idl_fileOutPrintf (idl_fileCur(), "        long size0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        %s *src0 = (%s *)from->%s;\n",
	    idl_scopedSplTypeIdent(nextType),
	    idl_scopedSplTypeIdent(nextType),
	    idl_cId(name));
        idl_fileOutPrintf (idl_fileCur(), "        %s *dst0 = &to->%s;\n\n",
            idl_sequenceIdent (idl_typeSeq(typeSpec)),
            idl_cId(name));
        idl_fileOutPrintf (idl_fileCur(), "        size0 = c_arraySize((c_sequence)(from->%s));\n",
	    idl_cId(name));
        if (idl_typeSeqMaxSize(idl_typeSeq(typeSpec)) == 0) {
            /* For unbounded sequence set maximum size */
            idl_fileOutPrintf (idl_fileCur(), "        DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, size0);\n",
                idl_sequenceIdent (idl_typeSeq(typeSpec)));
        } else {
            idl_fileOutPrintf (idl_fileCur(), "        DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, %d);\n",
                idl_sequenceIdent (idl_typeSeq(typeSpec)),
            idl_typeSeqMaxSize(idl_typeSeq(typeSpec)));
        }
	    idl_fileOutPrintf (idl_fileCur(), "        dst0->_length = size0;\n");
        idl_seqLoopCopy (nextType, "src0", "*dst", 1, 2);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
    } else {
	    /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
}

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
    if (idl_typeSpecType(subType) == idl_tarray) {
        idl_arrayDimensions (idl_typeArray(subType), resolveTypedefs);
    }
}

/** @brief Generate the loop variables for the array copy.
 *
 * Generate the loop variables for the array copy. Per dimension a
 * loop variable named i# is required, where # represents the dimension.
 * (I.e. first dimension handled by i1, second by i2, etc.)
 * @verbatim
   int i1; // dimension-first
   int i2; // dimension-first+1
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
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    int i%d;\n", loopIndent);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
	/* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopVariables (idl_typeArray(idl_typeArrayType(typeArray)), indent);
    } else {
        idl_fileOutPrintf (idl_fileCur(), "\n");
    }
    loopIndent--;
}

/** @brief Generate copy loop for an array.
 *
 * Generate the for statement for copying an array.
 * The variable \b loopIndent indicates the actual loop variable for this dimension.
 * The variable indent indicates the positional index for the loop.
 * @verbatim
   for (i1 = 0; i1 < <size-dimension-first>; i1++) {
       for (i2 = 0; i2 < <size-dimension-first+1>; i2++) {
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
    idl_printIndent (loopIndent + indent);
    idl_fileOutPrintf (idl_fileCur(), "for (i%d = 0; i%d < %d; i%d++) {\n",
	loopIndent,
	loopIndent,
	idl_typeArraySize(typeArray),
	loopIndent);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyOpen (idl_typeArray(idl_typeArrayType(typeArray)), indent);
    }
}

/** @brief Generate array index for copying array elements.
 *
 * Generate array index to the default output file.
 *
 * @verbatim
    [i1][i2]..[i#]
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
    idl_fileOutPrintf (idl_fileCur(), "[i%d]", varIndex);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyIndex (idl_typeArray(idl_typeArrayType(typeArray)));
    }
    varIndex--;
}

/** @brief Generate array index for copying array elements.
 *
 * Generate array index to string.
 *
 * @verbatim
    [i1][i2]..[i#]
   @endverbatim
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 */
static void
idl_arrayLoopCopyIndexString (
    char *indexString,
    idl_typeArray typeArray)
{
    c_char arrIndex[16];

    varIndex++;
    snprintf (arrIndex, (size_t)sizeof(arrIndex), "[i%d]", varIndex);
    os_strncat (indexString, arrIndex, (size_t)sizeof(arrIndex));
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopCopyIndexString (indexString, idl_typeArray(idl_typeArrayType(typeArray)));
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
idl_indexSize (
    idl_typeArray typeArray)
{
    c_long return_val = 1;

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
	/* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        return_val = idl_indexSize (idl_typeArray(idl_typeArrayType(typeArray)))+1;
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
 * Because the array is member of a structure or union case of a union, reuse
 * of the array is possible and replacement of strings and sequence buffers
 * must be supported.
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param from Identification of the source to copy from
 * @param to Identification of the destination to copy to
 * @param indent Indentation of the output
 */
static void
idl_arrayLoopCopyBody (
    idl_typeArray typeArray,
    idl_typeSpec typeSpec,
    const char *from,
    const char *to,
    c_long indent,
    idl_scope scope,
    const char* name,
    os_boolean stacRequested,
    os_boolean catsRequested)
{
    idl_typeSpec nextType;
    c_char source[256];
    c_char destin[256];
    c_long total_indent;

    loopIndent++;
    switch (idl_typeSpecType (typeSpec)) {
    case idl_tstruct:
    case idl_tunion:
        idl_printIndent (loopIndent + indent);
	    varIndex = 0;
	    idl_fileOutPrintf (idl_fileCur(), "{\n");
	    idl_fileOutPrintf (idl_fileCur(), "    extern void __%s__copyOut(void *, void *);\n",
	    idl_scopedTypeName (typeSpec));
	    idl_fileOutPrintf (idl_fileCur(), "    __%s__copyOut(&(*%s)",
	    idl_scopedTypeName(typeSpec),
	    from);
	    idl_arrayLoopCopyIndex(typeArray);
	    idl_fileOutPrintf (idl_fileCur(), ", &(%s)",
	    to);
	    idl_arrayLoopCopyIndex(typeArray);
	    idl_fileOutPrintf (idl_fileCur(), ");\n");
	    idl_fileOutPrintf (idl_fileCur(), "}\n");
	break;
    case idl_ttypedef:
        if(stacRequested)
        {
            /* stac found, bypass the typedef and recurse deeper */
            if(idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tarray)
            {
                /* An array type is not handled by idl_arrayLoopCopyBody operation
                 * so we have to go up to the idl_arrayElements operation
                 */
                idl_arrayElements(
                    scope,
                    name,
                    idl_typeArray(idl_typeDefActual(idl_typeDef(typeSpec))),
                    from,
                    to,
                    indent,
                    stacRequested,
                    catsRequested);
            } else
            {
                idl_arrayLoopCopyBody(
                    typeArray,
                    idl_typeDefActual(idl_typeDef(typeSpec)),
                    from,
                    to,
                    indent,
                    scope,
                    name,
                    stacRequested,
                    catsRequested);
            }
        } else
        {
            idl_printIndent (loopIndent + indent);
            switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
            case idl_tstruct:
            case idl_tunion:
            case idl_tarray:
            case idl_tseq:
                idl_fileOutPrintf (idl_fileCur(), "{\n");
                idl_fileOutPrintf (idl_fileCur(), "    extern void __%s__copyOut(void *, void *);\n",
                    idl_scopedTypeName (typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "    __%s__copyOut(&(*%s)",
                    idl_scopedTypeName(typeSpec),
                from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ", &(%s)",
                to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ");\n");
                idl_fileOutPrintf (idl_fileCur(), "}\n");
                break;
            case idl_tbasic:
                    /* QAC EXPECT 3416; No side effect here */
                if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                    idl_fileOutPrintf (idl_fileCur(), "    DDS_string_replace ((*%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), " ? (*%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), " : \"\", &(%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), ");\n");
                } else {
                    idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), ", %s", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), ", sizeof (%s", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), "));\n");
                }
                break;
            case idl_tenum:
                idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (%s));\n", to, from, to);
                break;
            default:
                printf ("idl_arrayLoopCopyBody: Unexpected type\n");
                assert (0);
                break;
            }
        }
	break;
    case idl_tbasic:
        idl_printIndent (loopIndent + indent);
        /* This may only be string */
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
            idl_fileOutPrintf (idl_fileCur(), "DDS_string_replace ((*%s)", from);
            idl_arrayLoopCopyIndex(typeArray);
            idl_fileOutPrintf (idl_fileCur(), " ? (*%s)", from);
            idl_arrayLoopCopyIndex(typeArray);
            idl_fileOutPrintf (idl_fileCur(), " : \"\", &(%s)", to);
            idl_arrayLoopCopyIndex(typeArray);
            idl_fileOutPrintf (idl_fileCur(), ");\n");
        } else {
            assert (0);
        }
        break;
    case idl_tseq:
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        total_indent = indent+idl_indexSize(typeArray);
        snprintf (destin, (size_t)sizeof(destin), "(%s)", to);
        idl_arrayLoopCopyIndexString (destin, typeArray);
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    long size0;\n");
        idl_printIndent (total_indent);
        snprintf (source, (size_t)sizeof(source), "(*%s)", from);
        idl_arrayLoopCopyIndexString (source, typeArray);
        idl_fileOutPrintf (idl_fileCur(), "    %s *src0 = (%s *)%s;\n",
	    idl_scopedSplTypeIdent(nextType),
	    idl_scopedSplTypeIdent(nextType),
	    source);
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s *dst0 = &%s;\n\n",
            idl_sequenceIdent (idl_typeSeq(typeSpec)),
            destin);
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    size0 = c_arraySize((c_sequence)(%s));\n",
	    "src0");
	if (idl_typeSeqMaxSize(idl_typeSeq(typeSpec)) == 0) {
	    /* For unbounded sequence set maximum size */
            idl_printIndent (total_indent);
	    idl_fileOutPrintf (idl_fileCur(), "    DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, size0);\n",
	        idl_sequenceIdent (idl_typeSeq(typeSpec)));
	} else {
            idl_printIndent (total_indent);
	    idl_fileOutPrintf (idl_fileCur(), "    DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, %d);\n",
	        idl_sequenceIdent (idl_typeSeq(typeSpec)),
		idl_typeSeqMaxSize(idl_typeSeq(typeSpec)));
	}
        idl_printIndent (total_indent);
	idl_fileOutPrintf (idl_fileCur(), "    dst0->_length = size0;\n");
        idl_seqLoopCopy (nextType, "src0", "*dst", 1, total_indent+1);
	break;
    default:
	assert (0);
	break;
    }
    loopIndent--;
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
 * For each all array dimensions generate a loop copy action.
 * An array copy action is construct of:
 * - a loop variable (idl_arrayLoopVariables)
 * - a loop statement (idl_arrayLoopCopyOpen)
 * - copying the array element (idl_arrayLoopCopyBody)
 * - closing the loop statement (idl_arrayLoopCopyClose)
 *
 * This results in the following pattern for copying arrays per element:
 * @verbatim
   int i1;
   int i2;
   ..
   int i#;

   for (i1 = 0; i1 < <size-dimension-first>; i1++) {
       for (i2 = 0; i2 < <size-dimension-first+1>; i2++) {
           ..
               for (i# = 0; i# < <size-dimension-last>; i#++) {
                    // copy array element
                    DDS_string_replace (from[i1][i2]..[i#] ? from[i1][i2]..[i#] : "", &to[i1][i2]..[i#]) // for string
                    __<type>__copyOut (&from[i1][i2]..[i#], &to[i1][i2]..[i#]) // for struct, union
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
    const char* name,
    os_boolean stacRequested,
    os_boolean catsRequested)
{
    loopIndent = 0;
    idl_arrayLoopVariables (typeArray, indent);
    idl_arrayLoopCopyOpen (typeArray, indent);
    idl_arrayLoopCopyBody (typeArray, idl_typeArrayActual(typeArray), from, to, indent, scope, name, stacRequested, catsRequested);
    idl_arrayLoopCopyClose (typeArray, indent);
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
 * @todo When the struct or union does not contain a reference, a plain
 * memory copy could be used because the memory map is the same in CORBA
 * and OpenSpliceDDS
 *
 * @param typeArray The type specification for the array which holds
 * the basic type as wel as the size for each dimension. The first dimension
 * is on top of that stack.
 * @param from Identification of the source to copy from
 * @param to Identification of the destination to copy to
 * @param indent Indentation of the output
 */
static void
idl_arrayElements (
    idl_scope scope,
    const char* name,
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_long indent,
    os_boolean stacRequested,
    os_boolean catsRequested)
{
    idl_typeSpec subType;
    idl_type idlType;

    subType = idl_typeArrayActual(typeArray);
    idlType = idl_typeSpecType(subType);
    /* if we are dealing with a member for which stac or cats was requested
     * and of which the type is a typedef, then we must bypass this typedef
     * completely and get to the actual type of the typedef
     */
    if(stacRequested || catsRequested)
    {
        if(idlType == idl_ttypedef)
        {
            subType = idl_typeDefResolveFully(subType);
            idlType = idl_typeSpecType(subType);
        }
    }
    switch (idlType) {
    case idl_tbasic:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType(idl_typeBasic(subType)) == idl_string) {
            loopIndent = 0;
            idl_arrayLoopCopy (typeArray, from, to, indent, scope, name, stacRequested, catsRequested);
        } else if(catsRequested)
        {
            idl_printIndent(indent);
            idl_fileOutPrintf(idl_fileCur(),"   strncpy(%s, %s, %d);\n", to, from, idl_typeArraySize(typeArray));
        } else
        {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, from);
        }
	break;
    case idl_tenum:
	    idl_printIndent (indent);
	    idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, from);
	break;
    case idl_tstruct:
    case idl_tunion:
	    loopIndent = 0;
	    idl_arrayLoopCopy (typeArray, from, to, indent, scope, name, stacRequested, catsRequested);
    break;
    case idl_ttypedef:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(subType))) == idl_tbasic) {
                /* QAC EXPECT 3416; No side effect here */
            if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(subType)))) == idl_string) {
                loopIndent = 0;
                idl_arrayLoopCopy (typeArray, from, to, indent, scope, name, stacRequested, catsRequested);
            } else {
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (%s));\n", to, from, to);
            }
                /* QAC EXPECT 3416; No side effect here */
        } else if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(subType))) == idl_tenum) {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (%s));\n", to, from, to);
        } else {
            loopIndent = 0;
            idl_arrayLoopCopy (typeArray, from, to, indent, scope, name, stacRequested, catsRequested);
        }
    break;
    case idl_tseq:
	    idl_arrayLoopCopy (typeArray, from, to, indent, scope, name, stacRequested, catsRequested);
    break;
    case idl_tarray:
	    printf ("idl_arrayElements: Unexpected type idl_tarray\n");
    break;
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

   os_strncpy (index, "", (size_t)sizeof(index));
    for (i = 0; i < indent; i++) {
        snprintf (is, (size_t)sizeof(is), "[i%d]", i);
        os_strncat (index, is, (size_t)sizeof(is));
    }
    return os_strdup(index);
}

/** @brief Function that generates code for copying sequences.
 *
 * Generate code for copying sequence types.
 *
 * The general pattern for copying sequences is as follows:
 * @verbatim
   long i0;
   long size0;
   <SPLICE-scoped-sequence-element-type-name> *src0 = (<SPLICE-scoped-sequence-element-type-name> *)<source-id>;
   <C-scoped-sequence-type-name> *dst0 = (<C-scoped-sequence-type-name> *)&<destination-id>;

   size0 = c_arraySize (src0);
   DDS_sequence_replacebuf (dst0, (bufferAllocatorType)<C-scoped-sequence-type-name>_allocbuf, size0);
   dst0->_length = size0;
   // Body to copy sequence contents from *src to <destination-id>
   @endverbatim
 *
 * Note that in C, nested sequences are always identified by a type. Thus sequence
 * copy routines in C do not need to nest the copy actions (in contrast with C++).
 * The variable names used "i0", "size0", "dst0" and "src0", suggest otherwise.
 *
 * This routine only generates the copying code for copying the sequence contents.
 * The context arround it as depicted above is expected to be generated by the
 * calling function.
 * Motivation for this is that are many variations on the pattern depending on
 * the situation, for instance if it is an array of an sequence, an array copy loop
 * must be coded arround it.
 *
 * The body to copy the sequence depends on the actual type of the sequence, however
 * the framework for copying is always the same:
 * @verbatim
   for (i0 = 0; i0 < (unsigned int)size0; i0++) {
       // Copy sequence element (type specific)
   }
   @endverbatim
 *
 * The element copyOut strategy is as follows:
 * - If the sequence element type is \b idl_tenum, direct assignment is used
 * - If the sequence element type is \b idl_tbasic, depending on wheter it is
 *   a string or not, elements are duplicated via c_newString or otherwise
 *   immediately assigned
 * - If the sequence element type is \b idl_tstruct or \b idl_tunion the copyOut
 *   function for that struct or union is called.
 * - If the sequence element type is \b idl_ttypedef the strategy depends on the
 *   actual type of the typedef. For structs unions and arrays the copyOut routine
 *   for that type is called. For basic string type, the string is duplicated
 *   via c_stringNew. For other basic types and enumeration it is just assigned.
 * - If the sequence element type is \b idl_tseq, a new sequence copy operation
 *   is setup for the specific sequence type.
 *
 * Because the sequence can be member of a structure or union case of a union, reuse
 * of the sequence is possible and replacement of strings and sequence buffers
 * must be supported.
 *
 * Types that do not contain any reference types, indentified by idl_isContiguous() are copied via
 * a plain memory copy because the sequence elements are located in consecutive
 * memory with the same memory map for C CORBA and OpenSpliceDDS.
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
    idl_typeSpec typeSpec,
    const char *from,
    const char *to,
    c_long loop_index,
    c_long indent)
{
    c_char source[32];
    idl_typeSpec nextType;

    if (idl_isContiguous(idl_typeSpecDef(typeSpec))) {
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "memcpy ((%s%d)._buffer,%s,size%d* sizeof(*((%s%d)._buffer)));\n",
            to, loop_index-1, from, loop_index-1, to, loop_index-1);
        return;
    }
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    long i%d;\n", loop_index-1);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    for (i%d = 0; i%d < size%d; i%d++) {\n",
            loop_index-1,
            loop_index-1,
            loop_index-1,
            loop_index-1);

    switch (idl_typeSpecType (typeSpec)) {
    case idl_tenum:
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        (%s%d)._buffer[i%d] = (%s)%s[i%d];\n",
                to,
                loop_index-1,
                loop_index-1,
                idl_corbaCTypeFromTypeSpec (typeSpec),
                from,
                loop_index-1);
        break;
    case idl_tbasic:
        idl_printIndent (indent);
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
            idl_fileOutPrintf (idl_fileCur(), "        DDS_string_replace (%s[i%d] ? %s[i%d] : \"\", &(%s%d)._buffer[i%d]);\n",
                    from,
                    loop_index-1,
                    from,
                    loop_index-1,
                    to,
                    loop_index-1,
                    loop_index-1);
        } else {
            /* This branch is never executed because the first if statement that is added as a copy improvement
               at the top of this method already covers this condition */
            idl_fileOutPrintf (idl_fileCur(), "        (%s%d)._buffer[i%d] = (%s)%s[i%d];\n",
                    to,
                    loop_index-1,
                    loop_index-1,
                    idl_corbaCTypeFromTypeSpec (typeSpec),
                    from,
                    loop_index-1);
        }
        break;
    case idl_tstruct:
    case idl_tunion:
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        extern void __%s__copyOut(void *, void *);\n",
                idl_scopedTypeName(typeSpec));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        __%s__copyOut(&%s[i%d], &(%s%d)._buffer[i%d]);\n",
                idl_scopedTypeName(typeSpec),
                from,
                loop_index-1,
                to,
                loop_index-1,
                loop_index-1);
        break;
    case idl_ttypedef:
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
        case idl_tstruct:
        case idl_tunion:
        case idl_tarray:
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        extern void __%s__copyOut(void *, void *);\n",
                    idl_scopedTypeName(typeSpec));
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        __%s__copyOut(&%s[i%d], &(%s%d)._buffer[i%d]);\n",
                    idl_scopedTypeName(typeSpec),
                    from,
                    loop_index-1,
                    to,
                    loop_index-1,
                    loop_index-1);
            break;
        case idl_tbasic:
            /* QAC EXPECT 3416; No side effect here */
            idl_printIndent(indent);
            if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                idl_fileOutPrintf(idl_fileCur(), "        DDS_string_replace (%s[i%d] ? %s[i%d] : \"\", &(%s%d)._buffer[i%d]);\n",
                        from,
                        loop_index - 1,
                        from,
                        loop_index - 1,
                        to,
                        loop_index - 1,
                        loop_index - 1);
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        (%s%d)._buffer[i%d] = (%s)%s[i%d];\n",
                        to,
                        loop_index-1,
                        loop_index-1,
                        idl_corbaCTypeFromTypeSpec (typeSpec),
                        from,
                        loop_index-1);
            }
            break;
        case idl_tenum:
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        (%s%d)._buffer[i%d] = (%s)%s[i%d];\n",
                    to,
                    loop_index-1,
                    loop_index-1,
                    idl_corbaCTypeFromTypeSpec (typeSpec),
                    from,
                    loop_index-1);
            break;
        default:
            printf ("idl_arrayLoopCopyBody: Unexpected type\n");
            assert (0);
        }
        break;
    case idl_tseq:
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        long size%d;\n", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf(idl_fileCur(), "        %s *src%d = (%s *)src%d%s;\n",
                idl_scopedSplTypeIdent(nextType),
                loop_index,
                idl_scopedSplTypeIdent(nextType),
                loop_index - 1,
                idl_seqIndex(loop_index));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        %s *dst%d = (%s *)&dst%d->_buffer[i%d];\n",
                idl_sequenceIdent (idl_typeSeq(typeSpec)),
                loop_index,
                idl_sequenceIdent (idl_typeSeq(typeSpec)),
                loop_index-1,
                loop_index-1);
        idl_fileOutPrintf (idl_fileCur(), "\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        size%d = c_arraySize((c_sequence)(src%d));\n",
                loop_index,
                loop_index);
        if (idl_typeSeqMaxSize(idl_typeSeq(typeSpec)) == 0) {
            /* For unbounded sequence set maximum size */
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        DDS_sequence_replacebuf (dst%d, (bufferAllocatorType)%s_allocbuf, size%d);\n",
                    loop_index,
                    idl_sequenceIdent (idl_typeSeq(typeSpec)),
                    loop_index);
        } else {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    DDS_sequence_replacebuf (dst%d, (bufferAllocatorType)%s_allocbuf, %d);\n",
                    loop_index,
                    idl_sequenceIdent (idl_typeSeq(typeSpec)),
                    idl_typeSeqMaxSize(idl_typeSeq(typeSpec)));
        }
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        dst%d->_length = size%d;\n",
                loop_index,
                loop_index);
        snprintf (source, (size_t)sizeof(source), "src%d", loop_index);
        idl_seqLoopCopy (nextType, source, to, loop_index+1, indent+1);
        break;
    default:
        assert (0);
        break;
    }
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    }\n");
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    /* QAC EXPECT 5101, 5103: Complexity is limited, by independent cases, per case the number of lines is lower  */
}

/** @brief Function that set up the sequence copy context for a typedef.
 *
 * Generate code to setup the sequence copy context for a typedef.
 *
 * Because the sequence can be member of a structure or union case of a union, reuse
 * of the sequence is possible and replacement of strings and sequence buffers
 * must be supported.
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
    idl_typeSeq typeSeq,
    c_long indent)
{
    c_char source[32];
    idl_typeSpec typeSpec = idl_typeSeqType(typeSeq);
    idl_typeSpec nextType = idl_typeSeqType(typeSeq);

    idl_fileOutPrintf (idl_fileCur(), "    long size%d;\n", indent);
    idl_fileOutPrintf (idl_fileCur(), "    %s *src%d = (%s *)(*from);\n",
	idl_scopedSplTypeName(typeSpec),
	indent,
	idl_scopedSplTypeName(typeSpec));
    idl_fileOutPrintf (idl_fileCur(), "    %s *dst0 = to;\n\n",
        idl_scopeStackC (scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "    size%d = c_arraySize((c_sequence)(src%d));\n", indent, indent);
    if (idl_typeSeqMaxSize(typeSeq) == 0) {
	/* For unbounded sequence set maximum size */
	idl_fileOutPrintf (idl_fileCur(), "    DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, size0);\n",
	    idl_sequenceIdent (typeSeq));
    } else {
	idl_fileOutPrintf (idl_fileCur(), "    DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, %d);\n",
	    idl_sequenceIdent (typeSeq),
	    idl_typeSeqMaxSize(typeSeq));
    }
    idl_fileOutPrintf (idl_fileCur(), "    dst0->_length = size0;\n");
    snprintf (source, (size_t)sizeof(source), "src%d", indent);
    idl_seqLoopCopy (nextType, source, "*dst", 1, indent+1);
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
 *   is generated. The copyOut routines calls the copyOut routine of the struct or union.
 * - If the actual type is \b idl_tarray, a named copyOut routine is generated.
 *   For that copyOut routine, array copy code is generated.
 * - If the actual type is \b idl_tseq, a named copyOut routine is generated.
 *   For that copyOut routine, sequence copy code is generated.
 *
 * Type definitions of basic type are immediately copied from within the generated
 * struct or union copyOut routine. No named copy routine is required.
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
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyOut(void *_from, void *_to)\n",
	    idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
	idl_fileOutPrintf (idl_fileCur(), "    extern void __%s__copyOut(void *, void *);\n",
	    idl_scopedTypeName(idl_typeDefActual(defSpec)));
	idl_fileOutPrintf (idl_fileCur(), "    __%s__copyOut(_from, _to);\n",
	    idl_scopedTypeName(idl_typeDefActual(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
	break;
    case idl_tarray:
        idl_fileOutPrintf (idl_fileCur(), "void\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyOut(void *_from, void *_to)\n",
	    idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
	idl_fileOutPrintf (idl_fileCur(), "    _%s *from = (_%s *)_from;\n",
	    idl_scopeStack(scope, "_", name),
	    idl_scopeStack(scope, "_", name));
	idl_fileOutPrintf (idl_fileCur(), "    %s *to = (%s *)_to;\n",
	    idl_scopeStack(scope, "_", name),
	    idl_scopeStack(scope, "_", name));
        idl_arrayElements (scope, name, idl_typeArray(idl_typeDefActual(defSpec)), "from", "*to", 0, OS_FALSE, OS_FALSE);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tseq:
        idl_fileOutPrintf (idl_fileCur(), "void\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyOut(void *_from, void *_to)\n",
	    idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
	idl_fileOutPrintf (idl_fileCur(), "    _%s *from = (_%s *)_from;\n",
	    idl_scopeStack(scope, "_", name),
	    idl_scopeStack(scope, "_", name));
	idl_fileOutPrintf (idl_fileCur(), "    %s *to = (%s *)_to;\n",
	    idl_scopeStack(scope, "_", name),
	    idl_scopeStack(scope, "_", name));
        idl_seqElements (scope, name, idl_typeSeq(idl_typeDefActual(defSpec)), 0);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    default:
	break;
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
 * For unions a copyOut routine named __<scope-elements>_<union-name>__copyOut
 * will be prepared. The signature of this copyOut routine is:
 * @verbatim
   void copyOut (void *_from, void *_to);
   @endverbatim
 *
 * The copyOut routine signature is generated based upon the input
 * parameters which specify the scope and it's name.
 *
 * Note that unions can be reused when part of a sequence.
 * Because reuse implies freeing resources when not needed anymore, complicated
 * algorithms for reusing unions using would be required. The major problem is
 * that the discriminant can change. The current union implementation maps union
 * cases on top of each other because the union is implemented as a real C union.
 * A simple strategy is chosen by first checking if the discriminant of the source
 * and target are different. If so, the destination union will first be cleaned
 * by calling the applicable <union>__free routine and then clearing all memory
 * of the union. If the discriminants are equal, reuse as applicable to structures
 * is used.
 *
 * The copyOut routine will first copy the source and destination addresses
 * to type specific variables, then check if cleaning of the destination
 * union is required. If so, this will be done. Then the routine will preset a
 * discriminant copy action from the source to the target union.
 * The diferent variant of the union are handled by an switch statement
 * that is generated by this function.
 * @verbatim
   struct _<scope-elements>_<union-name> *from = (struct _<scope-elements>_<union-name> *)_from;
   <scope-elements>_<union-name> *to = (<scope-elements>_<union-name> *)_to;

   void <scope-elements>_<union-name>__free (void *object);

   if (from->_d != to->_d) {
       <scope-elements>_<union-name>__free (_to);
       memset (to, 0, sizeof (<scope-elements>_<union-name>));
   }
   switch (to->_d) {
   case <x1>:                   // handled by idl_unionLabelOpenClose
        // copy union case      // handled by idl_unionCaseOpenClose
        break;                  // handled by idl_unionCaseOpenClose
   case <xn>:                   // handled by idl_unionLabelOpenClose
        // copy union case      // handled by idl_unionCaseOpenClose
        break;                  // handled by idl_unionCaseOpenClose
   default:                     // handled by idl_unionLabelOpenClose
        // copy union case      // handled by idl_unionCaseOpenClose
        break;                  // handled by idl_unionCaseOpenClose
   }                            // handled by idl_unionClose
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the union
 * @param unionSpec Specifies the number of union cases and the union switch type
 * @return Next action for this union (idl_explore)
 */
static idl_action
idl_unionOpen (
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    idl_fileOutPrintf (idl_fileCur(), "void\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyOut(void *_from, void *_to)\n",
	idl_scopeStack (scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    struct _%s *from = (struct _%s *)_from;\n",
	idl_scopeStack(scope, "_", name),
	idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "    %s *to = (%s *)_to;\n",
	idl_scopeStackC (scope, "_", name),
	idl_scopeStackC (scope, "_", name));
    if (idl_typeSpecHasRef (idl_typeSpec (unionSpec))) {
        idl_fileOutPrintf (idl_fileCur(), "    DDS_boolean %s__free (void *object);\n\n",
            idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "    if (from->_d != to->_d) {\n");
	idl_fileOutPrintf (idl_fileCur(), "        %s__free (_to);\n",
            idl_scopeStack (scope, "_", name));
	idl_fileOutPrintf (idl_fileCur(), "        memset (to, 0, sizeof (%s));\n",
	    idl_scopeStack (scope, "_", name));
	idl_fileOutPrintf (idl_fileCur(), "    }\n");
    }
    /* prepare assignment of discriminant used within unionClose */
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
        /* String is not allowed here */
        snprintf (union_discriminator, (size_t)sizeof(union_discriminator), "to->_d = (%s)from->_d;",
	    idl_corbaCTypeFromTypeSpec (idl_typeUnionSwitchKind(unionSpec)));
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tenum) {
        snprintf (union_discriminator, (size_t)sizeof(union_discriminator), "to->_d = (%s)from->_d;",
	    idl_corbaCTypeFromTypeSpec (idl_typeUnionSwitchKind(unionSpec)));
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef &&
        idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))) == idl_tbasic) {
        /* String is not allowed here */
        snprintf (union_discriminator, (size_t)sizeof(union_discriminator), "to->_d = (%s)from->_d;",
            idl_corbaCTypeFromTypeSpec (idl_typeUnionSwitchKind(unionSpec)));
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef &&
        idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))) == idl_tenum) {
        snprintf (union_discriminator, (size_t)sizeof(union_discriminator), "to->_d = (%s)from->_d;",
            idl_corbaCTypeFromTypeSpec (idl_typeUnionSwitchKind(unionSpec)));
    } else {
	/* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    idl_fileOutPrintf (idl_fileCur(), "    switch (from->_d) {\n");
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
 * The function finalizes the copyOut routine for the union.
 * The preset discriminant copy action will be written as well
 * as closeing brackets ('}') for the switch statement
 * and the copy function itself.
 *
 * @param name Name of the union
 */
static void
idl_unionClose (
    const char *name,
    void *userData)
{
    idl_fileOutPrintf (idl_fileCur(), "    }\n");
    idl_fileOutPrintf (idl_fileCur(), "    %s\n", union_discriminator);
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
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
 * Depending on the type of the union case, a copy strategy is used:
 * - If the type is \b idl_tbasic, for a string the string is duplicated
 *   via c_stringNew, for other basic  \b idl_typeBasicType is called.
 * - If the type is \b idl_typedef the copy strategy depends on the refered type.
 *   When this type is \b idl_tarray or \b idl_tseq a type specific copy routine
 *   is called. When type is something else, this routine is called recursively with
 *   with the refered type.
 * - If the type is \b idl_tenum, the element is immediately assigned.
 * - If the type is \b idl_tstruct or \b idl_tunion, the copyOut routine of that struct
 *   or union type is called.
 * - If the type is \b idl_tarray the array copy context is set and the service
 *   to generate code for copying arrays is called.
 * - If the type is \b idl_tseq, the sequence copy context is setup and the
 *   service to generate code for copying sequences is called.
 *
 * Note that unions can be reused when part of a sequence.
 * For that reason, replacement of strings and sequence buffers must be supported
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param name Name of the union case
 * @param typeSpec Specifies the type of the union case
 */
static void
idl_unionCaseOpenClose (
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    c_char * cIdName = idl_cId(name);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            idl_fileOutPrintf (idl_fileCur(), "        DDS_string_replace (from->_u.%s ? from->_u.%s : \"\", &to->_u.%s);\n",
                    cIdName,
                    cIdName,
                    cIdName);
        } else {
            idl_basicCaseType (scope, name, idl_typeBasic(typeSpec), "from->_u.", "to->_u.");
        }
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tarray) {
            idl_fileOutPrintf (idl_fileCur(), "        {\n");
            idl_fileOutPrintf (idl_fileCur(), "            extern void __%s__copyOut(void *, void *);\n",
                    idl_scopedTypeName (typeSpec));
            idl_fileOutPrintf (idl_fileCur(), "            __%s__copyOut(&from->_u.%s, &to->_u.%s);\n",
                    idl_scopedTypeName (typeSpec),
                    cIdName,
                    cIdName);
            idl_fileOutPrintf (idl_fileCur(), "        }\n");
            idl_fileOutPrintf (idl_fileCur(), "        break;\n");
            /* QAC EXPECT 3416; No side effect here */
        } else if (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tseq) {
            idl_fileOutPrintf (idl_fileCur(), "        {\n");
            idl_fileOutPrintf (idl_fileCur(), "            extern void __%s__copyOut(void *, void *);\n",
                    idl_scopedTypeName (typeSpec));
            idl_fileOutPrintf (idl_fileCur(), "            __%s__copyOut(&from->_u.%s, &to->_u.%s);\n",
                    idl_scopedTypeName (typeSpec),
                    cIdName,
                    cIdName);
            idl_fileOutPrintf (idl_fileCur(), "        }\n");
            idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        } else {
            /* Calls itself for the actual type in case of typedef */
            idl_unionCaseOpenClose (
                scope,
                name,
                idl_typeDefRefered(idl_typeDef(typeSpec)),
                userData);
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        idl_fileOutPrintf (idl_fileCur(), "        to->_u.%s = (%s)from->_u.%s;\n",
                cIdName,
                idl_corbaCTypeFromTypeSpec (typeSpec),
                cIdName);
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        idl_fileOutPrintf (idl_fileCur(), "        {\n");
        idl_fileOutPrintf (idl_fileCur(), "            extern void __%s__copyOut(void *, void *);\n",
                idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "            __%s__copyOut(&from->_u.%s, &to->_u.%s);\n",
                idl_scopedTypeName (typeSpec),
                cIdName,
                cIdName);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tunion) {
        idl_fileOutPrintf (idl_fileCur(), "        {\n");
        idl_fileOutPrintf (idl_fileCur(), "            extern void __%s__copyOut(void *, void *);\n",
                idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "            __%s__copyOut(&from->_u.%s, &to->_u.%s);\n",
                idl_scopedTypeName (typeSpec),
                cIdName,
                cIdName);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        c_char destin[256];

        snprintf(destin, (size_t)sizeof(destin), "to->_u.%s", name);
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        idl_fileOutPrintf (idl_fileCur(), "        typedef %s _DestType",
                idl_scopedSplTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec))));
        idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
        idl_fileOutPrintf (idl_fileCur(), ";\n");
        idl_fileOutPrintf (idl_fileCur(), "        _DestType *src = &from->_u.%s;\n\n",
                cIdName);
        idl_arrayElements (scope, name, idl_typeArray(typeSpec), "src", destin, 2, OS_FALSE, OS_FALSE);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_fileOutPrintf (idl_fileCur(), "    break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
        idl_typeSpec nextType = idl_typeSeqType(idl_typeSeq(typeSpec));

        snprintf (type_name, (size_t)sizeof(type_name), "_%s_seq", name);
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        idl_fileOutPrintf (idl_fileCur(), "        long size0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        %s *src0 = (%s *)from->_u.%s;\n",
                idl_scopedSplTypeName(nextType),
                idl_scopedSplTypeName(nextType),
                cIdName);
        idl_fileOutPrintf (idl_fileCur(), "        %s *dst0 = &to->_u.%s;\n\n",
                idl_sequenceIdent (idl_typeSeq(typeSpec)),
                cIdName);
        idl_fileOutPrintf (idl_fileCur(), "        size0 = c_arraySize((c_sequence)(from->_u.%s));\n",
                cIdName);
        if (idl_typeSeqMaxSize(idl_typeSeq(typeSpec)) == 0) {
            /* For unbounded sequence set maximum size */
            idl_fileOutPrintf (idl_fileCur(), "        DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, size0);\n",
                    idl_sequenceIdent (idl_typeSeq(typeSpec)));
        } else {
            idl_fileOutPrintf (idl_fileCur(), "        DDS_sequence_replacebuf (dst0, (bufferAllocatorType)%s_allocbuf, %d);\n",
                    idl_sequenceIdent (idl_typeSeq(typeSpec)),
                    idl_typeSeqMaxSize(idl_typeSeq(typeSpec)));
        }
        idl_fileOutPrintf (idl_fileCur(), "        dst0->_length = size0;\n");
        idl_seqLoopCopy (nextType, "src0", "*dst", 1, 2);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_fileOutPrintf (idl_fileCur(), "    break;\n");
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }

    os_free(cIdName);
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
 * The function generates code to produce the switch label related
 * to the union case.
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelOpenClose (
    idl_scope ownScope,
    idl_labelVal labelVal,
    void *userData)
{
    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(labelVal) == idl_ldefault) {
        idl_fileOutPrintf (idl_fileCur(), "    default:\n");
    } else {
        idl_fileOutPrintf (idl_fileCur(), "    case %s:\n", idl_valueFromLabelVal(labelVal));
    }
    /* QAC EXPECT 5101, 5103: Complexity is limited, by independent cases, per case the number of lines is lower  */
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
 */
static idl_programControl idlControl = {
        idl_prior
    };

/** @brief return the program control structure for the CORBA C CopyOut generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    return &idlControl;
}

static void
idl_artificialDefaultLabelOpenClose(
    idl_scope scope,
    idl_labelVal labelVal,
    idl_typeSpec typeSpec,
    void *userData)
{
    idl_fileOutPrintf (idl_fileCur(), "    default:\n");
    idl_fileOutPrintf (idl_fileCur(), "    break;\n");
}

/**
 * Specifies the callback table for the CORBA C CopyIn generation functions.
 */
static struct idl_program
idl_genCorbaCCopyout = {
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
    idl_unionOpen,
    idl_unionClose,
    idl_unionCaseOpenClose,
    NULL, /* idl_unionLabelsOpenClose */
    idl_unionLabelOpenClose,
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    idl_artificialDefaultLabelOpenClose,
    NULL  /* userData */
};

/** @brief return the callback table for the CORBA C CopyIn generation functions.
 */
idl_program
idl_genCorbaCCopyoutProgram (
    void)
{
    return &idl_genCorbaCCopyout;
}
