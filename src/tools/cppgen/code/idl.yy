/*

COPYRIGHT

Copyright 1992, 1993, 1994 Sun Microsystems, Inc.  Printed in the United
States of America.  All Rights Reserved.

This product is protected by copyright and distributed under the following
license restricting its use.

The Interface Definition Language Compiler Front End (CFE) is made
available for your use provided that you include this license and copyright
notice on all media and documentation and the software program in which
this product is incorporated in whole or part. You may copy and extend
functionality (but may not remove functionality) of the Interface
Definition Language CFE without charge, but you are not authorized to
license or distribute it to anyone else except as part of a product or
program developed by you or with the express written consent of Sun
Microsystems, Inc. ("Sun").

The names of Sun Microsystems, Inc. and any of its subsidiaries or
affiliates may not be used in advertising or publicity pertaining to
distribution of Interface Definition Language CFE as permitted herein.

This license is effective until terminated by Sun for failure to comply
with this license.  Upon termination, you shall destroy or return all code
and documentation for the Interface Definition Language CFE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED AS IS WITH NO WARRANTIES OF
ANY KIND INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR ARISING FROM A COURSE OF
DEALING, USAGE OR TRADE PRACTICE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED WITH NO SUPPORT AND WITHOUT
ANY OBLIGATION ON THE PART OF Sun OR ANY OF ITS SUBSIDIARIES OR AFFILIATES
TO ASSIST IN ITS USE, CORRECTION, MODIFICATION OR ENHANCEMENT.

SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES SHALL HAVE NO LIABILITY WITH
RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY
INTERFACE DEFINITION LANGUAGE CFE OR ANY PART THEREOF.

IN NO EVENT WILL SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES BE LIABLE FOR
ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL, INDIRECT AND CONSEQUENTIAL
DAMAGES, EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Use, duplication, or disclosure by the government is subject to
restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
Technical Data and Computer Software clause at DFARS 252.227-7013 and FAR
52.227-19.

Sun, Sun Microsystems and the Sun logo are trademarks or registered
trademarks of Sun Microsystems, Inc.

SunSoft, Inc.
2550 Garcia Avenue
Mountain View, California  94043

NOTE:

SunOS, SunSoft, Sun, Solaris, Sun Microsystems or the Sun logo are
trademarks or registered trademarks of Sun Microsystems, Inc.

 */

/*
 * idl.yy - YACC grammar for IDL 1.1
 */

/* Declarations */

%{
#include <os_stdlib.h>
#include <os_heap.h>
#include <idl.h>
#include <idl_extern.h>

#include <fe_private.h>
#include <utl_incl.h>
#include <xbe_scopestack.h>

#include <stdio.h>

void yyunput (int c);
extern int yylex (void);
extern void yyerror (const char *);
int yywrap (void);

%}

/*
 * Declare the type of values in the grammar
 */

%union {
  AST_Decl              *dcval;         /* Decl value           */
  UTL_StrList           *slval;         /* String list          */
  UTL_NameList          *nlval;         /* Name list            */
  UTL_ExprList          *elval;         /* Expression list      */
  UTL_LabelList         *llval;         /* Label list           */
  UTL_DeclList          *dlval;         /* Declaration list     */
  FE_InterfaceHeader    *ihval;         /* Interface header     */
  FE_ValueHeader        *vhval;         /* Value header         */
  FE_ValueInheritanceSpec *visval;      /* Value inheritance    */
  AST_Expression        *exval;         /* Expression value     */
  AST_UnionLabel        *ulval;         /* Union label          */
  AST_Field             *ffval;         /* Field value          */
  AST_Expression::ExprType etval;       /* Expression type      */
  AST_Argument::Direction dival;        /* Argument direction   */
  AST_Operation::Flags  ofval;          /* Operation flags      */
  FE_Declarator         *deval;         /* Declarator value     */
  bool                   bval;           /* Boolean value        */
  long                  ival;           /* Long value           */
  double                dval;           /* Double value         */
  float                 fval;           /* Float value          */
  char                  cval;           /* Char value           */
  UTL_String            *sval;          /* String value         */
  char                  *strval;        /* char * value         */
  Identifier            *idval;         /* Identifier           */
  UTL_IdList            *idlist;        /* Identifier list      */
}

/*
 * Token types: These are returned by the lexer
 */

%token <strval> IDENTIFIER

%token          CONST
%token          MODULE
%token          LOCAL
%token          INTERFACE
%token          TYPEDEF
%token          IDL_LONG
%token          IDL_SHORT
%token          UNSIGNED
%token          IDL_DOUBLE
%token          IDL_FLOAT
%token          IDL_CHAR
%token          IDL_WCHAR
%token          IDL_OCTET
%token          IDL_BOOLEAN
%token          ANY
%token          STRUCT
%token          UNION
%token          SWITCH
%token          ENUM
%token          SEQUENCE
%token          STRING
%token          WSTRING
%token          EXCEPTION
%token          CASE
%token          DEFAULT
%token          READONLY
%token          ATTRIBUTE
%token          ONEWAY
%token          IDEMPOTENT
%token          VOID
%token          IN
%token          OUT
%token          INOUT
%token          RAISES
%token          CUSTOM
%token          VALUETYPE
%token          TRUNCATABLE
%token          SUPPORTS
%token          IDL_PUBLIC
%token          IDL_PRIVATE
%token          FACTORY
%token          ABSTRACT
%token          IDL_CONTEXT
%token          OPAQUE

%token <strval> VERSION

%token <ival>   INTEGER_LITERAL
%token <sval>   STRING_LITERAL
%token <cval>   CHARACTER_LITERAL
%token <strval> FLOATING_PT_LITERAL
%token          TRUETOK
%token          FALSETOK

%token <strval> SCOPE_DELIMITOR
%token          LEFT_SHIFT
%token          RIGHT_SHIFT

%token          PRAGMA
%token          PRAGMA_INCLUDE
%token          PFROM
%token          PRAGMA_ASYNC_CLIENT
%token          PRAGMA_ASYNC_SERVER
%token          PRAGMA_ID
%token          PRAGMA_PREFIX
%token          PRAGMA_VERSION
%token          PRAGMA_ANY
%token          PRAGMA_END

/*
 * These are production names:
 */

%type <dcval>  type_spec simple_type_spec constr_type_spec
%type <dcval>  template_type_spec sequence_type_spec string_type_spec
%type <dcval>  struct_type enum_type switch_type_spec union_type
%type <dcval>  array_declarator op_type_spec seq_head wstring_type_spec

%type <idlist> scoped_name value_name
%type <slval>  opt_context at_least_one_string_literal
%type <slval>  string_literals

%type <nlval>  at_least_one_scoped_name scoped_names inheritance_spec
%type <nlval>  value_names
%type <nlval>  opt_raises

%type <elval>  at_least_one_array_dim array_dims

%type <llval>  at_least_one_case_label case_labels

%type <dlval>  at_least_one_declarator declarators

%type <ihval>  interface_header

%type <exval>  expression const_expr or_expr xor_expr and_expr shift_expr
%type <exval>  add_expr mult_expr unary_expr primary_expr literal string_literal
%type <exval>  positive_int_expr array_dim

%type <ulval>  case_label

%type <ffval>  element_spec

%type <etval>  const_type integer_type char_type boolean_type
%type <etval>  floating_pt_type any_type signed_int
%type <etval>  unsigned_int base_type_spec octet_type

%type <dival>  direction

%type <ofval>  opt_op_attribute

%type <deval>  declarator simple_declarator complex_declarator

%type <bval>   opt_readonly

%type <cval>   opt_local_or_abstract

%type <idval>  interface_decl id struct_decl union_decl

%type <vhval>  value_header
%type <visval> value_inheritance_spec
%type <nlval>  at_least_one_value_name support_dcl
%type <bval>   access_decl

%%

/*
 * Production starts here
 */
start :
        definitions
        ;

definitions
        : definition definitions
        | /* empty */
        ;

