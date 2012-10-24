/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/** \file q_expr.h
 *  \brief The component that implements the database query intermediate code.
 *
 * This header file defines the types, functions and marcos that belong to the
 * database query intermediate code component.
 * 
 * The database query constructor uses this intermediate code to specify the query.
 * Database users can generate intermediate code from a SQL statment using the
 * q_parse function provided by this interface or provide its own custom method
 * to generate intermediate code.
 * Also optimizing algorithms can easily be implemented and executed upon
 * intermediate code.
 *
 * The types, functions and macros defined by this interface helps users to
 * construct and interrogate intermediate code.
 *
 * The intermediate code is a hierarchical tree structure of expressions.
 * An expression can either be a value-object or be a function-object.
 * Value objects hold the value of an error, variable, integer, double, string or type.
 * Fuction objects specifies an operation on a list of expressions it holds.
 *
 * Intermediate code is allocated on the heap memory of the callee. The callee
 * is responsible for freeing allocated memory.
 * 
 */

#ifndef Q_EXPR_H
#define Q_EXPR_H

#include "c_base.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief This enumeration type specifies the kind of an expression.
 *
 * The following values are enumerated:
 * <ol>
 * <li> \b T_ERR: \b An error value.
 * <li> \b T_VAR: \b A variable id.
 * <li> \b T_INT: \b An integer value.
 * <li> \b T_DBL: \b A real value.
 * <li> \b T_CHR: \b A character value.
 * <li> \b T_STR: \b A string value.
 * <li> \b T_ID : \b A identifier.
 * <li> \b T_TYP: \b A type id.
 * <li> \b T_FNC: \b A function specifying an operation upon a list of expressions.
 * </ol>
 *
 */
typedef enum q_kind {
    T_ERR, T_VAR, T_INT, T_DBL, T_CHR, T_STR, T_ID, T_FNC, T_TYP
} q_kind;

