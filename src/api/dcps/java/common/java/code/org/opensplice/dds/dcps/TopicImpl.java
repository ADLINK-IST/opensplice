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

import java.util.Map;
import java.util.Map.Entry;

import DDS.ExtTopicListener;

/**
 * Implementation of the {@link DDS.Topic} interface.
 */
public class TopicImpl extends TopicBase implements DDS.Topic, TopicDescription {

    private static final long serialVersionUID = -4295358388686624149L;
    private String name = null;
    private DomainParticipantImpl participant = null;
    private TypeSupportImpl typeSupport = null;
    private DDS.TopicListener listener = null;
    private int refCount = 0; /* number of active topic users. */
    private String type_name = null;
    private int topicListenerInterest = 0;
    private int participantListenerInterest = 0;

    protected int init (
        DomainParticipantImpl participant,
        String topic_name,
        String type_name,
        DDS.TopicQos a_qos)
    {
        int result = DDS.RETCODE_OK.value;

        /* Parameters validated by DDS.DomainParticipant.create_topic. */

        TypeSupportImpl type_support = participant.lookup_typeSupport(type_name);
        if (type_support != null) {
            long uParticipant = participant.get_user_object();
            if (uParticipant != 0) {
                String key_list = type_support.get_key_list();
                String actual_type_name = type_support.get_internal_type_name();
                long uTopic = jniTopicNew(uParticipant, topic_name, actual_type_name, key_list, a_qos);
                if (uTopic != 0) {
                    this.name = topic_name;
                    this.type_name = type_name;
                    this.typeSupport = type_support;
                    this.participant = participant;
                    this.set_user_object(uTopic);
                    this.setDomainId(participant.getDomainId());
                } else {
                    result = DDS.RETCODE_ERROR.value;
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;
        long uTopic = 0;

        synchronized (this)
        {
            uTopic = this.get_user_object();
            if (uTopic != 0) {
                if (this.refCount == 0) {
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
                                this.typeSupport = null;
                                this.name = null;
                                this.type_name = null;
                                this.participant = null;
                                result = jniTopicFree(uTopic);
                                if (result == DDS.RETCODE_OK.value) {
                                    result = super.deinit();
                                }
                            }
                        }
                    }
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack
                            .report(result, "Topic '" + this.name
                                    + "' still referenced " + this.refCount
                                    + " times.");
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    @Override
    public int keep()
    {
        int result = DDS.RETCODE_OK.value;
        long uTopic;
        ReportStack.start();

        synchronized (this)
        {
            uTopic = this.get_user_object();
            if (uTopic != 0) {
                this.refCount++;
                result = DDS.RETCODE_OK.value;
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int free()
    {
        int result = DDS.RETCODE_OK.value;
        long uTopic;
        ReportStack.start();

        synchronized (this)
        {
            uTopic = this.get_user_object();
            if (uTopic != 0) {
                this.refCount--;
                result = DDS.RETCODE_OK.value;
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected int clone (
        long uTopic,
        DomainParticipantImpl dp,
        TypeSupportImpl type_support)
    {
        int result = DDS.RETCODE_OK.value;

        assert(uTopic != 0);
        assert(dp != null);

        synchronized (this)
        {
            this.set_user_object(uTopic);
            this.name = jniGetName(uTopic);
            this.participant = dp;
            this.setDomainId(dp.getDomainId());
            this.typeSupport = null;
            if (type_support != null) {
                this.typeSupport = type_support;
            } else {
                String typename = jniGetTypeName(uTopic);

                this.typeSupport = this.participant.lookup_typeSupport(typename);
                /*
                 * type is not registered with idl type name so do a lookup
                 * over all registered types to find a matching alias for
                 * the idl type name
                 */
                if (this.typeSupport == null) {
                    Map<String, TypeSupportImpl> ts = this.participant.get_typesupports();

                    for (Entry<String, TypeSupportImpl> tsp : ts.entrySet()) {
                        if (tsp.getValue().get_type_name().equals(typename)) {
                            this.typeSupport = tsp.getValue();
                            break;
                        }
                    }
                }

                if (this.typeSupport != null) {

                    // check if typesupport keys are compatible with topic type keys
                    String typeKeyList = this.typeSupport.get_key_list();
                    String topicKeyList = jniGetKeyExpr(uTopic);

                    if (typeKeyList == null || topicKeyList == null) {
                        if (typeKeyList == null && topicKeyList != null ) {
                            ReportStack.report(DDS.RETCODE_OK.value,
                                "incompatible keys: registered typesupport has no key but topic has key '" + topicKeyList + "'.");
                        } else if (typeKeyList != null && topicKeyList == null ) {
                            ReportStack.report(DDS.RETCODE_OK.value,
                                "incompatible keys: registered typesupport has key '" + typeKeyList + "' but topic has no key.");
                        }
                    } else {
                        String[] typeKeyArr = typeKeyList.split(",\\s");
                        String[] topicKeyArr = topicKeyList.split(",\\s");

                        boolean consistent = typeKeyArr.length == topicKeyArr.length;
                        if (consistent) {
                            for (int i = 0; consistent && i < typeKeyArr.length; i++) {
                                consistent = typeKeyArr[i].equals(topicKeyArr[i]);
                            }
                        }
                        if (!consistent) {
                            ReportStack.report(DDS.RETCODE_OK.value,
                                   "incompatible keys: registered typesupport has key '" + typeKeyList +
                                   "' but topic has key '" + topicKeyList + "'.");
                        }
                    }
                }
            }
        }

        return result;
    }

    protected int set (
        TypeSupportImpl type_support)
    {
        int result;
        if (type_support != null) {
            this.typeSupport = type_support;
            result = DDS.RETCODE_OK.value;
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }
        return result;
    }

    protected TypeSupportImpl get_typesupport()
    {
        return this.typeSupport;
    }

    @Override
    public String get_type_name ()
    {
        if (this.type_name == null) {
            if (this.typeSupport != null) {
               this.type_name = this.typeSupport.get_type_name();
            }
        }
        return type_name;
    }

    @Override
    public String get_name ()
    {
        return name;
    }

    /* see DDS.TopicOperations for javadoc */
    @Override
    public int get_inconsistent_topic_status (
        DDS.InconsistentTopicStatusHolder status)
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uTopic = this.get_user_object();
        if (uTopic != 0) {
            result = jniGetInconsistentTopicStatus(uTopic, status);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.TopicOperations for javadoc */
    @Override
    public int get_all_data_disposed_topic_status (
        DDS.AllDataDisposedTopicStatusHolder status)
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uTopic = this.get_user_object();
        if (uTopic != 0) {
            result = jniGetAllDataDisposedTopicStatus(uTopic, status);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.TopicOperations for javadoc */
    @Override
    public int get_qos (DDS.TopicQosHolder qos)
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            uTopic = this.get_user_object();
            if (uTopic != 0) {
                result = jniGetQos(uTopic, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.TopicOperations for javadoc */
    @Override
    public int set_qos (
        DDS.TopicQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.TOPIC_QOS_DEFAULT.value) {
            DDS.TopicQosHolder holder = new DDS.TopicQosHolder();
            this.participant.get_default_topic_qos(holder);
            qos = holder.value;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uTopic = this.get_user_object();
            if (uTopic != 0) {
                result = jniSetQos(uTopic, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.TopicOperations for javadoc */
    @Override
    public DDS.TopicListener get_listener ()
    {
        return this.listener;
    }

    /* see DDS.TopicOperations for javadoc */
    @Override
    public int set_listener (
        DDS.TopicListener a_listener, int mask)
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            uTopic = this.get_user_object();
            if (uTopic != 0) {
                this.listener = a_listener;
                this.topicListenerInterest = mask;
                result = this.set_listener_interest(mask | this.participantListenerInterest);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected int set_participant_listener_mask (
        int mask)
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            uTopic = this.get_user_object();
            if (uTopic != 0) {
                this.participantListenerInterest = mask;
                result = this.set_listener_interest(mask | this.topicListenerInterest);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.TopicDescriptionOperations for javadoc */
    @Override
    public DDS.DomainParticipant get_participant ()
    {
        return this.participant;
    }

    @Override
    public int dispose_all_data ()
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uTopic = this.get_user_object();
        if (uTopic != 0) {
            result = jniDisposeAllData(uTopic);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected DDS.DataWriter create_datawriter ()
    {
        DDS.DataWriter writer = null;

        if (this.get_user_object() != 0 && this.typeSupport != null) {
            writer = this.typeSupport.create_datawriter();
        }

        return writer;
    }

    @Override
    public DDS.DataReader create_datareader ()
    {
        DDS.DataReader reader = null;

        if (this.get_user_object() != 0 && this.typeSupport != null) {
            reader = this.typeSupport.create_datareader();
        }

        return reader;
    }

    @Override
    public DDS.DataReaderView create_dataview ()
    {
        DDS.DataReaderView view = null;

        if (this.get_user_object() != 0 && this.typeSupport != null) {
            view = this.typeSupport.create_dataview();
        }

        return view;
    }

    @Override
    protected int notify(Event e)
    {
        int result = DDS.RETCODE_OK.value;
        DDS.TopicListener tl = this.listener;

        if (e.kind == Event.INCONSISTENT_TOPIC) {
            if ((tl != null) &&
                ((this.topicListenerInterest & DDS.INCONSISTENT_TOPIC_STATUS.value) != 0))
            {
                DDS.InconsistentTopicStatus status = (DDS.InconsistentTopicStatus)e.status;
                tl.on_inconsistent_topic(this, status);
            } else if ((this.participantListenerInterest & DDS.INCONSISTENT_TOPIC_STATUS.value) != 0) {
                this.participant.notify(e);
            }
        }
        if (e.kind == Event.ALL_DATA_DISPOSED) {
            if ((tl != null) &&
                ((this.topicListenerInterest & DDS.ALL_DATA_DISPOSED_TOPIC_STATUS.value) != 0))
            {
                DDS.ExtTopicListener etl = (ExtTopicListener) this.listener;
                etl.on_all_data_disposed(this);
            } else if ((this.participantListenerInterest & DDS.ALL_DATA_DISPOSED_TOPIC_STATUS.value) != 0) {
                this.participant.notify(e);
            }
        }

        return result;
    }

    protected int validate_filter(
        String _expression,
        String[] _parameters)
    {
        long uTopic = 0;
        int result = DDS.RETCODE_OK.value;

        uTopic = this.get_user_object();
        if (uTopic != 0) {
            result = jniValidateFilter(uTopic, _expression, _parameters);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        return result;
    }

    private native long jniTopicNew(long uParticipant, String topic_name, String type_name, String key_list, DDS.TopicQos qos);
    private native int jniTopicFree(long uTopic);
    private native int jniGetInconsistentTopicStatus(long uTopic, DDS.InconsistentTopicStatusHolder status);
    private native int jniGetAllDataDisposedTopicStatus(long uTopic, DDS.AllDataDisposedTopicStatusHolder status);
    private native int jniGetQos(long uTopic, DDS.TopicQosHolder qos);
    private native int jniSetQos(long uTopic, DDS.TopicQos qos);
    private native String jniGetName(long uTopic);
    private native String jniGetKeyExpr(long uTopic);

    private native String jniGetTypeName(long uTopic);
    private native int jniDisposeAllData(long uTopic);
    private native int jniValidateFilter(long uTopic, String expression, String[] parameters);
}