definition : 
        type_dcl
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_TypeDeclSeen);
        }
        ';'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
        | const_dcl
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstDeclSeen);
        }
        ';'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
        | exception
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ExceptDeclSeen);
        }
        ';'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
        | interface_def
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_InterfaceDeclSeen);
        }
        ';'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
        | module
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ModuleDeclSeen);
        }
        ';'
        | value
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ValueDeclSeen);
        }
          ';'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
        }
        | OPAQUE IDENTIFIER
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_OpaqueDeclSeen);
           UTL_Scope *s = idl_global->scopes()->top_non_null();
           UTL_ScopedName *n = new UTL_ScopedName (new Identifier ($2) ,NULL);
           AST_Opaque *o = NULL;
        
           if(s != NULL)
           {
              o = idl_global->gen()->create_opaque(n,s->get_pragmas());
              (void) s->fe_add_opaque(o);
           }
        }
        ';'
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
        | error
        {
           idl_global->err()->syntax_error(idl_global->parse_state());
        }
        ';'
        {
           idl_global->set_parse_state (IDL_GlobalData::PS_NoState);
           yyerrok;
        }
        |
        pragma
        ;

pragma :
        PRAGMA_INCLUDE STRING_LITERAL PFROM STRING_LITERAL PRAGMA_END
        {
           UTL_IncludeFiles::AddIncludeFile($2->get_string(),$4->get_string());
        }
        | 
        PRAGMA_ASYNC_CLIENT scoped_name PRAGMA_END
        {
           UTL_Scope *s = idl_global->scopes()->top_non_null();
           AST_Decl *d = s->lookup_by_name($2,false);
           if(d)
           {
              if((AST_Interface *)d->narrow((long)&AST_Interface::type_id) ||
                 (AST_Operation *)d->narrow((long)&AST_Operation::type_id)) 
              {
                 d->get_decl_pragmas().set_pragma_client_synchronicity(false);
              }
              else
              {
                 idl_global->err()->warning_msg("Only operations and interfaces can be asynchronous.  Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->warning_msg("Identifier not found.");
           }
        }
        | 
        PRAGMA_ASYNC_SERVER scoped_name PRAGMA_END
        {
           UTL_Scope *s = idl_global->scopes()->top_non_null();
           AST_Decl *d = s->lookup_by_name($2,false);
           if(d)
           {
              if((AST_Interface *)d->narrow((long)&AST_Interface::type_id) ||
                 (AST_Operation *)d->narrow((long)&AST_Operation::type_id)) 
              {
                 d->get_decl_pragmas().set_pragma_server_synchronicity(true);
              }
              else
              {
                 idl_global->err()->warning_msg("Only operations and interfaces can be asynchronous.  Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->warning_msg("Identifier not found.");
           }
        }
        | 
        PRAGMA_ID scoped_name STRING_LITERAL PRAGMA_END
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           AST_Decl * d = s->lookup_by_name ($2, false);
           if(d)
           {
              if(!d->get_decl_pragmas().get_pragma_ID())
              {
                 d->get_decl_pragmas().set_pragma_ID
                    (new UTL_String ($3->get_string ()));
              }
              else
              {
                 idl_global->err()->warning_msg
                    ("Identifier already has ID. Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->warning_msg ("Identifier not found.");
           }
        }
        | 
        PRAGMA_PREFIX STRING_LITERAL PRAGMA_END
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null();
           s->get_pragmas().set_pragma_prefix ($2);
        }
        | 
        PRAGMA_ANY scoped_name PRAGMA_END
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           AST_Decl * d = s->lookup_by_name ($2, false);

           if (d)
           {
              d->set_gen_any ();
           }
           else
           {
              idl_global->err()->error_msg ("Identifier not found.");
           }
        }
        | 
        PRAGMA_VERSION scoped_name VERSION PRAGMA_END
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           AST_Decl * d = s->lookup_by_name ($2, false);
           if (d)
           {
              if (!d->get_decl_pragmas().get_pragma_version ())
              {
                 d->get_decl_pragmas().set_pragma_version (new UTL_String ($3));
              }
              else
              {
                 idl_global->err()->warning_msg
                    ("Identifier already has version. Pragma ignored.");
              }
           }
           else
           {
              idl_global->err()->error_msg ("Identifier not found.");
           }
        }
        | 
        PRAGMA error PRAGMA_END
        {
           idl_global->err()->warning_msg ("Unrecognized pragma ignored.");
        }
        ;

module : MODULE
          {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSeen);
          }
          IDENTIFIER
          {
            if (idl_global->valid_identifier($3))
            {
              Identifier * id = new Identifier ($3);
              UTL_ScopedName *n = new UTL_ScopedName (id, NULL);
              AST_Module *m = NULL;
              UTL_Scope  *s = idl_global->scopes()->top_non_null();

              idl_global->set_parse_state(IDL_GlobalData::PS_ModuleIDSeen);
              /*
               * Make a new module and add it to the enclosing scope
               */
              if (s != NULL)
              {
                 AST_Module * m2;
                 m = idl_global->gen()->create_module (n, s->get_pragmas ());
                 bool inMainFile = m->in_main_file();
                 m2 = s->fe_add_module (m);

                 /* Check if have re-opened existing module */

                 if (m2 != m)
                 {
                    delete m;
                    m = m2;

                    if (inMainFile)
                    {
                       m->set_imported (false);
                    }
                 }
              }
              /*
               * Push it on the stack
               */
              idl_global->scopes()->push(m);
              g_feScopeStack.Push(be_CppEnclosingScope(*n,
                be_CppEnclosingScope::NameIsScope()));
            }
          }
          '{'
          {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleSqSeen);
          }
          definitions
          {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleBodySeen);
          }
          '}'
          {
            idl_global->set_parse_state(IDL_GlobalData::PS_ModuleQsSeen);
            /*
             * Finished with this module - pop it from the scope stack
             */
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          }
          ;

interface_def
        : interface
        | forward
        ;

interface :
        interface_header
        {
          UTL_Scope     *s = idl_global->scopes()->top_non_null();
          AST_Interface *i = NULL;
          AST_Decl      *d = NULL;
          AST_Interface *fd = NULL;

          /*
           * Make a new interface node and add it to its enclosing scope
           */
          if (s != NULL && $1 != NULL) {
            i = idl_global->gen()->create_interface
            (
               $1->local(),
               $1->abstract(),
               $1->interface_name(),
               $1->inherits(),
               $1->n_inherits(),
               s->get_pragmas()
            );
            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), false)) != NULL) {
              /*
               * See if we're defining a forward declared interface.
               */
              if (d->node_type() == AST_Decl::NT_interface) {
                /*
                 * Narrow to an interface
                 */
                fd = AST_Interface::narrow_from_decl(d);
                /*
                 * Successful?
                 */
                if (fd == NULL) {
                  /*
                   * Should we give an error here?
                   */
                }
                /*
                 * If it is a forward declared interface..
                 */
                else if (!fd->is_defined()) {
                  /*
                   * Check if redefining in same scope
                   */
                  if (fd->defined_in() != s) {
                    idl_global->err()
                       ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
                                i,
                                fd,
                                ScopeAsDecl(s));
                  }
                  /*
                   * Check if fwd and interface not in same file
                   */

/*
                  if (fd->file_name() != idl_global->filename()) {
                    idl_global->err()
                       ->error1(UTL_Error::EIDL_FWD_DECL_LOOKUP,i);
                  }
*/

                  /*
                   * All OK, do the redefinition
                   */
                  else 
                  {
                     fd->set_local($1->local());
                     fd->set_inherits($1->inherits());
                     fd->set_n_inherits($1->n_inherits());
                    /*
                     * Update place of definition
                     */
                     fd->set_imported(idl_global->imported());
                     fd->set_in_main_file(idl_global->in_main_file());
                     fd->set_line(idl_global->lineno());
                     fd->set_file_name(idl_global->filename());
          //fd->set_decl_pragmas(s->get_pragmas());
                     /*
                      * Use full definition node
                      */
                     delete i;
                     i = fd;
                  }
                }
              }
            }
            /*
             * Add the interface to its definition scope
             */
            (void) s->fe_add_interface (i);
          }
          /*
           * Push it on the scope stack
           */
          idl_global->scopes()->push(i);
          g_feScopeStack.Push(be_CppEnclosingScope(*($1->interface_name()),
            be_CppEnclosingScope::NameIsScope()));
      }
        '{'
        {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceSqSeen);
        }
        exports
        {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceBodySeen);
        }
        '}'
        {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        }
        ;

interface_decl:
         INTERFACE
         {
            idl_global->set_parse_state (IDL_GlobalData::PS_InterfaceSeen);
         }
         id
         {
            idl_global->set_parse_state (IDL_GlobalData::PS_InterfaceIDSeen);
            $$ = $3;
         }
         ;

