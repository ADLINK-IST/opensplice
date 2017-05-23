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

/**
 * OpenSplice-specific extension to {@link org.omg.dds.core.policy.Reliability}
 * That specifies whether a DataWriter should wait for acknowledgements by all connected DataReaders that also have set a synchronous ReliabilityQosPolicy.
 * It is advised only to use this policy in combination with reliability, if used in combination with best effort data may not arrive at the DataReader
 * resulting in a timeout at the DataWriter indicating that the data has not been received.
 * Acknoledgments are always sent reliable so when the DataWriter encounters a timeout it is guaranteed that the DataReader hasn't received the data.

 */
public interface Reliability extends org.omg.dds.core.policy.Reliability{
    /**
     * @return true if synchronous property is set, false otherwise.
     */
    public boolean isSynchronous();

    /**
     * Copy this policy and override the value of the property.
     * @param synchronous       Specifies whether a DataWriter should wait for acknowledgements by all connected DataReaders
     *                          that also have set a synchronous ReliabilityQosPolicy.
     * @return a new Reliability policy
     */
    public Reliability withSynchronous(boolean synchronous);

}
