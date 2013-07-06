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

import org.opensplice.cm.data.UserData;

/**
 * Represents a storage as used by Recording & Replay. It doesn't extend the
 * Entity interface since a R&R storage is not a DDS entity. This interface
 * provides a DDS C&M API alike mechanism of retrieving or storing data in a
 * storage.
 *
 * Any call can throw a CMException right now. Perhaps some more granularity
 * should be added to distinguish the reason for the failure (e.g.,
 * CommunicationException, NotImplementedException, OutOfResourcesException, &c.).
 */
public interface Storage {
    public enum Result {
        SUCCESS, INVALID, FAILED, ERROR, OUTOFRESOURCES, BUSY, NOTIMPLEMENTED
    };

    /**
     * Opens the storage.
     * @throws CMException whenever any kind of error occured causing the storage
     * not to be opened.
     * @post The storage is opened and can be used to read and/or append data to
     */
    Result open(String attrs) throws CMException;

    /**
     * Closes the storage.
     * @throws CMException whenever any kind of error occured causing the storage
     * not to be closed.
     * @pre The store has been opened successfully
     * @post The storage is closed
     */
    Result close() throws CMException;

    /**
     * Appends data to the storage.
     * @param data The data to be appended to the storage
     * @throws CMException whenever any kind of error occured causing the data
     * not to be appended
     * @pre The store has been opened successfully
     * @post The data is successfully appended to the storage
     */
    Result append(UserData data) throws CMException;

    /**
     * Reads data to the storage. This read has pure read-forward/FIFO behaviour.
     * @return The data read from the storage, or null if no more data was
     * available
     * @throws CMException whenever any kind of error occured preventing data
     * to be read from the store
     * @pre The store has been opened successfully
     */
    UserData read() throws CMException;
}
