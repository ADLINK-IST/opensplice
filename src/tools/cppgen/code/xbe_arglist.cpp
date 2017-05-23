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
#include "xbe_arglist.h"

be_ArgumentList::be_ArgumentList(const TList<be_argument*>& oldlist)
{
   TList<be_argument*>::iterator iter;

   for (iter = oldlist.begin(); iter != oldlist.end(); ++iter)
   {
      const be_argument& arg = *(*iter);
      push_back(be_Argument(arg));
   }
}

be_ArgumentList::be_ArgumentList(const be_Argument& arg1)
{
   push_back(arg1);
}

unsigned int
be_ArgumentList::NumArgsSentFromClient() const
{
   unsigned int result = 0;

   const_iterator iter;

   for (iter = begin(); iter != end(); ++iter)
   {
      const be_Argument& arg = *iter;

      if (arg.IsSentFromClient())
      {
         ++result;
      }
   }

   return result;
}

unsigned int
be_ArgumentList::NumArgsSentFromServer() const
{
   unsigned int result = 0;

   const_iterator iter;

   for (iter = begin(); iter != end(); ++iter)
   {
      const be_Argument& arg = *iter;

      if (arg.IsSentFromServer())
      {
         ++result;
      }
   }

   return result;
}
