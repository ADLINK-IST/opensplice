/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.topic;

import java.io.Serializable;

import org.omg.dds.core.DDSObject;
import org.omg.dds.core.policy.UserData;
import org.omg.dds.type.Extensibility;
import org.omg.dds.type.ID;
import org.omg.dds.type.Key;
/**
* Class that contains information about available DomainParticipants within
* the system.
* <p>
* The DCPSParticipant topic communicates the existence of DomainParticipants
* by means of the ParticipantBuiltinTopicData datatype. Each
* ParticipantBuiltinTopicData sample in a Domain represents a DomainParticipant
* that participates in that Domain: a new ParticipantBuiltinTopicData instance
* is created when a newly-added DomainParticipant is enabled, and it is disposed
* when that DomainParticipant is deleted. An updated ParticipantBuiltinTopicData
* sample is written each time the DomainParticipant modifies its UserDataQosPolicy.
*/
@Extensibility(Extensibility.Kind.MUTABLE_EXTENSIBILITY)
public interface ParticipantBuiltinTopicData
extends Cloneable, Serializable, DDSObject
{
    @ID(0x0050) @Key
    public BuiltinTopicKey getKey();

    @ID(0x002C)
    public UserData getUserData();


    // -----------------------------------------------------------------------

    /**
     * Overwrite the state of this object with that of the given object.
     */
    public void copyFrom(ParticipantBuiltinTopicData src);


    // --- From Object: ------------------------------------------------------

    public ParticipantBuiltinTopicData clone();
}