/** \brief This enumeration type specifies the operation of a function.
 *
 * The following values are enumerated:
 * <ol>
 * <li> \b Q_EXPR_DONTCARE: \b No special meaning.
 * <li> \b Q_EXPR_ERROR: \b Defines an error value,
 *                     the functions expression list holds the value.
 * <li> \b Q_EXPR_PROGRAM: \b TBS.
 * <li> \b Q_EXPR_IMPORT: \b .
 * <li> \b Q_EXPR_DEFINE: \b .
 * <li> \b Q_EXPR_UNDEFINE: \b .
 * <li> \b Q_EXPR_BIND: \b .
 * <li> \b Q_EXPR_SELECT: \b .
 * <li> \b Q_EXPR_SELECTDISTINCT: \b .
 * <li> \b Q_EXPR_PARAMS: \b .
 * <li> \b Q_EXPR_PROJECTION: \b .
 * <li> \b Q_EXPR_FROM: \b .
 * <li> \b Q_EXPR_WHERE: \b .
 * <li> \b Q_EXPR_GROUP: \b .
 * <li> \b Q_EXPR_HAVING: \b .
 * <li> \b Q_EXPR_ORDER: \b .
 * <li> \b Q_EXPR_DESC: \b .
 * <li> \b Q_EXPR_CAST: \b .
 * <li> \b Q_EXPR_OR: \b .
 * <li> \b Q_EXPR_ORELSE: \b .
 * <li> \b Q_EXPR_AND: \b .
 * <li> \b Q_EXPR_FORALL: \b .
 * <li> \b Q_EXPR_EXISTS: \b .
 * <li> \b Q_EXPR_ANDTHEN: \b .
 * <li> \b Q_EXPR_LIKE: \b .
 * <li> \b Q_EXPR_EQ: \b Specifies an equality predicate between the value of the
 *                  first expression and the value of the second expression in
 *                  the list.
 * <li> \b Q_EXPR_NE: \b Specifies a not equal predicate between the value of the
 *                  first expression and the value of the second expression in
 *                  the list.
 * <li> \b Q_EXPR_LT: \b Specifies a less than predicate between the value of the
 *                  first expression and the value of the second expression in
 *                  the list.
 * <li> \b Q_EXPR_LE: \b Specifies a less or equal predicate between the value of the
 *                  first expression and the value of the second expression in
 *                  the list.
 * <li> \b Q_EXPR_GT: \b Specifies a greater than predicate between the value of the
 *                  first expression and the value of the second expression in
 *                  the list.
 * <li> \b Q_EXPR_GE: \b Specifies a greater or equal predicate between the value of the
 *                  first expression and the value of the second expression in
 *                  the list.
 * <li> \b Q_EXPR_SOME: \b .
 * <li> \b Q_EXPR_ANY: \b .
 * <li> \b Q_EXPR_ALL: \b .
 * <li> \b Q_EXPR_PLUS: \b .
 * <li> \b Q_EXPR_SUB: \b .
 * <li> \b Q_EXPR_UNION: \b .
 * <li> \b Q_EXPR_EXCEPT: \b .
 * <li> \b Q_EXPR_CONCAT: \b .
 * <li> \b Q_EXPR_MUL: \b .
 * <li> \b Q_EXPR_DIV: \b .
 * <li> \b Q_EXPR_MOD: \b .
 * <li> \b Q_EXPR_INTERSECT: \b .
 * <li> \b Q_EXPR_IN: \b .
 * <li> \b Q_EXPR_ABS: \b .
 * <li> \b Q_EXPR_NOT: \b .
 * <li> \b Q_EXPR_INDEX: \b .
 * <li> \b Q_EXPR_PROPERTY: \b .
 * <li> \b Q_EXPR_LIST: \b .
 * <li> \b Q_EXPR_RANGE_: \b .
 * <li> \b Q_EXPR_FUNCTION: \b .
 * <li> \b Q_EXPR_LISTTOSET: \b .
 * <li> \b Q_EXPR_ELEMENT: \b .
 * <li> \b Q_EXPR_DISTINCT: \b .
 * <li> \b Q_EXPR_FLATTEN: \b .
 * <li> \b Q_EXPR_FIRST: \b .
 * <li> \b Q_EXPR_LAST: \b .
 * <li> \b Q_EXPR_UNIQUE: \b .
 * <li> \b Q_EXPR_SUM: \b .
 * <li> \b Q_EXPR_MIN: \b .
 * <li> \b Q_EXPR_MAX: \b .
 * <li> \b Q_EXPR_AVG: \b .
 * <li> \b Q_EXPR_COUNT: \b .
 * <li> \b Q_EXPR_ISUNDEF: \b .
 * <li> \b Q_EXPR_ISDEF: \b .
 * <li> \b Q_EXPR_CLASS: \b .
 * <li> \b Q_EXPR_STRUCT: \b .
 * <li> \b Q_EXPR_ARRAY: \b .
 * <li> \b Q_EXPR_SET: \b .
 * <li> \b Q_EXPR_BAG: \b .
 * <li> \b Q_EXPR_DATE: \b .
 * <li> \b Q_EXPR_TIME: \b .
 * <li> \b Q_EXPR_TIMESTAMP: \b .
 * <li> \b Q_EXPR_VARIABLE: \b .
 * <li> \b Q_EXPR_SCOPEDNAME: \b .
 * </ol>
 *
 */
