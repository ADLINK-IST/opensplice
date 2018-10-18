/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
  DDS.DataReader get_datareader();
} // interface DataReaderViewOperations
