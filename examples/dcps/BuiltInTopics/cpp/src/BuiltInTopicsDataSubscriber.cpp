/*************************************************************************
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
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
#include <iostream>
#include "example_main.h"
#include "DDSEntityManager.h"


using namespace DDS;

typedef struct {
    Long nodeId;
    String hostName;
    Long participantCount;
} NodeInfo;


/*
In this example we could have used std::map to store the node data, however some embedded targets don't come with the standard library. To retain compatibility
we opted not to use this implementation.
*/

NodeInfo* updateNode ( ParticipantBuiltinTopicData* newNode,
         NodeInfo* nodeInfoArray,
         unsigned long nodeInfoIndex)
{
   if(newNode->user_data.value.length() > 0)
   {
        nodeInfoArray[ nodeInfoIndex ].hostName = string_alloc( newNode->user_data.value.length() );
        memcpy( nodeInfoArray[ nodeInfoIndex ].hostName, newNode->user_data.value.get_buffer(), newNode->user_data.value.length() );
        nodeInfoArray[ nodeInfoIndex ].hostName[ newNode->user_data.value.length() ] = '\0';
   }
   nodeInfoArray[ nodeInfoIndex ].nodeId = newNode->key[ 0 ];
   nodeInfoArray[ nodeInfoIndex ].participantCount = 1;

   return nodeInfoArray;
}

NodeInfo* getNodeInfo ( ParticipantBuiltinTopicData* node,
         NodeInfo** nodeInfoArray,
         unsigned long* nodeInfoArraySize )
{
   NodeInfo* nodeInfo = NULL;
   unsigned long nodeIndex;

   for( nodeIndex = 0 ; nodeIndex < *nodeInfoArraySize ; ++nodeIndex )
   {
      if( (*nodeInfoArray)[ nodeIndex ].nodeId == node->key[ 0 ] )
      {
          updateNode(node, *nodeInfoArray, nodeIndex);
         nodeInfo = &(*nodeInfoArray)[ nodeIndex ];
          break;
      }
   }
   if( nodeIndex == *nodeInfoArraySize )
   {
       NodeInfo* newNodeInfoArray;
       int newByteSize = ++(*nodeInfoArraySize) * sizeof(NodeInfo);
       newNodeInfoArray = (NodeInfo *)realloc(*nodeInfoArray, newByteSize);
       *nodeInfoArray = newNodeInfoArray;
       (*nodeInfoArray)[ nodeIndex ].hostName = NULL;
       updateNode(node, *nodeInfoArray, nodeIndex);
   }
   nodeInfo = &(*nodeInfoArray[ nodeIndex ]);


   return nodeInfo;
}

void freeNodeInfoArray ( NodeInfo** nodeInfoArray,
         unsigned long* nodeInfoArraySize )
{
   unsigned long nodeIndex;

   for( nodeIndex = 0 ; nodeIndex < *nodeInfoArraySize ; ++nodeIndex )
   {
      if( (*nodeInfoArray)[ nodeIndex ].hostName != NULL )
      {
         free(*nodeInfoArray);
      }
   }

   free(*nodeInfoArray);
   *nodeInfoArray = NULL;
   *nodeInfoArraySize = 0;
}

int OSPL_MAIN (int argc, char *argv[])
{
  bool automatic = true;
  if (argc > 1)
  {
    automatic = (strcmp(argv[1], "true") == 0);
  }

  std::string hostName;

  /* Resizable array to store the nodes' description with the count of participants for each. */

  NodeInfo* nodeInfoArray = NULL;
  NodeInfo* nodeInfo;
  unsigned long nodeInfoArraySize = 0;

  DDSEntityManager mgr;

  // create domain participant
  mgr.createParticipant();

  /* Resolve the built-in Subscriber. */
  Subscriber_var builtinSubscriber = mgr.participant->get_builtin_subscriber();
  /* Lookup the DataReader for the DCPSParticipant built-in Topic. */
  DataReader_var reader = builtinSubscriber->lookup_datareader("DCPSParticipant");

  /* Safely cast the DataReader to a ParticipantBuiltinTopicDataReader. */
  ParticipantBuiltinTopicDataDataReader_var participantReader =
  ParticipantBuiltinTopicDataDataReader::_narrow(reader.in());
  checkHandle(participantReader.in(), "ParticipantBuiltinTopicDataDataReader::_narrow");
  /* Allocate a new typed seq for data samples. */
  ParticipantBuiltinTopicDataSeq data;

  /* Allocate a new seq for sample infos. */
  SampleInfoSeq info;

  cout << "=== [BuiltInTopicsDataSubscriber] : Waiting for historical data ... " <<
    endl;

  /* Make sure all historical data is delivered in the DataReader. */
  participantReader->wait_for_historical_data(DDS::DURATION_INFINITE);

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

  /* Allocate a map to store node information later on. */
  /* The key of the map is the id of the node and the value is the */
  /* name of the node. */

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
      for (unsigned int i = 0; i < data.length(); i++)
      {
        /* Check sample info to see whether the instance is ALIVE. */
        if (info[i].instance_state == DDS::ALIVE_INSTANCE_STATE)
        {
          /* The splicedaemon publishes the host-name in the
           * user_data field
           */
            nodeInfo = getNodeInfo(&data[i], &nodeInfoArray, &nodeInfoArraySize );
          /* Increase the number of participants. */
            nodeInfo->participantCount++;

          /* If it's the first participant, report the node is up. */
          if (nodeInfo->participantCount == 1)
          {
                cout << "=== [BuiltInTopicsDataSubscriber] Node '" << nodeInfo->nodeId << "' started (Total nodes running: " <<
                nodeInfoArraySize << ")" << " participantCount =" << nodeInfo->participantCount <<std::endl;
          }
                if( data[i].user_data.value.length() > 0)
          {
                cout << "=== [BuiltInTopicsDataSubscriber] Hostname for node '" << nodeInfo->nodeId << " is '" << nodeInfo->hostName <<
                "'." << std::endl;
          }
        }
        else
        {
          /* Decrease the number of participants. */
          nodeInfo->participantCount--;

          /* If no more participants exist, report the node is down. */
          if (nodeInfo->participantCount == 0)
          {
              cout << "=== [BuiltInTopicsDataSubscriber] Node " << nodeInfo->nodeId << " (" << nodeInfo->hostName <<
                ") stopped (Total nodes running: " << nodeInfoArraySize << ")" <<
                std::endl;

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

  /* Delete the read condition */
  status = participantReader->delete_readcondition(readCond);
  checkStatus(status, "DDS::DataReader::delete_readcondition");

  /* Delete entities. */
  mgr.deleteContainedEntities();

  return 0;
}
