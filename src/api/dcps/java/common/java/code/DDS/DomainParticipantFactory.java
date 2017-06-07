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
package DDS;

/**
 * Allow the creation and destruction of {@link DomainParticipant} objects.
 * This class is implemented as a Singleton.
 */
public class DomainParticipantFactory
	extends org.opensplice.dds.dcps.DomainParticipantFactoryBase
{

	/**
     * The one and only instance of the DomainParticipantFactory.
     */
	private static DomainParticipantFactory theParticipantFactory = null;
	private DomainParticipantFactoryInterfaceOperations dpfImpl = null;

	/**
	 * Private constructor to prevent the creation of new instances of the
	 * DomainParticipantFactory. Call the static method get_instance to get a
	 * reference.
	 */
    private DomainParticipantFactory() {
    	dpfImpl = org.opensplice.dds.dcps.DomainParticipantFactoryImpl.get_instance();
    }

    /**
     * Static method to get a reference to the one DomainParticipantFactory
     * instance.
     * @return a reference to the one and only instance of the
     * DomainParticipantFactory.
     */
    public static synchronized DomainParticipantFactory get_instance() {
    	if(theParticipantFactory == null) {
    		theParticipantFactory = new DomainParticipantFactory();
    	}
    	return theParticipantFactory;
    }

    public DDS.DomainParticipant create_participant (int domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener, int mask) {
        return dpfImpl.create_participant (domainId, qos, a_listener, mask);
    }

    public int delete_participant (DDS.DomainParticipant a_participant) {
        return dpfImpl.delete_participant(a_participant);
    }

    public DDS.DomainParticipant lookup_participant (int domainId) {
        return dpfImpl.lookup_participant(domainId);
    }

    public int set_default_participant_qos (DDS.DomainParticipantQos qos) {
        return dpfImpl.set_default_participant_qos(qos);
    }

    public int get_default_participant_qos (DDS.DomainParticipantQosHolder qos) {
        return dpfImpl.get_default_participant_qos(qos);
    }

    public int set_qos (DDS.DomainParticipantFactoryQos qos){
		return dpfImpl.set_qos(qos);
    	}
    public int get_qos (DDS.DomainParticipantFactoryQosHolder qos){
		return dpfImpl.get_qos(qos);
    	}

    public DDS.Domain lookup_domain (int domain_id) {
        return dpfImpl.lookup_domain(domain_id);
    }

    public int delete_domain (DDS.Domain a_domain) {
        return dpfImpl.delete_domain(a_domain);
    }

    public int delete_contained_entities () {
        return dpfImpl.delete_contained_entities();
    }

    public int detach_all_domains (boolean block_operations, boolean delete_entities) {
        return dpfImpl.detach_all_domains(block_operations, delete_entities);
    }
} // DomainParticipantFactory
