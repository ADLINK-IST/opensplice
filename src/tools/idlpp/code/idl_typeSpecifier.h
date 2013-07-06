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
#ifndef IDL_TYPESPECIFIER_H
#define IDL_TYPESPECIFIER_H

#include "idl_program.h"
#include "idl_scope.h"
#include "c_typebase.h"
#include "c_metabase.h"

typedef enum {
    idl_explore,
    idl_abort
} idl_action;

typedef enum {
    idl_tbasic,
    idl_ttypedef,
    idl_tenum,
    idl_tstruct,
    idl_tunion,
    idl_tarray,
    idl_tseq
} idl_type;

typedef enum {
    idl_short,
    idl_ushort,
    idl_long,
    idl_ulong,
    idl_longlong,
    idl_ulonglong,
    idl_float,
    idl_double,
    idl_char,
    idl_string,
    idl_boolean,
    idl_octet
} idl_basicType;

/***********************************************************
 * idl_typeSpec
 ***********************************************************/

/** Cast to idl_typeSpec class */
#define idl_typeSpec(o) ((idl_typeSpec)(o))

C_CLASS(idl_typeSpec);

void
idl_typeSpecSetType (
    idl_typeSpec typeSpec,
    idl_type type);

void
idl_typeSpecSetDef (
    idl_typeSpec typeSpec,
    c_type def);

void
idl_typeSpecSetName (
    idl_typeSpec typeSpec,
    const char *name);

void
idl_typeSpecUnset (
    idl_typeSpec typeSpec);

c_char *
idl_typeSpecName (
    idl_typeSpec typeSpec);

idl_type
idl_typeSpecType (
    idl_typeSpec typeSpec);

idl_typeSpec
idl_typeSpecActual (
    idl_typeSpec typeSpec);

void
idl_typeSpecSetRef (
    idl_typeSpec typeSpec,
    c_bool hasRef);

c_bool
idl_typeSpecHasRef (
    idl_typeSpec typeSpec);

c_type
idl_typeSpecDef (
    idl_typeSpec typeSpec);

/***********************************************************
 * idl_typeSpecUser
 ***********************************************************/

/** Cast to idl_typeUser class */
#define idl_typeUser(o) ((idl_typeUser)(o))

C_CLASS(idl_typeUser);

void
idl_typeUserSetFileName (
    idl_typeUser typeUser,
    const char *fileName);

void
idl_typeUserSetScope (
    idl_typeUser typeUser,
    idl_scope scope);

c_char *
idl_typeUserFileName (
    idl_typeUser typeUser);

idl_scope
idl_typeUserScope (
    idl_typeUser typeUser);

/***********************************************************
 * idl_typeArray
 ***********************************************************/
/** Cast to idl_typeArray class */
#define idl_typeArray(o) ((idl_typeArray)(o))

C_CLASS(idl_typeArray);

idl_typeArray
idl_typeArrayNew (
    idl_typeSpec ofType,
    c_long size);

void
idl_typeArrayFree (
    idl_typeArray typeArray);

c_long
idl_typeArraySize (
    idl_typeArray typeArray);

idl_typeSpec
idl_typeArrayType (
    idl_typeArray typeArray);

idl_typeSpec
idl_typeArrayActual (
    idl_typeArray typeArray);

/***********************************************************
 * idl_typeSeq
 ***********************************************************/
/** Cast to idl_typeSeq class */
#define idl_typeSeq(o) ((idl_typeSeq)(o))

C_CLASS(idl_typeSeq);

idl_typeSeq
idl_typeSeqNew (
    idl_typeSpec ofType,
    c_long maxSize);

void
idl_typeSeqFree (
    idl_typeSeq typeSeq);

c_long
idl_typeSeqMaxSize (
    idl_typeSeq typeSeq);

idl_typeSpec
idl_typeSeqType (
    idl_typeSeq typeSeq);

idl_typeSpec
idl_typeSeqActual (
    idl_typeSeq typeSeq);

/***********************************************************
 * idl_typeDef
 ***********************************************************/
/** Cast to idl_typeDef class */
#define idl_typeDef(o) ((idl_typeDef)(o))

C_CLASS(idl_typeDef);

idl_typeDef
idl_typeDefNew (
    idl_typeSpec referedType,
    idl_typeSpec actualType);

void
idl_typeDefFree (
    idl_typeDef typeDef);

idl_typeSpec
idl_typeDefActual (
    idl_typeDef typeDef);

idl_typeSpec
idl_typeDefRefered (
    idl_typeDef typeDef);

idl_typeSpec
idl_typeDefResolveFully (
    idl_typeSpec type);

/***********************************************************
 * idl_typeUnion
 ***********************************************************/
/** Cast to idl_typeUnion class */
#define idl_typeUnion(o) ((idl_typeUnion)(o))

C_CLASS(idl_typeUnion);

idl_typeUnion
idl_typeUnionNew (
    idl_typeSpec switchKind,
    c_long noCases);

