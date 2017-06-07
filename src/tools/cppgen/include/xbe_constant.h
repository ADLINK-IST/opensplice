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
