/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
