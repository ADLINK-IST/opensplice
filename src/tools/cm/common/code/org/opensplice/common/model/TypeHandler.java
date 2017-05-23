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

package org.opensplice.common.model;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataReader;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.Time;
import org.opensplice.cm.Topic;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.qos.DurabilityKind;
import org.opensplice.cm.qos.HistoryQosKind;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.ReliabilityKind;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.cmdataadapter.protobuf.ProtobufDataAdapterFactory;
import org.opensplice.common.CommonException;

/**
 * TypeHandler is a class whose Singleton instance handles the reading of DCPSType built-in
 * topic samples, processing them into TypeInfo objects. This built-in data reader is operated
 * on a "by request" basis. The DCPSType data reader is queried for new Samples any time a TypeInfo
 * is requested. The TypeHandler will still be instantiated if the host OpenSplice installation does
 * not have the Protobuf feature included, but the DCPSType reader will not be created.
 */
public final class TypeHandler {
    /** The singleton instance for the TypeHandler. */
    private static TypeHandler instance = null;
    /** The name of the built-in partition. */
    private static final String BUILTIN_PARTITION = "__BUILT-IN PARTITION__";
    /** The subscriber for the built-in topics. */
    private Subscriber builtinSubscriber = null;
    /** The reader for DCPSType. */
    private DataReader dCPSTypeReader = null;

    /**
     * Process a Sample obtained from a DCPSType data reader. The Sample is used to retrieve
     * a TypeInfo object, and construct a new TypeEvolution object for it.
     * @param sample A Sample object from a DCPSType data reader.
     */
    private void handleSample(final Sample sample) {
        State state = sample.getState();
        boolean validData = state.test(State.WRITE);

        if (!validData) {
            UserData typeData = sample.getMessage().getUserData();
            String typeName = typeData.getFieldValue("name");
            TypeInfo typeInfo = TypeInfo.getTypeInfoByName(typeName);
            typeInfo.setData(sample);
        }
    }

    /**
     * Create the TypeHandler instance. If the protobuf feature is disabled,
     * then the DDS entities are not created=.
     * @param participant The main application participant.
     * @throws CommonException Thrown when the TypeHandler Entities fail to be created. If thrown,
     * {@link TypeHandler#free()} is automatically called.
     */
    public static void createTypeHandler(Participant participant) throws CommonException {
        if (instance == null) {
            instance = new TypeHandler(participant);
        }
    }

    /**
     * Get the TypeHandler instance.
     */
    public static TypeHandler getTypeHandler() {
        return instance;
    }

    /**
     * Create the TypeHandler instance. The TypeHandler creates a Subscriber and a DataReader entity.
     * These entities must later be freed by calling {@link TypeHandler#free()}. If the protobuf feature
     * is disabled, then the DDS entities are not created, and no exception is thrown.
     * @param participant The main application participant.
     * @throws CommonException Thrown when the TypeHandler Entities fail to be created. If thrown,
     * {@link TypeHandler#free()} is automatically called.
     */
    private TypeHandler(Participant participant) throws CommonException {
        if (ProtobufDataAdapterFactory.getInstance().isEnabled()) {
            try {
                createSubscriber(participant);
                createReader();
            } catch (CMException e) {
                free();
                throw new CommonException(e.getMessage());
            }
        }
    }

    /**
     * Create the built-in subscriber.
     * @param participant The main application participant.
     * @throws CMException Thrown when the Subscriber fails to be created.
     */
    private void createSubscriber(Participant participant) throws CMException {
        SubscriberQoS subQos = SubscriberQoS.getDefault();
        subQos.setPartition(BUILTIN_PARTITION);
        builtinSubscriber = participant.createSubscriber("ospltun_builtin", subQos);
    }

    /**
     * Create the built-in subscriber.
     * @param participant The main application participant.
     * @throws CMException Thrown when the Subscriber fails to be created.
     */
    private void createReader() throws CMException {

        ReaderQoS rdrqos = ReaderQoS.getDefault();
        rdrqos.getDurability().kind = DurabilityKind.VOLATILE;
        rdrqos.getReliability().kind = ReliabilityKind.BESTEFFORT;
        rdrqos.getHistory().kind = HistoryQosKind.KEEPALL;
        rdrqos.getHistory().depth = -1;

        dCPSTypeReader = builtinSubscriber.createDataReader("DCPSTypeReader",
                "select * from DCPSType", rdrqos);

        Time waittime = new Time(0, 0);
        try {
            dCPSTypeReader.waitForHistoricalData(waittime);
        } catch (CMException e) {
            // Do nothing
        }
    }

    /**
     * Take all available Samples from the DCPSType Reader, and process them.
     * If the reader is not instantiated, then this method is a no-op.
     * @throws CommonException Thrown when taking from the Reader fails.
     */
    public void handleUpdates() throws CommonException {
        if (dCPSTypeReader == null) {
            return;
        }
        boolean dataAvail = true;
        while (dataAvail) {
            Sample sample = null;
            try {
                sample = dCPSTypeReader.take();
            } catch (DataTypeUnsupportedException e) {
                throw new CommonException(e.getMessage());
            } catch (CMException e) {
                throw new CommonException(e.getMessage());
            }
            if (sample != null) {
                handleSample(sample);
            } else {
                dataAvail = false;
            }
        }
    }

    /**
     * Frees the TypeHandler Entities, and nullifies the TypeHandler instance.
     */
    public void free() {
        if (dCPSTypeReader != null) {
            dCPSTypeReader.free();
            dCPSTypeReader = null;
        }
        if (builtinSubscriber != null) {
            builtinSubscriber.free();
            builtinSubscriber = null;
        }
        instance = null;
    }

    /**
     * Retrieve the TypeInfo object corresponding to the provided Topic's type. Prior to returning, this
     * method handles updates on the DCPSType Reader to ensure the TypeInfo being retrieved is up
     * to date.
     * @param topic The Topic to find the TypeInfo with.
     * @return The TypeInfo object.
     * @throws CommonException Thrown when trying to get the MetaType from the Topic fails.
     */
    public TypeInfo getTypeInfo (Topic topic) throws CommonException {
        try {
            handleUpdates();
            TypeInfo typeInfo = TypeInfo.getTypeInfoByTopic(topic);
            return typeInfo;
        } catch (CmDataException e) {
            throw new CommonException(e.getMessage());
        }
    }
}
