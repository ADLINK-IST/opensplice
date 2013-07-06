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