interface_header :
        opt_local_or_abstract interface_decl inheritance_spec
        {
            bool local = ($1 == 'L');
            bool abstract = ($1 == 'A');

            idl_global->set_parse_state (IDL_GlobalData::PS_InheritSpecSeen);
            /*
             * Create an AST representation of the information in the header
             * part of an interface - this representation contains a computed
             * list of all interfaces which this interface inherits from,
             * recursively
             */
            $$ = new FE_InterfaceHeader (local, abstract, new UTL_ScopedName ($2, NULL), $3);
        }
        ;

inheritance_spec
        : ':'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_InheritColonSeen);
        }
        at_least_one_scoped_name
        {
          $$ = $3;
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

value
        : value_dcl
        | value_abs_dcl
        | value_box_dcl
        | value_forward_dcl
        ;

value_forward_dcl
        : VALUETYPE id
        {
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName($2, NULL);
              AST_ValueFwd      *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ValueForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_valuetype_fwd (false, n, s->get_pragmas());
                (void) s->fe_add_valuetype_fwd(f);
              }
        }
        | abstract_valuetype id
        {
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName($2, NULL);
              AST_ValueFwd      *f = NULL;

              idl_global->set_parse_state(IDL_GlobalData::PS_ValueForwardDeclSeen);
              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              if (s != NULL) {
                f = idl_global->gen()->create_valuetype_fwd
                   (true, n, s->get_pragmas());
                (void) s->fe_add_valuetype_fwd(f);
              }
        }
        ;

value_box_dcl
        : VALUETYPE id type_spec
        {
              UTL_Scope         *s = idl_global->scopes()->top_non_null();
              UTL_ScopedName    *n = new UTL_ScopedName($2, NULL);
              AST_BoxedValue    *b = NULL;
              AST_Type          *tp = AST_Type::narrow_from_decl($3);

              idl_global->set_parse_state(IDL_GlobalData::PS_BoxedValueDeclSeen);
              if (tp == NULL)
              {
                 idl_global->err()->not_a_type($3);
              }

              /*
               * Create a node representing a forward declaration of a
               * value type. Store it in the enclosing scope
               */
              else if (s != NULL)
              {
                 b = idl_global->gen()->create_boxed_valuetype(n, tp, s->get_pragmas());
                 (void) s->fe_add_boxed_valuetype(b);
              }
        }
        ;

value_abs_dcl
        : abstract_valuetype id value_inheritance_spec
        {
         UTL_Scope      *s = idl_global->scopes()->top_non_null();
         AST_Value      *i = NULL;
         AST_Decl       *d = NULL;
         AST_Value     *fd = NULL;
         UTL_ScopedName *n = new UTL_ScopedName($2, NULL);

         /*
          * Make a new interface node and add it to its enclosing scope
          */
         if (s != NULL)
         {
            i = idl_global->gen()->create_valuetype
            (
               true,
               false,
               $3->truncatable(),
               n,
               $3->inherits(),
               $3->n_inherits(),
               $3->supports(),
               $3->n_supports(),
               s->get_pragmas()
            );

            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), false)) != NULL) {
               /*
                * See if we're defining a forward declared interface.
                */
               if (d->node_type() == AST_Decl::NT_value) {
                  /*
                   * Narrow to an interface
                   */
                  fd = AST_Value::narrow_from_decl(d);
                  /*
                   * If it is a forward declared interface..
                   */
                  if (fd && !fd->is_defined()) {
                     /*
                      * Check if redefining in same scope
                      */
                     if (fd->defined_in() != s) {
                        idl_global->err()
                        ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
                                 i,
                                 fd,
                                 ScopeAsDecl(s));
                     }
                     /*
                      * All OK, do the redefinition
                      */
                     else {
                        fd->set_value_inherits($3->inherits());
                        fd->set_n_value_inherits($3->n_inherits());
                        fd->set_inherits($3->supports());
                        fd->set_n_inherits($3->n_supports());
                        /*
                         * Update place of definition
                         */
                        fd->set_imported(idl_global->imported());
                        fd->set_in_main_file(idl_global->in_main_file());
                        fd->set_line(idl_global->lineno());
                        fd->set_file_name(idl_global->filename());
                        /*
                         * Use full definition node
                         */
                        delete i;
                        i = fd;
                     }
                  }
               }
            }
            /*
             * Add the interface to its definition scope
             */
            (void) s->fe_add_valuetype(i);
         }
         /*
          * Push it on the scope stack
          */
         idl_global->scopes()->push(i);
         g_feScopeStack.Push(be_CppEnclosingScope(*n,
                                                  be_CppEnclosingScope::NameIsScope()));
      }
        '{' exports '}'
        {
            idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
            g_feScopeStack.Pop();
            idl_global->scopes()->pop();
        }
        ;

abstract_valuetype
        : ABSTRACT VALUETYPE
        ;

value_dcl
        : value_header
          {
         UTL_Scope     *s = idl_global->scopes()->top_non_null();
         AST_Value     *i = NULL;
         AST_Decl      *d = NULL;
         AST_Value     *fd = NULL;

         /*
          * Make a new interface node and add it to its enclosing scope
          */
         if (s != NULL && $1 != NULL) {
            i = idl_global->gen()->create_valuetype(false,
                                                    $1->custom(),
                                                    $1->truncatable(),
                                                    $1->value_name(),
                                                    $1->inherits(),
                                                    $1->n_inherits(),
                                                    $1->supports(),
                                                    $1->n_supports(),
                                                    s->get_pragmas());
            if (i != NULL &&
                (d = s->lookup_by_name(i->name(), false)) != NULL) {
               /*
                * See if we're defining a forward declared interface.
                */
               if (d->node_type() == AST_Decl::NT_value) {
                  /*
                   * Narrow to an interface
                   */
                  fd = AST_Value::narrow_from_decl(d);
                  /*
                   * If it is a forward declared interface..
                   */
                  if (fd && !fd->is_defined()) {
                     /*
                      * Check if redefining in same scope
                      */
                     if (fd->defined_in() != s)
                     {
                        idl_global->err()
                        ->error3(UTL_Error::EIDL_SCOPE_CONFLICT,
                                 i,
                                 fd,
                                 ScopeAsDecl(s));
                     }
                     /*
                      * All OK, do the redefinition
                      */
                     else 
                     {
                        fd->set_custom($1->custom());
                        fd->set_truncatable($1->truncatable());
                        fd->set_value_inherits($1->inherits());
                        fd->set_n_value_inherits($1->n_inherits());
                        fd->set_inherits($1->supports());
                        fd->set_n_inherits($1->n_supports());
                        /*
                         * Update place of definition
                         */
                        fd->set_imported(idl_global->imported());
                        fd->set_in_main_file(idl_global->in_main_file());
                        fd->set_line(idl_global->lineno());
                        fd->set_file_name(idl_global->filename());
                        /*
                         * Use full definition node
                         */
                        delete i;
                        i = fd;
                     }
                  }
               }
            }
            /*
             * Add the interface to its definition scope
             */
            (void) s->fe_add_valuetype(i);
         }
         /*
          * Push it on the scope stack
          */
         idl_global->scopes()->push(i);
         g_feScopeStack.Push(be_CppEnclosingScope(*(i->name()),
                                                  be_CppEnclosingScope::NameIsScope()));
      }
 '{' value_elements '}'
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_InterfaceQsSeen);
            /*
             * Done with this interface - pop it off the scopes stack
             */
           g_feScopeStack.Pop();
           idl_global->scopes()->pop();
        }
        ;

value_header
        : VALUETYPE id value_inheritance_spec
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_ValueInheritSpecSeen);
           $$ = new FE_ValueHeader (false, new UTL_ScopedName($2, NULL), $3);
        }
        | CUSTOM VALUETYPE id value_inheritance_spec
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_ValueInheritSpecSeen);
           $$ = new FE_ValueHeader (true, new UTL_ScopedName($3, NULL), $4);
        }
        ;

value_inheritance_spec
        : ':' at_least_one_value_name support_dcl
        {
           $$ = new FE_ValueInheritanceSpec (false, $2, $3);
        }
        | ':' TRUNCATABLE at_least_one_value_name support_dcl
        {
           $$ = new FE_ValueInheritanceSpec (true, $3, $4);
        }
        | support_dcl
        {
           $$ = new FE_ValueInheritanceSpec (true, NULL, $1);
        }
        ;

at_least_one_value_name
        : value_name value_names
        {
          $$ = new UTL_NameList($1, $2);
        }
        ;

value_names
        : value_names ',' value_name
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ValueNameSeen);

          if ($1 == NULL)
          {
            $$ = new UTL_NameList($3, NULL);
          }
          else
          {
            $1->nconc(new UTL_NameList($3, NULL));
            $$ = $1;
          }
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

