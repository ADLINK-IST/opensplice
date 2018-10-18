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

import java.util.HashMap;
import java.util.Set;

import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.WaitSet;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.ReadCondition;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.Subscriber.DataState;
import org.omg.dds.topic.ParticipantBuiltinTopicData;

/**
 * This examples application monitors the number of nodes that participate in a
 * DDS domain.
 *
 */
public class BuildInTopicsDataSubscriber {
    public static void main(String[] args) {

        try {
            boolean automatic = true;
            if (args.length > 1)
            {
                automatic = (args[0].equals("true"));
            }
            int domainId = 0;
            String hostName;
            /*
             * Select DDS implementation and initialize DDS ServiceEnvironment
             */
            System.setProperty(
                    ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                    "org.opensplice.dds.core.OsplServiceEnvironment");
            ServiceEnvironment env = ServiceEnvironment
                    .createInstance(BuildInTopicsDataSubscriber.class.getClassLoader());

            /* Resolve the singleton DomainParticipantFactory. */
            DomainParticipantFactory dpf = DomainParticipantFactory.getInstance(env);
            DomainParticipant participant = null;

            /* Optionally use the domainId provided as input parameter. */
            /* Create a DomainParticipant with default QoS and no listener. */
            if (args.length > 0) {
                try {
                    domainId = Integer.parseInt(args[0]);
                    participant = dpf.createParticipant(domainId);
                } catch (NumberFormatException e) {
                    System.err.println("Argument must be an integer");
                    participant = dpf.createParticipant();
                }
            } else {
                participant = dpf.createParticipant();
            }

            /* Check whether the participant has been created. */
            if (participant == null) {
                System.err.println("Could not connect to domain '" + domainId
                        + "'. Is OpenSplice running?");
                return;
            } else {
                System.out.println("Connected to domain '" + participant.getDomainId() + "'.");
            }

            /* Resolve the built-in Subscriber. */
            Subscriber builtinSubscriber = participant.getBuiltinSubscriber();

            /* Lookup the DataReader for the DCPSParticipant built-in Topic. */
            DataReader<ParticipantBuiltinTopicData> reader = builtinSubscriber.lookupDataReader("DCPSParticipant");

            System.out.print("Waiting for historical data... ");

            /* Make sure all historical data is delivered in the DataReader. */
            reader.waitForHistoricalData(Duration.infiniteDuration(env));

            System.out.println("done");

            /* Allocate a new Waitset */
            WaitSet waitset = WaitSet.newWaitSet(env);

            /* Create a new ReadCondition for the reader that matches all samples. */
            DataState ds = builtinSubscriber.createDataState().withAnyInstanceState().withAnySampleState().withAnyViewState();
            ReadCondition<?> condition = reader.createReadCondition(ds);

            /* Attach the condition to the waitset. */
            waitset.attachCondition(condition);


            /* Allocate a map to store node information later on. */
            /* The key of the map is the id of the node and the value is the */
            /* number of active participants on that node. */
            HashMap<Integer, Integer> nodes = new HashMap<Integer, Integer>();

            /* Allocate a map to store node information later on. */
            /* The key of the map is the id of the node and the value is the */
            /* name of the node. */
            HashMap<Integer, String> nodeNames = new HashMap<Integer, String>();

            /*
             * Block the current thread until the attached condition becomes true or
             * the user interrupts.
             */
            waitset.waitForConditions(Duration.infiniteDuration(env));
            boolean done = false;
            Set<InstanceHandle> participants = null;
            /* Initialise the number of participants for a node. */
            int participantCount = 0;

            /* Continue processing until interrupted. */
            while (!done) {

                /* In Java5 we cannot use the reader to read ParticipantBuiltinTopicData samples
                 * so we need to use the API call getDiscoveredParticipants in combination with getDiscoveredParticipantData
                 */
                participants = participant.getDiscoveredParticipants();
                /* Walk over each participant and request its userdata */
                for (InstanceHandle ih : participants) {
                    if (ih != null) {
                        ParticipantBuiltinTopicData data = participant.getDiscoveredParticipantData(ih);
                        if (data != null) {
                            /* Resolve the node identification. */
                            int nodeId = data.getKey().getValue()[0];

                            /* Check if we saw a participant for the node before. */
                            if (nodes.containsKey(nodeId)) {
                                /*
                                 * Resolve the actual number of participants on the
                                 * node.
                                 */
                                participantCount = nodes.get(nodeId);
                            }
                            /*
                             * The splicedaemon publishes the host-name in the
                             * user_data field.
                             */
                            if (data.getUserData().getValue().length != 0) {
                                hostName = new String(data.getUserData().getValue());
                                if (!hostName.startsWith("<TunerService>"))
                                {
                                  nodeNames.put(nodeId, hostName);
                                }
                                else
                                {
                                  hostName = null;
                                }
                            } else {
                                hostName = null;
                            }

                            /* Increase the number of participants. */
                            participantCount++;

                            /* Update the number of participants for the node. */
                            nodes.put(nodeId, participantCount);

                            /* If it's the first participant, report the node is up. */
                            if (participantCount == 1) {
                                System.out.println("Node '" + nodeId
                                        + "' started (Total nodes running: "
                                        + nodes.size() + ")");
                            }
                            if (hostName != null) {
                                System.out.println("Hostname for node '" + nodeId
                                        + "' is '" + hostName + "'.");
                            }
                            /* check if a participant has left */
                            if (participantCount > participants.size()) {
                                /* Decrease the number of participants. */
                                participantCount--;

                                /*
                                 * If no more participants exist, report the node is
                                 * down.
                                 */
                                if (participantCount == 0) {
                                    hostName = nodeNames.get(nodeId);
                                    nodeNames.remove(nodeId);
                                    nodes.remove(nodeId);

                                    if (hostName != null) {
                                        System.out.println("Node " + nodeId + " ("
                                                + hostName
                                                + ") stopped (Total nodes running: "
                                                + nodes.size() + ")");
                                    } else {
                                        System.out.println("Node " + nodeId
                                                + " stopped (Total nodes running: "
                                                + nodes.size() + ")");
                                    }
                                } else if (participantCount > 0) {
                                    nodes.put(nodeId, participantCount);
                                }
                            }
                         }
                    }

                }

                if (!automatic) {
                      /* Block the current thread until the attached condition becomes
                       * true or the user interrupts.
                       */
                      System.out.println("=== [BuiltInTopicsDataSubscriber] Waiting ... ");
                      waitset.waitForConditions(Duration.infiniteDuration(env));
                      done = true;
                } else {
                    done = true;
                }
            }
        } catch(Exception e) {
            System.out.println("Error occured: " + e.getMessage());
            e.printStackTrace();
        }

    }

}
