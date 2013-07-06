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
package org.opensplice.dds.dcps;

public class DomainParticipantFactoryBase extends SajSuperClass {
	
	public class DomainParticipantFactoryBaseImpl extends DDS._DomainParticipantFactoryInterfaceLocalBase {
		/* domainparticipantfactoryinterface operations  */
		public DDS.DomainParticipant create_participant(int domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener, int mask) { return null; }
		public int delete_participant(DDS.DomainParticipant a_participant) { return 0; }
		public DDS.DomainParticipant lookup_participant(int domainId) { return null; }
		public int set_default_participant_qos(DDS.DomainParticipantQos qos) { return 0; }
		public int get_default_participant_qos(DDS.DomainParticipantQosHolder qos) { return 0; }
		public int set_qos(DDS.DomainParticipantFactoryQos qos) { return 0; }
		public int get_qos(DDS.DomainParticipantFactoryQosHolder qos) { return 0; }
		public DDS.Domain lookup_domain(int domain_id) { return null; }
		public int delete_domain(DDS.Domain a_domain) { return 0; }
		public int delete_contained_entities() { return 0; }
	}
	
	private DDS._DomainParticipantFactoryInterfaceLocalBase base;
	
	public DomainParticipantFactoryBase() {
		base = new DomainParticipantFactoryBaseImpl();
	}
	
	public String[] _ids()	{
		return base._ids();
	}
	
}
