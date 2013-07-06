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
#include "xbe_globals.h"
#include "xbe_cppscope.h"

DDSString be_CppEnclosingScope::ToString () const
{
   DDSString result;

   DDSVector<be_CppName>::const_iterator iter;
   bool firstTime = true;

   for (iter = m_nameList.begin(); iter != m_nameList.end(); ++iter)
   {
      if (!firstTime)
      {
         result += BE_Globals::ScopeSeparator ();
      }

      firstTime = false;

      result += *iter;
   }

   return result;
}

DDSVector<be_CppName> be_CppEnclosingScope::ExtractScope 
   (UTL_ScopedName& utlScopedName)
{
   DDSVector<be_CppName> result;

   UTL_ScopedNameActiveIterator iter(&utlScopedName);
   int numAppendedSoFar = 0;
   const int numToAppend = utlScopedName.length() - 1;

   // append all but the last element of the list in utlScopedName,
   // which by definition doesn't enclose

   while (numAppendedSoFar < numToAppend)
   {
      result.push_back(be_CppName(iter.item()->get_string()));

      iter.next();
      numAppendedSoFar++;
   }

   return result;
}


DDSVector<be_CppName> be_CppEnclosingScope::FullNameToScope
   (UTL_ScopedName & utlScopedName)
{
   DDSVector<be_CppName> result;

   UTL_ScopedNameActiveIterator iter (&utlScopedName);

   while (!iter.is_done())
   {
      result.push_back (be_CppName (iter.item()->get_string ()));
      iter.next ();
   }

   return result;
}

DDSVector<be_CppName>
be_CppEnclosingScope::ScopeListToScope
   (const DDSVector<be_CppEnclosingScope>& scopeList)
{
   DDSVector<be_CppName> result;

   DDSVector<be_CppEnclosingScope>::const_iterator listIter;

   for (listIter = scopeList.begin();
         listIter != scopeList.end();
         ++listIter)
   {
      const be_CppEnclosingScope& cppScope = *listIter;

      const be_CppName& cppName =
         cppScope.m_nameList[cppScope.m_nameList.size () - 1];
      result.push_back(cppName);
   }

   return result;
}

DDSVector<be_CppName> be_CppEnclosingScope::ScopePlusFullName
(
   const be_CppEnclosingScope & cppScope,
   UTL_ScopedName & utlScopedName
)
{
   DDSVector<be_CppName> result = cppScope.m_nameList;

   UTL_ScopedNameActiveIterator iter(&utlScopedName);

   while (!iter.is_done())
   {
      result.push_back(be_CppName(iter.item()->get_string()));
      iter.next();
   }

   return result;
}

bool be_CppEnclosingScope::operator == (const be_CppEnclosingScope& that) const
{
   if (m_nameList.size () != that.m_nameList.size ())
   {
      return false;
   }

   for (unsigned int i = 0; i < m_nameList.size (); i++)
   {
      if (m_nameList[i] != that.m_nameList[i])
      {
         return false;
      }
   }

   return true;
}
