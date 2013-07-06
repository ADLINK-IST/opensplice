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
#ifndef _XBE_EXPR_VAL_HH
#define _XBE_EXPR_VAL_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"

class be_expr_val : public virtual AST_Expression
{
public:

   be_expr_val (UTL_ScopedName *n);
   be_expr_val (AST_Expression *b, AST_Expression::ExprType t);
   be_expr_val (AST_Expression::ExprComb c,
                AST_Expression *v1,
                AST_Expression *v2);
   be_expr_val (long l);
   be_expr_val (long l,
                AST_Expression::ExprType t);
   be_expr_val (unsigned long l);
   be_expr_val (UTL_String *s);
   be_expr_val (char c);
   be_expr_val (double d, const char * str);
};

#endif
