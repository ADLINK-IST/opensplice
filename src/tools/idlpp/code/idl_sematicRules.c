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
//  * 1 Introduction
//  *
//  *      This document analyses the semantic checks required for the OpenSpliceDDS IDL pre-processor on the IDL input.
//  *      The analysis is based upon OMG Spec "The Common Object Request Broker: Archtecture and Specification", V2.3.1.
//  *      Semantic rules are identified by a [] enclosed number.
//  *
//  * 2 Generic
//  *
//  *      [1]  OMG IDL file must have "idl" extension. (3-3 3.1)
//  *               function: Will not be checked
//  *
//  *      [2]  OMG IDL uses ASCII charset. (3-3 3.1)
//  *               function: Will not be checked
//  *
//  *      [3]  OMG IDL identifiers consist of an arbitrary long sequence of ASCII alphabetic, digit and underscore
//  *           characters. (3-6 3.2.3)
//  *               function: Is checked by the scanner
//  *
//  *      [4]  OMG IDL identifiers must start with an ASCII alphabetic character. (3-6 3.2.3)
//  *               function: Is checked by the scanner
//  *
//  *      [5]  Uppercase and lowercase characters are treated as the same letter, but OMG IDL identifiers that only
//  *           differ in case collide. Identifiers for a given definition must be spelled identically with respect
//  *           to the case. (3-6 3.2.3)
//  *               funtion: idl_checkReferencedIdentifier
//  *
//  *      [6]  An identifier may be used only once within a namespace. There is only one namespace for each
//  *           identifier in each scope. (3-6 3.2.3)
//  *           A type name may be redefined in a nested scope. (3-50 3.15.3)
//  *               function: idl_checkModuleDefinition
//  *               function: idl_checkConstantDefinition
//  *               function: idl_checkTypeDefinition
//  *               function: idl_checkinterfaceDefinition
//  *               function: idl_checkEnumerationElementDefinition
//  *               function: idl_checkExceptionDefinition
//  *               function: idl_checkOperationDefinition
//  *
//  * 3 Constant Declaration
//  *
//  *      [7]  The <scoped_name> in the <const_type> production must be a previously defined name of an <integer_type>,
//  *           <char_type>, <wide_char_type>, <boolean_type>, <floating_pt_type>, <string_type>, <wide_string_type>,
//  *           <octet_type>, or <enum_type> constant. (3-29 3.9.2)
//  *               function: checked by the parser
//  *
//  *      [8]  An infix operator can combine two integers, floats or fixeds, but not mixtures of these. (3-29 3.9.2)
//  *               function: idl_checkBinaryExpression
//  *
//  *      [9]  Infix operators are applicable only to integer, float and fixed types. (3-29 3.9.2)
//  *               function: idl_checkBinaryExpression
//  *
//  *      [10] It is an error if a sub-expression of an integer or float constant declaration exceeds the precision
//  *           of the target type. (3-29 3.9.2)
//  *               function: idl_checkBinaryExpression
//  *
//  *      [11] Unary '+' and '-' and binary '*', '/', '+' and '-' are applicable in floating-point and fixed-point
//  *           expressions. (3-30 3.9.2)
//  *               function: idl_checkUnaryExpression
//  *
//  *      [12] Unary '+', '-' and '~' and binary '*', '/', '%', '+', '-', '<<', '>>', '&', '|' and '^' operations
//  *           are applicable in integer expressions. (3-30 3.9.2)
//  *               function: idl_checkUnaryExpression
//  *               function: idl_checkBinaryExpression
//  *
//  *      [13] The right hand operand value of the ">>" and "<<" binary operator must be in the range 0..63. (3-31 3.9.2)
//  *               function: idl_checkBinaryExpression
//  *
//  *      [14] <positive_int_const> must evaluate to a positive integer constant.  (3-31 3.9.2)
//  *               function: idl_checkConstantDeclaration
//  *
//  *      [15] Values for an octet constant outside the range 0..255 shall cause a compile-time error. (3-31 3.9.2)
//  *               function: idl_checkConstantDeclaration
//  *
//  *      [16] The constant name for the right hand side of an enumerated constant definition must denote one of the
//  *           enumerators defined for the enumerated type of the constant. (3-31 3.9.2)
//  *               function: idl_checkConstantDeclaration
//  *
//  *      [17] The char data type can take an 8 bit quantity (0-255). (3-34 3.10.1.3)
//  *               function: idl_checkConstantDeclaration
//  *
//  *      [18] The boolean data type can take one of the values "TRUE" and "FALSE". (3-34 3.10.1.5)
//  *               function: idl_checkConstantDeclaration
//  *
//  * 4 Type Declaration
//  *
//  *      [19] The <scoped_name> in <simple_type_spec> must be a previously defined type. (3-31 3.10)
//  *               funtion: Checked by the parser
//  *
//  * 4.1 Constructed Type
//  *
//  *      [20] The only recursion allowed is via the use of the sequence template type. (3-35 3.10.2)
//  *
//  * 4.2 Structures
//  *
//  *      [21] Structure member declarators in a particular structure must be unique. (3-35 3.10.2.1)
//  *               function: idl_checkStructDeclaratorDefinition
//  *
//  * 4.3 Discriminated Unions
//  *
//  *      [22] The <const_exp> in a <case_label> must be consistent with the <switch_type_spec>. (3-36 3.10.2.2)
//  *               function: idl_checkConstantOperand
//  *
//  *      [23] A default case can appear at most once. (3-36 3.10.2.2)
//  *               function: idl_checkUnionCaseDefinition
//  *
//  *      [24] The <scoped_name> in the <switch_type_spec> must be a previously defined integer, char, boolean
//  *           or enum type. (3-36 3.10.2.2)
//  *               function: idl_checkUnionCaseDefinition
//  *
//  *      [25] Case labels must match or be automatically castable to the defined type of the discriminator
//  *           (see table 3-12). (3-36 3.10.2.2)
//  *               function: idl_checkConstantOperand
//  *
//  *      [26] Element declarators in a particular union must be unique. (3-37 3.10.2.2)
//  *               function: idl_checkUnionDeclaratorDefinition
//  *
//  *      [27] If the <switch_type_spec> is an <enum_type>, the identifier for the enumeration is in the scope of
//  *           the union; as a result, it must be distinct from the element declarators (3-37 3.10.2.2).
//  *               function: idl_checkUnionDeclaratorDefinition
//  *
//  *      [28] It is illegal to specify a union with the default case label if the set of case labels completely
//  *           covers the possible values for the discriminant. (IDL to Java Language Mapping Specification 1-21 1.9)
//  *               function: idl_checkUnionDeclaratorDefinition
//  *
//  * 4.4 Enumerations
//  *
//  *      [29] A maximum of 2^32 identifiers may be specified. (3-37 3.10.2.3)
//  *               function: idl_checkEnumerationElementCount
//  *
//  * 4.5 Template Types
//  *
//  *      [30] Value of <positive_int_const> must evaluate to a positive integer constant. (3-38 3.10.3.1)
//  *               function: idl_checkIntegerPositive
//  *
//  *      [31] Value of <positive_int_const> must evaluate to a positive integer constant. (3-38 3.10.3.2)
//  *               function: idl_checkIntegerPositive
//  *
//  *      [32] Value of <positive_int_const> must evaluate to a positive integer constant. (3-39 3.10.3.3)
//  *               function: idl_checkIntegerPositive
//  *
//  * 4.6 Complex Declarator
//  *
//  *      [33] Value of <positive_int_const> must evaluate to a positive integer constant. (3-39 3.10.4.1)
//  *               function: idl_checkIntegerPositive
//  *
//  * 5 Keylist
//  *
//  *      The following syntax is assumed: #pragma keylist <scoped_name> <member_declarator>*
//  *
//  *      [34] The <scoped_name> must be the name a previously defined type of which the actual type is a
//  *           structure type or an union type.
//  *               function: idl_checkKeyListTypeName
//  *
//  *      [35] The <member_declarator> is the name of a member of the structure that is specified with <scoped_name>.
//  *               function: idl_checkKeyListFieldName
//  *
//  *      [36] The syntax should also allow ',' as seperator for the <member_declarator>
//  *
//  *      [37] The member specified by the <member_declarator> should be one of the following types:
//  *           - primitive types: boolean, char, long, octet,
//  *           - enumeration
//  *
//  * 6 Unsupported types
//  *
//  *      [38] The preprocessor shall not generate code for the following primitive types or
//  *           constructed types using these primitive types:
//  *           - valuetype, long double, wchar, any, Object, wstring, fixed, ValueBase
//  *        function: checkUnsupportedTypeUsage (called from idl_checkKeyListTypeName)
//
*/
#include "c_iterator.h"
#include "c_metabase.h"
#include "c_misc.h"
#include "c_base.h"
#include "c_typebase.h"
#include "c_stringSupport.h"

