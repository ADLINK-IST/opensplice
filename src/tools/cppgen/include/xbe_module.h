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
#ifndef _XBE_MODULE_HH
#define _XBE_MODULE_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"

class be_module
:
   public virtual AST_Module,
   public be_CodeGenerator
{

public:

   be_module ();
   be_module (UTL_ScopedName *n, const UTL_Pragmas &p);

   // BE_CODEGENERATOR VIRTUALS

   virtual void Generate (be_ClientHeader& clientHeader);
   virtual void Generate (be_ClientImplementation& clientImpl);
   virtual void Generate (be_ServerHeader& serverHeader);
   virtual void Generate (be_ServerImplementation& serverImpl);

   // NARROWING

   DEF_NARROW_METHODS2(be_module, AST_Module, be_CodeGenerator);
   DEF_NARROW_FROM_DECL(be_module);
   DEF_NARROW_FROM_SCOPE(be_module);

private:

   const be_CppEnclosingScope m_cppScope;
};

#endif
