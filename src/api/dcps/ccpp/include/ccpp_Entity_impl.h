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
#ifndef CCPP_ENTITY_H
#define CCPP_ENTITY_H

#include "ccpp.h"
#include "ccpp_Utils.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API Entity_impl
    :
      public virtual ::DDS::Entity,
      public LOCAL_REFCOUNTED_OBJECT
  {
    protected:
      os_mutex e_mutex;
      gapi_entity _gapi_self;

      Entity_impl(gapi_entity handle);
     ~Entity_impl();

    public:

      gapi_entity get_gapi_self();

      virtual ::DDS::ReturnCode_t enable (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::StatusCondition_ptr get_statuscondition (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::StatusMask get_status_changes (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::InstanceHandle_t get_instance_handle (
      ) THROW_ORB_EXCEPTIONS;
  };
}

#endif /* ENTITY */
