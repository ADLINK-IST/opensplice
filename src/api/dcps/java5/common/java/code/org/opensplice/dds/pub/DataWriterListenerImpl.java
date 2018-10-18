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
package org.opensplice.dds.pub;

import java.io.Serializable;

import org.omg.dds.pub.DataWriter;
import org.omg.dds.pub.DataWriterListener;
import org.opensplice.dds.core.Listener;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.event.LivelinessLostEventImpl;
import org.opensplice.dds.core.event.OfferedDeadlineMissedEventImpl;
import org.opensplice.dds.core.event.OfferedIncompatibleQosEventImpl;
import org.opensplice.dds.core.event.PublicationMatchedEventImpl;
import org.opensplice.dds.core.status.StatusConverter;

import DDS.LivelinessLostStatus;
import DDS.OfferedDeadlineMissedStatus;
import DDS.OfferedIncompatibleQosStatus;
import DDS.PublicationMatchedStatus;

public class DataWriterListenerImpl<TYPE> extends
        Listener<DataWriterListener<TYPE>> implements DDS.DataWriterListener,
        Serializable {
    private static final long serialVersionUID = 8422072066415569795L;
    private final transient DataWriter<TYPE> writer;

    public DataWriterListenerImpl(OsplServiceEnvironment environment,
            DataWriter<TYPE> writer, DataWriterListener<TYPE> listener) {
        this(environment, writer, listener, false);
    }

    public DataWriterListenerImpl(OsplServiceEnvironment environment,
            DataWriter<TYPE> writer, DataWriterListener<TYPE> listener,
            boolean waitUntilInitialised) {
        super(environment, listener, waitUntilInitialised);
        this.writer = writer;
    }

    @Override
    public void on_liveliness_lost(DDS.DataWriter arg0,
            LivelinessLostStatus arg1) {
        this.waitUntilInitialised();
        this.listener.onLivelinessLost(new LivelinessLostEventImpl<TYPE>(
                this.environment, writer, StatusConverter.convert(
                        this.environment, arg1)));

    }

    @Override
    public void on_offered_deadline_missed(DDS.DataWriter arg0,
            OfferedDeadlineMissedStatus arg1) {
        this.waitUntilInitialised();
        this.listener.onOfferedDeadlineMissed(new OfferedDeadlineMissedEventImpl<TYPE>(
                this.environment, writer, StatusConverter.convert(
                        this.environment, arg1)));

    }

    @Override
    public void on_offered_incompatible_qos(DDS.DataWriter arg0,
            OfferedIncompatibleQosStatus arg1) {
        this.waitUntilInitialised();
        this.listener.onOfferedIncompatibleQos(new OfferedIncompatibleQosEventImpl<TYPE>(
                this.environment, writer, StatusConverter.convert(
                        this.environment, arg1)));

    }

    @Override
    public void on_publication_matched(DDS.DataWriter arg0,
            PublicationMatchedStatus arg1) {
        this.waitUntilInitialised();
        this.listener.onPublicationMatched(new PublicationMatchedEventImpl<TYPE>(
                this.environment, writer, StatusConverter.convert(
                        this.environment, arg1)));
    }

}
