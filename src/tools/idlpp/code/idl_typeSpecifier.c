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
 * This module handles descriptions of data types
 */

#include "c_base.h"
#include "idl_program.h"
#include "idl_genLanguageHelper.h"

#include "os_heap.h"
#include "os_stdlib.h"


/***********************************************************
 * idl_typeSpec
 ***********************************************************/
C_STRUCT(idl_typeSpec) {
    idl_type	type;
    c_char	*name;
    c_bool      hasRef;
    c_type	def;
};

/** @brief set the idl_typeSpec type attribute
 *
 * @param typeSpec Class to operate on
 * @param type Type to set
 */
void
idl_typeSpecSetType (
    idl_typeSpec typeSpec,
    idl_type type)
{
    typeSpec->type = type;
}

/** @brief set the idl_typeSpec type attribute
 *
 * @param typeSpec Class to operate on
 * @param def Meta database type
 */
void
idl_typeSpecSetDef (
    idl_typeSpec typeSpec,
    c_type def)
{
    typeSpec->def = def;
}

/** @brief set the idl_typeSpec name attribute
 *
 * @param typeSpec Class to operate on
 * @param name Name to set
 */
void
idl_typeSpecSetName (
    idl_typeSpec typeSpec,
    const char *name)
{
    typeSpec->name = os_strdup(name);
}

/** @brief unset all idl_typeSpec attributes
 *
 * @param typeSpec Class to operate on
 */
void
idl_typeSpecUnset (
    idl_typeSpec typeSpec)
{
    if (typeSpec->name) {
	os_free (typeSpec->name);
    }
}

/** @brief return the idl_typeSpec type attribute
 *
 * @param typeSpec Class to operate on
 * @return The type of the typeSpec
 */
idl_type
idl_typeSpecType (
    idl_typeSpec typeSpec)
{
    return typeSpec->type;
}

idl_typeSpec
idl_typeSpecActual (
    idl_typeSpec typeSpec)
{
    idl_typeSpec actual = typeSpec;

    if (typeSpec->type == idl_ttypedef) {
        actual = idl_typeDefActual(idl_typeDef(typeSpec));
    }
    return actual;
}

/** @brief return the idl_typeSpec name attribute
 *
 * @param typeSpec Class to operate on
 * @return The name of the type
 */
c_char *
idl_typeSpecName (
    idl_typeSpec typeSpec)
{
    return typeSpec->name;
}

/** @brief set idl_typeUser hasRef attribute
 *
 * @param typeSpec Class to operate on
 * @param hasRef Value to set the attribute to
 */
void
idl_typeSpecSetRef (
    idl_typeSpec typeSpec,
    c_bool hasRef)
{
    typeSpec->hasRef = hasRef;
}

/** @brief return idl_typeUser hasRef attribute
 *
 * @param typeSpec Class to operate on
 * @return TRUE if the type has a string or sequence, FALSE if not
 */
c_bool
idl_typeSpecHasRef (
    idl_typeSpec typeSpec)
{
    return typeSpec->hasRef;
}

/** @brief return idl_typeUser meta definition attribute
 *
 * @param typeSpec Class to operate on
 * @return meta database type attribute
 */
c_type
idl_typeSpecDef (
    idl_typeSpec typeSpec)
{
    return typeSpec->def;
}

/***********************************************************
 * idl_typeUser
 ***********************************************************/
C_STRUCT(idl_typeUser) {
    C_EXTENDS(idl_typeSpec);
    idl_scope	scope;
    c_char     *fileName;
};

/** @brief set the idl_typeUser file name attribute
 *
 * @param typeUser Class to operate on
 * @param fileName File name to set
 */
void idl_typeUserSetFileName (
    idl_typeUser typeUser,
    const char *fileName)
{
    typeUser->fileName = os_strdup(fileName);
}

/** @brief set the idl_typeUser scope attribute
 *
 * @param typeUser Class to operate on
 * @param scope Scope to set
 */
void idl_typeUserSetScope (
    idl_typeUser typeUser,
    idl_scope scope)
{
    typeUser->scope = scope;
}

