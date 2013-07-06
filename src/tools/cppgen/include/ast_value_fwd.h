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
#ifndef _AST_VALUE_FWD_HH
#define _AST_VALUE_FWD_HH


// Representation of a forward interface declaration

/*
** DEPENDENCIES: ast_decl.hh, ast_interface.hh, utl_scoped_name.hh,
**   utl_strlist.hh
**
** USE: Included from ast.hh
*/

class AST_ValueFwd : public virtual AST_Type
{

public:
   // Operations

   // Constructor(s)
   AST_ValueFwd();
   AST_ValueFwd
   (
      idl_bool abstract,
      UTL_ScopedName *n, 
      const UTL_Pragmas &p
   );
   virtual ~AST_ValueFwd()
   {}

   // Data Accessors
   AST_Value *full_definition();
   void set_full_definition(AST_Value *val);

   // Narrowing
   DEF_NARROW_METHODS1(AST_ValueFwd, AST_Type);
   DEF_NARROW_FROM_DECL(AST_ValueFwd);

   // AST Dumping
   virtual void dump(ostream &);

private:
   // Data
   AST_Value *pd_full_definition; // The value this is a forward declaration of
};

#endif           // _AST_VALUE_FWD_HH
