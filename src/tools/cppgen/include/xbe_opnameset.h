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
