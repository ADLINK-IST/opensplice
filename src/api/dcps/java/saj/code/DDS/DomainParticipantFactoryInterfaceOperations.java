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
	int detach_all_domains(boolean block_operations, boolean delete_entities);
}
