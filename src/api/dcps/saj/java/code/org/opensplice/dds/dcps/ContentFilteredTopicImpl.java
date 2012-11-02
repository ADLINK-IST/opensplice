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
 * Implementation of the {@link DDS.ContentFilteredTopic} interface. 
 */ 
public class ContentFilteredTopicImpl extends TopicDescriptionImpl implements DDS.ContentFilteredTopic { 

    /* see DDS.ContentFilteredTopicOperations for javadoc */ 
    public String get_filter_expression () {
        return jniGetFilterExpression();
    }

    /* see DDS.ContentFilteredTopicOperations for javadoc */ 
    public int get_expression_parameters (DDS.StringSeqHolder expression_parameters) {
        return jniGetExpressionParameters(expression_parameters);
    }

    /* see DDS.ContentFilteredTopicOperations for javadoc */ 
    public int set_expression_parameters (String[] expression_parameters) {
        return jniSetExpressionParameters(expression_parameters);
    }

    /* see DDS.ContentFilteredTopicOperations for javadoc */ 
    public DDS.Topic get_related_topic () {
        return jniGetRelatedTopic();
    }
    
    private native String jniGetFilterExpression();
    private native int jniGetExpressionParameters(DDS.StringSeqHolder expression_parameters);
    private native int jniSetExpressionParameters(String[] expression_parameters);
    private native DDS.Topic jniGetRelatedTopic();
}
