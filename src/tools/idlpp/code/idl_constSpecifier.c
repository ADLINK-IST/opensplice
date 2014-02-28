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
 * This module handles descriptions of constant definitions
 */

#include "idl_program.h"
#include "idl_constSpecifier.h"
#include "idl_genLanguageHelper.h"

#include "c_iterator.h"

#include <string.h>
#include "os_heap.h"
#include "os_stdlib.h"

static char *
operatorImage (
    idl_exprKind exprKind)
{
    char *opImage = NULL;

    switch (exprKind) {
    case idl_or:
	opImage = "|";
	break;
    case idl_xor:
	opImage = "^";
	break;
    case idl_and:
	opImage = "&";
	break;
    case idl_shiftright:
	opImage = ">>";
	break;
    case idl_shiftleft:
	opImage = "<<";
	break;
    case idl_plus:
	opImage = "+";
	break;
    case idl_minus:
	opImage = "-";
	break;
    case idl_mul:
	opImage = "*";
	break;
    case idl_div:
	opImage = "/";
	break;
    case idl_mod:
	opImage = "%";
	break;
    case idl_not:
	opImage = "~";
	break;
    }
    return opImage;
}

/***********************************************************
 * idl_operand
 ***********************************************************/
C_STRUCT(idl_operand) {
    idl_operKind kind;
};

void
idl_operandInit (
    idl_operand operand,
    idl_operKind kind)
{
    operand->kind = kind;
}

void
idl_operandDeinit (
    idl_operand operand)
{
}

void
idl_operandFree (
    idl_operand operand)
{
    switch (operand->kind) {
    case idl_cExpr:
	idl_constExpressionFree (idl_constExpression(operand));
	break;
    case idl_cLit:
	idl_constLiteralFree (idl_constLiteral(operand));
	break;
    case idl_cOper:
	idl_constOperandFree (idl_constOperand(operand));
	break;
    }
}

char *
idl_operandImage (
    idl_operand operand)
{
    char *image = NULL;

    switch (operand->kind) {
    case idl_cExpr:
	image = idl_constExpressionImage (idl_constExpression(operand));
	break;
    case idl_cLit:
	image = idl_constLiteralImage (idl_constLiteral(operand));
	break;
    case idl_cOper:
	image = idl_constOperandImage (idl_constOperand(operand));
	break;
    }
    return image;
}

/***********************************************************
 * idl_constExpression
 ***********************************************************/
C_STRUCT(idl_constExpression) {
    C_EXTENDS (idl_operand);
    idl_exprKind kind;
    c_iter operands;
};

idl_constExpression
idl_constExpressionNew (
    idl_exprKind expression)
{
    idl_constExpression constExpr = os_malloc (C_SIZEOF(idl_constExpression));

    if (constExpr) {
	memset (constExpr, 0, C_SIZEOF(idl_constExpression));
	idl_operandInit (idl_operand(constExpr), idl_cExpr);
	constExpr->kind = expression;
	constExpr->operands = c_iterNew (NULL);
    }
    return constExpr;
}

void
idl_constExpressionFree (
    idl_constExpression constExpression)
{
    while (c_iterLength (constExpression->operands)) {
	idl_operandFree (idl_operand(c_iterTakeFirst (constExpression->operands)));
    }
    c_iterFree (constExpression->operands);
}

char *
idl_constExpressionImage (
    idl_constExpression constExpression)
{
    char *image = NULL;
    char *operandImage = NULL;
    int i;
    int newLen = 0;

    if (c_iterLength (constExpression->operands) == 1) {
	/* Unary operator */
	operandImage = idl_operandImage (idl_operand(c_iterObject (constExpression->operands, 0)));
	newLen = strlen (operatorImage(constExpression->kind)) + strlen (operandImage) + 3;
	image = os_malloc (newLen);
	snprintf (image, newLen, "(%s%s)", operatorImage(constExpression->kind), operandImage);
	os_free (operandImage);
    } else {
	/* Binary operator */
        for (i = 0; i < c_iterLength (constExpression->operands); i++) {
	    operandImage = idl_operandImage (idl_operand(c_iterObject (constExpression->operands, i)));
	    if (image == NULL) {
	        newLen = strlen (operandImage) + 2;
	        image = os_malloc (newLen);
	       os_strncpy (image, "(", newLen);
	    } else {
	        newLen = strlen (image) + strlen (operatorImage(constExpression->kind)) + strlen (operandImage) + 4;
	        image = os_realloc (image, newLen);
		strncat (image, " ", newLen);
	        os_strncat (image, operatorImage(constExpression->kind), newLen);
		strncat (image, " ", newLen);
	    }
	    os_strncat (image, operandImage, newLen);
	    os_free (operandImage);
        }
	strncat (image, ")", newLen);
    }
    return image;
}

