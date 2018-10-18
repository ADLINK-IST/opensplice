/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

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
