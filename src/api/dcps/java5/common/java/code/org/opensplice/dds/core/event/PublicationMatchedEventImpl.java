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
import org.omg.dds.core.event.PublicationMatchedEvent;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.pub.DataWriter;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class PublicationMatchedEventImpl<TYPE> extends PublicationMatchedEvent<TYPE> {
    private static final long serialVersionUID = -8206681336933599009L;
    private final transient OsplServiceEnvironment environment;
    private final PublicationMatchedStatus status;

    public PublicationMatchedEventImpl(OsplServiceEnvironment environment,
            DataWriter<TYPE> source, PublicationMatchedStatus status) {
        super(source);
        this.environment = environment;
        this.status = status;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public PublicationMatchedStatus getStatus() {
        return this.status;
    }

    @Override
    public PublicationMatchedEvent<TYPE> clone() {
        return new PublicationMatchedEventImpl<TYPE>(this.environment, this.getSource(), this.status);
    }

    @SuppressWarnings("unchecked")
    @Override
    public DataWriter<TYPE> getSource() {
        return (DataWriter<TYPE>)this.source;
    }

}