support_dcl
        : SUPPORTS at_least_one_scoped_name
        {
          $$ = $2;
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

value_name
        : scoped_name
        ;

value_element
        : export
        | state_member
        | init_dcl
        ;

value_elements
        : value_elements value_element
        | /* EMPTY */
        ;

state_member
        : access_decl type_spec at_least_one_declarator ';'
        {
           UTL_Scope            *s = idl_global->scopes()->top_non_null();
           UTL_DecllistActiveIterator *l = NULL;
           AST_StateMember              *m = NULL;
           FE_Declarator                *d = NULL;

           /* idl_global->set_parse_state(IDL_GlobalData::PS_AttrCompleted); */
           /*
            * Create nodes representing attributes and add them to the
            * enclosing scope
            */
           if (s != NULL && $2 != NULL && $3 != NULL)
           {
              l = new UTL_DecllistActiveIterator($3);
              for (;!(l->is_done()); l->next())
              {
                 d = l->item();
                 if (d == NULL)
                    continue;
                 AST_Type *tp = d->compose($2);
                 if (tp == NULL)
                    continue;

                 // Check for anonymous type
                 if ((tp->node_type () == AST_Decl::NT_array)
                     || (tp->node_type () == AST_Decl::NT_sequence))
                 {
                    Identifier * id = d->name ()->head ();
                    const char *postfix =
                       (tp->node_type () == AST_Decl::NT_array)
                       ? "" : "_seq";
                    // first underscore removed by Identifier constructor
                    DDS_StdString anon_type_name =
                       DDS_StdString ("__") + DDS_StdString (id->get_string ())
                          + DDS_StdString (postfix);
                    UTL_ScopedName *anon_scoped_name =
                    new UTL_ScopedName
                    (
                       new Identifier (os_strdup(anon_type_name)),
                       NULL
                    );

                    (void) s->fe_add_typedef
                    (
                       idl_global->gen()->create_typedef
                       (
                          tp,
                          anon_scoped_name,
                          s->get_pragmas ()
                       )
                    );
                 }

                 m = idl_global->gen()->create_state_member($1, tp, d->name(), s->get_pragmas());
                 /*
                  * Add one attribute to the enclosing scope
                  */
                 (void) s->fe_add_state_member(m);
              }
              delete l;
           }
        }
        ;

access_decl
        : IDL_PUBLIC
        {
          $$ = true;
        }
        | IDL_PRIVATE
        {
          $$ = false;
        }
        ;

init_dcl
        : FACTORY id
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName($2, NULL);
          AST_Initializer       *i = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_InitializerIDSeen);
          /*
           * Create a node representing an operation on an interface
           * and add it to its enclosing scope
           */
          if (s != NULL)
          {
             i = idl_global->gen()->create_initializer(n, s->get_pragmas());
             (void) s->fe_add_initializer(i);
          }
          /*
           * Push the initilializer scope onto the scopes stack
           */
          idl_global->scopes()->push(i);
        }
        '(' init_param_dcls ')' raises_expr
        {
          idl_global->scopes()->pop();
        }
        ';'
        ;

init_param_dcls
        : init_param_dcls ',' init_param_dcl
        | init_param_dcl
        | /* EMPTY */
        ;

raises_expr
        : RAISES '(' at_least_one_scoped_name ')'
        | /* EMPTY */
        ;

init_param_dcl
        : IN simple_type_spec simple_declarator
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Argument          *a = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_InitParDeclSeen);
          /*
           * Create a node representing an argument to an initializer
           * Add it to the enclosing scope (the initializer scope)
           */
          if ($2 != NULL && $3 != NULL && s != NULL) {
            AST_Type *tp = $3->compose($2);
            if (tp != NULL) {
              a = idl_global->gen()->create_argument(AST_Argument::dir_IN, tp, $3->name(), s->get_pragmas());
              (void) s->fe_add_argument(a);
            }
          }
        }
        ;

exports
        : exports export
        | /* EMPTY */
        ;

export
        : type_dcl
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeDeclSeen);
        }
          ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
        | const_dcl
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ConstDeclSeen);
        }
          ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
        | exception
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptDeclSeen);
        }
          ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
        | attribute
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrDeclSeen);
        }
          ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
        | operation
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpDeclSeen);
        }
          ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
        }
        | error
        {
          idl_global->err()->syntax_error(idl_global->parse_state());
        }
        ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        }
        ;

at_least_one_scoped_name :
        scoped_name scoped_names
        {
          $$ = new UTL_NameList($1, $2);
        }
        ;

scoped_names
        : scoped_names
          ','
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SNListCommaSeen);
        }
          scoped_name
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopedNameSeen);

          if ($1 == NULL)
            $$ = new UTL_NameList($4, NULL);
          else {
            $1->nconc(new UTL_NameList($4, NULL));
            $$ = $1;
          }
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

scoped_name
        : id
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_SN_IDSeen);

          $$ = new UTL_IdList ($1, NULL);
        }
        | SCOPE_DELIMITOR
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ScopeDelimSeen);
        }
        id
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_SN_IDSeen);

          $$ = new UTL_IdList (new Identifier ($1), new UTL_IdList($3, NULL));
        }
        | scoped_name SCOPE_DELIMITOR
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ScopeDelimSeen);
        }
        id
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_SN_IDSeen);

          $1->nconc (new UTL_IdList ($4, NULL));
          $$ = $1;
        }
        ;

id : IDENTIFIER
        {
           (void) idl_global->valid_identifier ($1);
           $$ = new Identifier ($1);
        }
        ;

forward :
        opt_local_or_abstract interface_decl
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ($2, NULL);
           AST_InterfaceFwd * f = NULL;

           bool local = ($1 == 'L');
           bool abstract = ($1 == 'A');

           idl_global->set_parse_state (IDL_GlobalData::PS_ForwardDeclSeen);
           /*
           * Create a node representing a forward declaration of an
           * interface. Store it in the enclosing scope
           */
           if (s != NULL) 
           {
              f = idl_global->gen()->create_interface_fwd
                 (local, abstract, n, s->get_pragmas ());
              (void) s->fe_add_interface_fwd (f);
           }
        }
        ;

const_dcl :
        CONST
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstSeen);
        }
        const_type
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstTypeSeen);
        }
        id
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstIDSeen);
        }
        '='
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_ConstAssignSeen);
        }
        expression
        {
          UTL_ScopedName *n = new UTL_ScopedName ($5, NULL);
          UTL_Scope      *s = idl_global->scopes()->top_non_null ();
          AST_Constant   *c = NULL;

          idl_global->set_parse_state (IDL_GlobalData::PS_ConstExprSeen);

          /*
           * Create a node representing a constant declaration. Store
           * it in the enclosing scope
           */

          if ($9 != NULL && s != NULL)
          {
            if ($9->coerce ($3) == NULL)
            {
              idl_global->err()->coercion_error ($9, $3);
            }
            else
            {
              c = idl_global->gen()->create_constant
                 ($3, $9, n, s->get_pragmas ());
              (void) s->fe_add_constant (c);
            }
          }
        }
        ;

const_type
        : integer_type
        | char_type
        | octet_type
        | boolean_type
        | floating_pt_type
        | string_type_spec
        {
          $$ = AST_Expression::EV_string;
        }
        | wstring_type_spec
        {
          $$ = AST_Expression::EV_wstring;
        }
        | scoped_name
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Decl              *d = NULL;
          AST_PredefinedType    *c = NULL;
          AST_Typedef           *t = NULL;

          /*
           * If the constant's type is a scoped name, it must resolve
           * to a scalar constant type
           */
          if (s != NULL && (d = s->lookup_by_name($1, true)) != NULL) {
            /*
             * Look through typedefs
             */
            while (d->node_type() == AST_Decl::NT_typedef) {
              t = AST_Typedef::narrow_from_decl(d);
              if (t == NULL)
                break;
              d = t->base_type();
            }
            if (d == NULL)
              $$ = AST_Expression::EV_any;
            else if (d->node_type() == AST_Decl::NT_pre_defined) {
               c = AST_PredefinedType::narrow_from_decl(d);
               if (c != NULL) {
                  $$ = idl_global->PredefinedTypeToExprType(c->pt());
               } else {
                  $$ = AST_Expression::EV_any;
               }
            } else if (d->node_type() == AST_Decl::NT_string) {
               $$ = AST_Expression::EV_string;
            } else
               $$ = AST_Expression::EV_any;
          } else
             $$ = AST_Expression::EV_any;
        }
        ;

expression : const_expr ;

const_expr : or_expr ;

