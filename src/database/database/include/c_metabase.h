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
#ifndef C_METABASE_H
#define C_METABASE_H

#include "c_typebase.h"

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

/*============================================================================*/
/* Meta Forward Declarations                                                  */
/*============================================================================*/

C_CLASS(c_scope);
C_CLASS(c_baseObject);
C_CLASS(c_metaObject);
C_CLASS(c_specifier);
C_CLASS(c_constant);
C_CLASS(c_exception);
C_CLASS(c_property);
C_CLASS(c_attribute);
C_CLASS(c_relation);
C_CLASS(c_operation);
C_CLASS(c_collectionType);
C_CLASS(c_type);
C_CLASS(c_typeDef);
C_CLASS(c_extent);
C_CLASS(c_primitive);
C_CLASS(c_parameter);
C_CLASS(c_member);
C_CLASS(c_enumeration);
C_CLASS(c_structure);
C_CLASS(c_class);
C_CLASS(c_union);
C_CLASS(c_interface);
C_CLASS(c_operand);
C_CLASS(c_literal);
C_CLASS(c_expression);
C_CLASS(c_constOperand);
C_CLASS(c_unionCase);
C_CLASS(c_fixed);
C_CLASS(c_valueType);

/*============================================================================*/
/* Meta Specifiers                                                            */
/*============================================================================*/

typedef enum c_metaKind {
    M_UNDEFINED,
    M_ATTRIBUTE, M_CLASS, M_COLLECTION, M_CONSTANT, M_CONSTOPERAND,
    M_ENUMERATION, M_EXCEPTION, M_EXPRESSION, M_INTERFACE,
    M_LITERAL, M_MEMBER, M_MODULE, M_OPERATION, M_PARAMETER,
    M_PRIMITIVE, M_RELATION, M_BASE, M_STRUCTURE, M_TYPEDEF,
    M_UNION, M_UNIONCASE,
    M_EXTENT, M_EXTENTSYNC,
    M_COUNT
} c_metaKind;

typedef enum c_collKind {
    C_UNDEFINED,
    C_LIST, C_ARRAY, C_BAG, C_SET, C_MAP, C_DICTIONARY,
    C_SEQUENCE, C_STRING, C_WSTRING, C_QUERY, C_SCOPE,
    C_COUNT
} c_collKind;

typedef enum c_primKind {
    P_UNDEFINED,
    P_ADDRESS, P_BOOLEAN, P_CHAR, P_WCHAR, P_OCTET,
    P_SHORT, P_USHORT, P_LONG, P_ULONG, P_LONGLONG, P_ULONGLONG,
    P_FLOAT, P_DOUBLE, P_VOIDP,
    P_MUTEX, P_LOCK, P_COND,
    P_COUNT
} c_primKind;

typedef enum c_exprKind {
    E_UNDEFINED,
    E_OR, E_XOR, E_AND,
    E_SHIFTRIGHT, E_SHIFTLEFT,
    E_PLUS, E_MINUS, E_MUL, E_DIV,
    E_MOD, E_NOT,
    E_COUNT
} c_exprKind;

typedef enum c_direction {
    D_UNDEFINED,
    D_IN, D_OUT, D_INOUT,
    D_COUNT
} c_direction;

/*============================================================================*/
/* Meta Filter Definitions                                                    */
/*============================================================================*/