#include "os_stdlib.h"

#include "idl_sematicRules.h"
#include "idl_program.h"
#include "idl_unsupported.h"
#include "idl_walk.h"

#include "ut_stack.h"

/*************************************************************************************************
 * Local Defines
 *************************************************************************************************/

#define IDL_MAX_ERRORSIZE       512

/*************************************************************************************************
 * Local Type Definitions
 *************************************************************************************************/

typedef enum {
    idl_Integer,
    idl_Float,
    idl_Boolean,
    idl_Octet,
    idl_Char,
    idl_String,
    idl_Other
} idl_constantClass;

typedef enum {
    idl_Spelling,
    idl_RedeclarationSpelling,
    idl_Redeclaration,
    idl_SwitchConflictSpelling,
    idl_SwitchConflict,
    idl_DefaultLabel,
    idl_SwitchType,
    idl_NoDefault,
    idl_EnumerationLimit,
    idl_UndeclaredDeclarator,
    idl_UndeclaredIdentifier,
    idl_ConstantTypeConflict,
    idl_ConstantRangeConflict,
    idl_EnumerationElement,
    idl_IllegalUnaryOperator,
    idl_IllegalBinaryOperator,
    idl_ConstantExprMismatch,
    idl_ConstantExprConflict,
    idl_ShiftRangeConflict,
    idl_CaseLabelLiteralType,
    idl_IllegalKeyFields,
    idl_InvalidKind,
    idl_IllegalKeyType,
    idl_UnsupportedType,
    idl_UnsupportedKeyType,
    idl_IllegalFieldName
} idl_errorIndex;

struct unsupportedArg {
    c_bool unsupported;
    const c_char *typeName;
    ut_stack context;
};

/*************************************************************************************************
 * Local Read Only Data
 *************************************************************************************************/

static char *errorText [] = {
    "Identifier spelled before as %s",
    "Identifier %s redeclared, but spelled before as %s",
    "Identifier %s redeclared",
    "Identifier %s conflict with switch type, but spelled before as %s",
    "Identifier %s conflict with switch type",
    "Default label specified more than once",
    "Switch type is not of integer, char, boolean or enum type",
    "Default case label specified while type range is covered",
    "Not more than 2^32 enumeration elements are allowed",
    "Undeclared referenced declarator %s",
    "Undeclared referenced identifier %s",
    "Invalid constant type, %s expected",
    "Invalid constant, evaluated value is %s",
    "Enumeration constant does not match enumeration type",
    "Unary operator %s is applicable to %s types only",
    "Binary operator %s is applicable to %s types only",
    "Constant expression type does not match target type",
    "Binary operator %s requires consistent %s types",
    "Right hand size of << and >> operators must be in range 0..63",
    "Case label is not of integer, char, boolean or enum literal",
    "Key fields are only supported for struct type",
    "The referenced identifier %s is of invalid type in it's context",
    "Key can only be specified for struct and union types",
    "Type '%s' (defined in %s) unsupported",
    "Type '%s' of '%s' member is not supported for a key",
    "Illegal declarator reference. Declarator %s can not contain any '.' characters within this pragma scope."
};

/*************************************************************************************************
 * Local Functions and Prototypes
 *************************************************************************************************/

static int
idl_checkUnaryExpression(
    c_metaObject scope,
    c_type type,
    c_exprKind kind,
    c_constant constant,
    c_value *aggregated,
    idl_errorFunction rf);

static int
idl_checkBinaryExpression(
    c_metaObject scope,
    c_type type,
    c_exprKind kind,
    c_constant constantl,
    c_constant constantr,
    c_value *aggregated,
    idl_errorFunction rf);

static idl_constantClass
idl_checkClassFromType(
    c_type type);

static idl_constantClass
idl_checkClassFromConstant(
    c_operand operand);

static int
idl_checkEnumerationValue(
    c_enumeration enumeration,
    c_constant constant);

static int
idl_checkReferencedIdentifierInScope (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer [IDL_MAX_ERRORSIZE];
    c_metaObject mo;
    int result = 0;

    /* rule [5] */
    if (scope) {
        mo = c_metaObject(c_metaFindByName (scope, name, CQ_METAOBJECTS | CQ_CASEINSENSITIVE));
            if (mo) {
            if (strcmp(mo->name, name) != 0) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Spelling], mo->name);
                rf(errorBuffer);
            }
        } else {
            result++;
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UndeclaredIdentifier], name);
            rf(errorBuffer);
        }
    }
    return result;
}

static int
idl_checkNonDeclaratorDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer [IDL_MAX_ERRORSIZE];
    c_metaObject mo;
    int result = 0;

    mo = c_metaObject (c_metaFindByName (scope, name, CQ_METAOBJECTS | CQ_CASEINSENSITIVE | CQ_FIXEDSCOPE));
    if (mo && c_isFinal(mo)) {
        if (strcmp (mo->name, name) != 0) {
            result++;
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_RedeclarationSpelling], name, mo->name);
            rf(errorBuffer);
        } else {
            result++;
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Redeclaration], name);
            rf(errorBuffer);
        }
    }
    return result;
}