typedef enum q_tag {
/*-Wildcard-*/
    Q_EXPR_DONTCARE, Q_EXPR_ERROR,
/*-Calculus-*/
    Q_EXPR_PROGRAM, Q_EXPR_IMPORT, Q_EXPR_DEFINE, Q_EXPR_UNDEFINE,
    Q_EXPR_BIND, Q_EXPR_SELECT, Q_EXPR_SELECTDISTINCT, Q_EXPR_PARAMS,
    Q_EXPR_PROJECTION, Q_EXPR_FROM, Q_EXPR_WHERE, Q_EXPR_GROUP,
    Q_EXPR_HAVING, Q_EXPR_ORDER, Q_EXPR_DESC, Q_EXPR_CAST,
    Q_EXPR_OR, Q_EXPR_ORELSE, Q_EXPR_AND, Q_EXPR_FORALL, Q_EXPR_EXISTS,
    Q_EXPR_ANDTHEN, Q_EXPR_LIKE, Q_EXPR_EQ, Q_EXPR_NE, Q_EXPR_LT,
    Q_EXPR_LE, Q_EXPR_GT, Q_EXPR_GE, Q_EXPR_SOME, Q_EXPR_ANY, Q_EXPR_ALL,
    Q_EXPR_PLUS, Q_EXPR_SUB, Q_EXPR_UNION, Q_EXPR_EXCEPT, Q_EXPR_CONCAT,
    Q_EXPR_MUL, Q_EXPR_DIV, Q_EXPR_MOD, Q_EXPR_INTERSECT, Q_EXPR_IN,
    Q_EXPR_ABS, Q_EXPR_NOT, Q_EXPR_INDEX, Q_EXPR_PROPERTY, Q_EXPR_LIST,
    Q_EXPR_RANGE_, Q_EXPR_FUNCTION, Q_EXPR_LISTTOSET, Q_EXPR_ELEMENT,
    Q_EXPR_DISTINCT, Q_EXPR_FLATTEN, Q_EXPR_FIRST, Q_EXPR_LAST,
    Q_EXPR_UNIQUE, Q_EXPR_SUM, Q_EXPR_MIN, Q_EXPR_MAX, Q_EXPR_AVG,
    Q_EXPR_COUNT, Q_EXPR_ISUNDEF, Q_EXPR_ISDEF, Q_EXPR_CLASS,
    Q_EXPR_STRUCT, Q_EXPR_ARRAY, Q_EXPR_SET, Q_EXPR_BAG, Q_EXPR_DATE,
    Q_EXPR_TIME, Q_EXPR_TIMESTAMP, Q_EXPR_VARIABLE, Q_EXPR_SCOPEDNAME,
/*-Algebra-*/
    Q_EXPR_KEY, Q_EXPR_JOIN,
/*-Callback-*/
    Q_EXPR_CALLBACK
} q_tag;

/** \brief The \b expression \b class declaration.
 */
C_CLASS(q_expr);

/** \brief The \b expression list \b class declaration.
 */
C_CLASS(q_list);


/** \brief The default provided OQL parser.
 *
 * This function generates intermediate code from a given OQL expression.
 * This parser supports only the \b select \b statement as subset of OQL.
 *
 * \param expression The string expression that specifies an SQL select statement.
 *
 * \return The generated intermediate code.
 */
OS_API q_expr
q_parse(
    const c_char *expression);

/** \brief The expression constructors.
 *
 * The intermediate code component provides constructors for each kind of
 * expression.
 * <ol>
 * <li> q_expr q_newInt (c_longlong value);
 * <li> q_expr q_newDbl (c_double   value);
 * <li> q_expr q_newStr (c_char    *str);
 * <li> q_expr q_newVar (c_longlong id);
 * <li> q_expr q_newTyp (c_type     type);
 * <li> q_expr q_newFnc (q_tag operation, q_list params);
 * <li> q_expr q_exprCopy (q_expr expr);
 * </ol>
 * In addition a destructor and pretty print function is provided.
 * <ol>
 * <li> void q_dispose (q_expr expr);
 * <li> void q_print (q_expr expr, c_long indent);
 * </ol>
 */

/** \brief The integer value expression constructor.
 *
 * This constructor creates an integer value expression from a given value.
 *
 * \param value The given integer value.
 * \return The created integer value expression.
 */
OS_API q_expr
q_newInt(
    c_longlong value);

/** \brief The real value expression constructor.
 *
 * This constructor creates a real value expression from a given value.
 *
 * \param value The given real value.
 * \return The created real value expression.
 */
OS_API q_expr
q_newDbl(
    c_double value);

/** \brief The string value expression constructor.
 *
 * This constructor creates a character value expression from a given character.
 *
 * \param chr The given character value.
 * \return The created character value expression.
 */
OS_API q_expr
q_newChr(
    c_char chr);

/** \brief The string value expression constructor.
 *
 * This constructor creates a string value expression from a given string.
 *
 * \param str The given string value.
 * \return The created string value expression.
 */
OS_API q_expr
q_newStr(
    c_char *str);

/** \brief The string value expression constructor.
 *
 * This constructor creates a identifier value expression from a given string.
 *
 * \param str The given string value.
 * \return The created identifier value expression.
 */
OS_API q_expr
q_newId(
    c_char *str);

/** \brief The variable identifier expression constructor.
 *
 * This constructor creates a variable identifier expression from a given id.
 *
 * \param id The given variable identifier.
 * \return The created variable identifier expression.
 */
