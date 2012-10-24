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
#ifndef CCPP_CONTENTFILTEREDTOPIC_H
#define CCPP_CONTENTFILTEREDTOPIC_H


#include "ccpp.h"
#include "ccpp_DomainParticipant_impl.h"
#include "ccpp_TopicDescription_impl.h"
#include "os_mutex.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API ContentFilteredTopic_impl
    : public virtual ::DDS::ContentFilteredTopic,
      public ::DDS::TopicDescription_impl
  {
    friend class ::DDS::DomainParticipant_impl;

    private:
      os_mutex cft_mutex;

      ContentFilteredTopic_impl(
	    gapi_contentFilteredTopic handle);
     ~ContentFilteredTopic_impl();

    public:

      virtual char * get_filter_expression (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_expression_parameters (
        ::DDS::StringSeq & expression_parameters
      ) THROW_ORB_EXCEPTIONS;\

      virtual ::DDS::ReturnCode_t set_expression_parameters (
        const ::DDS::StringSeq & expression_parameters
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Topic_ptr get_related_topic (
      ) THROW_ORB_EXCEPTIONS;
  };
  typedef ContentFilteredTopic_impl* ContentFilteredTopic_impl_ptr;
}

#endif /* CCPP_CONTENTFILTEREDTOPIC */
