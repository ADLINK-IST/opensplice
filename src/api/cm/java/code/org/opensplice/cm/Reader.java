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

import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;



/**
 * Represents a Reader in SPLICE-DDS. This interface has been defined
 * abstract because only descendants of this interface can actually exist. 
 */
public abstract interface Reader extends Entity {
    /**
     * Makes a snapshot of the current contents of the reader database.
     * 
     * @return The snapshot of the reader database.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Reader is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public ReaderSnapshot makeSnapshot() throws CMException;
    
    /**
     * Provides access to the userData type of the contents of the reader
     * database.
     * 
     * @return The userData type of the data in the database.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Reader is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported.
     */
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException;
    
    /**
     * Reads data from Reader database.
     * 
     * @return The read Sammple, or null of no data was available.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Reader is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported.
     */
    public Sample read() throws DataTypeUnsupportedException, CMException;
    
    /**
     * Takes data from Reader database.
     * 
     * @return The taken Sammple, or null of no data was available.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Reader is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported.
     */
    public Sample take() throws DataTypeUnsupportedException, CMException;
    
    /**
     * Reads data from Reader database.
     * 
     * @return The read Sammple, or null of no data was available.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Reader is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when:
     *                                          - Data type of the Topic is not supported.
     */
    public Sample readNext(GID instanceHandle) throws DataTypeUnsupportedException, CMException;
    
    /**
     * Creates a new Query for this Reader.
     * 
     * @param name The name of the Query.
     * @param expression The query expression for the Query.
     * @return The newly created Query.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Reader is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public Query createQuery(String name, String expression) throws CMException;
}
