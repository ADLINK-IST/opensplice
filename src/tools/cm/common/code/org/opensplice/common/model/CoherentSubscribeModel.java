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
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataReader;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.Topic;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.qos.PresentationKind;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.common.CommonException;
import org.opensplice.common.model.table.CoherentSetSampleTableModel;

/**
 * The model for the coherent subscriber. It handles calls to the cmapi
 * Subscriber, manages freeing of contained entities, and holds the view's table
 * model (for the samples).
 *
 * @date Jul 26, 2016
 */
public class CoherentSubscribeModel {

    private Subscriber subscriber;
    private PresentationKind accessScope;
    private CoherentSetSampleTableModel coherentSetSampleTableModel;
    private Map<Reader, TypeInfo> subReaders;

    /**
     * Initializes the CoherentSubscribeModel.
     * @param _subscriber The Subscriber entity from which coherent changes are going to be subscribed from.
     * @throws TunerException If there is an error while acquiring the subscriber's owned readers.
     */
    public CoherentSubscribeModel(Subscriber _subscriber) throws CommonException {
        if (_subscriber == null) {
            throw new CommonException("Supplied subscriber == null");
        }
        subscriber = _subscriber;
        try {
            accessScope = ((SubscriberQoS) subscriber.getQoS()).getPresentation().access_scope;
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
        subReaders = new HashMap<Reader, TypeInfo>();
        refreshReaderList();
        coherentSetSampleTableModel = new CoherentSetSampleTableModel();
    }

    /**
     * Takes from all readers (in order) with data available under this
     * subscriber and passes all samples to the table model.
     *
     * @return true if samples were received, false if none.
     * @throws TunerException
     */
    public boolean takeSet() throws CommonException {
        Reader rtmp = null;
        try {
            CommonException ce = null;
            subscriber.beginAccess();
            DataReader[] orderedReaders = subscriber.getDataReaders();
            for (int i = 0; i < orderedReaders.length; i++) {
                Reader r = null;
                TypeInfo ti = null;
                rtmp = orderedReaders[i];
                if (!subReaders.containsKey(rtmp)) {
                    refreshReaderList();
                }
                for (Entry<Reader, TypeInfo> e : subReaders.entrySet()) {
                    if (e.getKey().equals(rtmp)) {
                        r = e.getKey();
                        ti = e.getValue();
                    }
                }
                try {
                    Sample s = r.take();
                    if (accessScope.value() == PresentationKind.GROUP.value()) {
                        if (s != null) {
                            coherentSetSampleTableModel.addData(s, r, ti);
                        }
                    } else {
                        while (s != null) {
                            coherentSetSampleTableModel.addData(s, r, ti);
                            s = r.take();
                        }
                    }
                } catch (CMException e) {
                    ce = new CommonException(e.getMessage());
                } catch (DataTypeUnsupportedException e) {
                    ce = new CommonException(e.getMessage());
                }
                if (!rtmp.isOwner()) {
                    rtmp.free();
                }
            }
            if (ce != null) {
                throw ce;
            }
            if (coherentSetSampleTableModel.getRowCount() != 0
                    && coherentSetSampleTableModel.getSample(coherentSetSampleTableModel.getRowCount() - 1) != null) {
                // Add a dummy row to visually separate the next set.
                coherentSetSampleTableModel.addDummyRow();
            }
            if (orderedReaders.length == 0) {
                return false;
            }
            return true;
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } finally {
            if (rtmp != null && !rtmp.isFreed() && !rtmp.isOwner()) {
                rtmp.free();
            }
            try {
                subscriber.endAccess();
            } catch (CMException e) {/*do nothing*/}
        }
    }

    /**
     * Synchronizes the Reader list contents with what the current state of the
     * subscriber's actual owned readers alive in the system. Readers that have
     * been deleted are removed from the list and readers that have been created
     * are added to the list.
     *
     * @throws TunerException
     */
    private void refreshReaderList() throws CommonException {
        List<Entity> ownedReaders = null;
        Topic t = null;
        try {
            ownedReaders = new ArrayList<Entity>(Arrays.asList(subscriber.getOwnedEntities(EntityFilter.READER)));

            // First, find if any readers in the list no longer exist in the
            // system, and free them, and remove it from the list model.
            Set<Reader> listedReaders = new HashSet<Reader>(subReaders.keySet());
            for (Reader listedReader : listedReaders) {
                if (!ownedReaders.contains(listedReader)) {
                    listedReader.free();
                    subReaders.remove(listedReader);
                }
            }

            // Then add any new readers to the list.
            Iterator<Entity> it = ownedReaders.iterator();
            while (it.hasNext()) {
                Reader r = (Reader) it.next();
                it.remove();
                if (!subReaders.containsKey(r)) {
                    t = (Topic) r.getOwnedEntities(EntityFilter.TOPIC)[0];
                    TypeInfo ti = TypeHandler.getTypeHandler().getTypeInfo(t);
                    t.free();
                    subReaders.put((Reader) r, ti);
                } else {
                    r.free();
                }
            }
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } finally {
            if (t != null) {
                t.free();
            }
            if (ownedReaders != null) {
                for (Entity r : ownedReaders) {
                    r.free();
                }
            }
        }
    }

    public void clearModel() {
        coherentSetSampleTableModel.clear();
    }

    /**
     * Gets this CoherentSubscribeModel's subscriber
     * @return The held publisher.
     */
    public Subscriber getSubscriber() {
        return subscriber;
    }

    /**
     * Gets the coherentSetTableModel.
     * @return the coherentSetTableModel
     */
    public CoherentSetSampleTableModel getCoherentSetTableModel() {
        return coherentSetSampleTableModel;
    }

    /**
     * Gets the subReaders.
     * @return the subReaders
     */
    public Set<Reader> getReaders() {
        return subReaders.keySet();
    }

    /**
     * Frees the Readers that were allocated for this model's Reader list.
     * This list must be freed.
     */
    public void freeReaders() {
        if (subReaders != null) {
            for (Reader r : subReaders.keySet()) {
                r.free();
            }
            subReaders.clear();
        }
    }
}