or_expr : xor_expr
        | or_expr '|' xor_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_or, $1, $3);
        }
        ;

xor_expr
        : and_expr
        | xor_expr '^' and_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_xor, $1, $3);
        }
        ;

and_expr
        : shift_expr
        | and_expr '&' shift_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_and, $1, $3);
        }
        ;

shift_expr
        : add_expr
        | shift_expr LEFT_SHIFT add_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_left,$1,$3);
        }
        | shift_expr RIGHT_SHIFT add_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_right,$1,$3);
        }
        ;

add_expr
        : mult_expr
        | add_expr '+' mult_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_add, $1, $3);
        }
        | add_expr '-' mult_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_minus,$1,$3);
        }
        ;

mult_expr
        : unary_expr
        | mult_expr '*' unary_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_mul, $1, $3);
        }
        | mult_expr '/' unary_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_div, $1, $3);
        }
        | mult_expr '%' unary_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_mod, $1, $3);
        }
        ;

unary_expr
        : primary_expr
        | '+' primary_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_u_plus,
                                              $2,
                                              NULL);
        }
        | '-' primary_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_u_minus,
                                              $2,
                                              NULL);
        }
        | '~' primary_expr
        {
          $$ = idl_global->gen()->create_expr(AST_Expression::EC_bit_neg,
                                              $2,
                                              NULL);
        }
        ;

primary_expr
        : scoped_name
        {
          /*
           * An expression which is a scoped name is not resolved now,
           * but only when it is evaluated (such as when it is assigned
           * as a constant value)
           */
          $$ = idl_global->gen()->create_expr($1);
        }
        | literal
        | '(' const_expr ')'
        {
          $$ = $2;
        }
        ;

literal
        : INTEGER_LITERAL
        {
          $$ = idl_global->gen()->create_expr ($1);
        }
        | string_literal
        {
          $$ = $1;
        }
        | CHARACTER_LITERAL
        {
          $$ = idl_global->gen()->create_expr ($1);
        }
        | FLOATING_PT_LITERAL
        {
          $$ = idl_global->gen()->create_expr (atof ($1), $1);
        }
        | TRUETOK
        {
          $$ = idl_global->gen()->create_expr((long) 1,
                                            AST_Expression::EV_bool);
        }
        | FALSETOK
        {
          $$ = idl_global->gen()->create_expr((long) 0,
                                            AST_Expression::EV_bool);
        }
        ;

string_literal
        : string_literal STRING_LITERAL
        {
           int len = strlen($1->ev()->u.strval->get_string());
           len += strlen($2->get_string());
           char *combined = new char[len+1];
           combined[0] = '\0';
           strcat(combined, $1->ev()->u.strval->get_string());
           strcat(combined, $2->get_string());
           UTL_String *str = new UTL_String(combined);
           delete $1->ev()->u.strval;
           $1->ev()->u.strval = str;
           $$ = $1;
        }
        | STRING_LITERAL
        {
           $$ = idl_global->gen()->create_expr ($1);
        }
        ;

positive_int_expr :
        const_expr
        {
            $1->evaluate (AST_Expression::EK_const);
            $$ = idl_global->gen()->create_expr ($1, AST_Expression::EV_ulong);
        }
        ;

type_dcl
        : TYPEDEF
          {
            idl_global->set_parse_state (IDL_GlobalData::PS_TypedefSeen);
          }
          type_declarator
        | struct_type
        | union_type
        | enum_type
        | constr_forward_decl
        ;

constr_forward_decl :
        struct_decl
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ($1, NULL);
           AST_Structure * str;

           idl_global->set_parse_state (IDL_GlobalData::PS_ForwardDeclSeen);

           if (s != NULL) 
           {
              str = idl_global->gen()->create_structure (n, s->get_pragmas ());
              (void) s->fe_add_structure (str);
           }
        }
        |
        union_decl
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ($1, NULL);
           AST_Union * u;

           idl_global->set_parse_state (IDL_GlobalData::PS_ForwardDeclSeen);

           if (s != NULL) 
           {
              u = idl_global->gen()->create_union (n, s->get_pragmas ());
              (void) s->fe_add_union (u);
           }
        }
        ;

struct_decl :
        STRUCT
        {
           idl_global->set_parse_state (IDL_GlobalData::PS_StructSeen);
        }
        id
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_StructIDSeen);
          $$ = $3;
        }
        ;

union_decl :
        UNION
        {
           idl_global->set_parse_state (IDL_GlobalData::PS_UnionSeen);
        }
        id
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionIDSeen);
          $$ = $3;
        }
        ;

type_declarator :
        type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_TypeSpecSeen);
        }
        at_least_one_declarator
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l;
          FE_Declarator         *d = NULL;
          AST_Typedef           *t = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_DeclaratorsSeen);
          /*
           * Create a list of type renamings. Add them to the
           * enclosing scope
           */
          if (s != NULL && $1 != NULL && $3 != NULL) {
            l = new UTL_DecllistActiveIterator($3);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type * tp = d->compose($1);
              if (tp == NULL)
                continue;
              t = idl_global->gen()->create_typedef(tp, d->name(), s->get_pragmas());
              (void) s->fe_add_typedef(t);
            }
            delete l;
          }
        }
        ;

type_spec
        : simple_type_spec
        | constr_type_spec
        ;

simple_type_spec
        : base_type_spec
        {
          $$ = idl_global->scopes()->bottom()->lookup_primitive_type($1);
        }
        | template_type_spec
        | scoped_name
        {
          UTL_Scope     *s = idl_global->scopes()->top_non_null();
          AST_Decl      *d = NULL;

          if (s != NULL)
            d = s->lookup_by_name($1, true);
          if (d == NULL || d->node_type() == AST_Decl::NT_field)
            idl_global->err()->lookup_error($1);
          $$ = d;
        }
        ;

base_type_spec
        : integer_type
        | floating_pt_type
        | char_type
        | boolean_type
        | octet_type
        | any_type
        ;

template_type_spec
        : sequence_type_spec
        | string_type_spec
        | wstring_type_spec
        ;

constr_type_spec
        : struct_type
        | union_type
        | enum_type
        ;

at_least_one_declarator :
        declarator declarators
        {
          $$ = new UTL_DeclList($1, $2);
        }
        ;
declarators
        : declarators
          ','
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_DeclsCommaSeen);
        }
          declarator
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_DeclsDeclSeen);

          if ($1 == NULL)
            $$ = new UTL_DeclList($4, NULL);
          else {
            $1->nconc(new UTL_DeclList($4, NULL));
            $$ = $1;
          }
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

declarator
        : simple_declarator
        | complex_declarator
        ;

simple_declarator :
        id
        {
          $$ = new FE_Declarator(new UTL_ScopedName($1, NULL),
                                 FE_Declarator::FD_simple, NULL);
        }
        ;

complex_declarator :
        array_declarator
        {
          $$ = new FE_Declarator(new UTL_ScopedName($1->local_name(), NULL),
                                 FE_Declarator::FD_complex,
                                 $1);
        }
        ;

integer_type
        : signed_int
        | unsigned_int
        ;

signed_int
        : IDL_LONG
        {
          $$ = AST_Expression::EV_long;
        }
        | IDL_LONG IDL_LONG
        {
          $$ = AST_Expression::EV_longlong;
        }
        | IDL_SHORT
        {
          $$ = AST_Expression::EV_short;
        }
        ;

unsigned_int
        : UNSIGNED IDL_LONG
        {
          $$ = AST_Expression::EV_ulong;
        }
        | UNSIGNED IDL_LONG IDL_LONG
        {
          $$ = AST_Expression::EV_ulonglong;
        }
        | UNSIGNED IDL_SHORT
        {
          $$ = AST_Expression::EV_ushort;
        }
        ;

floating_pt_type
        : IDL_DOUBLE
        {
          $$ = AST_Expression::EV_double;
        }
        | IDL_FLOAT
        {
          $$ = AST_Expression::EV_float;
        }
        | IDL_LONG IDL_DOUBLE
        {
          $$ = AST_Expression::EV_longdouble;
        }
        ;

char_type
        : IDL_CHAR
        {
          $$ = AST_Expression::EV_char;
        }
        | IDL_WCHAR
        {
          $$ = AST_Expression::EV_wchar;
        }
        ;

octet_type
        : IDL_OCTET
        { 
          $$ = AST_Expression::EV_octet;
        }
        ;

boolean_type
        : IDL_BOOLEAN
        { 
          $$ = AST_Expression::EV_bool;
        }
        ;

