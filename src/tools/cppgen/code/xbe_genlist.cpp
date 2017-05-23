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
#include "xbe_genlist.h"

be_GeneratorList g_generatorList;

void
be_GeneratorList::Add(be_CodeGenerator* codegen)
{
   m_list.push_back (codegen);
}

void
be_GeneratorList::GenerateGlobal (be_ClientHeader& source)
{
   for (unsigned int i = 0; i < m_list.size(); i++)
   {
      m_list[i]->GenerateGlobal (source);
   }
}
