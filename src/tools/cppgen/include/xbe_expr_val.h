/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
