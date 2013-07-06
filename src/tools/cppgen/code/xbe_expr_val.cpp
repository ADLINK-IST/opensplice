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
#include "idl.h"
#include "idl_extern.h"
#include "xbe_expr_val.h"
#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_argument.h"

#include "cppgen_iostream.h"

be_expr_val::be_expr_val (UTL_ScopedName *n)
:
   AST_Expression (n)
{}

be_expr_val::be_expr_val(
   AST_Expression *b,
   AST_Expression::ExprType t)
      :
      AST_Expression(b, t)
{}

be_expr_val::be_expr_val(
   AST_Expression::ExprComb c,
   AST_Expression *v1,
   AST_Expression *v2)
      :
      AST_Expression(c, v1, v2)
{}

be_expr_val::be_expr_val (long l) : AST_Expression (l)
{}

be_expr_val::be_expr_val (long l, AST_Expression::ExprType t)
   : AST_Expression (l, t)
{}

be_expr_val::be_expr_val (unsigned long l) : AST_Expression (l)
{}

be_expr_val::be_expr_val (UTL_String *s) : AST_Expression (s)
{}

be_expr_val::be_expr_val (char c) : AST_Expression (c)
{}

be_expr_val::be_expr_val (double d, const char * str) : AST_Expression (d, str)
{}
