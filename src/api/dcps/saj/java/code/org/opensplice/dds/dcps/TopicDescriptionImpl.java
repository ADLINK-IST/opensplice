/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */


package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.TopicDescription} interface. 
 */ 
public class TopicDescriptionImpl extends SajSuperClass implements DDS.TopicDescription { 

    /* see DDS.TopicDescriptionOperations for javadoc */ 
    public String get_type_name () {
        return jniGetTypeName();
    }

    /* see DDS.TopicDescriptionOperations for javadoc */ 
    public String get_name () {
        return jniGetName();
    }

    /* see DDS.TopicDescriptionOperations for javadoc */ 
    public DDS.DomainParticipant get_participant () {
        return jniGetParticipant();
    }

    private native String jniGetTypeName();
    private native String jniGetName();
    private native DDS.DomainParticipant jniGetParticipant();
}
