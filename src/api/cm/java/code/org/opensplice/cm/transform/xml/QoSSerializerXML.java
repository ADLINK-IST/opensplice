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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;

import org.opensplice.cm.Time;
import org.opensplice.cm.qos.*;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * The XML implementation of an QoSDeserializer. It is capable of
 * transforming a serialized XML representation to a QoS object.
 *
 * @date Feb 1, 2005
 */
public class QoSSerializerXML implements QoSSerializer{
    protected StringWriter writer;

    @Override
    public synchronized String serializeQoS(QoS qos)
            throws TransformationException {
        return this.serializeQoS(qos, false);
    }
    
    public synchronized String serializeQoSLegacyMode(QoS qos)
            throws TransformationException {
        return this.serializeQoS(qos, true);
    }

    private String serializeQoS(QoS qos, boolean legacyMode)
            throws TransformationException {
        writer = new StringWriter();

        if(qos == null){
            throw new TransformationException("Supplied QoS not valid.");
        }
        writer.write("<object>");

        if(qos instanceof ParticipantQoS){
            writer.write("<kind>V_PARTICIPANT_QOS</kind>");
            this.serializeParticipantQoS((ParticipantQoS) qos, legacyMode);
        } else if(qos instanceof PublisherQoS){
            writer.write("<kind>V_PUBLISHER_QOS</kind>");
            this.serializePublisherQoS((PublisherQoS) qos, legacyMode);
        } else if(qos instanceof SubscriberQoS){
            writer.write("<kind>V_SUBSCRIBER_QOS</kind>");
            this.serializeSubscriberQoS((SubscriberQoS) qos, legacyMode);
        } else if(qos instanceof TopicQoS){
            writer.write("<kind>V_TOPIC_QOS</kind>");
            this.serializeTopicQoS((TopicQoS) qos, legacyMode);
        } else if(qos instanceof ReaderQoS){
            writer.write("<kind>V_READER_QOS</kind>");
            this.serializeReaderQoS((ReaderQoS) qos, legacyMode);
        } else if(qos instanceof WriterQoS){
            writer.write("<kind>V_WRITER_QOS</kind>");
            this.serializeWriterQoS((WriterQoS) qos, legacyMode);
        } else {
            throw new TransformationException("Supplied QoS type unknown: " +
                                                    qos.getClass().getName());
        }
        writer.write("</object>");
        writer.flush();
        return writer.toString();
    }

    private void serializeParticipantQoS(ParticipantQoS qos, boolean legacyMode) {
        this.writeEntityFactory(qos.getEntityFactory(), "entityFactory",
                legacyMode);
        this.writeUserData(qos.getUserData(), "userData", legacyMode);
        this.writeSchedule(qos.getWatchdogScheduling(), "watchdogScheduling",
                legacyMode);
    }

    private void serializeSubscriberQoS(SubscriberQoS qos, boolean legacyMode) {
        this.writePresentation(qos.getPresentation(), "presentation",
                legacyMode);
        this.writePartition(qos.getPartition(), "partition", legacyMode);
        this.writeGroupData(qos.getGroupData(), "groupData", legacyMode);
        this.writeEntityFactory(qos.getEntityFactory(), "entityFactory",
                legacyMode);
        this.writeShare(qos.getShare(), "share", legacyMode);
    }

    private void serializePublisherQoS(PublisherQoS qos, boolean legacyMode) {
        this.writePresentation(qos.getPresentation(), "presentation",
                legacyMode);
        this.writePartition(qos.getPartition(), "partition", legacyMode);
        this.writeGroupData(qos.getGroupData(), "groupData", legacyMode);
        this.writeEntityFactory(qos.getEntityFactory(), "entityFactory",
                legacyMode);
    }

    private void serializeReaderQoS(ReaderQoS qos, boolean legacyMode) {
        this.writeDurability(qos.getDurability(), "durability", legacyMode);
        this.writeDeadline(qos.getDeadline(), "deadline", legacyMode);
        this.writeLatency(qos.getLatency(), "latency", legacyMode);
        this.writeLiveliness(qos.getLiveliness(), "liveliness", legacyMode);
        this.writeReliability(qos.getReliability(), "reliability", legacyMode);
        this.writeOrderby(qos.getOrderby(), "orderby", legacyMode);
        this.writeHistory(qos.getHistory(), "history", legacyMode);
        this.writeResource(qos.getResource(), "resource", legacyMode);
        this.writeUserData(qos.getUserData(), "userData", legacyMode);
        this.writeOwnership(qos.getOwnership(), "ownership", legacyMode);
        this.writePacing(qos.getPacing(), "pacing", legacyMode);
        this.writeReaderlifecycle(qos.getLifecycle(), "lifecycle", legacyMode);
        this.writeReaderLifespan(qos.getLifespan(), "lifespan", legacyMode);
        this.writeShare(qos.getShare(), "share", legacyMode);
        this.writeUserKey(qos.getUserKey(), "userKey", legacyMode);
    }

