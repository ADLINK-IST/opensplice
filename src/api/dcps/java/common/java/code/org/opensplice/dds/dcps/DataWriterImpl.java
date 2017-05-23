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


package org.opensplice.dds.dcps;

/**
 * Implementation of the {@link DDS.DataWriter} interface.
 */
public class DataWriterImpl extends DataWriterBase implements DDS.DataWriter {

    private static final long serialVersionUID = 278508615771428238L;
    private String name;
    private PublisherImpl publisher;
    private TopicImpl topic;
    private DDS.DataWriterListener listener = null;

    protected DataWriterImpl() { }

    protected int init (
        PublisherImpl publisher,
        String name,
        TopicImpl topic,
        DDS.DataWriterQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        long uPublisher = publisher.get_user_object();
        long uTopic = topic.get_user_object();

        if (uPublisher != 0 && uTopic != 0) {
            /* QoS is guaranteed to be valid by the creating publisher.
               DATAWRITER_QOS_DEFAULT and DATAWRITER_QOS_USE_TOPIC_QOS are
               properly resolved. */
            long uWriter = jniDataWriterNew(uPublisher, name, uTopic, qos);
            if (uWriter != 0) {
                topic.keep();
                this.set_user_object(uWriter);
                this.setDomainId(publisher.getDomainId());
                this.publisher = publisher;
                this.name = name;
                this.topic = topic;
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        int result;
        long uWriter;

        synchronized (this)
        {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                if (this.listener != null) {
                    /* Should always be done before disable_callbacks(), so
                     * that events that can trigger a listener on an owning
                     * entity are propagated instead of being consumed by
                     * the listener being destroyed. */
                    set_listener(this.listener, 0);
                }
                this.disable_callbacks();
                result = ((EntityImpl) this).detach_statuscondition();
                if (result == DDS.RETCODE_OK.value) {
                    this.topic.free();
                    this.name = null;
                    this.topic = null;
                    this.publisher = null;
                    result = jniDataWriterFree(uWriter);
                    if (result == DDS.RETCODE_OK.value) {
                        result = super.deinit();
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int set_qos (DDS.DataWriterQos qos)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAWRITER_QOS_DEFAULT.value) {
            DDS.DataWriterQosHolder holder = new DDS.DataWriterQosHolder();
            result = this.publisher.get_default_datawriter_qos(holder);
            qos = holder.value;
        } else if (qos == DDS.DATAWRITER_QOS_USE_TOPIC_QOS.value) {
            DDS.DataWriterQosHolder holder = new DDS.DataWriterQosHolder();
            result = this.publisher.copy_from_topic_qos(holder, DDS.TOPIC_QOS_DEFAULT.value);
            if (result == DDS.RETCODE_OK.value) {
                qos = holder.value;
                result = Utilities.checkQos(qos);
            }
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniSetQos(uWriter, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_qos (DDS.DataWriterQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            long uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetQos(uWriter, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int set_listener (DDS.DataWriterListener a_listener, int mask)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                this.listener = a_listener;
                result = this.set_listener_interest(mask);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public DDS.DataWriterListener get_listener () {
        return listener;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public DDS.Topic get_topic () {
        return this.topic;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public DDS.Publisher get_publisher () {
        return this.publisher;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int wait_for_acknowledgments(DDS.Duration_t max_wait)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            result = Utilities.checkDuration(max_wait);
            if (result == DDS.RETCODE_OK.value) {
                result = jniWaitForAcknowledgments(uWriter, max_wait);
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value &&
                                result != DDS.RETCODE_TIMEOUT.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_liveliness_lost_status(DDS.LivelinessLostStatusHolder status)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetLivelinessLostStatus(uWriter, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_offered_deadline_missed_status(DDS.OfferedDeadlineMissedStatusHolder status)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetOfferedDeadlineMissedStatus(uWriter, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_offered_incompatible_qos_status(DDS.OfferedIncompatibleQosStatusHolder status)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetOfferedIncompatibleQosStatus(uWriter, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_publication_matched_status(DDS.PublicationMatchedStatusHolder status)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetPublicationMatchedStatus(uWriter, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int assert_liveliness ()
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            result = jniAssertLiveliness(uWriter);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_matched_subscriptions (
        DDS.InstanceHandleSeqHolder subscription_handles)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (subscription_handles == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "subscription_handles 'null' is invalid.");
        } else {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetMatchedSubscriptions(uWriter, subscription_handles);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value &&
                                result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    /* see DDS.DataWriterOperations for javadoc */
    @Override
    public int get_matched_subscription_data (
        DDS.SubscriptionBuiltinTopicDataHolder subscription_data,
        long subscription_handle)
    {
        long uWriter = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (subscription_data == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "subscription_data 'null' is invalid.");
        } else if (subscription_handle == DDS.HANDLE_NIL.value) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "subscription_handle 'HANDLE_NIL' is invalid.");
        } else {
            uWriter = this.get_user_object();
            if (uWriter != 0) {
                result = jniGetMatchedSubscriptionData(uWriter,
                                                       subscription_data,
                                                       subscription_handle);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value &&
                                result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    protected int notify(Event e)
    {
        int result = DDS.RETCODE_OK.value;
        DDS.DataWriterListener dwl = this.listener;

        if (dwl == null)
            return result;

        switch(e.kind) {
        case DDS.OFFERED_DEADLINE_MISSED_STATUS.value:
                dwl.on_offered_deadline_missed(
                          this,
                          (DDS.OfferedDeadlineMissedStatus)e.status);
        break;
        case DDS.OFFERED_INCOMPATIBLE_QOS_STATUS.value:
                dwl.on_offered_incompatible_qos(
                          this,
                          (DDS.OfferedIncompatibleQosStatus)e.status);
        break;
        case DDS.LIVELINESS_LOST_STATUS.value:
                dwl.on_liveliness_lost(
                          this,
                          (DDS.LivelinessLostStatus)e.status);
        break;
        case DDS.PUBLICATION_MATCHED_STATUS.value:
                dwl.on_publication_matched(
                          this,
                          (DDS.PublicationMatchedStatus)e.status);
        break;
        default:
            // TODO [jeroenk]: Should result be updated?
            ReportStack.report(
                DDS.RETCODE_UNSUPPORTED.value,
                "Received unsupported event kind '" + e.kind + "'");
        break;
        }
        return result;
    }

    private native long jniDataWriterNew(long uPublisher, String name, long uTopic, DDS.DataWriterQos qos);
    private native int jniDataWriterFree(long uWriter);

    private native int jniSetQos(long uWriter, DDS.DataWriterQos qos);
    private native int jniGetQos(long uWriter, DDS.DataWriterQosHolder qos);
    private native int jniWaitForAcknowledgments(long uWriter, DDS.Duration_t max_wait);
    private native int jniGetLivelinessLostStatus(long uWriter, DDS.LivelinessLostStatusHolder status);
    private native int jniGetOfferedDeadlineMissedStatus(long uWriter, DDS.OfferedDeadlineMissedStatusHolder status);
    private native int jniGetOfferedIncompatibleQosStatus(long uWriter, DDS.OfferedIncompatibleQosStatusHolder status);
    private native int jniGetPublicationMatchedStatus(long uWriter, DDS.PublicationMatchedStatusHolder status);
    private native int jniAssertLiveliness(long uWriter);
    private native int jniGetMatchedSubscriptions(long uWriter, DDS.InstanceHandleSeqHolder subscription_handles);
    private native int jniGetMatchedSubscriptionData(long uWriter, DDS.SubscriptionBuiltinTopicDataHolder subscription_data, long subscription_handle);
}
