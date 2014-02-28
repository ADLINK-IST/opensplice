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
 * This module generates CORBA C++ CopyIn functions. It handles input types
 * that match the IDL/C++ mapping as specified by the OMG and writes the
 * data into data types as applicable for the OpenSpliceDDS database.
 */

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCorbaCxxCopyin.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"

#include "c_typebase.h"
#include "os_heap.h"
#include "os_stdlib.h"

#define BOUNDSCHECK ("OSPL_BOUNDS_CHECK")
    /** Text indentation level (4 spaces per indent) */
static c_long loopIndent;
    /** Index for array loop variables, incremented for each array dimension */
static c_long varIndex;

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
                snprintf(labelName, (size_t) sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal) ).is.UShort);
                break;
            case V_LONG:
                snprintf(labelName, (size_t) sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal) ).is.Long);
                break;
            case V_ULONG:
                snprintf(labelName, (size_t) sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal) ).is.ULong);
                break;
            case V_LONGLONG:
                snprintf(labelName, (size_t) sizeof(labelName), "%lld", idl_labelValueVal(idl_labelValue(labelVal) ).is.LongLong);
                break;
            case V_ULONGLONG:
                snprintf(labelName, (size_t) sizeof(labelName), "%llu", idl_labelValueVal(idl_labelValue(labelVal) ).is.ULongLong);
                break;
            case V_BOOLEAN:
                /* QAC EXPECT 3416; No side effect here */
                if ((int) idl_labelValueVal(idl_labelValue(labelVal) ).is.Boolean == OS_C_TRUE) {
                    snprintf(labelName, (size_t) sizeof(labelName), "OS_C_TRUE");
                } else {
                    snprintf(labelName, (size_t) sizeof(labelName), "OS_C_FALSE");
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
    idl_fileOutPrintf (idl_fileCur(), "#include <v_kernel.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <v_topic.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <os_stdlib.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <string.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <os_report.h>\n");
    if(idl_getIsISOCpp())
    {
      idl_fileOutPrintf (idl_fileCur(), "#include <org/opensplice/core/EntityRegistry.hpp>\n");
    }
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
        <scope-elements>::<structure-name> *from,
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
    const char * suffix = (char *) userData;

    idl_fileOutPrintf (idl_fileCur(), "c_bool\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn%s(\n",
            idl_scopeStack (scope, "_", name),
            suffix);
    idl_fileOutPrintf (idl_fileCur(), "    c_base base,\n");
    if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
    {
      idl_fileOutPrintf (idl_fileCur(), "    class %s *from,\n",
                         idl_scopeStackCxx (scope, "::", name));
    }
    else
    {
      idl_fileOutPrintf (idl_fileCur(), "    struct %s *from,\n",
                         idl_scopeStackCxx (scope, "::", name));
    }
    idl_fileOutPrintf (idl_fileCur(), "    struct _%s *to)\n",
    idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    c_bool result = OS_C_TRUE;\n");
    idl_fileOutPrintf (idl_fileCur(), "    (void) base;\n\n");

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
    idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
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
    c_long maxlen;
    c_char* cid;

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
        idl_fileOutPrintf (idl_fileCur(), "    %s%s = (%s)%s%s%s;\n",
                to_id,
                cid,
                idl_typeFromTypeSpec (idl_typeSpec(typeBasic)),
                from_id,
                cid,
                idl_isocppCxxStructMemberSuffix());
        break;
    case idl_string:
        maxlen = idl_typeBasicMaxlen(typeBasic);
        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
            if (maxlen != 0)
            {
              idl_fileOutPrintf (idl_fileCur(),"        if(((unsigned int)%s%s_.length())) <= %d){\n",
                          from_id,
                          cid,
                          maxlen);
              if(stacRequested)
              {
                  idl_fileOutPrintf (idl_fileCur(),"            /* The strncpy takes a size of the maximum string bounds plus 1, as the database size accomodates this */\n");
                  idl_fileOutPrintf(idl_fileCur(), "            strncpy((%s%s), %s%s_.c_str(), %d);\n",
                          to_id,
                          cid,
                          from_id,
                          cid,
                          (maxlen+1));
              } else
              {
                  idl_fileOutPrintf(idl_fileCur(), "            %s%s = c_stringNew(base, %s%s_.c_str());\n",
                          to_id,
                          cid,
                          from_id,
                          cid);
              }
              idl_fileOutPrintf (idl_fileCur(),"        } else {\n");
              idl_fileOutPrintf (idl_fileCur(),"            ");
              idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeBasic, name);
              idl_fileOutPrintf (idl_fileCur(),"            result = OS_C_FALSE;\n");
              idl_fileOutPrintf (idl_fileCur(),"        }\n");
            }
            else {
                idl_fileOutPrintf (idl_fileCur(), "        %s%s = c_stringNew(base, %s%s_.c_str());\n",
                        to_id,
                        cid,
                        from_id,
                        cid);
            }
        }
        else
        {
            idl_fileOutPrintf (idl_fileCur(),"    if(%s%s){\n",
                    from_id,
                    cid);
            if(maxlen != 0)
            {
                /* strlen must be smaller or equal to the maxLen. Be aware that maxLen value here is still
                * the same value as defined in the IDL file. And be aware that the character array in the database
                * which will store the contents of the bounded string has been increased with 1 to accomodate the
                * '\0' character. That is why the actual copy takes maxLen plus 1.
                */
                idl_fileOutPrintf (idl_fileCur(),"        if(((unsigned int)strlen(%s%s)) <= %d){\n",
                        from_id,
                        cid,
                        maxlen);
                if(stacRequested)
                {
                    idl_fileOutPrintf (idl_fileCur(),"            /* The strncpy takes a size of the maximum string bounds plus 1, as the database size accomodates this */\n");
                    idl_fileOutPrintf(idl_fileCur(), "            strncpy((%s%s), %s%s, %d);\n",
                            to_id,
                            cid,
                            from_id,
                            cid,
                            (maxlen+1));
                } else
                {
                    idl_fileOutPrintf(idl_fileCur(), "            %s%s = c_stringNew(base, %s%s);\n",
                            to_id,
                            cid,
                            from_id,
                            cid);
                }
                idl_fileOutPrintf (idl_fileCur(),"        } else {\n");
                idl_fileOutPrintf (idl_fileCur(),"            ");
                idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeBasic, name);
                idl_fileOutPrintf (idl_fileCur(),"            result = OS_C_FALSE;\n");
                idl_fileOutPrintf (idl_fileCur(),"        }\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        %s%s = c_stringNew(base, %s%s);\n",
                        to_id,
                        cid,
                        from_id,
                        cid);
            }

            idl_fileOutPrintf (idl_fileCur(),"    } else {\n");
            idl_fileOutPrintf (idl_fileCur(),"        ");
            idl_boundsCheckFailNull(MEMBER, scope, (idl_typeSpec)typeBasic, name);
            idl_fileOutPrintf (idl_fileCur(),"        result = OS_C_FALSE;\n");
            idl_fileOutPrintf (idl_fileCur(),"    }\n");
        }
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
          if(stacRequested)
          {
              idl_fileOutPrintf (idl_fileCur(),"            /* The strncpy takes a size of the maximum string bounds plus 1, as the database size accomodates this */\n");
              idl_fileOutPrintf(idl_fileCur(), "            strncpy((%s%s), %s%s_.c_str(), %d);\n",
                      to_id,
                      cid,
                      from_id,
                      cid,
                      (maxlen+1));
          } else
          {
              idl_fileOutPrintf(idl_fileCur(), "            %s%s = c_stringNew(base, %s%s_.c_str());\n",
                      to_id,
                      cid,
                      from_id,
                      cid);
          }
        } else {
          if(stacRequested)
          {
              idl_fileOutPrintf (idl_fileCur(),"            /* The strncpy takes a size of the maximum string bounds plus 1, as the database size accomodates this */\n");
              idl_fileOutPrintf(idl_fileCur(), "            strncpy((%s%s), %s%s, %d);\n",
                      to_id,
                      cid,
                      from_id,
                      cid,
                      (maxlen+1));
          } else
          {
              idl_fileOutPrintf (idl_fileCur(), "    %s%s = c_stringNew(base, %s%s);\n",
                      to_id,
                      cid,
                      from_id,
                      cid);
          }
        }
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        break;
    default:
        printf ("idl_basicMemberType: Unexpected basic type\n");
        break;
    }

    os_free(cid);
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
    c_long maxlen;
    c_char* cid;

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
        idl_fileOutPrintf (idl_fileCur(), "        %s%s = (%s)%s%s();\n",
                to_id,
                cid,
                idl_typeFromTypeSpec (idl_typeSpec(typeBasic)),
                from_id,
                cid);
        break;
    case idl_string:
        maxlen = idl_typeBasicMaxlen(typeBasic);

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_fileOutPrintf (idl_fileCur(),    "    if(%s%s){\n",
                from_id,
                cid);

        if(maxlen != 0){
            idl_fileOutPrintf (idl_fileCur(),"        if(((unsigned int)strlen(%s%s)) <= %d){\n",
                    from_id,
                    cid,
                    maxlen);
            idl_fileOutPrintf(idl_fileCur(), "            %s%s = c_stringNew(base, %s%s);\n",
                    to_id,
                    cid,
                    from_id,
                    cid);
            idl_fileOutPrintf (idl_fileCur(),"        } else {\n");
            idl_fileOutPrintf (idl_fileCur(),"            ");
            idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeBasic, name);
            idl_fileOutPrintf (idl_fileCur(),"            result = OS_C_FALSE;\n");
            idl_fileOutPrintf (idl_fileCur(),"        }\n");
        } else {
            idl_fileOutPrintf (idl_fileCur(), "        %s%s = c_stringNew(base, %s%s());\n",
                    to_id,
                    cid,
                    from_id,
                    cid);
        }
        idl_fileOutPrintf (idl_fileCur(),    "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(),    "        ");
        idl_boundsCheckFailNull(MEMBER, scope, (idl_typeSpec)typeBasic, name);
        idl_fileOutPrintf (idl_fileCur(),    "        result = OS_C_FALSE;\n");
        idl_fileOutPrintf (idl_fileCur(),    "    }\n");
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_fileOutPrintf (idl_fileCur(), "    %s%s = c_stringNew(base, %s%s());\n",
                to_id,
                cid,
                from_id,
                cid);
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        break;
    default:
        printf ("idl_basicCaseType: Unexpected basic type\n");
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
    c_long maxlen;
    c_char *typeName;
    c_char *scopedName;
    c_char* cid;
    const char * suffix = (char *)userData;

    cid = idl_cxxId(name);

    /* Expected types: idl_tbasic, idl_ttypedef, idl_tenum, idl_tstruct, idl_tunion, idl_tarray, idl_tseq */

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        os_boolean stacRequested = OS_FALSE;
        /* is a stac pragma defined for this member? */
        stacRequested = idl_stacDef_isStacDefined(scope, name, typeSpec, NULL);
        /* Handles all basic types, inclusive strings */
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
            /* if cats or stac is defined then resolve the first level of
            * typedef and recurse deeper into the operation
            */
            typeDereffered = idl_typeDefResolveFully(typeSpec);
            idl_structureMemberOpenClose (
                scope,
                name,
                typeDereffered,
                userData);
        } else
        {
            /* QAC EXPECT 3416; No side effect here */
            if ((idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tstruct) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tunion) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tarray) ||
                (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tseq)) {

                idl_fileOutPrintf (idl_fileCur(), "    if(result){\n");
                idl_fileOutPrintf (idl_fileCur(), "        extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
                    idl_scopedTypeName (typeSpec),
                    suffix,
                    idl_corbaCxxTypeFromTypeSpec(typeSpec),
                    idl_scopedTypeName (typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "        result = __%s__copyIn%s(base, &from->%s%s, &to->%s);\n",
                    idl_scopedTypeName (typeSpec),
                    suffix,
                    cid,
                    idl_isocppCxxStructMemberSuffix(),
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
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)from->%s%s) >= 0) && (((c_long)from->%s%s) < %d) ){\n",
            cid ,
            idl_isocppCxxStructMemberSuffix(),
            cid,
            idl_isocppCxxStructMemberSuffix(),
            maxlen);
        idl_fileOutPrintf (idl_fileCur(), "        to->%s = (%s)from->%s%s;\n",
            cid,
            idl_scopedSplTypeName (typeSpec),
            cid,
            idl_isocppCxxStructMemberSuffix());
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(),"        ");
        idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");

        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_fileOutPrintf (idl_fileCur(), "    to->%s = (%s)from->%s%s;\n",
        cid,
        idl_scopedSplTypeName (typeSpec),
        cid,
        idl_isocppCxxStructMemberSuffix());
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
               (idl_typeSpecType(typeSpec) == idl_tunion)) {
        idl_fileOutPrintf (idl_fileCur(), "    if(result){\n");
        idl_fileOutPrintf (idl_fileCur(), "        extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
            idl_scopedTypeName (typeSpec),
            suffix,
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
            idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "        result = __%s__copyIn%s(base, &from->%s%s, &to->%s);\n",
            idl_scopedTypeName (typeSpec),
            suffix,
            cid,
            idl_isocppCxxStructMemberSuffix(),
            cid);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray)
    {
        c_char source[256];
        snprintf(source, (size_t)sizeof(source), "from->%s%s", name, idl_isocppCxxStructMemberSuffix());
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
                idl_fileOutPrintf(idl_fileCur(),"    {\n");
                idl_arrayElements (scope, idl_typeArray(typeSpec), source, cid, 1, stacRequested, catsRequested, userData);
                idl_fileOutPrintf(idl_fileCur(),"    }\n");
            } else if(stacRequested)
            {
                assert(!catsRequested);
                assert(baseTypeDereffered);
                idl_fileOutPrintf (idl_fileCur(),"    {\n");
                idl_fileOutPrintf (idl_fileCur(),"        typedef c_char _DestType");
                idl_arrayDimensions(idl_typeArray(typeSpec), OS_TRUE);
                idl_fileOutPrintf (idl_fileCur(),"[%d]", idl_typeBasicMaxlen(idl_typeBasic(baseTypeDereffered))+1);
                idl_fileOutPrintf (idl_fileCur(),";\n");
                idl_fileOutPrintf (idl_fileCur(),"        _DestType *dest = &to->%s;\n", cid);
                idl_arrayElements (scope, idl_typeArray(typeSpec), source, "dest", 1, stacRequested, catsRequested, userData);
                idl_fileOutPrintf (idl_fileCur(),"    }\n");
            }else
            {
                idl_typeSpec subType;

                subType = idl_typeArrayActual(idl_typeArray(typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "    {\n");
                idl_fileOutPrintf (idl_fileCur(), "        typedef %s _DestType",idl_scopedSplTypeIdent (subType));
                idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
                idl_fileOutPrintf (idl_fileCur(), ";\n");
                idl_fileOutPrintf (idl_fileCur(), "        _DestType *dest = &to->%s;\n", cid);
                idl_arrayElements (scope, idl_typeArray(typeSpec), source, "dest", 1, stacRequested, catsRequested, userData);
                idl_fileOutPrintf (idl_fileCur(), "    }\n");
            }
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
        idl_typeSpec nextType;

        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedSplTypeName(nextType);

        snprintf (type_name, (size_t)sizeof(type_name), "_%s_seq", name);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));

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
        idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_fileOutPrintf (idl_fileCur(), "        %sc_type type0 = NULL;\n", *suffix == '\0' ? "static " : "");
        idl_fileOutPrintf (idl_fileCur(), "        c_type subtype0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        c_long length0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "        %s *src = &from->%s%s;\n\n", idl_scopeStackCxx (scope, "::", type_name), cid, idl_isocppCxxStructMemberSuffix());
        /* QAC EXPECT 5001; Bypass qactools bug */
        idl_fileOutPrintf (idl_fileCur(), "        if (type0 == NULL) {\n");
        idl_fileOutPrintf (idl_fileCur(), "            subtype0 = c_type(c_metaResolve (c_metaObject(base), \"%s\"));\n", typeName);
        if(maxlen > 0){
            idl_fileOutPrintf (idl_fileCur(), "            type0 = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s,%d>\",subtype0,%d);\n", typeName, maxlen, maxlen);
        } else {
            idl_fileOutPrintf (idl_fileCur(), "            type0 = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s>\",subtype0,0);\n", typeName);
        }
        idl_fileOutPrintf (idl_fileCur(), "            c_free(subtype0);\n");
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
            idl_fileOutPrintf (idl_fileCur(), "        length0 = (c_long)(*src).size();\n");
        }
        else
        {
            idl_fileOutPrintf (idl_fileCur(), "        length0 = (c_long)(*src).length();\n");
        }
        idl_fileOutPrintf (idl_fileCur(), "#ifdef %s\n", BOUNDSCHECK);

        if(maxlen){
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "    if(length0 > %d){\n", maxlen);
            idl_printIndent(1);
            idl_fileOutPrintf (idl_fileCur(),"        ");
            idl_boundsCheckFail(MEMBER, scope, (idl_typeSpec)typeSpec, name);
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "        result = OS_C_FALSE;\n");
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "    } else {\n");
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0),length0);\n", scopedName);
            idl_seqLoopCopy(scope, nextType, "*src", "dest0", 1, 3, userData);
            idl_fileOutPrintf(idl_fileCur(), "            to->%s = (c_sequence)dest0;\n", cid);
            idl_fileOutPrintf(idl_fileCur(), "        }\n");
        } else {
            idl_printIndent(1);
            idl_fileOutPrintf(idl_fileCur(), "     dest0 = (%s *)c_newSequence(c_collectionType(type0),length0);\n", scopedName);
            idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_printIndent(1);
            idl_fileOutPrintf (idl_fileCur(), "    to->%s = (c_sequence)dest0;\n",
                    idl_cxxId(name));
        }
        idl_fileOutPrintf(idl_fileCur(), "#else\n");
        idl_fileOutPrintf(idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0),length0);\n", scopedName);
        idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
        idl_fileOutPrintf(idl_fileCur(), "        to->%s = (c_sequence)dest0;\n", cid);
        idl_fileOutPrintf(idl_fileCur(), "#endif\n");
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        if(*suffix != '\0'){
            idl_fileOutPrintf(idl_fileCur(), "    c_free(type0);\n");
        }
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    /* QAC EXPECT 5103; Code is clearly separated in a number of cases of which each is maintainable */

    os_free(cid);
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
    snprintf(arrIndex, (size_t)sizeof(arrIndex), "[i%d]", varIndex);
    os_strncat(indexString, arrIndex, (size_t)sizeof(arrIndex));
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
    c_long total_indent, maxlen;
    const c_char* suffix = (c_char *)userData;

    loopIndent++;
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tstruct:
    case idl_tunion:
        idl_printIndent(loopIndent + indent);
        varIndex = 0;
        idl_fileOutPrintf(idl_fileCur(), "extern c_bool __%s__copyIn%s(c_base base,\n",
            idl_scopedTypeName(typeSpec),
            suffix);
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "%s *From,\n",
            idl_scopeStack(idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec)));
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "%s *To);\n\n",
            idl_scopedSplTypeIdent(typeSpec));
        idl_fileOutPrintf(idl_fileCur(), "if(result){\n");
        indent++;
        idl_printIndent (loopIndent + indent);
        idl_fileOutPrintf (idl_fileCur(),"result = __%s__copyIn%s(base, (%s *)&(%s)",
            idl_scopedTypeName(typeSpec),
            suffix,
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
            from);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ", (%s *)&(*%s)",
            idl_scopedSplTypeName(typeSpec),
            to);
        idl_arrayLoopCopyIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ");\n");
        indent--;
        idl_printIndent(loopIndent + indent);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
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
                    idl_typeArray(idl_typeDefActual(idl_typeDef(typeSpec))),
                    from,
                    to,
                    indent,
                    stacRequested,
                    catsRequested,
                    userData);
            } else
            {
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
        } else
        {
            switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
            case idl_tstruct:
            case idl_tunion:
            case idl_tarray:
            case idl_tseq:
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "extern c_bool __%s__copyIn(c_base base,",
                    idl_scopedTypeName(typeSpec));
                idl_fileOutPrintf(idl_fileCur(), "%s *From,",
                    idl_scopeStack(idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec)));
                idl_fileOutPrintf(idl_fileCur(), "%s *To);\n\n",
                    idl_scopedSplTypeIdent(typeSpec));
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "if(result){\n");
                indent++;
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(),"result = __%s__copyIn(base, (%s *)&(%s)",
                    idl_scopedTypeName(typeSpec),
                    idl_corbaCxxTypeFromTypeSpec(typeSpec),
                from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ", (%s *)&(*%s)",
                    idl_scopedSplTypeName(typeSpec),
                to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ");\n");
                indent--;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "}\n");
                break;
            case idl_tbasic:
                /* QAC EXPECT 3416; No side effect here */
                if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                    maxlen = idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec))));

                    idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(), "if(result){\n");
                    indent++;
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(), "if((%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(), "){\n", from);
                    indent++;
                    if(maxlen != 0){
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"if(((unsigned int)strlen((%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),")) <= %d){\n", maxlen);
                        indent++;
                        idl_printIndent(loopIndent + indent);
                        if(stacRequested)
                        {
                            idl_fileOutPrintf (idl_fileCur(),"            strncpy((*%s", to);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf (idl_fileCur(),"), (%s", from);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf (idl_fileCur(),"), %d);\n", (maxlen+1));
                        } else
                        {
                            idl_fileOutPrintf(idl_fileCur(), "%s", to);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(base, (%s)", from);
                            idl_arrayLoopCopyIndex(typeArray);
                            idl_fileOutPrintf (idl_fileCur(),");\n");
                        }
                        indent--;
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"} else {\n");
                        indent++;
                        idl_printIndent(loopIndent + indent);
                        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"result = OS_C_FALSE;\n");
                        indent--;
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(),"}\n");
                    } else {
                        idl_printIndent(loopIndent + indent);
                        idl_fileOutPrintf (idl_fileCur(), "%s", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(), " = c_stringNew(base, (%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),");\n");
                    }
                    indent--;
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(),"} else {\n");
                    indent++;
                    idl_printIndent(loopIndent + indent);
                    idl_boundsCheckFailNull(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(),"result = OS_C_FALSE;\n");
                    indent--;
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(),"}\n");
                    indent--;
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(),"}\n");
                    idl_fileOutPrintf (idl_fileCur(),"#else\n");
                    idl_printIndent(loopIndent + indent);
                    if(stacRequested)
                    {
                        idl_fileOutPrintf (idl_fileCur(),"        strncpy((*%s", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),"), (%s", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),"), %d);\n", (maxlen+1));
                    } else
                    {
                        idl_fileOutPrintf (idl_fileCur(), "%s", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(), " = c_stringNew(base, (%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),");\n");
                    }
                    idl_fileOutPrintf (idl_fileCur(),"#endif\n");
                } else {
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf(idl_fileCur(), "%s", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), " = %s;\n", from);
                    idl_arrayLoopCopyIndex(typeArray);
                }
                break;
            case idl_tenum:
                maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(typeSpec))));

                idl_fileOutPrintf(idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "if( (((c_long)%s", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ") >= 0) && (((c_long)%s", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ") < %d)){\n", maxlen);
                indent++;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf(idl_fileCur(), "%s", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), " = %s", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf(idl_fileCur(), ";\n", from);
                indent--;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
                indent++;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "        ");
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
                indent--;
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "    }\n");
                idl_fileOutPrintf (idl_fileCur(),"#else\n");
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "%s", to);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), " = %s", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), ";\n", from);
                idl_fileOutPrintf (idl_fileCur(),"#endif\n");
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

                idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
                idl_printIndent(loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "if((%s)", from);
                idl_arrayLoopCopyIndex(typeArray);
                idl_fileOutPrintf (idl_fileCur(), "){\n");
                indent++;
                if(maxlen != 0){
                    idl_printIndent(loopIndent + indent);
                    idl_fileOutPrintf(idl_fileCur(), "if(((unsigned int)(strlen((%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), "))) <= %d){\n", maxlen);
                    indent++;
                    if(stacRequested)
                    {
                        idl_fileOutPrintf (idl_fileCur(),"strncpy((*%s)", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),", (%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf (idl_fileCur(),", %d);\n", (maxlen+1));
                    }
                    else
                    {
                        idl_printIndent (loopIndent + indent);
                        idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(base, (%s)", from);
                        idl_arrayLoopCopyIndex(typeArray);
                        idl_fileOutPrintf(idl_fileCur(), ");\n");
                    }
                    indent--;
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(), "} else {\n");
                    indent++;
                    idl_printIndent (loopIndent + indent);
                    idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(), "    result = OS_C_FALSE;\n");
                    indent--;
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf (idl_fileCur(), "}\n");
                } else {
                    idl_printIndent (loopIndent + indent);
                    idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(base, (%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), ");\n");
                }
                indent--;
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "} else {\n");
                indent++;
                idl_printIndent(loopIndent + indent);
                idl_boundsCheckFailNull(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "result = OS_C_FALSE;\n");
                indent--;
                idl_printIndent (loopIndent + indent);
                idl_fileOutPrintf (idl_fileCur(), "}\n");

                idl_fileOutPrintf (idl_fileCur(),"#else\n");
                idl_printIndent (loopIndent + indent);
                if(stacRequested)
                {
                    idl_fileOutPrintf (idl_fileCur(),"strncpy((*%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(),", (%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf (idl_fileCur(),", %d);\n", (maxlen+1));
                }
                else
                {
                    idl_fileOutPrintf(idl_fileCur(), "(*%s)", to);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), " = c_stringNew(base, (%s)", from);
                    idl_arrayLoopCopyIndex(typeArray);
                    idl_fileOutPrintf(idl_fileCur(), ");\n");
                }
                idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        } else {
            /* QAC EXPECT 3416; No side effect here */
            assert (0);
        }
        break;
    case idl_tseq:
        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedSplTypeName(nextType);

        total_indent = indent+idl_indexSize(typeArray);

        if (idl_typeSpecType(nextType) == idl_tbasic) {
            if (idl_typeBasicMaxlen(idl_typeBasic(nextType)) > 0) {
                typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
            } else {
                typeName = idl_typeSpecName(nextType);
            }
        } else {
            typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
        }
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));

        snprintf(source, (size_t)sizeof(source), "(%s)", from);
        snprintf(destin, (size_t)sizeof(destin), "(*%s)", to);

        idl_arrayLoopCopyIndexString (source, typeArray);
        idl_arrayLoopCopyIndexString (destin, typeArray);

        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    %sc_type type0 = NULL;\n", *suffix == '\0' ? "static " : "");
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    c_type subtype0;\n");
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    c_long length0;\n");
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s *dest0;\n\n", scopedName);
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    if (type0 == NULL) {\n");
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "            subtype0 = c_type(c_metaResolve (c_metaObject(base), \"%s\"));\n", typeName);
        idl_printIndent (total_indent);
        if(maxlen > 0){
            idl_fileOutPrintf (idl_fileCur(), "            type0 = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s,%d>\",subtype0,%d);\n", typeName, maxlen, maxlen);
        } else {
            idl_fileOutPrintf (idl_fileCur(), "            type0 = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s>\",subtype0,0);\n", typeName);
        }
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "            c_free(subtype0);\n");
        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");

        idl_printIndent (total_indent);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
          idl_fileOutPrintf (idl_fileCur(), "    length0 = (c_long)(%s).size();\n", source);
        }
        else
        {
          idl_fileOutPrintf (idl_fileCur(), "    length0 = (c_long)(%s).length();\n", source);
        }

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);

        if(maxlen){
            idl_fileOutPrintf(idl_fileCur(), "    if(length0 > %d){\n", maxlen);
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        ");
            idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        result = OS_C_FALSE;\n");
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "    } else {\n");
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0),length0);\n", scopedName);
                idl_seqLoopCopy(scope, nextType, source, "dest0", 1, total_indent+2, userData);

            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "        %s = (c_sequence)dest0;\n",
                destin);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
        } else {
            idl_printIndent(total_indent);
            idl_fileOutPrintf(idl_fileCur(), "    dest0 = (%s *)c_newSequence(c_collectionType(type0),length0);\n", scopedName);
                idl_seqLoopCopy(scope, nextType, source, "dest0", 1, total_indent+1, userData);

            idl_printIndent (total_indent);
            idl_fileOutPrintf (idl_fileCur(), "    %s = (c_sequence)dest0;\n", destin);
        }
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_printIndent (total_indent);
        idl_fileOutPrintf(idl_fileCur(), "    dest0 = (%s *)c_newSequence(c_collectionType(type0),length0);\n", scopedName);
        idl_seqLoopCopy(scope, nextType, source, "dest0", 1, total_indent+1, userData);

        idl_printIndent (total_indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s = (c_sequence)dest0;\n", destin);
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        if(*suffix != '\0'){
            idl_fileOutPrintf (idl_fileCur(), "    c_free(type0);\n");
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
    c_long maxLen;
    char *buf;
    const c_char *suffix = (c_char*)userData;

    char* from;
    if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
    {
        from = os_malloc(strlen(from_) + strlen("().data()") + 1);
        os_sprintf(from, "%s%s%s", "(", from_, ").data()");
    }
    else
    {
        from = os_malloc(strlen(from_) + 1);
        os_sprintf(from, "%s", from_);
    }
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
            idl_arrayLoopCopy (typeArray, from, to, indent, scope, stacRequested, catsRequested, userData);
        } else {
            /*stac requested should be covered by the above if statement, as
             * stac always involves strings. And if this basic type is not a string
             * then stac requested can not be true
             */
            assert(!stacRequested);
            if(catsRequested)
            {
                idl_fileOutPrintf(idl_fileCur(),"        /* Allocate the length of the array (and null terminator) as a database\n");
                idl_fileOutPrintf(idl_fileCur(),"        * string\n");
                idl_fileOutPrintf(idl_fileCur(),"        */\n");
                idl_fileOutPrintf(idl_fileCur(),"        to->%s = c_stringMalloc(base, (%d + 1));\n", to, idl_typeArraySize(typeArray));
                idl_fileOutPrintf(idl_fileCur(),"        if(to->%s)\n", to);
                idl_fileOutPrintf(idl_fileCur(),"        {\n");
                idl_fileOutPrintf(idl_fileCur(),"            /* Copy the value of the array into the database string */\n");
                idl_fileOutPrintf(idl_fileCur(),"           strncpy(to->%s, %s, %d);\n", to, from, idl_typeArraySize(typeArray));
                idl_fileOutPrintf(idl_fileCur(),"            to->%s[%d] = '\\0';\n", to, idl_typeArraySize(typeArray));
                idl_fileOutPrintf(idl_fileCur(),"        }\n");
            } else
            {
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
            }
        }
        break;
    case idl_tenum:
        maxLen = idl_typeEnumNoElements(idl_typeEnum(subType));

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    /* TODO: Validate all enum elements here - Xref issue dds#175 */\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
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
            } else {
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
            }
        } else if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(subType))) == idl_tenum) {
            maxLen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(subType))));

            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
    /*
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)%s) >= 0) && (((c_long)%s) < %d)){\n",
                from, from, maxLen);
    */
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    /* TODO: Validate all enum elements here - Xref issue dds#175 */\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
    /*
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
    */
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s, %s, sizeof (*%s));\n", to, from, to);
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
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

    os_strncpy (index, "", sizeof(index));
    for (i = 0; i < indent; i++) {
    snprintf (is, (size_t)sizeof(is), "[i%d]", i);
    strncat (index, is, (size_t)sizeof(is));
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
    idl_typeSpec nextType;
    c_long maxlen;
    const c_char * suffix = (c_char *)userData;

    if (idl_isContiguous(idl_typeSpecDef(typeSpec))) {
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n",__FILE__,__LINE__);
        idl_printIndent (indent);
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
            idl_fileOutPrintf (idl_fileCur(), "    if(length0 > 0)\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    {\n");
            indent += 1;
        }
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s *buf%d;\n",idl_scopedSplTypeName(typeSpec),loop_index-1);
        idl_printIndent (indent);
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
            idl_fileOutPrintf (idl_fileCur(), "    buf%d = reinterpret_cast<%s *>(&((%s)%s[0]));\n",
                loop_index-1, idl_scopedSplTypeName(typeSpec), from, idl_seqIndex (loop_index-1));
        }
        else
        {
            idl_fileOutPrintf (idl_fileCur(), "    buf%d = (%s *)(%s)%s.get_buffer();\n",
                loop_index-1, idl_scopedSplTypeName(typeSpec), from, idl_seqIndex (loop_index-1));
        }

        if(idl_typeSpecType (typeSpec) == idl_tenum){
            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    /* TODO: Validate enum elements here - Xref issue dds#175  */\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s,buf%d,length%d* sizeof(*%s));\n",
                to, loop_index-1, loop_index-1, to);
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s,buf%d,length%d* sizeof(*%s));\n",
                to, loop_index-1, loop_index-1, to);
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        } else {
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    memcpy (%s,buf%d,length%d* sizeof(*%s));\n",
                to, loop_index-1, loop_index-1, to);
        }

        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
            indent -=1;
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
        }
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        return;
    }

    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n",__FILE__,__LINE__);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    unsigned int i%d;\n", loop_index-1);
    idl_printIndent (indent);
    idl_fileOutPrintf (idl_fileCur(), "    for (i%d = 0; (i%d < (unsigned int)length%d) && result; i%d++) {\n",
        loop_index-1,
        loop_index-1,
        loop_index-1,
        loop_index-1);

    switch (idl_typeSpecType (typeSpec)) {
    case idl_tenum:
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)((%s)%s)) >= 0) && (((c_long)((%s)%s)) < %d)){\n",
                from,
                idl_seqIndex(loop_index),
                from,
                idl_seqIndex(loop_index),
                maxlen);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                to,
                loop_index-1,
                idl_scopedSplTypeName(typeSpec),
                from,
                idl_seqIndex(loop_index));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "    %s[i%d] = (%s)(%s)%s;\n",
                to,
                loop_index-1,
                idl_scopedSplTypeName(typeSpec),
                from,
                idl_seqIndex(loop_index));
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        break;
    case idl_tbasic:
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType (idl_typeBasic(typeSpec)) == idl_string) {
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));

            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
            idl_printIndent (indent);
            idl_fileOutPrintf    (idl_fileCur(), "        if((%s)%s){\n",
                    from,
                    idl_seqIndex(loop_index));
            idl_printIndent (indent);

            if(maxlen != 0){
                idl_fileOutPrintf(idl_fileCur(), "            if(((unsigned int)(strlen((%s)%s))) <= %d){\n",
                        from,
                        idl_seqIndex(loop_index),
                        maxlen);

                idl_printIndent (indent+1);
                idl_fileOutPrintf(idl_fileCur(), "            %s[i%d] = c_stringNew(base, (%s)%s);\n",
                        to,
                        loop_index-1,
                        from,
                        idl_seqIndex(loop_index));
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "            ");
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "            result = OS_C_FALSE;\n");
                idl_printIndent (indent + 1);
                idl_fileOutPrintf (idl_fileCur(), "        }\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "            %s[i%d] = c_stringNew(base, (%s)%s);\n",
                        to,
                        loop_index-1,
                        from,
                        idl_seqIndex(loop_index));
            }
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "            ");
            idl_boundsCheckFailNull(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "            result = OS_C_FALSE;\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        }\n");
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = c_stringNew(base, (%s)%s);\n",
                    to,
                    loop_index-1,
                    from,
                    idl_seqIndex(loop_index));
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        } else {
            /* This branch is never executed because the first if statement that is added as a copy improvement
               at the top of this method already covers this condition */
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                    to,
                    loop_index-1,
                    idl_scopedSplTypeName(typeSpec),
                    from,
                    idl_seqIndex(loop_index));
        }
        break;
    case idl_tstruct:
    case idl_tunion:
        varIndex = 0;
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        extern c_bool __%s__copyIn%s(c_base base,\n",
            idl_scopedTypeName(typeSpec),
            suffix);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "            %s *From,\n",
        idl_scopeStack (idl_typeUserScope(idl_typeUser(typeSpec)), "::", idl_typeSpecName(typeSpec)));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "            %s *To);\n\n",
        idl_scopedSplTypeIdent(typeSpec));
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        result = __%s__copyIn%s(base, &(%s)%s, (%s *)&%s[i%d]);\n",
            idl_scopedTypeName(typeSpec),
            suffix,
            from,
            idl_seqIndex(loop_index),
            idl_scopedSplTypeIdent(typeSpec),
            to,
            loop_index-1);
        break;
    case idl_ttypedef:
        idl_printIndent (indent);
        switch (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec)))) {
        case idl_tstruct:
        case idl_tunion:
        case idl_tarray:
            idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
            idl_fileOutPrintf (idl_fileCur(), "            extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
                idl_scopedTypeName (typeSpec),
                suffix,
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                idl_scopedTypeName (typeSpec));
            idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn%s(base, &(%s)%s, (%s *)&%s[i%d]);\n",
                idl_scopedTypeName(typeSpec),
                suffix,
                from,
                idl_seqIndex(loop_index),
                idl_scopedSplTypeName(typeSpec),
                to,
                loop_index-1);
            idl_fileOutPrintf (idl_fileCur(), "        }\n");
            break;
        case idl_tbasic:
                /* QAC EXPECT 3416; No side effect here */
            if (idl_typeBasicType(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec)))) == idl_string) {
                maxlen = idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefActual(idl_typeDef(typeSpec))));

                idl_fileOutPrintf (idl_fileCur(),"\n#ifdef %s\n", BOUNDSCHECK);
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        if((%s)%s){\n",
                        from,
                        idl_seqIndex(loop_index));
                idl_printIndent (indent);

                if(maxlen != 0){
                    idl_fileOutPrintf(idl_fileCur(), "            if(((unsigned int)(strlen((%s)%s))) <= %d){\n",
                            from,
                            idl_seqIndex(loop_index),
                            maxlen);
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf(idl_fileCur(), "            %s[i%d] = c_stringNew(base, (%s)%s);\n",
                            to,
                            loop_index-1,
                            from,
                            idl_seqIndex(loop_index));
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "            ");
                    idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "            result = OS_C_FALSE;\n");
                    idl_printIndent (indent + 1);
                    idl_fileOutPrintf (idl_fileCur(), "        }\n");
                } else {
                    idl_fileOutPrintf (idl_fileCur(), "            %s[i%d] = c_stringNew(base, (%s)%s);\n",
                                to,
                                loop_index-1,
                                from,
                                idl_seqIndex(loop_index));
                }
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "            ");
                idl_boundsCheckFailNull(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "            result = OS_C_FALSE;\n");
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        }\n");

                idl_fileOutPrintf (idl_fileCur(),"#else\n");
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = c_stringNew(base, (%s)%s);\n",
                    to,
                    loop_index-1,
                    from,
                    idl_seqIndex(loop_index));
                idl_fileOutPrintf (idl_fileCur(),"#endif\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                to,
                loop_index-1,
                idl_scopedSplTypeName(typeSpec),
                from,
                idl_seqIndex(loop_index));
            }
            break;
        case idl_tenum:
            maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(typeSpec))));
            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)((%s)%s)) >= 0) && (((c_long)((%s)%s)) < %d)){\n",
                    from,
                    idl_seqIndex(loop_index),
                    from,
                    idl_seqIndex(loop_index),
                    maxlen);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s[i%d] = (%s)(%s)%s;\n",
                    to,
                    loop_index-1,
                    idl_scopedSplTypeName(typeSpec),
                    from,
                    idl_seqIndex(loop_index));
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        ");
            idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    }\n");
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "    %s[i%d] = (%s)(%s)%s;\n",
                    to,
                    loop_index-1,
                    idl_scopedSplTypeName(typeSpec),
                    from,
                    idl_seqIndex(loop_index));
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
            break;
        case idl_tseq:
            nextType = idl_typeSeqType(idl_typeSeq(idl_typeDefActual(idl_typeDef(typeSpec))));
            scopedName = idl_scopedSplTypeIdent(nextType);
            maxlen = idl_typeSeqMaxSize(idl_typeSeq(idl_typeDefActual(idl_typeDef(typeSpec))));

            if (idl_typeSpecType(nextType) == idl_tbasic) {
                if (idl_typeBasicMaxlen(idl_typeBasic(nextType)) > 0) {
                    typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
                } else {
                    typeName = idl_typeSpecName(nextType);
                }
            } else {
                typeName = idl_scopeStack (idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
            }

            snprintf (destin, (size_t)sizeof(destin), "dest%d", loop_index);

            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %sc_type type%d = NULL;\n", *suffix == '\0' ? "static ": "", loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        c_type subtype%d;\n", loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        c_long length%d;\n", loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        %s *dest%d;\n\n", scopedName, loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        if (type%d == NULL) {\n", loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "            subtype%d = c_type(c_metaResolve(c_metaObject(base), \"%s\"));\n", loop_index, typeName);
            idl_printIndent (indent);
            if(maxlen>0){
                idl_fileOutPrintf (idl_fileCur(), "            type%d = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s,%d>\",subtype%d,%d);\n", loop_index, typeName, maxlen, loop_index, maxlen);
            } else {
                idl_fileOutPrintf (idl_fileCur(), "            type%d = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s>\",subtype%d,0);\n", loop_index, typeName, loop_index);
            }
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "            c_free(subtype%d);\n", loop_index);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        }\n");

            idl_printIndent (indent);
            if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
            {

                idl_fileOutPrintf (idl_fileCur(), "        length%d = (c_long)(%s)%s.size();\n", loop_index, from, idl_seqIndex(loop_index));
            }
            else
            {

                idl_fileOutPrintf (idl_fileCur(), "        length%d = (c_long)(%s)%s.length();\n", loop_index, from, idl_seqIndex(loop_index));
            }

            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
            idl_printIndent (indent);

            if(maxlen > 0){
                idl_fileOutPrintf(idl_fileCur(), "        if(length%d > %d){\n", loop_index, maxlen);
                idl_printIndent (indent);
                idl_fileOutPrintf(idl_fileCur(), "            ");
                idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSpec, from);
                idl_printIndent(indent);
                idl_fileOutPrintf(idl_fileCur(), "            result = OS_C_FALSE;");
                idl_printIndent(indent);
                idl_fileOutPrintf(idl_fileCur(), "        } else {");
                idl_printIndent(indent);
                idl_fileOutPrintf (idl_fileCur(), "            dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                    loop_index, scopedName, loop_index, loop_index);
                idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+2, userData);
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "            dest%d[i%d] = (c_sequence)dest%d;\n",
                    loop_index-1, loop_index-1, loop_index);
                idl_printIndent(indent);
                idl_fileOutPrintf(idl_fileCur(), "        }\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                    loop_index, scopedName, loop_index, loop_index);
                idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+1, userData);
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        dest%d[i%d] = (c_sequence)dest%d;\n",
                    loop_index-1, loop_index-1, loop_index);
            }
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                loop_index, scopedName, loop_index, loop_index);
            idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+1, userData);
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        dest%d[i%d] = (c_sequence)dest%d;\n",
                loop_index-1, loop_index-1, loop_index);
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
            if(*suffix != '\0'){
                idl_printIndent (indent);
                idl_fileOutPrintf (idl_fileCur(), "        c_free(type%d);\n", loop_index);
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
        scopedName = idl_scopedSplTypeIdent(nextType);
        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));

        if (idl_typeSpecType(nextType) == idl_tbasic) {
            if (idl_typeBasicMaxlen(idl_typeBasic(nextType)) > 0) {
                typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
            } else {
                typeName = idl_typeSpecName(nextType);
            }
        } else {
            typeName = idl_scopeStack (idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
        }

        snprintf (destin, (size_t)sizeof(destin), "dest%d", loop_index);

        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        %sc_type type%d = NULL;\n", *suffix == '\0' ? "static " : "", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        c_type subtype%d;\n", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        c_long length%d;\n", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        %s *dest%d;\n\n", scopedName, loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        if (type%d == NULL) {\n", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "            subtype%d = c_type(c_metaResolve (c_metaObject(base), \"%s\"));\n", loop_index, typeName);
        idl_printIndent (indent);
        if(maxlen > 0){
            idl_fileOutPrintf (idl_fileCur(), "            type%d = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s,%d>\",subtype%d,%d);\n", loop_index, typeName, maxlen, loop_index, maxlen);
        } else {
            idl_fileOutPrintf (idl_fileCur(), "            type%d = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s>\",subtype%d,0);\n", loop_index, typeName, loop_index);
        }
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "            c_free(subtype%d);\n", loop_index);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_printIndent (indent);
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
          idl_fileOutPrintf (idl_fileCur(), "        length%d = (c_long)(%s)%s.size();\n", loop_index, from, idl_seqIndex(loop_index));
        }
        else
        {
          idl_fileOutPrintf (idl_fileCur(), "        length%d = (c_long)(%s)%s.length();\n", loop_index, from, idl_seqIndex(loop_index));
        }
        idl_printIndent (indent);

        idl_fileOutPrintf (idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n",
                loop_index, scopedName, loop_index, loop_index);
        idl_seqLoopCopy (scope, nextType, from, destin, loop_index+1, indent+2, userData);
        idl_printIndent (indent);
        idl_fileOutPrintf (idl_fileCur(), "        dest%d[i%d] = (c_sequence)dest%d;\n",
                loop_index-1, loop_index-1, loop_index);
        if(*suffix != '\0'){
            idl_printIndent (indent);
            idl_fileOutPrintf (idl_fileCur(), "        c_free(type%d);\n", loop_index);
        }
        break;
    default:
        assert (0);
        break;
    }
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
    idl_typeSeq typeSeq,
    c_long indent,
    void *userData)
{
    c_char destin[32];
    c_long maxlen;
    c_char *typeName;
    c_char *scopedName;
    idl_typeSpec nextType;
    const c_char * suffix = (c_char *)userData;

    nextType = idl_typeSeqType(typeSeq);

    if (idl_typeSpecType(nextType) == idl_tbasic) {
        if (idl_typeBasicMaxlen(idl_typeBasic(nextType)) > 0) {
            typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
        } else {
            typeName = idl_typeSpecName(nextType);
        }
    } else {
        typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
    }
    scopedName = idl_scopedSplTypeName(nextType);
    snprintf(destin, (size_t)sizeof(destin), "dest%d", indent);
    maxlen = idl_typeSeqMaxSize(typeSeq);

    idl_fileOutPrintf(idl_fileCur(), "/* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
    idl_fileOutPrintf(idl_fileCur(), "    %sc_type type%d = NULL;\n", *suffix == '\0' ? "static " : "", indent);
    idl_fileOutPrintf(idl_fileCur(), "    c_type subtype%d = NULL;\n", indent);
    idl_fileOutPrintf(idl_fileCur(), "    c_long length%d;\n", indent);
    idl_fileOutPrintf(idl_fileCur(), "    %s *dest%d;\n\n", idl_scopedSplTypeName(nextType), indent);
    idl_fileOutPrintf(idl_fileCur(), "    if (type%d == NULL) {\n", indent);
    idl_fileOutPrintf(idl_fileCur(), "        subtype%d = c_type(c_metaResolve (c_metaObject(base), \"%s\"));\n", indent, typeName);
    if(maxlen > 0){
        idl_fileOutPrintf(idl_fileCur(), "        type%d = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s,%d>\",subtype%d,%d);\n", indent, typeName, maxlen, indent,maxlen);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "        type%d = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s>\",subtype%d,0);\n", indent, typeName, indent);
    }
    idl_fileOutPrintf(idl_fileCur(), "        c_free(subtype%d);\n",indent);
    idl_fileOutPrintf(idl_fileCur(), "    }\n");
    if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
    {
        idl_fileOutPrintf(idl_fileCur(), "    length%d = (c_long)(*from).size();\n", indent);
    }
    else
    {
        idl_fileOutPrintf(idl_fileCur(), "    length%d = (c_long)(*from).length();\n", indent);
    }

    idl_fileOutPrintf(idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);

    if(maxlen != 0){
        idl_fileOutPrintf(idl_fileCur(), "    if(length%d > %d){\n", indent, maxlen);
        idl_fileOutPrintf(idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)typeSeq, name);
        idl_fileOutPrintf(idl_fileCur(), "        result = OS_C_FALSE;\n");
        idl_fileOutPrintf(idl_fileCur(), "    } else {\n");

        idl_fileOutPrintf(idl_fileCur(), "        dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n", indent, scopedName, indent, indent);
            idl_seqLoopCopy(scope, nextType, "*from", destin, 1, indent+2, userData);
        idl_fileOutPrintf(idl_fileCur(), "        *to = (_%s)dest%d;\n", idl_scopeStack(scope, "_", name), indent);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n", indent, scopedName, indent, indent);
            idl_seqLoopCopy (scope, nextType, "*from", destin, 1, indent+1, userData);
        idl_fileOutPrintf(idl_fileCur(), "    *to = (_%s)dest%d;\n", idl_scopeStack(scope, "_", name), indent);
    }
    idl_fileOutPrintf(idl_fileCur(), "#else\n");
    idl_fileOutPrintf(idl_fileCur(), "    dest%d = (%s *)c_newSequence(c_collectionType(type%d), length%d);\n", indent, scopedName, indent, indent);
        idl_seqLoopCopy (scope, nextType, "*from", destin, 1, indent+1, userData);
    idl_fileOutPrintf(idl_fileCur(), "    *to = (_%s)dest%d;\n", idl_scopeStack(scope, "_", name), indent);
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");
    if(*suffix != '\0'){
        idl_fileOutPrintf(idl_fileCur(), "    c_free(type%d);\n", indent);
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
    const c_char * suffix = (c_char *) userData;

    switch (idl_typeSpecType(idl_typeDefActual(defSpec))) {
    case idl_tstruct:
    case idl_tunion:
        idl_fileOutPrintf (idl_fileCur(), "c_bool\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn%s(\n",
                idl_scopeStack (scope, "_", name),
                suffix);
        idl_fileOutPrintf (idl_fileCur(), "    c_base base,\n");
        idl_fileOutPrintf (idl_fileCur(), "    %s *from,\n",
        idl_scopeStackCxx (scope, "::", name));
        idl_fileOutPrintf (idl_fileCur(), "    _%s *to)\n",
        idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    c_bool result = OS_C_TRUE;\n\n");
        idl_fileOutPrintf (idl_fileCur(), "    extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
            idl_scopedTypeName (idl_typeDefActual(defSpec)),
            suffix,
            idl_corbaCxxTypeFromTypeSpec(idl_typeDefActual(defSpec)),
            idl_scopedTypeName (idl_typeDefActual(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    result = __%s__copyIn%s(base, (%s *)from, (%s *)to);\n",
            idl_scopedTypeName(idl_typeDefActual(defSpec)),
            suffix,
            idl_corbaCxxTypeFromTypeSpec(idl_typeDefActual(defSpec)),
            idl_scopedSplTypeName(idl_typeDefActual(defSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tarray:
        idl_fileOutPrintf (idl_fileCur(), "c_bool\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn%s(\n",
                idl_scopeStack (scope, "_", name),
                suffix);
        idl_fileOutPrintf (idl_fileCur(), "    c_base base,\n");
        idl_fileOutPrintf (idl_fileCur(), "    %s *from,\n",
        idl_scopeStackCxx (scope, "::", name));
        idl_fileOutPrintf (idl_fileCur(), "    _%s *to)\n",
        idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    c_bool result = OS_C_TRUE;\n");
        idl_fileOutPrintf (idl_fileCur(), "    (void) base;\n\n");
        idl_arrayElements (scope, idl_typeArray(idl_typeDefActual(defSpec)), "*from", "to", 0, OS_FALSE, OS_FALSE, userData);
        idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
        idl_fileOutPrintf (idl_fileCur(), "}\n");
        idl_fileOutPrintf (idl_fileCur(), "\n");
        break;
    case idl_tseq:
        idl_fileOutPrintf (idl_fileCur(), "c_bool\n");
        idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn%s(\n",
                idl_scopeStack (scope, "_", name),
                suffix);
        idl_fileOutPrintf (idl_fileCur(), "    c_base base,\n");
        idl_fileOutPrintf (idl_fileCur(), "    %s *from,\n",
        idl_scopeStackCxx (scope, "::", name));
        idl_fileOutPrintf (idl_fileCur(), "    _%s *to)\n",
        idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "{\n");
        idl_fileOutPrintf (idl_fileCur(), "    c_bool result = OS_C_TRUE;\n");
        idl_fileOutPrintf (idl_fileCur(), "    (void) base;\n\n");
        idl_seqElements (scope, name, idl_typeSeq(idl_typeDefActual(defSpec)), 0, userData);
        idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
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
    c_long maxlen;
    const char * suffix = (char *) userData;

    idl_fileOutPrintf (idl_fileCur(), "c_bool\n");
    idl_fileOutPrintf (idl_fileCur(), "__%s__copyIn%s(\n",
            idl_scopeStack (scope, "_", name),
            suffix);
    idl_fileOutPrintf (idl_fileCur(), "    c_base base,\n");
    idl_fileOutPrintf (idl_fileCur(), "    %s *from,\n",
    idl_scopeStackCxx (scope, "::", name));
    idl_fileOutPrintf (idl_fileCur(), "    struct _%s *to)\n",
    idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    c_bool result = OS_C_TRUE;\n");
    idl_fileOutPrintf (idl_fileCur(), "    (void) base;\n\n");
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
        /* String is not allowed here */
        idl_fileOutPrintf (idl_fileCur(), "    to->_d = (%s)from->_d();\n",
        idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec)));
    } else if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tenum) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeUnionSwitchKind(unionSpec)));
        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_fileOutPrintf (idl_fileCur(), "    if((((c_long)(from->_d())) >= 0) && (((c_long)(from->_d())) < %d)){\n",
                maxlen);
        idl_fileOutPrintf (idl_fileCur(), "        to->_d = (%s)from->_d();\n",
                idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)unionSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_fileOutPrintf (idl_fileCur(), "    to->_d = (%s)from->_d();\n",
            idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec)));
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
    } else if ((idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) &&
            (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))) == idl_tbasic)) {
        /* String is not allowed here */
        idl_fileOutPrintf (idl_fileCur(), "    to->_d = (%s)from->_d();\n",
        idl_scopedSplTypeName (idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))));
    } else if ((idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_ttypedef) &&
            (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))) == idl_tenum)) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(idl_typeDefActual(idl_typeDef(idl_typeUnionSwitchKind(unionSpec)))));

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_fileOutPrintf (idl_fileCur(), "    if( (((c_long)(from->_d())) >= 0) && (((c_long)(from->_d())) < %d)){\n",
                maxlen);
        idl_fileOutPrintf (idl_fileCur(), "        to->_d = (%s)from->_d();\n",
                idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec)));
        idl_fileOutPrintf (idl_fileCur(), "    } else {\n");
        idl_fileOutPrintf (idl_fileCur(), "        ");
        idl_boundsCheckFail(ELEMENT, scope, (idl_typeSpec)unionSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "        result = OS_C_FALSE;\n");
        idl_fileOutPrintf (idl_fileCur(), "    }\n");
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_fileOutPrintf (idl_fileCur(), "    to->_d = (%s)from->_d();\n",
                idl_scopedSplTypeName (idl_typeUnionSwitchKind(unionSpec)));
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    idl_fileOutPrintf (idl_fileCur(), "    switch (to->_d) {\n");
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
    idl_fileOutPrintf (idl_fileCur(), "    }\n");
    idl_fileOutPrintf (idl_fileCur(), "    return result;\n");
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
    c_long maxlen;
    c_char *typeName;
    c_char *scopedName;
    c_char* cid;
    const c_char * suffix = (c_char *)userData;

    cid = idl_cxxId(name);

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));

            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
            idl_fileOutPrintf (idl_fileCur(), "        if(from->%s()){\n",
                    cid);

            if(maxlen > 0){
                idl_fileOutPrintf(idl_fileCur(),  "            if(((unsigned int)(strlen(from->%s()))) <= %d){\n",
                    cid,
                    maxlen);
                idl_fileOutPrintf (idl_fileCur(), "                to->_u.%s = c_stringNew(base, from->%s());\n",
                    cid,
                    cid);
                idl_fileOutPrintf (idl_fileCur(), "            } else {\n");
                idl_fileOutPrintf (idl_fileCur(), "                ");
                idl_boundsCheckFail(CASE, scope, (idl_typeSpec)typeSpec, name);
                idl_fileOutPrintf (idl_fileCur(), "                result = OS_C_FALSE;\n");
                idl_fileOutPrintf (idl_fileCur(), "            }\n");
            } else {
                idl_fileOutPrintf (idl_fileCur(), "        to->_u.%s = c_stringNew(base, from->%s());\n",
                        cid,
                        cid);
            }
            idl_fileOutPrintf (idl_fileCur(),     "        } else {\n");
            idl_fileOutPrintf (idl_fileCur(),     "            ");
            idl_boundsCheckFailNull(CASE, scope, (idl_typeSpec)typeSpec, name);
            idl_fileOutPrintf (idl_fileCur(),     "            result = OS_C_FALSE;\n");
            idl_fileOutPrintf (idl_fileCur(),     "        }\n");
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_fileOutPrintf (idl_fileCur(), "    to->_u.%s = c_stringNew(base, from->%s());\n",
                cid,
                cid);
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        } else {
            idl_basicCaseType (scope, name, idl_typeBasic(typeSpec), "from->", "to->_u.");
        }
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        if (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tarray && idl_getIsISOCpp() && idl_getIsISOCppTypes()) {
                idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
                idl_fileOutPrintf (idl_fileCur(), "            extern c_bool __%s__copyIn(c_base, %s *, _%s *);\n",
                idl_scopedTypeName (typeSpec),
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                idl_scopedTypeName (typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "            %s &x = from->%s();\n",
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                cid);
                idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn(base, &x, &to->_u.%s);\n",
                idl_scopedTypeName (typeSpec),
                cid);
                idl_fileOutPrintf (idl_fileCur(), "        }\n");
                idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        }
        /* QAC EXPECT 3416; No side effect here */
        else if (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tarray) {
                idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
            idl_fileOutPrintf (idl_fileCur(), "            extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
                idl_scopedTypeName (typeSpec),
                suffix,
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                idl_scopedTypeName (typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "            %s *x = (%s *)from->%s();\n",
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                cid);
            idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn%s(base, x, &to->_u.%s);\n",
                idl_scopedTypeName (typeSpec),
                suffix,
                cid);
                idl_fileOutPrintf (idl_fileCur(), "        }\n");
                idl_fileOutPrintf (idl_fileCur(), "        break;\n");
                /* QAC EXPECT 3416; No side effect here */
        } else if (idl_typeSpecType(idl_typeDefRefered(idl_typeDef(typeSpec))) == idl_tseq) {
                idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
                idl_fileOutPrintf (idl_fileCur(), "            extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
                    idl_scopedTypeName (typeSpec),
                    suffix,
                    idl_corbaCxxTypeFromTypeSpec(typeSpec),
                    idl_scopedTypeName (typeSpec));
                idl_fileOutPrintf (idl_fileCur(), "            %s &x = from->%s();\n",
                idl_corbaCxxTypeFromTypeSpec(typeSpec),
                cid);
                idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn%s(base, &x, &to->_u.%s);\n",
                    idl_scopedTypeName (typeSpec),
                    suffix,
                    cid);
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
        }
            /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        maxlen = idl_typeEnumNoElements(idl_typeEnum(typeSpec));

        idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
        idl_fileOutPrintf (idl_fileCur(), "        if((((c_long)(from->%s())) >= 0) && (((c_long)(from->%s())) < %d)){\n",
                cid,
                cid,
                maxlen);
        idl_fileOutPrintf (idl_fileCur(), "            to->_u.%s = (%s)from->%s();\n",
                cid,
                idl_scopedSplTypeName (typeSpec),
                cid);
        idl_fileOutPrintf (idl_fileCur(), "        } else {\n");
        idl_fileOutPrintf (idl_fileCur(), "            ");
        idl_boundsCheckFail(CASE, scope, (idl_typeSpec)typeSpec, name);
        idl_fileOutPrintf (idl_fileCur(), "            result = OS_C_FALSE;\n");
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(),"#else\n");
        idl_fileOutPrintf (idl_fileCur(), "        to->_u.%s = (%s)from->%s();\n",
            cid,
            idl_scopedSplTypeName (typeSpec),
            cid);
        idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
        idl_fileOutPrintf (idl_fileCur(), "            extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
            idl_scopedTypeName (typeSpec),
            suffix,
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
            idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "            %s &x = from->%s();\n",
        idl_corbaCxxTypeFromTypeSpec(typeSpec),
        cid);
        idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn%s(base, &x, &to->_u.%s);\n",
            idl_scopedTypeName (typeSpec),
            suffix,
            cid);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tunion) {
        idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
        idl_fileOutPrintf (idl_fileCur(), "            extern c_bool __%s__copyIn%s(c_base, %s *, _%s *);\n",
            idl_scopedTypeName (typeSpec),
            suffix,
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
            idl_scopedTypeName (typeSpec));
        idl_fileOutPrintf (idl_fileCur(), "            %s &x = from->%s();\n",
        idl_corbaCxxTypeFromTypeSpec(typeSpec),
        cid);
        idl_fileOutPrintf (idl_fileCur(), "            result = __%s__copyIn%s(base, &x, &to->_u.%s);\n",
            idl_scopedTypeName (typeSpec),
            suffix,
            cid);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        c_char source[256];

        snprintf(source, (size_t)sizeof(source), "from->%s()", name);
        idl_fileOutPrintf (idl_fileCur(), "        if(result){\n");
        idl_fileOutPrintf (idl_fileCur(), "            typedef %s _DestType",
                idl_scopedSplTypeIdent (idl_typeArrayActual(idl_typeArray(typeSpec))));
        idl_arrayDimensions (idl_typeArray(typeSpec), OS_FALSE);
        idl_fileOutPrintf (idl_fileCur(), ";\n");
        idl_fileOutPrintf (idl_fileCur(), "            _DestType *dest = &to->_u.%s;\n", cid);
        idl_arrayElements (scope, idl_typeArray(typeSpec), source, "dest", 2, OS_FALSE, OS_FALSE, userData);
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        c_char type_name[256];
        idl_typeSpec nextType;

        nextType = idl_typeSeqType(idl_typeSeq(typeSpec));
        scopedName = idl_scopedSplTypeName(nextType);

        if (idl_typeSpecType(nextType) == idl_tbasic) {
            if (idl_typeBasicMaxlen(idl_typeBasic(nextType)) > 0) {
                typeName = idl_scopeStack(scope, "::", idl_typeSpecName(nextType));
            } else {
                typeName = idl_typeSpecName(nextType);
            }
        } else {
            typeName = idl_scopeStack(idl_typeUserScope(idl_typeUser(nextType)), "::", idl_typeSpecName(nextType));
        }

        maxlen = idl_typeSeqMaxSize(idl_typeSeq(typeSpec));
        snprintf (type_name, (size_t)sizeof(type_name), "_%s_seq", name);

        idl_fileOutPrintf (idl_fileCur(), "/* Code generated by %s at line %d */\n\n",__FILE__,__LINE__);
        idl_fileOutPrintf (idl_fileCur(), "    if(result){\n");
        idl_fileOutPrintf (idl_fileCur(), "        %sc_type type0 = NULL;\n", *suffix == '\0' ? "static " : "");
        idl_fileOutPrintf (idl_fileCur(), "        c_type subtype0 = NULL;\n");
        idl_fileOutPrintf (idl_fileCur(), "        c_long length0;\n");
        idl_fileOutPrintf (idl_fileCur(), "        %s *dest0;\n", scopedName);
        idl_fileOutPrintf (idl_fileCur(), "        %s *src = &(from->%s());\n\n", idl_scopeStackCxx (scope, "::", type_name), cid);
        idl_fileOutPrintf (idl_fileCur(), "        if (type0 == NULL) {\n");
        idl_fileOutPrintf (idl_fileCur(), "            subtype0 = c_type(c_metaResolve (c_metaObject(base), \"%s\"));\n", typeName);
        if(maxlen > 0){
            idl_fileOutPrintf (idl_fileCur(), "            type0 = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s,%d>\",subtype0,%d);\n",typeName,maxlen,maxlen);
        } else {
            idl_fileOutPrintf (idl_fileCur(), "            type0 = c_metaSequenceTypeNew(c_metaObject(base),\"C_SEQUENCE<%s>\",subtype0,0);\n",typeName);
        }
        idl_fileOutPrintf (idl_fileCur(), "            c_free(subtype0);\n");
        idl_fileOutPrintf (idl_fileCur(), "        }\n");
        if (idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
          idl_fileOutPrintf (idl_fileCur(), "        length0 = (c_long)(*src).size();\n");
        }
        else
        {
          idl_fileOutPrintf (idl_fileCur(), "        length0 = (c_long)(*src).length();\n");
        }

        if(maxlen != 0){
            idl_fileOutPrintf (idl_fileCur(),"#ifdef %s\n", BOUNDSCHECK);
            idl_fileOutPrintf(idl_fileCur(), "        if(length0 > %d){\n", maxlen);
            idl_fileOutPrintf(idl_fileCur(), "            ");
            idl_boundsCheckFail(CASE, scope, (idl_typeSpec)typeSpec, name);
            idl_fileOutPrintf(idl_fileCur(), "            result = OS_C_FALSE;\n");
            idl_fileOutPrintf(idl_fileCur(), "        } else {\n");

            idl_fileOutPrintf (idl_fileCur(), "            dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
                idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 3, userData);
            idl_fileOutPrintf (idl_fileCur(), "            to->_u.%s = (c_sequence)dest0;\n", cid);

            idl_fileOutPrintf(idl_fileCur(), "        }\n");
            idl_fileOutPrintf (idl_fileCur(),"#else\n");
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
                idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->_u.%s = (c_sequence)dest0;\n", cid);
            idl_fileOutPrintf (idl_fileCur(),"#endif\n");
        } else {
            idl_fileOutPrintf (idl_fileCur(), "        dest0 = (%s *)c_newSequence(c_collectionType(type0), length0);\n", scopedName);
                idl_seqLoopCopy (scope, nextType, "*src", "dest0", 1, 2, userData);
            idl_fileOutPrintf (idl_fileCur(), "        to->_u.%s = (c_sequence)dest0;\n", cid);
        }
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        idl_fileOutPrintf(idl_fileCur(), "    break;\n");
        if(*suffix != '\0'){
            idl_fileOutPrintf(idl_fileCur(), "    c_free(type0);\n");
        }
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC reports */
    }
    /* QAC EXPECT 5101, 5103: Complexity is limited, by independent cases, per case the number of lines is lower  */
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
 * Specifies the callback table for the CORBA C++ CopyIn generation functions.
 */
static struct idl_program
idl_genCorbaCxxCopyin = {
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
idl_genCorbaCxxCopyinProgram (
    void)
{
    return &idl_genCorbaCxxCopyin;
}
