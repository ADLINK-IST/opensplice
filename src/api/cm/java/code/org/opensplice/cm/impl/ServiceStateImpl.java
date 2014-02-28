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
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
import org.opensplice.cm.ServiceState;
import org.opensplice.cm.ServiceStateKind;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.status.Status;

/**
 * Implementation of the ServiceState interface.
 * 
 * @date May 18, 2005 
 */
public class ServiceStateImpl extends EntityImpl implements ServiceState {
    private String stateName;
    private ServiceStateKind kind;
    
    /** 
     * Constructs a new Service from the supplied arguments. This function
     * is for internal use only and should not be used by API users.
     * 
     * @param _index The index of the handle of the kernel entity that is
     *               associated with this entity.
     * @param _serial The serial of the handle of the kernel entity that is
     *                associated with this entity.
     * @param _pointer The address of the user layer entity that is associated
     *                 with this entity.
     * @param _name The name of the kernel entity that is associated with this
     *              entity.
     * @param _stateName The name of the state.
     * @param _kind The state kind of the state.
     */
    public ServiceStateImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name,
                            String _stateName, ServiceStateKind _kind) {
        super(communicator, _index, _serial, _pointer, _name);
        stateName = _stateName;
        kind = _kind;
    }
    
    /**
     * Resolves the name of the state.
     * 
     * @return The name of the state.
     */
    public String getStateName(){
        return stateName;
    }
    
    /**
     * Resolves the kind of the state.
     * 
     * @return The kind of the state.
     */
    public ServiceStateKind getServiceStateKind(){
        return kind;
    }
    
    /**
     * Frees the entity, by deregistering it from the factory.
     */
    public synchronized void free(){
        /*do nothing.*/
    }
    
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }
}
