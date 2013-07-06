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
package DDS;

public interface DomainParticipantFactoryInterfaceOperations
{
	/* operations  */
	DDS.DomainParticipant create_participant(int domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener, int mask);
	int delete_participant(DDS.DomainParticipant a_participant);
	DDS.DomainParticipant lookup_participant(int domainId);
	int set_default_participant_qos(DDS.DomainParticipantQos qos);
	int get_default_participant_qos(DDS.DomainParticipantQosHolder qos);
	int set_qos(DDS.DomainParticipantFactoryQos qos);
	int get_qos(DDS.DomainParticipantFactoryQosHolder qos);
	DDS.Domain lookup_domain(int domain_id);
	int delete_domain(DDS.Domain a_domain);
	int delete_contained_entities();
}
