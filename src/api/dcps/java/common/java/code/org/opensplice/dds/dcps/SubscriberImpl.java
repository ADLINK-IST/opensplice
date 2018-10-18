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
 * Implementation of the {@link DDS.Subscriber} interface.
 */
public class SubscriberImpl extends SubscriberBase implements DDS.Subscriber {

    private static final long serialVersionUID = 4597853351956995366L;
    private DDS.DataReaderQos defaultDataReaderQos = Utilities.defaultDataReaderQos;
    private DomainParticipantImpl participant = null;
    private final Set<DataReaderImpl> readers = Collections.synchronizedSet(new HashSet<DataReaderImpl>());
    private String name;
    private DDS.SubscriberListener listener = null;
    private boolean factoryAutoEnable = false;

    protected SubscriberImpl() { }

    protected int init (
        DomainParticipantImpl participant,
        String name,
        DDS.SubscriberQos qos)
    {
        int result = DDS.RETCODE_OK.value;

        long uParticipant = participant.get_user_object();
        if (uParticipant != 0) {
            long uSubscriber = jniSubscriberNew(uParticipant, name, qos);
            if (uSubscriber != 0) {
                this.set_user_object(uSubscriber);
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
        long uSubscriber;

        synchronized (this)
        {
            uSubscriber = this.get_user_object();
            if (uSubscriber != 0) {
                if (this.readers.size() == 0) {
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
                                result = jniSubscriberFree(uSubscriber);
                                if (result == DDS.RETCODE_OK.value) {
                                    result = super.deinit();
                                }
                            }
                        }
                    }
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                        "Subscriber still contains '" + this.readers.size() +
                        "' DataReader entities.");
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public DDS.DataReader create_datareader (
        DDS.TopicDescription description,
        DDS.DataReaderQos qos,
        DDS.DataReaderListener a_listener,
        int mask)
    {
        DataReaderImpl reader = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (description == null || qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,"Description or Qos is null");
        } else if (qos == DDS.DATAREADER_QOS_DEFAULT.value) {
            qos = this.defaultDataReaderQos;
        } else if (qos == DDS.DATAREADER_QOS_USE_TOPIC_QOS.value) {
            DDS.DataReaderQosHolder holder = new DDS.DataReaderQosHolder();
            this.copy_from_topicdescription_qos(holder, description);
            qos = holder.value;

            result = Utilities.checkQos(qos);
        } else {
            result = Utilities.checkQos(qos);
        }


        if (result == DDS.RETCODE_OK.value) {
            synchronized(this) {
                long uSubscriber = this.get_user_object();

                if (uSubscriber != 0) {
                    if (this.participant.equals(description.get_participant())) {
                        String name = "reader <" + description.get_name() + ">";
                        reader = (DataReaderImpl) ((org.opensplice.dds.dcps.TopicDescription) description)
                                .create_datareader();
                        if (reader != null) {
                            result = reader.init(this, name, description, qos);
                            if (result == DDS.RETCODE_OK.value) {
                                this.readers.add(reader);
                                ListenerDispatcher dispatcher = this
                                        .get_dispatcher();
                                result = reader.set_dispatcher(dispatcher);
                            } else {
                                reader = null;
                                ReportStack.report(result,"DataReader could not be initialized.");
                            }
                            if (result == DDS.RETCODE_OK.value) {
                                result = reader.set_listener(a_listener, mask);
                            }
                            if (result == DDS.RETCODE_OK.value) {
                                if (this.factoryAutoEnable && this.is_enabled()) {
                                    result = reader.enable();
                                    if (result != DDS.RETCODE_OK.value) {
                                        ReportStack.report(result,"DataReader could not be enabled.");
                                    }
                                }
                            }
                            if (result != DDS.RETCODE_OK.value
                                    && reader != null) {
                                this.delete_datareader(reader);
                                reader = null;
                            }
                        } else {
                            result = DDS.RETCODE_ERROR.value;
                            ReportStack.report(result,"DataReader could not be created.");
                        }
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack
                                .report(result,
                                        "Topic does not belong to the same DomainParticipant as this Subscriber.");
                    }
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return reader;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int delete_datareader (
        DDS.DataReader reader)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();
        if (reader == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_datareader 'null' is invalid.");
        } else {
            DataReaderImpl dr = (DataReaderImpl)reader;
            boolean removed = false;
            synchronized (this) {
                removed = this.readers.remove(dr);

                if (removed) {
                    result = dr.deinit();
                    if (result == DDS.RETCODE_PRECONDITION_NOT_MET.value) {
                        this.readers.add(dr);
                    }
                } else {
                    if (dr.get_user_object() == 0) {
                        result = DDS.RETCODE_ALREADY_DELETED.value;
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack.report(result,
                            "DataReader not created by Subscriber.");
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    public boolean contains_entity (
        long a_handle)
    {
        boolean found = false;
        long handle;
        ReportStack.start();

        synchronized (this.readers)
        {
            Iterator<DataReaderImpl> it = this.readers.iterator();
            while (it.hasNext() && !found) {
                handle = it.next().get_instance_handle();
                found = (handle == a_handle);
            }
        }

        ReportStack.flush(this, found == false);
        return found;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int delete_contained_entities ()
    {
        int result, endResult = DDS.RETCODE_OK.value;
        ReportStack.start();

        HashSet<DataReaderImpl> survivors = new HashSet<DataReaderImpl>();
        DataReaderImpl dr = null;
        do {
            dr = null;
            synchronized (this.readers) {
                Iterator<DataReaderImpl> dri = this.readers.iterator();
                while (dri.hasNext()) {
                    dr = dri.next();
                    dri.remove();
                    if (!survivors.contains(dr)) {
                        break;
                    } else {
                        dr = null;
                    }
                }
            }
            if (dr != null) {
                result = dr.delete_contained_entities();
                if (result == DDS.RETCODE_OK.value) {
                    result = dr.deinit();
                    if (result != DDS.RETCODE_OK.value && result != DDS.RETCODE_ALREADY_DELETED.value) {
                        survivors.add(dr);
                        synchronized (this.readers) {
                            this.readers.add(dr);
                        }
                    }
                    if (result != DDS.RETCODE_OK.value) {
                        ReportStack.report(result, "Deletion of DataReader contained in Subscriber failed.");
                    }
                } else {
                    ReportStack.report(result,"delete_contained_entities failed on DataReader contained in Subscriber.");
                    if (result != DDS.RETCODE_ALREADY_DELETED.value) {
                        survivors.add(dr);
                        synchronized (this.readers) {
                            this.readers.add(dr);
                        }
                    }
                }
                if (endResult == DDS.RETCODE_OK.value) {
                    /* Store first encountered error. */
                    endResult = result;
                }
            }
        } while (dr != null);

        ReportStack.flush(this, endResult != DDS.RETCODE_OK.value);
        return endResult;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public DDS.DataReader lookup_datareader (
        String topic_name)
    {
        DataReaderImpl found = null;
        synchronized (this.readers)
        {
            for(DataReaderImpl reader : readers){
                if (reader.get_topicdescription().get_name().equals(topic_name)) {
                    found = reader;
                    break;
                }
            }
        }
        return found;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int get_datareaders (
        DDS.DataReaderSeqHolder readers,
        int sample_states,
        int view_states,
        int instance_states)
    {
        int result = DDS.RETCODE_OK.value;

        ReportStack.start();
        if (readers == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "readers 'null' is invalid.");
        } else {
            synchronized (this)
            {
                long uSubscriber = this.get_user_object();
                if (uSubscriber != 0) {
                    result = jniGetDataReaders(uSubscriber, readers,
                            sample_states, view_states, instance_states);
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);

        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int notify_datareaders ()
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (this.get_user_object() != 0) {
            HashSet<DDS.DataReader> processed = new HashSet<DDS.DataReader>();
            DDS.DataReader reader = null;
            do {
                reader = null;
                synchronized (this.readers) {
                    for (DataReaderImpl r : this.readers ) {
                        if (!processed.contains(r)) {
                            reader = r;
                            break;
                        }
                    }
                }
                if (reader != null) {
                    if ((reader.get_status_changes() & DDS.DATA_AVAILABLE_STATUS.value) != 0) {
                        DDS.DataReaderListener readerListener = reader.get_listener();
                        if (readerListener != null) {
                            readerListener.on_data_available(reader);
                        }
                    }
                    processed.add(reader);
                }
            } while (reader != null);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int set_qos (
        DDS.SubscriberQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.SUBSCRIBER_QOS_DEFAULT.value) {
            DDS.SubscriberQosHolder holder = new DDS.SubscriberQosHolder();
            this.participant.get_default_subscriber_qos(holder);
            qos = holder.value;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uSubscriber = this.get_user_object();
            if (uSubscriber != 0) {
                result = jniSetQos(uSubscriber, qos);
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

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int get_qos (
        DDS.SubscriberQosHolder qosHolder)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qosHolder == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            long uSubscriber = this.get_user_object();
            if (uSubscriber != 0) {
                result = jniGetQos(uSubscriber, qosHolder);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int set_listener (
        DDS.SubscriberListener a_listener, int mask)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            if (this.get_user_object() != 0) {
                this.listener = a_listener;
                result = this.set_listener_interest(mask);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public DDS.SubscriberListener get_listener ()
    {
        return listener;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int begin_access ()
    {
        long uSubscriber = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uSubscriber = this.get_user_object();
        if (uSubscriber != 0) {
            result = jniBeginAccess(uSubscriber);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int end_access ()
    {
        long uSubscriber = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uSubscriber = this.get_user_object();
        if (uSubscriber != 0) {
            result = jniEndAccess(uSubscriber);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public DDS.DomainParticipant get_participant ()
    {
        DDS.DomainParticipant participant = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (this.get_user_object() != 0) {
            participant = this.participant;
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return participant;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int set_default_datareader_qos (
        DDS.DataReaderQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAREADER_QOS_USE_TOPIC_QOS.value) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'DATAREADER_QOS_USE_TOPIC_QOS' is invalid in this context.");
        } else if (qos == DDS.DATAREADER_QOS_DEFAULT.value) {
            qos = Utilities.defaultDataReaderQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            if (this.get_user_object() != 0) {
                this.defaultDataReaderQos = Utilities.deepCopy(qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int get_default_datareader_qos (
        DDS.DataReaderQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (this.get_user_object() == 0) {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        } else {
            qos.value = Utilities.deepCopy(this.defaultDataReaderQos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.SubscriberOperations for javadoc */
    @Override
    public int copy_from_topic_qos (
        DDS.DataReaderQosHolder qosHolder,
        DDS.TopicQos topic_qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qosHolder == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_datareader_qos 'null' is invalid.");
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
            DDS.DataReaderQos qos = qosHolder.value;
            if (qos == null) {
                qos = Utilities.deepCopy(this.defaultDataReaderQos);
            }
            qos.durability = Utilities.deepCopy(topic_qos.durability);
            qos.deadline = Utilities.deepCopy(topic_qos.deadline);
            qos.latency_budget = Utilities.deepCopy(topic_qos.latency_budget);
            qos.liveliness = Utilities.deepCopy(topic_qos.liveliness);
            qos.reliability = Utilities.deepCopy(topic_qos.reliability);
            qos.destination_order = Utilities.deepCopy(topic_qos.destination_order);
            qos.history = Utilities.deepCopy(topic_qos.history);
            qos.resource_limits = Utilities.deepCopy(topic_qos.resource_limits);
            qos.ownership = Utilities.deepCopy(topic_qos.ownership);
            qosHolder.value = qos;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected int copy_from_topicdescription_qos (
        DDS.DataReaderQosHolder qosHolder,
        DDS.TopicDescription description)
    {
        int result = DDS.RETCODE_OK.value;
        TopicImpl topic = null;

        if (description instanceof TopicImpl) {
            topic = (TopicImpl)description;
        } else if (description instanceof ContentFilteredTopicImpl) {
            topic = (TopicImpl)((ContentFilteredTopicImpl)description).get_related_topic();
        } else if (description instanceof MultiTopicImpl) {
            result = DDS.RETCODE_UNSUPPORTED.value;
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        if (result == DDS.RETCODE_OK.value) {
            DDS.TopicQosHolder holder = new DDS.TopicQosHolder();
            result = topic.get_qos(holder);
            if (result == DDS.RETCODE_OK.value) {
                result = this.copy_from_topic_qos(qosHolder, holder.value);
            }
        }

        return result;
    }

    @Override
    protected int notify(Event e)
    {
        int result = DDS.RETCODE_OK.value;

        DDS.SubscriberListener sl = this.listener;
        if (sl == null)
            return result;

        switch(e.kind) {
        case DDS.DATA_ON_READERS_STATUS.value:
                sl.on_data_on_readers((DDS.Subscriber) e.observer);
        break;
        case DDS.DATA_AVAILABLE_STATUS.value:
                sl.on_data_available((DDS.DataReader) e.observable);
        break;
        case DDS.REQUESTED_DEADLINE_MISSED_STATUS.value:
                sl.on_requested_deadline_missed(
                          (DDS.DataReader)e.observable,
                          (DDS.RequestedDeadlineMissedStatus)e.status);
        break;
        case DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS.value:
                sl.on_requested_incompatible_qos(
                          (DDS.DataReader)e.observable,
                          (DDS.RequestedIncompatibleQosStatus)e.status);
        break;
        case DDS.SAMPLE_REJECTED_STATUS.value:
                sl.on_sample_rejected(
                          (DDS.DataReader)e.observable,
                          (DDS.SampleRejectedStatus)e.status);
        break;
        case DDS.LIVELINESS_CHANGED_STATUS.value:
                sl.on_liveliness_changed(
                          (DDS.DataReader)e.observable,
                          (DDS.LivelinessChangedStatus)e.status);
        break;
        case DDS.SUBSCRIPTION_MATCHED_STATUS.value:
                sl.on_subscription_matched(
                          (DDS.DataReader)e.observable,
                          (DDS.SubscriptionMatchedStatus)e.status);
        break;
        case DDS.SAMPLE_LOST_STATUS.value:
                sl.on_sample_lost(
                          (DDS.DataReader)e.observable,
                          (DDS.SampleLostStatus)e.status);
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

    private native long jniSubscriberNew(long uParticipant, String name, DDS.SubscriberQos qos);
    private native int jniSubscriberFree(long uSubscriber);
    private native int jniSetQos(long uSubscriber, DDS.SubscriberQos qos);
    private native int jniGetQos(long uSubscriber, DDS.SubscriberQosHolder qos);
    private native int jniBeginAccess(long uSubscriber);
    private native int jniEndAccess(long uSubscriber);
    private native int jniGetDataReaders(long uSubscriber, DDS.DataReaderSeqHolder readers,
            int sample_states, int view_states, int instance_states);
}
