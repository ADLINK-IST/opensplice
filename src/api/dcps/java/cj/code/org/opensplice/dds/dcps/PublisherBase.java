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

public class PublisherBase extends EntityImpl {
	
	public class PublisherBaseImpl extends DDS._PublisherLocalBase {
		/* publisher operations */
		public DDS.DataWriter create_datawriter(DDS.Topic a_topic, DDS.DataWriterQos qos, DDS.DataWriterListener a_listener, int mask) { return null; }
		public int delete_datawriter(DDS.DataWriter a_datawriter) { return 0; }
		public DDS.DataWriter lookup_datawriter(java.lang.String topic_name) { return null; }
		public int delete_contained_entities() { return 0; }
		public int set_qos(DDS.PublisherQos qos) { return 0; }
		public int get_qos(DDS.PublisherQosHolder qos) { return 0; }
		public int set_listener(DDS.PublisherListener a_listener, int mask) { return 0; }
		public DDS.PublisherListener get_listener() { return null; }
		public int suspend_publications() { return 0; }
		public int resume_publications() { return 0; }
		public int begin_coherent_changes() { return 0; }
		public int end_coherent_changes() { return 0; }
		public int wait_for_acknowledgments(DDS.Duration_t max_wait) { return 0; }
		public DDS.DomainParticipant get_participant() { return null; }
		public int set_default_datawriter_qos(DDS.DataWriterQos qos) { return 0; }
		public int get_default_datawriter_qos(DDS.DataWriterQosHolder qos) { return 0; }
		public int copy_from_topic_qos(DDS.DataWriterQosHolder a_datawriter_qos, DDS.TopicQos a_topic_qos) { return 0; }
		
		/* entity operations */
		public int enable() { return 0; }
		public DDS.StatusCondition get_statuscondition() { return null; }
		public int get_status_changes() { return 0; }
		public long get_instance_handle() { return 0; }
	}
	
	private DDS._PublisherLocalBase base;
	
	public PublisherBase() {
		base = new PublisherBaseImpl();
	}
	
	public String[] _ids()	{
		return base._ids();
	}
	
}