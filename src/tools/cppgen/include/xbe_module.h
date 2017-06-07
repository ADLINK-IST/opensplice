/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
