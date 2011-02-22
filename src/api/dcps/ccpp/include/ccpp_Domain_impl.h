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
#ifndef CCPP_DOMAIN_H
#define CCPP_DOMAIN_H

#include "ccpp.h"
#include "ccpp_DomainParticipantFactory.h"

#include "gapi.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API Domain_impl
    : public virtual ::DDS::Domain,
      public LOCAL_REFCOUNTED_OBJECT
  {
      friend class ::DDS::DomainParticipantFactory;
    private:
        gapi_domain _gapi_self;
        ~Domain_impl();
        Domain_impl(gapi_domain handle);

    public:
      virtual ::DDS::ReturnCode_t create_persistent_snapshot (
        const char *partition_expression,
        const char *topic_expression,
        const char *URI
      ) THROW_ORB_EXCEPTIONS;
  };
  typedef Domain_impl * Domain_impl_ptr;
}

#endif /* DOMAIN */