/* Search Type Qualifiers */
#define CQ_ATTRIBUTE	   (1 << (M_ATTRIBUTE-1))
#define CQ_CLASS	   (1 << (M_CLASS-1))
#define CQ_COLLECTION	   (1 << (M_COLLECTION-1))
#define CQ_CONSTANT	   (1 << (M_CONSTANT-1))
#define CQ_CONSTOPERAND	   (1 << (M_CONSTOPERAND-1))
#define CQ_ENUMERATION	   (1 << (M_ENUMERATION-1))
#define CQ_EXCEPTION	   (1 << (M_EXCEPTION-1))
#define CQ_EXPRESSION	   (1 << (M_EXPRESSION-1))
#define CQ_INTERFACE	   (1 << (M_INTERFACE-1))
#define CQ_LITERAL	   (1 << (M_LITERAL-1))
#define CQ_MEMBER	   (1 << (M_MEMBER-1))
#define CQ_MODULE	   (1 << (M_MODULE-1))
#define CQ_OPERATION	   (1 << (M_OPERATION-1))
#define CQ_PARAMETER	   (1 << (M_PARAMETER-1))
#define CQ_PRIMITIVE	   (1 << (M_PRIMITIVE-1))
#define CQ_RELATION	   (1 << (M_RELATION-1))
#define CQ_BASE		   (1 << (M_BASE-1))
#define CQ_STRUCTURE	   (1 << (M_STRUCTURE-1))
#define CQ_TYPEDEF	   (1 << (M_TYPEDEF-1))
#define CQ_UNION           (1 << (M_UNION-1))
#define CQ_UNIONCASE	   (1 << (M_UNIONCASE-1))
#define CQ_EXTENT	   (1 << (M_EXTENT-1))
#define CQ_EXTENTSYNC	   (1 << (M_EXTENTSYNC-1))

/* Search Directives */
#define CQ_CASEINSENSITIVE (1U << 31) /* Search case insensitive, which is slower */
#define CQ_FIXEDSCOPE      (1 << 30) /* Only search in the provided namespace */

/* Search Specifiers Directive */
#define CQ_SPECIFIERS \
        (CQ_PARAMETER | CQ_MEMBER | CQ_UNIONCASE)

/* Search Meta Objects Directive */
#define CQ_METAOBJECTS \
        (CQ_TYPEDEF | CQ_COLLECTION | CQ_PRIMITIVE | CQ_ENUMERATION | \
         CQ_UNION | CQ_STRUCTURE | CQ_EXCEPTION | CQ_INTERFACE | \
         CQ_MODULE | CQ_CONSTANT | CQ_OPERATION | CQ_ATTRIBUTE | \
         CQ_RELATION | CQ_CLASS)

/* Search Types Directive */
#define CQ_TYPEOBJECTS \
        (CQ_COLLECTION | CQ_ENUMERATION | CQ_PRIMITIVE | \
         CQ_STRUCTURE | CQ_TYPEDEF | CQ_UNION | CQ_INTERFACE)

/* Search Operands Directive */
#define CQ_OPERANDS \
        (CQ_CONSTOPERAND | CQ_EXPRESSION | CQ_LITERAL)

/* Search All Directive */
#define CQ_ALL\
        (CQ_ATTRIBUTE | CQ_CLASS | CQ_COLLECTION | CQ_CONSTANT | \
         CQ_CONSTOPERAND | CQ_ENUMERATION | CQ_EXCEPTION | \
         CQ_EXPRESSION | CQ_INTERFACE | CQ_LITERAL | CQ_MEMBER | \
         CQ_MODULE | CQ_OPERATION | CQ_PARAMETER | CQ_PRIMITIVE | \
         CQ_RELATION | CQ_BASE | CQ_STRUCTURE | CQ_TYPEDEF | CQ_UNION | \
         CQ_UNIONCASE | CQ_EXTENT)

/* Check If baseObject type is in the mask */
#define CQ_KIND_IN_MASK(object,set) \
        ((1 << (c_baseObject(object)->kind-1)) & (set))

/*============================================================================*/
/* Meta Abstract Base Types                                                   */
/*============================================================================*/

C_STRUCT(c_baseObject) {
    c_metaKind kind;
};

C_STRUCT(c_operand) {
    C_EXTENDS(c_baseObject);
};

C_STRUCT(c_specifier) {
    C_EXTENDS(c_baseObject);
    c_string name;
    c_type type;
};

