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

import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Level;

import org.omg.CORBA.LongHolder;
import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.Reliability;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.omg.dds.pub.PublisherQos;

import FACE.CONNECTION_DIRECTION_TYPE;
import FACE.RETURN_CODE_TYPE;
import FACE.VALIDITY_TYPE;

public class SourceConnection<TYPE> extends Connection<TYPE> {
    private Publisher publisher = null;
    private DataWriter<TYPE> dataWriter = null;
    private AtomicLong transactionid;

    public SourceConnection(ConnectionDescription description,
            Class<TYPE> dataType) {
        super(description,dataType);
        this.transactionid = new AtomicLong(0);
        this.setupPublisher();
        this.setupDataWriter();
    }

    private void setupPublisher() {
        PublisherQos qos = this.getDescription().getPublisherQos();
        try {
            if (qos == null) {
                qos = this.getParticipant().getDefaultPublisherQos();
                qos.withPolicy(PolicyFactory
                        .getPolicyFactory(this.getDescription().getEnvironment())
                        .Partition().withName(this.getDescription().getName()));
            } else {
                Set<String> names = qos.getPartition().getName();

                if (names.size() == 1 && names.contains("")) {
                    qos = qos.withPolicy(qos.getPartition().withName(
                            this.getDescription().getName()));
                }
            }
            this.publisher = this.getParticipant().createPublisher(qos);
            this.setLastMessageValidity(VALIDITY_TYPE.VALID);
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
    }

    private void setupDataWriter() {
        try {
        DataWriterQos qos = this.getDescription().getDataWriterQos();

        if (qos == null) {
            qos = this.publisher.getDefaultDataWriterQos();
        }

        this.dataWriter = this.publisher.createDataWriter(this.getTopic());
        this.setLastMessageValidity(VALIDITY_TYPE.VALID);
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
    }

    @Override
    public CONNECTION_DIRECTION_TYPE getDirection() {
        return CONNECTION_DIRECTION_TYPE.SOURCE;
    }


    public RETURN_CODE_TYPE sendMessage(TYPE instanceData, long timeout, LongHolder transaction_id) {
        try {
            if (this.dataWriter.getQos().getReliability().getKind() == Reliability.Kind.RELIABLE) {
                long maxBlockingTime = this.dataWriter.getQos()
                        .getReliability().getMaxBlockingTime()
                        .getDuration(TimeUnit.NANOSECONDS);

                if (timeout > maxBlockingTime) {
                    Logger.getInstance().log(
                            "Supplied time-out > DataWriterQos.reliability.max_blocking_time ("
                                    + timeout + " > " + maxBlockingTime + ").",
                            Level.WARNING);

                    this.setLastMessageValidity(VALIDITY_TYPE.VALID);
                    return RETURN_CODE_TYPE.INVALID_PARAM;
                }
            }
            try {
                this.dataWriter.write(instanceData);
                transaction_id.value = this.transactionid.incrementAndGet();
                this.setLastMessageValidity(VALIDITY_TYPE.VALID);
            } catch (AlreadyClosedException e) {
                this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
            } catch (TimeoutException e) {
                Logger.getInstance().log("Write time-out", Level.WARNING);
                this.setLastMessageValidity(VALIDITY_TYPE.VALID);
                return RETURN_CODE_TYPE.TIMED_OUT;
            }
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
            return RETURN_CODE_TYPE.CONNECTION_CLOSED;

        }
        return RETURN_CODE_TYPE.NO_ERROR;
    }
}
