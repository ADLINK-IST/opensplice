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
