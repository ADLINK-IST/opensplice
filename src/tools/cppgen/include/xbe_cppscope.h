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
#ifndef _XBE_CPPSCOPE_H
#define _XBE_CPPSCOPE_H

#include "idl.h"
#include "xps_vector.h"
#include "xbe_cppname.h"
#include "xbe_utils.h"

// A be_CppEnclosingScope is a C++ scope that encloses a variable or type.

class be_CppEnclosingScope
{
public:

   // dummy class for distinguishing constructors

   class NameIsScope {}; 

   be_CppEnclosingScope () : m_nameList () {}

   // grab just the enclosing scope from utlScopedName; leave out the
   // name (the last element of utlScopedName)

   be_CppEnclosingScope (UTL_ScopedName & utlScopedName)
      : m_nameList (ExtractScope (utlScopedName))
   {}

   // utlScopedName is itself a scope

   be_CppEnclosingScope (UTL_ScopedName& utlScopedName, const NameIsScope&)
      : m_nameList (FullNameToScope (utlScopedName))
   {}

   // concatenate a vector of be_CppEnclosingScopes

   be_CppEnclosingScope (const DDSVector<be_CppEnclosingScope> & scopeList)
      : m_nameList (ScopeListToScope (scopeList))
   {}

   // concatenate a be_CppEnclosingScope and a UTL_ScopedName,
   // including the latter's last element

   be_CppEnclosingScope
   (
      const be_CppEnclosingScope & cppScope,
      UTL_ScopedName & utlScopedName
   )
      : m_nameList (ScopePlusFullName (cppScope, utlScopedName))
   {}

   be_CppEnclosingScope (const be_CppEnclosingScope & that)
      : m_nameList (that.m_nameList)
   {}

   DDSString ToString () const;

   inline DDS::ULong size () const
   {
      return m_nameList.size ();
   }

   inline bool ContainsAtLeastOneName () const
   {
      return m_nameList.size () > 0;
   }

   bool operator == (const be_CppEnclosingScope & that) const;
   inline bool operator != (const be_CppEnclosingScope & that) const
   {
      return !(*this == that);
   }

private:

   const DDSVector<be_CppName> m_nameList;

   // DO NOT ASSIGN  (reason: just didn't implement it; copy constructor
   // seems to be enough)

   be_CppEnclosingScope& operator=(const be_CppEnclosingScope& that);

   // PRIVATE HELPERS

   static DDSVector<be_CppName> ExtractScope(UTL_ScopedName& utlScopedName);

   static DDSVector<be_CppName> FullNameToScope(UTL_ScopedName& utlScopedName);

   static DDSVector<be_CppName> ScopeListToScope(
      const DDSVector<be_CppEnclosingScope>& scopeList);

   static DDSVector<be_CppName> ScopePlusFullName(
      const be_CppEnclosingScope& cppScope, UTL_ScopedName& utlScopedName);
};

#endif // _XBE_CPPSCOPE_H
