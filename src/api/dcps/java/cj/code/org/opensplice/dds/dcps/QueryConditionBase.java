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

public class QueryConditionBase extends ReadConditionImpl {
	
	public class QueryConditionBaseImpl extends DDS._QueryConditionLocalBase {
		/* querycondition operations  */
		public java.lang.String get_query_expression() { return null; }
		public int get_query_parameters(DDS.StringSeqHolder query_parameters) { return 0; }
		public int set_query_parameters(java.lang.String[] query_parameters) { return 0; }
		
		/* readcondition operations  */
		public int get_sample_state_mask() { return 0; }
		public int get_view_state_mask() { return 0; }
		public int get_instance_state_mask() { return 0; }
		public DDS.DataReader get_datareader() { return null; }
		public DDS.DataReaderView get_datareaderview() { return null; }
		
		/* condition operations  */
		public boolean get_trigger_value() { return false; }
	}
	
	private DDS._QueryConditionLocalBase base;
	
	public QueryConditionBase() {
		base = new QueryConditionBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}