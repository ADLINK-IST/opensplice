/*
 * ast_interface_fwd.cc - Implementation of class AST_InterfaceFwd
 *
 * AST_InterfaceFwd nodes denote forward declarations of IDL interfaces
 * AST_InterfaceFwd nodes have a field containing the full declaration
 * of the interface, which is initialized when that declaration is
 * encountered.
 */

#include <idl.h>
#include <idl_extern.h>

/*
 * Constructor(s) and destructor
 */
AST_InterfaceFwd::AST_InterfaceFwd ()
   : pd_next (0), pd_full_definition (0)
{}

AST_InterfaceFwd::AST_InterfaceFwd
(
   bool local,
   bool abstract,
   UTL_ScopedName * n,
   const UTL_Pragmas & p
)
: AST_Decl (AST_Decl::NT_interface_fwd, n, p)
{
   /*
    * Create a dummy placeholder for the forward declared interface. This
    * interface node is not yet defined (n_inherits < 0), so some operations
    * will fail
    */
   pd_full_definition = idl_global->gen()->create_interface
      (local, abstract, n, NULL, -1, p);
   /*
    * Record the node in a list to be checked after the entire AST has been
    * parsed. All nodes in the list must have n_inherits >= 0, else this
    * indicates that a full definition was not seen for this forward
    * delcared interface
    */
   AST_record_fwd_interface(this);
}

void AST_InterfaceFwd::dump (ostream &o)
{
   o << "interface ";
   local_name()->dump(o);
}

AST_Interface * AST_InterfaceFwd::full_definition ()
{
   return pd_full_definition;
}

void AST_InterfaceFwd::set_full_definition (AST_Interface *nfd)
{
   pd_full_definition = nfd;
}

IMPL_NARROW_METHODS1(AST_InterfaceFwd, AST_Type)
IMPL_NARROW_FROM_DECL(AST_InterfaceFwd)
