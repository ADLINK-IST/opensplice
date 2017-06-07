
/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "implementation.hpp"
#include "dds/dds.hpp"

#include "org/opensplice/topic/TopicTraits.hpp"
#include "org/opensplice/topic/DataRepresentation.hpp"


#include "common/example_utilities.h"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace BuiltinTopics {

/**
 * @addtogroup examplesdcpsBuiltinTopicsisocpp The ISO C++ DCPS API BuiltinTopics example
 *
 * This example demonstrates the use of builtin topics by monitoring the number of nodes
 * that participate in a DDS domain.
 * @ingroup examplesdcpsisocpp
 *
 * @see <a href="../../../examples/dcps/BuiltInTopics/README.html">Example Description</a>
 * @see @ref isocpp2_dcpsexamples "IsoC++2 Examples"
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp {

/** A list of running nodes. A 'node' is a 'spliced' instance.
 *  In shared memory mode, this will be a splice daemon process managing the domain.
 *  In single process mode each application will be a 'node'.
 */
class NodeInfo
{
public:
    NodeInfo(const dds::topic::ParticipantBuiltinTopicData& participantData)
    {
        nodeID_ = participantData.key().value()[0];
    }

    bool operator==(const NodeInfo& rhs)
    {
        return nodeID_ == rhs.nodeID_;
    }

    int32_t nodeID_;
    std::string hostName_;
    /**
    * Vector to hold the IDs of the currently alive participants in the node
    */
    std::vector<uint32_t> participants;
};

std::vector<NodeInfo> nodes;

/**
 * Finds a node a node with an ID obtained from the ParticipantBuiltinTopicData, updates the
 * hostname and returns the node. If no node is found then a new one is created and returned.
 *
 * @param participantData The ParticipantBuiltinTopicData of the node
 * @return The found/created node
 */
NodeInfo& getNodeInfo(const dds::topic::ParticipantBuiltinTopicData& participantData)
{
    /** Find and return the node if it already exists */
    for(std::vector<NodeInfo>::iterator it = nodes.begin(); it < nodes.end(); it++)
    {
        if(it->nodeID_ == participantData.key().value()[0])
        {
            /** Update the hostname */
            if(participantData.user_data().value().size() > 0
            && *participantData.user_data().value().data() != '<' )
            {
                it->hostName_ = std::string(reinterpret_cast<const char*> (participantData.user_data().value().data()),
                                        participantData.user_data().value().size());
            }
            return *it;
        }
    }

    /** Create a new node and return it if no node existed */
    NodeInfo node(participantData);
    nodes.push_back(node);
    return nodes.back();
}

/**
 * Runs the subscriber role of this example.
 * @return 0 if a sample is successfully read, 1 otherwise.
 */
