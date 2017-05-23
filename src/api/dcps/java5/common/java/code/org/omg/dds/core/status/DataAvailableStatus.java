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
package org.omg.dds.core.status;

import org.omg.dds.core.policy.Ownership;

import DDS.OwnershipQosPolicyKind;

/**
 * There is new data in the DataReader
 *
 * Any of the following events will cause the DataAvailableStatus to become TRUE:
 * <ul>
 * <li>The arrival of new data.</li>
 * <li>A change in the InstanceState of a contained instance. This can be caused by either:
 *      <ul><li>The arrival of the notification that an instance has been disposed by:
 *          <ul><li>the DataWriter that owns it if {@link Ownership} QoS kind= {@link OwnershipQosPolicyKind}</li>
 *          <li>or by any DataWriter if {@link Ownership} QoS kind=SHARED.</li></ul>
 *      </li></ul>
 * </li>
 * <li>The loss of liveliness of the DataWriter of an instance for which there is no other DataWriter.</li>
 * <li>The arrival of the notification that an instance has been unregistered by the only DataWriter that is
 *     known to be writing the instance.</li>
 * </ul>
 *
 * Any of the following events will cause the DataAvailableStatus to become FALSE:
 * <ul>
 * <li> The DataAvailableStatus becomes FALSE when either the corresponding listener operation (onDataAvailable) is called
 * or the read or take operation (or their variants) is called on the associated DataReader.</li>
 * <li>The DataOnReadersStatus becomes FALSE when any of the following events occurs:
 *      <ul><li>The corresponding listener operation (onDataOnReaders) is called.</li>
 *      <li>The onDataAvailable listener operation is called on any DataReader belonging to the Subscriber.</li>
 *      <li>The read or take operation (or their variants) is called on any DataReader belonging to the Subscriber.</li>
 *      </ul>
 * </li>
 * </ul>
 *
 * @see org.omg.dds.core.event.DataAvailableEvent
 */
public abstract class DataAvailableStatus extends Status {

	/**
	 * Constants
	 */
	private static final long serialVersionUID = -784438173237266372L;

}
