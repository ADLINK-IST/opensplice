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

import org.opensplice.cm.qos.WriterQoS;

/**
 * Represents a Publisher in SPLICE-DDS. 
 */
public interface Publisher extends Entity {
    
    /**
     * Makes the Publisher publish data in the Partitions that match the 
     * supplied expression. Remember that the Partitions must have been created 
     * within the Participant of this Publisher prior to calling this function.
     * 
     * @param expression The partition expression to apply.
     * @throws CMException Thrown when:
     *                      - C&M API not initialized.
     *                      - Publisher is not available.
     *                      - Supplied parameters not correct.
     *                      - Communication with SPLICE failed.
     */
    public void publish(String expression) throws CMException;
    
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
}
