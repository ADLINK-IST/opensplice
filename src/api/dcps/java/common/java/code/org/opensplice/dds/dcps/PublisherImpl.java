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


package org.opensplice.dds.dcps;

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/**
 * Implementation of the {@link DDS.Publisher} interface.
 */
public class PublisherImpl extends PublisherBase implements DDS.Publisher {

    private static final long serialVersionUID = 518234841867708066L;
    private String name;
    private DomainParticipantImpl participant = null;
    private DDS.DataWriterQos defaultDataWriterQos = Utilities.defaultDataWriterQos;
    private final Set<DataWriterImpl> writers = Collections.synchronizedSet(new HashSet<DataWriterImpl>());
    private DDS.PublisherListener listener = null;
    private boolean factoryAutoEnable = false;

    protected PublisherImpl() { }

    protected int init (
        DomainParticipantImpl participant,
        String name,
        DDS.PublisherQos qos)
    {
        int result = DDS.RETCODE_OK.value;

        assert (participant != null);
        assert (qos != null);

        long uParticipant = participant.get_user_object();
        if (uParticipant != 0) {
            long uPublisher = jniPublisherNew(uParticipant, name, qos);
            if (uPublisher != 0) {
                this.set_user_object(uPublisher);
                this.name = name;
                this.factoryAutoEnable = qos.entity_factory.autoenable_created_entities;
                this.participant = participant;
                this.setDomainId(participant.getDomainId());
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;
        long uPublisher;

        synchronized (this)
        {
            uPublisher = this.get_user_object();
            if (uPublisher != 0) {
                if (this.writers.size() == 0) {
                    if (this.listener != null) {
                        /* Should always be done before disable_callbacks(), so
                         * that events that can trigger a listener on an owning
                         * entity are propagated instead of being consumed by
                         * the listener being destroyed. */
                        result = set_listener(this.listener, 0);
                    }
                    if (result == DDS.RETCODE_OK.value) {
                        result = this.disable_callbacks();
                        if (result == DDS.RETCODE_OK.value) {
                            result = ((EntityImpl) this).detach_statuscondition();
                            if (result == DDS.RETCODE_OK.value) {
                                this.participant = null;
                                this.name = null;
                                result = jniPublisherFree(uPublisher);
                                if (result == DDS.RETCODE_OK.value) {
                                    result = super.deinit();
                                }
                            }
                        }
                    }
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                        "Publisher still contains '" + this.writers.size() +
                        "' DataWriter entities.");
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public DDS.DataWriter create_datawriter (
        DDS.Topic a_topic,
        DDS.DataWriterQos qos,
        DDS.DataWriterListener a_listener,
        int mask)
    {
        DataWriterImpl writer = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (a_topic == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_topic 'null' is invalid.");
        } else if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAWRITER_QOS_DEFAULT.value) {
            qos = this.defaultDataWriterQos;
        } else if (qos == DDS.DATAWRITER_QOS_USE_TOPIC_QOS.value) {
            DDS.TopicQosHolder topicQosHolder = new DDS.TopicQosHolder();
            a_topic.get_qos(topicQosHolder);

            DDS.DataWriterQosHolder writerQosHolder = new DDS.DataWriterQosHolder();
            result = this.copy_from_topic_qos(writerQosHolder, topicQosHolder.value);
            if (result == DDS.RETCODE_OK.value) {
                qos = writerQosHolder.value;
                result = Utilities.checkQos(qos);
            }
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            synchronized(this) {
                long uPublisher = this.get_user_object();

                if (uPublisher != 0) {
                    TopicImpl topic = (TopicImpl) a_topic;

                    if (this.participant.equals(topic.get_participant())) {
                        String name = "writer <" + topic.get_name() + ">";
                        writer = (DataWriterImpl) topic.create_datawriter();

                        if (writer != null) {
                            result = writer.init(this, name, topic, qos);
                            if (result == DDS.RETCODE_OK.value) {
                                this.writers.add(writer);
                                ListenerDispatcher dispatcher = this.get_dispatcher();
                                result = writer.set_dispatcher(dispatcher);
                            } else {
                                writer = null;
                            }
                            if (result == DDS.RETCODE_OK.value) {
                                result = writer.set_listener(a_listener, mask);
                            }
                            if (result == DDS.RETCODE_OK.value) {
                                if (this.factoryAutoEnable) {
                                    result = writer.enable();
                                }
                            }

                            if (result != DDS.RETCODE_OK.value && writer != null) {
                                this.delete_datawriter(writer);
                                writer = null;
                            }
                        }
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack
                                .report(result,
                                        "Topic does belong to the same DomainParticipant as this Publisher.");
                    }
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return writer;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int delete_datawriter (
        DDS.DataWriter writer)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (writer == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "DataWriter 'null' is invalid.");
        } else {

            DataWriterImpl dw = (DataWriterImpl)writer;
            boolean removed = false;
            synchronized (this) {
                removed = this.writers.remove(dw);

                if (removed) {
                    result = dw.deinit();
                    if (result == DDS.RETCODE_PRECONDITION_NOT_MET.value) {
                        this.writers.add(dw);
                    }
                }  else {
                    if (dw.get_user_object() == 0) {
                        result = DDS.RETCODE_ALREADY_DELETED.value;
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack.report(result, "DataWriter not created by Publisher");
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public DDS.DataWriter lookup_datawriter (
        String topic_name)
    {
        DataWriterImpl found = null;
        ReportStack.start();

        synchronized (this.writers)
        {
            Iterator<DataWriterImpl> i = this.writers.iterator();
            while ((found == null) && (i.hasNext())) {
                found = i.next();
                if (!found.get_topic().get_name().equals(topic_name)) {
                    found = null;
                }
            }
        }

        ReportStack.flush(this, false);
        return found;
    }

    public boolean contains_entity (
        long a_handle)
    {
        boolean found = false;
        long handle;
        ReportStack.start();

        synchronized (this.writers)
        {
            Iterator<DataWriterImpl> it = this.writers.iterator();
            while (it.hasNext() && !found) {
                handle = it.next().get_instance_handle();
                found = (handle == a_handle);
            }
        }

        ReportStack.flush(this, false);
        return found;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int delete_contained_entities ()
    {
        int result, endResult = DDS.RETCODE_OK.value;
        ReportStack.start();

        HashSet<DataWriterImpl> survivors = new HashSet<DataWriterImpl>();
        DataWriterImpl dw = null;
        do {
            dw = null;
            synchronized (this.writers) {
                Iterator<DataWriterImpl> dwi = this.writers.iterator();
                while (dwi.hasNext()) {
                    dw = dwi.next();
                    dwi.remove();
                    if (!survivors.contains(dw)) {
                        break;
                    }
                }
            }

            if (dw != null) {
                result = dw.deinit();
                if (result != DDS.RETCODE_OK.value && result != DDS.RETCODE_ALREADY_DELETED.value) {
                    survivors.add(dw);
                    synchronized (this.writers) {
                        this.writers.add(dw);
                    }
                }
                if (result != DDS.RETCODE_OK.value) {
                    ReportStack.report(result, "Deletion of DataWriter contained in Publisher failed.");
                    if (endResult == DDS.RETCODE_OK.value) {
                        /* Store first encountered error. */
                        endResult = result;
                    }
                }
            }
        } while (dw != null);

        ReportStack.flush(this, endResult != DDS.RETCODE_OK.value);
        return endResult;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int set_qos (DDS.PublisherQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.PUBLISHER_QOS_DEFAULT.value) {
            DDS.PublisherQosHolder holder = new DDS.PublisherQosHolder();
            this.participant.get_default_publisher_qos(holder);
            qos = holder.value;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uPublisher = this.get_user_object();
            if (uPublisher != 0) {
                result = jniSetQos(uPublisher, qos);
                if (result == DDS.RETCODE_OK.value) {
                    this.factoryAutoEnable = qos.entity_factory.autoenable_created_entities;
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int get_qos (DDS.PublisherQosHolder qosHolder)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qosHolder == null) {
            result = DDS.RETCODE_OK.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            long uPublisher = this.get_user_object();
            if (uPublisher != 0) {
                result = jniGetQos(uPublisher, qosHolder);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int set_listener (DDS.PublisherListener a_listener, int mask)
    {
        long uPublisher = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            uPublisher = this.get_user_object();
            if (uPublisher != 0) {
                this.listener = a_listener;
                result = this.set_listener_interest(mask);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public DDS.PublisherListener get_listener ()
    {
        return listener;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int suspend_publications ()
    {
        long uPublisher = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uPublisher = this.get_user_object();
        if (uPublisher != 0) {
            result = jniSuspendPublications(uPublisher);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int resume_publications ()
    {
        long uPublisher = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uPublisher = this.get_user_object();
        if (uPublisher != 0) {
            result = jniResumePublications(uPublisher);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int begin_coherent_changes ()
    {
        long uPublisher = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uPublisher = this.get_user_object();
        if (uPublisher != 0) {
            result = jniBeginCoherentChanges(uPublisher);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int end_coherent_changes ()
    {
        long uPublisher = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uPublisher = this.get_user_object();
        if (uPublisher != 0) {
            result = jniEndCoherentChanges(uPublisher);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int wait_for_acknowledgments(
        DDS.Duration_t max_wait)
    {
        long uPublisher = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        result = Utilities.checkDuration (max_wait);
        if (result == DDS.RETCODE_OK.value) {
            uPublisher = this.get_user_object();

            if (uPublisher != 0) {
                result = jniWaitForAcknowledgments(uPublisher, max_wait);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public DDS.DomainParticipant get_participant ()
    {
        DDS.DomainParticipant participant = null;
        ReportStack.start();

        if (this.get_user_object() != 0) {
           participant = this.participant;
        }

        ReportStack.flush(this, participant == null);
        return participant;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int set_default_datawriter_qos (
        DDS.DataWriterQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAWRITER_QOS_USE_TOPIC_QOS.value) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is invalid in this context.");
        } else if (qos == DDS.DATAWRITER_QOS_DEFAULT.value) {
            qos = Utilities.defaultDataWriterQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            if (this.get_user_object() != 0) {
                this.defaultDataWriterQos = Utilities.deepCopy(qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int get_default_datawriter_qos (
        DDS.DataWriterQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (this.get_user_object() == 0) {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        } else {
            qos.value = Utilities.deepCopy(this.defaultDataWriterQos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PublisherOperations for javadoc */
    @Override
    public int copy_from_topic_qos (
        DDS.DataWriterQosHolder qosHolder,
        DDS.TopicQos topic_qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qosHolder == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_datawriter_qos 'null' is invalid.");
        } else if (topic_qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_topic_qos 'null' is invalid.");
        } else if (topic_qos == DDS.TOPIC_QOS_DEFAULT.value) {
            DDS.TopicQosHolder holder = new DDS.TopicQosHolder();
            this.participant.get_default_topic_qos(holder);
            topic_qos = holder.value;
        } else {
            result = Utilities.checkQos(topic_qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            DDS.DataWriterQos qos = qosHolder.value;
            if (qos == null) {
                qos = Utilities.deepCopy(this.defaultDataWriterQos);
            }
            qos.durability = Utilities.deepCopy(topic_qos.durability);
            qos.deadline = Utilities.deepCopy(topic_qos.deadline);
            qos.latency_budget = Utilities.deepCopy(topic_qos.latency_budget);
            qos.liveliness = Utilities.deepCopy(topic_qos.liveliness);
            qos.reliability = Utilities.deepCopy(topic_qos.reliability);
            qos.destination_order = Utilities.deepCopy(topic_qos.destination_order);
            qos.history = Utilities.deepCopy(topic_qos.history);
            qos.resource_limits = Utilities.deepCopy(topic_qos.resource_limits);
            qos.transport_priority = Utilities.deepCopy(topic_qos.transport_priority);
            qos.lifespan = Utilities.deepCopy(topic_qos.lifespan);
            qos.ownership = Utilities.deepCopy(topic_qos.ownership);
            qosHolder.value = qos;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    protected int notify(Event e)
    {
        int result = DDS.RETCODE_OK.value;

        DDS.PublisherListener pl = this.listener;

        if (pl == null)
            return result;

        switch(e.kind) {
            case DDS.OFFERED_DEADLINE_MISSED_STATUS.value:
                    pl.on_offered_deadline_missed(
                            (DDS.DataWriter) e.observable,
                            (DDS.OfferedDeadlineMissedStatus)e.status);
            break;
            case DDS.OFFERED_INCOMPATIBLE_QOS_STATUS.value:
                    pl.on_offered_incompatible_qos(
                            (DDS.DataWriter) e.observable,
                            (DDS.OfferedIncompatibleQosStatus)e.status);
            break;
            case DDS.LIVELINESS_LOST_STATUS.value:
                    pl.on_liveliness_lost(
                            (DDS.DataWriter) e.observable,
                            (DDS.LivelinessLostStatus)e.status);
            break;
            case DDS.PUBLICATION_MATCHED_STATUS.value:
                    pl.on_publication_matched(
                            (DDS.DataWriter) e.observable,
                            (DDS.PublicationMatchedStatus)e.status);
            break;
            default:
                // TODO [jeroenk]: Should result be updated?
                ReportStack.report(
                    DDS.RETCODE_UNSUPPORTED.value,
                    "Received unsupported event kind '" + e.kind + "'.");
            break;
        }
        return result;
    }

    private native long jniPublisherNew(long uParticipant, String name, DDS.PublisherQos qos);
    private native int jniPublisherFree(long uPublisher);
    private native int jniSetQos(long uPublisher, DDS.PublisherQos qos);
    private native int jniGetQos(long uPublisher, DDS.PublisherQosHolder qos);
    private native int jniSuspendPublications(long uPublisher);
    private native int jniResumePublications(long uPublisher);
    private native int jniBeginCoherentChanges(long uPublisher);
    private native int jniEndCoherentChanges(long uPublisher);
    private native int jniWaitForAcknowledgments(long uPublisher, DDS.Duration_t max_wait);
}
