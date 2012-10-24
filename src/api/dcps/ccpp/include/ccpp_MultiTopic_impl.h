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
#ifndef CCPP_MULTITOPIC_H
#define CCPP_MULTITOPIC_H

#include "ccpp.h"
#include "ccpp_TopicDescription_impl.h"
#include "ccpp_DomainParticipant_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API MultiTopic_impl
    : public virtual ::DDS::MultiTopic,
      public ::DDS::TopicDescription_impl
  {
    friend class ::DDS::DomainParticipant_impl;

    private:
      gapi_multiTopic _gapi_self;
      os_mutex mt_mutex;
      MultiTopic_impl(gapi_multiTopic handle);
     ~MultiTopic_impl();

    public:
    virtual char * get_subscription_expression (
    ) THROW_ORB_EXCEPTIONS;

    virtual ::DDS::ReturnCode_t get_expression_parameters (
      ::DDS::StringSeq & expression_parameters
    ) THROW_ORB_EXCEPTIONS;

    virtual ::DDS::ReturnCode_t set_expression_parameters (
      const ::DDS::StringSeq & expression_parameters
    ) THROW_ORB_EXCEPTIONS;
  };
  typedef MultiTopic_impl* MultiTopic_impl_ptr;
}

#endif /* MULTITOPIC */
