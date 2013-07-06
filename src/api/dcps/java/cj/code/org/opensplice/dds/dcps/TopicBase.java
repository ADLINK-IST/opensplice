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

public class TopicBase extends EntityImpl {
	
	public class TopicBaseImpl extends DDS._TopicLocalBase {
		/* topic operations  */
		public int get_inconsistent_topic_status(DDS.InconsistentTopicStatusHolder a_status) { return 0; }
		public int get_qos(DDS.TopicQosHolder qos) { return 0; }
		public int set_qos(DDS.TopicQos qos) { return 0; }
		public DDS.TopicListener get_listener() { return null; }
		public int set_listener(DDS.TopicListener a_listener, int mask) { return 0; }
		public int dispose_all_data() { return 0; }
		
		/* topicdescription operations  */
		public java.lang.String get_type_name() { return null; }
		public java.lang.String get_name() { return null; }
		public DDS.DomainParticipant get_participant() { return null; }
		
		/* entity operations */
		public int enable() { return 0; }
		public DDS.StatusCondition get_statuscondition() { return null; }
		public int get_status_changes() { return 0; }
		public long get_instance_handle() { return 0; }
	}
	
	private DDS._TopicLocalBase base;
	
	public TopicBase() {
		base = new TopicBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}