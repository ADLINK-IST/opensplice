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
package org.opensplice.dds.core.status;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.status.SubscriptionMatchedStatus;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class SubscriptionMatchedStatusImpl extends SubscriptionMatchedStatus {
    private static final long serialVersionUID = 5672304659614131159L;
    private final transient OsplServiceEnvironment environment;
    private final int totalCount;
    private final int totalCountChange;
    private final int currentCount;
    private final int currentCountChange;
    private final InstanceHandle lastPublicationHandle;

    public SubscriptionMatchedStatusImpl(OsplServiceEnvironment environment,
            int totalCount, int totalCountChange, int currentCount,
            int currentCountChange, InstanceHandle lastPublicationHandle) {
        this.environment = environment;
        this.totalCount = totalCount;
        this.totalCountChange = totalCountChange;
        this.currentCount = currentCount;
        this.currentCountChange = currentCountChange;
        this.lastPublicationHandle = lastPublicationHandle;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public int getTotalCount() {
        return this.totalCount;
    }

    @Override
    public int getTotalCountChange() {
        return this.totalCountChange;
    }

    @Override
    public int getCurrentCount() {
        return this.currentCount;
    }

    @Override
    public int getCurrentCountChange() {
        return this.currentCountChange;
    }

    @Override
    public InstanceHandle getLastPublicationHandle() {
        return this.lastPublicationHandle;
    }
}
