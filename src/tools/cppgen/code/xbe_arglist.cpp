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
