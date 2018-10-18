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
package org.opensplice.cm;

import org.opensplice.cm.data.Mask;
import org.opensplice.cm.qos.ReaderQoS;

/**
 * Represents a Subscriber in SPLICE-DDS. 
 */
public interface Subscriber extends Entity {
    
    /**
     * Creates a new DataReader for this Subscriber.
     * 
     * @param name The name of the DataReader.
     * @param view The View that determines what data the DataReader reads.
     * @param qos The QoS policies for the DataReader.
     * @return The newly created DataReader.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Subscriber is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public DataReader createDataReader(String name, String view, ReaderQoS qos) throws CMException;

    /**
     * This operation indicates that the application is about to access the data
     * samples in any of the DataReader objects attached to the Subscriber. The
     * application is required to use this operation only if PRESENTATION
     * QosPolicy of the Subscriber to which the DataReader belongs has the
     * access_scope set to 'GROUP'.
     * 
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Subscriber is not available.
     *                      - Communication with SPLICE failed.
     */
    public void beginAccess() throws CMException;

    
    /**
     * Indicates that the application has finished accessing the data samples in
     * DataReader objects managed by the Subscriber. This operation must be used
     * to 'close' a corresponding beginAccess.
     * 
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Subscriber is not available.
     *                      - No corresponding beginAccess has been done
     *                      - Communication with SPLICE failed.
     */
    public void endAccess() throws CMException;

    /**
     * This operation allows the application to access the DataReader objects
     * that belong to this Subscriber independent of their sample, view and
     * instance states.
     */
    public DataReader[] getDataReaders() throws CMException;

    /**
     * This operation allows the application to access the DataReader objects
     * that contain samples with the specified sample states, view states, and
     * instance states.
     * 
     * @param mask
     *            a DataReader will only be placed into the readers collection
     *            if it has data available with one of these sample states, view
     *            states and instance states.
     * @return A collection of DataReaders that contain samples with the
     *         specified sample states.
     */
    public DataReader[] getDataReaders(Mask mask) throws CMException;
}
