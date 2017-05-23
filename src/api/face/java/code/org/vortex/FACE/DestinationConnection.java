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
 */
package org.vortex.FACE;

import java.io.IOException;
import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Level;

import org.omg.CORBA.LongHolder;
import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.WaitSet;
import org.omg.dds.core.event.DataAvailableEvent;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReader.Selector;
import org.omg.dds.sub.DataReaderAdapter;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.InstanceState;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.omg.dds.sub.Subscriber;
import org.omg.dds.sub.SubscriberQos;
import FACE.CONNECTION_DIRECTION_TYPE;
import FACE.RETURN_CODE_TYPE;
import FACE.RETURN_CODE_TYPEHolder;
import FACE.VALIDITY_TYPE;

public class DestinationConnection<TYPE> extends Connection<TYPE> {
    private Subscriber subscriber;
    private DataReader<TYPE> dataReader;
    private Selector<TYPE> selector;
    private WaitSet waitset;
    private Read_Callback<TYPE> callback;
    private AtomicLong transactionid;

    public DestinationConnection(ConnectionDescription description,
            Class<TYPE> dataType) {
        super(description, dataType);
        this.callback = null;
        this.transactionid = new AtomicLong(0);
        this.setupSubscriber();
        this.setupDataReader();
        this.setupSelector();
        this.setupWaitset();
    }