static int
idl_checkLiteral(
    c_type type,
    c_value value,
    idl_errorFunction rf)
{
    char errorBuffer [IDL_MAX_ERRORSIZE];
    int result = 0;

    switch (c_baseObject(type)->kind) {
        case M_COLLECTION:
            if (c_collectionType(type)->kind == C_STRING) {
            /* string */
            if (value.kind != V_STRING) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "string");
                rf(errorBuffer);
            }
            } else {
            /* unexpected collection kind */
            }
            break;
        case M_PRIMITIVE:
            switch (c_primitive(type)->kind) {
                case P_BOOLEAN:
                    if (value.kind != V_BOOLEAN) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "boolean");
                        rf(errorBuffer);
                    } else {
                if (value.is.Boolean > 1) {
                    /* value out of range */
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict], "FALSE or TRUE");
                    rf(errorBuffer);
                }
            }
                    break;
                case P_CHAR:
                    if (value.kind != V_CHAR) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "char");
                        rf(errorBuffer);
                    } else {
                /* value check is not required */
                    }
                    break;
                case P_OCTET:
                    if (value.kind != V_LONGLONG) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "octet");
                        rf(errorBuffer);
                    } else {
                if ((value.is.LongLong < 0) || (value.is.LongLong > 255)) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                             "not within range 0..255");
                    rf(errorBuffer);
                }
                    }
                    break;
                case P_SHORT:
                    if (value.kind != V_LONGLONG) {
                if (value.kind != V_OCTET) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict],
                        "short");
                    rf(errorBuffer);
                }
                    } else {
                if (value.is.LongLong < C_MIN_SHORT(LL)) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                        "more negative than short allows");
                    rf(errorBuffer);
                } else {
                    if (value.is.LongLong > C_MAX_SHORT(LL)) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                            "more positive than short allows");
                        rf(errorBuffer);
                    }
                }
                    }
                    break;
                case P_USHORT:
                    if (value.kind != V_LONGLONG) {
                if (value.kind != V_OCTET) {
                                result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict],
                        "unsigned short");
                            rf(errorBuffer);
                }
            } else {
                if (value.is.LongLong < (long long)C_MIN_USHORT(LL)) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                             "negative which unsigned short does not allow");
                    rf(errorBuffer);
                } else {
                    if (value.is.LongLong > (long long)C_MAX_USHORT(LL)) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                                 "more positive than unsigned short allows");
                        rf(errorBuffer);
                    }
                }
            }
                    break;
                case P_LONG:
                    if (value.kind != V_LONGLONG) {
                if (value.kind != V_OCTET) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict],
                             "long");
                            rf(errorBuffer);
                }
            } else {
                if (value.is.LongLong < C_MIN_LONG(LL)) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                             "more negative than long allows");
                    rf(errorBuffer);
                } else {
                    if (value.is.LongLong > (long long)C_MAX_LONG(LL)) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                                 "more positive than long allows");
                        rf(errorBuffer);
                    }
                }
            }
                    break;
                case P_ULONG:
                    if (value.kind != V_LONGLONG) {
                if (value.kind != V_OCTET) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict],
                             "unsigned long");
                            rf(errorBuffer);
                }
                    } else {
                if (value.is.LongLong < (long long)C_MIN_ULONG(LL)) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                             "negative which unsigned long does not allow");
                    rf(errorBuffer);
                } else {
                    if (value.is.LongLong > (long long)C_MAX_ULONG(LL)) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                                 "more positive than unsigned long allows");
                        rf(errorBuffer);
                    }
                }
                    }
                    break;
                case P_LONGLONG:
                    if (value.kind != V_LONGLONG) {
                if (value.kind != V_OCTET) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict],
                             "long long");
                            rf(errorBuffer);
                }
                    }
                    break;
                case P_ULONGLONG:
                    if (value.kind != V_LONGLONG) {
                if (value.kind != V_OCTET) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict],
                             "unsigned long long");
                            rf(errorBuffer);
                }
                    }
                    break;
                case P_FLOAT:
                    if (value.kind != V_DOUBLE) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "float");
                        rf(errorBuffer);
                    } else {
                if (value.is.Double < -C_MAX_FLOAT) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                             "more negative than float allows");
                    rf(errorBuffer);
                } else {
                    if (value.is.Double > C_MAX_FLOAT) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                                 "more positive than float allows");
                        rf(errorBuffer);
                    } else {
                        if ((value.is.Double > -C_MIN_FLOAT) &&
                            (value.is.Double < C_MIN_FLOAT) &&
                            (value.is.Double != 0.0)) {
                            result++;
                            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantRangeConflict],
                                     "smaller than float resolution allows");
                            rf(errorBuffer);
                        }
                    }
                }
                    }
                    break;
                case P_DOUBLE:
                    if (value.kind != V_DOUBLE) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "double");
                        rf(errorBuffer);
                    }
                    break;
                default:
                    /* unexpected literal kind */
                    break;
            }
            break;
        default:
            /* unexpected base object kind */
            break;
    }

    return result;
}

static int
get_expressionValue(
    c_metaObject scope,
    c_type type,
    c_operand constant,
    c_value *value,
    idl_errorFunction rf)
{
    int result = 0;

    if (c_baseObject(constant)->kind == M_LITERAL) {
        *value = c_literal(constant)->value;
    } else {
        if (c_baseObject(constant)->kind == M_EXPRESSION) {
            if (c_arraySize(c_expression(constant)->operands) == 1) {
                result = idl_checkUnaryExpression(
                             scope, type, c_expression(constant)->kind,
                             c_expression(constant)->operands[0], value, rf);
            } else {
                if (c_arraySize(c_expression(constant)->operands) == 2) {
                     result = idl_checkBinaryExpression(
                                  scope, type, c_expression(constant)->kind,
                                  c_expression(constant)->operands[0],
                                  c_expression(constant)->operands[1], value, rf);
                }
            }
        } else {
            if (c_baseObject(constant)->kind == M_CONSTANT) {
                result = get_expressionValue(scope, type, c_constant(constant)->operand, value, rf);
            } else {
                printf ("get_expressionValue: Unexpected constant expression %d\n", c_baseObject(constant)->kind);
            }
        }
    }
    return result;
}

static int
idl_checkUnaryExpression(
    c_metaObject scope,
    c_type type,
    c_exprKind kind,
    c_constant constant,
    c_value *aggregated,
    idl_errorFunction rf)
{
    char errorBuffer [IDL_MAX_ERRORSIZE];
    int result = 0;
    idl_constantClass cc;
    idl_constantClass tc;

    cc = idl_checkClassFromConstant(c_operand(constant));
    tc = idl_checkClassFromType(type);
    if ((cc != tc) && !((tc == idl_Octet) && (cc == idl_Integer))) {
        /* assignment of integer to octet is valid */
        rf(errorText[idl_ConstantExprMismatch]);
        result++;
    }
    switch (kind) {
    case E_PLUS:
        if (cc != idl_Float && cc != idl_Integer) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalUnaryOperator], "+", "float and integer");
            rf(errorBuffer);
            result++;
        } else {
            result = get_expressionValue(
                         scope, type, c_operand(constant), aggregated, rf);
            if (result == 0) {
                result = idl_checkLiteral(type, *aggregated, rf);
            }
        }
        break;
    case E_MINUS:
        if (cc != idl_Float && cc != idl_Integer) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalUnaryOperator], "-", "float and integer");
            rf(errorBuffer);
            result++;
        } else {
            result = get_expressionValue(
                         scope, type, c_operand(constant), aggregated, rf);
            if (result == 0) {
                if (cc == idl_Float) {
                    aggregated->is.Double = -aggregated->is.Double;
                } else {
                    aggregated->is.LongLong = -aggregated->is.LongLong;
                }
                result = idl_checkLiteral (type, *aggregated, rf);
            }
        }
        break;
    case E_NOT:
        if (cc != idl_Integer) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalUnaryOperator], "~", "integer");
            rf(errorBuffer);
            result++;
        } else {
            result = get_expressionValue(scope, type, c_operand(constant), aggregated, rf);
            if (result == 0) {
                aggregated->is.LongLong = ~aggregated->is.LongLong;
                result = idl_checkLiteral (type, *aggregated, rf);
            }
        }
        break;
    default:
        printf ("idl_checkUnaryExpression: Illegal unary operator %d\n", kind);
        break;
    }
    return result;
}

