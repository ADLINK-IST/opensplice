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

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.UserData;
import org.omg.dds.topic.BuiltinTopicKey;
import org.omg.dds.topic.ParticipantBuiltinTopicData;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.policy.PolicyConverter;

public class ParticipantBuiltinTopicDataImpl implements
        ParticipantBuiltinTopicData {
    private static final long serialVersionUID = 9013061767095970450L;
    private final transient OsplServiceEnvironment environment;
    private DDS.ParticipantBuiltinTopicData old;
    private BuiltinTopicKey key;

    public ParticipantBuiltinTopicDataImpl(OsplServiceEnvironment environment,
            DDS.ParticipantBuiltinTopicData old) {
        this.environment = environment;
        this.old = old;
        this.key = new BuiltinTopicKeyImpl(this.environment, old.key);
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public BuiltinTopicKey getKey() {
        return this.key;
    }

    @Override
    public UserData getUserData() {
        return PolicyConverter.convert(this.environment, old.user_data);
    }

    @Override
    public void copyFrom(ParticipantBuiltinTopicData src) {
        if (src == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid ParticipantBuiltinTopicData (null) provided.");
        }
        try {
            ParticipantBuiltinTopicDataImpl impl = (ParticipantBuiltinTopicDataImpl) src;
            this.old = impl.old;
            this.key = new BuiltinTopicKeyImpl(this.environment, this.old.key);
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(
                    this.environment,
                    "ParticipantBuiltinTopicData.copyFrom() on non-OpenSplice ParticipantBuiltinTopicData implementation is not supported.");
        }
    }

    @Override
    public ParticipantBuiltinTopicData clone() {
        return new ParticipantBuiltinTopicDataImpl(this.environment, this.old);
    }
}
