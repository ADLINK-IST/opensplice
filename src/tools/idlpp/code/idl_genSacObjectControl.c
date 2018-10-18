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
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_keyDef.h"
#include "idl_genSacObjectControl.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"

#include <ctype.h>
#include "c_typebase.h"

static c_bool struct_hasRef = FALSE;
static c_bool union_hasRef = FALSE;

static c_long loopIndent;
static c_long varIndex;

static void idl_arrayElements (idl_typeArray typeArray, const char *identifier, c_long indent, c_bool inlineBoundedStrings);
static void
idl_arrayLoopRemoveBody(
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent,
    c_bool inlineBoundedStrings);

static void
idl_arrayLoopRemove(
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent,
    c_bool inlineBoundedStrings);

static c_char *
idl_valueFromLabelVal(
    idl_labelVal labelVal)
{
    static c_char labelName [1000];

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
        snprintf (labelName, sizeof(labelName), "_%s", idl_labelEnumVal(idl_labelEnum(labelVal)));
    } else {
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
	    case V_CHAR:
            snprintf(labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
		break;
	    case V_SHORT:
            snprintf(labelName, sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
		break;
	    case V_USHORT:
            snprintf(labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
		break;
	    case V_LONG:
            snprintf(labelName, sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
		break;
	    case V_ULONG:
            snprintf(labelName, sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
		break;
	    case V_LONGLONG:
            snprintf(labelName, sizeof(labelName), "%"PA_PRId64, idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
		break;
	    case V_ULONGLONG:
            snprintf(labelName, sizeof(labelName), "%"PA_PRIu64, idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
		break;
	    case V_BOOLEAN:
            /* QAC EXPECT 3416; No side effect here */
            if ((int)idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
                snprintf(labelName, sizeof(labelName), "TRUE");
            } else {
                snprintf(labelName, sizeof(labelName), "FALSE");
            }
		break;
	    default:
            printf ("idl_valueFromLabelVal: Unexpected label type\n");
		break;
        }
    }
    return labelName;
    /* QAC EXPECT 5101; Because of the switch statement the real complexity is rather low, no need to change */
}

static c_bool
idl_inlineBoundedString(
     idl_typeSpec typeSpec,
     c_bool inlineBoundedString)
{
    c_ulong maxlen = 0;

    if (inlineBoundedString) {
        if (idl_typeSpecType(typeSpec) == idl_tbasic) {
            if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
                maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeSpec));
            }
        }
    }
    return maxlen > 0;
}

/** @brief callback function called on opening the IDL input file.
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
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

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
 * This function generates allocation/deallocation code for structures. If
 * the structure contains any reference type opbject (string or
 * sequence), that object will also be deallocated.
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
    idl_action action;
    c_char *scopedName;

    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(userData);

    scopedName = idl_scopeStack(scope, "_", name);

    if (idl_definitionExists ("objManagImpl", scopedName)) {
        os_free(scopedName);
        return idl_abort;
    }

    idl_definitionAdd("objManagImpl", scopedName);
    idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
            scopedName, scopedName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    if (idl_typeSpecHasRef(idl_typeSpec(structSpec))) {
        idl_fileOutPrintf(idl_fileCur(), "    DDS_ReturnCode_t %s__free (void *object);\n\n",
                scopedName);
        idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_alloc(sizeof(%s), %s__free);\n",
                scopedName, scopedName, scopedName);
        struct_hasRef = TRUE;
        action = idl_explore;
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_alloc(sizeof(%s), NULL);\n",
                scopedName, scopedName);
        struct_hasRef = FALSE;
        action = idl_abort;
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    if (idl_typeSpecHasRef(idl_typeSpec(structSpec))) {
        idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *object);\n",
                scopedName);
        /* Deallocation routine are required */
        idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *object)\n",
                scopedName);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    %s *o = (%s *)object;\n\n",
                scopedName, scopedName);
        idl_fileOutPrintf(idl_fileCur(), "    (void) o;\n\n");
    }
    /* return action to indicate that the rest of the structure needs to be processed or not */
    os_free(scopedName);

    return action;
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
        }
   @endverbatim
 *
 * @param name Name of the structure (not used)
 */