static int
idl_checkBinaryExpression(
    c_metaObject scope,
    c_type type,
    c_exprKind kind,
    c_constant constantl,
    c_constant constantr,
    c_value *aggregated,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    int result = 0;
    idl_constantClass ccl;
    idl_constantClass ccr;
    idl_constantClass tc;
    c_value valuel;
    c_value valuer;

    ccl = idl_checkClassFromConstant(c_operand(constantl));
    ccr = idl_checkClassFromConstant(c_operand(constantr));
    tc = idl_checkClassFromType(type);

    switch (kind) {
    case E_MUL:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "*", "float or integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Float || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            if (tc == idl_Integer || tc == idl_Octet) {
                                aggregated->kind = V_LONGLONG;
                                aggregated->is.LongLong = valuel.is.LongLong * valuer.is.LongLong;
                            } else {
                                aggregated->kind = V_DOUBLE;
                                aggregated->is.Double = valuel.is.Double * valuer.is.Double;
                            }
                            result = idl_checkLiteral (type, *aggregated, rf);
                        }
                    }
                    } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "*", "float and integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_DIV:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "/", "float or integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Float || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            if (tc == idl_Integer || tc == idl_Octet) {
                                aggregated->kind = V_LONGLONG;
                                aggregated->is.LongLong = valuel.is.LongLong / valuer.is.LongLong;
                            } else {
                                aggregated->kind = V_DOUBLE;
                                aggregated->is.Double = valuel.is.Double / valuer.is.Double;
                            }
                            result = idl_checkLiteral (type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "/", "float and integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_PLUS:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "+", "float or integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Float || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            if (tc == idl_Integer || tc == idl_Octet) {
                                aggregated->kind = V_LONGLONG;
                                aggregated->is.LongLong = valuel.is.LongLong + valuer.is.LongLong;
                            } else {
                                aggregated->kind = V_DOUBLE;
                                aggregated->is.Double = valuel.is.Double + valuer.is.Double;
                            }
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "+", "float and integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_MINUS:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "-", "float or integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Float || tc == idl_Octet) {
                    result = get_expressionValue (scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue (scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            if (tc == idl_Integer || tc == idl_Octet) {
                                aggregated->kind = V_LONGLONG;
                                aggregated->is.LongLong = valuel.is.LongLong - valuer.is.LongLong;
                            } else {
                                aggregated->kind = V_DOUBLE;
                                aggregated->is.Double = valuel.is.Double - valuer.is.Double;
                            }
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "-", "float and integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_MOD:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "%", "integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            aggregated->kind = V_LONGLONG;
                            aggregated->is.LongLong = valuel.is.LongLong % valuer.is.LongLong;
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "%", "integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_AND:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "&", "integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            aggregated->kind = V_LONGLONG;
                            aggregated->is.LongLong = valuel.is.LongLong & valuer.is.LongLong;
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "&", "integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_OR:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "|", "integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue (scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            aggregated->kind = V_LONGLONG;
                            aggregated->is.LongLong = valuel.is.LongLong | valuer.is.LongLong;
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "|", "integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_XOR:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "^", "integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (result == 0) {
                            aggregated->kind = V_LONGLONG;
                            aggregated->is.LongLong = valuel.is.LongLong ^ valuer.is.LongLong;
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "^", "integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_SHIFTLEFT:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], "<<", "integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (valuer.is.LongLong > 63LL || valuer.is.LongLong < 0LL) {
                            result++;
                            rf(errorText[idl_ShiftRangeConflict]);
                        } else if (result == 0) {
                            aggregated->kind = V_LONGLONG;
                            aggregated->is.LongLong = valuel.is.LongLong << valuer.is.LongLong;
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], "<<", "integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    case E_SHIFTRIGHT:
        if (ccl != ccr) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantExprConflict], ">>", "integer");
            rf(errorBuffer);
            result++;
        } else {
            if ((ccl != tc) && !((tc == idl_Octet) && (ccl == idl_Integer))) {
                /* assignment of integer to octet is valid */
                rf(errorText[idl_ConstantExprMismatch]);
                result++;
            } else {
                if (tc == idl_Integer || tc == idl_Octet) {
                    result = get_expressionValue(scope, type, c_operand(constantl), &valuel, rf);
                    if (result == 0) {
                        result = get_expressionValue(scope, type, c_operand(constantr), &valuer, rf);
                        if (valuer.is.LongLong > 63LL || valuer.is.LongLong < 0LL) {
                            result++;
                            rf(errorText[idl_ShiftRangeConflict]);
                        } else if (result == 0) {
                            aggregated->kind = V_LONGLONG;
                            aggregated->is.LongLong = (long long)(((unsigned long long)valuel.is.LongLong) >> valuer.is.LongLong);
                            result = idl_checkLiteral(type, *aggregated, rf);
                        }
                    }
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalBinaryOperator], ">>", "integer");
                    rf(errorBuffer);
                    result++;
                }
            }
        }
        break;
    default:
        printf("idl_checkBinaryExpression: Illegal binary operator %d\n", kind);
        break;
    }

    return result;
}

static idl_constantClass
idl_checkClassFromType (
    c_type type)
{
    idl_constantClass cc = idl_Other;

    if (c_baseObject(type)->kind == M_PRIMITIVE) {
        switch (c_primitive(type)->kind) {
        case P_BOOLEAN:
            cc = idl_Boolean;
        break;
        case P_CHAR:
            cc = idl_Char;
        break;
        case P_OCTET:
            cc = idl_Octet;
        break;
        case P_SHORT:
        case P_USHORT:
        case P_LONG:
        case P_ULONG:
        case P_LONGLONG:
        case P_ULONGLONG:
            cc = idl_Integer;
        break;
        case P_FLOAT:
        case P_DOUBLE:
            cc = idl_Float;
        break;
        default:
            cc = idl_Other;
        }
    } else {
        if (c_baseObject(type)->kind == M_COLLECTION) {
            if (c_collectionType(type)->kind == C_STRING) {
                cc = idl_String;
            }
        }
    }
    return cc;
}

static idl_constantClass
idl_checkClassFromConstant(
    c_operand operand)
{
    idl_constantClass cc = idl_Other;

    if (c_baseObject(operand)->kind == M_PRIMITIVE) {
        cc = idl_checkClassFromType (c_type(operand));
    } else if (c_baseObject(operand)->kind == M_EXPRESSION) {
        cc = idl_checkClassFromConstant (c_expression(operand)->operands[0]);
    } else if (c_baseObject(operand)->kind == M_CONSTANT) {
        cc = idl_checkClassFromConstant (c_constant(operand)->operand);
    } else if (c_baseObject(operand)->kind == M_LITERAL) {
        switch (c_literal(operand)->value.kind) {
        case V_BOOLEAN:
            cc = idl_Boolean;
            break;
        case V_OCTET:
            cc = idl_Octet;
            break;
        case V_SHORT:
        case V_LONG:
        case V_LONGLONG:
        case V_USHORT:
        case V_ULONG:
        case V_ULONGLONG:
            cc = idl_Integer;
            break;
        case V_FLOAT:
        case V_DOUBLE:
            cc = idl_Float;
            break;
        case V_CHAR:
            cc = idl_Char;
            break;
        case V_STRING:
            cc = idl_String;
            break;
        default:
            cc = idl_Other;
        }
    } else if (c_baseObject(operand)->kind == M_COLLECTION) {
        if (c_collectionType(operand)->kind == C_STRING) {
            cc = idl_String;
        }
    }
    return cc;
}

