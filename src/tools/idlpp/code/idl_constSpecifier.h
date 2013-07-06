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
#ifndef IDL_CONSTSPECIFIER_H
#define IDL_CONSTSPECIFIER_H

#include "idl_scope.h"
#include "idl_typeSpecifier.h"
#include "c_typebase.h"

typedef enum {
    idl_or,
    idl_xor,
    idl_and,
    idl_shiftright,
    idl_shiftleft,
    idl_plus,
    idl_minus,
    idl_mul,
    idl_div,
    idl_mod,
    idl_not 
} idl_exprKind;

typedef enum {
    idl_cExpr,
    idl_cLit,
    idl_cOper
} idl_operKind;

/* idl_operand class */
#define idl_operand(o)          ((idl_operand)(o))
C_CLASS(idl_operand);

/* idl_constExpression class */
#define idl_constExpression(o)	((idl_constExpression)(o))
C_CLASS(idl_constExpression);

/* idl_constLiteral class */
#define idl_constLiteral(o)	((idl_constLiteral)(o))
C_CLASS(idl_constLiteral);

/* idl_constOperand class */
#define idl_constOperand(o)	((idl_constOperand)(o))
C_CLASS(idl_constOperand);

/* idl_constSpec class */
#define idl_constSpec(o)	((idl_constSpec)(o))
C_CLASS(idl_constSpec);

/***********************************************************
 * idl_operand
 ***********************************************************/
void idl_operandInit (idl_operand operand, idl_operKind kind);

void idl_operandDeinit (idl_operand operand);

void idl_operandFree (idl_operand operand);

char *idl_operandImage (idl_operand operand);

/***********************************************************
 * idl_constExpression
 ***********************************************************/
idl_constExpression idl_constExpressionNew (idl_exprKind expression);

void idl_constExpressionFree (idl_constExpression constExpression);

char *idl_constExpressionImage (idl_constExpression constExpression);

void idl_constExpressionAdd (idl_constExpression constExpression, idl_operand operand);

c_long idl_constExpressionSize (idl_constExpression constExpression);

idl_operand idl_constExpressionMember (idl_constExpression constExpression, c_long index);

/***********************************************************
 * idl_constLiteral
 ***********************************************************/
idl_constLiteral idl_constLiteralNew (char *value_image);

char *idl_constLiteralImage (idl_constLiteral constLiteral);

void idl_constLiteralFree (idl_constLiteral constLiteral);

/***********************************************************
 * idl_constOperand
 ***********************************************************/
idl_constOperand idl_constOperandNew (idl_constSpec constSpec);

void idl_constOperandFree (idl_constOperand constOperand);

char *idl_constOperandImage (idl_constOperand constOperand);

/***********************************************************
 * idl_constSpec
 ***********************************************************/
idl_constSpec idl_constSpecNew (char *name, idl_typeSpec type, idl_scope scope);

void idl_constSpecFree (idl_constSpec constSpec);

void idl_constSpecOperandSet (idl_constSpec constSpec, idl_operand operand);

idl_operand idl_constSpecOperandGet (idl_constSpec constSpec);

idl_typeSpec idl_constSpecTypeGet (idl_constSpec constSpec);

idl_scope idl_constSpecScopeGet (idl_constSpec constSpec);

char *idl_constSpecImage (idl_constSpec constSpec);

char *idl_constSpecName (idl_constSpec constSpec);

#endif /* IDL_CONSTSPECIFIER_H */
