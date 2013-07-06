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

import org.opensplice.cm.qos.ReaderQoS;

/**
 * Represents a Subscriber in SPLICE-DDS. 
 */
public interface Subscriber extends Entity {
    
    /**
     * Makes the Subscriber subscribe to data that is published in the 
     * Partitions that match the supplied expression. Remember that the 
     * Partitions must have been created within the Participant of this 
     * Subscriber prior to calling this function.
     * 
     * @param expression The partition expression to apply.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Subscriber is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void subscribe(String expression) throws CMException;
    
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
}