static void
idl_structureClose(
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    if (struct_hasRef) {
        idl_fileOutPrintf(idl_fileCur(), "    return DDS_RETCODE_OK;\n");
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
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
    c_bool inlineBoundedStrings = ((SACObjectControlUserData *)userData)->inlineBoundedStrings;
    c_char *scopedName;

    if (idl_typeSpecHasRef(typeSpec)) {
        if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
            (idl_typeSpecType(typeSpec) == idl_tunion)) {

            /* case 6148 */
            scopedName = idl_scopedTypeName(typeSpec);
            idl_fileOutPrintf(idl_fileCur(), "    {\n");
            idl_fileOutPrintf(idl_fileCur(), "        extern DDS_ReturnCode_t %s__free(void *object);\n",
                    scopedName);
            os_free(scopedName);

            scopedName = idl_scopedSacTypeIdent((typeSpec));
            idl_fileOutPrintf(idl_fileCur(), "        %s__free (&o->%s);\n",
                    scopedName, name);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
            os_free(scopedName);

        } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
            idl_structureMemberOpenClose(scope, name, idl_typeDefRefered(idl_typeDef(typeSpec)), userData);
        } else if (idl_typeSpecType(typeSpec) == idl_tbasic &&
                   idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            if (!idl_inlineBoundedString(typeSpec, inlineBoundedStrings)) {
                idl_fileOutPrintf(idl_fileCur(), "    DDS_string_clean (&o->%s);\n",
                        name);
            }
        } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
            char identifier[256];

            snprintf(identifier, sizeof(identifier), "o->%s", name);
            idl_arrayElements(idl_typeArray(typeSpec), identifier, 0, inlineBoundedStrings);
        } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
            idl_fileOutPrintf(idl_fileCur(), "    DDS_sequence_clean ((_DDS_sequence)&o->%s);\n",
                    name);
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
 * This function generates deallocation code for unions. If
 * the unions contains any reference type opbject (string or
 * sequence), that object will also be deallocated.
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
    idl_action action;
    c_char *scopedName;

    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(userData);

    scopedName = idl_scopeStack (scope, "_", name);

    if (idl_definitionExists("objManagImpl", scopedName)) {
        os_free(scopedName);
        return idl_abort;
    }
    idl_definitionAdd("objManagImpl", scopedName);
    idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
            scopedName, scopedName);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    if (idl_typeSpecHasRef(idl_typeSpec(unionSpec))) {
        idl_fileOutPrintf(idl_fileCur(), "    DDS_ReturnCode_t %s__free (void *object);\n\n",
                scopedName);
        idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_alloc(sizeof(%s), %s__free);\n",
                scopedName, scopedName, scopedName);
        union_hasRef = TRUE;
        action = idl_explore;
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_alloc(sizeof(%s), NULL);\n",
                scopedName, scopedName);
        union_hasRef = FALSE;
        action = idl_abort;
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    if (idl_typeSpecHasRef(idl_typeSpec(unionSpec))) {
        /* Deallocation routine are required */
        idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *object);\n",
                scopedName);
        idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *object)\n",
                scopedName);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    %s *o = (%s *)object;\n\n",
                scopedName, scopedName);
        idl_fileOutPrintf(idl_fileCur(), "    switch (o->_d) {\n");
    }
    os_free(scopedName);
    /* return action to indicate that the rest of the union needs to be processed or not */
    return action;
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
 * @param name Name of the union
 */
