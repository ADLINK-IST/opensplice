/************************************************************************
 *  
 * Copyright (c) 2010
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    ListenerDataListener.h
 * FUNCTION:        .
 * MODULE:          .
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the headers for all operations required
 * 
 ***/
#ifndef __LISTENER_H__
  #define __LISTENER_H__

  #include <string>
  #include <sstream>
  #include <iostream>
  #include "ccpp_dds_dcps.h" 
  #include "CheckStatus.h"
  #include "ccpp_ListenerData.h"
  using namespace ListenerData;

  // ------------------------------ Listeners ------------------------------
  class ListenerDataListener: public virtual DDS::DataReaderListener
  {

    public:

      bool m_closed;
      MsgDataReader_var m_MsgReader;
      DDS::GuardCondition_var m_guardCond;

	  ListenerDataListener () {
		  m_guardCond = new DDS::GuardCondition();
		  m_closed = false;
	  }

      /* Callback method implementation. */
      virtual void on_data_available(DDS::DataReader_ptr reader)
        THROW_ORB_EXCEPTIONS;

      virtual void on_requested_deadline_missed(DDS::DataReader_ptr reader,
        const DDS::RequestedDeadlineMissedStatus &status)THROW_ORB_EXCEPTIONS;

      virtual void on_requested_incompatible_qos(DDS::DataReader_ptr reader,
        const DDS::RequestedIncompatibleQosStatus &status)THROW_ORB_EXCEPTIONS;

      virtual void on_sample_rejected(DDS::DataReader_ptr reader, const DDS
        ::SampleRejectedStatus &status)THROW_ORB_EXCEPTIONS;

      virtual void on_liveliness_changed(DDS::DataReader_ptr reader, const DDS
        ::LivelinessChangedStatus &status)THROW_ORB_EXCEPTIONS;

      virtual void on_subscription_matched(DDS::DataReader_ptr reader, const
        DDS::SubscriptionMatchedStatus &status)THROW_ORB_EXCEPTIONS;

      virtual void on_sample_lost(DDS::DataReader_ptr reader, const DDS
        ::SampleLostStatus &status)THROW_ORB_EXCEPTIONS;
  };
#endif
