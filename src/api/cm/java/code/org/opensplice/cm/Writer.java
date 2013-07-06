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
