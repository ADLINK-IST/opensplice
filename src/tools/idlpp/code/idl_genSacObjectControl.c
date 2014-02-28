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

#include "os_heap.h"
#include "os_stdlib.h"

#include <ctype.h>
#include "c_typebase.h"

	/** indentation level */
static c_long indent_level = 0;
	/** enumeration element index */
static c_long enum_element = 0;
static c_bool struct_hasRef = FALSE;
static c_bool union_hasRef = FALSE;

static c_long loopIndent;
static c_long varIndex;

static void idl_arrayDimensions (idl_typeArray typeArray);
static void idl_arrayElements (idl_typeArray typeArray, const char *identifier, c_long indent);
static void
idl_arrayLoopRemoveBody(
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent);
static void
idl_arrayLoopRemove(
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent);

static c_char *
idl_valueFromLabelVal(
    idl_labelVal labelVal)
{
    static c_char labelName [1000];

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
        snprintf (labelName, (size_t)sizeof(labelName), "_%s", idl_labelEnumVal(idl_labelEnum(labelVal)));
    } else {
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
	    case V_CHAR:
            snprintf(labelName, (size_t)sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
		break;
	    case V_SHORT:
            snprintf(labelName, (size_t)sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
		break;
	    case V_USHORT:
            snprintf(labelName, (size_t)sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
		break;
	    case V_LONG:
            snprintf(labelName, (size_t)sizeof(labelName), "%d", idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
		break;
	    case V_ULONG:
            snprintf(labelName, (size_t)sizeof(labelName), "%u", idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
		break;
	    case V_LONGLONG:
            snprintf(labelName, (size_t)sizeof(labelName), "%lld", idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
		break;
	    case V_ULONGLONG:
            snprintf(labelName, (size_t)sizeof(labelName), "%llu", idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
		break;
	    case V_BOOLEAN:
            /* QAC EXPECT 3416; No side effect here */
            if ((int)idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
                snprintf(labelName, (size_t)sizeof(labelName), "TRUE");
            } else {
                snprintf(labelName, (size_t)sizeof(labelName), "FALSE");
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

/* @brief callback function called on opening the IDL input file.
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
    idl_fileOutPrintf(idl_fileCur(), "#include <dds_dcps_private.h>\n\n");

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

    if (idl_definitionExists ("objManagImpl", idl_scopeStack (scope, "_", name))) {
        return idl_abort;
    }
    idl_definitionAdd("objManagImpl", idl_scopeStack (scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
	idl_scopeStack(scope, "_", name),
	idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    if (idl_typeSpecHasRef(idl_typeSpec(structSpec))) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    DDS_boolean %s__free (void *object);\n\n",
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (%s *)DDS__malloc (%s__free, 0, sizeof(%s));\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        struct_hasRef = TRUE;
        action = idl_explore;
    } else {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (%s *)DDS__malloc (NULL, 0, sizeof(%s));\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        struct_hasRef = FALSE;
        action = idl_abort;
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    if (idl_typeSpecHasRef(idl_typeSpec(structSpec))) {
        /* Deallocation routine are required */
        idl_fileOutPrintf(
            idl_fileCur(),
            "DDS_boolean %s__free (void *object)\n",
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(
            idl_fileCur(),
            "    %s *o = (%s *)object;\n\n",
    	    idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
    }
    /* return action to indicate that the rest of the structure needs to be processed or not */
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
    if (struct_hasRef) {
        idl_fileOutPrintf(idl_fileCur(), "    return TRUE;\n");
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
    if (idl_typeSpecHasRef(typeSpec)) {
        if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
            (idl_typeSpecType(typeSpec) == idl_tunion)) {

	    /* case 6148 */
            idl_fileOutPrintf(idl_fileCur(), "    {\n");
            idl_fileOutPrintf(idl_fileCur(), "    extern DDS_boolean %s__free(void *object);\n",
                idl_scopedTypeName(typeSpec));

            idl_fileOutPrintf(
                idl_fileCur(),
                "    %s__free (&o->%s);\n",
    	        idl_scopedSacTypeIdent((typeSpec)),
                name);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
            } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
                idl_structureMemberOpenClose(scope, name, idl_typeDefRefered(idl_typeDef(typeSpec)), userData);
            } else if (idl_typeSpecType(typeSpec) == idl_tbasic &&
                idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    DDS_string_clean (&o->%s);\n",
                    name);
            } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
                char identifier[256];

                snprintf(identifier, sizeof(identifier), "o->%s", name);
                idl_arrayElements(idl_typeArray(typeSpec), identifier, 0);
            } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    DDS_sequence_clean (&o->%s);\n",
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

    if (idl_definitionExists("objManagImpl", idl_scopeStack (scope, "_", name))) {
        return idl_abort;
    }
    idl_definitionAdd("objManagImpl", idl_scopeStack (scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "%s *%s__alloc (void)\n",
	idl_scopeStack(scope, "_", name),
	idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    if (idl_typeSpecHasRef(idl_typeSpec(unionSpec))) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    DDS_boolean %s__free (void *object);\n\n",
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (%s *)DDS__malloc (%s__free, 0, sizeof(%s));\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        union_hasRef = TRUE;
        action = idl_explore;
    } else {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (%s *)DDS__malloc (NULL, 0, sizeof(%s));\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        union_hasRef = FALSE;
        action = idl_abort;
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
    if (idl_typeSpecHasRef(idl_typeSpec(unionSpec))) {
	/* Deallocation routine are required */
        idl_fileOutPrintf(
            idl_fileCur(),
            "DDS_boolean %s__free (void *object)\n",
            idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(
            idl_fileCur(),
            "    %s *o = (%s *)object;\n\n",
            idl_scopeStack (scope, "_", name),
            idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    switch (o->_d) {\n");
    }
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
    if (union_hasRef) {
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        idl_fileOutPrintf(idl_fileCur(), "    return TRUE;\n");
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
    if (idl_typeSpecHasRef(typeSpec)) {
        if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
            (idl_typeSpecType(typeSpec) == idl_tunion)) {

            idl_fileOutPrintf(
                idl_fileCur(),
                "        %s__free (&o->_u.%s);\n",
                idl_scopedSacTypeIdent((typeSpec)),
                name);
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
            idl_unionCaseOpenClose(scope, name, idl_typeDefRefered(idl_typeDef(typeSpec)), userData);
        } else if (idl_typeSpecType(typeSpec) == idl_tbasic) {
            if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "        DDS_string_clean (&o->_u.%s);\n",
                    name);
            }
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
            char identifier[256];

            snprintf(identifier, sizeof(identifier), "o->_u.%s",name);
            idl_arrayElements(idl_typeArray(typeSpec), identifier, 1);
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        DDS_sequence_clean (&o->_u.%s);\n",
                name);
            idl_fileOutPrintf(idl_fileCur(), "        break;\n");
        } else {
            idl_fileOutPrintf (idl_fileCur(), "        break;\n");
        }
    } else {
        idl_fileOutPrintf (idl_fileCur(), "        break;\n");
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
    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(labelVal) == idl_ldefault) {
        idl_fileOutPrintf(idl_fileCur(), "    default:\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    case %s:\n", idl_valueFromLabelVal(labelVal));
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
    idl_typeArray typeArray)
{
    idl_typeSpec idlSubtype;

    idlSubtype = idl_typeDefResolveFully(idl_typeArrayType(typeArray));
    if (idl_typeSpecType(idlSubtype) == idl_tarray) {
        idl_arrayDimensions(idl_typeArray(idlSubtype));
    }
    idl_fileOutPrintf(idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
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
        loopIndent,
        loopIndent,
        idl_typeArraySize(typeArray),
        loopIndent);
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
    c_long indent)
{
    idl_typeSpec typeSpec = idl_typeDefRefered(typeDef);

    switch (idl_typeSpecType(typeSpec)) {
    case idl_ttypedef:
        idl_typedefRemove(typeArray, idl_typeDef(typeSpec), identifier, indent);
	break;
    case idl_tstruct:
    case idl_tunion:

	/* case 6148 */
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    extern DDS_boolean %s__free(void *object);\n",
            idl_scopedSacTypeIdent(typeSpec));

        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    %s__free (&%s",
            idl_scopedSacTypeIdent((typeSpec)),
            identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
	break;
    case idl_tbasic:
        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    DDS_string_clean (&%s",
            identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
	break;
    case idl_tseq:
        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    DDS_sequence_clean (&%s",
            identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ");\n");
	break;
    case idl_tarray:
        idl_arrayLoopRemoveBody(idl_typeArray(typeSpec), identifier, indent);
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
    c_long indent)
{
    idl_typeSpec typeSpec = idl_typeArrayActual(typeArray);

    loopIndent++;
    varIndex = 0;
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tstruct:
    case idl_tunion:

	/* case 6148 */
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    extern DDS_boolean %s__free(void *object);\n",
            idl_scopedSacTypeIdent(typeSpec));

        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    %s__free (&%s",
            idl_scopedSacTypeIdent((typeSpec)),
            identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
	break;
    case idl_ttypedef:
        idl_typedefRemove(typeArray, idl_typeDef(typeSpec), identifier, indent);
	break;
    case idl_tbasic:
        idl_printIndent(indent);
        idl_fileOutPrintf
            (idl_fileCur(),
            "    DDS_string_clean (&%s",
            identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf(idl_fileCur(), ");\n");
	break;
    case idl_tseq:
        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    DDS_sequence_clean (&%s",
            identifier);
        idl_arrayLoopRemoveIndex(typeArray);
        idl_fileOutPrintf (idl_fileCur(), ");\n");
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
    c_long indent)
{
    loopIndent = 0;
    idl_arrayLoopVariables(typeArray, indent);
    idl_arrayLoopRemoveOpen(typeArray, indent);
    idl_arrayLoopRemoveBody(typeArray, identifier, indent + idl_indexSize(typeArray));
    idl_arrayLoopRemoveClose(typeArray, indent);
}

static void
idl_arrayElements (
    idl_typeArray typeArray,
    const char *identifier,
    c_long indent)
{
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    switch (idl_typeSpecType(idl_typeArrayActual(typeArray))) {
    case idl_tbasic:
        /* This is only called for string type */
        if (idl_typeBasicType(idl_typeBasic(idl_typeArrayActual(typeArray))) == idl_string) {
            idl_arrayLoopRemove (typeArray, identifier, indent+1);
        }
	break;
    case idl_tstruct:
    case idl_tunion:
    case idl_ttypedef:
    case idl_tseq:
        idl_arrayLoopRemove(typeArray, identifier, indent+1);
    break;
    default:
        printf ("idl_arrayElements: Unexpected type %d\n",
	    idl_typeSpecType(idl_typeArrayActual(typeArray)));
    break;
    }
    idl_printIndent(indent);
    idl_fileOutPrintf(idl_fileCur(), "    }\n");
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
    if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct) ||
        (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion)) {
        /* Generate allocation routine for the struct or union */
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s *%s__alloc (void)\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (%s *)%s__alloc ();\n",
            idl_scopeStack(scope, "_", name),
            idl_scopedSacTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec))));
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        /* Generate deallocation routine for the struct or union */
        if (idl_typeSpecHasRef(idl_typeDefRefered(defSpec))) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "DDS_boolean %s__free (void *object)\n",
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_fileOutPrintf(
                idl_fileCur(),
                "    %s__free (object);\n",
                idl_scopedSacTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec))));
            idl_fileOutPrintf(idl_fileCur(), "    return TRUE;\n");
            idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        }
    } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_ttypedef) {
        idl_typedefOpenClose(scope, name, idl_typeDef(idl_typeDefRefered(defSpec)), userData);
    } else {
        if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tbasic &&
            idl_typeBasicType(idl_typeBasic(idl_typeDefRefered(defSpec))) == idl_string) {
            /* Generate allocation routine for the string */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s *%s__alloc (DDS_unsigned_long len)\n",
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_fileOutPrintf(
                idl_fileCur(),
                "    return (%s *)DDS_string_alloc (len);\n",
                idl_scopeStack(scope, "_", name),
                idl_scopedSacTypeIdent(idl_typeSpec(idl_typeDefRefered(defSpec))));
            idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tarray) {
            /* Generate allocation routine for the array */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s_slice *%s__alloc (void)\n",
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name));
                idl_fileOutPrintf(idl_fileCur(), "{\n");
            if (idl_typeSpecHasRef(idl_typeDefRefered(defSpec))) {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    DDS_boolean %s__free (void *array);\n",
                    idl_scopeStack (scope, "_", name));
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    return (%s_slice *)DDS__malloc (%s__free, 0, sizeof(%s));\n",
                    idl_scopeStack(scope, "_", name),
                    idl_scopeStack(scope, "_", name),
                    idl_scopeStack(scope, "_", name));
            } else {
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    return (%s_slice *)DDS__malloc (NULL, 0, sizeof(%s));\n",
                    idl_scopeStack(scope, "_", name),
                    idl_scopeStack(scope, "_", name));
            }
            idl_fileOutPrintf(idl_fileCur(), "}\n");
            if (idl_typeSpecHasRef(idl_typeDefRefered(defSpec))) {
                /* Deallocation routine for the array elements is required */
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "\nDDS_boolean %s__free (void *array)\n",
                    idl_scopeStack(scope, "_", name));
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    %s *a = (%s *)array;\n",
                    idl_scopeStack(scope, "_", name),
                    idl_scopeStack (scope, "_", name));
                    idl_arrayElements(idl_typeArray(idl_typeDefRefered(defSpec)), "(*a)", 0);
                    idl_fileOutPrintf(idl_fileCur(), "    return TRUE;\n");
                    idl_fileOutPrintf(idl_fileCur(), "}\n");
            }
            idl_fileOutPrintf (idl_fileCur(), "\n");
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
            /* Generate allocation routine for the sequence */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s *%s__alloc (void)\n",
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_fileOutPrintf(
                idl_fileCur(),
                "    return (%s *)%s__alloc ();\n",
                idl_scopeStack(scope, "_", name),
                idl_sequenceIdent(idl_typeSeq(idl_typeDefRefered(defSpec))));
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
            /* Generate allocation routine for the sequence buffer */
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s *%s_allocbuf (DDS_unsigned_long len)\n",
                idl_scopedSacTypeIdent(idl_typeSeqType(idl_typeSeq(idl_typeDefRefered(defSpec)))),
                idl_scopeStack(scope, "_", name));
                idl_fileOutPrintf(idl_fileCur(), "{\n");
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    return (%s *)%s_allocbuf(len);\n",
                    idl_scopedSacTypeIdent(idl_typeSeqType(idl_typeSeq(idl_typeDefRefered(defSpec)))),
                    idl_sequenceIdent (idl_typeSeq(idl_typeDefRefered(defSpec))));
                idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        }
    }
}


