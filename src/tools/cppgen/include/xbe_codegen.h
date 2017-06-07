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
#ifndef _XBE_CODE_GENERATOR_H_
#define _XBE_CODE_GENERATOR_H_

#include "idl_narrow.h"
#include "xbe_source.h"

/*
 * BE_CodeGenerator
 */

class be_CodeGenerator
{

public:

   be_CodeGenerator():m_at_module_scope(false) {}

   virtual ~be_CodeGenerator()
   {}

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& clientHeader);
   virtual void Generate(be_ClientImplementation& clientImpl);
   virtual void Generate(be_ServerHeader& serverHeader);
   virtual void Generate(be_ServerImplementation& serverImpl);
   virtual void GenerateGlobal (be_ClientHeader& clientHeader);

   virtual void isAtModuleScope(bool is_at_module);  
   virtual bool isAtModuleScope() const;
   bool m_at_module_scope;
   static bool is_local_type(AST_Decl * d);

   DEF_NARROW_METHODS0(be_CodeGenerator);
};

inline void
be_CodeGenerator::isAtModuleScope(bool is_at_module)
{                                                        
   m_at_module_scope = is_at_module;
}

inline bool
be_CodeGenerator::isAtModuleScope() const
{
   return m_at_module_scope;
}

// anything that generates client-side code

class be_ClientGenerator
{

public:

   virtual ~be_ClientGenerator()
   {}

   virtual void Generate(be_ClientHeader& clientHeader) = 0;
   virtual void Generate(be_ClientImplementation& clientImpl) = 0;
};

// anything that generates server-side code

class be_ServerGenerator
{

public:

   virtual ~be_ServerGenerator()
   {}

   virtual void Generate(be_ServerHeader& serverHeader) = 0;
   virtual void Generate(be_ServerImplementation& serverImpl) = 0;
};


#endif // _XBE_CODE_GENERATOR_H_
