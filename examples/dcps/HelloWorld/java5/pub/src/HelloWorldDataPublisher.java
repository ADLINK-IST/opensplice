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

import java.util.concurrent.TimeoutException;
import java.util.HashSet;
import java.util.Collection;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.Durability;
import org.omg.dds.core.policy.Partition;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.Publisher;
import org.omg.dds.topic.Topic;

import HelloWorldData.Msg;

public class HelloWorldDataPublisher {

    public static void main(String[] args) {
        // Set "serviceClassName" property to OpenSplice implementation
        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");

        // Instantiate a DDS ServiceEnvironment
        ServiceEnvironment env = ServiceEnvironment
                .createInstance(HelloWorldDataPublisher.class.getClassLoader());

        // Get the DomainParticipantFactory
        DomainParticipantFactory dpf = DomainParticipantFactory
                .getInstance(env);

        // Create a DomainParticipant with default domainID
        DomainParticipant p = dpf.createParticipant();

        // Create Reliability and Durability QoS
        Reliability r = PolicyFactory.getPolicyFactory(env).Reliability()
                .withReliable();
        Durability d = PolicyFactory.getPolicyFactory(env).Durability()
                .withTransient();

        // Create a Topic named "HelloWorldData_Msg" and with
        // "HelloWorldData.Msg" as a type.
        // "HelloWorldData.Msg" as a type.
        Collection<Class<? extends Status>> statuses = new HashSet<Class<? extends Status>>();
        Topic<Msg> topic = p.createTopic("HelloWorldData_Msg", Msg.class, p.getDefaultTopicQos().withPolicies(r, d), null , statuses);

        // Create a Partition QoS with "HelloWorld example" as partition.
        Partition partition = PolicyFactory.getPolicyFactory(env).Partition()
                .withName("HelloWorld example");

        // Create a Subscriber using default QoS except partition
        Publisher pub = p.createPublisher(p.getDefaultPublisherQos()
                .withPolicy(partition));


        // Create DataReader on our topic with default QoS except Reliability
        // and Durability
        DataWriter<Msg> writer = pub.createDataWriter(topic, pub
                .getDefaultDataWriterQos().withPolicies(r, d));

        // The message we want to publish
        Msg msg = new Msg(1, "Hello World");

        try {
            System.out
                    .println(" ________________________________________________________________");
            System.out.println("|");
            System.out.println("| Publish message : " + msg.message);
            System.out
                    .println("|________________________________________________________________");
            System.out.println("");

            // Publish the message
            writer.write(msg);
        } catch (TimeoutException e) {
            // TimeoutException may happen using Reliable QoS (if publication
            // buffers are full)
            e.printStackTrace();
        }

        try {
            // Wait to ensure data is received before we delete writer
            Thread.sleep(1000);
        } catch (InterruptedException e1) {
            e1.printStackTrace();
        }

        // Close Participant (closing also children entities: Topic, Publisher,
        // DataWriter)
        p.close();
    }
}
