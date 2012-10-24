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
 * Implementation of the {@link DDS.MultiTopic} interface. 
 */ 
public class MultiTopicImpl extends TopicDescriptionImpl implements DDS.MultiTopic { 

    /* see DDS.MultiTopicOperations for javadoc */ 
    public String get_subscription_expression () {
        return jniGetSubscriptionExpression();
    }

    /* see DDS.MultiTopicOperations for javadoc */ 
    public int get_expression_parameters (DDS.StringSeqHolder expression_parameters) {
        return jniGetExpressionParameters(expression_parameters);
    }

    /* see DDS.MultiTopicOperations for javadoc */ 
    public int set_expression_parameters (String[] expression_parameters) {
        return jniSetExpressionParameters(expression_parameters);
    }

    private native String jniGetSubscriptionExpression();
    private native int jniGetExpressionParameters(DDS.StringSeqHolder expression_parameters);
    private native int jniSetExpressionParameters(String[] expression_parameters);
}