    private void serializeWriterQoS(WriterQoS qos, boolean legacyMode) {
        this.writeDurability(qos.getDurability(), "durability", legacyMode);
        this.writeDeadline(qos.getDeadline(), "deadline", legacyMode);
        this.writeLatency(qos.getLatency(), "latency", legacyMode);
        this.writeLiveliness(qos.getLiveliness(), "liveliness", legacyMode);
        this.writeReliability(qos.getReliability(), "reliability", legacyMode);
        this.writeOrderby(qos.getOrderby(), "orderby", legacyMode);
        this.writeHistory(qos.getHistory(), "history", legacyMode);
        this.writeResource(qos.getResource(), "resource", legacyMode);
        this.writeTransport(qos.getTransport(), "transport", legacyMode);
        this.writeLifespan(qos.getLifespan(), "lifespan", legacyMode);
        this.writeUserData(qos.getUserData(), "userData", legacyMode);
        this.writeOwnership(qos.getOwnership(), "ownership", legacyMode);
        this.writeStrength(qos.getStrength(), "strength", legacyMode);
        this.writeWriterLifecycle(qos.getLifecycle(), "lifecycle", legacyMode);
    }

    private void serializeTopicQoS(TopicQoS qos, boolean legacyMode) {
        this.writeTopicData(qos.getTopicData(), "topicData", legacyMode);
        this.writeDurability(qos.getDurability(), "durability", legacyMode);
        this.writeDurabilityService(qos.getDurabilityService(),
                "durabilityService", legacyMode);
        this.writeDeadline(qos.getDeadline(), "deadline", legacyMode);
        this.writeLatency(qos.getLatency(), "latency", legacyMode);
        this.writeLiveliness(qos.getLiveliness(), "liveliness", legacyMode);
        this.writeReliability(qos.getReliability(), "reliability", legacyMode);
        this.writeOrderby(qos.getOrderby(), "orderby", legacyMode);
        this.writeHistory(qos.getHistory(), "history", legacyMode);
        this.writeResource(qos.getResource(), "resource", legacyMode);
        this.writeTransport(qos.getTransport(), "transport", legacyMode);
        this.writeLifespan(qos.getLifespan(), "lifespan", legacyMode);
        this.writeOwnership(qos.getOwnership(), "ownership", legacyMode);
    }