C_STRUCT(c_metaObject) {
    C_EXTENDS(c_baseObject);
    c_metaObject definedIn;
    c_string name;
};

C_STRUCT(c_property) {
    C_EXTENDS(c_metaObject);
    c_ulong offset;      /* implementation memory mapping */
    c_type type;
};

C_STRUCT(c_type) {
    C_EXTENDS(c_metaObject);
    c_address alignment;
    c_base base;
    c_ulong objectCount;
    c_address size;
};

/*============================================================================*/
/* Meta Operand Types                                                         */
/*============================================================================*/

C_STRUCT(c_literal) {
    C_EXTENDS(c_operand);
    c_value value;
};

C_STRUCT(c_constOperand) {
    C_EXTENDS(c_operand);
    c_constant constant;
};

C_STRUCT(c_expression) {
    C_EXTENDS(c_operand);
    c_exprKind kind;
    c_array operands;
};

/*============================================================================*/
/* Meta Specifier Types                                                       */
/*============================================================================*/

C_STRUCT(c_parameter) {
    C_EXTENDS(c_specifier);
    c_direction mode;
};

C_STRUCT(c_member) {
    C_EXTENDS(c_specifier);
    c_address offset;      /* implementation memory mapping */
};

C_STRUCT(c_unionCase) {
    C_EXTENDS(c_specifier);
    c_array labels;     /* c_literal */
};

/*============================================================================*/
/* Meta Property Types                                                        */
/*============================================================================*/

C_STRUCT(c_attribute) {
    C_EXTENDS(c_property);
    c_bool isReadOnly;
};

C_STRUCT(c_relation) {
    C_EXTENDS(c_property);
    c_string inverse;
};

/*============================================================================*/
/* Meta Object Types                                                          */
/*============================================================================*/

C_STRUCT(c_constant) {
    C_EXTENDS(c_metaObject);
    c_operand operand;
    c_type type;
};

C_STRUCT(c_operation) {
    C_EXTENDS(c_metaObject);
    c_array parameters;
    c_type result;
};

C_STRUCT(c_typeDef) {
    C_EXTENDS(c_type);
    c_type alias;
};

/*============================================================================*/
/* Meta Type Types                                                            */
/*============================================================================*/

C_CLASS(c_blob);

C_STRUCT(c_blob) {
    c_long size;
    c_address offset;
    c_array members;
};

C_STRUCT(c_collectionType) {
    C_EXTENDS(c_type);
    c_collKind kind;
    c_long maxSize;
    c_type subType;
};

C_STRUCT(c_primitive) {
    C_EXTENDS(c_type);
    c_primKind kind;
};

C_STRUCT(c_enumeration) {
    C_EXTENDS(c_type);
    c_array elements;   /* c_constant */
};

C_STRUCT(c_union) {
    C_EXTENDS(c_type);
    c_array cases;      /* c_unionCase */
    c_array references; /* optimization */
    c_scope scope;
    c_type switchType;
};

C_STRUCT(c_structure) {
    C_EXTENDS(c_type);
    c_array members;    /* c_member */
    c_array references; /* optimization */
    c_scope scope;
};

C_STRUCT(c_exception) {
    C_EXTENDS(c_structure);
};

C_STRUCT(c_interface) {
    C_EXTENDS(c_type);
    c_bool abstract;
    c_array inherits;   /* c_interface references!!!!! */
    c_array references; /* optimization */
    c_scope scope;
};

C_STRUCT(c_class) {
    C_EXTENDS(c_interface);
    c_class extends;
    c_array keys;       /* c_string */
};

/*============================================================================*/
/* Meta Miscelanious Types                                                    */
/*============================================================================*/

C_STRUCT(c_fixed) {
    C_EXTENDS(c_type);
    c_ushort digits;
    c_short scale;
};

C_STRUCT(c_valueType) {
    C_EXTENDS(c_interface);
};

