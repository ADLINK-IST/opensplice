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
 * Represents a ServiceState of a Service in SPLICE-DDS.
 */
public interface ServiceState extends Entity {
    /**
     * Resolves the name of the state.
     * 
     * @return The name of the state.
     */
    public String getStateName();
    
    /**
     * Resolves the kind of the state.
     * 
     * @return The kind of the state.
     */
    public ServiceStateKind getServiceStateKind();
}
