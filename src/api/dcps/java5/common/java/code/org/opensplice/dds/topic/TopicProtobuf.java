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
package org.opensplice.dds.topic;

import java.util.Collection;

import org.omg.dds.core.status.Status;
import org.omg.dds.topic.TopicListener;
import org.omg.dds.topic.TopicQos;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.domain.DomainParticipantImpl;
import org.opensplice.dds.type.TypeSupportProtobuf;

public class TopicProtobuf<PROTOBUF_TYPE> extends TopicImpl<PROTOBUF_TYPE> {

    public TopicProtobuf(OsplServiceEnvironment environment,
            DomainParticipantImpl participant, String topicName,
            TypeSupportProtobuf<PROTOBUF_TYPE, ?> typeSupport, TopicQos qos,
            TopicListener<PROTOBUF_TYPE> listener,
            Collection<Class<? extends Status>> statuses) {
        super(environment, participant, topicName, typeSupport, qos, listener,
                statuses);
    }
}
