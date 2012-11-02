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
 * Implementation of the {@link DDS.DataReader} interface. 
 */ 
public class DataReaderViewImpl extends EntityImpl implements DDS.DataReaderView { 


    public DDS.ReadCondition create_readcondition (int sample_states, int view_states, int instance_states) {
        return jniCreateReadcondition(sample_states, view_states, instance_states);
    }

    public DDS.QueryCondition create_querycondition (int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters) {
        return jniCreateQuerycondition(sample_states, view_states, instance_states, query_expression, query_parameters);
    }

    public int delete_readcondition (DDS.ReadCondition a_condition) {
        return jniDeleteReadcondition(a_condition);
    }

    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    public int set_qos (DDS.DataReaderViewQos qos) {
        return jniSetQos(qos);
    }

    public int get_qos (DDS.DataReaderViewQosHolder qos) {
        return jniGetQos(qos);
    }

    public DDS.DataReader get_datareader () {
        return jniGetDataReader();
    }
    
    public DDS.StatusCondition get_statuscondition(){
        return jniGetStatusCondition();
    }
    
    public int get_status_changes(){
        return jniGetStatusChanges();
    }

    private native DDS.ReadCondition jniCreateReadcondition(int sample_states, int view_states, int instance_states);
    private native DDS.QueryCondition jniCreateQuerycondition(int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters);
    private native int jniDeleteReadcondition(DDS.ReadCondition a_condition);
    private native int jniDeleteContainedEntities();
    private native int jniSetQos(DDS.DataReaderViewQos qos);
    private native int jniGetQos(DDS.DataReaderViewQosHolder qos);
    private native DDS.DataReader jniGetDataReader();
    private native DDS.StatusCondition jniGetStatusCondition();
    private native int jniGetStatusChanges();
}
