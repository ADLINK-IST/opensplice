/*
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
    private StringWriter writer;

    public synchronized String serializeQoS(QoS qos) throws TransformationException {
        writer = new StringWriter();

        if(qos == null){
            throw new TransformationException("Supplied QoS not valid.");
        }
        writer.write("<object>");

        if(qos instanceof ParticipantQoS){
            writer.write("<kind>V_PARTICIPANT_QOS</kind>");
            this.serializeParticipantQoS((ParticipantQoS)qos);
        } else if(qos instanceof PublisherQoS){
            writer.write("<kind>V_PUBLISHER_QOS</kind>");
            this.serializePublisherQoS((PublisherQoS)qos);
        } else if(qos instanceof SubscriberQoS){
            writer.write("<kind>V_SUBSCRIBER_QOS</kind>");
            this.serializeSubscriberQoS((SubscriberQoS)qos);
        } else if(qos instanceof TopicQoS){
            writer.write("<kind>V_TOPIC_QOS</kind>");
            this.serializeTopicQoS((TopicQoS)qos);
        } else if(qos instanceof ReaderQoS){
            writer.write("<kind>V_READER_QOS</kind>");
            this.serializeReaderQoS((ReaderQoS)qos);
        } else if(qos instanceof WriterQoS){
            writer.write("<kind>V_WRITER_QOS</kind>");
            this.serializeWriterQoS((WriterQoS)qos);
        } else {
            throw new TransformationException("Supplied QoS type unknown: " +
                                                    qos.getClass().getName());
        }
        writer.write("</object>");
        writer.flush();
        return writer.toString();
    }

    private void serializeParticipantQoS(ParticipantQoS qos){
        this.writeEntityFactory(qos.getEntityFactory(), "entityFactory");
        this.writeUserData(qos.getUserData(), "userData");
        this.writeSchedule(qos.getWatchdogScheduling(), "watchdogScheduling");
    }

    private void serializeSubscriberQoS(SubscriberQoS qos){
        this.writePresentation(qos.getPresentation(), "presentation");
        this.writePartition(qos.getPartition(), "partition");
        this.writeGroupData(qos.getGroupData(), "groupData");
        this.writeEntityFactory(qos.getEntityFactory(), "entityFactory");
        this.writeShare(qos.getShare(), "share");
    }

    private void serializePublisherQoS(PublisherQoS qos){
        this.writePresentation(qos.getPresentation(), "presentation");
        this.writePartition(qos.getPartition(), "partition");
        this.writeGroupData(qos.getGroupData(), "groupData");
        this.writeEntityFactory(qos.getEntityFactory(), "entityFactory");
    }

    private void serializeReaderQoS(ReaderQoS qos){
        this.writeDurability(qos.getDurability(), "durability");
        this.writeDeadline(qos.getDeadline(), "deadline");
        this.writeLatency(qos.getLatency(), "latency");
        this.writeLiveliness(qos.getLiveliness(), "liveliness");
        this.writeReliability(qos.getReliability(), "reliability");
        this.writeOrderby(qos.getOrderby(), "orderby");
        this.writeHistory(qos.getHistory(), "history");
        this.writeResource(qos.getResource(), "resource");
        this.writeUserData(qos.getUserData(), "userData");
        this.writeOwnership(qos.getOwnership(), "ownership");
        this.writePacing(qos.getPacing(), "pacing");
        this.writeReaderlifecycle(qos.getLifecycle(), "lifecycle");
        this.writeReaderLifespan(qos.getLifespan(), "lifespan");
        this.writeShare(qos.getShare(), "share");
        this.writeUserKey(qos.getUserKey(), "userKey");
    }

    private void serializeWriterQoS(WriterQoS qos){
        this.writeDurability(qos.getDurability(), "durability");
        this.writeDeadline(qos.getDeadline(), "deadline");
        this.writeLatency(qos.getLatency(), "latency");
        this.writeLiveliness(qos.getLiveliness(), "liveliness");
        this.writeReliability(qos.getReliability(), "reliability");
        this.writeOrderby(qos.getOrderby(), "orderby");
        this.writeHistory(qos.getHistory(), "history");
        this.writeResource(qos.getResource(), "resource");
        this.writeTransport(qos.getTransport(), "transport");
        this.writeLifespan(qos.getLifespan(), "lifespan");
        this.writeUserData(qos.getUserData(), "userData");
        this.writeOwnership(qos.getOwnership(), "ownership");
        this.writeStrength(qos.getStrength(), "strength");
        this.writeWriterLifecycle(qos.getLifecycle(), "lifecycle");
    }

    private void serializeTopicQoS(TopicQoS qos){
        this.writeTopicData(qos.getTopicData(), "topicData");
        this.writeDurability(qos.getDurability(), "durability");
        this.writeDurabilityService(qos.getDurabilityService(), "durabilityService");
        this.writeDeadline(qos.getDeadline(), "deadline");
        this.writeLatency(qos.getLatency(), "latency");
        this.writeLiveliness(qos.getLiveliness(), "liveliness");
        this.writeReliability(qos.getReliability(), "reliability");
        this.writeOrderby(qos.getOrderby(), "orderby");
        this.writeHistory(qos.getHistory(), "history");
        this.writeResource(qos.getResource(), "resource");
        this.writeTransport(qos.getTransport(), "transport");
        this.writeLifespan(qos.getLifespan(), "lifespan");
        this.writeOwnership(qos.getOwnership(), "ownership");
    }
    /*
    private void serializeViewQoS(ViewQoS qos){
        this.writeKeyPolicy(qos.getKeyList(), "keyList");
    }
    */
    private void writeEntityFactory(EntityFactoryPolicy policy, String name){
        writer.write("<" + name + "><autoenable_created_entities>" +
                        this.getBoolean(policy.autoenable_created_entities) +
                        "</autoenable_created_entities></" + name + ">");
    }

    private void writeUserData(UserDataPolicy policy, String name){
        String size;

        writer.write("<" + name + "><value>");

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
        writer.write("</value><size>" + size + "</size></" + name + ">");
    }

    private void writeGroupData(GroupDataPolicy policy, String name){
        String size;

        writer.write("<" + name + "><value>");

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
        writer.write("</value><size>" + size + "</size></" + name + ">");
    }

    private void writeTopicData(TopicDataPolicy policy, String name){
        String size;

        writer.write("<" + name + "><value>");

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
        writer.write("</value><size>" + size + "</size></" + name + ">");
    }

    private void writeTransport(TransportPolicy policy, String name){
        writer.write("<" + name + "><value>" + policy.value +
                        "</value></" + name + ">");
    }

    private void writeLifespan(LifespanPolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.duration, "duration");
        writer.write("</" + name + ">");
    }

    private void writeSchedule(SchedulePolicy policy, String name){
        writer.write("<" + name + "><kind>");
        writer.write(policy.kind.toKernelString());
        writer.write("</kind><priorityKind>");
        writer.write(policy.priorityKind.toKernelString());
        writer.write("</priorityKind><priority>");
        writer.write(Integer.toString(policy.priority));
        writer.write("</priority></" + name + ">");
    }

    private void writeDurability(DurabilityPolicy policy, String name){
        writer.write("<" + name + "><kind>" + policy.kind.toKernelString() + "</kind></" + name + ">");
    }

    private void writeDurabilityService(DurabilityServicePolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.service_cleanup_delay, "service_cleanup_delay");
        writer.write("<history_kind>" +
                            policy.history_kind.toKernelString() +
                      "</history_kind>");
        writer.write("<history_depth>" + policy.history_depth + "</history_depth>");
        writer.write("<max_samples>" + policy.max_samples +
                     "</max_samples><max_instances>" + policy.max_instances +
                     "</max_instances><max_samples_per_instance>" +
                     policy.max_samples_per_instance +
                     "</max_samples_per_instance>");
        writer.write("</" + name + ">");
    }

    private void writePresentation(PresentationPolicy policy, String name){
        writer.write("<" + name + "><access_scope>" +
                        policy.access_scope.toKernelString() + "</access_scope>" +
                        "<coherent_access>" +
                            this.getBoolean(policy.coherent_access) +
                        "</coherent_access>" +
                        "<ordered_access>" +
                            this.getBoolean(policy.ordered_access) +
                        "</ordered_access>" +
                     "</" + name + ">");
    }

    private void writeDeadline(DeadlinePolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.period, "period");
        writer.write("</" + name + ">");
    }

    private void writeLatency(LatencyPolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.duration, "duration");
        writer.write("</" + name + ">");
    }

    private void writeOwnership(OwnershipPolicy policy, String name){
        writer.write("<" + name + "><kind>" + policy.kind.toKernelString() +
                        "</kind></" + name + ">");
    }

    private void writeStrength(StrengthPolicy policy, String name){
        writer.write("<" + name + "><value>" + policy.value + "</value></" + name + ">");
    }

    private void writeLiveliness(LivelinessPolicy policy, String name){
        writer.write("<" + name + "><kind>" + policy.kind.toKernelString() + "</kind>");
        this.writeTime(policy.lease_duration, "lease_duration");
        writer.write("</" + name + ">");
    }

    private void writeReliability(ReliabilityPolicy policy, String name){
        writer.write("<" + name + "><kind>" +
                        policy.kind.toKernelString() +
                     "</kind>");
        this.writeTime(policy.max_blocking_time, "max_blocking_time");
        writer.write("<synchronous>" +
                        this.getBoolean(policy.synchronous) +
                     "</synchronous>");
        writer.write("</" + name + ">");
    }

    private void writeHistory(HistoryPolicy policy, String name){
        writer.write("<" + name + "><kind>" + policy.kind.toKernelString() +
                "</kind><depth>" + policy.depth + "</depth></" + name + ">");
    }

    private void writeOrderby(OrderbyPolicy policy, String name){
        writer.write("<" + name + "><kind>" + policy.kind.toKernelString() +
                "</kind></" + name + ">");
    }

    private void writeResource(ResourcePolicy policy, String name){
        writer.write("<" + name + "><max_samples>" + policy.max_samples +
                "</max_samples><max_instances>" + policy.max_instances +
                "</max_instances><max_samples_per_instance>" +
                policy.max_samples_per_instance +
                "</max_samples_per_instance></" + name + ">");
    }

    private void writePacing(PacingPolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.minSeperation, "minSeperation");
        writer.write("</" + name + ">");
    }

    private void writeShare(SharePolicy share, String name){
        writer.write("<" + name + ">");

        if(share.name == null){
            writer.write("<name>&lt;NULL&gt;</name>");
        } else {
            writer.write("<name>" + share.name + "</name>");
        }
        writer.write("<enable>" + this.getBoolean(share.enable) + "</enable>");
        writer.write("</" + name + ">");
    }

    private void writeUserKey(UserKeyPolicy uk, String name){
        writer.write("<" + name + ">");

        writer.write("<enable>" + this.getBoolean(uk.enable) + "</enable>");

        if(uk.expression == null){
            writer.write("<expression>&lt;NULL&gt;</expression>");
        } else {
            writer.write("<expression>" + uk.expression + "</expression>");
        }
        writer.write("</" + name + ">");
    }

    private void writeWriterLifecycle(WriterLifecyclePolicy policy, String name){
        writer.write("<" + name + "><autodispose_unregistered_instances>" +
                        this.getBoolean(policy.autodispose_unregistered_instances) +
                        "</autodispose_unregistered_instances>");
        this.writeTime(policy.autopurge_suspended_samples_delay, "autopurge_suspended_samples_delay");
        this.writeTime(policy.autounregister_instance_delay, "autounregister_instance_delay");
        writer.write("</" + name + ">");
    }

    private void writeReaderlifecycle(ReaderLifecyclePolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.autopurge_nowriter_samples_delay,
                        "autopurge_nowriter_samples_delay");
        this.writeTime(policy.autopurge_disposed_samples_delay,
                        "autopurge_disposed_samples_delay");
        writer.write("<enable_invalid_samples>" +
                        getBoolean(policy.enable_invalid_samples)  +
                     "</enable_invalid_samples>");
        writer.write("</" + name + ">");
    }

    private void writeReaderLifespan(ReaderLifespanPolicy policy, String name){
        writer.write("<" + name + ">");
        writer.write("<used>" + getBoolean(policy.used) + "</used>");
        this.writeTime(policy.duration,
                        "duration");
        writer.write("</" + name + ">");
    }
    /*
    private void writeSimulation(SimulationPolicy policy, String name){
        writer.write("<" + name + ">");
        this.writeTime(policy.oldTime, "oldTime");
        this.writeTime(policy.newTime, "newTime");
        writer.write("<relativeSpeed>" + policy.relativeSpeed +
                     "</relativeSpeed></" + name + ">");
    }
    */
    private void writePartition(String expression, String name){
        if(expression == null){
            writer.write("<" + name + ">&lt;NULL&gt;</" + name + ">");
        } else {
            writer.write("<" + name + ">" + expression + "</" + name + ">");
        }
    }
    /*
    private void writeKeyPolicy(String keyPolicy, String name){
        writer.write("<" + name + "><expression>" + keyPolicy + "</expression></" + name + ">");
    }
    */
    private void writeTime(Time time, String name){
        writer.write("<" + name + "><seconds>" + time.sec  +
                        "</seconds><nanoseconds>" + time.nsec +
                        "</nanoseconds></" + name + ">");
    }

    private String getBoolean(boolean bool){
        return Boolean.toString(bool).toUpperCase();
    }
}
