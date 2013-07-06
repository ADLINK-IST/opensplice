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

class SubscriberBase extends EntityImpl {
	
	public class SubscriberBaseImpl extends DDS._SubscriberLocalBase {
		/* subscriber operations  */
		public DDS.DataReader create_datareader(DDS.TopicDescription a_topic, DDS.DataReaderQos qos, DDS.DataReaderListener a_listener, int mask) { return null; }
		public int delete_datareader(DDS.DataReader a_datareader) { return 0; }
		public int delete_contained_entities() { return 0; }
		public DDS.DataReader lookup_datareader(java.lang.String topic_name) { return null; }
		public int get_datareaders(DDS.DataReaderSeqHolder readers, int sample_states, int view_states, int instance_states) { return 0; }
		public int notify_datareaders() { return 0; }
		public int set_qos(DDS.SubscriberQos qos) { return 0; }
		public int get_qos(DDS.SubscriberQosHolder qos) { return 0; }
		public int set_listener(DDS.SubscriberListener a_listener, int mask) { return 0; }
		public DDS.SubscriberListener get_listener() { return null; }
		public int begin_access() { return 0; }
		public int end_access() { return 0; }
		public DDS.DomainParticipant get_participant() { return null; }
		public int set_default_datareader_qos(DDS.DataReaderQos qos) { return 0; }
		public int get_default_datareader_qos(DDS.DataReaderQosHolder qos) { return 0; }
		public int copy_from_topic_qos(DDS.DataReaderQosHolder a_datareader_qos, DDS.TopicQos a_topic_qos) { return 0; }
		
		/* entity operations */
		public int enable() { return 0; }
		public DDS.StatusCondition get_statuscondition() { return null; }
		public int get_status_changes() { return 0; }
		public long get_instance_handle() { return 0; }
	}
	
	private DDS._SubscriberLocalBase base;
	
	public SubscriberBase() {
		base = new SubscriberBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}