OS_API q_expr
q_newVar(
    c_longlong id);

/** \brief The type reference expression constructor.
 *
 * This constructor creates a type reference expression from a given type.
 *
 * \param type The given type reference.
 * \return The created type reference expression.
 */
OS_API q_expr
q_newTyp(
    c_type type);

/** \brief The function expression constructor.
 *
 * This constructor creates a function expression from a given operation and
 *  expression list.
 *
 * \param tag Specifies the function operation.
 * \param params The functions expression list.
 * \return The created function expression.
 */
OS_API q_expr
q_newFnc(
    q_tag tag,
    q_list params);

/** \brief The expression copy constructor.
 *
 * \param expr The given expression.
 * \return The constructed value copy of the given expression.
 */
OS_API q_expr
q_exprCopy(
    q_expr expr);

/** \brief The expression destructor.
 *
 * This function recursively walks the components of the given expression and
 * frees all allocated resources.
 *
 * \param expr The expression that will be destructed.
 */
OS_API void
q_dispose(
    q_expr expr);

/** \brief The expression pretty print function.
 *
 * This function prints the value of the given expression and prints it to
 * standard output in a humanoid readable format.
 *
 * \param expr The expression that will be printed.
 */
OS_API void
q_print(
    q_expr expr,
    c_long indent);

OS_API c_longlong
q_getInt(
    q_expr expr);

OS_API c_double
q_getDbl(
    q_expr expr);
    
OS_API c_char
q_getChr(
    q_expr expr);
    
OS_API c_char *
q_getStr(
    q_expr expr);
    
OS_API c_char *
q_getId(
    q_expr expr);

OS_API c_longlong
q_getVar(
    q_expr expr);
    
OS_API c_type
q_getTyp(
    q_expr expr);

OS_API q_kind
q_getKind(
    q_expr expr);

OS_API q_list
q_getLst(
    q_expr expr,
    c_long index);

OS_API c_bool
q_isInt(
    q_expr expr);
    
OS_API c_bool
q_isDbl(
    q_expr expr);
    
OS_API c_bool
q_isStr(
    q_expr expr);
    
OS_API c_bool
q_isId(
    q_expr expr);

OS_API c_bool
q_isVar(
    q_expr expr);
    
OS_API c_bool
q_isTyp(
    q_expr expr);
    
OS_API c_bool
q_isFnc(
    q_expr expr,
    q_tag tag);

OS_API q_tag
q_getTag(
q_expr expr);

OS_API q_tag
q_setTag(
    q_expr expr,
    q_tag tag);

/*-List methods--------------------------------------------*/

/** \brief List constructors and functions
 *
 * Lists don't have constructors, instead lists are created by adding an
 * expression to an empty list represented by a NULL value.
 *
 * The constructing methods:
 * <ol>
 * <li> q_list q_append (q_list list, q_expr expr);
 * <li> q_list q_insert (q_list list, q_expr expr);
 * </ol>
 * Other list methods:
 * <ol>
 * <li> q_list q_next     (q_list list);
 * <li> q_expr q_element  (q_list list);
 * <li> q_list q_listCopy (q_list list);
 * </ol>
 */

OS_API q_list
q_append(
    q_list list,
    q_expr expr);
    
OS_API q_list
q_insert(
    q_list list,
    q_expr expr);
    
OS_API q_list
q_next(
    q_list list);
    
OS_API q_expr
q_element(
    q_list list);
    
OS_API q_list
q_listCopy(
    q_list list);

/*-Convienience methods------------------------------------*/

OS_API c_long
q_getLen(
    q_expr expr);

OS_API q_list
q_getLst(
    q_expr expr,
    c_long index);
    
OS_API q_expr
q_getPar(
    q_expr expr,
    c_long index);
    
OS_API q_expr
q_takePar(
    q_expr expr,
    c_long index);
    
OS_API void
q_swapExpr(
    q_expr oldExpr,
    q_expr newExpr);
    
OS_API q_expr
q_swapPar(
    q_expr expr,
    c_long index,
    q_expr par);
    
OS_API void
q_insertPar(
    q_expr expr,
    q_expr par);

OS_API void
q_addPar(
    q_expr expr,
    q_expr par);