static void
idl_unionClose(
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    if (union_hasRef) {
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        idl_fileOutPrintf(idl_fileCur(), "    return DDS_RETCODE_OK;\n");
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    }
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
    c_bool inlineBoundedStrings = ((SACObjectControlUserData *)userData)->inlineBoundedStrings;

    if (idl_typeSpecHasRef(typeSpec)) {
        if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
            (idl_typeSpecType(typeSpec) == idl_tunion)) {
            c_char *scopedTypeIdent = idl_scopedSacTypeIdent((typeSpec));

            idl_fileOutPrintf(idl_fileCur(), "        %s__free (&o->_u.%s);\n",
                    scopedTypeIdent, name);
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
            os_free(scopedTypeIdent);
        } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
            idl_unionCaseOpenClose(scope, name, idl_typeDefRefered(idl_typeDef(typeSpec)), userData);
        } else if (idl_typeSpecType(typeSpec) == idl_tbasic) {
            if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
                if (!idl_inlineBoundedString(typeSpec, inlineBoundedStrings)) {
                    idl_fileOutPrintf(idl_fileCur(), "        DDS_string_clean (&o->_u.%s);\n",
                            name);
                }
            }
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
            char identifier[256];

            snprintf(identifier, sizeof(identifier), "o->_u.%s",name);
            idl_arrayElements(idl_typeArray(typeSpec), identifier, 1, inlineBoundedStrings);
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
            idl_fileOutPrintf(idl_fileCur(), "        DDS_sequence_clean ((_DDS_sequence)&o->_u.%s);\n",
                    name);
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else {
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        }
    } else {
        idl_fileOutPrintf(idl_fileCur(), "        break;\n");
    }
}

/** @brief callback function called on definition of a union case label in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1;
   =>       ..         ..
   =>       case label1.n;
                <union-case-1>;
   =>       case label2.1;
   =>       ..         ..
   =>       case label2.n;
                ...        ...
   =>       case labeln.1;
   =>       ..         ..
   =>       case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * The generated code creates a literal meta object for the labels
 * indexed by label_index. After that the value of the label is set.
 *
 * @param scope Current scope (the union the label is defined in)
 * @param labelVal Specifies the value of the label
 */
static void
idl_unionLabelOpenClose (
    idl_scope scope,
    idl_labelVal labelVal,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(labelVal) == idl_ldefault) {
        idl_fileOutPrintf(idl_fileCur(), "    default:\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    case %s:\n", idl_valueFromLabelVal(labelVal));
    }
}

static void
idl_arrayLoopVariables(
    idl_typeArray typeArray,
    c_long indent)
{
    idl_typeSpec idlSubtype;
    loopIndent++;
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    int i%d;\n", loopIndent);
    /* QAC EXPECT 3416; No side effect here */
    idlSubtype = idl_typeDefResolveFully(idl_typeArrayType(typeArray));

    if (idl_typeSpecType(idlSubtype) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopVariables(idl_typeArray(idlSubtype), indent);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    loopIndent--;
}


static void
idl_arrayLoopRemoveOpen(
    idl_typeArray typeArray,
    c_long indent)
{
    idl_typeSpec idlSubtype;

    loopIndent++;
    idlSubtype = idl_typeDefResolveFully(idl_typeArrayType(typeArray));
    idl_printIndent(loopIndent + indent);
    idl_fileOutPrintf(idl_fileCur(), "for (i%d = 0; i%d < %d; i%d++) {\n",
            loopIndent, loopIndent, idl_typeArraySize(typeArray), loopIndent);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idlSubtype) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopRemoveOpen (idl_typeArray(idlSubtype), indent);
    }
}

static void
idl_arrayLoopRemoveIndex (
    idl_typeArray typeArray)
{
    idl_typeSpec idlSubtype;

    varIndex++;
    idlSubtype = idl_typeDefResolveFully(idl_typeArrayType(typeArray));
    idl_fileOutPrintf(idl_fileCur(), "[i%d]", varIndex);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idlSubtype) == idl_tarray) {
	/* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopRemoveIndex(idl_typeArray(idlSubtype));
    }
    varIndex--;
}

static void
idl_typedefRemove(
    idl_typeArray typeArray,
    idl_typeDef typeDef,
    const char *identifier,
    c_long indent,
    c_bool inlineBoundedStrings)
{
    idl_typeSpec typeSpec = idl_typeDefRefered(typeDef);
    c_char *scopedTypeIdent;

    switch (idl_typeSpecType(typeSpec)) {
    case idl_ttypedef:
        idl_typedefRemove(typeArray, idl_typeDef(typeSpec), identifier, indent, inlineBoundedStrings);
        break;
    case idl_tstruct:
    case idl_tunion:

        /* case 6148 */
        scopedTypeIdent = idl_scopedSacTypeIdent(typeSpec);
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    extern DDS_ReturnCode_t %s__free(void *object);\n",
                scopedTypeIdent);

        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    %s__free (&%s",
                scopedTypeIdent, identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        os_free(scopedTypeIdent);
        break;
    case idl_tbasic:
        if (!idl_inlineBoundedString(typeSpec, inlineBoundedStrings)) {
            idl_printIndent(indent);
            idl_fileOutPrintf(idl_fileCur(), "    DDS_string_clean (&%s",
                    identifier);
            idl_arrayLoopRemoveIndex(typeArray);
            idl_fileOutPrintf(idl_fileCur(), ");\n");
        }
        break;
    case idl_tseq:
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    DDS_sequence_clean ((_DDS_sequence)&%s",
                identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ");\n");
        break;
    case idl_tarray:
        idl_arrayLoopRemoveBody(idl_typeArray(typeSpec), identifier, indent, inlineBoundedStrings);
        break;
    default:
        /* QAC EXPECT 3416; No side effect here */
        assert (0);
        break;
    }
}

