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
package org.opensplice.dds.domain;

import org.opensplice.dds.core.event.AllDataDisposedEvent;

/**
 * Extension of {@link org.omg.dds.domain.DomainParticipantListener} to provide
 * callback to notify AllDataDisposedEvent as well.
 */
public interface DomainParticipantListener extends org.omg.dds.domain.DomainParticipantListener {

    /**
     * Called whenever {@link org.opensplice.dds.topic.Topic#disposeAllData()}
     * has been performed.
     * 
     * @see org.opensplice.dds.core.event.AllDataDisposedEvent
     * @see org.opensplice.dds.topic.Topic#disposeAllData()
     */
    public void onAllDataDisposed(AllDataDisposedEvent<?> status);
}
