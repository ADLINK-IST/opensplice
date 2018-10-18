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
#ifndef _XBE_OPNAMESET_HH
#define _XBE_OPNAMESET_HH

#include "idl.h"
#include "StdString.h"
#include "ast_decl.h"
#include "xbe_hash.h"

class be_OpMapElement
{
public:

   DDS_StdString m_implname; // Name of implementing class
   DDS_StdString m_opname;  // The name to be generated.
   DDS_StdString m_opdispatcher;  // The name of the _dispatch operation
   DDS_StdString m_opDispatchName;  // The name sent across the wire.
   const AST_Decl* m_opdecl;
   DDS::ULong m_ophash;
   bool m_remaining; // still waiting to do something with this element?

   be_OpMapElement () {}

   be_OpMapElement
   (
      DDS_StdString implname,
      DDS_StdString opname,
      DDS_StdString opdispatcher,
      DDS_StdString opDispatchName,
      const AST_Decl* opdecl
   );
};

class be_OpNameSet
{
public:

   typedef DDSMap<DDS_StdString, be_OpMapElement> Be_OpMap;
   typedef Be_OpMap::iterator OnsIterator;

   be_OpNameSet();
   ~be_OpNameSet();

   void AddOpName
   (
      const DDS_StdString& implname,
      const DDS_StdString& opname,
      const DDS_StdString& opdisp,
      const DDS_StdString& opdispname,
      const AST_Decl* decl
   );

   OnsIterator find(const DDS_StdString& opname) const;
   OnsIterator begin() const;
   OnsIterator end() const;
   DDS::ULong MinWordLength() const;
   DDS::ULong MaxWordLength() const;
   DDS::ULong size() const;

private:
   Be_OpMap m_opMap;
   DDS::ULong m_minLength;
   DDS::ULong m_maxLength;
};

inline DDS::ULong
be_OpNameSet::MinWordLength() const
{
   return m_minLength;
}

inline DDS::ULong
be_OpNameSet::MaxWordLength() const
{
   return m_maxLength;
}

inline DDS::ULong
be_OpNameSet::size() const
{
   return ((be_OpNameSet*)this)->m_opMap.size();
}

#endif // _XBE_OPNAMESET_HH
