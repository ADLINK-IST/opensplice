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
#ifndef _XBE_ARGLIST_H
#define _XBE_ARGLIST_H

#include "xps_vector.h"
#include "xbe_argument2.h"

class be_argument;

// an DDSVector of be_Arguments; includes functions for querying the list

class be_ArgumentList : public DDSVector<be_Argument>
{
public:

   be_ArgumentList () {};

   be_ArgumentList (const be_Argument & arg1); // create a 1-element list
   be_ArgumentList (const TList<be_argument*>& oldlist);
   be_ArgumentList (const be_ArgumentList& that)
      : DDSVector<be_Argument>(that)
   {};

   unsigned int NumArgsSentFromClient () const;
   unsigned int NumArgsSentFromServer () const;
};

#endif // _XBE_ARGLIST_H
