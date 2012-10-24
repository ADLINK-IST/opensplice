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

package DDS;


public interface DataReaderViewOperations  extends DDS.EntityOperations
{

  //      in InstanceHandle_t handle);
  DDS.ReadCondition create_readcondition (int sample_states, int view_states, int instance_states);
  DDS.QueryCondition create_querycondition (int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters);
  int delete_readcondition (DDS.ReadCondition a_condition);
  int delete_contained_entities ();
  int set_qos (DDS.DataReaderViewQos qos);
  int get_qos (DDS.DataReaderViewQosHolder qos);
  DDS.StatusCondition get_statuscondition();
  int get_status_changes();
  DDS.DataReader get_datareader();
    
} // interface DataReaderViewOperations
