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
 */
package org.vortex.FACE;

import java.util.ArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.domain.DomainParticipantQos;
import org.omg.dds.topic.Topic;
import org.omg.dds.topic.TopicQos;

import FACE.CONNECTION_DIRECTION_TYPE;
import FACE.TRANSPORT_CONNECTION_STATUS_TYPE;
import FACE.VALIDITY_TYPE;

public abstract class Connection<TYPE> {
    private final ConnectionDescription description;
    private final Class<TYPE> dataType;
    private DomainParticipant participant;
    private Topic<TYPE> topic;
    private TRANSPORT_CONNECTION_STATUS_TYPE status;
    private final ReadWriteLock readWriteLock = new ReentrantReadWriteLock();
    private final Lock readLock = readWriteLock.readLock();
    private final Lock writeLock = readWriteLock.writeLock();

    public Connection(ConnectionDescription description, Class<TYPE> dataType) {
        this.description = description;
        this.dataType = dataType;
        this.setupParticipant();
        this.setupTopic();
        this.status = new TRANSPORT_CONNECTION_STATUS_TYPE(0, 1, 0,
                this.getDirection(), 0, this.description.getRefreshPeriod(),
                VALIDITY_TYPE.INVALID);
    }

    public TRANSPORT_CONNECTION_STATUS_TYPE getStatus() {
        return new TRANSPORT_CONNECTION_STATUS_TYPE(this.status.MAX_MESSAGE,
                this.status.MAX_MESSAGE_SIZE, this.status.MESSAGE,
                this.status.CONNECTION_DIRECTION,
                this.status.WAITING_PROCESSES_OR_MESSAGES,
                this.status.REFRESH_PERIOD, this.status.LAST_MSG_VALIDITY);
    }

    protected void setLastMessageValidity(VALIDITY_TYPE lastMessageValidity) {
        synchronized(this) {
            this.status.LAST_MSG_VALIDITY = lastMessageValidity;
        }
    }

    public abstract CONNECTION_DIRECTION_TYPE getDirection();

    public ConnectionDescription getDescription() {
        return this.description;
    }

    public DomainParticipant getParticipant() {
        return this.participant;
    }

    public Topic<TYPE> getTopic() {
        return this.topic;
    }

    private void setupParticipant() {
        DomainParticipantFactory df = this.description.getEnvironment()
                .getSPI().getParticipantFactory();

        DomainParticipantQos qos = this.description.getDomainParticipantQos();

        if (qos == null) {
            qos = df.getDefaultParticipantQos();
        }
        this.participant = df.createParticipant(this.description.getDomainId(),
                qos, null, new ArrayList<Class<? extends Status>>());
    }

    private void setupTopic() {
        TopicQos qos = this.description.getTopicQos();

        if (qos == null) {
            qos = this.participant.getDefaultTopicQos();
        }
        this.topic = this.participant.createTopic(
                this.description.getTopicName(), this.dataType, qos, null,
                new ArrayList<Class<? extends Status>>());
    }

    public static <TYPE> Connection<TYPE> getConnection(
            ConnectionDescription description, Class<TYPE> dataType) {

        switch (description.getDirection().value()) {
        case FACE.CONNECTION_DIRECTION_TYPE._SOURCE:
            return new SourceConnection<TYPE>(description, dataType);
        case FACE.CONNECTION_DIRECTION_TYPE._DESTINATION:
            return new DestinationConnection<TYPE>(description, dataType);
        default:
            return null;
        }
    }

    public void close() {
        try {
            this.participant.close();
        } catch(AlreadyClosedException e) {
            // ignore
        }
        this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
    }

    @SuppressWarnings("unchecked")
    public <OTHER> Connection<OTHER> cast() {
        Connection<OTHER> other;

        try {
            other = (Connection<OTHER>) this;
        } catch (ClassCastException c) {
            other = null;
        }
        return other;
    }

    public DestinationConnection<TYPE> asDestination() {
        if (this.getDirection() != CONNECTION_DIRECTION_TYPE.DESTINATION) {
            return null;
        }
        return (DestinationConnection<TYPE>) this;
    }

    public SourceConnection<TYPE> asSource() {
        if (this.getDirection() != CONNECTION_DIRECTION_TYPE.SOURCE) {
            return null;
        }
        return (SourceConnection<TYPE>) this;
    }

    protected void readLock() {
        this.readLock.lock();
    }

    protected void readUnlock() {
        this.readLock.unlock();
    }

    protected void writeLock() {
        this.writeLock.lock();
    }

    protected void writeUnlock() {
        this.writeLock.unlock();
    }
}