/*============================================================================*/
/* Meta Cast Macro's                                                          */
/*============================================================================*/
#define c_baseObject(o)     ((c_baseObject)(o))
#define c_metaObject(o)     ((c_metaObject)(o))
#define c_property(o)       ((c_property)(o))
#define c_specifier(o)      ((c_specifier)(o))
#define c_operand(o)        ((c_operand)(o))
#define c_type(o)           ((c_type)(o))
#define c_interface(o)      ((c_interface)(o))
#define c_collectionType(o) (C_CAST(o,c_collectionType))
#define c_structure(o)      (C_CAST(o,c_structure))
#define c_attribute(o)      (C_CAST(o,c_attribute))
#define c_class(o)          (C_CAST(o,c_class))
#define c_constant(o)       (C_CAST(o,c_constant))
#define c_enumeration(o)    (C_CAST(o,c_enumeration))
#define c_exception(o)      (C_CAST(o,c_exception))
#define c_operation(o)      (C_CAST(o,c_operation))
#define c_primitive(o)      (C_CAST(o,c_primitive))
#define c_relation(o)       (C_CAST(o,c_relation))
#define c_typeDef(o)        (C_CAST(o,c_typeDef))
#define c_union(o)          (C_CAST(o,c_union))
#define c_literal(o)        (C_CAST(o,c_literal))
#define c_constOperand(o)   (C_CAST(o,c_constOperand))
#define c_expression(o)     (C_CAST(o,c_expression))
#define c_parameter(o)      (C_CAST(o,c_parameter))
#define c_unionCase(o)      (C_CAST(o,c_unionCase))
#define c_member(o)         (C_CAST(o,c_member))
#define c_blob(o)           ((c_blob)(o))

/*============================================================================*/
/* Meta Support Methods                                                       */
/*============================================================================*/
typedef enum c_result {
    S_ACCEPTED, S_REDECLARED, S_ILLEGALOBJECT, S_REDEFINED, S_ILLEGALRECURSION, S_COUNT
} c_result;

typedef enum c_metaEquality {
    E_UNEQUAL, E_EQUAL
} c_metaEquality;

typedef void   *c_metaWalkActionArg;
typedef void   (*c_metaWalkAction)    (c_metaObject metaObject, c_metaWalkActionArg arg);

OS_API c_object
c_metaDefine(
    c_metaObject scope,
    c_metaKind kind);

OS_API c_metaObject
c_metaDeclare(
    c_metaObject scope,
    const c_char *name,
    c_metaKind kind);

OS_API c_result
c_metaFinalize(
    c_metaObject object);

OS_API c_metaObject
c_metaBind(
    c_metaObject scope,
    const c_char *name,
    c_metaObject object);

OS_API c_metaObject
c_metaResolve(
    c_metaObject scope,
    const c_char *name);

OS_API c_metaObject
c_metaResolveFixedScope(
    c_metaObject scope,
    const c_char *name);

OS_API c_metaObject
c_metaResolveType(
    c_metaObject scope,
    const c_char *name);

OS_API c_metaObject
c_metaResolveSpecifier(
    c_metaObject scope,
    const c_char *name);

OS_API c_baseObject
c_metaFindByName(
    c_metaObject scope,
    const char *name,
    c_long metaFilter);

OS_API void
c_metaWalk(
    c_metaObject scope,
    c_metaWalkAction action,
    c_metaWalkActionArg arg);

OS_API c_metaObject
c_metaModule(
    c_metaObject object);

OS_API c_string
c_metaName(
    c_metaObject object);

OS_API c_long
c_metaNameLength(
    c_metaObject object);

OS_API c_char *
c_metaScopedName(
    c_metaObject object); /* mallocs heap memeory */

OS_API c_metaEquality
c_metaCompare(
    c_metaObject object,
    c_metaObject o);

OS_API c_valueKind
c_metaValueKind(
    c_metaObject object);
