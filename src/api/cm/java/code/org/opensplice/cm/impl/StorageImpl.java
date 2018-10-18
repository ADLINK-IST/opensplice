/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
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
    @Override
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
    @Override
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
    @Override
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
    @Override
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
