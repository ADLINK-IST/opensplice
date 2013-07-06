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
 * Represents a Topic in SPLICE-DDS.
 */
public interface Topic extends Entity {
    /**
     * Provides access to the type name of the topic.
     * 
     * @return The type name of the topic.
     */
    public String getTypeName();

    /**
     * Provides access to the key list.
     *
     * @return Comma separated list of keys.
     */
    public String getKeyList();
    
    /**
     * Resolves the data type of the topic.
     * 
     * @return The dataType of the topic.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Topic is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     * @throws DataTypeUnsupportedException Thrown when the data type of the
     *                                      Topic is not supported by this API.
     */
    public MetaType getDataType() throws DataTypeUnsupportedException, CMException;
}
