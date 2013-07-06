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
 * Represents a Service in SPLICE-DDS.
 */
public interface Service extends Participant {
    /**
     * Resolves the current state of the Service.
     * 
     * @return The current state of the Service.
     * @throws CMException
     *             TThrown when: - C&M API not initialized. - Service is not
     *             available. - Supplied parameters not correct. - Communication
     *             with SPLICE failed.
     */
    public ServiceState getState() throws CMException;

}
