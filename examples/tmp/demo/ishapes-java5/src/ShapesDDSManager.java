/**
 *                             Vortex Cafe
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
 */

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;

import org.omg.dds.core.DDSException;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.QosPolicy.ForDataReader;
import org.omg.dds.core.policy.QosPolicy.ForDataWriter;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.Publisher;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.topic.Topic;
import org.omg.dds.type.TypeSupport;

public class ShapesDDSManager {

    static private final Logger log = Logger.getInstance();

    private int domainId;
    private String partition;

    private DomainParticipantFactory dpf = null;
    private DomainParticipant dp = null;
    private PolicyFactory pf = null;
    private Publisher pub = null;
    private Subscriber sub = null;

    private Map<ShapeKind, Topic<ShapeType>> topics;

    public ShapesDDSManager() {
        // get domainId and partition
        domainId = Integer.parseInt(System.getProperty("dds.domainId", "0"));
        partition = System.getProperty("dds.partition", "");
        topics = new HashMap<ShapeKind, Topic<ShapeType>>(
                ShapeKind.values().length);
        log.log("DDS will use Domain " + domainId + " and Partition \""
                + partition + "\"", Level.INFO);
    }

    public void init() throws DDSException {
        // Select DDS implementation and initialize DDS ServiceEnvironment
        log.log("Load DDS implementation", Level.INFO);

        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");
        ServiceEnvironment env = ServiceEnvironment
                .createInstance(ShapesDDSManager.class.getClassLoader());

        // create Participant
        log.log("Create Participant", Level.INFO);
        dpf = DomainParticipantFactory.getInstance(env);
        dp = dpf.createParticipant(domainId);

        // get PolicyFactory
        pf = PolicyFactory.getPolicyFactory(env);

        // create TypeSupport to use a specific name for type registration
        TypeSupport<ShapeType> typeSupport = TypeSupport.newTypeSupport(
                ShapeType.class, ShapesConstants.TYPE_NAME_TO_REGISTER, env);

        // create topics
        log.log("Create Topics", Level.INFO);
        topics.put(ShapeKind.CIRCLE, dp.createTopic("Circle", typeSupport));
        topics.put(ShapeKind.SQUARE, dp.createTopic("Square", typeSupport));
        topics.put(ShapeKind.TRIANGLE, dp.createTopic("Triangle", typeSupport));

        // create publisher
        log.log("Create Publisher", Level.INFO);
        pub = dp.createPublisher(dp.getDefaultPublisherQos().withPolicy(
                pf.Partition().withName(Arrays.asList(partition))));

        // create subsciber
        log.log("Create Subscriber", Level.INFO);
        sub = dp.createSubscriber(dp.getDefaultSubscriberQos().withPolicy(
                pf.Partition().withName(Arrays.asList(partition))));
    }

    public PolicyFactory getPolicyFactory() {
        return pf;
    }

    public DataReader<ShapeType> getDataReader(ShapeKind shapeKind,
            ForDataReader... qos) {
        log.log("Create DataReader", Level.INFO);
        return sub.createDataReader(topics.get(shapeKind), sub
                .getDefaultDataReaderQos().withPolicies(qos));
    }

    public DataWriter<ShapeType> getDataWriter(ShapeKind shapeKind,
            ForDataWriter... qos) {
        log.log("Create DataWriter", Level.INFO);
        return pub.createDataWriter(topics.get(shapeKind), pub
                .getDefaultDataWriterQos().withPolicies(qos));
    }

    public void close() {
        try {
            dp.closeContainedEntities();
            dp.close();
        } catch (DDSException e) {
            log.log("Error closing DDS: " + e + " caused by " + e.getCause(),
                    Level.SEVERE);
            // re-throw for stack in logs
            throw e;
        }
    }
}
