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
import org.omg.dds.topic.BuiltinTopicKey;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class BuiltinTopicKeyImpl implements BuiltinTopicKey {
    private static final long serialVersionUID = 4755982116057745495L;
    private final transient OsplServiceEnvironment environment;
    private int[] value;

    public BuiltinTopicKeyImpl(OsplServiceEnvironment environment, int[] value) {
        this.environment = environment;

        if (value == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid BuiltinTopicKey provided.");
        }
        this.value = new int[value.length];
        System.arraycopy(value, 0, this.value, 0, value.length);
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public int[] getValue() {
        int[] result = new int[this.value.length];
        System.arraycopy(this.value, 0, result, 0, this.value.length);
        return result;
    }

    @Override
    public void copyFrom(BuiltinTopicKey src) {
        if (src == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal BuiltinTopicKey (null) provided.");
        }
        this.value = src.getValue();
    }

    @Override
    public BuiltinTopicKey clone() {
        return new BuiltinTopicKeyImpl(this.environment, this.value);
    }
}
