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
#ifndef _FE_VALUE_HEADER_FE_VALUE_HH
#define _FE_VALUE_HEADER_FE_VALUE_HH


// FE_ValueHeader
//
// Internal class for FE to describe value headers

/*
** DEPENDENCIES: utl_scoped_name.hh, ast_value.hh, utl_scope.hh,
**   ast_decl.hh
**
** USE: Included from fe.hh
*/

class FE_ValueHeader
{

public:
   // Operations

   // Constructor(s)
   FE_ValueHeader
   (
      idl_bool custom,
      UTL_ScopedName *n,
      FE_ValueInheritanceSpec *inherits
   );
   virtual ~FE_ValueHeader()
   {}

   // Data Accessors
   UTL_ScopedName *value_name();
   AST_Value **inherits();
   long n_inherits();
   AST_Interface **supports();
   long n_supports();
   idl_bool custom();
   idl_bool truncatable();

private:
   // Data
   UTL_ScopedName *pd_value_name;
   FE_ValueInheritanceSpec *pd_inherits;
   idl_bool pd_custom;
};

#endif           // _FE_VALUE_HEADER_FE_VALUE_HH