/** @brief return the idl_typeUser file name attribute
 *
 * @param typeUser Class to operate on
 * @return the filename related to the user defined type
 */
c_char *
idl_typeUserFileName (
    idl_typeUser typeUser)
{
    return typeUser->fileName;
}

/** @brief return the idl_typeUser scope attribute
 *
 * @param typeUser Class to operate on
 * @return the scope of the user defined type
 */
idl_scope
idl_typeUserScope (
    idl_typeUser typeUser)
{
    return typeUser->scope;
}

/** @brief unset all idl_typeUser attributes
 *
 * @param typeUser Class to operate on
 */
void
idl_typeUserUnset (
    idl_typeUser typeUser)
{
    if (typeUser->fileName) {
        os_free (typeUser->fileName);
    }
    if (typeUser->scope) {
        idl_scopeFree (typeUser->scope);
    }
}

/***********************************************************
 * idl_typeArray
 ***********************************************************/
C_STRUCT(idl_typeArray) {
    C_EXTENDS(idl_typeUser);
    idl_typeSpec arrayType;
    c_long	 size;
};

/** @brief create a new idl_typeArray instance
 *
 * @param ofType Specifies the array type
 * @param size Specifies the array size
 * @return A new array class holding the specified type and the size
 */
idl_typeArray
idl_typeArrayNew (idl_typeSpec ofType, c_long size)
{
    idl_typeArray typeArray = os_malloc ((size_t)C_SIZEOF(idl_typeArray));
    memset(typeArray, 0x00, (size_t)C_SIZEOF(idl_typeArray));

    typeArray->arrayType = ofType;
    typeArray->size = size;
    idl_typeSpec(typeArray)->type = idl_tarray;

    return typeArray;
}

/** @brief free an idl_typeArray instance
 *
 * @param typeArray Class to operate on
 */
void
idl_typeArrayFree (
    idl_typeArray typeArray)
{
    switch (idl_typeSpec(typeArray->arrayType)->type) {
    case idl_tbasic:
	idl_typeBasicFree (idl_typeBasic(typeArray->arrayType));
	break;
    case idl_ttypedef:
	idl_typeDefFree (idl_typeDef(typeArray->arrayType));
	break;
    case idl_tenum:
	idl_typeEnumFree (idl_typeEnum(typeArray->arrayType));
	break;
    case idl_tstruct:
	idl_typeStructFree (idl_typeStruct(typeArray->arrayType));
	break;
    case idl_tunion:
	idl_typeUnionFree (idl_typeUnion(typeArray->arrayType));
	break;
    case idl_tarray:
	idl_typeArrayFree (idl_typeArray(typeArray->arrayType));
	break;
    case idl_tseq:
	idl_typeSeqFree (idl_typeSeq(typeArray->arrayType));
	break;
    }
    os_free (typeArray);
}

/** @brief return the idl_typeArray array size attribute
 *
 * @param typeArray Class to operate on
 * @return The size of the array
 */
c_long
idl_typeArraySize (
    idl_typeArray typeArray)
{
    return typeArray->size;
}

/** @brief return the idl_typeArray type attribute
 *
 * @param typeArray Class to operate on
 * @return The type of the one array element
 */
idl_typeSpec
idl_typeArrayType (
    idl_typeArray typeArray)
{
    return typeArray->arrayType;
}

/** @brief return the actual type of an idl_typeArray instance
 *
 * @param typeArray Class to operate on
 * @return The actual type of one array element (skipping other array dimensions)
 */
idl_typeSpec
idl_typeArrayActual (
    idl_typeArray typeArray)
{
    if (idl_typeSpecType(idl_typeSpec(typeArray->arrayType)) == idl_tarray) {
	return idl_typeArrayActual (idl_typeArray(typeArray->arrayType));
    } else {
        return typeArray->arrayType;
    }
}

/***********************************************************
 * idl_typeSeq
 ***********************************************************/
C_STRUCT(idl_typeSeq) {
    C_EXTENDS(idl_typeUser);
    idl_typeSpec seqType;
    c_long	maxSize;
};

