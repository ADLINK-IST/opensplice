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
