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

public class TopicDescriptionBase extends SajSuperClass {
	
	public class TopicDescriptionBaseImpl extends DDS._TopicDescriptionLocalBase {
		/* topicdescription operations  */
		public java.lang.String get_type_name() { return null; }
		public java.lang.String get_name() { return null; }
		public DDS.DomainParticipant get_participant() { return null; }
	}
	
	private DDS._TopicDescriptionLocalBase base;
	
	public TopicDescriptionBase() {
		base = new TopicDescriptionBaseImpl();
	}
	
	public String[] _ids() {
		return base._ids();
	}
}