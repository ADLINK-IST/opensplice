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
package org.opensplice.dds.core.event;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.event.DataAvailableEvent;
import org.omg.dds.core.status.DataAvailableStatus;
import org.omg.dds.sub.DataReader;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.status.DataAvailableStatusImpl;

public class DataAvailableEventImpl<TYPE> extends DataAvailableEvent<TYPE> {
    private static final long serialVersionUID = 5283779390935565015L;
    private final transient OsplServiceEnvironment environment;
    private final DataAvailableStatusImpl status;
    
    public DataAvailableEventImpl(OsplServiceEnvironment environment, DataReader<TYPE> source, DataAvailableStatusImpl status) {
        super(source);
        this.environment = environment;
        this.status = status;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public DataAvailableStatus getStatus() {
        return this.status;
    }

    @Override
    public DataAvailableEvent<TYPE> clone() {
        return new DataAvailableEventImpl<TYPE>(this.environment, this.getSource(), this.status);
    }

    @SuppressWarnings("unchecked")
    @Override
    public DataReader<TYPE> getSource() {
        return (DataReader<TYPE>)this.source;
    }

}
