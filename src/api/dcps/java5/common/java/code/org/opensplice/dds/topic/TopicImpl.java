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
package org.opensplice.dds.topic;

import java.util.Collection;

import org.omg.dds.core.StatusCondition;
import org.omg.dds.core.status.InconsistentTopicStatus;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicDescription;
import org.omg.dds.topic.TopicListener;
import org.omg.dds.topic.TopicQos;
import org.omg.dds.type.TypeSupport;
import org.opensplice.dds.core.DomainEntityImpl;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.IllegalOperationExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.PreconditionNotMetExceptionImpl;
import org.opensplice.dds.core.StatusConditionImpl;
import org.opensplice.dds.core.Utilities;
import org.opensplice.dds.core.status.AllDataDisposedStatus;
import org.opensplice.dds.core.status.StatusConverter;
import org.opensplice.dds.domain.DomainParticipantImpl;
import org.opensplice.dds.type.AbstractTypeSupport;
import org.opensplice.dds.type.TypeSupportImpl;

public class TopicImpl<TYPE>
        extends
        DomainEntityImpl<DDS.Topic, DomainParticipantImpl, DDS.DomainParticipant, TopicQos, TopicListener<TYPE>, TopicListenerImpl<TYPE>>
        implements org.opensplice.dds.topic.AbstractTopic<TYPE> {
    private AbstractTypeSupport<TYPE> typeSupport;

    public TopicImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl participant, String topicName,
            AbstractTypeSupport<TYPE> typeSupport, TopicQos qos,
            TopicListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        super(environment, participant, participant.getOld());

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DataReaderQos is null.");
        }
        if (typeSupport == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied TypeSupport is null.");
        }
        this.typeSupport = typeSupport;

        int rc = this.typeSupport.getOldTypeSupport().register_type(
                parent.getOld(), this.typeSupport.getTypeName());
        Utilities.checkReturnCode(
                rc,
                this.environment,
                "Registration of Type with name '"
                        + this.typeSupport.getTypeName() + "' failed.");
        DDS.TopicQos oldQos;

        try {
            oldQos = ((TopicQosImpl) qos).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Cannot create Topic with non-OpenSplice qos");
        }

        if (listener != null) {
            this.listener = new TopicListenerImpl<TYPE>(this.environment, this,
                    listener, true);
        } else {
            this.listener = null;
        }
        DDS.Topic old = this.parent.getOld().create_topic(topicName,
                this.typeSupport.getTypeName(), oldQos, this.listener,
                StatusConverter.convertMask(this.environment, statuses));

        if (old == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        this.setOld(old);

        if (this.listener != null) {
            this.listener.setInitialised();
        }
    }

    @SuppressWarnings("unchecked")
    public TopicImpl(OsplServiceEnvironment environment,
            DomainParticipantImpl participant, String topicName, DDS.Topic old) {
        super(environment, participant, participant.getOld());
        this.listener = null;

        if (topicName == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied Topic name is null.");
        }
        if (old == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Invalid <null> Topic provided.");
        }
        this.setOld(old);

        try {
            AbstractTypeSupport<?> temp;

            if ("DCPSParticipant".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.ParticipantBuiltinTopicData>(
                        this.environment,
                        DDS.ParticipantBuiltinTopicData.class, null);
            } else if ("DCPSTopic".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.TopicBuiltinTopicData>(
                        this.environment, DDS.TopicBuiltinTopicData.class, null);
            } else if ("CMSubscriber".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.CMSubscriberBuiltinTopicData>(
                        this.environment,
                        DDS.CMSubscriberBuiltinTopicData.class, null);
            } else if ("CMPublisher".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.CMPublisherBuiltinTopicData>(
                        this.environment,
                        DDS.CMPublisherBuiltinTopicData.class, null);
            } else if ("CMParticipant".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.CMParticipantBuiltinTopicData>(
                        this.environment,
                        DDS.CMParticipantBuiltinTopicData.class, null);
            } else if ("DCPSSubscription".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.SubscriptionBuiltinTopicData>(
                        this.environment,
                        DDS.SubscriptionBuiltinTopicData.class, null);
            } else if ("CMDataReader".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.CMDataReaderBuiltinTopicData>(
                        this.environment,
                        DDS.CMDataReaderBuiltinTopicData.class, null);
            } else if ("DCPSPublication".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.PublicationBuiltinTopicData>(
                        this.environment,
                        DDS.PublicationBuiltinTopicData.class, null);
            } else if ("CMDataWriter".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.CMDataWriterBuiltinTopicData>(
                        this.environment,
                        DDS.CMDataWriterBuiltinTopicData.class, null);
            } else if ("DCPSType".equals(topicName)) {
                temp = new TypeSupportImpl<DDS.TypeBuiltinTopicData>(
                        this.environment,
                        DDS.TypeBuiltinTopicData.class, null);
            } else {
                temp = null;
            }
            this.typeSupport = (AbstractTypeSupport<TYPE>) temp;
        } catch (ClassCastException cce) {
            this.typeSupport = null;
        }
    }

    private void setListener(TopicListener<TYPE> listener, int mask) {
        TopicListenerImpl<TYPE> wrapperListener;
        int rc;

        if (listener != null) {
            wrapperListener = new TopicListenerImpl<TYPE>(this.environment,
                    this, listener);
        } else {
            wrapperListener = null;
        }
        rc = this.getOld().set_listener(wrapperListener, mask);
        Utilities.checkReturnCode(rc, this.environment,
                "Topic.setListener() failed.");

        this.listener = wrapperListener;
    }

    @Override
    public void setListener(TopicListener<TYPE> listener) {
        this.setListener(listener, StatusConverter.getAnyMask());
    }

    @Override
    public void setListener(TopicListener<TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public void setListener(TopicListener<TYPE> listener,
            Class<? extends Status>... statuses) {
        this.setListener(listener,
                StatusConverter.convertMask(this.environment, statuses));
    }

    @Override
    public TopicQos getQos() {
        DDS.TopicQosHolder holder = new DDS.TopicQosHolder();
        int rc = this.getOld().get_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Topic.getQos() failed.");

        return TopicQosImpl.convert(this.environment, holder.value);
    }

    @Override
    public void setQos(TopicQos qos) {
        TopicQosImpl q;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied TopicQos is null.");
        }
        try {
            q = (TopicQosImpl) qos;
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Setting non-OpenSplice Qos not supported.");
        }
        int rc = this.getOld().set_qos(q.convert());
        Utilities.checkReturnCode(rc, this.environment,
                "Topic.setQos() failed.");

    }

    @Override
    public InconsistentTopicStatus getInconsistentTopicStatus() {
        DDS.InconsistentTopicStatusHolder holder = new DDS.InconsistentTopicStatusHolder();
        int rc = this.getOld().get_inconsistent_topic_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Topic.getInconsistentTopicStatus()");

        return StatusConverter.convert(this.environment, holder.value);
    }

    @Override
    public StatusCondition<Topic<TYPE>> getStatusCondition() {
        DDS.StatusCondition oldCondition = this.getOld().get_statuscondition();

        if (oldCondition == null) {
            Utilities.throwLastErrorException(this.environment);
        }
        return new StatusConditionImpl<Topic<TYPE>>(this.environment,
                oldCondition, this);
    }

    @Override
    protected void destroy() {
        this.parent.destroyTopic(this);

    }

    @Override
    public TypeSupport<TYPE> getTypeSupport() {
        if (this.typeSupport == null) {
            throw new PreconditionNotMetExceptionImpl(this.environment,
                    "TypeSupport is unknown for this Topic. Has Topic been "
                            + "obtained using DomainParticipant.findTopic() "
                            + "maybe?");
        }
        return this.typeSupport;
    }

    @SuppressWarnings("unchecked")
    @Override
    public <OTHER> TopicDescription<OTHER> cast() {
        TopicDescription<OTHER> other;

        try {
            other = (TopicDescription<OTHER>) this;
        } catch (ClassCastException cce) {
            throw new IllegalOperationExceptionImpl(this.environment,
                    "Unable to perform requested cast.");
        }
        return other;
    }

    @Override
    public String getTypeName() {
        /*
         * Can be null in case Topic has been obtained using the findTopic
         * method
         */
        if (this.typeSupport != null) {
            return this.typeSupport.getTypeName();
        }
        return null;
    }

    @Override
    public String getName() {
        return this.getOld().get_name();
    }

    @Override
    public DomainParticipant getParent() {
        return this.parent;
    }

    @Override
    public void disposeAllData() {
        int rc = this.getOld().dispose_all_data();
        Utilities.checkReturnCode(rc, this.environment,
                "Topic.disposeAllData() failed.");
    }

    @Override
    public AllDataDisposedStatus getAllDataDisposedTopicStatus() {
        DDS.AllDataDisposedTopicStatusHolder holder = new DDS.AllDataDisposedTopicStatusHolder();
        int rc = this.getOld().get_all_data_disposed_topic_status(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Topic.getAllDataDisposedTopicStatus()");

        return StatusConverter.convert(this.environment, holder.value);
    }

}
