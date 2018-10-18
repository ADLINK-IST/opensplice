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
package org.opensplice.dds.core.status;

import org.omg.dds.core.status.Status;

/**
 * All instances for one or more {@link org.omg.dds.topic.Topic}s have been
 * disposed through {@link org.opensplice.dds.topic.Topic#disposeAllData()}.
 *
 * @see org.opensplice.dds.topic.Topic#disposeAllData()
 */
public abstract class AllDataDisposedStatus extends Status{
    private static final long serialVersionUID = 5333898527426448236L;

    /**
     * Total cumulative count of the times all instances for the corresponding
     * {@link org.omg.dds.topic.Topic} have been disposed.
     */
    public abstract int getTotalCount();

    /**
     * The incremental number of times all instances have been disposed for the
     * corresponding {@link org.omg.dds.topic.Topic} since the last time the
     * listener was called or the status was read.
     */
    public abstract int getTotalCountChange();
}