void
idl_arrayLoopRemoveBody(
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent,
    c_bool inlineBoundedStrings)
{
    idl_typeSpec typeSpec = idl_typeArrayActual(typeArray);
    c_char *scopedTypeIdent;

    loopIndent++;
    varIndex = 0;
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tstruct:
    case idl_tunion:

        /* case 6148 */
        scopedTypeIdent = idl_scopedSacTypeIdent(typeSpec);
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    extern DDS_ReturnCode_t %s__free(void *object);\n",
                scopedTypeIdent);

        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    %s__free (&%s",
                scopedTypeIdent, identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        os_free(scopedTypeIdent);
        break;
    case idl_ttypedef:
        idl_typedefRemove(typeArray, idl_typeDef(typeSpec), identifier, indent, inlineBoundedStrings);
        break;
    case idl_tbasic:
        if (!idl_inlineBoundedString(typeSpec, inlineBoundedStrings)) {
            idl_printIndent(indent);
            idl_fileOutPrintf(idl_fileCur(), "    DDS_string_clean (&%s",
                    identifier);
            idl_arrayLoopRemoveIndex(typeArray);
            idl_fileOutPrintf(idl_fileCur(), ");\n");
        }
        break;
    case idl_tseq:
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    DDS_sequence_clean ((_DDS_sequence)&%s",
                identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
        break;
    default:
        /* QAC EXPECT 3416; No side effect here */
        assert (0);
        break;
    }
    loopIndent--;
    /* QAC EXPECT 5101, 5103: Complexity is limited, by independent cases, per case the number of lines is lower  */
}

static void
idl_arrayLoopRemoveClose(
    idl_typeArray typeArray,
    c_long indent)
{
    idl_typeSpec idlSubtype;

    idlSubtype = idl_typeDefResolveFully(idl_typeArrayType(typeArray));
    idl_printIndent(loopIndent + indent);
    loopIndent--;
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idlSubtype) == idl_tarray) {
        /* QAC EXPECT 3670; Recursive calls is a good practice, the recursion depth is limited here */
        idl_arrayLoopRemoveClose(idl_typeArray(idlSubtype), indent);
    }
}

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

void
idl_arrayLoopRemove(
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent,
    c_bool inlineBoundedStrings)
{
    loopIndent = 0;
    idl_arrayLoopVariables(typeArray, indent);
    idl_arrayLoopRemoveOpen(typeArray, indent);
    idl_arrayLoopRemoveBody(typeArray, identifier, indent + idl_indexSize(typeArray), inlineBoundedStrings);
    idl_arrayLoopRemoveClose(typeArray, indent);
}

