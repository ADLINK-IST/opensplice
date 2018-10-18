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
package org.opensplice.dds.core.event;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.event.RequestedDeadlineMissedEvent;
import org.omg.dds.core.status.RequestedDeadlineMissedStatus;
import org.omg.dds.sub.DataReader;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class RequestedDeadlineMissedEventImpl<TYPE> extends RequestedDeadlineMissedEvent<TYPE> {
    private static final long serialVersionUID = 2098881791326037014L;
    private final transient OsplServiceEnvironment environment;
    private final RequestedDeadlineMissedStatus status;

    public RequestedDeadlineMissedEventImpl(OsplServiceEnvironment environment,
            DataReader<TYPE> source, RequestedDeadlineMissedStatus status) {
        super(source);
        this.environment = environment;
        this.status = status;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public RequestedDeadlineMissedStatus getStatus() {
        return this.status;
    }

    @Override
    public RequestedDeadlineMissedEvent<TYPE> clone() {
        return new RequestedDeadlineMissedEventImpl<TYPE>(this.environment, this.getSource(), this.status);
    }

    @SuppressWarnings("unchecked")
    @Override
    public DataReader<TYPE> getSource() {
        return (DataReader<TYPE>)this.source;
    }

}