any_type
        : ANY
        {
          $$ = AST_Expression::EV_any;
        }
        ;

struct_type :
        struct_decl '{'
        {
           UTL_Scope * s = idl_global->scopes()->top_non_null ();
           UTL_ScopedName * n = new UTL_ScopedName ($1, NULL);
           AST_Structure * st = NULL;

           idl_global->set_parse_state (IDL_GlobalData::PS_StructSqSeen);
           if (s != NULL) 
           {
              st = idl_global->gen()->create_structure (n, s->get_pragmas ());
              st->set_defined (true);
              st = s->fe_add_structure (st);
              st->set_defined (true);

              /* Update the declaration */

              st->set_imported (idl_global->imported ());
              st->set_in_main_file (idl_global->in_main_file ());
              st->set_line (idl_global->lineno ());
              st->set_file_name (idl_global->filename ());
           }
           idl_global->scopes()->push (st);
           g_feScopeStack.Push
           (
              be_CppEnclosingScope
              (
                 *n,
                 be_CppEnclosingScope::NameIsScope ()
              )
           );
        }
        at_least_one_member
        {
           idl_global->set_parse_state (IDL_GlobalData::PS_StructBodySeen);
        }
        '}'
        {
           idl_global->set_parse_state (IDL_GlobalData::PS_StructQsSeen);
           /*
            * Done with this struct. Pop its scope off the scopes stack
            */
           if (idl_global->scopes()->top() == NULL)
           {
              $$ = NULL;
           }
           else 
           {
              $$ = AST_Structure::narrow_from_scope
                 (idl_global->scopes()->top_non_null ());
              idl_global->scopes()->pop();
              g_feScopeStack.Pop ();
           }
        }
        ;

at_least_one_member : member members ;

members
        : members member
        | /* EMPTY */
        ;

member  :
        type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_MemberTypeSeen);
        }
        at_least_one_declarator
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsSeen);
        }
        ';'
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l = NULL;
          FE_Declarator         *d = NULL;
          AST_Field             *f = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_MemberDeclsCompleted);
          /*
           * Check for illegal recursive use of type
           */
          if ($1 != NULL && AST_illegal_recursive_type($1))
            idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, $1);
          /*
           * Create a node representing a struct or exception member
           * Add it to the enclosing scope
           */
          else if (s != NULL && $1 != NULL && $3 != NULL) {
            l = new UTL_DecllistActiveIterator($3);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type *tp = d->compose($1);
              if (tp == NULL)
                continue;

              // Check for anonymous type
              if ((tp->node_type () == AST_Decl::NT_array)
                  || (tp->node_type () == AST_Decl::NT_sequence))
              {
                 Identifier * id = d->name ()->head ();
                 const char *postfix =
                    (tp->node_type () == AST_Decl::NT_array)
                    ? "" : "_seq";
                 // first underscore removed by Identifier constructor
                 DDS_StdString anon_type_name =
                    DDS_StdString ("__") + DDS_StdString (id->get_string ())
                       + DDS_StdString (postfix);
                 UTL_ScopedName *anon_scoped_name =
                 new UTL_ScopedName
                 (
                    new Identifier (os_strdup(anon_type_name)),
                    NULL
                 );

                 (void) s->fe_add_typedef
                 (
                    idl_global->gen()->create_typedef
                    (
                       tp,
                       anon_scoped_name,
                       s->get_pragmas ()
                    )
                 );
              }

              f = idl_global->gen()->create_field(tp, d->name(), s->get_pragmas());
              (void) s->fe_add_field(f);
            }
            delete l;
          }
        }
        | error
        {
          idl_global->err()->syntax_error(idl_global->parse_state());
        }
        ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        }
        ;

union_type :
        union_decl SWITCH
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchSeen);
        }
        '('
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchOpenParSeen);
        }
        switch_type_spec
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchTypeSeen);
        }
        ')'
        {
          UTL_Scope * s = idl_global->scopes()->top_non_null ();
          UTL_ScopedName * n = new UTL_ScopedName ($1, NULL);
          AST_Union * u = NULL;

          idl_global->set_parse_state (IDL_GlobalData::PS_SwitchCloseParSeen);
          /*
           * Create a node representing a union. Add it to its enclosing
           * scope
           */

          if ($6 != NULL && s != NULL)
          {
             AST_ConcreteType * tp = AST_ConcreteType::narrow_from_decl ($6);
             if (tp == NULL)
             {
                idl_global->err()->not_a_type ($6);
             }
             else
             {
                u = idl_global->gen()->create_union (n, s->get_pragmas ());
                u->set_defined (true);
                u = s->fe_add_union (u);
                u->set_defined (true);

                u->set_disc_type (tp);

                u->set_imported (idl_global->imported ());
                u->set_in_main_file (idl_global->in_main_file ());
                u->set_line (idl_global->lineno ());
                u->set_file_name (idl_global->filename ());
             }
          }
          idl_global->scopes()->push (u);
          g_feScopeStack.Push
          (
             be_CppEnclosingScope
             (
                *n,
                be_CppEnclosingScope::NameIsScope ()
             )
          );
        }
        '{'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionSqSeen);
        }
        at_least_one_case_branch
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionBodySeen);
        }
        '}'
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_UnionQsSeen);
          /*
           * Done with this union. Pop its scope from the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            $$ = NULL;
          else {
            $$ =
              AST_Union::narrow_from_scope(
                                idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
            g_feScopeStack.Pop();
          }
        }
        ;

switch_type_spec :
        integer_type
        {
          $$ = idl_global->scopes()->bottom()->lookup_primitive_type($1);
        }
        | char_type
        {
          $$ = idl_global->scopes()->bottom()->lookup_primitive_type($1);
        }
        | octet_type
        {
          $$ = idl_global->scopes()->bottom()->lookup_primitive_type($1);
        }
        | boolean_type
        {
          $$ = idl_global->scopes()->bottom()->lookup_primitive_type($1);
        }
        | enum_type
        | scoped_name
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Decl              *d = NULL;
          AST_PredefinedType    *p = NULL;
          AST_Typedef           *t = NULL;
          bool found               = false;

          /*
           * The discriminator is a scoped name. Try to resolve to
           * one of the scalar types or to an enum. Thread through
           * typedef's to arrive at the base type at the end of the
           * chain
           */
          if (s != NULL && (d = s->lookup_by_name($1, true)) != NULL) {
            while (!found) {
              switch (d->node_type()) {
              case AST_Decl::NT_enum:
                $$ = d;
                found = true;
                break;
              case AST_Decl::NT_pre_defined:
                p = AST_PredefinedType::narrow_from_decl(d);
                if (p != NULL) {
                  switch (p->pt()) {
                  case AST_PredefinedType::PT_long:
                  case AST_PredefinedType::PT_ulong:
                  case AST_PredefinedType::PT_longlong:
                  case AST_PredefinedType::PT_ulonglong:
                  case AST_PredefinedType::PT_short:
                  case AST_PredefinedType::PT_ushort:
                  case AST_PredefinedType::PT_char:
                  case AST_PredefinedType::PT_wchar:
                  case AST_PredefinedType::PT_octet:
                  case AST_PredefinedType::PT_boolean:
                    $$ = p;
                    found = true;
                    break;
                  default:
                    $$ = NULL;
                    found = true;
                    break;
                  }
                }
                break;
              case AST_Decl::NT_typedef:
                t = AST_Typedef::narrow_from_decl(d);
                if (t != NULL) d = t->base_type();
                break;
              default:
                $$ = NULL;
                found = true;
                break;
              }
            }
          } else
            $$ = NULL;

          if ($$ == NULL)
            idl_global->err()->lookup_error($1);
        }
        ;

at_least_one_case_branch : case_branch case_branches ;

case_branches
        : case_branches case_branch
        | /* empty */
        ;

case_branch :
        at_least_one_case_label
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionLabelSeen);
        }
        element_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemSeen);
        }
        ';'
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_LabellistActiveIterator *l = NULL;
          AST_UnionLabel        *d = NULL;
          AST_UnionBranch       *b = NULL;
          AST_Field             *f = $3;

          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemCompleted);
          /*
           * Create several nodes representing branches of a union.
           * Add them to the enclosing scope (the union scope)
           */
          if (s != NULL && $1 != NULL && $3 != NULL) {
            l = new UTL_LabellistActiveIterator($1);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              b = idl_global->gen()->create_union_branch(d,
                                                      f->field_type(),
                                                      f->name(),
                                                      f->get_decl_pragmas());
              (void) s->fe_add_union_branch(b);
            }
            delete l;
          }
        }
        | error
        {
          idl_global->err()->syntax_error(idl_global->parse_state());
        }
        ';'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_NoState);
          yyerrok;
        }
        ;

