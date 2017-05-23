/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
package org.opensplice.common.model;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import javax.swing.DefaultListModel;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Writer;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.CoherentSetUserDataTableModel;

/**
 * The model for the coherent publisher. It handles calls to the cmapi
 * Publisher, manages freeing of contained entities, and holds the view's table
 * model (for the data) and list model (for the writer list).
 *
 * @date Jul 24, 2016
 */
public class CoherentPublishModel {

    private Publisher publisher;
    private DefaultListModel pubWritersListModel;
    private CoherentSetUserDataTableModel coherentSetUserDataTableModel;
    private boolean setInProgress;

    /**
     * Initializes the CoherentPublishModel.
     * @param _publisher The Publisher entity from which coherent changes are going to be published from.
     * @throws CommonException If there is an error while acquiring the publisher's owned writers.
     */
    public CoherentPublishModel(Publisher _publisher) throws CommonException {
        if (_publisher == null) {
            throw new CommonException("Supplied publisher == null");
        }
        publisher = _publisher;
        pubWritersListModel = new DefaultListModel();
        refreshWriterList();
        coherentSetUserDataTableModel = new CoherentSetUserDataTableModel();
        setInProgress = false;
    }

    /**
     * Adds the supplied data to the table. It is expected that the Writer has
     * already written the data at this point.
     *
     * @param data
     *            The UserData object that was constructed by a user data
     *            editor.
     * @param writeState
     *            The State that the data was written as. State is expected to
     *            be one of WRITE, DISPOSE, REGISTER, UNREGISTER, or (WRITE |
     *            DISPOSE).
     * @param writer
     *            The Writer that was used to write the data.
     * @param typeInfo
     *            The TypeInfo object associated with the Writer's backing data
     *            type. It is used to acquire the type's keys.
     * @throws CommonException
     *             If there was an error while acquired the key fields from the
     *             typeInfo
     */
    public void writeToCoherentSet(UserData data, State writeState, Writer writer, TypeInfo typeInfo)
            throws CommonException {
        coherentSetUserDataTableModel.addData(data, writeState, writer, typeInfo);
    }

    /**
     * Gets this CoherentPublishModel's publisher
     * @return The held publisher.
     */
    public Publisher getPublisher() {
        return publisher;
    }

    /**
     * Invokes beginCoherentChanges() on the publisher.
     * @throws CommonException If the cmapi call to beginCoherentChanges() fails.
     */
    public void beginCoherentChanges() throws CommonException {
        try {
            publisher.beginCoherentChanges();
            coherentSetUserDataTableModel.clear();
            setInProgress = true;
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
    }

    /**
     * Invokes endCoherentChanges() on the publisher.
     * @throws CommonException If the cmapi call to endCoherentChanges() fails.
     */
    public void endCoherentChanges() throws CommonException {
        try {
            publisher.endCoherentChanges();
            setInProgress = false;
        } catch (CMException e) {
            throw new CommonException("Error:" + e.getMessage());
        }
    }

    /**
     * Gets the coherentSetTableModel.
     * @return the coherentSetTableModel
     */
    public CoherentSetUserDataTableModel getCoherentSetTableModel() {
        return coherentSetUserDataTableModel;
    }

    /**
     * Gets the pubWritersListModel.
     * @return the pubWritersListModel
     */
    public DefaultListModel getWriterListModel() {
        return pubWritersListModel;
    }

    /**
     * Synchronizes the Writer list model's contents with what the current state
     * of the publisher's actual owned writers alive in the system. Writers that
     * have been deleted are removed from the list and writers that have been
     * created are added to the list.
     *
     * @throws CommonException
     */
    public void refreshWriterList() throws CommonException {
        List<Entity> ownedWriters = null;
        try {
            ownedWriters = new ArrayList<Entity>(Arrays.asList(publisher.getOwnedEntities(EntityFilter.WRITER)));

            // First, find if any writers in the list no longer exist in the
            // system, and free them, and remove it from the list model.
            List<Object> listedWriters = new ArrayList<Object>(Arrays.asList(pubWritersListModel.toArray()));
            for (Object o : listedWriters) {
                Writer listedWriter = (Writer) o;
                if (!ownedWriters.contains(listedWriter)) {
                    listedWriter.free();
                    pubWritersListModel.removeElement(listedWriter);
                }
            }

            // Then add any new writers to the list.
            Iterator<Entity> it = ownedWriters.iterator();
            while (it.hasNext()) {
                Writer ownedWriter = (Writer) it.next();
                it.remove();
                if (!pubWritersListModel.contains(ownedWriter)) {
                    pubWritersListModel.addElement(ownedWriter);
                } else {
                    ownedWriter.free();
                }
            }
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } finally {
            if (ownedWriters != null) {
                for (Entity w : ownedWriters) {
                    w.free();
                }
            }
        }
    }

    /**
     * Free's the Writers that were allocated for this model's Writer list.
     * This list must be freed
     */
    public void freeWriters() {
        if (pubWritersListModel != null) {
            for (int i = 0; i < pubWritersListModel.size(); i++) {
                ((Writer)pubWritersListModel.get(i)).free();
            }
        }
    }

    /**
     * Returns the publisher's state of coherent changes. Will return true if a
     * call has been made to {@link CoherentPublishModel#beginCoherentChanges()}
     * and no call yet made to {@link CoherentPublishModel#endCoherentChanges()}
     * 
     * @return true if a coherent set is in progress.
     */
    public boolean isCoherentSetInProgress() {
        return setInProgress;
    }
}