    private void setupSubscriber() {
        try {
            SubscriberQos qos = this.getDescription().getSubscriberQos();

            if (qos == null) {
                qos = this.getParticipant().getDefaultSubscriberQos();
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
            this.subscriber = this.getParticipant().createSubscriber(qos);
            this.setLastMessageValidity(VALIDITY_TYPE.VALID);
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
    }

    private void setupDataReader() {
        try {
            DataReaderQos qos = this.getDescription().getDataReaderQos();

            if (qos == null) {
                qos = this.subscriber.getDefaultDataReaderQos();
            }
            this.dataReader = this.subscriber
                    .createDataReader(this.getTopic(), qos);
            this.setLastMessageValidity(VALIDITY_TYPE.VALID);
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
    }

    private void setupWaitset() {
        try {
            this.waitset = this.getDescription().getEnvironment().getSPI()
                    .newWaitSet();
            this.waitset.attachCondition(this.selector.getCondition());
            this.setLastMessageValidity(VALIDITY_TYPE.VALID);
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
    }

    private void setupSelector() {
        try {
            this.selector = this.dataReader.select();
            this.selector = this.selector.dataState(this.subscriber
                    .createDataState().withAnySampleState().withAnyViewState()
                    .with(InstanceState.ALIVE));
            this.selector = this.selector.maxSamples(1);
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
        return CONNECTION_DIRECTION_TYPE.DESTINATION;
    }

    public Read_Callback<TYPE> getCallback() {
        return callback;
    }

    @Override
    public void close() {
        unregisterCallback();
        super.close();

    }

    public void receiveMessage(long timeout, LongHolder transaction_id,
            Holder<TYPE> message, int message_size,
            RETURN_CODE_TYPEHolder return_code) {
        try {
            if (timeout == FACE.INF_TIME_VALUE.value) {
                timeout = Long.MAX_VALUE;
            }
            this.waitset.waitForConditions(timeout, TimeUnit.NANOSECONDS);
            Iterator<TYPE> samples = this.dataReader.take(this.selector);
            Sample<TYPE> sample = samples.next();
            this.setLastMessageValidity(VALIDITY_TYPE.VALID);

            if (sample != null) {
                transaction_id.value = this.transactionid.incrementAndGet();
                message.value = sample.getData();
                return_code.value = RETURN_CODE_TYPE.NO_ERROR;
            } else {
                Logger.getInstance()
                        .log("receiveMessage took no data even though waitset did trigger.",
                                Level.SEVERE);
                return_code.value = RETURN_CODE_TYPE.INVALID_CONFIG;
            }

        } catch (TimeoutException e) {
            Logger.getInstance().log("receiveMessage timed out.", Level.FINE);
            return_code.value = RETURN_CODE_TYPE.TIMED_OUT;
            this.setLastMessageValidity(VALIDITY_TYPE.VALID); /* Timeout is valid */
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
    }

    @SuppressWarnings("unchecked")
    public RETURN_CODE_TYPE registerCallback(
            Read_CallbackHolder<TYPE> callback, int maxMessageSize) {
        if (maxMessageSize > this.getStatus().MAX_MESSAGE_SIZE) {
            return RETURN_CODE_TYPE.INVALID_PARAM;
        }
        this.writeLock();

        this.callback = callback.value;

        final Read_Callback<TYPE> cb = this.callback;
        try {
            this.dataReader.setListener(new DataReaderAdapter<TYPE>() {
                private final Read_Callback<TYPE> callback = cb;

                @Override
                public void onDataAvailable(DataAvailableEvent<TYPE> status) {
                    try {
                        Iterator<TYPE> samples = dataReader.take();
                        setLastMessageValidity(VALIDITY_TYPE.VALID);

                        while (samples.hasNext()) {
                            Sample<TYPE> sample = samples.next();
                            TYPE data = sample.getData();

                            if (data != null) {
                                if (sample.getInstanceState().equals(
                                        InstanceState.ALIVE)) {
                                    RETURN_CODE_TYPEHolder holder = new RETURN_CODE_TYPEHolder();
                                    Holder<TYPE> dataHolder = new Holder<TYPE>();

                                    dataHolder.value = data;
                                    holder.value = RETURN_CODE_TYPE.NO_ERROR;

                                    readLock();
                                    if (this.callback != null) {
                                        this.callback.send_event(transactionid
                                                .incrementAndGet(), dataHolder,
                                                getDescription()
                                                        .getPlatformViewGuid(), 0,
                                                null, holder);
                                    }
                                    readUnlock();

                                    if (holder.value != RETURN_CODE_TYPE.NO_ERROR) {
                                        Logger.getInstance().log(
                                                "send_event callback returned "
                                                        + holder.value,
                                                Level.SEVERE);
                                    }
                                }
                            }
                        }
                        samples.close();
                    } catch (IOException i) {
                        Logger.getInstance()
                                .log("Iterator.close() failed (" + i.getMessage()
                                        + ").", Level.SEVERE);
                        setLastMessageValidity(VALIDITY_TYPE.INVALID);
                    } catch (AlreadyClosedException e) {
                        setLastMessageValidity(VALIDITY_TYPE.INVALID);
                    } catch (Exception e) {
                        Logger.getInstance().log(
                                "Exception occurred" + e.getMessage() + ").",
                                Level.SEVERE);
                        Logger.getInstance().log(e.toString(), Level.FINEST);
                        setLastMessageValidity(VALIDITY_TYPE.INVALID);
                        close();
                    }
                }

            }, DataAvailableStatus.class);
        } catch (AlreadyClosedException e) {
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        } catch (Exception exc) {
            Logger.getInstance().log(exc.getMessage(), Level.SEVERE);
            Logger.getInstance().log(exc.toString(), Level.FINEST);
            this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
        }
        this.writeUnlock();

        return RETURN_CODE_TYPE.NO_ERROR;
    }


    public RETURN_CODE_TYPE unregisterCallback() {
        RETURN_CODE_TYPE result = RETURN_CODE_TYPE.NO_ACTION;

        this.writeLock();

        if (this.callback != null) {
            try {
                this.dataReader.setListener(null);
                this.setLastMessageValidity(VALIDITY_TYPE.VALID);
            } catch(AlreadyClosedException e) {
                this.setLastMessageValidity(VALIDITY_TYPE.INVALID);
            }
            this.callback = null;
            result = RETURN_CODE_TYPE.NO_ERROR;
        }
        this.writeUnlock();

        return result;
    }
}
