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
package org.opensplice.cm;

import org.opensplice.cm.qos.WriterQoS;

/**
 * Represents a Publisher in SPLICE-DDS. 
 */
public interface Publisher extends Entity {
    
    /**
     * Creates a Writer for this Publisher. The supplied Topic must be known
     * in SPLICE.
     * 
     * @param name The name of the Writer.
     * @param topic The Topic that the Writer must write.
     * @param qos The QoS policies for the Writer.
     * @return The newly created Writer.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Publisher is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public Writer createWriter(String name, Topic topic, WriterQoS qos) throws CMException;

    /**
     * This operation requests that the application will begin a 'coherent set'
     * of modifications using DataWriter objects attached to this Publisher. The
     * 'coherent set' will be completed by a matching call to endCoherentChanges
     * 
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Publisher is not available.
     *                      - Communication with SPLICE failed.
     */
    public void beginCoherentChanges() throws CMException;

    /**
     * This operation terminates the 'coherent set' initiated by the matching
     * call to beginCoherentChanges
     * 
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Publisher is not available.
     *                      - Communication with SPLICE failed.
     */
    public void endCoherentChanges() throws CMException;
}
