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
