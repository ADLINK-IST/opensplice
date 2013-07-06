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
#ifndef _AST_STATE_MEMBER_HH
#define _AST_STATE_MEMBER_HH


// Representation of attribute declaration:
//
// An attribute is a field with a readonly property

/*
** DEPENDENCIES: ast_field.hh, ast_type.hh, utl_scoped_name.hh,
**   utl_strlist.hh, ast_decl.hh
**
** USE: Included from ast.h
*/

class AST_StateMember : public virtual AST_Field
{

public:
   // Operations

   // Constructor(s)
   AST_StateMember();
   AST_StateMember
   (
      idl_bool public_access,
      AST_Type *ft,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );
   virtual ~AST_StateMember()
   {}

   // Data Accessors
   idl_bool public_access ();

   // Narrowing
   DEF_NARROW_METHODS1(AST_StateMember, AST_Field);
   DEF_NARROW_FROM_DECL(AST_StateMember);

   // AST Dumping
   virtual void dump(ostream &o);

private:
   // Data
   const idl_bool pd_public_access; // Is public or private
};

#endif           // _AST_STATE_MEMBER_HH
