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

#include <string>
#include <iostream>
#include "example_main.h"
#include "DDSEntityManager.h"

/**
 * @file
 */

using namespace DDS;

/** A list of the participants within a node.
 */
class ParticipantInfo
{
public:
  ParticipantInfo() :
    participant_id_(0),
    next_participant_(0) {}
  ~ParticipantInfo()
  {
    if (this->next_participant_)
      delete this->next_participant_;
  }
  /** Record a participant as alive or not */
  void alive(DDS::Long participant_id, bool alive)
  {
    if (this->participant_id_ == 0 || this->participant_id_ == participant_id)
    {
      this->alive_ = alive;
      this->participant_id_ = participant_id;
    }
    else
    {
      if (!this->next_participant_)
        this->next_participant_ = new ParticipantInfo();
      this->next_participant_->alive(participant_id, alive);
    }
  }
  DDS::Long participant_id_;
  bool alive_;
  ParticipantInfo* next_participant_;
};

/** A list of running nodes. A 'node' is a 'spliced' instance.
 * In shared memory mode, this will be a splice daemon process managing the domain.
 * In single process mode each application will be a 'node'. */
class NodeInfo
{
public:
  NodeInfo() :
    nodeId(0),
    hostName(),
    participants_(new ParticipantInfo()),
    next_node_(0) {}

  ~NodeInfo()
  {
    if (this->next_node_)
      delete this->next_node_;
    if (this->participants_)
      delete this->participants_;
  }

  /** Gets the NodeInfo record for a particular 'node', updated
   * with the supplied DDS::ParticipantBuiltinTopicDataDataReader
   * Updates the hostname (if possible) and the node key.
   * @param participant_data The built-in topic data for the read participant.
   * @return A NodeInfo record.
   * */
  NodeInfo* get_node(const DDS::ParticipantBuiltinTopicData& participant_data)
  {
    if (this->nodeId == 0 || this->nodeId == participant_data.key[0])
    {
      /** Each spliced participant writes the DNS hostname into its UserDataQoSPolicy octet
      * sequence, however the CM Soap service writes a string like:
      * <TunerService><Ip>10.1.0.18:8000</Ip></TunerService>
      * so we ignore that */
      if(participant_data.user_data.value.length() > 0
        && *participant_data.user_data.value.get_buffer() != '<' )
      {
          this->hostName = std::string(reinterpret_cast<const char*> (participant_data.user_data.value.get_buffer()),
                                      participant_data.user_data.value.length());
      }
      this->nodeId = participant_data.key[0];
      return this;
    }
    else
    {
      if (!this->next_node_)
        this->next_node_ = new NodeInfo();
      return this->next_node_->get_node(participant_data);
    }
  }

  /** Counts the nodes
   * @return count of running 'nodes' */
  DDS::ULong node_count()
  {
    if (this->nodeId == 0)
      return 0;
    else if (this->next_node_)
      return (this->participant_count() > 0 ? 1 : 0) + this->next_node_->node_count();
    else
      return (this->participant_count() > 0 ? 1 : 0);
  }

   /** The count of participants connected to the node
    * @return Number of particpants that are alive */
  DDS::Long participant_count()
  {
      DDS::Long count = 0;
      ParticipantInfo* next_part = this->participants_;
      while (next_part)
      {
        if (next_part->alive_)
          ++count;
        next_part = next_part->next_participant_;
      }
      return count;
  }

  /** The unique identifier of the DDS spliced kernel */
  DDS::Long nodeId;

  /** The DNS hostname of where the node is running. */
  std::string hostName;

  /** A list of participants */
  ParticipantInfo* participants_;

  /** The next NodeInfo in the list */
  NodeInfo* next_node_;
};

int OSPL_MAIN (int argc, char *argv[])
{
  bool automatic = true;
  int result = 0;
  if (argc > 1)
  {
    automatic = (strcmp(argv[1], "true") == 0);
  }

  NodeInfo nodeInfoList;
  NodeInfo* nodeInfo;

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

  cout << "=== [BuiltInTopicsDataSubscriber] : Waiting for historical data ... " << std::endl;

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
        nodeInfo = nodeInfoList.get_node(data[i]);
        /* Check sample info to see whether the instance is ALIVE. */
        if (info[i].instance_state == DDS::ALIVE_INSTANCE_STATE)
        {
          /* Record this participant as alive. */
          nodeInfo->participants_->alive(data[i].key[1], true);

          /* If this is the first participant, report the node is up. */
          if (nodeInfo->participant_count() == 1)
          {
                cout << "=== [BuiltInTopicsDataSubscriber] Node '" << nodeInfo->nodeId << "' started (Total nodes running: " <<
                nodeInfoList.node_count() << ")" << std::endl;
          }

          if( nodeInfo->hostName.length() > 0
              && data[i].user_data.value.length() > 0
              && *data[i].user_data.value.get_buffer() != '<')
          {
                cout << "=== [BuiltInTopicsDataSubscriber] Hostname for node '" << nodeInfo->nodeId << "' is '" << nodeInfo->hostName <<
                "'." << std::endl;
          }
        }
        else
        {
          /* Remove this particpant from the running list */
          nodeInfo->participants_->alive(data[i].key[1], false);

          /* If no more participants exist, report the node is down. */
          if (nodeInfo->participant_count() == 0)
          {
              cout << "=== [BuiltInTopicsDataSubscriber] Node " << nodeInfo->nodeId << " (" << nodeInfo->hostName <<
                ") stopped (Total nodes running: " << nodeInfoList.node_count() << ")" <<
                std::endl;

          }
        }
      }
    }

    nodeInfo = &nodeInfoList;
    while (nodeInfo && nodeInfo->nodeId && nodeInfo->participant_count() > 0)
    {
        cout << "=== [BuiltInTopicsDataSubscriber] Node '" << nodeInfo->nodeId << "' has '" << nodeInfo->participant_count() <<"' participants." << std::endl;
        nodeInfo = nodeInfo->next_node_;
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
      /* There must be at least one running 'node' (i.e. us) or we have failed */
      result = nodeInfoList.node_count() > 0 ? 0 : 1;
    }
  }

  /* Delete the read condition */
  status = participantReader->delete_readcondition(readCond);
  checkStatus(status, "DDS::DataReader::delete_readcondition");

  /* Delete entities. */
  mgr.deleteContainedEntities();

  return result;
}
