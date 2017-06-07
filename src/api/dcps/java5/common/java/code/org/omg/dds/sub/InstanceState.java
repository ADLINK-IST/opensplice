/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.sub;

/**
 * For each instance the Data Distribution Service internally maintains an InstanceState.
 * The InstanceState can be:
 * <ul>
 * <li>ALIVE</li>
 * <li>NOT_ALIVE_DISPOSED</li>
 * <li>NOT_ALIVE_NO_WRITERS</li>
 * </ul>
 * <p>
 *
 * The precise events that cause the InstanceState to change depends on the setting of the
 * {@link org.omg.dds.core.policy.Ownership}:
 * <ul>
 * <li>If OwnershipQosPolicy is set to EXCLUSIVE, then the InstanceState becomes NOT_ALIVE_DISPOSED
 *     only if the DataWriter that "owns" the instance explicitly disposes of it. The instanceState
 *     becomes ALIVE again only if the DataWriter that owns the instance writes it.</li>
 * <li>If OwnershipQosPolicy is set to SHARED, then the instanceState becomes NOT_ALIVE_DISPOSED
 *     if any DataWriter explicitly disposes of the instance. The instanceState becomes ALIVE
 *     as soon as any DataWriter writes the instance again.</li>
 * </ul>
 */
public enum InstanceState {
    // -----------------------------------------------------------------------
    // States
    // -----------------------------------------------------------------------
    /**
     * ALIVE indicates that:
     * <ul>
     * <li>samples have been received for the instance</li>
     * <li>there are live DataWriter objects writing the instance</li>
     * <li>the instance has not been explicitly disposed of (or else samples have been received after it was disposed of)</li>
     * </ul>
     */
    ALIVE(0x0001 << 0),
    /**
     * NOT_ALIVE_DISPOSED indicates the instance was disposed of by a DataWriter
     * either explicitly by means of the {@link org.omg.dds.pub.DataWriter#dispose(org.omg.dds.core.InstanceHandle)}
     * operation or implicitly in case the autodisposeUnregisteredInstances field of the {@link org.omg.dds.core.policy.WriterDataLifecycle}
     * equals TRUE when the instance gets unregistered and no new samples for that instance have been written afterwards.
     */
    NOT_ALIVE_DISPOSED(0x0001 << 1),
    /**
     * NOT_ALIVE_NO_WRITERS indicates the instance has been declared as not-alive by the DataReader because it detected that
     * there are no live DataWriter objects writing that instance.
     */
    NOT_ALIVE_NO_WRITERS(0x0001 << 2);



    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    public final int value;



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    private InstanceState(int value) {
        this.value = value;
    }

}