void
idl_constExpressionAdd (
    idl_constExpression constExpression, 
    idl_operand operand)
{
    constExpression->operands = c_iterAppend (constExpression->operands, operand);
}

c_long
idl_constExpressionSize (
    idl_constExpression constExpression)
{
    return (c_long)c_iterLength (constExpression->operands);
}

idl_operand
idl_constExpressionMember (
    idl_constExpression constExpression, 
    c_long index)
{
    return (idl_operand)c_iterObject (constExpression->operands, index);
}

/***********************************************************
 * idl_constLiteral
 ***********************************************************/
C_STRUCT(idl_constLiteral) {
    C_EXTENDS (idl_operand);
    char *valueImage;
};

idl_constLiteral
idl_constLiteralNew (
    char *value_image)
{
    idl_constLiteral constLit = os_malloc (C_SIZEOF(idl_constLiteral));

    if (constLit) {
	constLit->valueImage = os_strdup(value_image);
	idl_operandInit (idl_operand(constLit), idl_cLit);
    }
    return constLit;
}

char *
idl_constLiteralImage (
    idl_constLiteral constLiteral)
{
    return os_strdup (constLiteral->valueImage);
}

void
idl_constLiteralFree (
    idl_constLiteral constLiteral)
{
    os_free (constLiteral->valueImage);
    os_free (constLiteral);
}

/***********************************************************
 * idl_constOperand
 ***********************************************************/
C_STRUCT(idl_constOperand) {
    C_EXTENDS (idl_operand);
    idl_constSpec constSpec;
};

idl_constOperand
idl_constOperandNew (
    idl_constSpec constSpec)
{
    idl_constOperand constOper = os_malloc (C_SIZEOF(idl_constOperand));

    if (constOper) {
	constOper->constSpec = constSpec;
	idl_operandInit (idl_operand(constOper), idl_cOper);
    }
    return constOper;
}

void
idl_constOperandFree (
    idl_constOperand constOperand)
{
    os_free (constOperand);
}

char *
idl_constOperandImage (
    idl_constOperand constOperand)
{
    char *image;
    c_char *getter;
    char *tmp;

    image = idl_scopeStackLanguage(
                idl_constSpecScopeGet(constOperand->constSpec),
                idl_constSpecName(constOperand->constSpec));
    getter = idl_genLanguageConstGetter();
    if (getter) {
        tmp = os_malloc(strlen(image) + strlen(getter) + 1);
        os_strcpy(tmp, image);
        os_strcat(tmp, getter);
        os_free(getter);
        os_free(image);
        image = tmp;
    }

    return image;
}

/***********************************************************
 * idl_constSpec
 ***********************************************************/
C_STRUCT(idl_constSpec) {
    char *constantName;
    idl_typeSpec constantType;
    idl_operand operand;
    idl_scope scope;
};

idl_constSpec
idl_constSpecNew (
    char *name,
    idl_typeSpec type,
    idl_scope scope)
{
    idl_constSpec constSpec = os_malloc (C_SIZEOF(idl_constSpec));

    if (constSpec) {
	memset (constSpec, 0, C_SIZEOF(idl_constSpec));
	constSpec->constantName = os_strdup (name);
	constSpec->constantType = type;
	constSpec->scope = scope;
    }
    return constSpec;
}

void
idl_constSpecFree (
    idl_constSpec constSpec)
{
    idl_scopeFree (constSpec->scope);
    os_free (constSpec->constantName);
    os_free (constSpec);
}

void
idl_constSpecOperandSet (
    idl_constSpec constSpec, 
    idl_operand operand)
{
    constSpec->operand = operand;
}

idl_operand
idl_constSpecOperandGet (
    idl_constSpec constSpec)
{
    return constSpec->operand;
}

idl_typeSpec
idl_constSpecTypeGet (
    idl_constSpec constSpec)
{
    return constSpec->constantType;
}

idl_scope
idl_constSpecScopeGet (
    idl_constSpec constSpec)
{
    return (constSpec->scope);
}

char *
idl_constSpecImage (
    idl_constSpec constSpec)
{
    if (constSpec->operand) {
        return idl_operandImage (constSpec->operand);
    }
    return os_strdup ("");
}

char *
idl_constSpecName (
    idl_constSpec constSpec)
{
    return constSpec->constantName;
}
