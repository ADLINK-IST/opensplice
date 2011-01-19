/************************************************************************
 *
 * Copyright (c) 2010
 * PrismTech Ltd.
 * All rights Reserved.
 *
 * LOGICAL_NAME:    ListenerDataListener.cpp
 * FUNCTION:        .
 * MODULE:          .
 * DATE             August 2010.
 ************************************************************************
 *
 * This file contains the implementation of ListenerDataListener
 *
 ***/

#include "ListenerDataListener.h"
#include "CheckStatus.h"
#include <sstream>

using namespace DDS;
using namespace ListenerData;

// --------------------------------------------------------------------------------------------------------
//                                              ListenerDataListener                                     --
// --------------------------------------------------------------------------------------------------------

void ListenerDataListener::on_data_available(DDS::DataReader_ptr reader)
  THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t status;
  MsgSeq msgList;
  SampleInfoSeq infoSeq;

  status = m_MsgReader->read(msgList, infoSeq, LENGTH_UNLIMITED,
    ANY_SAMPLE_STATE, NEW_VIEW_STATE, ANY_INSTANCE_STATE);
  checkStatus(status, "MsgDataReader::read");
  cout << "=== [ListenerDataListener::on_data_available] - msgList.length : " <<
    msgList.length() << endl;
  for (CORBA::ULong j = 0; j < msgList.length(); j++)
    {
      cout << "    --- message received ---" << endl;
      cout << "    userID  : " << msgList[j].userID << endl;
      cout << "    Message : \"" << msgList[j].message << "\"" << endl;
    }
  status = m_MsgReader->return_loan(msgList, infoSeq);
  checkStatus(status, "MsgDataReader::return_loan");
  // unblock the waitset in Subscriber main loop
  m_guardCond->set_trigger_value(TRUE);
};

void ListenerDataListener::on_requested_deadline_missed(DDS::DataReader_ptr
  reader, const DDS::RequestedDeadlineMissedStatus &status)THROW_ORB_EXCEPTIONS
{
  cout << "=== [ListenerDataListener::on_requested_deadline_missed] : triggered" << endl;
  cout << "=== [ListenerDataListener::on_requested_deadline_missed] : stopping" << endl;
  m_closed = true;
  // unblock the waitset in Subscriber main loop
  m_guardCond->set_trigger_value(TRUE);
};

void ListenerDataListener::on_requested_incompatible_qos(DDS::DataReader_ptr
  reader, const DDS::RequestedIncompatibleQosStatus &status)
  THROW_ORB_EXCEPTIONS
{
  cout << "=== [ListenerDataListener::on_requested_incompatible_qos] : triggered" << endl;
};

void ListenerDataListener::on_sample_rejected(DDS::DataReader_ptr reader, const
  DDS::SampleRejectedStatus &status)THROW_ORB_EXCEPTIONS
{
  cout << "=== [ListenerDataListener::on_sample_rejected] : triggered" << endl;
};

void ListenerDataListener::on_liveliness_changed(DDS::DataReader_ptr reader,
  const DDS::LivelinessChangedStatus &status)THROW_ORB_EXCEPTIONS
{
  cout << "=== [ListenerDataListener::on_liveliness_changed] : triggered" << endl;
};

void ListenerDataListener::on_subscription_matched(DDS::DataReader_ptr reader,
  const DDS::SubscriptionMatchedStatus &status)THROW_ORB_EXCEPTIONS
{
  cout << "=== [ListenerDataListener::on_subscription_matched] : triggered" << endl;
};

void ListenerDataListener::on_sample_lost(DDS::DataReader_ptr reader, const DDS
  ::SampleLostStatus &status)THROW_ORB_EXCEPTIONS
{
  cout << "=== [ListenerDataListener::on_sample_lost] : triggered" << endl;
};
