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
#ifndef XBE_GENLIST_HH_INCLUDED
#define XBE_GENLIST_HH_INCLUDED

#include "idl.h"
#include "xbe_codegen.h"
#include "xbe_cppscope.h"

class be_GeneratorList
{
   public:
      void Add(be_CodeGenerator* codegen);
      void GenerateGlobal (be_ClientHeader& source);

   private:
      DDSVector<be_CodeGenerator*> m_list;
};

extern be_GeneratorList g_generatorList;

#endif // XBE_GENLIST_HH_INCLUDED
