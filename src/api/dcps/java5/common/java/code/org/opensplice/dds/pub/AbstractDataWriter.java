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
package org.opensplice.dds.pub;

import org.omg.dds.pub.DataWriterListener;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.Publisher;
import org.opensplice.dds.core.DomainEntityImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public abstract class AbstractDataWriter<TYPE>
        extends
        DomainEntityImpl<DDS.DataWriter, PublisherImpl, DDS.Publisher, DataWriterQos, DataWriterListener<TYPE>, DataWriterListenerImpl<TYPE>>
        implements org.opensplice.dds.pub.DataWriter<TYPE> {

    public AbstractDataWriter(OsplServiceEnvironment environment,
            PublisherImpl parent) {
        super(environment, parent, parent.getOld());
    }

    @Override
    protected void destroy() {
        this.parent.destroyDataWriter(this);
    }

    @Override
    public Publisher getParent() {
        return this.parent;
    }
}