static int
idl_checkEnumerationValue(
    c_enumeration enumeration,
    c_constant constant)
{
    int result = 1;
    int i;

    if (c_baseObject(constant->operand)->kind == M_CONSTANT) {
        result = idl_checkEnumerationValue(enumeration, c_constant(constant->operand));
    } else {
        for (i = 0; i < c_arraySize(enumeration->elements); i++) {
            if (c_constant(enumeration->elements[i]) == constant) {
                return 0;
            }
        }
    }
    return result;
}

static char *
objectName(
    c_baseObject o)
{
    if (CQ_KIND_IN_MASK (o, CQ_METAOBJECTS)) {
        return c_metaObject(o)->name;
    } else {
        if (CQ_KIND_IN_MASK (o, CQ_SPECIFIERS)) {
            return c_specifier(o)->name;
        }
    }
    return "";
}

#if 0
static void
checkUnsupportedTypeUsage(
   c_metaObject metaObject,
   c_metaWalkActionArg arg)
{
    struct unsupportedArg *a = (struct unsupportedArg *)arg;
    int i;
    c_metaObject mo;

    if ((metaObject->name != NULL) &&
        (strncmp(IDL_UNSUP_PREFIX, metaObject->name, strlen(IDL_UNSUP_PREFIX)) == 0)) {
        a->unsupported = TRUE;
        a->typeName = idl_unsupportedTypeActualName(metaObject->name);
        return;
    }
    switch (c_baseObject(metaObject)->kind) {
    case M_COLLECTION:
        checkUnsupportedTypeUsage(c_metaObject(c_collectionType(metaObject)->subType), arg);
    break;
    case M_CONSTANT:
        checkUnsupportedTypeUsage(c_metaObject(c_constant(metaObject)->type), arg);
    break;
    case M_MEMBER:
        checkUnsupportedTypeUsage(c_metaObject(c_specifier(metaObject)->type), arg);
    break;
    case M_TYPEDEF:
        mo = c_metaObject(c_typeActualType(c_type(metaObject)));
        checkUnsupportedTypeUsage(mo, arg);
    break;
    case M_UNION:
        i = 0;
        while ((i < c_arraySize(c_union(metaObject)->cases)) &&
               (a->unsupported == FALSE)) {
            checkUnsupportedTypeUsage(c_metaObject(c_union(metaObject)->cases[i]), arg);
            i++;
        }
    break;
    case M_STRUCTURE:
        i = 0;
        while ((i < c_arraySize(c_structure(metaObject)->members)) &&
               (a->unsupported == FALSE)) {
            checkUnsupportedTypeUsage(c_metaObject(c_structure(metaObject)->members[i]), arg);
            i++;
        }
    break;
    case M_UNIONCASE:
        checkUnsupportedTypeUsage(c_metaObject(c_specifier(metaObject)->type), arg);
    break;
    /* supported by default */
    case M_ENUMERATION:
    case M_LITERAL:
    case M_PRIMITIVE:
        /* do nothing */
    break;
    /* unsupported by default */
    case M_UNDEFINED:
    case M_ATTRIBUTE:
    case M_CLASS:
    case M_CONSTOPERAND:
    case M_EXCEPTION:
    case M_EXPRESSION:
    case M_INTERFACE:
    case M_MODULE:
    case M_OPERATION:
    case M_PARAMETER:
    case M_RELATION:
    case M_BASE:
    case M_COUNT:
    default:
        a->unsupported = TRUE;
        a->typeName = metaObject->name;
    break;
    }
}
#else

static ut_result
checkRecursion(
    void *o,
    void *arg)
{
    if (o == arg) {
        return UT_RESULT_WALK_ABORTED;
    } else {
        return UT_RESULT_OK;
    }
}

#define STACK_INC (256)
static void
checkUnsupportedTypeUsage(
   c_metaObject metaObject,
   c_metaWalkActionArg arg)
{
    struct unsupportedArg *a = (struct unsupportedArg *)arg;
    int i;
    c_metaObject mo;
    ut_result utr;

    /* detect recursion */
    utr = ut_stackWalk(a->context, checkRecursion, metaObject);
    if (utr == UT_RESULT_OK) {
        if ((metaObject->name != NULL) &&
            (strncmp(IDL_UNSUP_PREFIX, metaObject->name, strlen(IDL_UNSUP_PREFIX)) == 0)) {
            a->unsupported = TRUE;
            a->typeName = idl_unsupportedTypeActualName(metaObject->name);
        } else {
            switch (c_baseObject(metaObject)->kind) {
            case M_COLLECTION:
                checkUnsupportedTypeUsage(c_metaObject(c_collectionType(metaObject)->subType), arg);
            break;
            case M_CONSTANT:
                checkUnsupportedTypeUsage(c_metaObject(c_constant(metaObject)->type), arg);
            break;
            case M_MEMBER:
                checkUnsupportedTypeUsage(c_metaObject(c_specifier(metaObject)->type), arg);
            break;
            case M_TYPEDEF:
                checkUnsupportedTypeUsage(c_metaObject(c_typeActualType(c_type(metaObject))), arg);
            break;
            case M_UNION:
                utr = ut_stackPush(a->context, (void *)metaObject);
                i = 0;
                while (i < c_arraySize(c_union(metaObject)->cases)) {
                    checkUnsupportedTypeUsage(c_metaObject(c_union(metaObject)->cases[i]), arg);
                    i++;
                }
                mo = ut_stackPop(a->context);
                assert(mo == metaObject);
            break;
            case M_STRUCTURE:
                utr = ut_stackPush(a->context, (void *)metaObject);
                i = 0;
                while (i < c_arraySize(c_structure(metaObject)->members)) {
                    checkUnsupportedTypeUsage(c_metaObject(c_structure(metaObject)->members[i]), arg);
                    i++;
                }
                mo = ut_stackPop(a->context);
                assert(mo == metaObject);
            break;
            case M_UNIONCASE:
                checkUnsupportedTypeUsage(c_metaObject(c_specifier(metaObject)->type), arg);
            break;
                /* supported by default */
            case M_ENUMERATION:
            case M_LITERAL:
            case M_PRIMITIVE:
                /* do nothing */
            break;
                /* unsupported by default */
            case M_UNDEFINED:
            case M_ATTRIBUTE:
            case M_CLASS:
            case M_CONSTOPERAND:
            case M_EXCEPTION:
            case M_EXPRESSION:
            case M_INTERFACE:
            case M_MODULE:
            case M_OPERATION:
            case M_PARAMETER:
            case M_RELATION:
            case M_BASE:
            case M_COUNT:
            default:
                a->unsupported = TRUE;
                a->typeName = metaObject->name;
            break;
            }
        }
    } /* else recursion detected, so skip this metaobject */
}
#endif
/*************************************************************************************************
 * Global Functions
 *************************************************************************************************/

int
idl_checkReferencedIdentifier(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    c_metaObject mo;
    int result = 0;

    /* rule [5] */
    if (scope) {
        mo = c_metaObject(c_metaFindByName (scope, name, CQ_METAOBJECTS | CQ_CASEINSENSITIVE));
        if (mo) {
            if (strcmp (mo->name, name) != 0) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Spelling], mo->name);
                rf(errorBuffer);
            } else {
                /* Check if already defined in enclosing scope with different case */
                scope = scope->definedIn;
                if (scope) {
                    result = idl_checkReferencedIdentifier(scope, name, rf);
                }
            }
        }
    }
    return result;
}

