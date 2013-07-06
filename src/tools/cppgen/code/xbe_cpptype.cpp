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
#include "xbe_cpptype.h"
#include "xbe_globals.h"

be_CppName
be_CppType::ScopedName() const
{
   DDSString result;

   if (m_enclosingScope.ContainsAtLeastOneName())
   {
      result = m_enclosingScope.ToString() + BE_Globals::ScopeSeparator();
   }

   result += m_name;

   return result;
}

be_CppName
be_CppType::LowestLevelName(UTL_ScopedName& utlScopedName)
{
   DDSString result;

   const int indexOfLowestLevelName = utlScopedName.length() - 1;
   // the name we want is the last one in the UTL_ScopedName list

   if (indexOfLowestLevelName < 0) // if the list has no elements
   {
      return result;  // return null result
   }

   int index = 0;
   UTL_ScopedNameActiveIterator iter(&utlScopedName);

   while (index < indexOfLowestLevelName)
   {
      iter.next();
      index++;
   }

   assert (index == indexOfLowestLevelName);
   result = iter.item()->get_string();
   return result;
}