/** @brief create a new idl_typeSeq instance
 *
 * @param ofType Specifies the sequence type
 * @param maxSize Specifies the maximum sequence size (0 for unbounded sequence)
 * @return A new sequence holding the specified type and with the specified maximum size
 */
idl_typeSeq
idl_typeSeqNew (idl_typeSpec ofType, c_long maxSize)
{
    idl_typeSeq typeSeq = os_malloc ((size_t)C_SIZEOF(idl_typeSeq));
    memset(typeSeq, 0x00, (size_t)C_SIZEOF(idl_typeSeq));

    typeSeq->seqType = ofType;
    typeSeq->maxSize = maxSize;
    idl_typeSpec(typeSeq)->type = idl_tseq;

    return typeSeq;
}

/** @brief free an idl_typeSeq instance
 *
 * @param typeSeq Class to operate on
 */
void
idl_typeSeqFree (
    idl_typeSeq typeSeq)
{
    switch (idl_typeSpec(typeSeq->seqType)->type) {
    case idl_tbasic:
	idl_typeBasicFree (idl_typeBasic(typeSeq->seqType));
	break;
    case idl_ttypedef:
	idl_typeDefFree (idl_typeDef(typeSeq->seqType));
	break;
    case idl_tenum:
	idl_typeEnumFree (idl_typeEnum(typeSeq->seqType));
	break;
    case idl_tstruct:
	idl_typeStructFree (idl_typeStruct(typeSeq->seqType));
	break;
    case idl_tunion:
	idl_typeUnionFree (idl_typeUnion(typeSeq->seqType));
	break;
    case idl_tarray:
	idl_typeArrayFree (idl_typeArray(typeSeq->seqType));
	break;
    case idl_tseq:
	idl_typeSeqFree (idl_typeSeq(typeSeq->seqType));
	break;
    }
    os_free (typeSeq);
}

/** @brief return an idl_typeSeq maximum size attribute
 *
 * @param typeSeq Class to operate on
 * @return The maxiumu size of the sequence, where 0 specifies unlimited
 */
c_long
idl_typeSeqMaxSize (
    idl_typeSeq typeSeq)
{
    return typeSeq->maxSize;
}

/** @brief return an idl_typeSeq type attribute
 *
 * @param typeSeq Class to operate on
 * @return The type, the sequence holds
 */
idl_typeSpec
idl_typeSeqType (
    idl_typeSeq typeSeq)
{
    return typeSeq->seqType;
}

/** @brief return the actual type of an idl_typeSeq instance
 *
 * @param typeSeq Class to operate on
 * @return The actual type, the sequence holds (skipping nested sequences)
 */
idl_typeSpec
idl_typeSeqActual (
    idl_typeSeq typeSeq)
{
    if (idl_typeSpecType(idl_typeSpec(typeSeq->seqType)) == idl_tseq) {
	return idl_typeSeqActual (idl_typeSeq(typeSeq->seqType));
    } else {
        return typeSeq->seqType;
    }
}

/***********************************************************
 * idl_typeDef
 ***********************************************************/
C_STRUCT(idl_typeDef) {
    C_EXTENDS(idl_typeUser);
    idl_typeSpec referedType;
    idl_typeSpec actualType;
};

/** @brief create a new idl_typeDef instance
 *
 * @param referedType Specifies the refered type
 * @param actualType Specifies the actual type
 * @return A new typedef refering to the specified type and the specified
 * actual type
 */
idl_typeDef
idl_typeDefNew (
    idl_typeSpec referedType,
    idl_typeSpec actualType)
{
    idl_typeDef typeDef = os_malloc ((size_t)C_SIZEOF(idl_typeDef));
    memset(typeDef, 0x00, (size_t)C_SIZEOF(idl_typeDef));

    typeDef->referedType = referedType;
    typeDef->actualType = actualType;
    idl_typeSpec(typeDef)->type = idl_ttypedef;

    return typeDef;
}

/** @brief free an idl_typeDef instance
 *
 * @param typeDef Class to operate on
 */