int
idl_checkModuleDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    c_metaObject mo;
    int result = 0;

    /* rule [5] */
    if (scope) {
        mo = c_metaObject(c_metaFindByName (scope, name, CQ_MODULE | CQ_CASEINSENSITIVE | CQ_FIXEDSCOPE));
        if (mo) {
            if (strcmp(mo->name, name) != 0) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Spelling], mo->name);
                rf(errorBuffer);
            }
        }
    }
    return result;
}

int
idl_checkStructDeclaratorDefinition(
    c_metaObject scope,
    c_iter iter,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    int result = 0;
    int i, j;
    char *namei;
    char *namej;
    c_type type;

    /* rule [21] */
    i = c_iterLength(iter);
    j = i;
    for (i = 0; i < c_iterLength(iter); i++) {
        for (j = i+1; j < c_iterLength(iter); j++) {
            namei = c_specifier(c_iterObject(iter, i))->name;
            namej = c_specifier(c_iterObject(iter, j))->name;
            if (os_strcasecmp(namei, namej) == 0) {
                if (strcmp(namei, namej) != 0) {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_RedeclarationSpelling], namej, namei);
                } else {
                    snprintf (errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Redeclaration], namei);
                }
                rf(errorBuffer);
                result++;
               }
        }
        type = c_specifier(c_iterObject(iter, i))->type;
        if (!CQ_KIND_IN_MASK(c_metaObject(type), CQ_TYPEOBJECTS)) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_InvalidKind],
            objectName(c_baseObject(type)));
            rf(errorBuffer);
            result++;
        }
    }
    return result;
}

int
idl_checkUnionDeclaratorDefinition(
    c_metaObject scope,
    c_type switchType,
    c_iter iter,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    int result = 0;
    int i, j;
    char *namei;
    char *namej;
    c_type type;

    /* rule [26] */
    for (i = 0; i < c_iterLength(iter); i++) {
        for (j = i+1; j < c_iterLength(iter); j++) {
            namei = c_specifier(c_iterObject(iter, i))->name;
            namej = c_specifier(c_iterObject(iter, j))->name;
            if (os_strcasecmp(namei, namej) == 0) {
                if (strcmp(namei, namej) != 0) {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_RedeclarationSpelling], namej, namei);
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Redeclaration], namei);
                }
                rf(errorBuffer);
                result++;
            }
        }
        type = c_specifier(c_iterObject(iter, i))->type;
        if (!CQ_KIND_IN_MASK(c_metaObject(type), CQ_TYPEOBJECTS)) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_InvalidKind],
            objectName(c_baseObject(type)));
            rf(errorBuffer);
            result++;
        }
    }
    /* rule [27] */
    if (c_baseObject(switchType)->kind == M_ENUMERATION) {
        /* check if identifier is conflicting with enumeration identifier */
        for (i = 0; i < c_iterLength(iter); i++) {
            namei = c_specifier(c_iterObject(iter, i))->name;
            if (os_strcasecmp(namei, c_metaObject(switchType)->name) == 0) {
                if (strcmp(namei, c_metaObject(switchType)->name) != 0) {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_SwitchConflictSpelling],
                             namei, c_metaObject(switchType)->name);
                } else {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_SwitchConflict], namei);
                }
                rf (errorBuffer);
                result++;
            }
        }
    }
    return result;
}

int
idl_checkUnionCaseDefinition(
    c_metaObject scope,
    c_type switchType,
    c_iter iter,
    idl_errorFunction rf)
{
    int result = 0;
    int i;
    c_unionCase uc;
    unsigned int defaultLabelCount = 0;
    c_bool switchTypeError = TRUE;
    unsigned long long rangeMax;
    unsigned long long labelCount = 0L;

    /* rule [23] */
    i = c_iterLength(iter);
    for (i = 0; i < c_iterLength(iter); i++) {
        uc = c_unionCase(c_iterObject(iter, i));
        if (c_arraySize(uc->labels) == 0) {
            defaultLabelCount++;
        } else {
            labelCount += c_arraySize(uc->labels);
        }
    }
    if (defaultLabelCount > 1) {
        rf(errorText[idl_DefaultLabel]);
        result++;
    }
    switch (c_baseObject(c_typeActualType(switchType))->kind) {
    case M_PRIMITIVE:
        /* Check primitive kind */
        switch (c_primitive(c_typeActualType(switchType))->kind) {
        case P_BOOLEAN:
            switchTypeError = FALSE;
            rangeMax = 2ULL-1ULL;
            break;
        case P_CHAR:
        case P_WCHAR:
            switchTypeError = FALSE;
            rangeMax = 256ULL-1ULL;
            break;
        case P_SHORT:
        case P_USHORT:
            switchTypeError = FALSE;
            rangeMax = (2ULL^16)-1ULL;
            break;
        case P_LONG:
        case P_ULONG:
            switchTypeError = FALSE;
            rangeMax = (2ULL^32)-1ULL;
            break;
        case P_LONGLONG:
        case P_ULONGLONG:
            /* no action, this is OK */
            switchTypeError = FALSE;
            rangeMax = 0xffffffffffffffffLL;
            break;
        default:
            /* Error */
            switchTypeError = TRUE;
        }
    break;
    case M_ENUMERATION:
        /* no action, this is OK */
        switchTypeError = FALSE;
        rangeMax = (unsigned long long)c_arraySize(c_enumeration(c_typeActualType(switchType))->elements)-1ULL;
    break;
    default:
        /* Error */
        switchTypeError = TRUE;
    }
    if (switchTypeError) {
        /* rule [24] */
        rf(errorText[idl_SwitchType]);
        result++;
    } else {
        /* rule [28] */
        if ((labelCount > rangeMax) && (defaultLabelCount > 0ULL)) {
            rf(errorText[idl_NoDefault]);
            result++;
        }
    }
    return result;
}

int
idl_checkConstantDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    /* rule [6] */
    return idl_checkNonDeclaratorDefinition(scope, name, rf);
}

