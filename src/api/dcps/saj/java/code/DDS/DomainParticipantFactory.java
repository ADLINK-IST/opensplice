/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
package DDS;

/**
 * Allow the creation and destruction of {@link DomainParticipant} objects.
 * This class is implemented as a Singleton.
 */
public class DomainParticipantFactory
	implements DomainParticipantFactoryOperations
{

	/**
     * The one and only instance of the DomainParticipantFactory.
     */
	private static DomainParticipantFactory theParticipantFactory = null;
	private DomainParticipantFactoryOperations dpfImpl = null;

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

    public DDS.DomainParticipant create_participant (String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener, int mask) {
        return dpfImpl.create_participant (domainId, qos, a_listener, mask);
    }

    public int delete_participant (DDS.DomainParticipant a_participant) {
        return dpfImpl.delete_participant(a_participant);
    }

    public DDS.DomainParticipant lookup_participant (String domainId) {
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

    public DDS.Domain lookup_domain (String domain_id) {
        return dpfImpl.lookup_domain(domain_id);
    }

    public int delete_domain (DDS.Domain a_domain) {
        return dpfImpl.delete_domain(a_domain);
    }

    public int delete_contained_entities () {
        return dpfImpl.delete_contained_entities();
    }
} // DomainParticipantFactory
