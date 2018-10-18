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


public interface TopicOperations  extends DDS.EntityOperations, DDS.TopicDescriptionOperations
{

  // Access the status
  int get_inconsistent_topic_status (DDS.InconsistentTopicStatusHolder status);
  int get_all_data_disposed_topic_status (DDS.AllDataDisposedTopicStatusHolder status);
  int get_qos (DDS.TopicQosHolder qos);
  int set_qos (DDS.TopicQos qos);
  DDS.TopicListener get_listener ();
  int set_listener (DDS.TopicListener a_listener, int mask);
  int dispose_all_data();
} // interface TopicOperations