static void
idl_arrayElements (
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent,
    c_bool inlineBoundedStrings)
{
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    switch (idl_typeSpecType(idl_typeArrayActual(typeArray))) {
    case idl_tbasic:
        /* This is only called for string type */
        if (idl_typeBasicType(idl_typeBasic(idl_typeArrayActual(typeArray))) == idl_string) {
            if (!idl_inlineBoundedString(idl_typeArrayActual(typeArray), inlineBoundedStrings)) {
                idl_arrayLoopRemove (typeArray, identifier, indent+1, inlineBoundedStrings);
            }
        }
	break;
    case idl_tstruct:
    case idl_tunion:
    case idl_ttypedef:
    case idl_tseq:
        idl_arrayLoopRemove(typeArray, identifier, indent+1, inlineBoundedStrings);
    break;
    default:
        printf ("idl_arrayElements: Unexpected type %d\n",
	    idl_typeSpecType(idl_typeArrayActual(typeArray)));
    break;
    }
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    }\n");
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
    c_bool inlineBoundedStrings = ((SACObjectControlUserData *)userData)->inlineBoundedStrings;
    c_char *scopedName = idl_scopeStack(scope, "_", name);
    c_char *scopedTypeIdent;

    if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct) ||
        (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion)) {
        /* Generate allocation routine for the struct or union */
        scopedTypeIdent = idl_scopedSacTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec)));
        idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
                scopedName, scopedName);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    return (%s *)%s__alloc ();\n",
                scopedName, scopedTypeIdent);
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        /* Generate deallocation routine for the struct or union */
        if (idl_typeSpecHasRef(idl_typeDefRefered(defSpec))) {
            idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *object);\n",
                    scopedName);
            idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *object)\n",
                    scopedName);
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_fileOutPrintf(idl_fileCur(), "    %s__free (object);\n",
                    scopedTypeIdent);
            idl_fileOutPrintf(idl_fileCur(), "    return DDS_RETCODE_OK;\n");
            idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        }
        os_free(scopedTypeIdent);
    } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_ttypedef) {
        idl_typedefOpenClose(scope, name, idl_typeDef(idl_typeDefRefered(defSpec)), userData);
    } else {
        if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tbasic &&
            idl_typeBasicType(idl_typeBasic(idl_typeDefRefered(defSpec))) == idl_string) {
            /* Generate allocation routine for the string */
            if (idl_inlineBoundedString(idl_typeDefRefered(defSpec), inlineBoundedStrings)) {
                idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc ()\n",
                        scopedName, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    return (%s *) DDS_alloc(sizeof(%s), NULL);\n",
                        scopedName, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
            } else {
                idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (DDS_unsigned_long len)\n",
                        scopedName, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_string_alloc (len);\n",
                        scopedName);
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
            }
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tarray) {
            /* Generate allocation routine for the array */
            idl_fileOutPrintf(idl_fileCur(), "%s_slice *%s__alloc (void)\n",
                    scopedName, scopedName);
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            if (idl_typeSpecHasRef(idl_typeDefRefered(defSpec))) {
                idl_fileOutPrintf(idl_fileCur(), "    DDS_ReturnCode_t %s__free (void *array);\n",
                        scopedName);
                idl_fileOutPrintf(idl_fileCur(), "    return (%s_slice *)DDS_alloc(sizeof(%s), %s__free);\n",
                        scopedName, scopedName, scopedName);
            } else {
                idl_fileOutPrintf(idl_fileCur(), "    return (%s_slice *)DDS_alloc(sizeof(%s), NULL);\n",
                        scopedName, scopedName);
            }
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            if (idl_typeSpecHasRef(idl_typeDefRefered(defSpec))) {
                /* Deallocation routine for the array elements is required */
                idl_fileOutPrintf(idl_fileCur(), "\nDDS_ReturnCode_t %s__free (void *array);\n",
                        scopedName);
                idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s__free (void *array)\n",
                        scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    %s *a = (%s *)array;\n",
                        scopedName, scopedName);
                idl_arrayElements(idl_typeArray(idl_typeDefRefered(defSpec)), "(*a)", 0, inlineBoundedStrings);
                idl_fileOutPrintf(idl_fileCur(), "    return DDS_RETCODE_OK;\n");
                idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            idl_fileOutPrintf (idl_fileCur(), "\n");
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
            char *sequenceElementName = idl_sequenceElementIdent(idl_typeDefRefered(defSpec));
            char *sequenceName;
            idl_typeSeq seqType = idl_typeSeq(idl_typeDefRefered(defSpec));
            idl_typeSpec subType = idl_typeSeqType(seqType);

            if (idl_typeSpecType(subType) == idl_ttypedef) {
                subType = idl_typeDefResolveFully(subType);
            }

            /* If the element type of the sequence has a related key, which means that readers and
             * writers have been created, and which also means that type sequence support functions
             * have already been created in the other context.
             */

            if (idl_sequenceSupportFunctionsExist(scope, idl_typeSeq(idl_typeDefRefered(defSpec)), sequenceElementName)) {
                if (idl_keyDefIncludesType(idl_keyDefDefGet(), sequenceElementName)) {
                    sequenceName = idl_sequenceIdent (idl_typeSeq(idl_typeDefRefered(defSpec)));
                } else {
                    sequenceName = idl_sequenceIdentScoped (scope, idl_typeSeq(idl_typeDefRefered(defSpec)));
                }
            } else {
                sequenceName = idl_sequenceIdent (idl_typeSeq(idl_typeDefRefered(defSpec)));
            }

            /* Generate allocation routine for the sequence */

            if (idl_inlineBoundedString(subType, inlineBoundedStrings)) {
                scopedTypeIdent = idl_scopedSacTypeIdent(idl_typeSeqType(idl_typeSeq(idl_typeDefRefered(defSpec))));
                idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
                        scopedName, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_sequence_malloc ();\n",
                        scopedName);
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
                /* Generate allocation routine for the sequence buffer */
                idl_fileOutPrintf(idl_fileCur(), "%s *%s_allocbuf (DDS_unsigned_long len)\n",
                        scopedTypeIdent, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_sequence_allocbuf(NULL, sizeof(%s), len);\n",
                        scopedTypeIdent, scopedTypeIdent);
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
                os_free(scopedTypeIdent);
            } else {
                scopedTypeIdent = idl_scopedSacTypeIdent(idl_typeSeqType(idl_typeSeq(idl_typeDefRefered(defSpec))));

                idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
                        scopedName, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    return (%s *)%s__alloc ();\n",
                        scopedName, sequenceName);
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
                /* Generate allocation routine for the sequence buffer */
                idl_fileOutPrintf(idl_fileCur(), "%s *%s_allocbuf (DDS_unsigned_long len)\n",
                        scopedTypeIdent, scopedName);
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(idl_fileCur(), "    return (%s *)%s_allocbuf(len);\n",
                        scopedTypeIdent, sequenceName);
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
                os_free(scopedTypeIdent);
            }
            os_free(sequenceElementName);
            os_free(sequenceName);
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
    c_bool createFunctions = TRUE;
    char *sequenceElementName;
    char *sequenceName;
    c_bool inlineBoundedStrings = ((SACObjectControlUserData *)userData)->inlineBoundedStrings;

    sequenceElementName = idl_sequenceElementIdent(idl_typeSeqType(typeSeq));
    sequenceName = idl_sequenceIdent(typeSeq);

    /* The purpose of this function is to create type sequence support functions.
     *
     * The type, however, can be in a different file than this sequence. Let's
     * call that context. The context can be checked by getting the base of a
     * scope. By comparing the base of the working scope and the type scope, you
     * can determine if the context is the same or not.
     *
     * If the contexts are the same, then there's no worries and the type sequence
     * support functions should be created.
     *
     * If the contexts are different, then problems can occur because
     * 'idl_definitionExists()' only checks the current context for possible type
     * sequence support functions. When we would create new support function while
     * another context already has them, then the generated code will not be able
     * to compile due to 'multiple definitions'.
     *
     * So, we have to figure out if the current type is defined within the current
     * working context by checking the bases. If not, then figure out if the other
     * context already has defined the type sequence support functions.
     *
     * N.B. The working context is always the lowest in the current hierarchy.
     */

    /* Check if current sequence support functions are already defined within
     * the current working context.
     */
    if (idl_definitionExists("objManagImpl", sequenceName)) {
        createFunctions = FALSE;
    } else {
        idl_definitionAdd("objManagImpl", sequenceName);
        if ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tbasic) &&
            ((idl_typeBasicType(idl_typeBasic(idl_typeSeqType(typeSeq))) == idl_octet) ||
            (idl_typeBasicType(idl_typeBasic(idl_typeSeqType(typeSeq))) == idl_string))) {
            createFunctions = FALSE;
        } else if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_ttypedef) {
            if (idl_inlineBoundedString(idl_typeSeqType(typeSeq), inlineBoundedStrings)) {
                createFunctions = FALSE;
            }
        } else {
            if (idl_sequenceSupportFunctionsExist(scope, typeSeq, sequenceElementName)) {
                createFunctions = FALSE;
            }
        }
    }

    if (createFunctions) {
        char *sequenceScopedName;

        if (idl_scopeStackSize(scope) > 0) {
            if (idl_keyDefIncludesType(idl_keyDefDefGet(), sequenceElementName)) {
                sequenceScopedName = os_strdup(sequenceName);
            } else {
                sequenceScopedName = idl_sequenceIdentScoped(scope, typeSeq);
            }
        } else {
            sequenceScopedName = os_strdup(sequenceName);
        }

        /* Generate allocation routine for the sequence */
        idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
                sequenceName, sequenceScopedName);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_sequence_malloc();\n",
                sequenceName);
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");

        /* Generate allocation routine for the sequence buffer */
        idl_fileOutPrintf(idl_fileCur(), "%s *%s_allocbuf (DDS_unsigned_long len)\n",
                sequenceElementName, sequenceScopedName);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        if (idl_typeSpecHasRef(idl_typeSeqType(typeSeq))) {
            idl_fileOutPrintf(idl_fileCur(), "    DDS_ReturnCode_t %s_freebuf (void *buffer);\n\n",
                    sequenceScopedName);
            idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_sequence_allocbuf (%s_freebuf, sizeof (%s), len);\n",
                    sequenceElementName, sequenceScopedName, sequenceElementName);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "    return (%s *)DDS_sequence_allocbuf (NULL, sizeof (%s), len);\n",
                    sequenceElementName, sequenceElementName);
        }
        idl_fileOutPrintf(idl_fileCur(), "}\n");

        if (idl_typeSpecHasRef(idl_typeSeqType(typeSeq))) {
            /* Deallocation routine for the buffer is required */
            idl_fileOutPrintf(idl_fileCur(), "\nDDS_ReturnCode_t %s_freebuf (void *buffer);\n",
                    sequenceScopedName);
            idl_fileOutPrintf(idl_fileCur(), "DDS_ReturnCode_t %s_freebuf (void *buffer)\n",
                    sequenceScopedName);
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long *count = (DDS_unsigned_long *)DDS__header (buffer);\n");
            idl_fileOutPrintf(idl_fileCur(), "    %s *b = (%s *)buffer;\n",
                    sequenceElementName, sequenceElementName);
            idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long i;\n");
            if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tseq ||
                    ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_ttypedef) &&
                    (idl_typeSpecType(idl_typeDefResolveFully(idl_typeSeqType(typeSeq))) == idl_tseq))) {
                idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
                idl_fileOutPrintf(idl_fileCur(), "        DDS_sequence_free ((_DDS_sequence)&b[i]);\n");
                idl_fileOutPrintf(idl_fileCur(), "    }\n");
            } else {
                if ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tbasic) ||
                    ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_ttypedef) &&
                    (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))) == idl_tbasic))) {
                    /* Only string has reference */
                    if (!idl_inlineBoundedString(idl_typeSeqType(typeSeq), inlineBoundedStrings)) {
                        idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
                        idl_fileOutPrintf(idl_fileCur(), "        DDS_string_clean (&b[i]);\n");
                        idl_fileOutPrintf(idl_fileCur(), "    }\n");
                    }
                } else {
                    idl_fileOutPrintf(idl_fileCur(), "    DDS_ReturnCode_t %s__free (void *object);\n\n",
                            sequenceElementName);
                    idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
                    idl_fileOutPrintf(idl_fileCur(), "        %s__free (&b[i]);\n",
                            sequenceElementName);
                    idl_fileOutPrintf(idl_fileCur(), "    }\n");
                }
            }
            idl_fileOutPrintf (idl_fileCur(), "    return DDS_RETCODE_OK;\n");
            idl_fileOutPrintf (idl_fileCur(), "}\n");
        }
        idl_fileOutPrintf (idl_fileCur(), "\n");

        os_free(sequenceScopedName);
    }

    os_free(sequenceName);
    os_free(sequenceElementName);
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
idl_genSacObjectControl = {
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
    idl_sequenceOpenClose,
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSacObjectControlProgram(
        SACObjectControlUserData *userData)
{
    idl_genSacObjectControl.userData = userData;

    return &idl_genSacObjectControl;
}
