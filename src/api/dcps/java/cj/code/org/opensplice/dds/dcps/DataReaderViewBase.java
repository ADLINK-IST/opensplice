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

public class DataReaderViewBase extends EntityImpl {
	
	public class DataReaderViewBaseImpl extends DDS._DataReaderViewLocalBase {
		/* datareaderview operations  */
		public DDS.ReadCondition create_readcondition(int sample_states, int view_states, int instance_states) { return null; }
		public DDS.QueryCondition create_querycondition(int sample_states, int view_states, int instance_states, java.lang.String query_expression, java.lang.String[] query_parameters) { return null; }
		public int delete_readcondition(DDS.ReadCondition a_condition) { return 0; }
		public int delete_contained_entities() { return 0; }
		public int set_qos(DDS.DataReaderViewQos qos) { return 0; }
		public int get_qos(DDS.DataReaderViewQosHolder qos) { return 0; }
		public DDS.DataReader get_datareader() { return null; }
		
		/* entity operations */
		public int enable() { return 0; }
		public DDS.StatusCondition get_statuscondition() { return null; }
		public int get_status_changes() { return 0; }
		public long get_instance_handle() { return 0; }
	}
	
	private DDS._DataReaderViewLocalBase base;
	
	public DataReaderViewBase() {
		base = new DataReaderViewBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}