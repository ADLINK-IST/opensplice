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

import java.io.Serializable;

import org.opensplice.dds.core.Listener;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.event.AllDataDisposedEventImpl;
import org.opensplice.dds.core.event.InconsistentTopicEventImpl;
import org.opensplice.dds.core.status.StatusConverter;
import org.omg.dds.topic.TopicListener;

import DDS.AllDataDisposedTopicStatusHolder;
import DDS.InconsistentTopicStatus;

public class TopicListenerImpl<TYPE> extends
 Listener<TopicListener<TYPE>>
        implements DDS.ExtTopicListener, Serializable {
    private static final long serialVersionUID = -3957061097858393241L;
    private final AbstractTopic<TYPE> topic;
    private final transient org.opensplice.dds.topic.TopicListener<TYPE> extListener;

    public TopicListenerImpl(OsplServiceEnvironment environment,
            AbstractTopic<TYPE> topic, TopicListener<TYPE> listener) {
        this(environment, topic, listener, false);
    }

    public TopicListenerImpl(OsplServiceEnvironment environment,
            AbstractTopic<TYPE> topic, TopicListener<TYPE> listener,
            boolean waitUntilInitialised) {
        super(environment, listener, waitUntilInitialised);
        this.topic = topic;

        if(listener instanceof org.opensplice.dds.topic.TopicListener<?>){
            this.extListener = (org.opensplice.dds.topic.TopicListener<TYPE>)listener;
        } else{
            this.extListener = null;
        }
    }

    @Override
    public void on_inconsistent_topic(DDS.Topic arg0,
            InconsistentTopicStatus arg1) {
        this.waitUntilInitialised();
        this.listener.onInconsistentTopic(new InconsistentTopicEventImpl<TYPE>(
                this.environment, this.topic, StatusConverter.convert(this.environment,
                        arg1)));

    }

    @Override
    public void on_all_data_disposed(DDS.Topic arg0) {
        AllDataDisposedTopicStatusHolder holder = new AllDataDisposedTopicStatusHolder();
        int rc = arg0.get_all_data_disposed_topic_status(holder);

        if(rc == DDS.RETCODE_OK.value){
            if(extListener != null){
                this.waitUntilInitialised();
                if(holder.value != null){
                    this.extListener
                        .onAllDataDisposed(new AllDataDisposedEventImpl<TYPE>(
                                this.environment, this.topic,
                                StatusConverter.convert(this.environment, holder.value)));
                } else {
                    this.extListener
                    .onAllDataDisposed(new AllDataDisposedEventImpl<TYPE>(
                            this.environment, this.topic, null));
                }
            }
        }
    }
}
