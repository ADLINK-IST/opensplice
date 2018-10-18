/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2015 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/**
 * @file
 * This module generates CORBA C++ CopyIn functions. It handles input types
 * that match the IDL/C++ mapping as specified by the OMG and writes the
 * data into data types as applicable for the OpenSpliceDDS database.
 */

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genISOCxx2Copyin.h"
#include "idl_genISOCxx2Helper.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
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

static c_long memberIndex = -1;

static c_long caseIndex = -1;

static void
idl_arrayDimensions (
    idl_typeArray typeArray,
    os_boolean resolveTypedefs);

static void
idl_arrayElements (
    idl_scope scope,
    idl_typeArray typeArray,
    const char *from,
    const char *to,
    c_long indent,
    os_boolean stacRequested,
    os_boolean catsRequested,
    void *userData);

static void idl_seqElements (idl_scope scope, const char *name, idl_typeSeq typeSeq, c_long indent, void *userData);
static void idl_seqLoopCopy (idl_scope scope, idl_typeSpec typeSpec, const char *from, const char *to, c_long loop_index, c_long indent, void *userData);


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
        c_char *labelEnumVal = idl_labelEnumVal(idl_labelEnum(labelVal));
        snprintf (labelName, sizeof(labelName), "_%s", labelEnumVal);
        os_free(labelEnumVal);
    } else {
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
            case V_CHAR:
                snprintf (labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
                break;
            case V_SHORT:
                snprintf (labelName, sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
                break;
            case V_USHORT:
                snprintf(labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal) ).is.UShort);
                break;
            case V_LONG:
                snprintf(labelName, sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal) ).is.Long);
                break;
            case V_ULONG:
                snprintf(labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal) ).is.ULong);
                break;
            case V_LONGLONG:
                snprintf(labelName, sizeof(labelName), "%"PA_PRId64, idl_labelValueVal(idl_labelValue(labelVal) ).is.LongLong);
                break;
            case V_ULONGLONG:
                snprintf(labelName, sizeof(labelName), "%"PA_PRIu64, idl_labelValueVal(idl_labelValue(labelVal) ).is.ULongLong);
                break;
            case V_BOOLEAN:
                /* QAC EXPECT 3416; No side effect here */
                if ((int) idl_labelValueVal(idl_labelValue(labelVal) ).is.Boolean == OS_C_TRUE) {
                    snprintf(labelName, sizeof(labelName), "OS_C_TRUE");
                } else {
                    snprintf(labelName, sizeof(labelName), "OS_C_FALSE");
                }
                break;
            default:
                printf("idl_valueFromLabelVal: Unexpected label type\n");
                break;
        }
    }
    return labelName;
    /* QAC EXPECT 5101; Because of the switch statement the real complexity is rather low, no need to change */
}

/** @brief callback function called on opening the IDL input file.
 *
 * Generate code to include standard include files as well as
 * OpenSpliceDDS specific include files:
 * - v_kernel.h
 * - v_topic.h
 * - string.h
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

    idl_fileOutPrintf (idl_fileCur(), "#include <v_kernel.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <v_topic.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <os_stdlib.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <string.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <os_report.h>\n");
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
    void copyIn (c_base base,
        const <scope-elements>::<structure-name> *from,
        struct _<scope-elements>_<structure-name> *to);
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
    c_char *cScopedName = idl_scopeStack(scope, "_", name);
    c_char *cxxScopedName = idl_scopeStackISOCxx2(scope, "::", name);

    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf (idl_fileCur(), "v_copyin_result\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n", cScopedName);
    idl_fileOutPrintf (idl_fileCur(), "    c_type dbType,\n");
    idl_fileOutPrintf (idl_fileCur(), "    const class %s *from,\n", cxxScopedName);
    idl_fileOutPrintf (idl_fileCur(), "    struct _%s *to)\n", cScopedName);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    v_copyin_result result = V_COPYIN_RESULT_OK;\n");
    idl_fileOutPrintf (idl_fileCur(), "    (void) dbType;\n\n");

    memberIndex = 0;

    os_free(cxxScopedName);
    os_free(cScopedName);

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

    idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");

    memberIndex = -1;
}

/** @brief Generate copy statement for elements of basic type.
 *
 * The function generates for the provided basic type a copyIn
 * statement. The identification of the source and destination
 * elements must be provided by the caller.
 *
 * Basic type elements, apart from string types, are assigned
 * immediately. Strings are duplicated calling c_stringNew.
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
    c_ulong maxlen;
    c_char *idlType;

    OS_UNUSED_ARG(stacRequested);

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
        idlType = idl_typeFromTypeSpec (idl_typeSpec(typeBasic));
        idl_fileOutPrintf (idl_fileCur(), "    %s%s = (%s)%s%s();\n",
                to_id,
                name,
                idlType,
                from_id,
                name);
        os_free(idlType);
        break;
    case idl_string:
        maxlen = idl_typeBasicMaxlen(typeBasic);
        if (maxlen != 0) {
          idl_fileOutPrintf (idl_fileCur(),"    if(((unsigned int)%s%s().length()) <= %u){\n",
                      from_id,
                      name,
                      maxlen);
          if(stacRequested)
          {
              idl_fileOutPrintf (idl_fileCur(),"        /* The strncpy takes a size of the maximum string bounds plus 1, as the database size accomodates this */\n");
              idl_fileOutPrintf(idl_fileCur(), "        strncpy((%s%s), %s%s_.c_str(), %u);\n",
                      to_id,
                      name,
                      from_id,
                      name,
                      (maxlen+1));
          } else {
              idl_fileOutPrintf(idl_fileCur(), "        %s%s = c_stringNew(c_getBase(dbType), %s%s_.c_str());\n",
                      to_id,
                      name,
                      from_id,
                      name);
          }
          idl_fileOutPrintf (idl_fileCur(),"    } else {\n");
          idl_fileOutPrintf (idl_fileCur(),"        ");
          idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeBasic, name);
          idl_fileOutPrintf (idl_fileCur(),"        result = V_COPYIN_RESULT_INVALID;\n");
          idl_fileOutPrintf (idl_fileCur(),"    }\n");
        } else {
            idl_fileOutPrintf (idl_fileCur(), "    %s%s = c_stringNew(c_getBase(dbType), %s%s_.c_str());\n",
                    to_id,
                    name,
                    from_id,
                    name);
        }
        break;
    default:
        printf ("idl_basicMemberType: Unexpected basic type\n");
        break;
    }
}