int
idl_checkConstantDeclaration(
    c_metaObject scope,
    c_type type,
    c_constant constant,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    int result = 0;
    c_operand operand = constant->operand;
    c_value value;
    int longRef = 0;

    if (type == NULL) {
        longRef = 1;
        type = c_type(c_metaResolveType(scope, "c_ulong"));
    }
    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        if (c_baseObject(operand)->kind == M_LITERAL) {
            if (c_literal(operand)->value.kind != V_STRING) {
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "string");
                rf(errorBuffer);
                result++;
            }
        } else {
            if (c_baseObject(operand)->kind == M_CONSTANT) {
                result = idl_checkConstantDeclaration(scope, type, c_constant(operand), rf);
            } else {
                if (c_baseObject(operand)->kind == M_EXPRESSION) {
                    if (c_arraySize(c_expression(operand)->operands) == 1) {
                        result = idl_checkUnaryExpression(
                                    scope, type, c_expression(operand)->kind,
                                    c_expression(operand)->operands[0], &value, rf);
                    } else if (c_arraySize(c_expression(operand)->operands) == 2) {
                        result = idl_checkBinaryExpression(
                                    scope, type, c_expression(operand)->kind,
                                    c_expression(operand)->operands[0],
                                    c_expression(operand)->operands[1], &value, rf);
                    }
                } else {
                    printf ("idl_checkConstantDeclaration: Illegal string constant value type\n");
                }
            }
        }
        break;
    case M_ENUMERATION:
        if (c_baseObject(operand)->kind == M_CONSTANT) {
            /* rule [16] */
            /* Check if correct enumeration element is refered */
            result = idl_checkEnumerationValue(c_enumeration(type), c_constant(operand));
            if (result != 0) {
                rf(errorText[idl_EnumerationElement]);
            }
        } else {
            rf(errorText[idl_EnumerationElement]);
            result++;
        }
    break;
    case M_PRIMITIVE:
        /* Check if literal matches the target type */
        /* Check if literal value matches the range of the target type */
        if (c_baseObject(operand)->kind == M_LITERAL) {
            result = idl_checkLiteral(type, c_literal(operand)->value, rf);
        } else {
            if (c_baseObject(operand)->kind == M_CONSTANT) {
                result = idl_checkConstantDeclaration(scope, type, c_constant(operand), rf);
            } else {
                if (c_baseObject(operand)->kind == M_EXPRESSION) {
                    if (c_arraySize(c_expression(operand)->operands) == 1) {
                        result = idl_checkUnaryExpression(
                                    scope, type, c_expression(operand)->kind,
                                    c_expression(operand)->operands[0], &value, rf);
                    } else {
                        if (c_arraySize(c_expression(operand)->operands) == 2) {
                            result = idl_checkBinaryExpression(
                                        scope, type, c_expression(operand)->kind,
                                        c_expression(operand)->operands[0],
                                        c_expression(operand)->operands[1], &value, rf);
                        }
                    }
                } else {
                    printf("idl_checkConstantDeclaration: Illegal primitive constant value type\n");
                }
            }
        }
    break;
    case M_TYPEDEF:
        result = idl_checkConstantDeclaration(scope, c_typeActualType(type), constant, rf);
        break;
    default:
        printf ("idl_checkConstantDeclaration: Unexpected type %d\n", c_baseObject(type)->kind);
    }
    if (longRef) {
        c_free(type);
    }
    return result;
}

int
idl_checkTypeDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    /* rule [6] */
    return idl_checkNonDeclaratorDefinition(scope, name, rf);
}

int
idl_checkTypeReference(
    c_metaObject scope,
    c_type type,
    idl_errorFunction rf)
{
    int result = 0;
    char errorBuffer[IDL_MAX_ERRORSIZE];

    /* rule [6] */
    if (type) {
        if (!CQ_KIND_IN_MASK(type, CQ_TYPEOBJECTS)) {
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_InvalidKind], objectName(c_baseObject(type)));
            rf(errorBuffer);
            result++;
        }
    }
    return result;
}

int
idl_checkEnumerationElementDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    /* rule [6] */
    return idl_checkNonDeclaratorDefinition(scope, name, rf);
}

int
idl_checkEnumerationElementCount(
    c_iter iter,
    idl_errorFunction rf)
{
    int result = 0;

    /* rule [29] */
    /* Note: because of the rtpe of c_iterLength, no check on (2^32)-1 is possible */
    if ((unsigned int)c_iterLength(iter) == 0x7fffffff) {
        rf(errorText[idl_EnumerationLimit]);
        result++;
    }
    return result;
}

int
idl_checkInterfaceDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    c_metaObject mo;
    int result = 0;

    /* rule [6] */
    /* Ignore Interface definitions because they can be forward declarated without any method to find out if the meta */
    /* object is still a forward declaration or is already the real declaration                                       */
    mo = c_metaObject(c_metaFindByName(scope, name, (CQ_METAOBJECTS | CQ_CASEINSENSITIVE | CQ_FIXEDSCOPE) & (~CQ_INTERFACE)));
    if (mo) {
        if (strcmp(mo->name, name) != 0) {
            result++;
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_RedeclarationSpelling], name, mo->name);
            rf(errorBuffer);
        } else {
            result++;
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Redeclaration], name);
            rf(errorBuffer);
        }
    }
    return result;
}

int
idl_checkExceptionDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    /* rule [6] */
    return idl_checkNonDeclaratorDefinition(scope, name, rf);
}

int
idl_checkOperationDefinition(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    /* rule [6] */
    return idl_checkNonDeclaratorDefinition(scope, name, rf);
}

int
idl_checkKeyListTypeName(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    c_metaObject mo;
    int result = 0;
    struct unsupportedArg arg;

    /* rule [34],[38] */
    if (scope) {
        mo = c_metaObject(c_metaFindByName(scope, name, CQ_METAOBJECTS | CQ_CASEINSENSITIVE));
        if (mo) {
            if (strcmp(mo->name, name) != 0) {
                result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Spelling], mo->name);
                rf(errorBuffer);
            } else {
                /* The referenced identifier can also be a typedef of a structure or union.
                 * So first determine the actual type before checking whether it is a
                 * structure or union.
                 */
                mo = c_metaObject(c_typeActualType(c_type(mo)));
                if (!CQ_KIND_IN_MASK(mo, CQ_UNION | CQ_STRUCTURE)) {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalKeyType]);
                    rf(errorBuffer);
                    } else {
                    arg.unsupported = FALSE;
                    arg.typeName = NULL;
                    arg.context = ut_stackNew(256);
                    checkUnsupportedTypeUsage(mo, &arg);
                    if (arg.unsupported == TRUE) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1,
                            errorText[idl_UnsupportedType], arg.typeName, name);
                        rf(errorBuffer);
                    }
                    ut_stackFree(arg.context);
                }
            }
        } else {
            result++;
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UndeclaredIdentifier], name);
            rf(errorBuffer);
           }
    }
    return result;
}

int
idl_checkSimpleFieldName(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    c_iter fields = NULL;
    c_specifier sp;
    c_metaObject keyMetaObj;
    char *fieldName;
    int result = 0;

    /* simple field name consists of <field>, not [<field>.]*<field> like for keylist. */

    if (c_baseObject(c_typeActualType(c_type(scope)))->kind != M_STRUCTURE) {
        rf(errorText[idl_IllegalKeyFields]);
        result++;
    } else {
        fields = c_splitString(name, ".");
        if(c_iterLength(fields) > 1)
        {
            result++;
            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalFieldName], name);
            rf(errorBuffer);
        } else
        {
            fieldName = c_iterTakeFirst(fields);
            if(strlen(fieldName) > 1 && fieldName[0] == '!')
            {
                fieldName = &(fieldName[1]);
            }
            /* find specificer (sp) corresponding to keylist field name */
            keyMetaObj = scope;
            while (fieldName != NULL) {
                if (keyMetaObj) {
                    sp = c_specifier(c_metaFindByName(keyMetaObj, fieldName, CQ_FIXEDSCOPE | CQ_MEMBER | CQ_CASEINSENSITIVE));
                    if (sp) {
                        if (strcmp(sp->name, fieldName) != 0) {
                            result++;
                            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Spelling], fieldName);
                            rf(errorBuffer);
                        }
                        keyMetaObj = c_metaObject(sp->type);
                    } else {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UndeclaredDeclarator], fieldName);
                        rf(errorBuffer);
                        keyMetaObj = NULL;
                    }
                }
                fieldName = c_iterTakeFirst(fields);
            }
            c_iterFree(fields);
        }
    }
    return result;
}