void
idl_typeDefFree (
    idl_typeDef typeDef)
{
    switch (idl_typeSpec(typeDef->actualType)->type) {
    case idl_tbasic:
	idl_typeBasicFree (idl_typeBasic(typeDef->actualType));
	break;
    case idl_ttypedef:
	idl_typeDefFree (idl_typeDef(typeDef->actualType));
	break;
    case idl_tenum:
	idl_typeEnumFree (idl_typeEnum(typeDef->actualType));
	break;
    case idl_tstruct:
	idl_typeStructFree (idl_typeStruct(typeDef->actualType));
	break;
    case idl_tunion:
	idl_typeUnionFree (idl_typeUnion(typeDef->actualType));
	break;
    case idl_tarray:
	idl_typeArrayFree (idl_typeArray(typeDef->actualType));
	break;
    case idl_tseq:
	idl_typeSeqFree (idl_typeSeq(typeDef->actualType));
	break;
    }
    if (typeDef->actualType != typeDef->referedType) {
        switch (idl_typeSpec(typeDef->referedType)->type) {
        case idl_tbasic:
	    idl_typeBasicFree (idl_typeBasic(typeDef->referedType));
	    break;
        case idl_ttypedef:
	    idl_typeDefFree (idl_typeDef(typeDef->referedType));
	    break;
        case idl_tenum:
	    idl_typeEnumFree (idl_typeEnum(typeDef->referedType));
	    break;
        case idl_tstruct:
	    idl_typeStructFree (idl_typeStruct(typeDef->referedType));
	    break;
        case idl_tunion:
	    idl_typeUnionFree (idl_typeUnion(typeDef->referedType));
	    break;
        case idl_tarray:
	    idl_typeArrayFree (idl_typeArray(typeDef->referedType));
	    break;
        case idl_tseq:
	    idl_typeSeqFree (idl_typeSeq(typeDef->referedType));
	    break;
        }
    }
    idl_typeUserUnset (idl_typeUser(typeDef));
    os_free (typeDef);
}

/** @brief return the actual type of an idl_typeDef instance
 *
 * @param typeDef Class to operate on
 * @return The actual type of the typedef (skipping nested type definitions)
 */
idl_typeSpec
idl_typeDefActual (
    idl_typeDef typeDef)
{
    return idl_typeSpec(typeDef->actualType);
}

/** @brief return the type of an idl_typeDef instance
 *
 * @param typeDef Class to operate on
 * @return The type, the typedef is refering to
 */
idl_typeSpec
idl_typeDefRefered (
    idl_typeDef typeDef)
{
    return idl_typeSpec(typeDef->referedType);
}

idl_typeSpec
idl_typeDefResolveFully (
    idl_typeSpec type)
{
    idl_typeSpec typeTMP;
    idl_typeSpec retVal = NULL;

    typeTMP = type;
    while(typeTMP)
    {
        if(idl_typeSpecType(typeTMP) == idl_ttypedef)
        {
            typeTMP = idl_typeDefRefered(idl_typeDef(typeTMP));
        } else
        {
            retVal = typeTMP;
            typeTMP = NULL;
        }
    }
    return retVal;
}

/***********************************************************
 * idl_typeUnion
 ***********************************************************/
C_STRUCT(idl_typeUnion) {
    C_EXTENDS(idl_typeUser);
    c_long	noCases;
    idl_typeSpec switchKind;
};

/** @brief create a new idl_typeUnion instance
 *
 * @param switchKind Specifies the type of the switch
 * @param noCases Specifies the number of cases in the union
 * @return A new uniun definition with the specified switch kind and
 * the specified number of union cases
 */
idl_typeUnion
idl_typeUnionNew (
    idl_typeSpec switchKind,
    c_long noCases)
{
    idl_typeUnion typeUnion = os_malloc ((size_t)C_SIZEOF(idl_typeUnion));
    memset(typeUnion, 0x00, (size_t)C_SIZEOF(idl_typeUnion));

    typeUnion->switchKind = switchKind;
    typeUnion->noCases = noCases;
    idl_typeSpec(typeUnion)->type = idl_tunion;

    return typeUnion;
}

