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
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'Subscriber' executable.
 *
 ***/
using System;
using System.Text;
using System.Collections.Generic;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using DDSAPIHelper;


namespace BuiltInTopicsDataSubscriber
{
    class BuiltInTopicsDataSubscriber
    {
        static void Main(string[] args)
        {
            bool automatic = true;
            if (args.Length > 1)
            {
                automatic = args[0].Equals("true");
            }

            string hostName;
            DDSEntityManager mgr = new DDSEntityManager("BuiltInTopics");

            // create domain participant
            mgr.createParticipant("BuiltInTopics");

            // Resolve the built-in Subscriber
            ISubscriber builtInSubscriber = mgr.getParticipant().BuiltInSubscriber;

            // Lookup the DataReader for the DCPSParticipant built-in Topic
            IDataReader reader = builtInSubscriber.LookupDataReader("DCPSParticipant");

            // Cast the DataReader to a ParticipantBuiltInTopicDataReader
            ParticipantBuiltinTopicDataDataReader participantReader = reader as ParticipantBuiltinTopicDataDataReader;
            ErrorHandler.checkHandle(participantReader, "ParticipantBuiltInTopicDataReader narrow");

            // Allocate a new typed seq for data samples
            ParticipantBuiltinTopicData[] data = null;

            // Allocate a new seq for sample infos
            SampleInfo[] info = null;

            Console.WriteLine("=== [BuiltInTopicsDataSubscriber] : Waiting for historical data ... ");
            participantReader.WaitForHistoricalData(Duration.Infinite);
            Console.WriteLine("=== [BuiltInTopicsDataSubscriber] : done");

            // Create a new ReadCondition for the reader that matches all samples
            IReadCondition readCond = participantReader.CreateReadCondition(SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
            ErrorHandler.checkHandle(readCond, "DataReader.CreateReadCondition");

            // Create a waitset and add the ReadCondition created above
            WaitSet aWaitSet = new WaitSet();
            ReturnCode status = aWaitSet.AttachCondition(readCond);
            ErrorHandler.checkStatus(status, "WaitSet.AttachCondition");

            // Initialize and pre-allocate the seq used to obtain the triggered Conditions.
            ICondition[] condSeq = new ICondition[0];

            // Allocate a map store node information later on
            // The key of map is the id of the node and the value is the
            // number of active participants on that node. A Dictionary is used
            // as an UnorderedMap.
            Dictionary<int, int> nodes = new Dictionary<int, int>();

            // Allocate a map to store node information later on.
            // The key of the map is the id of the node and the value is the
            // name of the node. A Dictionary is used as an UnorderedMap.
            Dictionary<int, string> nodeNames = new Dictionary<int, string>();

            Console.WriteLine("=== [BuiltInTopicsDataSubscriber] Ready ...");

            // Block the current thread until the attached condition becomes true
            // or the user interrupts.
            status = aWaitSet.Wait(ref condSeq, Duration.Infinite);
            ErrorHandler.checkStatus(status, "WaitSet.Wait");

            bool done = false;
            // Continue processing until interrupted.
            while (!done)
            {
                // Take all the available data from the reader
                status = participantReader.Take(ref data, ref info, Length.Unlimited,
                    SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                ErrorHandler.checkStatus(status, "ParticipantBuiltinTopicDataDataReader.Take");

                // Verify the data has been taken
                if (status == ReturnCode.Ok)
                {
                    // Iterate the list of taken samples.
                    for (int i = 0; i < data.Length ; i++)
                    {
                        // Resolve the node identification
                        int nodeId = data[i].Key[0];

                        // Initialise the number of participants for a node
                        int participantCount = 0;

                        // Check if we saw a participant for the node before.
                        if (nodes.ContainsKey(nodeId))
                        {
                            participantCount = nodes[nodeId];
                        }

                        // Check sample info to see whether the instance is ALIVE.
                        if (info[i].InstanceState == InstanceStateKind.Alive)
                        {
                            // The splicedaemon publishes the host-name in the
                            // user_data field
                            if (data[i].UserData.Value.Length != 0)
                            {
                                hostName = Encoding.UTF8.GetString(data[i].UserData.Value);
                                nodeNames[nodeId] = hostName;
                            }
                            else
                            {
                                hostName = null;
                            }

                            // Increase the number of participants.
                            participantCount++;

                            // Update the number of participants for the node.
                            nodes[nodeId] = participantCount;

                            // If it's the first participant, report the node is up.
                            if (participantCount == 1)
                            {
                                Console.WriteLine("=== [BuiltInTopicsDataSubscriber] Node ' {0} ' started (Total nodes running: {1} ) participantCount = {2}",
                                    nodeId, nodes.Count, participantCount);
                            }
                            if (hostName != null)
                            {
                                Console.WriteLine("=== [BuiltInTopicsDataSubscriber] Hostname for node ' {0} ' is hostname: {1}", nodeId, hostName);
                            }
                        }
                        else
                        {
                            // Decrease the number of participants.
                            participantCount--;

                            // If no more participants exist, report the node is down.
                            if (participantCount == 0)
                            {
                                hostName = nodeNames[nodeId];
                                nodeNames.Remove(nodeId);
                                nodes.Remove(nodeId);

                                if (hostName != null)
                                {
                                    Console.WriteLine("=== [BuiltInTopicsDataSubscriber] Node {0} ({1}) stopped (Total nodes running: ({2})",
                                        nodeId, hostName, nodes.Count);
                                }
                                else
                                {
                                    Console.WriteLine("=== [BuiltInTopicsDataSubscriber] Node {0} stopped (Total nodes running : {1})",
                                        nodeId, nodes.Count);
                                }
                            }
                            else if (participantCount > 0)
                            {
                                nodes[nodeId] = participantCount;
                            }
                        }
                    }
                }

                status = participantReader.ReturnLoan(ref data, ref info);
                ErrorHandler.checkStatus(status, "ParticipantReader.ReturnLoan");

                if (!automatic)
                {
                    // Block the current thread until the attached condition becomes
                    // true or the user interrupts.
                    Console.WriteLine("=== [BuiltInTopicsDataSubscriber] Waiting ... ");
                    status = aWaitSet.Wait(ref condSeq, Duration.Infinite);
                    done = (status != ReturnCode.Ok);
                }
                else
                {
                    done = true;
                }
            }

            // Delete read condition
            status = participantReader.DeleteReadCondition(readCond);
            ErrorHandler.checkStatus(status, "DataReader.DeleteReadCondition");

            // Recursively delete all entities in the DomainParticipant.
            mgr.getParticipant().DeleteContainedEntities();

            // Delete DomainParticipant
            mgr.getDomainParticipantFactory().DeleteParticipant(mgr.getParticipant());
        }
    }
}
