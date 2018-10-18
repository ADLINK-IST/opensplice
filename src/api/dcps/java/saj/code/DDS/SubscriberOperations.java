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




public interface SubscriberOperations  extends DDS.EntityOperations
{
  DDS.DataReader create_datareader (DDS.TopicDescription a_topic, DDS.DataReaderQos qos, DDS.DataReaderListener a_listener, int mask);
  int delete_datareader (DDS.DataReader a_datareader);
  int delete_contained_entities ();
  DDS.DataReader lookup_datareader (String topic_name);
  int get_datareaders (DDS.DataReaderSeqHolder readers, int sample_states, int view_states, int instance_states);
  int notify_datareaders ();
  int set_qos (DDS.SubscriberQos qos);
  int get_qos (DDS.SubscriberQosHolder qos);
  int set_listener (DDS.SubscriberListener a_listener, int mask);
  DDS.SubscriberListener get_listener ();
  int begin_access ();
  int end_access ();
  DDS.DomainParticipant get_participant ();
  int set_default_datareader_qos (DDS.DataReaderQos qos);
  int get_default_datareader_qos (DDS.DataReaderQosHolder qos);
  int copy_from_topic_qos (DDS.DataReaderQosHolder a_datareader_qos, DDS.TopicQos a_topic_qos);
} // interface SubscriberOperations