    private void writeEntityFactory(EntityFactoryPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<autoenable_created_entities>"
                + this.getBoolean(policy.autoenable_created_entities)
                + "</autoenable_created_entities>");
        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeUserData(UserDataPolicy policy, String name,
            boolean legacyMode) {
        String size;

        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<value>");

        if(policy.value == null){
            writer.write("&lt;NULL&gt;");
            size = "0";
        } else {
            size = Integer.toString(policy.value.length);
            writer.write("<size>" + size + "</size>");

            for(int i=0; i<policy.value.length;i++){
                writer.write("<element>" + policy.value[i] + "</element>");
            }
        }
        writer.write("</value><size>" + size + "</size>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeGroupData(GroupDataPolicy policy, String name,
            boolean legacyMode) {
        String size;

        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<value>");

        if(policy.value == null){
            writer.write("&lt;NULL&gt;");
            size = "0";
        } else {
            size = Integer.toString(policy.value.length);
            writer.write("<size>" + size + "</size>");

            for(int i=0; i<policy.value.length;i++){
                writer.write("<element>" + policy.value[i] + "</element>");
            }
        }
        writer.write("</value><size>" + size + "</size>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeTopicData(TopicDataPolicy policy, String name,
            boolean legacyMode) {
        String size;

        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<value>");

        if(policy.value == null){
            writer.write("&lt;NULL&gt;");
            size = "0";
        } else {
            size = Integer.toString(policy.value.length);
            writer.write("<size>" + size + "</size>");

            for(int i=0; i<policy.value.length;i++){
                writer.write("<element>" + policy.value[i] + "</element>");
            }
        }
        writer.write("</value><size>" + size + "</size>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeTransport(TransportPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<value>" + policy.value + "</value>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeLifespan(LifespanPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        this.writeDuration(policy.duration, "duration");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeSchedule(SchedulePolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>");
        writer.write(policy.kind.toKernelString());
        writer.write("</kind><priorityKind>");
        writer.write(policy.priorityKind.toKernelString());
        writer.write("</priorityKind><priority>");
        writer.write(Integer.toString(policy.priority));
        writer.write("</priority>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeDurability(DurabilityPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>" + policy.kind.toKernelString() + "</kind>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeDurabilityService(DurabilityServicePolicy policy,
            String name, boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        this.writeDuration(policy.service_cleanup_delay, "service_cleanup_delay");
        writer.write("<history_kind>" +
                            policy.history_kind.toKernelString() +
                      "</history_kind>");
        writer.write("<history_depth>" + policy.history_depth + "</history_depth>");
        writer.write("<max_samples>" + policy.max_samples +
                     "</max_samples><max_instances>" + policy.max_instances +
                     "</max_instances><max_samples_per_instance>" +
                     policy.max_samples_per_instance +
                     "</max_samples_per_instance>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writePresentation(PresentationPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write(  "<access_scope>" +
                            policy.access_scope.toKernelString() +
                        "</access_scope>" +
                        "<coherent_access>" +
                            this.getBoolean(policy.coherent_access) +
                        "</coherent_access>" +
                        "<ordered_access>" +
                            this.getBoolean(policy.ordered_access) +
                        "</ordered_access>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeDeadline(DeadlinePolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        this.writeDuration(policy.period, "period");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeLatency(LatencyPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        this.writeDuration(policy.duration, "duration");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeOwnership(OwnershipPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>" + policy.kind.toKernelString() + "</kind>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeStrength(StrengthPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<value>" + policy.value + "</value>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeLiveliness(LivelinessPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>" + policy.kind.toKernelString() + "</kind>");
        this.writeDuration(policy.lease_duration, "lease_duration");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeReliability(ReliabilityPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>" +
                        policy.kind.toKernelString() +
                     "</kind>");
        this.writeDuration(policy.max_blocking_time, "max_blocking_time");
        writer.write("<synchronous>" +
                        this.getBoolean(policy.synchronous) +
                     "</synchronous>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeHistory(HistoryPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>" + policy.kind.toKernelString() + "</kind><depth>"
                + policy.depth + "</depth>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeOrderby(OrderbyPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");
        
        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<kind>" + policy.kind.toKernelString() +
                "</kind>");
        
        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeResource(ResourcePolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");
        
        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<max_samples>" + policy.max_samples +
                "</max_samples><max_instances>" + policy.max_instances +
                "</max_instances><max_samples_per_instance>" +
                policy.max_samples_per_instance +
                "</max_samples_per_instance>");
        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writePacing(PacingPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");
        if (!legacyMode) {
            writer.write("<v>");
        }
        this.writeDuration(policy.minSeperation, "minSeperation");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeShare(SharePolicy share, String name, boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }

        if(share.name == null){
            writer.write("<name>&lt;NULL&gt;</name>");
        } else {
            writer.write("<name>" + share.name + "</name>");
        }
        writer.write("<enable>" + this.getBoolean(share.enable) + "</enable>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeUserKey(UserKeyPolicy uk, String name, boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<enable>" + this.getBoolean(uk.enable) + "</enable>");

        if(uk.expression == null){
            writer.write("<expression>&lt;NULL&gt;</expression>");
        } else {
            writer.write("<expression>" + uk.expression + "</expression>");
        }
        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeWriterLifecycle(WriterLifecyclePolicy policy,
            String name, boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<autodispose_unregistered_instances>"
                +
                        this.getBoolean(policy.autodispose_unregistered_instances) +
                        "</autodispose_unregistered_instances>");
        this.writeDuration(policy.autopurge_suspended_samples_delay, "autopurge_suspended_samples_delay");
        this.writeDuration(policy.autounregister_instance_delay, "autounregister_instance_delay");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeReaderlifecycle(ReaderLifecyclePolicy policy,
            String name, boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        this.writeDuration(policy.autopurge_nowriter_samples_delay,
                        "autopurge_nowriter_samples_delay");
        this.writeDuration(policy.autopurge_disposed_samples_delay,
                        "autopurge_disposed_samples_delay");
        writer.write("<autopurge_dispose_all>" +
                        getBoolean(policy.autopurge_dispose_all)  +
                     "</autopurge_dispose_all>");
        writer.write("<enable_invalid_samples>" +
                        getBoolean(policy.enable_invalid_samples)  +
                     "</enable_invalid_samples>");
        writer.write("<invalid_sample_visibility>" +
                         policy.invalid_sample_visibility.toKernelString() +
                     "</invalid_sample_visibility>");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writeReaderLifespan(ReaderLifespanPolicy policy, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        writer.write("<used>" + getBoolean(policy.used) + "</used>");
        this.writeDuration(policy.duration,
                        "duration");

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    private void writePartition(String expression, String name,
            boolean legacyMode) {
        writer.write("<" + name + ">");

        if (!legacyMode) {
            writer.write("<v>");
        }
        if (expression == null) {
            writer.write("&lt;NULL&gt;");
        } else {
            writer.write(expression);
        }

        if (!legacyMode) {
            writer.write("</v>");
        }
        writer.write("</" + name + ">");
    }

    protected void writeDuration(Time time, String name){
        writer.write("<" + name + "><seconds>" + time.sec  +
                        "</seconds><nanoseconds>" + time.nsec +
                        "</nanoseconds></" + name + ">");
    }

    private String getBoolean(boolean bool){
        return Boolean.toString(bool).toUpperCase();
    }
}
