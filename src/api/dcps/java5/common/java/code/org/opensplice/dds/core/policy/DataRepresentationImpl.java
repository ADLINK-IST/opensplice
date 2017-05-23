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
package org.opensplice.dds.core.policy;

import java.util.List;
import java.util.ArrayList;
import java.util.Collections;

import org.omg.dds.core.policy.DataRepresentation;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class DataRepresentationImpl extends QosPolicyImpl implements
        DataRepresentation {
    private static final long serialVersionUID = -3169717693569991730L;
    private final ArrayList<Short> value;

    public DataRepresentationImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.value = new ArrayList<Short>();
    }

    public DataRepresentationImpl(OsplServiceEnvironment environment,
            List<Short> value) {
        super(environment);

        if (value == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid list of values.");
        }
        this.value = new ArrayList<Short>(value);
    }

    public DataRepresentationImpl(OsplServiceEnvironment environment,
            short... value) {
        super(environment);
        this.value = new ArrayList<Short>(value.length);

        for (short val : value) {
            this.value.add(val);
        }
    }

    @Override
    public Comparable<DataRepresentation> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(DataRepresentation arg0) {
        if (arg0 == null) {
            return 1;
        }
        return this.value.hashCode() - arg0.getValue().hashCode();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof DataRepresentationImpl)) {
            return false;
        }
        DataRepresentationImpl d = (DataRepresentationImpl) other;

        if (d.value.size() != this.value.size()) {
            return false;
        }
        for (int i = 0; i < this.value.size(); i++) {
            if (!(this.value.get(i).equals(d.value.get(i)))) {
                return false;
            }
        }
        return true;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        for (short s : this.value) {
            result = prime * result + s;
        }
        return result;
    }

    @Override
    public List<Short> getValue() {
        return Collections.unmodifiableList(this.value);
    }

    @Override
    public DataRepresentation withValue(List<Short> value) {
        return new DataRepresentationImpl(this.environment, value);
    }

    @Override
    public DataRepresentation withValue(short... value) {
        return new DataRepresentationImpl(this.environment, value);
    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return DataRepresentation.class;
    }

}