/** @brief free an idl_typeUnion instance
 *
 * @param typeUnion Class to operate on
 */
void
idl_typeUnionFree (
    idl_typeUnion typeUnion)
{
    switch (idl_typeSpec(typeUnion->switchKind)->type) {
    case idl_tbasic:
	idl_typeBasicFree (idl_typeBasic(typeUnion->switchKind));
	break;
    case idl_ttypedef:
	idl_typeDefFree (idl_typeDef(typeUnion->switchKind));
	break;
    case idl_tenum:
	idl_typeEnumFree (idl_typeEnum(typeUnion->switchKind));
	break;
    case idl_tstruct:
	idl_typeStructFree (idl_typeStruct(typeUnion->switchKind));
	break;
    case idl_tunion:
	idl_typeUnionFree (idl_typeUnion(typeUnion->switchKind));
	break;
    case idl_tarray:
	idl_typeArrayFree (idl_typeArray(typeUnion->switchKind));
	break;
    case idl_tseq:
	idl_typeSeqFree (idl_typeSeq(typeUnion->switchKind));
	break;
    }
    idl_typeUserUnset (idl_typeUser(typeUnion));
    os_free (typeUnion);
}

/** @brief return an idl_typeUnion switch kind attribute
 *
 * @param typeUnion Class to operate on
 * @return The type of the switch
 */
idl_typeSpec
idl_typeUnionSwitchKind (
    idl_typeUnion typeUnion)
{
    return idl_typeSpec(typeUnion->switchKind);
}

/** @brief return an idl_typeUnion number of union cases attribute
 *
 * @param typeUnion Class to operate on
 * @return The number of union cases related to the union
 */
c_long
idl_typeUnionNoCases (
    idl_typeUnion typeUnion)
{
    return typeUnion->noCases;
}

/***********************************************************
 * idl_typeStruct
 ***********************************************************/
C_STRUCT(idl_typeStruct) {
    C_EXTENDS(idl_typeUser);
    c_long	noMembers;
};

/** @brief create a new idl_typeStruct instance
 *
 * @param noMembers Specifies the number of members of the structure
 * @return A new structure definition with the specified number
 * of structure members
 */
idl_typeStruct
idl_typeStructNew (
    c_long noMembers)
{
    idl_typeStruct typeStruct = os_malloc ((size_t)C_SIZEOF(idl_typeStruct));
    memset(typeStruct, 0x00, (size_t)C_SIZEOF(idl_typeStruct));

    typeStruct->noMembers = noMembers;
    idl_typeSpec(typeStruct)->type = idl_tstruct;

    return typeStruct;
}

/** @brief free an idl_typeStruct instance
 *
 * @param typeStruct Class to operate on
 */
void
idl_typeStructFree (
    idl_typeStruct typeStruct)
{
    idl_typeUserUnset (idl_typeUser(typeStruct));
    os_free (typeStruct);
}

/** @brief return an idl_typeStruct number of members attribute
 *
 * @param typeStruct Class to operate on
 * @return The number of members in the structure
 */
c_long
idl_typeStructNoMembers (
    idl_typeStruct typeStruct)
{
    return typeStruct->noMembers;
}

/***********************************************************
 * idl_typeEnum
 ***********************************************************/
C_STRUCT(idl_typeEnum) {
    C_EXTENDS(idl_typeUser);
    c_long	noElements;
};

/** @brief create a new idl_typeEnum instance
 *
 * @param noElements Specifies the number of elements of the enumeration
 * @return A new enumeration definition with the specified number
 * of elements
 */
idl_typeEnum
idl_typeEnumNew (
    c_long noElements)
{
    idl_typeEnum typeEnum = os_malloc ((size_t)C_SIZEOF(idl_typeEnum));
    memset(typeEnum, 0x00, (size_t)C_SIZEOF(idl_typeEnum));

    typeEnum->noElements = noElements;
    idl_typeSpec(typeEnum)->type = idl_tenum;

    return typeEnum;
}

