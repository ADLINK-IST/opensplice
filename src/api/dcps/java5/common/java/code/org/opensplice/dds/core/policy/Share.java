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
package org.opensplice.dds.core.policy;

import org.omg.dds.core.policy.QosPolicy;

/**
 * This QosPolicy allows sharing of entities by multiple processes or threads.
 * When the policy is enabled, the data distribution service will try to look up
 * an existing Entity that matches the name supplied in the ShareQosPolicy. A
 * new Entity will only be created if a shared Entity registered under the
 * specified name doesn't exist yet. Shared Readers can be useful for
 * implementing algorithms like the worker pattern, where a single shared reader
 * can contain samples representing different tasks that may be processed in
 * parallel by separate processes. In this algorithm each processes consumes the
 * task it is going to perform (i.e. it takes the sample representing that
 * task), thus preventing other processes from consuming and therefore
 * performing the same task.
 * <p>
 * <b>Note:</b> Entities can only be shared between processes if OpenSplice is
 * running in federated mode, because it requires shared memory to communicate
 * between the different processes. By default, the Share QosPolicy is not used.
 * The name must be set to a valid string for the Share to be valid. This
 * QosPolicy is applicable to DataReader and Subscriber entities, and cannot be
 * modified after the DataReader or Subscriber is enabled. Note that a
 * DataReader can only be shared if its Subscriber is also shared.
 */
public interface Share extends QosPolicy.ForDataReader, QosPolicy.ForSubscriber {
    /**
     * @return The label used to identify the shared Entity.
     */
    public String getName();

    /**
     * Copy this policy and override the value of the property.
     * @param name              The label used to identify the shared Entity.
     * @return a new Share policy
     */
    public Share withName(String name);
}
