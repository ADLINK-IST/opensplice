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
package org.opensplice.dds.domain;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.status.Status;
import org.omg.dds.domain.DomainParticipant;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.domain.DomainParticipantFactoryQos;
import org.omg.dds.domain.DomainParticipantListener;
import org.omg.dds.domain.DomainParticipantQos;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.IllegalOperationExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.Utilities;

public class DomainParticipantFactoryImpl
        extends DomainParticipantFactory
        implements org.opensplice.dds.domain.DomainParticipantFactory {
    private OsplServiceEnvironment environment;
    private DDS.DomainParticipantFactory factory;
    private HashMap<DDS.DomainParticipant, DomainParticipantImpl> participants;

    public DomainParticipantFactoryImpl(OsplServiceEnvironment environment) {
        this.environment = environment;
        this.participants = new HashMap<DDS.DomainParticipant, DomainParticipantImpl>();
        this.factory = DDS.DomainParticipantFactory.get_instance();
    }

    public void destroyParticipant(DomainParticipantImpl participant) {
        DDS.DomainParticipant old = participant.getOld();

        synchronized (this.participants) {
            old.delete_contained_entities();
            int rc = this.factory.delete_participant(old);
            this.participants.remove(old);
            Utilities.checkReturnCode(rc, this.environment,
                    "DomainParticipant.close() failed.");
        }
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public DomainParticipant createParticipant() {
        return createParticipant(DDS.DOMAIN_ID_DEFAULT.value);
    }

    @Override
    public DomainParticipant createParticipant(int domainId) {
        return this.createParticipant(domainId,
                this.getDefaultParticipantQos(), null, new HashSet<Class<? extends Status>>());
    }

    @Override
    public DomainParticipant createParticipant(int domainId,
            DomainParticipantQos qos, DomainParticipantListener listener,
            Collection<Class<? extends Status>> statuses) {
        DomainParticipantImpl participant;

        synchronized (this.participants) {
            participant = new DomainParticipantImpl(this.environment, this,
                    domainId, qos, listener, statuses);
            this.participants.put(participant.getOld(), participant);
        }
        return participant;
    }

    @Override
    public DomainParticipant createParticipant(int domainId,
            DomainParticipantQos qos, DomainParticipantListener listener,
            Class<? extends Status>... statuses) {
        return createParticipant(domainId, qos, listener, Arrays.asList(statuses));
    }

    @Override
    public DomainParticipant lookupParticipant(int domainId) {
        DomainParticipantImpl participant;
        DDS.DomainParticipant oldParticipant;

        synchronized (this.participants) {
            oldParticipant = factory.lookup_participant(domainId);

            if (oldParticipant != null) {
                participant = participants.get(oldParticipant);
            } else {
                participant = null;
            }
        }
        return participant;
    }

    @Override
    public DomainParticipantFactoryQos getQos() {
        DDS.DomainParticipantFactoryQosHolder holder = new DDS.DomainParticipantFactoryQosHolder();
        int rc = this.factory.get_qos(holder);

        Utilities.checkReturnCode(rc, this.environment,
                "DomainParticipantFactory.getQos() failed.");
        return DomainParticipantFactoryQosImpl.convert(this.environment,
                holder.value);
    }

    @Override
    public void setQos(DomainParticipantFactoryQos qos) {
        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DomainParticipantFactoryQos is null.");
        }
        if (qos instanceof DomainParticipantFactoryQosImpl) {
            DDS.DomainParticipantFactoryQos oldQos = ((DomainParticipantFactoryQosImpl) qos)
                    .convert();
            int rc = this.factory.set_qos(oldQos);
            Utilities.checkReturnCode(rc, this.environment,
                    "DomainParticipantFactory.setQos() failed.");
        } else {
            throw new IllegalOperationExceptionImpl(this.environment,
                    "DomainParticipantFactoryQos not supplied by OpenSplice Service provider.");
        }
    }

    @Override
    public DomainParticipantQos getDefaultParticipantQos() {
        DomainParticipantQos qos;
        DDS.DomainParticipantQosHolder holder;
        int rc;

        holder = new DDS.DomainParticipantQosHolder();
        rc = this.factory.get_default_participant_qos(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "DomainParticipantFactory.getDefaultParticipantQos() failed.");

        qos = DomainParticipantQosImpl.convert(this.environment, holder.value);

        return qos;
    }

    @Override
    public void setDefaultParticipantQos(DomainParticipantQos qos) {
        DDS.DomainParticipantQos oldQos;
        int rc;

        if (qos == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Supplied DomainParticipantQos is null.");
        }
        oldQos = ((DomainParticipantQosImpl) qos).convert();
        rc = this.factory.set_default_participant_qos(oldQos);
        Utilities.checkReturnCode(rc, this.environment,
                "DomainParticipantFactory.setDefaultParticipantQos() failed.");

    }

    @Override
    public void detachAllDomains(boolean blockOperations, boolean deleteEntities) {
        this.factory.detach_all_domains(blockOperations, deleteEntities);
    }


}