/** @brief free an idl_typeEnum instance
 *
 * @param typeEnum Class to operate on
 */
void
idl_typeEnumFree (
    idl_typeEnum typeEnum)
{
    idl_typeUserUnset (idl_typeUser(typeEnum));
    os_free (typeEnum);
}

/** @brief return an idl_typeEnum number of elements attribute
 *
 * @param typeEnum Class to operate on
 * @return The number of elements in the enumeration
 */
c_long
idl_typeEnumNoElements (
    idl_typeEnum typeEnum)
{
    return typeEnum->noElements;
}

/***********************************************************
 * idl_typeBasic
 ***********************************************************/
C_STRUCT(idl_typeBasic) {
    C_EXTENDS(idl_typeUser);
    idl_basicType basicType;
    c_long max_len; /* for bounded string only 		*/
		    /* for unbounded string set to 0 	*/
};

/** @brief create a new idl_typeBasic instance
 *
 * @param basicType Specifies the subtype of the basic type
 * @return A new basic type definition representing the specified basic type
 */
idl_typeBasic
idl_typeBasicNew (
    idl_basicType basicType)
{
    idl_typeBasic typeBasic = os_malloc ((size_t)C_SIZEOF(idl_typeBasic));
    memset(typeBasic, 0x00, (size_t)C_SIZEOF(idl_typeBasic));

    typeBasic->basicType = basicType;
    typeBasic->max_len = -1;
    idl_typeSpec(typeBasic)->type = idl_tbasic;
    idl_typeSpec(typeBasic)->hasRef = FALSE;

    return typeBasic;
}

/** @brief free an idl_typeBasic instance
 *
 * @param typeBasic Class to operate on
 */
void
idl_typeBasicFree (
    idl_typeBasic typeBasic)
{
    os_free (typeBasic);
}

/** @brief set an idl_typeBasic type attribute
 *
 * @param typeBasic Class to operate on
 * @return The basic type
 */
idl_basicType
idl_typeBasicType (
    idl_typeBasic typeBasic)
{
    if(typeBasic->basicType > 11){ assert(FALSE); }
    return typeBasic->basicType;
}

/** @brief set an idl_typeBasic maximum length attribute
 *
 * @param typeBasic Class to operate on
 * @param maxLen Maximum length of the subtype (valid for string only)
 */
void
idl_typeBasicSetMaxlen (
    idl_typeBasic typeBasic,
    c_long maxLen)
{
    if (typeBasic->basicType == idl_string) {
	typeBasic->max_len = maxLen;
    }
}

/** @brief return an idl_typeBasic maximum length attribute
 *
 * @param typeBasic Class to operate on
 * @return The specified maximum length (only != 0 for bounded strings)
 */
c_long
idl_typeBasicMaxlen (
    idl_typeBasic typeBasic)
{
    if (typeBasic->basicType == idl_string) {
	return typeBasic->max_len;
    } else {
	return -1;
    }
}

/***********************************************************
 * idl_labelSpec
 ***********************************************************/
C_STRUCT(idl_labelSpec) {
    c_long noLabels;
    idl_typeSpec labelKind;
};

/** @brief create a new idl_labelSpec instance
 *
 * @param labelKind Specifies the type of the label
 * @param noLabels Specifies the number of labels
 * @return A new label specification of the specified kind and
 * the specified number of labels
 */
idl_labelSpec
idl_labelSpecNew (
    idl_typeSpec labelKind,
    c_long noLabels)
{
    idl_labelSpec labelSpec = os_malloc ((size_t)C_SIZEOF(idl_labelSpec));

    labelSpec->labelKind = labelKind;
    labelSpec->noLabels = noLabels;

    return labelSpec;
}

/** @brief free an idl_labelSpec instance
 *
 * @param labelSpec Class to operate on
 */
