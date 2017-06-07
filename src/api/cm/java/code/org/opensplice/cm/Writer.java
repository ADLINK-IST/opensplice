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

import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;

/**
 * Represents a Writer in SPLICE-DDS. 
 */
public interface Writer extends Entity {
    
    /**
     * Makes a snapshot of the current contents of the writer history.
     * 
     * @return The snapshot of the writer history.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public WriterSnapshot makeSnapshot() throws CMException;
    
    /**
     * Writes the supplied data in the Splice system.
     * 
     * @param data The data to write.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void write(UserData data) throws CMException;
    
    /**
     * Disposes the supplied data in the Splice system.
     * 
     * @param data The data to dispose.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void dispose(UserData data) throws CMException;
    
    /**
     * WriteDisposes the supplied data in the Splice system.
     * 
     * @param data The data to writeDispose.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void writeDispose(UserData data) throws CMException;
    
    /**
     * Registers the supplied data as instance in the Splice system.
     * 
     * @param data The data to dispose.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void register(UserData data) throws CMException;
    
    /**
     * Unregisters the supplied data as instance in the Splice system.
     * 
     * @param data The data to dispose.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void unregister(UserData data) throws CMException;
    
    /**
     * Provides access to the userData type of the data the Writer writes.
     * 
     * @return The userData type of the data that is written by the Writer.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Writer is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported.
     */
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException;
}