/** @brief Generate copy statement for union case of basic type.
 *
 * The function generates for the provided basic type a copyIn
 * statement. The identification of the source and destination
 * elements must be provided by the caller.
 *
 * Basic type elements, apart from string types, are assigned
 * immediately. Strings are duplicated calling c_stringNew.
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
    c_ulong maxlen;
    c_char *idlType;

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
        idlType = idl_typeFromTypeSpec (idl_typeSpec(typeBasic));
        idl_fileOutPrintf (idl_fileCur(), "        %s%s = (%s)%s%s();\n",
                to_id,
                name,
                idl_typeFromTypeSpec (idl_typeSpec(typeBasic)),
                from_id,
                name);
        os_free(idlType);
        break;
    case idl_string:
        maxlen = idl_typeBasicMaxlen(typeBasic);

        if(maxlen != 0){
            idl_fileOutPrintf (idl_fileCur(),"    if((unsigned int) %s%s().length() <= %u){\n",
                    from_id,
                    name,
                    maxlen);
            idl_fileOutPrintf(idl_fileCur(), "        %s%s = c_stringNew(c_getBase(dbType), %s%s().c_str());\n",
                    to_id,
                    name,
                    from_id,
                    name);
            idl_fileOutPrintf (idl_fileCur(),"    } else {\n");
            idl_fileOutPrintf (idl_fileCur(),"        ");
            idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeBasic, name);
            idl_fileOutPrintf (idl_fileCur(),"        result = V_COPYIN_RESULT_INVALID;\n");
            idl_fileOutPrintf (idl_fileCur(),"    }\n");
        } else {
            idl_fileOutPrintf (idl_fileCur(), "    %s%s = c_stringNew(c_getBase(dbType), %s%s().c_str());\n",
                    to_id,
                    name,
                    from_id,
                    name);
        }
        break;
    default:
        printf ("idl_basicCaseType: Unexpected basic type\n");
        break;
    }
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
    c_ulong maxlen;
    c_char *typeName;
    c_char *scopedName;
    c_char* cxxName;

    cxxName = idl_ISOCxx2Id(name);

    /* Expected types: idl_tbasic, idl_ttypedef, idl_tenum, idl_tstruct, idl_tunion, idl_tarray, idl_tseq */

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        os_boolean stacRequested = OS_FALSE;
        /* is a stac pragma defined for this member? */
        stacRequested = idl_stacDef_isStacDefined(scope, name, typeSpec, NULL);
        /* Handles all basic types, inclusive strings */
        idl_basicMemberType (scope, cxxName, idl_typeBasic(typeSpec), "from->", "to->", stacRequested);
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
            /* if cats or stac is defined then resolve the first level of
            * typedef and recurse deeper into the operation
            */
            typeDereffered = idl_typeDefResolveFully(typeSpec);
            idl_structureMemberOpenClose (
                scope,
                cxxName,
                typeDereffered,
                userData);
        } else {
            /* QAC EXPECT 3416; No side effect here */
            if ((idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tstruct) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tunion) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tarray) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tseq)) {
                scopedName = idl_scopedTypeName (typeSpec);
                typeName = idl_ISOCxx2TypeFromTypeSpec(typeSpec);

                idl_fileOutPrintf (idl_fileCur(), "    if(V_COPYIN_RESULT_IS_OK(result)){\n");
                idl_fileOutPrintf (idl_fileCur(), "        extern v_copyin_result __%s__copyIn(c_type, const %s *, _%s *);\n",
                    scopedName,
                    typeName,
                    scopedName);
                idl_fileOutPrintf (idl_fileCur(), "        c_type type0 = c_memberType(c_structureMember(c_structure(dbType), %d));\n", memberIndex);
                if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tarray) {
                    idl_fileOutPrintf (idl_fileCur(), "        while(c_baseObjectKind(type0) == M_COLLECTION && c_collectionTypeKind(type0) == OSPL_C_ARRAY) {;\n");
                    idl_fileOutPrintf (idl_fileCur(), "            type0 = c_collectionTypeSubType(type0);\n");
                    idl_fileOutPrintf (idl_fileCur(), "        }\n");
                }
                idl_fileOutPrintf (idl_fileCur(), "        result = __%s__copyIn(type0, &from->%s(), &to->%s);\n",
                    scopedName,
                    cxxName,
                    cxxName);
                idl_fileOutPrintf (idl_fileCur(), "    }\n");
                os_free(typeName);
                os_free(scopedName);
            } else {
                /* Calls itself for the actual type in case of typedef */
                /* QAC EXPECT 3670; We wan't to use recursion, the recursion is finite */
                idl_structureMemberOpenClose (
                    scope,
                    cxxName,
                    idl_typeDefActual(idl_typeDef(typeSpec)),
                    userData);
                /* Compensate for recursion, where memberIndex will now be incremented twice. */
                memberIndex--;
            }
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        c_char *dbTypeName = idl_scopedSplTypeName (typeSpec);
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));

        idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)from->%s()) >= 0) && (((c_long)from->%s()) < %d) ){\n",
            cxxName ,
            cxxName,
            maxlen);
        idl_fileOutPrintf (idl_fileCur(), "        to->%s = (%s)from->%s();\n",
            cxxName,
            dbTypeName,
            cxxName);
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(),"        ");
        idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeSpec, cxxName);
        idl_fileOutPrintf (idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");

        os_free(dbTypeName);
        /* QAC EXPECT 3416; No side effect here */
    } else if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
               (idl_typeSpecType(typeSpec) == idl_tunion)) {
        scopedName = idl_scopedTypeName (typeSpec);
        typeName = idl_ISOCxx2TypeFromTypeSpec(typeSpec);
        idl_fileOutPrintf (idl_fileCur(), "    if(V_COPYIN_RESULT_IS_OK(result)){\n");
        idl_fileOutPrintf (idl_fileCur(), "        extern v_copyin_result __%s__copyIn(c_type, const %s *, _%s *);\n",
            scopedName,
            typeName,
            scopedName);
        idl_fileOutPrintf (idl_fileCur(), "        result = __%s__copyIn(c_memberType(c_structureMember(dbType, %d)), &from->%s(), &to->%s);\n",
            scopedName,
            memberIndex,
            cxxName,
            cxxName);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        os_free(typeName);
        os_free(scopedName);
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        c_char source[256];
        snprintf(source, sizeof(source), "from->%s()", cxxName);
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
            if(catsRequested) {
                idl_fileOutPrintf(idl_fileCur(),"    {\n");
                idl_arrayElements (scope, idl_typeArray(typeSpec), source, cxxName, 1, stacRequested, catsRequested, userData);
                idl_fileOutPrintf(idl_fileCur(),"    }\n");
            } else if(stacRequested) {
                assert(!catsRequested);
                assert(baseTypeDereffered);
                idl_fileOutPrintf (idl_fileCur(),"    {\n");
                idl_fileOutPrintf (idl_fileCur(),"        typedef c_char _DestType");
                idl_arrayDimensions(idl_typeArray(typeSpec), OS_TRUE);
                idl_fileOutPrintf (idl_fileCur(),"[%u]", idl_typeBasicMaxlen(idl_typeBasic(baseTypeDereffered))+1);
                idl_fileOutPrintf (idl_fileCur(),";\n");
                idl_fileOutPrintf (idl_fileCur(),"        _DestType *dest = &to->%s;\n", cxxName);
                idl_arrayElements (scope, idl_typeArray(typeSpec), source, "dest", 1, stacRequested, catsRequested, userData);
                idl_fileOutPrintf (idl_fileCur(),"    }\n");
            } else {
                idl_typeSpec subType = idl_typeArrayActual(idl_typeArray(typeSpec));
                c_char *dbSubType = idl_scopedSplTypeIdent(subType);

                idl_fileOutPrintf (idl_fileCur(), "    {\n");
                idl_fileOutPrintf (idl_fileCur(), "        typedef %s _DestType", dbSubType);
                idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
                idl_fileOutPrintf (idl_fileCur(), ";\n");
                idl_fileOutPrintf (idl_fileCur(), "        _DestType *dest = &to->%s;\n", cxxName);
                idl_fileOutPrintf (idl_fileCur(), "        c_type type0 = c_memberType(c_structureMember(c_structure(dbType), %d));\n", memberIndex);
                idl_fileOutPrintf (idl_fileCur(), "        while(c_baseObjectKind(type0) == M_COLLECTION && c_collectionTypeKind(type0) == OSPL_C_ARRAY) {;\n");
                idl_fileOutPrintf (idl_fileCur(), "            type0 = c_collectionTypeSubType(type0);\n");
                idl_fileOutPrintf (idl_fileCur(), "        }\n");
                idl_arrayElements (scope, idl_typeArray(typeSpec), source, "dest", 1, stacRequested, catsRequested, userData);
                idl_fileOutPrintf (idl_fileCur(), "    }\n");
                os_free(dbSubType);
            }
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
        idl_typeSpec nextType;
        c_char *cxxScopedName;

        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedSplTypeName(nextType);

        snprintf (type_name, sizeof(type_name), "_%s_seq", name);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        cxxScopedName = idl_scopeStackCxx (scope, "::", type_name);

        if (idl_typeSpecType(nextType) == idl_tbasic) {
            if (idl_typeBasicMaxlen(idl_typeBasic(nextType)) > 0) {
                typeName = idl_scopeStack(scope, "::", idl_typeSpecName(nextType));
            } else {
                typeName = idl_typeSpecName(nextType);
            }
        } else {
            typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
        }

        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        idl_fileOutPrintf (idl_fileCur(), "        /* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_fileOutPrintf (idl_fileCur(), "        c_type type0 = c_memberType(c_structureMember(c_structure(dbType), %d));\n", memberIndex);
        idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "        const %s *src = &from->%s();\n\n", cxxScopedName, cxxName);
        idl_fileOutPrintf (idl_fileCur(), "        c_ulong length0 = (c_ulong)(*src).size();\n");
        if(maxlen){
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "    if(length0 > %u){\n", maxlen);
            idl_printIndent(1);
            idl_fileOutPrintf (idl_fileCur(),"        ");
            idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeSpec, name);
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "    } else {\n");
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
            idl_seqLoopCopy(scope, nextType, "*src", "dest0", 1, 3, userData);
            idl_fileOutPrintf(idl_fileCur(), "            to->%s = (c_sequence) dest0;\n", cxxName);
            idl_fileOutPrintf(idl_fileCur(), "        }\n");
        } else {
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "     dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_printIndent(1);
            idl_fileOutPrintf (idl_fileCur(), "    to->%s = (c_sequence) dest0;\n",
                    idl_cxxId(name));
        }
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        os_free(typeName);
        os_free(cxxScopedName);
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    /* QAC EXPECT 5103; Code is clearly separated in a number of cases of which each is maintainable */

    os_free(cxxName);
    memberIndex++;
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

    idl_fileOutPrintf (idl_fileCur(), "[%u]", idl_typeArraySize(typeArray));
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
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    int i%d;\n", loopIndent);
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
    idl_printIndent(loopIndent + indent);
    idl_fileOutPrintf(idl_fileCur(), "for (i%d = 0; (i%d < %d) && result; i%d++) {\n",
    loopIndent,
    loopIndent,
    idl_typeArraySize(typeArray),
    loopIndent);
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
    idl_fileOutPrintf(idl_fileCur(), "[i%d]", varIndex);
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
    [i1][i2]..[i#]
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
    snprintf(arrIndex, sizeof(arrIndex), "[i%d]", varIndex);
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
    os_boolean stacRequested,
    os_boolean catsRequested,
    void *userData)
{
    idl_typeSpec nextType;
    c_char source[256];
    c_char destin[256];
    c_char *typeName;
    c_char *scopedName;
    c_char *dbTypeName;
    c_long total_indent;
    c_ulong maxlen;

    loopIndent++;
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tstruct:
    case idl_tunion:
        scopedName = idl_scopedTypeName(typeSpec);
        typeName = idl_scopeStackISOCxx2(idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec));
        dbTypeName = idl_scopedSplTypeIdent(typeSpec);
        idl_printIndent(loopIndent + indent);
        varIndex = 0;
        idl_fileOutPrintf(idl_fileCur(), "extern v_copyin_result __%s__copyIn(c_type dbType,\n", scopedName);
        idl_printIndent(loopIndent + indent + 1);
        idl_fileOutPrintf(idl_fileCur(), "const %s *From,\n", typeName);
        idl_printIndent(loopIndent + indent + 1);
        idl_fileOutPrintf(idl_fileCur(), "%s *To);\n\n", dbTypeName);
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf(idl_fileCur(), "if(V_COPYIN_RESULT_IS_OK(result)){\n");
        indent++;
        idl_printIndent (loopIndent + indent);
        idl_fileOutPrintf (idl_fileCur(),"result = __%s__copyIn(type0, (%s *)&(%s)",
            scopedName,
            typeName,
            from);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ", (%s *)&(*%s)", dbTypeName, to);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ");\n");
        indent--;
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        os_free(dbTypeName);
        os_free(typeName);
        os_free(scopedName);
        break;
    case idl_ttypedef:
        if(stacRequested) {
            /* stac found, bypass the typedef and recurse deeper */
            if(idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tarray) {
                /* An array type is not handled by idl_arrayLoopCopyBody operation
                 * so we have to go up to the idl_arrayElements operation
                 */
                idl_arrayElements(
                    scope,
                    idl_typeArray(idl_typeDefActual(idl_typeDef(typeSpec))),
                    from,
                    to,
                    indent,
                    stacRequested,
                    catsRequested,
                    userData);
            } else {
                idl_arrayLoopCopyBody(
                    typeArray,
                    idl_typeDefActual(idl_typeDef(typeSpec)),
                    from,
                    to,
                    indent,
                    scope,
                    stacRequested,
                    catsRequested,
                    userData);
            }
        } else {
            switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
            case idl_tstruct:
            case idl_tunion:
            case idl_tarray:
            case idl_tseq:
                scopedName = idl_scopedTypeName(typeSpec);
                typeName = idl_scopeStackISOCxx2(idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec));
                dbTypeName = idl_scopedSplTypeIdent(typeSpec);

                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "extern v_copyin_result __%s__copyIn(c_type dbType,", scopedName);
                idl_fileOutPrintf(idl_fileCur(), "const %s *From,", typeName);
                idl_fileOutPrintf(idl_fileCur(), "%s *To);\n\n", dbTypeName);
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "if(V_COPYIN_RESULT_IS_OK(result)){\n");
                indent++;
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(),"result = __%s__copyIn(type0, (%s *)&(%s)", scopedName, typeName, from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ", (%s *)&(*%s)", dbTypeName, to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ");\n");
                indent--;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
                os_free(dbTypeName);
                os_free(typeName);
                os_free(scopedName);
                break;
            case idl_tbasic:
                /* QAC EXPECT 3416; No side effect here */
                if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                    maxlen = idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec))));

                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(), "if(V_COPYIN_RESULT_IS_OK(result)){\n");
                    indent++;
                    if(maxlen != 0){
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"if((unsigned int) (%s", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),".length()) <= %u){\n", maxlen);
                        indent++;
                        idl_printIndent(loopIndent + indent);
                        if(stacRequested)
                        {
                            idl_fileOutPrintf (idl_fileCur(),"            strncpy((*%s", to);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf (idl_fileCur(),"), (%s", from);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf (idl_fileCur(),"), %u);\n", (maxlen+1));
                        } else
                        {
                            idl_fileOutPrintf(idl_fileCur(), "%s", to);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(c_getBase(dbType), (%s)", from);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf (idl_fileCur(),".c_str());\n");
                        }
                        indent--;
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"} else {\n");
                        indent++;
                        idl_printIndent(loopIndent + indent);
                        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"result = V_COPYIN_RESULT_INVALID;\n");
                        indent--;
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"}\n");
                    } else {
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(), "%s", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(), " = c_stringNew(c_getBase(dbType), (%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),".c_str());\n");
                    }
                    indent--;
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(),"}\n");
                } else {
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), " = %s", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), ";\n");
                }
                break;
            case idl_tenum:
                maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(typeSpec))));
                dbTypeName = idl_scopedSplTypeIdent(typeSpec);

                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "if( (unsigned)%s", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), " < %u){\n", maxlen);
                indent++;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), " = (%s) (%s", dbTypeName, from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ");\n");
                indent--;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "} else {\n");
                indent++;
                idl_printIndent(loopIndent + indent);
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "result = V_COPYIN_RESULT_INVALID;\n");
                indent--;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "}\n");
                os_free(dbTypeName);
                break;
            default:
                printf ("idl_loopCopyBody: Unexpected type\n");
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

            if(maxlen != 0) {
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "if((%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ".length() <= %u){\n", maxlen);
                indent++;
                if(stacRequested) {
                    idl_fileOutPrintf (idl_fileCur(),"strncpy((*%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(),", (%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(),".c_str(), %u);\n", (maxlen+1));
                } else {
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(c_getBase(dbType), (%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), ".c_str());\n");
                }
                indent--;
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "} else {\n");
                indent++;
                idl_printIndent (loopIndent + indent);
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "    result = V_COPYIN_RESULT_INVALID;\n");
                indent--;
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "}\n");
            } else {
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(c_getBase(dbType), (%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ".c_str());\n");
            }

        } else {
            idl_printIndent(loopIndent + indent);
            idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
            idl_arrayLoopCopyIndex(typeArray);
            idl_fileOutPrintf(idl_fileCur(), " = %s", from);
            idl_arrayLoopCopyIndex(typeArray);
            idl_fileOutPrintf(idl_fileCur(), ";\n");
        }
        break;
    case idl_tenum:
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));
        dbTypeName = idl_scopedSplTypeIdent(typeSpec);

        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf(idl_fileCur(), "if( (unsigned)%s", from);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), " < %u){\n", maxlen);
        indent++;
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), " = (%s) (%s", dbTypeName, from);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
        indent--;
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf (idl_fileCur(), "} else {\n");
        indent++;
        idl_printIndent(loopIndent + indent);
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf (idl_fileCur(), "result = V_COPYIN_RESULT_INVALID;\n");
        indent--;
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        os_free(dbTypeName);
        break;
    case idl_tseq:
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        total_indent = indent+idl_indexSize(typeArray);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        scopedName = idl_scopedSplTypeName(nextType);

        snprintf(source, sizeof(source), "(%s)", from);
        snprintf(destin, sizeof(destin), "(*%s)", to);

        idl_arrayLoopCopyIndexString (source, typeArray);
        idl_arrayLoopCopyIndexString (destin, typeArray);

        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    /* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s *dest0;\n\n", scopedName);
        idl_printIndent (total_indent);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "    c_ulong length0 = (c_ulong)(%s).size();\n", source);

        if(maxlen){
            idl_fileOutPrintf(idl_fileCur(), "    if(length0 > %u){\n", maxlen);
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        ");
            idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "    } else {\n");
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
            idl_seqLoopCopy(scope, nextType, source, "dest0", 1, total_indent+2, userData);

            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        %s = (c_sequence)dest0;\n",
                destin);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
        } else {
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "    dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
                idl_seqLoopCopy(scope, nextType, source, "dest0", 1, total_indent+1, userData);

            idl_printIndent (total_indent);
            idl_fileOutPrintf (idl_fileCur(), "    %s = (c_sequence) dest0;\n", destin);
        }
        os_free(scopedName);
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
   int i1;
   int i2;
   ..
   int i#;

   for (i1 = 0; i1 < <size-dimension-first>; i1++) {
       for (i2 = 0; i2 < <size-dimension-first+1>; i2++) {
           ..
               for (i# = 0; i# < <size-dimension-last>; i#++) {
                    // copy array element
                    to[i1][i2]..[i#] = c_stringNew (from[i1][i2]..[i#]) // for string
                    __<type>__copyIn (&from[i1][i2]..[i#], &to[i1][i2]..[i#]) // for struct, union
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
    os_boolean stacRequested,
    os_boolean catsRequested,
    void *userData)
{
    loopIndent = 0;
    idl_arrayLoopVariables(typeArray, indent);
    idl_arrayLoopCopyOpen(typeArray, indent);
    idl_arrayLoopCopyBody(typeArray, idl_typeArrayActual(typeArray), from, to, indent, scope, stacRequested, catsRequested, userData);
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
 * @todo When the struct (probably not the union) does not contain a reference,
 * a plain memory copy could be used because the memory map is the same in CORBA
 * and OpenSpliceDDS. The C++ sequence buffer must be aquired then.
 *
 * @todo Add bounds checking for enums. See issue dss#175. Grep this issue id in source.
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
    idl_typeArray typeArray,
    const char *from_,
    const char *to,
    c_long indent,
    os_boolean stacRequested,
    os_boolean catsRequested,
    void *userData)
{
    idl_typeSpec subType = idl_typeArrayActual(typeArray);
    idl_type idlType;
    char *buf;

    char* from;
    from = os_malloc(strlen(from_) + strlen("().data()") + 1);
    os_sprintf(from, "%s%s%s", "(", from_, ").data()");
    idlType = idl_typeSpecType(subType);
    /* if we are dealing with a member for which stac or cats was requested
     * and of which the type is a typedef, then we must bypass this typedef
     * completely and get to the actual type of the typedef
     */
    if(stacRequested || catsRequested) {
        if(idlType == idl_ttypedef) {
            subType = idl_typeDefResolveFully(subType);
            idlType = idl_typeSpecType(subType);
        }
    }
    switch (idlType) {
    case idl_tbasic:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType(idl_typeBasic(subType)) == idl_string) {
            idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
        } else {
            /*stac requested should be covered by the above if statement, as
             * stac always involves strings. And if this basic type is not a string
             * then stac requested can not be true
             */
            assert(!stacRequested);
            if(catsRequested) {
                idl_fileOutPrintf(idl_fileCur(),"        /* Allocate the length of the array (and null terminator) as a database\n");
                idl_fileOutPrintf(idl_fileCur(),"        * string\n");
                idl_fileOutPrintf(idl_fileCur(),"        */\n");
                idl_fileOutPrintf(idl_fileCur(),"        to->%s = c_stringMalloc(base, (%u + 1));\n", to, idl_typeArraySize(typeArray));
                idl_fileOutPrintf(idl_fileCur(),"        if(to->%s)\n", to);
                idl_fileOutPrintf(idl_fileCur(),"        {\n");
                idl_fileOutPrintf(idl_fileCur(),"            /* Copy the value of the array into the database string */\n");
                idl_fileOutPrintf(idl_fileCur(),"           strncpy(to->%s, %s, %u);\n", to, from, idl_typeArraySize(typeArray));
                idl_fileOutPrintf(idl_fileCur(),"            to->%s[%u] = '\\0';\n", to, idl_typeArraySize(typeArray));
                idl_fileOutPrintf(idl_fileCur(),"        }\n");
            } else if (idl_typeSpecType(idl_typeArrayType(typeArray)) != idl_tarray){
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
            } else {
                idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
            }
        }
        break;
    case idl_tenum:
        idl_printIndent (indent);
        idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
        break;
    case idl_tstruct:
    case idl_tunion:
        idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
        break;
    case idl_ttypedef:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(subType))) == idl_tbasic) {
            if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(subType)))) == idl_string) {
               buf = os_malloc(strlen(to)+4);
                os_sprintf(buf, "(*%s)", to);
                idl_arrayLoopCopy (typeArray, from, buf, indent, scope, stacRequested, catsRequested, userData);
                os_free(buf);
            } else if (idl_typeSpecType(idl_typeArrayType(typeArray)) != idl_tarray) {
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
            } else {
                idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
            }
        } else {
            idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
        }
        break;
    case idl_tseq:
        idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
        break;
    case idl_tarray:
        printf ("idl_arrayElements: Unexpected type idl_tarray\n");
        break;
    }

    os_free(from);
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
        snprintf (is, sizeof(is), "[i%d]", i);
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
   static c_type type0 = NULL;
   unsigned int i0;
   c_long length0;
   <SPLICE-C-scoped-sequence-element-type-name> *dest0;

   if (type0 == NULL) {
       type0 = c_type(c_metaResolve (c_metaObject(base), "<metadata-scoped-sequence-type-name>"));
   }
   length0 = (c_long)(*src0).length();
   dest0 = (<C-scoped-sequence-element-type-name> *)c_sequenceNew(type0, maxlen, length0);
   // Body to copy sequence contents from <source-id> to dest0
   <destination-id> = (c_sequence)dest0;
   @endverbatim
 *
 * Note that in C++, nested sequences are always anonymous. Thus sequence copy routines in
 * C++ need to nest the copy actions (in contrast with C).
 * The variable names used "i0", "type0", "length0", "dst0" and "src0", suggest otherwise.
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
   for (i0 = 0; i0 < (unsigned int)length0; i0++) {
       // Copy sequence element (type specific)
   }
   @endverbatim
 *
 * The element copyIn strategy is as follows:
 * - If the sequence element type is \b idl_tenum, direct assignment is used
 * - If the sequence element type is \b idl_tbasic, depending on wheter it is
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
 * consequtive memory with the same memory map for C CORBA as well as OpenSpliceDDS.
 * These are identified using idl_isContiguous().
 *
 * @todo Add bounds checking for enums. See issue dss#175. Grep this issue id in source.
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
    c_char destin[32];
    c_char *typeName;
    c_char *scopedName;
    c_char *seqIndex;
    c_char *funcPrefix;
    idl_typeSpec nextType;
    c_ulong maxlen;

    if (idl_isContiguous(idl_typeSpecDef(typeSpec)) && idl_typeSpecType (typeSpec) != idl_tenum) {
        seqIndex = idl_seqIndex (loop_index - 1);
        scopedName = idl_scopedSplTypeName(typeSpec);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    /* Code generated by %s at line %d */\n",__FILE__,__LINE__);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    if(length0 > 0)\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    {\n");
        indent += 1;
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    const %s *buf%d;\n", scopedName, loop_index-1);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    buf%d = reinterpret_cast<const %s *>(&((%s)%s[0]));\n",
            loop_index-1, scopedName, from, seqIndex);

        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s,buf%d,length%d* sizeof(*%s));\n",
            to, loop_index-1, loop_index-1, to);

        indent -=1;
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        os_free(seqIndex);
        os_free(scopedName);
        return;
    }

    seqIndex = idl_seqIndex (loop_index);
    scopedName = idl_scopedSplTypeName(typeSpec);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    /* Code generated by %s at line %d */\n",__FILE__,__LINE__);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    unsigned int i%d;\n", loop_index-1);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    for (i%d = 0; (i%d < length%d) && result; i%d++) {\n",
        loop_index-1,
        loop_index-1,
        loop_index-1,
        loop_index-1);

    switch (idl_typeSpecType (typeSpec)) {
    case idl_tenum:
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));

        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    if((unsigned)((%s)%s) < %u){\n",
                from,
                seqIndex,
                maxlen);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                to,
                loop_index-1,
                scopedName,
                from,
                seqIndex);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        break;
    case idl_tbasic:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));

            idl_printIndent (indent);

            if(maxlen != 0){
                idl_fileOutPrintf(idl_fileCur(), "            if((unsigned int)(%s)%s.length() <= %d){\n",
                        from,
                        seqIndex,
                        maxlen);

                idl_printIndent (indent+1);
                idl_fileOutPrintf(idl_fileCur(), "            %s[i%d] = c_stringNew(c_getBase(dbType), (%s)%s.c_str());\n",
                        to,
                        loop_index-1,
                        from,
                        seqIndex);
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "            ");
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "            result = V_COPYIN_RESULT_INVALID;\n");
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "        }\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "            %s[i%d] = c_stringNew(c_getBase(dbType), (%s)%s.c_str());\n",
                        to,
                        loop_index-1,
                        from,
                        seqIndex);
            }
        } else {
            /* This branch is never executed because the first if statement that is added as a copy improvement
               at the top of this method already covers this condition */
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                    to,
                    loop_index-1,
                    scopedName,
                    from,
                    seqIndex);
        }
        break;
    case idl_tstruct:
    case idl_tunion:
        varIndex = 0;
        typeName = idl_scopeStackISOCxx2(idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec));
        funcPrefix = idl_scopedTypeName(typeSpec);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        extern v_copyin_result __%s__copyIn(c_type dbType,\n", funcPrefix);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "            const %s *From,\n",
                typeName);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "            %s *To);\n\n",
                scopedName);
        idl_printIndent (indent);
        idl_fileOutPrintf (
            idl_fileCur(),
            "        result = __%s__copyIn(c_collectionTypeSubType(type%d), &(%s)%s, (%s *)&%s[i%d]);\n",
            funcPrefix,
            loop_index - 1,
            from,
            seqIndex,
            scopedName,
            to,
            loop_index-1);
        os_free(funcPrefix);
        os_free(typeName);
        break;
    case idl_ttypedef:
        idl_printIndent (indent);
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
        case idl_tstruct:
        case idl_tunion:
        case idl_tarray:
            typeName = idl_ISOCxx2TypeFromTypeSpec(typeSpec);
            funcPrefix = idl_scopedTypeName(typeSpec);
            idl_fileOutPrintf (idl_fileCur(), "        if(V_COPYIN_RESULT_IS_OK(result)){\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "            extern v_copyin_result __%s__copyIn(c_type, const %s *, %s *);\n",
                funcPrefix,
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                scopedName);
            idl_printIndent (indent);
            idl_fileOutPrintf (
                idl_fileCur(),
                "            result = __%s__copyIn(c_collectionTypeSubType(type%d), &(%s)%s, (%s *)&%s[i%d]);\n",
                funcPrefix,
                loop_index - 1,
                from,
                seqIndex,
                scopedName,
                to,
                loop_index-1);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        }\n");
            os_free(funcPrefix);
            os_free(typeName);
            break;
        case idl_tbasic:
                /* QAC EXPECT 3416; No side effect here */
            if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                maxlen = idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec))));

                idl_printIndent (indent);

                if(maxlen != 0){
                    idl_fileOutPrintf(idl_fileCur(), "            if((unsigned int)(%s)%s.length() <= %d){\n",
                            from,
                            seqIndex,
                            maxlen);
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf(idl_fileCur(), "            %s[i%d] = c_stringNew(c_getBase(dbType), (%s)%s.c_str());\n",
                            to,
                            loop_index-1,
                            from,
                            seqIndex);
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "            ");
                    idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "            result = V_COPYIN_RESULT_INVALID;\n");
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "        }\n");
                } else {
                    idl_fileOutPrintf (idl_fileCur(), "            %s[i%d] = c_stringNew(c_getBase(dbType), (%s)%s.c_str());\n",
                                to,
                                loop_index-1,
                                from,
                                seqIndex);
                }
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                to,
                loop_index-1,
                scopedName,
                from,
                seqIndex);
            }
            break;
        case idl_tenum:
            maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(typeSpec))));
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)((%s)%s)) >= 0) && (((c_long)((%s)%s)) < %u)){\n",
                    from,
                    seqIndex,
                    from,
                    seqIndex,
                    maxlen);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                    to,
                    loop_index-1,
                    scopedName,
                    from,
                    seqIndex);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        ");
            idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
            break;
        case idl_tseq:
            nextType = idl_typeSeqType(idl_typeSeq(idl_typeDefActual(idl_typeDef(typeSpec))));
            os_free(scopedName);
            scopedName = idl_scopedSplTypeIdent(nextType);
            maxlen = idl_typeSeqMaxSize(idl_typeSeq(idl_typeDefActual(idl_typeDef(typeSpec))));
            snprintf (destin, sizeof(destin), "dest%d", loop_index);

            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    /* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
            idl_printIndent (indent);
            if (loop_index > 0) {
                idl_fileOutPrintf (
                        idl_fileCur(),
                        "        c_type type%d = c_collectionTypeSubType(type%d);\n",
                        loop_index,
                        loop_index - 1);
            } else if (memberIndex > -1) {
                idl_fileOutPrintf (
                        idl_fileCur(),
                        "        c_type type%d = c_memberType(c_structureMember(c_structure(dbType), %d));\n",
                        loop_index,
                        memberIndex);
            } else if (caseIndex > -1) {
                idl_fileOutPrintf (
                        idl_fileCur(),
                        "        c_type type%d = c_unionCaseType(c_unionUnionCase(c_union(dbType), %d));\n",
                        loop_index,
                        caseIndex);
            } else {
                assert(0);
            }
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s *dest%d;\n\n", scopedName, loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        c_long length%d = (%s)%s.size();\n", loop_index, from, seqIndex);

            idl_printIndent (indent);

            if(maxlen > 0){
                idl_fileOutPrintf(idl_fileCur(), "        if(length%d > %u){\n", loop_index, maxlen);
                idl_printIndent (indent);
                idl_fileOutPrintf(idl_fileCur(), "            ");
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent(indent);
                idl_fileOutPrintf(idl_fileCur(), "            result = V_COPYIN_RESULT_INVALID;");
                idl_printIndent(indent);
                idl_fileOutPrintf(idl_fileCur(), "        } else {");
                idl_printIndent(indent);
                idl_fileOutPrintf (idl_fileCur(), "            dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                    loop_index, scopedName, loop_index, loop_index);
                idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+3, userData);
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "            dest%d[i%d] = (c_sequence)dest%d;\n",
                    loop_index-1, loop_index-1, loop_index);
                idl_printIndent(indent);
                idl_fileOutPrintf(idl_fileCur(), "        }\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                    loop_index, scopedName, loop_index, loop_index);
                idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+2, userData);
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        dest%d[i%d] = (c_sequence)dest%d;\n",
                    loop_index-1, loop_index-1, loop_index);
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
        os_free(scopedName);
        scopedName = idl_scopedSplTypeIdent(nextType);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));

        snprintf (destin, sizeof(destin), "dest%d", loop_index);

        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    /* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_printIndent (indent);
        if (loop_index > 0) {
            idl_fileOutPrintf (
                    idl_fileCur(),
                    "        c_type type%d = c_collectionTypeSubType(type%d);\n",
                    loop_index,
                    loop_index - 1);
        } else if (memberIndex > -1) {
            idl_fileOutPrintf (
                    idl_fileCur(),
                    "    c_type type%d = c_memberType(c_structureMember(c_structure(dbType), %d));\n",
                    loop_index,
                    memberIndex);
        } else if (caseIndex > -1) {
            idl_fileOutPrintf (
                    idl_fileCur(),
                    "    c_type type%d = c_unionCaseType(c_unionUnionCase(c_union(dbType), %d));\n",
                    loop_index,
                    caseIndex);
        } else {
            assert(0);
        }
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        %s *dest%d;\n\n", scopedName, loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        c_ulong length%d = (%s)%s.size();\n", loop_index, from, idl_seqIndex(loop_index));
        idl_printIndent (indent);

        idl_fileOutPrintf (idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                loop_index, scopedName, loop_index, loop_index);
        idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+2, userData);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        dest%d[i%d] = (c_sequence)dest%d;\n",
                loop_index-1, loop_index-1, loop_index);
        break;
    default:
        assert (0);
        break;
    }
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    }\n");
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "}\n");

    os_free(scopedName);
    os_free(seqIndex);
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
    idl_typeSeq typeSeq,
    c_long indent,
    void *userData)
{
    c_char destin[32];
    c_ulong maxlen;
    c_char *dbTypeName;
    c_char *scopedName;
    idl_typeSpec nextType;

    nextType = idl_typeSeqType(typeSeq);

    dbTypeName = idl_scopedSplTypeName(nextType);
    scopedName = idl_scopeStack(scope, "_", name);
    snprintf(destin, sizeof(destin), "dest%d", indent);
    maxlen = idl_typeSeqMaxSize(typeSeq);

    idl_fileOutPrintf(idl_fileCur(), "    /* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
    if (indent == 0) {
        idl_fileOutPrintf(idl_fileCur(), "    c_type type%d = c_typeDef(dbType)->alias;\n", indent);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    c_type type%d = c_typeDef(type%d)->alias;\n", indent, indent - 1);
    }
    idl_fileOutPrintf(idl_fileCur(), "    %s *dest%d;\n\n", dbTypeName, indent);
    idl_fileOutPrintf(idl_fileCur(), "    c_ulong length%d = (*from).size();\n", indent);

    if(maxlen != 0){
        idl_fileOutPrintf(idl_fileCur(), "    if(length%d > %u){\n", indent, maxlen);
        idl_fileOutPrintf(idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSeq, name);
        idl_fileOutPrintf(idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
        idl_fileOutPrintf(idl_fileCur(), "    } else {\n");

        idl_fileOutPrintf(idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n", indent, dbTypeName, indent, indent);
        idl_seqLoopCopy(scope, nextType, "*from", destin, 1, indent+2, userData);
        idl_fileOutPrintf(idl_fileCur(), "        *to = (_%s)dest%d;\n", scopedName, indent);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n", indent, dbTypeName, indent, indent);
            idl_seqLoopCopy (scope, nextType, "*from", destin, 1, indent+1, userData);
        idl_fileOutPrintf(idl_fileCur(), "    *to = (_%s)dest%d;\n", scopedName, indent);
    }
    os_free(scopedName);
    os_free(dbTypeName);
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
    c_char *scopedName = idl_scopeStack (scope, "_", name);
    c_char *cxxTypeName = idl_scopeStackISOCxx2 (scope, "::", name);
    idl_typeSpec actualType = idl_typeDefActual(defSpec);

    switch (idl_typeSpecType(idl_typeDefActual(defSpec))) {
    case idl_tstruct:
    case idl_tunion:
        idl_fileOutPrintf (idl_fileCur(), "v_copyin_result\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "    c_type dbType,\n");
        idl_fileOutPrintf (idl_fileCur(), "    const %s *from,\n", cxxTypeName);
        idl_fileOutPrintf (idl_fileCur(), "    _%s *to)\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    v_copyin_result result = V_COPYIN_RESULT_OK;\n\n");
        os_free(scopedName);
        os_free(cxxTypeName);
        scopedName = idl_scopedTypeName(actualType);
        cxxTypeName = idl_ISOCxx2TypeFromTypeSpec(actualType);
        idl_fileOutPrintf (idl_fileCur(), "    extern v_copyin_result __%s__copyIn(c_type, const %s *, _%s *);\n",
            scopedName,
            cxxTypeName,
            scopedName);
        idl_fileOutPrintf (idl_fileCur(), "    result = __%s__copyIn(c_typeDef(dbType)->alias, (%s *)from, (_%s *)to);\n",
            scopedName,
            cxxTypeName,
            scopedName);
        idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tarray:
        idl_fileOutPrintf (idl_fileCur(), "v_copyin_result\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "    c_type dbType,\n");
        idl_fileOutPrintf (idl_fileCur(), "    const %s *from,\n", cxxTypeName);
        idl_fileOutPrintf (idl_fileCur(), "    _%s *to)\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    v_copyin_result result = V_COPYIN_RESULT_OK;\n");
        idl_fileOutPrintf (idl_fileCur(), "    c_type type0 = c_typeDef(dbType)->alias;\n\n");
        idl_fileOutPrintf (idl_fileCur(), "    while(c_baseObjectKind(type0) == M_COLLECTION && c_collectionTypeKind(type0) == OSPL_C_ARRAY) {;\n");
        idl_fileOutPrintf (idl_fileCur(), "        type0 = c_collectionTypeSubType(type0);\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_arrayElements (scope, idl_typeArray(idl_typeDefActual(defSpec)), "*from", "to", 0, OS_FALSE, OS_FALSE, userData);
        idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tseq:
        idl_fileOutPrintf (idl_fileCur(), "v_copyin_result\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "    c_type dbType,\n");
        idl_fileOutPrintf (idl_fileCur(), "    const %s *from,\n", cxxTypeName);
        idl_fileOutPrintf (idl_fileCur(), "    _%s *to)\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    v_copyin_result result = V_COPYIN_RESULT_OK;\n");
        idl_seqElements (scope, name, idl_typeSeq(idl_typeDefActual(defSpec)), 0, userData);
        idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    default:
        break;
    }
    os_free(cxxTypeName);
    os_free(scopedName);
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
 * For unions a copyIn routine named __<scope-elements>_<union-name>__copyIn
 * will be prepared. The signature of this copyIn routine is:
 * @verbatim
   void copyIn (
        c_base base,
        <scope-elements>::<union-name> *from,
        struct _<scope-elements>_<union-name> *to);
   @endverbatim
 *
 * The copyIn routine signature is generated based upon the input
 * parameters which specify the scope and it's name.
 *
 * The copyIn routine will first copy the union discriminant from the
 * source to the target. This code is also generated by this function.
 * The diferent variants of the union are handled by an switch statement
 * that is generated by this function.
 *
 * @verbatim
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
    c_ulong maxlen;
    c_char *scopedName;
    c_char *cxxTypeName;
    c_char *dbTypeName = NULL;

    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(userData);

    scopedName = idl_scopeStack (scope, "_", name);
    cxxTypeName = idl_scopeStackISOCxx2 (scope, "::", name);
    idl_fileOutPrintf (idl_fileCur(), "v_copyin_result\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn(\n", scopedName);
    idl_fileOutPrintf (idl_fileCur(), "    c_type dbType,\n");
    idl_fileOutPrintf (idl_fileCur(), "    const %s *from,\n", cxxTypeName);
    idl_fileOutPrintf (idl_fileCur(), "    struct _%s *to)\n", scopedName);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    v_copyin_result result = V_COPYIN_RESULT_OK;\n");
    idl_fileOutPrintf (idl_fileCur(), "    (void) dbType;\n\n");
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
        dbTypeName = idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec));
        /* String is not allowed here */
        idl_fileOutPrintf (idl_fileCur(), "    to->_d = (%s)from->_d();\n", dbTypeName);
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tenum) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeUnionSwitchKind(unionSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)(from->_d())) >= 0) && (((c_long)(from->_d())) < %d)){\n",
                maxlen);
        dbTypeName = idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec));
        idl_fileOutPrintf (idl_fileCur(), "        to->_d = (%s)from->_d();\n", dbTypeName);
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)unionSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
    } else if ((idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) &&
            (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))) == idl_tbasic)) {
        /* String is not allowed here */
        dbTypeName = idl_scopedSplTypeName (idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec))));
        idl_fileOutPrintf (idl_fileCur(), "    to->_d = (%s)from->_d();\n", dbTypeName);
    } else if ((idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) &&
            (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))) == idl_tenum)) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))));
        dbTypeName = idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec));
        idl_fileOutPrintf (idl_fileCur(), "    if( (((c_long)(from->_d())) >= 0) && (((c_long)(from->_d())) < %d)){\n",
                maxlen);
        idl_fileOutPrintf (idl_fileCur(), "        to->_d = (%s)from->_d();\n", dbTypeName);
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)unionSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "        result = V_COPYIN_RESULT_INVALID;\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    idl_fileOutPrintf (idl_fileCur(), "    switch (to->_d) {\n");

    caseIndex = 0;
    os_free(scopedName);
    os_free(cxxTypeName);
    os_free(dbTypeName);

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
 * The function finalizes the copyIn routine for the union.
 *
 * @param name Name of the union
 */
static void
idl_unionClose (
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf (idl_fileCur(), "    }\n");
    idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");

    caseIndex = -1;
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
 *   via c_stringNew, for other basic types \b idl_typeBasicType is called.
 * - If the type is \b idl_typedef the copy strategy depends on the refered type.
 *   When this type is \b idl_tarray or \b idl_tseq a type specific copy routine
 *   is called. When type is something else, this routine is called recursively with
 *   with the refered type.
 * - If the type is \b idl_tenum, the element is immediately assigned.
 * - If the type is \b idl_tstruct or \b idl_tunion, the copyIn routine of that struct
 *   or union type is called.
 * - If the type is \b idl_tarray the array copy context is set and the service
 *   to generate code for copying arrays is called.
 * - If the type is \b idl_tseq, the sequence copy context is setup and the
 *   service to generate code for copying sequences is called.
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
    c_ulong maxlen;
    c_char *scopedName;
    c_char* cxxName;
    c_char *cxxTypeName;

    cxxName = idl_cxxId(name);
    scopedName = idl_scopedTypeName (typeSpec);
    cxxTypeName = idl_ISOCxx2TypeFromTypeSpec(typeSpec);

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* QAC EXPECT 3416; No side effect here */
        idl_basicCaseType (scope, cxxName, idl_typeBasic(typeSpec), "from->", "to->_u.");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        if (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tarray ||
                idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tseq ||
                idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tstruct ||
                idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tunion) {
                idl_fileOutPrintf (idl_fileCur(), "        if(V_COPYIN_RESULT_IS_OK(result)){\n");
                idl_fileOutPrintf (idl_fileCur(), "            extern v_copyin_result __%s__copyIn(c_type, const %s *, _%s *);\n",
                    scopedName,
                    cxxTypeName,
                    scopedName);
                idl_fileOutPrintf (idl_fileCur(), "            const %s &x = from->%s();\n",
                    cxxTypeName,
                    cxxName);
                idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn(c_unionCaseType(c_unionUnionCase(dbType, %d)), &x, &to->_u.%s);\n",
                    scopedName,
                    caseIndex,
                    cxxName);
                idl_fileOutPrintf (idl_fileCur(), "        }\n");
                idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        } else {
            /* Calls itself for the actual type in case of typedef */
            /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
            idl_unionCaseOpenClose (
                scope,
                name,
                idl_typeDefRefered(idl_typeDef(typeSpec)),
                userData);
            /* Compensate for recursion, where caseIndex will now be incremented twice. */
            caseIndex--;
        }
            /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));
        os_free(scopedName);
        scopedName = idl_scopedSplTypeName (typeSpec);

        idl_fileOutPrintf (idl_fileCur(), "        if((((c_long)(from->%s())) >= 0) && (((c_long)(from->%s())) < %d)){\n",
                cxxName,
                cxxName,
                maxlen);
        idl_fileOutPrintf (idl_fileCur(), "            to->_u.%s = (%s)from->%s();\n",
                cxxName,
                scopedName,
                cxxName);
        idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
        idl_fileOutPrintf (idl_fileCur(), "            ");
        idl_boundsCheckFail(CASE, scope, (idl_typeSpec)typeSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "            result = V_COPYIN_RESULT_INVALID;\n");
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
            idl_typeSpecType(typeSpec) == idl_tunion) {
        idl_fileOutPrintf (idl_fileCur(), "        if(V_COPYIN_RESULT_IS_OK(result)){\n");
        idl_fileOutPrintf (idl_fileCur(), "            extern v_copyin_result __%s__copyIn(c_type, const %s *, _%s *);\n",
            scopedName,
            cxxTypeName,
            scopedName);
        idl_fileOutPrintf (idl_fileCur(), "            const %s &x = from->%s();\n",
            cxxTypeName,
            cxxName);
        idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn(c_unionCaseType(c_unionUnionCase(dbType, %d)), &x, &to->_u.%s);\n",
            scopedName,
            caseIndex,
            cxxName);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        c_char source[256];

        os_free(scopedName);
        scopedName = idl_scopedSplTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec)));
        snprintf(source, sizeof(source), "from->%s()", name);
        idl_fileOutPrintf (idl_fileCur(), "        if(V_COPYIN_RESULT_IS_OK(result)){\n");
        idl_fileOutPrintf (idl_fileCur(), "            typedef %s _DestType", scopedName);
        idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
        idl_fileOutPrintf (idl_fileCur(), ";\n");
        idl_fileOutPrintf (idl_fileCur(), "            _DestType *dest = &to->_u.%s;\n", cxxName);
        idl_fileOutPrintf (idl_fileCur(), "            c_type type0 = c_unionCaseType(c_unionUnionCase(dbType, %d));\n", caseIndex);
        idl_fileOutPrintf (idl_fileCur(), "            while(c_baseObjectKind(type0) == M_COLLECTION && c_collectionTypeKind(type0) == OSPL_C_ARRAY) {;\n");
        idl_fileOutPrintf (idl_fileCur(), "                type0 = c_collectionTypeSubType(type0);\n");
        idl_fileOutPrintf (idl_fileCur(), "            }\n");
        idl_arrayElements (scope, idl_typeArray(typeSpec), source, "dest", 2, OS_FALSE, OS_FALSE, userData);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
        idl_typeSpec nextType;

        os_free(scopedName);
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedSplTypeName(nextType);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        snprintf (type_name, sizeof(type_name), "_%s_seq", name);

        idl_fileOutPrintf (idl_fileCur(), "    /* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_fileOutPrintf (idl_fileCur(), "    if(V_COPYIN_RESULT_IS_OK(result)){\n");
        idl_fileOutPrintf (idl_fileCur(), "        c_type type0 = c_unionCaseType(c_unionUnionCase(dbType, %d));\n", caseIndex);
        idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "        const %s *src = &(from->%s());\n\n", cxxTypeName, cxxName);
        idl_fileOutPrintf (idl_fileCur(), "        c_long length0 = (c_long)(*src).size();\n");

        if(maxlen != 0){
            idl_fileOutPrintf(idl_fileCur(), "        if(length0 > %u){\n", maxlen);
            idl_fileOutPrintf(idl_fileCur(), "            ");
            idl_boundsCheckFail(CASE, scope, (idl_typeSpec)typeSpec, name);
            idl_fileOutPrintf(idl_fileCur(), "            result = OS_C_FALSE;\n");
            idl_fileOutPrintf(idl_fileCur(), "        } else {\n");

            idl_fileOutPrintf (idl_fileCur(), "            dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 3, userData);
            idl_fileOutPrintf (idl_fileCur(), "            to->_u.%s = (c_sequence)dest0;\n", cxxName);

            idl_fileOutPrintf(idl_fileCur(), "        }\n");
        } else {
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->_u.%s = (c_sequence)dest0;\n", cxxName);
        }
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        idl_fileOutPrintf(idl_fileCur(), "    break;\n");
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    /* QAC EXPECT 5101, 5103: Complexity is limited, by independent cases, per case the number of lines is lower  */

    caseIndex++;
    os_free(cxxTypeName);
    os_free(scopedName);
    os_free(cxxName);
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
    OS_UNUSED_ARG(ownScope);
    OS_UNUSED_ARG(userData);

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(labelVal) == idl_ldefault) {
        idl_fileOutPrintf (idl_fileCur(), "    default:\n");
    } else {
        idl_fileOutPrintf (idl_fileCur(), "    case %s:\n", idl_valueFromLabelVal(labelVal));
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

static void
idl_artificialDefaultLabelOpenClose(
    idl_scope scope,
    idl_labelVal labelVal,
    idl_typeSpec typeSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(labelVal);
    OS_UNUSED_ARG(typeSpec);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf (idl_fileCur(), "    default:\n");
    idl_fileOutPrintf (idl_fileCur(), "    break;\n");
}

/**
 * Specifies the callback table for the CORBA C++ CopyIn generation functions.
 */
static struct idl_program
idl_genISOCxx2Copyin = {
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
    idl_artificialDefaultLabelOpenClose, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/** @brief return the callback table for the CORBA C++ CopyIn generation functions.
 */
idl_program
idl_genISOCxx2CopyinProgram (
        void *userData)
{
    idl_genISOCxx2Copyin.userData = userData;
    return &idl_genISOCxx2Copyin;
}