at_least_one_case_label :
        case_label case_labels
        {
          $$ = new UTL_LabelList($1, $2);
        }
        ;

case_labels
        : case_labels case_label
        {
          if ($1 == NULL)
            $$ = new UTL_LabelList($2, NULL);
          else {
            $1->nconc(new UTL_LabelList($2, NULL));
            $$ = $1;
          }
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

case_label
        : DEFAULT
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_DefaultSeen);
        }
          ':'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

          $$ = idl_global->gen()->
                    create_union_label(AST_UnionLabel::UL_default,
                                       NULL);
        }
        | CASE
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_CaseSeen);
        }
          const_expr
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelExprSeen);
        }
        ':'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_LabelColonSeen);

          $$ = idl_global->gen()->create_union_label(AST_UnionLabel::UL_label,
                                                     $3);
        }
        ;

element_spec :
        type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemTypeSeen);
        }
        declarator
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          idl_global->set_parse_state(IDL_GlobalData::PS_UnionElemDeclSeen);
          /*
           * Check for illegal recursive use of type
           */
          if ($1 != NULL && AST_illegal_recursive_type($1))
            idl_global->err()->error1(UTL_Error::EIDL_RECURSIVE_TYPE, $1);
          /*
           * Create a field in a union branch
           */
          else if ($1 == NULL || $3 == NULL)
            $$ = NULL;
          else {
            AST_Type *tp = $3->compose($1);
            if (tp == NULL)
            {
              $$ = NULL;
            }
            else
            {
              // Check for anonymous type
              if ((tp->node_type () == AST_Decl::NT_array)
                  || (tp->node_type () == AST_Decl::NT_sequence))
              {
                 Identifier * id = $3->name ()->head ();
                 const char *postfix =
                    (tp->node_type () == AST_Decl::NT_array)
                    ? "" : "_seq";
                 // first underscore removed by Identifier constructor
                 DDS_StdString anon_type_name =
                    DDS_StdString ("__") + DDS_StdString (id->get_string ())
                       + DDS_StdString (postfix);
                 UTL_ScopedName *anon_scoped_name =
                 new UTL_ScopedName
                 (
                    new Identifier (os_strdup(anon_type_name)),
                    NULL
                 );

                 (void) s->fe_add_typedef
                 (
                    idl_global->gen()->create_typedef
                    (
                       tp,
                       anon_scoped_name,
                       s->get_pragmas ()
                    )
                 );
              }

              $$ = idl_global->gen()->create_field(tp,
                                                   $3->name(),
                                                   s->get_pragmas());
            }
          }
        }
        ;

enum_type :
        ENUM
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumSeen);
        }
        id
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName($3, NULL);
          AST_Enum              *e = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_EnumIDSeen);
          /*
           * Create a node representing an enum and add it to its
           * enclosing scope
           */
          if (s != NULL) {
            e = idl_global->gen()->create_enum(n, s->get_pragmas());
            /*
             * Add it to its defining scope
             */
            (void) s->fe_add_enum(e);
          }
          /*p
           * Push the enum scope on the scopes stack
           */
          idl_global->scopes()->push(e);
        }
        '{'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumSqSeen);
        }
        at_least_one_enumerator
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumBodySeen);
        }
        '}'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumQsSeen);
          /*
           * Done with this enum. Pop its scope from the scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            $$ = NULL;
          else {
            $$ = AST_Enum::narrow_from_scope(idl_global->scopes()->top_non_null());
            idl_global->scopes()->pop();
          }
        }
        ;

at_least_one_enumerator : enumerator enumerators ;

enumerators
        : enumerators
          ','
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_EnumCommaSeen);
        }
          enumerator
        | /* EMPTY */
        ;

enumerator :
        IDENTIFIER
        {
          if (idl_global->valid_identifier($1))
          {
            UTL_Scope             *s = idl_global->scopes()->top_non_null();
            UTL_ScopedName        *n =
                  new UTL_ScopedName (new Identifier ($1), NULL);
            AST_EnumVal           *e = NULL;
            AST_Enum              *c = NULL;

            /*
             * Create a node representing one enumerator in an enum
             * Add it to the enclosing scope (the enum scope)
             */
            if (s != NULL && s->scope_node_type() == AST_Decl::NT_enum) {
              c = AST_Enum::narrow_from_scope(s);
              if (c != NULL)
                e = idl_global->gen()->create_enum_val(c->next_enum_val(), n, s->get_pragmas());
              (void) s->fe_add_enum_val(e);
            }
          }
        }
        ;

sequence_type_spec
        : seq_head
          ','
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceCommaSeen);
        }
        positive_int_expr
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceExprSeen);
        }
          '>'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceQsSeen);
          /*
           * Remove sequence marker from scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            idl_global->scopes()->pop();
          /*
           * Create a node representing a sequence
           */
          if ($4 == NULL || $4->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error($4, AST_Expression::EV_ulong);
            $$ = NULL;
          } else if ($1 == NULL) {
            $$ = NULL;
          } else {
            AST_Type *tp = AST_Type::narrow_from_decl($1);
            if (tp == NULL)
              $$ = NULL;
            else {
              $$ = idl_global->gen()->create_sequence($4, tp);
              /*
               * Add this AST_Sequence to the types defined in the global scope
               */
              (void) idl_global->root()
                        ->fe_add_sequence(AST_Sequence::narrow_from_decl($$));
            }
          }
        }
        | seq_head
          '>'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceQsSeen);
          /*
           * Remove sequence marker from scopes stack
           */
          if (idl_global->scopes()->top() == NULL)
            idl_global->scopes()->pop();
          /*
           * Create a node representing a sequence
           */
          if ($1 == NULL)
            $$ = NULL;
          else {
            AST_Type *tp = AST_Type::narrow_from_decl($1);
            if (tp == NULL)
              $$ = NULL;
            else {
              $$ =
                idl_global->gen()->create_sequence(
                             idl_global->gen()->create_expr((unsigned long) 0),
                             tp);
              /*
               * Add this AST_Sequence to the types defined in the global scope
               */
              (void) idl_global->root()
                        ->fe_add_sequence(AST_Sequence::narrow_from_decl($$));
            }
          }
        }
        ;

seq_head:
        SEQUENCE
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSeen);
          /*
           * Push a sequence marker on scopes stack
           */
          idl_global->scopes()->push(NULL);
        }
        '<'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceSqSeen);
        }
        simple_type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_SequenceTypeSeen);
          $$ = $5;
        }
        ;

string_type_spec
        : string_head
          '<'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        }
        positive_int_expr
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        }
        '>'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
          /*
           * Create a node representing a string
           */
          if ($4 == NULL || $4->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error($4, AST_Expression::EV_ulong);
            $$ = NULL;
          } else {
            $$ = idl_global->gen()->create_string($4);
            /*
             * Add this AST_String to the types defined in the global scope
             */
            (void) idl_global->root()
                      ->fe_add_string(AST_String::narrow_from_decl($$));
          }
        }
        | string_head
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
          /*
           * Create a node representing a string
           */
          $$ =
            idl_global->gen()->create_string(
                         idl_global->gen()->create_expr((unsigned long) 0));
          /*
           * Add this AST_String to the types defined in the global scope
           */
          (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl($$));
        }
        ;

string_head:
        STRING
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        }
        ;

wstring_type_spec
        : wstring_head
          '<'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSqSeen);
        }
        positive_int_expr
        {
           idl_global->set_parse_state(IDL_GlobalData::PS_StringExprSeen);
        }
        '>'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringQsSeen);
          /*
           * Create a node representing a string
           */
          if ($4 == NULL || $4->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error($4, AST_Expression::EV_ulong);
            $$ = NULL;
          } else {
            $$ = idl_global->gen()->create_wstring($4);
            /*
             * Add this AST_String to the types defined in the global scope
             */
            (void) idl_global->root()
                      ->fe_add_string(AST_String::narrow_from_decl($$));
          }
        }
        | wstring_head
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringCompleted);
          /*
           * Create a node representing a string
           */
          $$ =
            idl_global->gen()->create_wstring(
                         idl_global->gen()->create_expr((unsigned long) 0));
          /*
           * Add this AST_String to the types defined in the global scope
           */
          (void) idl_global->root()
                    ->fe_add_string(AST_String::narrow_from_decl($$));
        }
        ;

