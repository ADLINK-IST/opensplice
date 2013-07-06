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

/**
 * Represents a data reader in SPLICE-DDS.
 *  
 */
public interface DataReader extends Reader {
    public void waitForHistoricalData(Time maxWaitTime) throws CMException;
}
