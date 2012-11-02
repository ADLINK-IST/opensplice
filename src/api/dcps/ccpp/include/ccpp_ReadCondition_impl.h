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
#ifndef CCPP_READCONDITION_H
#define CCPP_READCONDITION_H

#include "ccpp.h"
#include "ccpp_DataReader_impl.h"
#include "ccpp_DataReaderView_impl.h"
#include "ccpp_Condition_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API ReadCondition_impl
    : public virtual ::DDS::ReadCondition,
      public ::DDS::Condition_impl
  {
    friend class ::DDS::DataReader_impl;
    friend class ::DDS::DataReaderView_impl;

    protected:
        os_mutex rc_mutex;
        ReadCondition_impl(gapi_readCondition handle);
       ~ReadCondition_impl();

    public:

      virtual ::DDS::SampleStateMask get_sample_state_mask (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ViewStateMask get_view_state_mask (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::InstanceStateMask get_instance_state_mask (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DataReader_ptr get_datareader (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DataReaderView_ptr get_datareaderview(
      ) THROW_ORB_EXCEPTIONS;
  };
  typedef ReadCondition_impl* ReadCondition_impl_ptr;
}

#endif /* READCONDITION */