void
idl_typeUnionFree (
    idl_typeUnion typeUnion);

idl_typeSpec
idl_typeUnionSwitchKind (
    idl_typeUnion typeUnion);

c_long
idl_typeUnionNoCases (
    idl_typeUnion typeUnion);

/***********************************************************
 * idl_typeStruct
 ***********************************************************/
/** Cast to idl_typeStruct class */
#define idl_typeStruct(o) ((idl_typeStruct)(o))

C_CLASS(idl_typeStruct);

idl_typeStruct
idl_typeStructNew (
    c_long noMembers);

void
idl_typeStructFree (
    idl_typeStruct typeStruct);

c_long
idl_typeStructNoMembers (
    idl_typeStruct typeStruct);

/***********************************************************
 * idl_typeEnum
 ***********************************************************/
/** Cast to idl_typeEnum class */
#define idl_typeEnum(o) ((idl_typeEnum)(o))

C_CLASS(idl_typeEnum);

idl_typeEnum
idl_typeEnumNew (
    c_long noElements);

void
idl_typeEnumFree (
    idl_typeEnum typeEnum);

c_long
idl_typeEnumNoElements (
    idl_typeEnum typeEnum);

/***********************************************************
 * idl_typeBasic
 ***********************************************************/
/** Cast to idl_typeBasic class */
#define idl_typeBasic(o) ((idl_typeBasic)(o))

C_CLASS(idl_typeBasic);

idl_typeBasic
idl_typeBasicNew (
    idl_basicType basicType);

void
idl_typeBasicFree (
    idl_typeBasic typeBasic);

idl_basicType
idl_typeBasicType (
    idl_typeBasic typeBasic);

void
idl_typeBasicSetMaxlen (
    idl_typeBasic typeBasic, c_long maxLen);

c_long
idl_typeBasicMaxlen (
    idl_typeBasic typeBasic);
	/* returns -1 for wrong type 		*/
	/* returns  0 for unbounded sequence 	*/
	/* returns >0 for bounded sequence 	*/

/***********************************************************
 * idl_labelSpec
 ***********************************************************/
/** Cast to idl_labelSpec class */
#define idl_labelSpec(o) ((idl_labelSpec)(o))

C_CLASS(idl_labelSpec);

idl_labelSpec
idl_labelSpecNew (
    idl_typeSpec labelKind, c_long noLabels);

void
idl_labelSpecFree (
    idl_labelSpec labelSpec);

idl_typeSpec
idl_labelSpecLabelKind (
    idl_labelSpec labelSpec);

c_long
idl_labelSpecNoLabels (
    idl_labelSpec labelSpec);

/***********************************************************
 * idl_labelVal
 ***********************************************************/
/** Cast to idl_labelVal class */
#define idl_labelVal(o) ((idl_labelVal)(o))

C_CLASS(idl_labelVal);

typedef enum {
    idl_lenum,
    idl_lvalue,
    idl_ldefault
} idl_labelType;

#if 0
idl_labelVal
idl_labelValDefault (
    idl_labelVal alternativeValue);
#endif

void
idl_labelValFree (
    idl_labelVal labelVal);

idl_labelType
idl_labelValType (
    idl_labelVal labelVal);

/***********************************************************
 * idl_labelDefault
 ***********************************************************/
/** Cast to idl_labelDefault class */
#define idl_labelDefault(o) ((idl_labelDefault)(o))

C_CLASS(idl_labelDefault);

idl_labelDefault
idl_labelDefaultNew (
    idl_labelVal alternativeVal);

void
idl_labelDefaultFree (
    idl_labelDefault labelVal);

idl_labelVal
idl_labelDefaultAlternative (
    idl_labelDefault labelVal);

/***********************************************************
 * idl_labelEnum
 ***********************************************************/
/** Cast to idl_labelEnum class */
#define idl_labelEnum(o) ((idl_labelEnum)(o))

C_CLASS(idl_labelEnum);

idl_labelEnum
idl_labelEnumNew (
    idl_scope scope, const char *val);

void
idl_labelEnumFree (
    idl_labelEnum labelVal);

c_char *
idl_labelEnumVal (
    idl_labelEnum labelVal);

c_char *
idl_labelEnumImage (
    idl_labelEnum labelVal);

/***********************************************************
 * idl_labelValue
 ***********************************************************/
/** Cast to idl_labelValue class */
#define idl_labelValue(o) ((idl_labelValue)(o))

C_CLASS(idl_labelValue);

idl_labelValue
idl_labelValueNew (
    c_value val);

void
idl_labelValueFree (
    idl_labelValue labelVal);

c_value
idl_labelValueVal (
    idl_labelValue labelVal);

/***********************************************************
 * idl_labelValue
 ***********************************************************/

c_bool
idl_isContiguous(
    c_type type);

#endif /* IDL_TYPESPECIFIER_H */
