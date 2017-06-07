#ifndef _AST_INTERFACE_FWD_AST_INTERFACE_FWD_HH
#define _AST_INTERFACE_FWD_AST_INTERFACE_FWD_HH

// Representation of a forward interface declaration

/*
** DEPENDENCIES: ast_decl.hh, ast_interface.hh, utl_scoped_name.hh,
**   utl_strlist.hh
**
** USE: Included from ast.hh
*/

class AST_InterfaceFwd : public virtual AST_Type
{
public:

   AST_InterfaceFwd ();
   AST_InterfaceFwd 
   (
      bool local,
      bool abstract,
      UTL_ScopedName * n, 
      const UTL_Pragmas & p
   );

   virtual ~AST_InterfaceFwd () {}

   AST_Interface *full_definition ();
   void set_full_definition (AST_Interface *nfd);

   DEF_NARROW_METHODS1 (AST_InterfaceFwd, AST_Type);
   DEF_NARROW_FROM_DECL (AST_InterfaceFwd);

   virtual void dump (ostream &);

   AST_InterfaceFwd * pd_next;

private:

   AST_Interface *pd_full_definition;
};

#endif
