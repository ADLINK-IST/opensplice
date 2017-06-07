/*
 * ast_check.cc - Check AST nodes representing fwd declared interfaces
 * structs and unions after parse of AST is completed.
 *
 * The check ensures that for every forward declared type we also
 * have a full definition of that type.
 */

#include "idl.h"
#include "idl_extern.h"

/*
 * Static storage for remembering nodes
 */

static AST_InterfaceFwd * ast_fwd_interfaces = 0;

/*
 * Store a node representing a forward declared type
 */

void AST_record_fwd_interface (AST_InterfaceFwd * i)
{
   i->pd_next = ast_fwd_interfaces;
   ast_fwd_interfaces = i;
}

/*
 * Check that all forward declared types were also defined
 */

void AST_check_fwds ()
{
   AST_InterfaceFwd * fi = ast_fwd_interfaces;

   AST_Interface * ri;

   while (fi)
   {
      ri = fi->full_definition ();
      if (!(ri->is_defined ()))
      {
         idl_global->err()->fwd_decl_not_defined (ri);
      }
      fi = fi->pd_next;
   }
}
