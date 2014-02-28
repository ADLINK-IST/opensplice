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
import org.opensplice.cm.DataReader;
import org.opensplice.cm.Time;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.ReaderQoS;

/**
 * Implementation of the DataReader interface.
 * 
 * @date May 18, 2005 
 */
public class DataReaderImpl extends ReaderImpl implements DataReader{
    /** 
     * Constructs a new DataReader from the supplied arguments.
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
    public DataReaderImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name) {
        super(communicator, _index, _serial, _pointer, _name);
    }
    
    /**
     * Constructs a new DataReader from the supplied arguments.
     * @param subscriber The subscriber to attach the DataReader on.
     * @param name The name of the DataReader.
     * @param view The view expression to apply on the DataReader.
     * @param qos The quality of service to apply to the DataReader.
     *
     * @throws CMException Thrown when the DataReader could not be created.
     */
    public DataReaderImpl(SubscriberImpl subscriber, String name, String view, ReaderQoS qos) throws CMException{
        super(subscriber.getCommunicator(), 0, 0, "", "");
        owner = true;
        DataReaderImpl d;
        try {
            d = (DataReaderImpl)getCommunicator().dataReaderNew(subscriber, name, view, qos);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        
        if(d == null){
            throw new CMException("DataReader could not be created.");
        }
        this.index = d.index;
        this.serial = d.serial;
        this.name = d.name;
        this.pointer = d.pointer;
        this.enabled = d.enabled;
        d.freed = true;
    }
    
    public void waitForHistoricalData(Time maxWaitTime) throws CMException{
        if(freed){
            throw new CMException("DataReader has already been freed.");
        }
        try{
            getCommunicator().dataReaderWaitForHistoricalData(this, maxWaitTime);
        } catch (CommunicationException e) {
            throw new CMException("WaitForHistoricalData on '" + this.toString() + 
            "' failed.");
        }
    }
    
    public void setQoS(QoS qos) throws CMException{
        if(freed){
            throw new CMException("Supplied entity is not available (anymore).");
        } else if(qos instanceof ReaderQoS){
            try {
                getCommunicator().entitySetQoS(this, qos);
            } catch (CommunicationException ce) {
                throw new CMException(ce.getMessage());
            }
        } else {
            throw new CMException("Supplied entity requires a ReaderQoS.");
        }
    }
}
