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
#include "c_metabase.h"

typedef int (*idl_errorFunction) (char *text);

/*
 * Check if a referenced identifier is visible in the identified scope.
 * It must be a metaObject and the scan is case insensitive.
 * It scans metaObjects and case insensitive.
 *
 * returns 0 if referenced identifier is found.
 * returns >0 if referenced identifier is not found
 *   && problem is reported to error channel.
 */
int
idl_checkReferencedIdentifier (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the identifier of a constant definition is unique
 * in the identified scope (no scan in a higher level scope).
 * It scans metaObjects and case insensitive.
 *
 * returns 0 if the identifier is unique.
 * returns >0 if the identifier is not unique
 *   && problem is reported to error channel.
 */
int
idl_checkConstantDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the operand is valid literal value or expression
 * to result in a value to match the identified target type.
 *
 * idl_checkConstantOperand checks rules regarding the types
 * related to an expression (e.g. the binary + operator may
 * only be applied to integer and float values, but not to
 * a combination of both).
 *
 * idl_checkConstantOperand checks if a subexpression value
 * matches the target type (e.g. if the target type is unsigned
 * short, a subexpression may not evaluate to a negative value,
 * thus (-2)*(-4) is illegal, although the resulting value is
 * positive).
 *
 * idl_checkConstantOperand checks the resulting value of
 * the expression if it matches the target type.
 *
 * returns 0 if the constant literal value or expression
 *     matches the target type.
 * returns >0 when any mismatch or invalid operation is
 *     encountered && problem is reported to error channel.
 */
int
idl_checkConstantOperand (
    c_metaObject scope,
    c_type type,
    c_operand operand,
    idl_errorFunction rf
    );

/*
 * Check if the constant is valid literal value or expression
 * to result in a value to match the identified target type.
 *
 * See idl_checkConstantOperand for comparable behaviour.
 */
int
idl_checkConstantDeclaration (
    c_metaObject scope,
    c_type type,
    c_constant constant,
    idl_errorFunction rf
    );

/*
 * Check if the identifier of a type definition is unique
 * in the identified scope (no scan in a higher level scope).
 * It scans metaObjects and case insensitive.
 *
 * returns 0 if the identifier is unique.
 * returns >0 if the identifier is not unique
 *   && problem is reported to error channel.
 */
int
idl_checkTypeDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check module definition
 */
int
idl_checkModuleDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the referenced base object is of the correct kind.
 * (type kind specifications like struct, union, typedef etc).
 *
 * returns 0 if the object is of correct kind.
 * returns >0 if the identifier is not of correct kind
 *   && problem is reported to error channel.
 */
int
idl_checkTypeReference (
    c_metaObject scope,
    c_type type,
    idl_errorFunction rf
    );

/*
 * Check if the identifier of the enumeration element is unique
 * in the identified scope (no scan in a higher level scope).
 * It scans metaObjects and case insensitive.
 *
 * returns 0 if the identifier is unique.
 * returns >0 if the identifier is not unique
 *   && problem is reported to error channel.
 */
int
idl_checkEnumerationElementDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the enumeration element list represented by iter
 * contains (2^31)-1 elements. Note that this is actually not
 * yet an error situation, but the iterator length is expressed
 * in signed integer, which limits the options.
 *
 * returns 0 if list contains less than (2^31)-1 elements.
 * returns >0 if list contains (2^31)-1 elements
 *   && problem is reported to error channel.
 */
int
idl_checkEnumerationElementCount (
    c_iter iter,
    idl_errorFunction rf
    );

/*
 * Check if the identifier of the interface is unique
 * in the identified scope (no scan in a higher level scope).
 * It scans metaObjects (ignoring interfaces) and case insensitive.
 *
 * returns 0 if the identifier is unique.
 * returns >0 if the identifier is not unique
 *   && problem is reported to error channel.
 */
int
idl_checkInterfaceDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the identifier of the exception is unique
 * in the identified scope (no scan in a higher level scope).
 * It scans metaObjects and case insensitive.
 *
 * returns 0 if the identifier is unique.
 * returns >0 if the identifier is not unique
 *   && problem is reported to error channel.
 */
int
idl_checkExceptionDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the identifier of the operation is unique
 * in the identified scope (no scan in a higher level scope).
 * It scans metaObjects and case insensitive.
 *
 * returns 0 if the identifier is unique.
 * returns >0 if the identifier is not unique
 *   && problem is reported to error channel.
 */
int
idl_checkOperationDefinition (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the identifiers in the structure declarator list (iter) are unique.
 * It scans case insensitive.
 *
 * returns 0 if all identifiers are unique.
 * returns >0 if not all identifiers are unique
 *   && problem is reported to error channel.
 */
int
idl_checkStructDeclaratorDefinition (
    c_metaObject scope,
    c_iter iter,
    idl_errorFunction rf
    );

/*
 * Check if the identifiers in the union declarator list (iter) are unique.
 * It scans case insensitive.
 * If the switchType is an enumeration it also checks if an declarator identifier
 * does not conflict with the enumeration identification.
 *
 * returns 0 if all identifiers are unique.
 * returns >0 if not all identifiers are unique
 *   && problem is reported to error channel.
 */
int
idl_checkUnionDeclaratorDefinition (
    c_metaObject scope,
    c_type switchType,
    c_iter iter,
    idl_errorFunction rf
    );

/*
 * Check if the union switch type is a legal in its context.
 * It also checks if there is an default case where it is not allowed.
 *
 * returns 0 if switch is a legal type && not default case error.
 * returns >0 if (not switch is a legal type || default case error)
 *   && problem is reported to error channel.
 */
int
idl_checkUnionCaseDefinition (
    c_metaObject scope,
    c_type switchType,
    c_iter iter,
    idl_errorFunction rf
    );

/*
 * Check if the keylist type name is a visible struct or union type in
 * the identified scope (no scan in a higher level scope).
 *
 * returns 0 if referenced struct or union identifier is found.
 * returns >0 if not referenced struct or union identifier is found
 *   && problem is reported to error channel.
 */
int
idl_checkKeyListTypeName (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the fieldName identified by name addresses a correct sequence
 * of declarators within a struct identified by scope. name is a "."
 * seperated list of nested declarators.
 *
 * returns 0 if all declarators are found within the scope.
 * returns >0 if at least one declarator is not found
 *   && problem is reported to error channel.
 */
int
idl_checkKeyListFieldName (
    c_metaObject scope,
    char *name,
    idl_errorFunction rf
    );

/*
 * Check if the fieldName identified by name addresses a correct sequence
 * of declarators within a struct identified by scope. name is a declarator,
 * no nesting allowed.
 *
 * returns 0 if the declarator is found within the scope.
 * returns >0 if the declarator is not found
 *   && problem is reported to error channel.
 */
int
idl_checkSimpleFieldName(
    c_metaObject scope,
    char *name,
    idl_errorFunction rf);

/*
 * Check is the operand is a positive integer value within the range
 * that matches unsigned long.
 */
int
idl_checkPositiveInt (
    c_metaObject scope,
    c_operand operand,
    idl_errorFunction rf
    );