/*-Convienience methods------------------------------------*/

OS_API q_expr
q_newBind(
    c_char *id,
    q_expr expr);
    
OS_API q_expr
q_newStruct(
    q_list list);
    
OS_API q_expr
q_newIterate(
    q_expr variable,
    q_expr collection);
    
OS_API q_expr
q_newProject(
    q_list list);

OS_API q_expr
q_newCompr(
    q_tag  tag,
    q_list list);
    
OS_API q_expr
q_newEqual(
    q_expr left,
    q_expr right);

OS_API q_expr     q_takeTerm       (q_expr *e);
OS_API q_expr     q_takeField      (q_expr *e, c_char *name);
OS_API q_expr     q_takeKey        (q_expr *e, c_array keyList);
OS_API void       q_prefixFieldNames (q_expr *e, c_char *prefix);
OS_API void       q_disjunctify    (q_expr e);
OS_API q_expr     q_removeNots     (q_expr e);
OS_API c_long     q_countVar       (q_expr e);

OS_API c_char    *q_propertyName   (q_expr e);

OS_API c_bool     q_isComparison   (q_expr expr);
OS_API c_bool     q_isAggregate    (q_expr expr);

/*-Convenience macros--------------------------------------*/

#define    q_isAll(e)       (q_isFnc(e, Q_EXPR_ALL))
#define    q_isAny(e)       (q_isFnc(e, Q_EXPR_ANY))
#define    q_isBind(e)      (q_isFnc(e, Q_EXPR_BIND)      && (q_getLen(e)==2))
#define    q_isCall(e)      (q_isFnc(e, Q_EXPR_CALL))
#define    q_isConstruct(e) (q_isFnc(e, Q_EXPR_CONSTRUCT))
#define    q_isGroup(e)     (q_isFnc(e, Q_EXPR_GROUP))
#define    q_isIntersect(e) (q_isFnc(e, Q_EXPR_INTERSECT) && (q_getLen(e)==2))
#define    q_isIterate(e)   (q_isFnc(e, Q_EXPR_ITERATE)   && (q_getLen(e)==2))
#define    q_isListToSet(e) (q_isFnc(e, Q_EXPR_LISTTOSET) && (q_getLen(e)==1))
#define    q_isMember(e)    (q_isFnc(e, Q_EXPR_MEMBER)    && (q_getLen(e)==2))
#define    q_isProject(e)   (q_isFnc(e, Q_EXPR_PROJECT))
#define    q_isSome(e)      (q_isFnc(e, Q_EXPR_SOME))
#define    q_isStruct(e)    (q_isFnc(e, Q_EXPR_STRUCT))

#define    q_leftPar(e)      q_getPar(e,0)
#define    q_rightPar(e)     q_getPar(e,1)
#define    q_swapRight(e,p)  q_swapPar(e,1,p)
#define    q_swapLeft(e,p)   q_swapPar(e,0,p)

#define L0(tag) q_newFnc(tag,NULL)
#define L1(tag,list) q_newFnc(tag,list)
#define L2(tag,e1,list) q_newFnc(tag,q_insert(list,e1))
#define L3(tag,e1,e2,list) L2(tag,e1,q_insert(list,e2))
#define L4(tag,e1,e2,e3,list) L3(tag,e1,e2,q_insert(list,e3))

#define List1(e1) q_insert(NULL,e1)
#define List2(e1,e2) q_insert(List1(e2),e1)
#define List3(e1,e2,e3) q_insert(List2(e2,e3),e1)
#define List4(e1,e2,e3,e4) q_insert(List3(e2,e3,e4),e1)
#define List5(e1,e2,e3,e4,e5) q_insert(List4(e2,e3,e4,e5),e1)

#define F1(tag,e1) q_newFnc(tag,List1(e1))
#define F2(tag,e1,e2) q_newFnc(tag,List2(e1,e2))
#define F3(tag,e1,e2,e3) q_newFnc(tag,List3(e1,e2,e3))
#define F4(tag,e1,e2,e3,e4) q_newFnc(tag,List4(e1,e2,e3,e4))
#define F5(tag,e1,e2,e3,e4,e5) q_newFnc(tag,List5(e1,e2,e3,e4,e5))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* Q_EXPR_H */
