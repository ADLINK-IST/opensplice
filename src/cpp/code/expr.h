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
#ifndef _EXPR_
#define _EXPR_

typedef struct EORB_CPP_node
{
   int leaf;
   char *name;
   struct EORB_CPP_node *left;
   struct EORB_CPP_node *right;
   int op;
}
EORB_CPP_node;

extern EORB_CPP_node * read_expr_ (void);
extern EORB_CPP_node * read_expr_p (void);
extern int eval_expr (int, int);

int expr_sharp;

#endif
