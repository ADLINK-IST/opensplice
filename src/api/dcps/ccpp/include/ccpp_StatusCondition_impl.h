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
#ifndef CCPP_STATUSCONDITION_H
#define CCPP_STATUSCONDITION_H

#include "ccpp.h"
#include "ccpp_Condition_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API StatusCondition_impl
    :
      public virtual ::DDS::StatusCondition,
      public ::DDS::Condition_impl
  {
    friend class WaitSet_impl;

    private:
      os_mutex sc_mutex;

    public:
      StatusCondition_impl(
        gapi_statusCondition _gapi_handle
      );

      ~StatusCondition_impl();

      virtual ::DDS::StatusMask get_enabled_statuses (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_enabled_statuses (
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Entity_ptr get_entity (
      ) THROW_ORB_EXCEPTIONS;
  };

  typedef StatusCondition_impl *StatusCondition_impl_ptr;
}

#endif /* STATUSCONDITION */
