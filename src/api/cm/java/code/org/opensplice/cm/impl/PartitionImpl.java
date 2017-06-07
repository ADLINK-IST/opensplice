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
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Partition;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.qos.QoS;

/**
 * Implementation of the Partition interface.
 * 
 * @date May 18, 2005
 */
public class PartitionImpl extends EntityImpl implements Partition{
    public static final String DEFAULT_PARTITION = "";
    public static final String DEFAULT_PARTITION_REPRESENTATION = "<DEFAULT>";

    /**
     * Constructs a new Domain from the supplied arguments. This function is for
     * internal use only and should not be used by API users.
     * 
     * @param _index
     *            The index of the handle of the kernel entity that is
     *            associated with this entity.
     * @param _serial
     *            The serial of the handle of the kernel entity that is
     *            associated with this entity.
     * @param _pointer
     *            The address of the user layer entity that is associated with
     *            this entity.
     * @param _name
     *            The name of the kernel entity that is associated with this
     *            entity.
     */
    public PartitionImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name){
        super(communicator, _index, _serial, _pointer, _name);
    }

    /**
     * Constructs a new Partition from the supplied arguments. The Partition is
     * owner by the caller of this constructor.
     * 
     * @param participant
     *            The participant where to create the partition in.
     * @param name
     *            The name of the partition.
     * @throws CMException
     *             Thrown when Partition could not be created.
     */
    public PartitionImpl(ParticipantImpl participant, String name) throws CMException{
        super(participant.getCommunicator(), 0, 0, "", "");
        owner = true;
        PartitionImpl d;
        try {
            d = (PartitionImpl)getCommunicator().partitionNew(participant, name);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(d == null){
            throw new CMException("Partition could not be created.");
        }
        this.index = d.index;
        this.serial = d.serial;
        this.name = d.name;
        this.pointer = d.pointer;
        this.enabled = d.enabled;
        d.freed = true;
    }

    @Override
    public QoS getQoS() throws CMException{
        throw new CMException("Entity type has no QoS.");
    }

    @Override
    public String toString(){
        String result = super.toString();

        if(PartitionImpl.DEFAULT_PARTITION.equals(result)){
            result = PartitionImpl.DEFAULT_PARTITION_REPRESENTATION;
        }
        return result;
    }

    @Override
    public String toStringExtended(){
        String result = super.toStringExtended();
        String tmp = super.toString();

        if(PartitionImpl.DEFAULT_PARTITION.equals(tmp)){
            result += PartitionImpl.DEFAULT_PARTITION_REPRESENTATION;
        }
        return result;
    }
}
