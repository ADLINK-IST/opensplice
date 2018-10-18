/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "xbe_opnameset.h"

DDSHashValue std_strhash(const DDS_StdString& str)
{
   const long p = 1073741827L;  // prime
   const char* d = (const char *)str;
   int n = strlen(d);
   long h = 0;
   unsigned retval = 0;

   for (int i = 0; i < n; ++i, ++d)
   {
      h = (h << 2) + *d;
   }

   retval = ((h >= 0) ? (h % p) : ( -h % p));

   return retval;
}

be_OpMapElement::be_OpMapElement
(
   DDS_StdString implname,
   DDS_StdString opname,
   DDS_StdString opdispatcher,
   DDS_StdString opDispatchName,
   const AST_Decl* opdecl
)
:
   m_implname (implname),
   m_opname (opname),
   m_opdispatcher (opdispatcher),
   m_opDispatchName (opDispatchName),
   m_opdecl (opdecl),
   m_remaining (true)
{}

// -------------------------------------------------
//  be_OpNameSet IMPLEMENTATION
// -------------------------------------------------
be_OpNameSet::be_OpNameSet ()
:
   m_opMap(std_strhash),
   m_minLength(0),
   m_maxLength(0)
{}

be_OpNameSet::~be_OpNameSet()
{
   //
   // clean m_opMap
   //
   m_opMap.erase(m_opMap.begin(), m_opMap.end());
}

void be_OpNameSet::AddOpName
(
   const DDS_StdString& implname,
   const DDS_StdString& opname,
   const DDS_StdString& opdispatcher,
   const DDS_StdString& opDispatchName,
   const AST_Decl* opdecl
)
{
   be_OpMapElement newElem (implname, opname, opdispatcher, opDispatchName, opdecl);

   if (opDispatchName)
   {
      DDS::ULong len = strlen(opDispatchName);

      if (!m_minLength)
      {
         m_minLength = m_maxLength = len;
      }
      else
      {
         if (len < m_minLength)
         {
            m_minLength = len;
         }
         else if (len > m_maxLength)
         {
            m_maxLength = len;
         }
      }

      m_opMap[newElem.m_opDispatchName] = newElem;
   }
}

be_OpNameSet::Be_OpMap::iterator
be_OpNameSet::find(const DDS_StdString & opname) const
{
   return ((be_OpNameSet*)this)->m_opMap.find(opname);
}

be_OpNameSet::Be_OpMap::iterator
be_OpNameSet::begin() const
{
   return ((be_OpNameSet*)this)->m_opMap.begin();
}

be_OpNameSet::Be_OpMap::iterator
be_OpNameSet::end() const
{
   return ((be_OpNameSet*)this)->m_opMap.end();
}
