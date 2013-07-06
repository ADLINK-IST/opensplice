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
#ifndef _XBE_CONSTANT_HH
#define _XBE_CONSTANT_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"

class be_constant
   : public virtual AST_Constant, public be_CodeGenerator
{

private:

   DDS_StdString typeName;
   pbbool IsGlobalScope ();

public:

   be_constant ();
   be_constant
   (
      AST_Expression::ExprType et,
      AST_Expression * v,
      UTL_ScopedName * n,
      const UTL_Pragmas & p
   );

   void GenerateDecl (be_Source& source);
   const DDS_StdString & TypeName ();
   DDS_StdString QuotedConstValue ();
   DDS_StdString EndValueType ();

   // be_codegenerator virtuals

   virtual void Generate (be_ClientHeader & clientHeader);
   virtual void Generate (be_ClientImplementation & clientImpl);
   virtual void Generate (be_ServerHeader &) {}
   virtual void Generate (be_ServerImplementation &) {}

   // narrowing

   DEF_NARROW_METHODS2 (be_constant, AST_Constant, be_CodeGenerator);
   DEF_NARROW_FROM_DECL (be_constant);
};

#endif
