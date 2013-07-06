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

public class DataWriterBase extends EntityImpl {
	
	public class DataWriterBaseImpl extends DDS._DataWriterLocalBase {
		/* datawriter operations  */
		public int set_qos(DDS.DataWriterQos qos) { return 0; }
		public int get_qos(DDS.DataWriterQosHolder qos) { return 0; }
		public int set_listener(DDS.DataWriterListener a_listener, int mask) { return 0; }
		public DDS.DataWriterListener get_listener() { return null; }
		public DDS.Topic get_topic() { return null; }
		public DDS.Publisher get_publisher() { return null; }
		public int wait_for_acknowledgments(DDS.Duration_t max_wait) { return 0; }
		public int get_liveliness_lost_status(DDS.LivelinessLostStatusHolder status) { return 0; }
		public int get_offered_deadline_missed_status(DDS.OfferedDeadlineMissedStatusHolder status) { return 0; }
		public int get_offered_incompatible_qos_status(DDS.OfferedIncompatibleQosStatusHolder status) { return 0; }
		public int get_publication_matched_status(DDS.PublicationMatchedStatusHolder status) { return 0; }
		public int assert_liveliness() { return 0; }
		public int get_matched_subscriptions(DDS.InstanceHandleSeqHolder subscription_handles) { return 0; }
		public int get_matched_subscription_data(DDS.SubscriptionBuiltinTopicDataHolder subscription_data, long subscription_handle) { return 0; }
		
		/* entity operations */
		public int enable() { return 0; }
		public DDS.StatusCondition get_statuscondition() { return null; }
		public int get_status_changes() { return 0; }
		public long get_instance_handle() { return 0; }
	}
	
	private DDS._DataWriterLocalBase base;
	
	public DataWriterBase() {
		base = new DataWriterBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}
