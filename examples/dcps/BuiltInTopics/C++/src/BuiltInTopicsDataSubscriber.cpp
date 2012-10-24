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

#include <map>

using namespace DDS;

int main(int argc, char *argv[])
{
  bool automatic = true;
  if (argc > 1)
  {
    automatic = (strcmp(argv[1], "true") == 0);
  }

  std::string hostName;

  DDSEntityManager *mgr = new DDSEntityManager();

  // create domain participant
  mgr->createParticipant();

  /* Resolve the built-in Subscriber. */
  Subscriber_var builtinSubscriber = mgr->participant->get_builtin_subscriber();

  /* Lookup the DataReader for the DCPSParticipant built-in Topic. */
  DataReader_var reader = builtinSubscriber->lookup_datareader("DCPSParticipant");

  /* Safely cast the DataReader to a ParticipantBuiltinTopicDataReader. */
  ParticipantBuiltinTopicDataDataReader_var participantReader =
    ParticipantBuiltinTopicDataDataReader::_narrow(reader);
  checkHandle(participantReader, "ParticipantBuiltinTopicDataDataReader::_narrow");

  /* Allocate a new typed seq for data samples. */
  ParticipantBuiltinTopicDataSeq data;

  /* Allocate a new seq for sample infos. */
  SampleInfoSeq info;

  cout << "=== [BuiltInTopicsDataSubscriber] : Waiting for historical data ... " <<
    endl;

  /* Make sure all historical data is delivered in the DataReader. */
  //participantReader->wait_for_historical_data(DDS::DURATION_INFINITE);

  cout << "=== [BuiltInTopicsDataSubscriber] : done" << std::endl;

  /* Create a new ReadCondition for the reader that matches all samples.*/
  ReadCondition_var readCond = participantReader->create_readcondition
    (ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
  checkHandle(readCond.in(), "DDS::DataReader::create_readcondition");

  /* Create a waitset and add the  ReadCondition created above */
  WaitSet_var aWaitSet = new WaitSet();
  ReturnCode_t status = aWaitSet->attach_condition(readCond.in());
  checkStatus(status, "DDS::WaitSet::attach_condition");

  /* Initialize and pre-allocate the seq used to obtain the triggered Conditions. */
  ConditionSeq condSeq;
  condSeq.length(1);

  /* Allocate a map to store node information later on. */
  /* The key of the map is the id of the node and the value is the */
  /* number of active participants on that node. */
  std::map < int, int > nodes;

  /* Allocate a map to store node information later on. */
  /* The key of the map is the id of the node and the value is the */
  /* name of the node. */
  std::map < int, std::string > nodeNames;

  cout << "=== [BuiltInTopicsDataSubscriber] Ready ..." << endl;

  /* Block the current thread until the attached condition becomes true
   * or the user interrupts.
   */
  status = aWaitSet->wait(condSeq, DDS::DURATION_INFINITE);
  checkStatus(status, "DDS::WaitSet::wait");

  bool done = false;
   /* Continue processing until interrupted. */
  while (!done)
  {
    /* Take all available data from the reader. */
    status = participantReader->take(data, info, LENGTH_UNLIMITED,
      ANY_SAMPLE_STATE, ANY_VIEW_STATE, ANY_INSTANCE_STATE);
    checkStatus(status, "DDS::ParticipantBuiltinTopicDataDataReader::take");
   /* Verify that data has been taken. */
    if (status == DDS::RETCODE_OK)
    {
      /* Iterate the list of taken samples. */
      for (int i = 0; i < data.length(); i++)
      {
        /* Resolve the node identification. */
        int nodeId = data[i].key[0];

        /* Initialise the number of participants for a node. */
        int participantCount = 0;

        /* Check if we saw a participant for the node before. */
	if (nodes.find(nodeId) != nodes.end())
        {
          /* Resolve the actual number of participants on the node. */
          participantCount = nodes[nodeId];
          //cout << "--- HERE (1) "<< std::endl;
        }

        /* Check sample info to see whether the instance is ALIVE. */
        if (info[i].instance_state == DDS::ALIVE_INSTANCE_STATE)
        {
          /* The splicedaemon publishes the host-name in the
           * user_data field
           */
		 char name [255];
		 int len = data[i].user_data.value.length();
         if (len != 0)
         {
		   for (int j = 0; j < len; j++){
		     name[j] = data[i].user_data.value[j];
		   }
		   name[len] = '\0';
 		   hostName = CORBA::string_dup (name) ;
           nodeNames[nodeId] = hostName;
         }
         else
         {
            hostName = "";
         }

          /* Increase the number of participants. */
          participantCount++;

          /* Update the number of participants for the node. */
          nodes[nodeId] = participantCount;

          /* If it's the first participant, report the node is up. */
          if (participantCount == 1)
          {
            cout << "=== [BuiltInTopicsDataSubscriber] Node '" << nodeId << "' started (Total nodes running: " <<
              nodes.size() << ")" << " participantCount =" << participantCount <<std::endl;
          }
          if (hostName != "")
          {
            cout << "=== [BuiltInTopicsDataSubscriber] Hostname for node '" << nodeId << " is '" << hostName <<
              "'." << std::endl;
          }
        }
        else
        {
          /* Decrease the number of participants. */
          participantCount--;

          /* If no more participants exist, report the node is down. */
          if (participantCount == 0)
          {
            hostName = nodeNames[nodeId];
            nodeNames.erase(nodeId);
            nodes.erase(nodeId);

            if (hostName != "")
            {
              cout << "=== [BuiltInTopicsDataSubscriber] Node " << nodeId << " (" << hostName <<
                ") stopped (Total nodes running: " << nodes.size() << ")" <<
                std::endl;
            }
            else
            {
              cout << "=== [BuiltInTopicsDataSubscriber] Node " << nodeId << " stopped (Total nodes running: " <<
                nodes.size() << ")" << std::endl;
            }
          }
          else if (participantCount > 0)
          {

		//cout << "--- HERE (2) "<< std::endl;
            nodes[nodeId] = participantCount;
          }
        }
      }
    }

    /* Indicate to reader that data/info is no longer accessed.*/
    participantReader->return_loan(data, info);
    if (! automatic) {
      /* Block the current thread until the attached condition becomes
       * true or the user interrupts.
       */
      cout << "=== [BuiltInTopicsDataSubscriber] Waiting ... " << std::endl;
      status = aWaitSet->wait(condSeq, DURATION_INFINITE);
      done = (status != DDS::RETCODE_OK);
      } else {
      done = true;
      }
  }



  /* Delete entities. */
  mgr->deleteContainedEntities();

  delete mgr;
  return 0;
}
