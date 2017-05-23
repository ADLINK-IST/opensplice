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

import org.opensplice.dds.core.policy.Scheduling.ListenerScheduling;
import org.opensplice.dds.core.policy.Scheduling.WatchdogScheduling;

/**
 * OpenSplice-specific extension to
 * {@link org.omg.dds.domain.DomainParticipantQos} with support to control
 * scheduling class and priorities of listener and watchdog threads created by
 * the middleware.
 * 
 * @see org.opensplice.dds.core.policy.Scheduling
 * @see org.opensplice.dds.core.policy.Scheduling.ListenerScheduling
 * @see org.opensplice.dds.core.policy.Scheduling.WatchdogScheduling
 */
public interface DomainParticipantQos extends
        org.omg.dds.domain.DomainParticipantQos {

    /**
     * Scheduling for the Listener thread of a
     * {@link org.omg.dds.domain.DomainParticipant}
     * 
     * @return The scheduling for the Listener thread of a DomainParticipant.
     * 
     * @see org.opensplice.dds.core.policy.Scheduling.ListenerScheduling
     */
    public ListenerScheduling getListenerScheduling();

    /**
     * Scheduling for the Watchdog thread of a
     * {@link org.omg.dds.domain.DomainParticipant}
     * 
     * @return The scheduling for the Watchdog thread of a DomainParticipant.
     * 
     * @see org.opensplice.dds.core.policy.Scheduling.WatchdogScheduling
     */
    public WatchdogScheduling getWatchdogScheduling();
}
