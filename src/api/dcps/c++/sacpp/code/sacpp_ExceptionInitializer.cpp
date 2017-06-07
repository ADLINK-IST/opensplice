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
#include "sacpp_ExceptionInitializer.h"

DDS::ExceptionInitializer *DDS::ExceptionInitializer::m_head = 0;

DDS::ExceptionInitializer::ExceptionInitializer 
   (const char *name, Factory fact)
:
   m_name (name),
   m_fact (fact)
{
   if (lookup (name) == 0)
   {
      m_next = m_head;
      m_head = this;
   }
}

DDS::ExceptionInitializer::Factory DDS::ExceptionInitializer::lookup (const char *name)
{
   for (ExceptionInitializer *node = m_head; node != 0; node = node->m_next)
   {
      if (DDS::string_cmp (node->m_name, name) == 0)
      {
         return node->m_fact;
      }
   }
   return 0;
}
