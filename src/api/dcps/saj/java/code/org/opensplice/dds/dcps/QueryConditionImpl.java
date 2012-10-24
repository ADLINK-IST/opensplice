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
 * Implementation of the {@link DDS.QueryCondition} interface. 
 */ 
public class QueryConditionImpl extends ReadConditionImpl implements DDS.QueryCondition { 

    /* see DDS.QueryConditionOperations for javadoc */ 
    public String get_query_expression () {
        return jniGetQueryExpression();
    }

    /* see DDS.QueryConditionOperations for javadoc */ 
    public int get_query_parameters (DDS.StringSeqHolder query_parameters) {
        return jniGetQueryParameters(query_parameters);
    }

    /* see DDS.QueryConditionOperations for javadoc */ 
    public int set_query_parameters (String[] query_parameters) {
        return jniSetQueryParameters(query_parameters);
    }


    private native String jniGetQueryExpression();
    private native int jniGetQueryParameters(DDS.StringSeqHolder query_parameters);
    private native int jniSetQueryParameters(String[] query_parameters);
}