wstring_head:
        WSTRING
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_StringSeen);
        }
        ;

array_declarator :
        id
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ArrayIDSeen);
        }
        at_least_one_array_dim
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ArrayCompleted);
          /*
           * Create a node representing an array
           */
          if ($3 != NULL) {
             $$ = idl_global->gen()->create_array(new UTL_ScopedName($1, NULL),
                                                  $3->length(), $3);
          }
        }
        ;

at_least_one_array_dim :
        array_dim array_dims
        {
          $$ = new UTL_ExprList($1, $2);
        }
        ;

array_dims
        : array_dims array_dim
        {
          if ($1 == NULL)
            $$ = new UTL_ExprList($2, NULL);
          else {
            $1->nconc(new UTL_ExprList($2, NULL));
            $$ = $1;
          }
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

array_dim :
        '['
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_DimSqSeen);
        }
        positive_int_expr
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_DimExprSeen);
        }
        ']'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_DimQsSeen);
          /*
           * Array dimensions are expressions which must be coerced to
           * positive integers
           */
          if ($3 == NULL || $3->coerce(AST_Expression::EV_ulong) == NULL) {
            idl_global->err()->coercion_error($3, AST_Expression::EV_ulong);
            $$ = NULL;
          } else
            $$ = $3;
        }
        ;

attribute:
        opt_readonly
        ATTRIBUTE
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrSeen);
        }
        simple_type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_AttrTypeSeen);
        }
        at_least_one_declarator
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_DecllistActiveIterator *l = NULL;
          AST_Attribute         *a = NULL;
          FE_Declarator         *d = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_AttrCompleted);
          /*
           * Create nodes representing attributes and add them to the
           * enclosing scope
           */
          if (s != NULL && $4 != NULL && $6 != NULL) {
            l = new UTL_DecllistActiveIterator($6);
            for (;!(l->is_done()); l->next()) {
              d = l->item();
              if (d == NULL)
                continue;
              AST_Type *tp = d->compose($4);
              if (tp == NULL)
                continue;
              a = idl_global->gen()->create_attribute($1, tp, d->name(), s->get_pragmas());
              /*
               * Add one attribute to the enclosing scope
               */
              (void) s->fe_add_attribute(a);
            }
            delete l;
          }
        }
        ;

opt_readonly
        : READONLY
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_AttrROSeen);
          $$ = true;
        }
        | /* EMPTY */
        {
          $$ = false;
        }
        ;

/*
   opt_local_or_abstract returns:

      'L' for local interface
      'A' for abstract interface
      'I' for interface
*/

opt_local_or_abstract :
        LOCAL
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_LocalSeen);
          $$ = 'L';
        }
        | ABSTRACT
        {
          idl_global->set_parse_state (IDL_GlobalData::PS_AbstractSeen);
          $$ = 'A';
        }
        | /* EMPTY */
        {
          $$ = 'I';
        }
        ;

exception :
        EXCEPTION
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSeen);
        }
        id
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          UTL_ScopedName        *n = new UTL_ScopedName($3, NULL);
          AST_Exception         *e = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptIDSeen);
          /*
           * Create a node representing an exception and add it to
           * the enclosing scope
           */
          if (s != NULL) {
            e = idl_global->gen()->create_exception(n, s->get_pragmas());
            (void) s->fe_add_exception(e);
          }
          /*
           * Push the exception scope on the scope stack
           */
          idl_global->scopes()->push(e);
        }
        '{'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptSqSeen);
        }
        members
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptBodySeen);
        }
        '}'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_ExceptQsSeen);
          /*
           * Done with this exception. Pop its scope from the scope stack
           */
          idl_global->scopes()->pop();
        }
        ;

operation :
        opt_op_attribute
        op_type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpTypeSeen);
        }
        IDENTIFIER
        {
          if (idl_global->valid_identifier($4))
          {
            UTL_Scope             *s = idl_global->scopes()->top_non_null();
            UTL_ScopedName        *n =
                  new UTL_ScopedName (new Identifier ($4), NULL);
            AST_Operation         *o = NULL;

            idl_global->set_parse_state(IDL_GlobalData::PS_OpIDSeen);
            /*
             * Create a node representing an operation on an interface
             * and add it to its enclosing scope
             */
            if (s != NULL && $2 != NULL) {
              AST_Type *tp = AST_Type::narrow_from_decl($2);
              if (tp == NULL) {
                idl_global->err()->not_a_type($2);
              } else if (tp->node_type() == AST_Decl::NT_except) {
                idl_global->err()->not_a_type($2);
              } else {
                o = idl_global->gen()->create_operation(tp, $1, n, s->get_pragmas());
                (void) s->fe_add_operation(o);
              }
            }
            /*
             * Push the operation scope onto the scopes stack
             */
            idl_global->scopes()->push(o);
          }
        }
        parameter_list
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParsCompleted);
        }
        opt_raises
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseCompleted);
        }
        opt_context
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Operation         *o = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_OpCompleted);
          /*
           * Add exceptions and context to the operation
           */
          if (s != NULL && s->scope_node_type() == AST_Decl::NT_op) {
            o = AST_Operation::narrow_from_scope(s);

            if ($8 != NULL && o != NULL)
              (void) o->fe_add_exceptions($8);
            if ($10 != NULL)
              (void) o->fe_add_context($10);
          }
          /*
           * Done with this operation. Pop its scope from the scopes stack
           */
          idl_global->scopes()->pop();
        }
        ;

opt_op_attribute
        : ONEWAY
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
          $$ = AST_Operation::OP_oneway;
        }
        | IDEMPOTENT
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpAttrSeen);
          $$ = AST_Operation::OP_idempotent;
        }
        | /* EMPTY */
        {
          $$ = AST_Operation::OP_noflags;
        }
        ;

op_type_spec
        : simple_type_spec
        | VOID
        {
          $$ =
            idl_global->scopes()->bottom()
               ->lookup_primitive_type(AST_Expression::EV_void);
        }
        ;

parameter_list
        : '('
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        }
          ')'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        }
        | '('
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpSqSeen);
        }
          at_least_one_parameter
          ')'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpQsSeen);
        }
        ;

at_least_one_parameter : parameter parameters ;

parameters
        : parameters
          ','
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParCommaSeen);
        }
          parameter
        | /* EMPTY */
        ;

parameter :
        direction
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParDirSeen);
        }
        simple_type_spec
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpParTypeSeen);
        }
        declarator
        {
          UTL_Scope             *s = idl_global->scopes()->top_non_null();
          AST_Argument          *a = NULL;

          idl_global->set_parse_state(IDL_GlobalData::PS_OpParDeclSeen);
          /*
           * Create a node representing an argument to an operation
           * Add it to the enclosing scope (the operation scope)
           */
          if ($3 != NULL && $5 != NULL && s != NULL) {
            AST_Type *tp = $5->compose($3);
            if (tp != NULL) {
              a = idl_global->gen()->create_argument($1, tp, $5->name(), s->get_pragmas());
              (void) s->fe_add_argument(a);
            }
          }
        }
        ;

direction
        : IN
        {
          $$ = AST_Argument::dir_IN;
        }
        | OUT
        {
          $$ = AST_Argument::dir_OUT;
        }
        | INOUT
        {
          $$ = AST_Argument::dir_INOUT;
        }
        ;

opt_raises
        : RAISES
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSeen);
        }
          '('
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseSqSeen);
        }
          at_least_one_scoped_name
          ')'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpRaiseQsSeen);
          $$ = $5;
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

opt_context
        : IDL_CONTEXT
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSeen);
        }
          '('
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextSqSeen);
        }
          at_least_one_string_literal
          ')'
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextQsSeen);
          $$ = $5;
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

at_least_one_string_literal :
        STRING_LITERAL string_literals
        {
          $$ = new UTL_StrList($1, $2);
        }
        ;

string_literals
        : string_literals
          ','
        {
          idl_global->set_parse_state(IDL_GlobalData::PS_OpContextCommaSeen);
        }
          STRING_LITERAL
        {
          if ($1 == NULL)
            $$ = new UTL_StrList($4, NULL);
          else {
            $1->nconc(new UTL_StrList($4, NULL));
            $$ = $1;
          }
        }
        | /* EMPTY */
        {
          $$ = NULL;
        }
        ;

%%
/* programs */

/*
 * ???
 */
int yywrap ()
{
  return 1;
}

/*
 * Report an error situation discovered in a production
 *
 * This does not do anything since we report all error situations through
 * idl_global->err() operations
 */
void yyerror (const char *)
{
}
