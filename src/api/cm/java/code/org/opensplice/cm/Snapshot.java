/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.cm;

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;

/**
 * Abstract class that represents a snapshot of data from a database.
 * 
 * @date Nov 17, 2004 
 */
public abstract interface Snapshot {
    
    /**
     * Provides access to id.
     * 
     * @return Returns the id.
     */
    public String getId();
    
    /**
     * Reads a Sample from the snapshot.
     * 
     * @return The read sample, or null if no Sample is available.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Snapshot is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type is not
     *                                      supported by this API.
     */
    public Sample read() throws DataTypeUnsupportedException, CMException;
    
    /**
     * Takes a Sample from the snapshot.
     * 
     * @return The taken sample, or null if no Sample is available.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Snapshot is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type is not
     *                                      supported by this API.
     */
    public Sample take() throws DataTypeUnsupportedException, CMException;
    
    /**
     * Frees the snapshot.
     */
    public void free();
    
    /**
     * Provides access to the userData type of the data within the snapshot.
     * 
     * @return The userData type of the data.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Snapshot is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type is not
     *                                      supported by this API.
     */
    public MetaType getUserDataType() throws CMException, DataTypeUnsupportedException;
    
}
