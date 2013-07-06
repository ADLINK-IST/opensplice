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

import org.opensplice.cm.meta.MetaType;

/**
 * Represents a snapshot of the history of a Writer.
 * 
 * @date Nov 17, 2004 
 */
public interface WriterSnapshot extends Snapshot {
    
    /**
     * Provides access to userDataType.
     * 
     * @return Returns the userDataType.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - WriterSnapshot is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when:
     *                                      - Data type of the writer is not
     *                                        supported by this API.
     */
    public MetaType getUserDataType() throws DataTypeUnsupportedException, CMException;

    /**
     * Provides access to writer.
     * 
     * @return Returns the writer.
     */
    public Writer getWriter();
}
