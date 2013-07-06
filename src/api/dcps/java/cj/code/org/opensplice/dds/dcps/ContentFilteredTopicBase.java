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

public class ContentFilteredTopicBase extends TopicDescriptionImpl {
	
	public class ContentFilteredTopicBaseImpl extends DDS._ContentFilteredTopicLocalBase {
		/* contentfilteredtopic operations  */
		public java.lang.String get_filter_expression() { return null; }
		public int get_expression_parameters(DDS.StringSeqHolder expression_parameters) { return 0; }
		public int set_expression_parameters(java.lang.String[] expression_parameters) { return 0; }
		public DDS.Topic get_related_topic() { return null; }
		
		/* topicdescription operations  */
		public java.lang.String get_type_name() { return null; }
		public java.lang.String get_name() { return null; }
		public DDS.DomainParticipant get_participant() { return null; }
	}
	
	private DDS._ContentFilteredTopicLocalBase base;
	
	public ContentFilteredTopicBase() {
		base = new ContentFilteredTopicBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
	
}