int
idl_checkKeyListFieldName(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    c_iter fields = NULL;
    c_specifier sp;
    c_metaObject keyMetaObj;
    char *fieldName;
    c_type type, subtype;
    c_collectionType collType;
    c_primitive primitiveType;
    int result = 0;

    /* Keylist field name consists of [<field>.]*<field> */

    if (c_baseObject(c_typeActualType(c_type(scope)))->kind != M_STRUCTURE) {
        rf(errorText[idl_IllegalKeyFields]);
        result++;
    } else {
        fields = c_splitString(name, ".");
        fieldName = c_iterTakeFirst(fields);
        /* find specificer (sp) corresponding to keylist field name */
        keyMetaObj = scope;
        while (fieldName != NULL) {
            if (keyMetaObj) {
                sp = c_specifier(c_metaFindByName(keyMetaObj, fieldName, CQ_FIXEDSCOPE | CQ_MEMBER | CQ_CASEINSENSITIVE));
                if (sp) {
                    if (strcmp(sp->name, fieldName) != 0) {
                        result++;
                        snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_Spelling], fieldName);
                        rf(errorBuffer);
                    }
                    keyMetaObj = c_metaObject(sp->type);
                } else {
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UndeclaredDeclarator], fieldName);
                    rf(errorBuffer);
                    keyMetaObj = NULL;
                }
            }
            fieldName = c_iterTakeFirst(fields);
        }

        if (keyMetaObj != NULL) {
            /* Now keyMetaObj contains the type of key. But it can be a typedef.
             * So first determine the actual type.
             */
            type = c_typeActualType(c_type(keyMetaObj));
            /* Check if type is acceptable for a key.
             * Enum and primitive are acceptable
             */
            if (!CQ_KIND_IN_MASK(type, CQ_ENUMERATION | CQ_PRIMITIVE)) {
                /* only some M_COLLECTION types are acceptable */
                if (c_baseObject(type)->kind == M_COLLECTION) {
                    collType = c_collectionType(type);
                    /* string or bounded string are acceptable collections */
                    if (collType->kind != C_STRING) {
                        /* now, char array and long array for builtin topics remains acceptable */
                        subtype = c_typeActualType(collType->subType);
                        if (collType->kind == C_ARRAY &&
                            c_baseObject(subtype)->kind == M_PRIMITIVE)
                        {
                            primitiveType = c_primitive(subtype);
                            if (primitiveType->kind != P_LONG &&
                                primitiveType->kind != P_CHAR)
                            {
                                result++;
                                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UnsupportedKeyType], keyMetaObj->name, name);
                                rf(errorBuffer);
                            }
                            else if (primitiveType->kind == P_LONG) {
                                /* check that type is 1 of the 4 builtin topics:
                                 *   DDS.ParticipantBuiltinTopicData
                                 *   DDS.TopicBuiltinTopicData
                                 *   DDS.PublicationBuiltinTopicData
                                 *   DDS.SubscriptionBuiltinTopicData
                                 */
                                if (!scope->definedIn                                            /* if topic has no parent */
                                    || scope->definedIn->name == 0                               /* if topic parent has no name */
                                    || c_baseObject(scope->definedIn)->kind != M_MODULE          /* if topic parent is not a module */
                                    || strcmp(scope->definedIn->name, "DDS") != 0                /* if topic parent is not DDS module */
                                    || !scope->definedIn->definedIn                              /* if DDS module has no parent (DDS module should have root element as parent) */
                                    || scope->definedIn->definedIn->definedIn                    /* if DDS module has a grand-parent (meaning that DDS module is not in root element) */
                                    || (  strcmp(scope->name, "ParticipantBuiltinTopicData") != 0   /* if topic is not DDS.ParticipantBuiltinTopicData */
                                       && strcmp(scope->name, "TopicBuiltinTopicData") != 0         /* if topic is not DDS.TopicBuiltinTopicData */
                                       && strcmp(scope->name, "PublicationBuiltinTopicData") != 0   /* if topic is not DDS.PublicationBuiltinTopicData */
                                       && strcmp(scope->name, "SubscriptionBuiltinTopicData") != 0  /* if topic is not DDS.SubscriptionBuiltinTopicData */
                                       )
                                    )
                                {
                                    result++;
                                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UnsupportedKeyType], keyMetaObj->name, name);
                                    rf(errorBuffer);
                                }
                            }
                        } else {
                            /* not array or non-primitive array */
                            result++;
                            snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UnsupportedKeyType], keyMetaObj->name, name);
                            rf(errorBuffer);
                        }
                    }
                } else {
                    /* not a collection */
                    result++;
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UnsupportedKeyType], keyMetaObj->name, name);
                    rf(errorBuffer);
                }
            }
        }
    }
    c_iterFree(fields);
    return result;
}

int
idl_checkConstantOperand(
    c_metaObject scope,
    c_type type,
    c_operand operand,
    idl_errorFunction rf)
{
    char errorBuffer[IDL_MAX_ERRORSIZE];
    int result = 0;
    c_value value;

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        if (c_baseObject(operand)->kind == M_LITERAL) {
            if (c_literal(operand)->value.kind != V_STRING) {
                snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_ConstantTypeConflict], "string");
                rf(errorBuffer);
                result++;
            }
        } else {
            if (c_baseObject(operand)->kind == M_CONSTANT) {
                result = idl_checkConstantDeclaration(scope, type, c_constant(operand), rf);
            } else {
                printf ("idl_checkConstantDeclaration: Illegal string constant value type\n");
            }
        }
        break;
    case M_ENUMERATION:
        if (c_baseObject(operand)->kind == M_CONSTANT) {
            /* rule [16] */
            /* Check if correct enumeration element is refered */
            result = idl_checkEnumerationValue(c_enumeration(type), c_constant(operand));
            if (result != 0) {
                rf (errorText[idl_EnumerationElement]);
            }
        } else {
            rf (errorText[idl_EnumerationElement]);
            result++;
            }
    break;
    case M_PRIMITIVE:
        /* Check if literal matches the target type */
        /* Check if literal value matches the range of the target type */
        if (c_baseObject(operand)->kind == M_LITERAL) {
            result = idl_checkLiteral(type, c_literal(operand)->value, rf);
        } else {
            if (c_baseObject(operand)->kind == M_CONSTANT) {
                result = idl_checkConstantDeclaration (scope, type, c_constant(operand), rf);
            } else {
                if (c_baseObject(operand)->kind == M_EXPRESSION) {
                    if (c_arraySize(c_expression(operand)->operands) == 1) {
                        result = idl_checkUnaryExpression(
                                    scope, type, c_expression(operand)->kind,
                                    c_expression(operand)->operands[0], &value, rf);
                    } else if (c_arraySize(c_expression(operand)->operands) == 2) {
                        result = idl_checkBinaryExpression(
                                    scope, type, c_expression(operand)->kind,
                                    c_expression(operand)->operands[0],
                                    c_expression(operand)->operands[1], &value, rf);
                    }
                } else {
                    printf("idl_checkConstantDeclaration: Illegal primitive constant value type\n");
                }
            }
        }
    break;
    case M_TYPEDEF:
        result = idl_checkConstantOperand(scope, c_typeActualType(type), operand, rf);
        break;
    default:
        printf("idl_checkConstantOperand: Unexpected type %d\n", c_baseObject(type)->kind);
    }

    return result;
}

int
idl_checkPositiveInt(
    c_metaObject scope,
    c_operand operand,
    idl_errorFunction rf)
{
    int result = 0;
    c_baseObject ull;

    ull = c_baseObject(c_metaResolve(scope, "c_ulong"));
    result = idl_checkConstantOperand(scope, c_type(ull), operand, rf);
    c_free(ull);

    return result;
}
