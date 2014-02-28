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
import org.opensplice.cm.CMFactory;
import org.opensplice.cm.Service;
import org.opensplice.cm.ServiceState;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;

/**
 * Implementation of the Service interface.
 * 
 * @date May 18, 2005 
 */
public class ServiceImpl extends ParticipantImpl implements Service{

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
     */
    public ServiceImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }
    
    /**
     * Resolves the current state of the Service.
     * 
     * @return The current state of the Service. 
     * @throws CMException Thrown when the service has already been freed, or
     *                     when its kernel service could not be claimed. 
     */
    public ServiceState getState() throws CMException{
        ServiceState state;
        
        if(freed){
            throw new CMException("Service already freed.");
        }
        try {
            state = getCommunicator().serviceGetState(this);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return state;
    }
}