void
idl_labelSpecFree (
    idl_labelSpec labelSpec)
{
    switch (idl_typeSpec(labelSpec->labelKind)->type) {
    case idl_tbasic:
	idl_typeBasicFree (idl_typeBasic(labelSpec->labelKind));
	break;
    case idl_ttypedef:
	idl_typeDefFree (idl_typeDef(labelSpec->labelKind));
	break;
    case idl_tenum:
	idl_typeEnumFree (idl_typeEnum(labelSpec->labelKind));
	break;
    case idl_tstruct:
	idl_typeStructFree (idl_typeStruct(labelSpec->labelKind));
	break;
    case idl_tunion:
	idl_typeUnionFree (idl_typeUnion(labelSpec->labelKind));
	break;
    case idl_tarray:
	idl_typeArrayFree (idl_typeArray(labelSpec->labelKind));
	break;
    case idl_tseq:
	idl_typeSeqFree (idl_typeSeq(labelSpec->labelKind));
	break;
    }
    os_free (labelSpec);
}

/** @brief return an idl_labelSpec label kind attribute
 *
 * @param labelSpec Class to operate on
 * @return The kind of the labels
 */
idl_typeSpec
idl_labelSpecLabelKind (
    idl_labelSpec labelSpec)
{
    return labelSpec->labelKind;
}

/** @brief return an idl_labelSpec number of labels attribute
 *
 * @param labelSpec Class to operate on
 * @return The number of the labels
 */
c_long
idl_labelSpecNoLabels (
    idl_labelSpec labelSpec)
{
    return labelSpec->noLabels;
}

/***********************************************************
 * idl_labelVal
 ***********************************************************/
C_STRUCT(idl_labelVal) {
    idl_labelType labelType;
};

#if 0
/** @brief create a new idl_labelVal instance for the default case
 *
 * @return A new label representing the "default" label
 */
idl_labelVal
idl_labelValDefault (
    idl_labelVal alternativeValue)
{
    idl_labelVal labelVal = os_malloc((size_t)C_SIZEOF(idl_labelVal));

    labelVal->labelType = idl_ldefault;

    return labelVal;
}
#endif

/** @brief free an idl_labelVal instance
 *
 * @param labelVal Class to operate on
 */
void
idl_labelValFree (
    idl_labelVal labelVal)
{
    os_free (labelVal);
}

/** @brief return an idl_labelVal type attribute
 *
 * @param labelVal Class to operate on
 * @return The type of the label value
 */
idl_labelType
idl_labelValType (
    idl_labelVal labelVal)
{
    return labelVal->labelType;
}

/***********************************************************
 * idl_labelDefault
 ***********************************************************/
C_STRUCT(idl_labelDefault) {
    C_EXTENDS(idl_labelVal);
    idl_labelVal alternativeVal;
};

/** @brief create a new idl_labelDefault instance
 *
 * @param val String value of the enumeration
 * @return A new label representing the label of enumeration type
 */
idl_labelDefault
idl_labelDefaultNew (
    idl_labelVal alternativeVal)
{
    idl_labelDefault labelDefault = os_malloc ((size_t)C_SIZEOF(idl_labelDefault));

    idl_labelVal(labelDefault)->labelType = idl_ldefault;
    labelDefault->alternativeVal = alternativeVal;

    return labelDefault;
}

/** @brief free an idl_labelDefault instance
 *
 * @param labelVal Class to operate on
 */
void
idl_labelDefaultFree (
    idl_labelDefault labelVal)
{
    os_free (labelVal->alternativeVal);
    os_free (labelVal);
}

/** @brief return an idl_labelDefault label value attribute
 *
 * @param labelVal Class to operate on
 * @return The image of the enumeration type label and scope
 */
idl_labelVal
idl_labelDefaultAlternative (
    idl_labelDefault labelVal)
{
   return labelVal->alternativeVal;
}

/***********************************************************
 * idl_labelEnum
 ***********************************************************/
C_STRUCT(idl_labelEnum) {
    C_EXTENDS(idl_labelVal);
    c_char *labelVal;
    idl_scope scope;
};

/** @brief create a new idl_labelEnum instance
 *
 * @param scope Scope of the enumeration
 * @param val String value of the enumeration
 * @return A new label representing the label of enumeration type
 */
