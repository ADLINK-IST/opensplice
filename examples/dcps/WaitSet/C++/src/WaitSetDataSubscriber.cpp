/*************************************************************************
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

/************************************************************************
 * LOGICAL_NAME:    Subscriber.cpp
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'Subscriber' executable.
 *
 ***/
#include <string>
#include <sstream>
#include <iostream>
#include "DDSEntityManager.h"
#include "ccpp_WaitSetData.h"
#include "os.h"

#include <time.h>

using namespace DDS;
using namespace WaitSetData;

int main(int argc, char *argv[])
{
  os_time delay_2ms = { 0, 2000000 };
  Duration_t wait_timeout = {20,0};
  MsgSeq msgList;
  SampleInfoSeq infoSeq;

  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  char partition_name[] = "WaitSet example";
  mgr->createParticipant(partition_name);

  //create type
  MsgTypeSupport_var st = new MsgTypeSupport();
  mgr->registerType(st.in());

  //create Topic
  char topic_name[] = "WaitSetData_Msg";
  mgr->createTopic(topic_name);

  //create Subscriber
  mgr->createSubscriber();

  // create DataReader
  mgr->createReader();

  DataReader_ptr dreader = mgr->getReader();
  MsgDataReader_var MsgReader = MsgDataReader::_narrow(dreader);
  checkHandle(MsgReader, "MsgDataReader::_narrow");

  /* 1- Create a ReadCondition that will contain new Msg only */
  ReadCondition_var newMsg = MsgReader->create_readcondition(NOT_READ_SAMPLE_STATE,
    NEW_VIEW_STATE, ANY_INSTANCE_STATE);
  checkHandle(newMsg.in(), "DDS::DataReader::create_readcondition");

  /* 2- Create QueryCondition */
  // create a query string
  StringSeq queryStr;
  queryStr.length(1);
  queryStr[0] = "Hello again";
  //Create QueryCondition
  cout << "=== [WaitSetDataSubscriber] Query : message = " << '"' << "Hello again" << '"' << endl;
  QueryCondition_var queryCond = MsgReader->create_querycondition(ANY_SAMPLE_STATE,
    ANY_VIEW_STATE, ANY_INSTANCE_STATE, "message=%0", queryStr);
  checkHandle(queryCond, "create_querycondition");

  /* 3- Obtain a StatusCondition associated to a Writer
     and that triggers only when the Writer changes Liveliness */
  StatusCondition_var leftMsgWriter = MsgReader->get_statuscondition();
  checkHandle(leftMsgWriter.in(), "DDS::DataReader::get_statuscondition");
  ReturnCode_t status = leftMsgWriter->set_enabled_statuses(LIVELINESS_CHANGED_STATUS);
  checkStatus(status, "DDS::StatusCondition::set_enabled_statuses");

  /* 4- Create a GuardCondition which will be used to close the subscriber */
  DDS::GuardCondition_var escape;
  escape = new GuardCondition();

  /* Create a waitset and add the 4 Conditions created above :
     ReadCondition, QueryCondition, StatusCondition, GuardCondition */
  WaitSet_var newMsgWS = new WaitSet();
  status = newMsgWS->attach_condition(newMsg.in());		// ReadCondition
  checkStatus(status, "DDS::WaitSetData::attach_condition (newMsg)");
  status = newMsgWS->attach_condition(queryCond.in());		// QueryCondition
  checkStatus(status, "DDS::WaitSetData::attach_condition (queryCond)");
  status = newMsgWS->attach_condition(leftMsgWriter.in());	// StatusCondition
  checkStatus(status, "DDS::WaitSetData::attach_condition (leftMsgWriter)");
  status = newMsgWS->attach_condition(escape.in());		// GuardCondition
  checkStatus(status, "DDS::WaitSetData::attach_condition (escape)");

  /* Initialize and pre-allocate the GuardList used to obtain the triggered Conditions. */
  ConditionSeq guardList;
  guardList.length(4);

  cout << "=== [WaitSetDataSubscriber] Ready ..." << endl;

  // var used to manage the status condition
  CORBA::Long prevCount = 0;
  LivelinessChangedStatus livChangStatus;

  bool closed = false;
  bool escaped = false;
  bool writerLeft = false;
  int count = 0;
  time_t tzero, now;
  double dif;
  while (!closed && count < 20)
  {
    /* Wait until at least one of the Conditions in the waitset triggers. */
    time(&tzero);
    status = newMsgWS->wait(guardList, wait_timeout);
    if (status == DDS::RETCODE_OK) {
        /* Walk over all guards to display information */
        for (CORBA::ULong i = 0; i < guardList.length(); i++)
        {
          if (guardList[i].in() == newMsg.in())
          {
            /* The newMsg ReadCondition contains data */
            status = MsgReader->take_w_condition(msgList, infoSeq, LENGTH_UNLIMITED,
              newMsg.in());
            checkStatus(status, "WaitSetData::MsgDataReader::take_w_condition");

            for (CORBA::ULong j = 0; j < msgList.length(); j++)
            {
				if(infoSeq[j].valid_data)
				{
                   cout << endl << "    --- New message received ---" << endl;
                   cout << "    userID  : " << msgList[j].userID << endl;
                   cout << "    Message : \"" << msgList[j].message << "\"" << endl;
				}
            }
            if (msgList.length() > 0)
            {
               status = MsgReader->return_loan(msgList, infoSeq);
               checkStatus(status, "WaitSetData::MsgDataReader::return_loan");
            }
          }
          else if (guardList[i].in() == queryCond.in())
          {
            /* The queryCond QueryCondition contains data  */
            status = MsgReader->take_w_condition(msgList, infoSeq, LENGTH_UNLIMITED,
          queryCond.in());
            checkStatus(status, "WaitSetData::MsgDataReader::take_w_condition");

            for (CORBA::ULong j = 0; j < msgList.length(); j++)
            {
				if(infoSeq[j].valid_data)
				{
                   cout << endl << "    --- message received (with QueryCOndition on message field) ---" << endl;
                   cout << "    userID  : " << msgList[j].userID << endl;
                   cout << "    Message : \"" << msgList[j].message << "\"" << endl;
				}
            }
            if (msgList.length() > 0)
            {
               status = MsgReader->return_loan(msgList, infoSeq);
               checkStatus(status, "WaitSetData::MsgDataReader::return_loan");
            }
          } else if ( guardList[i].in() == leftMsgWriter.in() ) {
            /* Some liveliness has changed (either a DataWriter joined or a DataWriter left) */
            status = MsgReader->get_liveliness_changed_status(livChangStatus);
            checkStatus(status, "DDS::DataReader::get_liveliness_changed_status");
            if (livChangStatus.alive_count < prevCount) {
            /* a DataWriter lost its liveliness */
              cout << endl << "!!! a MsgWriter lost its liveliness" << endl;
              writerLeft = true;
              // SubscriberUsingWaitset terminated.
              cout << endl << "=== Triggerin escape condition " << endl;
              status = escape->set_trigger_value(TRUE);
              checkStatus(status, "DDS::GuardCondition::set_trigger_value");
               }
          else {
            /* a DataWriter joined*/
              cout << endl << "!!! a MsgWriter joined" << endl;
            }
            prevCount = livChangStatus.alive_count;
          }
          else if (guardList[i].in() == escape.in())
          {
            // SubscriberUsingWaitset terminated.
            cout << endl << "!!! escape condition triggered - count = " << count << endl;
            escaped = true;
            status = escape->set_trigger_value(FALSE);
            checkStatus(status, "DDS::GuardCondition::set_trigger_value");
         }
          else
          {
            assert(0); // error
          };
        } /* for */
    }
    else if (status != DDS::RETCODE_TIMEOUT) {
        // DDS_RETCODE_TIMEOUT is considered as an error
        // only after it has occurred count times
        checkStatus(status, "DDS::WaitSetData::wait");
    } else {
        cout << endl << "!!! [INFO] WaitSet timedout - count = " << count << endl;
        time(&now);
	dif = difftime(now, tzero);
        printf("=== Epalsed time %.2lf sec\n", dif);
    }
    ++count;
    closed = escaped && writerLeft;
  } /* while (!closed) */
  if (count >= 20)  cout << endl << "*** Error : Timed out - count = " << count << " ***" << endl;

  /* Remove all Conditions from the WaitSetData. */
  status = newMsgWS->detach_condition(escape.in());
  checkStatus(status, "DDS::WaitSetData::detach_condition (escape)");
  status = newMsgWS->detach_condition(newMsg.in());
  checkStatus(status, "DDS::WaitSetData::detach_condition (newMsg)");
  status = newMsgWS->detach_condition(leftMsgWriter.in());
  checkStatus(status, "DDS::WaitSetData::detach_condition (leftMsgWriter)");
  status = newMsgWS->detach_condition(queryCond.in());
  checkStatus(status, "DDS::WaitSetData::detach_condition (queryCond)");

  cout << "=== [Subscriber] Closed" << endl;

  MsgReader->delete_readcondition(newMsg.in());
  MsgReader->delete_readcondition(queryCond.in());
  mgr->deleteReader(MsgReader.in ());
  mgr->deleteSubscriber();
  mgr->deleteTopic();
  mgr->deleteParticipant();

  delete mgr;

  return 0;
}
