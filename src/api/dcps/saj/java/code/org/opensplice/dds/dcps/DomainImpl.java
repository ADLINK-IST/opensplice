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
 * Implementation of the {@link DDS.Domain} interface. 
 */ 
public class DomainImpl extends EntityImpl implements DDS.Domain { 

    /* see DDS.DomainOperations for javadoc */ 
    public int create_persistent_snapshot (String partition_expression, String topic_expression, String URI) {
        return jniCreatePersistentSnapshot(partition_expression, topic_expression,URI);
    }

    private native int jniCreatePersistentSnapshot(String partition_expression, String topic_expression, String URI);
}
