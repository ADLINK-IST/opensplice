#ifndef _XBE_EXCEPTIONLIST_H
#define _XBE_EXCEPTIONLIST_H

#include <xps_vector.h>

class be_exception;

// an DDSVector of be_Arguments; includes functions for querying the list

class be_ExceptionList: public DDSVector<be_exception*>
{
   public:
      be_ExceptionList()
      {
      }

      be_ExceptionList(const TList<be_exception*>& oldlist)
      {
         TList<be_exception*>::iterator iter;

         for (iter = oldlist.begin(); iter != oldlist.end(); ++iter)
         {
            push_back (*iter);
         }
      }

      be_ExceptionList(const be_ExceptionList& that)
         : DDSVector<be_exception*>(that)
      {
      }
};

#endif // _XBE_ARGLIST_H
