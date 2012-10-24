/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef CCPP_CONDITION_H
#define CCPP_CONDITION_H

#include "gapi.h"
#include "ccpp.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API Condition_impl
      : public virtual ::DDS::Condition,
        public LOCAL_REFCOUNTED_OBJECT
  {
    friend class WaitSet;

    protected:
      gapi_condition _gapi_self;

      Condition_impl(
        gapi_condition _gapi_handle
      );

    public:
      CORBA::Boolean get_trigger_value () THROW_ORB_EXCEPTIONS;
  };
}

#endif /* CCPP_CONDITION_H */