static void
idl_sequenceOpenClose(
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    int   createFunctions;
    char *sequenceElementName;
    char *sequenceName;
    idl_scope typeScope;

    createFunctions = TRUE;

    sequenceElementName = idl_sequenceElementIdent(idl_typeSeqType(typeSeq));
    sequenceName = idl_sequenceIdent(typeSeq);

    /* Check if current sequence support functions are already defined within
     * the current working context. */
    if (idl_definitionExists("objManagImpl", sequenceName))
    {
        createFunctions = FALSE;
    } else {
        idl_definitionAdd("objManagImpl", sequenceName);
    }

    if (createFunctions) {
        /* Already defined as part of the core implementation */
        if ((strcmp(sequenceName, "DDS_sequence_octet")  == 0) ||
            (strcmp(sequenceName, "DDS_sequence_string") == 0) )
        {
            createFunctions = FALSE;
        }
    }

    if (createFunctions) {
        /*
         * The purpose of this function is to create type sequence support functions.
         *
         * The type, however, can be in a different file than this sequence. Let's
         * call that context. The context can be checked by getting the base of a
         * scope. By comparing the base of the working scope and the type scope, you
         * can determine if the context is the same or not.
         *
         * If the contexts are the same, then there's no worries and the type sequence
         * support functions should be created.
         *
         * If the contexts are different, then proplems can occur because
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
        typeScope = idl_typeUserScope(idl_typeUser(idl_typeSeqType(typeSeq)));
        if (typeScope != NULL) {

            char *typeBase = idl_scopeBasename(typeScope);
            char *workBase = idl_scopeBasename(scope);
            if (strcmp(typeBase, workBase) != 0) {

                /* If a type has a related key, that means that readers and
                 * writers have been created, which also means that type
                 * sequence support functions have already been created in
                 * the other context. */
                if (idl_keyDefIncludesType(idl_keyDefDefGet(), sequenceElementName)) {
                    createFunctions = FALSE;
                }

                /*
                 * We should search all scopes of all contexts to be sure that
                 * no sequence support functions for this type are defined
                 * anywhere within the hierarchy by means of an actual
                 * 'sequence<Type>' definition in an idl file.
                 * We shoudn't only search the context of the type itself for
                 * sequences of this type because there could be a file include
                 * between the current working context and the type definition
                 * context, in which a sequence can be defined.
                 *
                 * We don't have access to all contexts at this point.
                 * Also, only the scope information isn't enough to detect
                 * sequences. So, basically, we don't have enough information
                 * here to do a proper search.
                 *
                 * This multiple sequences problem, however, can be solved by
                 * adding a sequence typedef in the idl hierarchy and use that
                 * typedef everywhere else.
                 */
            }
            os_free(typeBase);
            os_free(workBase);
        }
    }

    if (createFunctions) {
       /* Generate allocation routine for the sequence */
       idl_fileOutPrintf(
          idl_fileCur(),
          "%s *%s__alloc (void)\n",
          sequenceName,
          sequenceName);
       idl_fileOutPrintf(idl_fileCur(), "{\n");
       idl_fileOutPrintf(
          idl_fileCur(),
          "    return (%s *)DDS_sequence_malloc();\n",
          sequenceName);
       idl_fileOutPrintf(idl_fileCur(), "}\n\n");

       /* Only needed if the sac dcps API does not have it already... */
       if ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tbasic) &&
           ((idl_typeBasicType(idl_typeBasic(idl_typeSeqType(typeSeq))) == idl_octet) ||
            (idl_typeBasicType(idl_typeBasic(idl_typeSeqType(typeSeq))) == idl_string)))
       {
          idl_fileOutPrintf(
             idl_fileCur(),
             "/* Ignoring code generation for %s *%s_allocbuf (void)*/\n",
             sequenceName,
             sequenceName);
       }
       else
       {
          /* Generate allocation routine for the sequence buffer */
          idl_fileOutPrintf(
             idl_fileCur(),
             "%s *%s_allocbuf (DDS_unsigned_long len)\n",
             sequenceElementName,
             sequenceName);
          idl_fileOutPrintf(idl_fileCur(), "{\n");
          if (idl_typeSpecHasRef(idl_typeSeqType(typeSeq)))
          {
             idl_fileOutPrintf(
                idl_fileCur(),
                "    DDS_boolean %s_freebuf (void *buffer);\n\n",
                sequenceName);
             idl_fileOutPrintf(
                idl_fileCur(),
                "    return (%s *)DDS_sequence_allocbuf (%s_freebuf, sizeof (%s), len);\n",
                sequenceElementName,
                sequenceName,
                sequenceElementName);

          }
          else
          {
             idl_fileOutPrintf(
                idl_fileCur(),
                "    return (%s *)DDS_sequence_allocbuf (NULL, sizeof (%s), len);\n",
                sequenceElementName,
                sequenceElementName);
          }
          idl_fileOutPrintf(idl_fileCur(), "}\n");
       }
       if (idl_typeSpecHasRef(idl_typeSeqType(typeSeq)))
       {
          /* Deallocation routine for the buffer is required */
          idl_fileOutPrintf(
             idl_fileCur(),
             "\nDDS_boolean %s_freebuf (void *buffer)\n",
             sequenceName);
          idl_fileOutPrintf(idl_fileCur(), "{\n");
          idl_fileOutPrintf(idl_fileCur(),
                            "    DDS_unsigned_long *count = (DDS_unsigned_long *)DDS__header (buffer);\n");
          idl_fileOutPrintf(idl_fileCur(),
                            "    %s *b = (%s *)buffer;\n",
                            sequenceElementName,
                            sequenceElementName);
          idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long i;\n");
          if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tseq)
          {
             idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
             idl_fileOutPrintf(
                idl_fileCur(),
                "        DDS_sequence_free (&b[i]);\n",
                sequenceElementName);
             idl_fileOutPrintf(idl_fileCur(), "    }\n");
          }
          else
          {
             if ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tbasic) ||
                 ((idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_ttypedef) &&
                  (idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))) == idl_tbasic)))
             {
                /* Only string has reference */
                idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
                idl_fileOutPrintf(idl_fileCur(), "        DDS_string_clean (&b[i]);\n");
                idl_fileOutPrintf(idl_fileCur(), "    }\n");
             }
             else
             {
                idl_fileOutPrintf(
                   idl_fileCur(),
                   "    DDS_boolean %s__free (void *object);\n\n",
                   sequenceElementName);
                idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
                idl_fileOutPrintf(
                   idl_fileCur(),
                   "        %s__free (&b[i]);\n",
                   sequenceElementName);
                idl_fileOutPrintf(idl_fileCur(), "    }\n");
             }
          }
          idl_fileOutPrintf (idl_fileCur(), "    return TRUE;\n");
          idl_fileOutPrintf (idl_fileCur(), "}\n");
       }
       idl_fileOutPrintf (idl_fileCur(), "\n");
    }
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
    void)
{
    return &idl_genSacObjectControl;
}