int subscriber(int argc, char *argv[])
{
    int result = 0;
    bool automatic = true;
    if (argc > 1)
    {
        automatic = (strcmp(argv[1], "true") == 0);
    }

    try
    {
        /** A domainParticipant is created */
        dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());

        /** Get the builtin Subscriber */
        dds::sub::Subscriber builtinSubscriber = dds::sub::builtin_subscriber(dp);

        /** Get the ParticipantBuiltinTopicDataReader */
        std::string topicName = "DCPSParticipant";
        std::vector<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData> > prv;
        dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData> participantReader(dds::core::null);
        if(dds::sub::find<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData>,
            std::back_insert_iterator<std::vector<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData> > > >(
                builtinSubscriber, topicName,
                std::back_inserter<std::vector<dds::sub::DataReader<dds::topic::ParticipantBuiltinTopicData> > >(
                    prv)) > 0)
        {
            participantReader = prv[0];
        }
        else
        {
            throw "Could not get ParticipantBuiltinTopicDataReader";
        }

        std::cout << "=== [BuiltInTopicsDataSubscriber] Waiting for historical data ... " << std::endl;

        /* Make sure all historical data is delivered in the DataReader */
        participantReader.wait_for_historical_data(dds::core::Duration::infinite());

        std::cout << "=== [BuiltInTopicsDataSubscriber] Done" << std::endl;

        /* Create a new ReadCondition for the reader that matches all samples.*/
        dds::sub::cond::ReadCondition readCond(participantReader, dds::sub::status::DataState::any());

        /* Create a WaitSet and attach the ReadCondition created above */
        dds::core::cond::WaitSet waitSet;
        waitSet += readCond;

        std::cout << "=== [BuiltInTopicsDataSubscriber] Ready ..." << std::endl;

        /*
        * Block the current thread until the attached condition becomes true
        * or the user interrupts.
        */
        waitSet.wait();

        bool done = false;
        while(!done)
        {
            /**
            * Take all data from the reader and iterate through it
            */
            dds::sub::LoanedSamples<dds::topic::ParticipantBuiltinTopicData> samples = participantReader.take();
            for (dds::sub::LoanedSamples<dds::topic::ParticipantBuiltinTopicData>::const_iterator sample = samples.begin();
                sample < samples.end(); ++sample)
            {
                if(sample->info().valid())
                {
                    NodeInfo& nodeInfo = getNodeInfo(sample->data());

                    /**
                    * Check if the participant is alive and add it to the nodeInfo
                    */
                    if(sample->info().state().instance_state() == dds::sub::status::InstanceState::alive())
                    {
                        nodeInfo.participants.push_back(sample->data().key().value()[1]);
                        /**
                        * If this is the first participant, output that the node is running
                        */
                        if(nodeInfo.participants.size() == 1)
                        {
                            std::cout << "=== [BuiltInTopicsDataSubscriber] Node '"
                                        << nodeInfo.nodeID_ << "' started (Total nodes running: "
                                        << nodes.size() << ")" << std::endl;
                        }

                        /**
                        * Output info for each node
                        */
                        if(nodeInfo.hostName_.length() > 0
                            && sample->data().user_data().value().size() > 0
                            && *(sample->data().user_data().value().data()) != '<')
                        {
                            std::cout << "=== [BuiltInTopicsDataSubscriber] Hostname for node '"
                                        << nodeInfo.nodeID_ << "' is '" << nodeInfo.hostName_
                                        << "'." << std::endl;
                        }
                    }
                    /**
                    * If the participant is not alive remove it from the nodeInfo
                    */
                    else
                    {
                        nodeInfo.participants.erase(std::remove(nodeInfo.participants.begin(),
                                                                nodeInfo.participants.end(),
                                                                sample->data().key().value()[1]),
                                                    nodeInfo.participants.end());

                        /**
                        * If no more participants exist, output that the node has stopped
                        */
                        if(nodeInfo.participants.size() == 0)
                        {
                            std::cout << "=== [BuiltInTopicsDataSubscriber] Node "
                                        << nodeInfo.nodeID_ << " (" << nodeInfo.hostName_
                                        << ") stopped (Total nodes running: "
                                        << nodes.size() << ")" << std::endl;
                        }
                    }
                }
            }

            for(std::vector<NodeInfo>::iterator it = nodes.begin(); it < nodes.end(); it++)
            {
                if(it->participants.size() > 0)
                {
                    std::cout << "=== [BuiltInTopicsDataSubscriber] Node '" << it->nodeID_
                                << "' has '" << it->participants.size()
                                << "' participants." << std::endl;
                }
            }

            if(!automatic)
            {
                /* Block the current thread until the attached condition becomes
                *  true or the user interrupts.
                */
                std::cout << "=== [BuiltInTopicsDataSubscriber] Waiting ... " << std::endl;
                try
                {
                    waitSet.wait();
                }
                catch(...)
                {
                    done = true;
                }
            }
            else
            {
                done = true;
                /**
                * If there is not at least one running node (i.e. this program) then the result is failure
                */
                result = nodes.size() > 0 ? 0 : 1;
            }
        }
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    catch (const std::string& e)
    {
        std::cerr << "ERROR: " << e << std::endl;
        result = 1;
    }
    return result;
}

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_BuiltinTopics_subscriber, examples::dcps::BuiltinTopics::isocpp::subscriber)