OS_API c_bool
c_isFinal(
    c_metaObject object);

OS_API c_literal
c_enumValue(
    c_enumeration e,
    const c_char *label);

OS_API c_literal
c_operandValue(
    c_operand operand);

OS_API c_bool
c_typeIsRef(
    c_type type);

OS_API c_bool
c_typeHasRef(
    c_type type);

OS_API c_long
c_typeSize(
    c_type type);


/** \brief This method will return a non typedef.
 *  It will test if the type is a typedef and if that is the case then it will
 *  return the actual type.
 */
OS_API c_type
c_typeActualType(
    c_type type);

#define c_baseObjectKind(_this) \
        c_baseObject(_this)->kind

#define c_specifierType(_this) \
        c_type(c_specifier(_this)->type)

#define c_specifierName(_this) \
        c_specifier(_this)->name

#define c_enumerationCount(_this) \
        c_arraySize(c_enumeration(_this)->elements)

#define c_primitiveKind(_this) \
        c_primitive(_this)->kind

#define c_collectionTypeKind(_this) \
        c_collectionType(_this)->kind

#define c_collectionTypeMaxSize(_this) \
        c_collectionType(_this)->maxSize

#define c_collectionTypeSubType(_this) \
        c_collectionType(_this)->subType

#define c_metaResolveProperty(scope,name) \
        c_property(c_metaResolve(c_metaObject(scope),name))

#define c_structureMemberCount(_this) \
        c_arraySize(c_structure(_this)->members)

#define c_structureMember(_this,_index) \
        c_member(c_structure(_this)->members[(_index)])

#define c_memberType(_this) \
        c_specifierType(_this)

#define c_memberOffset(_this) \
        c_member(_this)->offset

#define c_unionUnionCaseCount(_this) \
        c_arraySize(c_union(_this)->cases)

#define c_unionUnionCase(_this,_index) \
        c_unionCase(c_union(_this)->cases[(_index)])

#define c_unionCaseType(_this) \
        c_specifierType(_this)

OS_API c_type c_member_t    (c_base _this);
OS_API c_type c_constant_t  (c_base _this);
OS_API c_type c_literal_t   (c_base _this);
OS_API c_type c_property_t  (c_base _this);
OS_API c_type c_unionCase_t (c_base _this);


/**************************************************************************

  The functions c_arraySize and c_sequenceSize are specified in
  c_metabase.h and not in c_collection.h because the type system
  uses arrays/sequences to store its meta-data.

  On the other hand, c_collection.h has dependencies on c_metabase.h
  because it offers functions to instantiate a collection for any given
  element-type, which is passed as a c_type.

  Specifying both c_arraySize and c_sequenceSize on this level avoids
  cyclic dependencies between c_metabase.h and c_collection.h.

  However, the implementation of the functions has moved to c_base.h,
  mainly because their implementation require inside knowledge about
  headers and confidence numbers.

 *************************************************************************/


/**
 * \brief This operation returns the number of element the specified array
 *        can hold.
 *
 * This operation returns the number of element the specified array can hold.
 *
 * \param a The array on which this operation operates.
 *
 * \return The size in elements of the given array.
 */
OS_API c_long
c_arraySize (
    c_array a);

/**
 * \brief This operation returns the number of element the specified sequence
 *        can hold.
 *
 * This operation returns the number of element the specified sequence can hold.
 *
 * \param a The sequence on which this operation operates.
 *
 * \return The size in elements of the given sequence.
 */
OS_API c_long
c_sequenceSize (
    c_sequence s);

/**************************************************************************/

OS_API c_type
c_metaArrayTypeNew(
    c_metaObject scope,
    const c_char *name,
    c_type subType,
    c_long maxSize);

OS_API c_type
c_metaSequenceTypeNew(
    c_metaObject scope,
    const c_char *name,
    c_type subType,
    c_long maxSize);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_METABASE_H */