idl_labelEnum
idl_labelEnumNew (
    idl_scope scope,
    const char *val)
{
    idl_labelEnum labelEnum = os_malloc ((size_t)C_SIZEOF(idl_labelEnum));

    idl_labelVal(labelEnum)->labelType = idl_lenum;
    labelEnum->labelVal = os_strdup(val);
    labelEnum->scope = scope;

    return labelEnum;
}

/** @brief free an idl_labelEnum instance
 *
 * @param labelVal Class to operate on
 */
void
idl_labelEnumFree (
    idl_labelEnum labelVal)
{
    os_free (labelVal->labelVal);
    os_free (labelVal);
}

/** @brief return an idl_labelEnum label value attribute
 *
 * @param labelVal Class to operate on
 * @return The image of the enumeration type label and scope
 */
c_char *
idl_labelEnumVal (
    idl_labelEnum labelVal)
{
   return idl_scopeStack(labelVal->scope, "_", labelVal->labelVal);
}

/** @brief return an idl_labelEnum label image
 *
 * @param labelVal Class to operate on
 * @return The image of the enumeration type label
 */
c_char *
idl_labelEnumImage (
    idl_labelEnum labelVal)
{
   return labelVal->labelVal;
}

/***********************************************************
 * idl_labelValue
 ***********************************************************/
C_STRUCT(idl_labelValue) {
    C_EXTENDS(idl_labelVal);
    c_value labelVal;
};

/** @brief create a new idl_labelValue instance
 *
 * @param val Constant value of the label
 * @return A new label representing the label of numeric constant value
 */
idl_labelValue
idl_labelValueNew (
    c_value val)
{
    idl_labelValue labelValue = os_malloc((size_t)C_SIZEOF(idl_labelValue));

    idl_labelVal(labelValue)->labelType = idl_lvalue;
    labelValue->labelVal = val;

    return labelValue;
}

/** @brief free an idl_labelValue instance
 *
 * @param labelVal Class to operate on
 */
void
idl_labelValueFree (
    idl_labelValue labelVal)
{
    os_free (labelVal);
}

/** @brief return an idl_labelValue label value attribute
 *
 * @param labelVal Class to operate on
 * @return The constant value of the label
 */
c_value
idl_labelValueVal (
    idl_labelValue labelVal)
{
    return labelVal->labelVal;
}

/***********************************************************
 * Identifies contiguous types.
 * Indentifies if a type can be copied straight in or out
 * of the internal database rep with a vanilla memcpy on account
 * of being laid out exactly the same.
 * @param type The definition of the type.
 * @return TRUE if it is a contiguous / copyable type, FALSE otherwise.
 ***********************************************************/
c_bool
idl_isContiguous(
    c_type type)
{
    c_long i;

    switch (c_baseObject(type)->kind) {
#if 0
    case M_COLLECTION:
        switch(c_collectionType(type)->kind) {
        case C_ARRAY:
            if (c_collectionType(type)->maxSize == 0) {
                return FALSE;
            } else {
                return idl_isContiguous(c_collectionType(type)->subType);
            }
        break;
        default:
            return FALSE;
        }
    break;
#endif
    case M_EXCEPTION:
    case M_STRUCTURE:
        for (i=0; i<c_arraySize(c_structure(type)->members); i++) {
            if (!idl_isContiguous(c_specifier(c_structure(type)->members[i])->type)) {
                return FALSE;
            }
        }
        return TRUE;
    break;
    case M_PRIMITIVE:
        if (c_primitive(type)->kind == P_BOOLEAN &&
             idl_getIsISOCpp() && idl_getIsISOCppTypes())
        {
            /* Those clever chaps at the STL have optimised
             * vector<bool> for size. One bit per bool apparently. */
            return FALSE;
        }
        /* Drop through... */
    case M_ENUMERATION:
        return TRUE;
    break;
    case M_TYPEDEF:
        return idl_isContiguous(c_typeDef(type)->alias);
    break;
    default:
        return FALSE;
    break;
    }
}

