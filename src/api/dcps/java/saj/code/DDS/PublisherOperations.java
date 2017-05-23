/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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




public interface PublisherOperations  extends DDS.EntityOperations
{
  DDS.DataWriter create_datawriter (DDS.Topic a_topic, DDS.DataWriterQos qos, DDS.DataWriterListener a_listener, int mask);
  int delete_datawriter (DDS.DataWriter a_datawriter);
  DDS.DataWriter lookup_datawriter (String topic_name);
  int delete_contained_entities ();
  int set_qos (DDS.PublisherQos qos);
  int get_qos (DDS.PublisherQosHolder qos);
  int set_listener (DDS.PublisherListener a_listener, int mask);
  DDS.PublisherListener get_listener ();
  int suspend_publications ();
  int resume_publications ();
  int begin_coherent_changes ();
  int end_coherent_changes ();
  int wait_for_acknowledgments(DDS.Duration_t max_wait);
  DDS.DomainParticipant get_participant ();
  int set_default_datawriter_qos (DDS.DataWriterQos qos);
  int get_default_datawriter_qos (DDS.DataWriterQosHolder qos);
  int copy_from_topic_qos (DDS.DataWriterQosHolder a_datawriter_qos, DDS.TopicQos a_topic_qos);
} // interface PublisherOperations
