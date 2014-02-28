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
import org.opensplice.cm.Storage;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.data.UserData;

/**
 * @author Maurits
 *
 */
public class StorageImpl implements Storage {
    /**
     * Opaque representation of storage as provided by underlying API's. This is
     * null if not opened.
     */
    private Object opaqueStorage;

    private final Communicator communicator;

    /**
     * Creates a new Storage in un-opened state.
     */
    public StorageImpl(Communicator communicator){
        if(communicator == null) {
        	throw new IllegalArgumentException("The communicator parameter can not be null.");
        }
        this.opaqueStorage = null;
        this.communicator = communicator;
    }

    /* (non-Javadoc)
     * @see org.opensplice.cm.Storage#open()
     */
    public Result open(String attrs) throws CMException {
        Result result = Result.SUCCESS;

        if(this.opaqueStorage == null){
            /* Storage not yet opened */
            try {
                this.opaqueStorage = getCommunicator().storageOpen(attrs);
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            }

            if(this.opaqueStorage == null){
                result = Result.ERROR; // TODO: Provide actual error
            }
        }  /* Storage already opened */

        return result;
    }

    /* (non-Javadoc)
     * @see org.opensplice.cm.Storage#close()
     */
    public Result close() throws CMException {
        Result result = Result.ERROR;
        try {
            result = getCommunicator().storageClose(this.opaqueStorage);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }

        return result;
    }

    /* (non-Javadoc)
     * @see org.opensplice.cm.Storage#append(org.opensplice.cm.data.UserData)
     */
    public Result append(UserData data) throws CMException {
        Result result = Result.ERROR;

        try {
            result = getCommunicator().storageAppend(this.opaqueStorage, data);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }

        return result;
    }

    /* (non-Javadoc)
     * @see org.opensplice.cm.Storage#read()
     */
    public UserData read() throws CMException {
        UserData data = null;

        try {
            data = getCommunicator().storageRead(this.opaqueStorage);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }

        return data;
    }

    protected Communicator getCommunicator() throws CMException {
      return communicator;
    }
}
