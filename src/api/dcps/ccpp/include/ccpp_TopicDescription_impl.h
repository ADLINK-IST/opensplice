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
#ifndef CCPP_TOPICDESCRIPTION_H
#define CCPP_TOPICDESCRIPTION_H

#include "ccpp.h"
#include "ccpp_DomainParticipant_impl.h"
#include "ccpp_Subscriber_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API TopicDescription_impl
   :  public virtual ::DDS::TopicDescription,
      public ::DDS::Entity_impl
    {
    friend class ::DDS::DomainParticipant_impl;
    friend class ::DDS::Subscriber_impl;

    protected:
        gapi_topicDescription __gapi_self;
        TopicDescription_impl(gapi_topicDescription handle);

    public:
      virtual char * get_type_name (
      ) THROW_ORB_EXCEPTIONS;

      virtual char * get_name (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DomainParticipant_ptr get_participant (
      ) THROW_ORB_EXCEPTIONS;
  };
  typedef TopicDescription_impl* TopicDescription_impl_ptr;
}

#endif /* TOPICDESCRIPTION */
