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




public interface DomainParticipantOperations  extends DDS.EntityOperations
{

  // Factory interfaces
  DDS.Publisher create_publisher (DDS.PublisherQos qos, DDS.PublisherListener a_listener, int mask);
  int delete_publisher (DDS.Publisher p);
  DDS.Subscriber create_subscriber (DDS.SubscriberQos qos, DDS.SubscriberListener a_listener, int mask);
  int delete_subscriber (DDS.Subscriber s);
  DDS.Subscriber get_builtin_subscriber ();
  DDS.Topic create_topic (String topic_name, String type_name, DDS.TopicQos qos, DDS.TopicListener a_listener, int mask);
  int delete_topic (DDS.Topic a_topic);
  DDS.Topic find_topic (String topic_name, DDS.Duration_t timeout);
  DDS.TopicDescription lookup_topicdescription (String name);
  DDS.ContentFilteredTopic create_contentfilteredtopic (String name, DDS.Topic related_topic, String filter_expression, String[] expression_parameters);
  int delete_contentfilteredtopic (DDS.ContentFilteredTopic a_contentfilteredtopic);
  DDS.MultiTopic create_multitopic (String name, String type_name, String subscription_expression, String[] expression_parameters);
  int delete_multitopic (DDS.MultiTopic a_multitopic);
  int delete_contained_entities ();
  int set_qos (DDS.DomainParticipantQos qos);
  int get_qos (DDS.DomainParticipantQosHolder qos);
  int set_listener (DDS.DomainParticipantListener a_listener, int mask);
  DDS.DomainParticipantListener get_listener ();
  int ignore_participant (long handle);
  int ignore_topic (long handle);
  int ignore_publication (long handle);
  int ignore_subscription (long handle);
  int get_domain_id ();
  int assert_liveliness ();
  int set_default_publisher_qos (DDS.PublisherQos qos);
  int get_default_publisher_qos (DDS.PublisherQosHolder qos);
  int set_default_subscriber_qos (DDS.SubscriberQos qos);
  int get_default_subscriber_qos (DDS.SubscriberQosHolder qos);
  int set_default_topic_qos (DDS.TopicQos qos);
  int get_default_topic_qos (DDS.TopicQosHolder qos);
  int get_discovered_participants (DDS.InstanceHandleSeqHolder participant_handles);
  int get_discovered_participant_data (DDS.ParticipantBuiltinTopicDataHolder participant_data, long handle);
  int get_discovered_topics (DDS.InstanceHandleSeqHolder topic_handles);
  int get_discovered_topic_data (DDS.TopicBuiltinTopicDataHolder topic_data, long handle);
  boolean contains_entity (long a_handle);
  int get_current_time (DDS.Time_tHolder current_time);
  int delete_historical_data (String partition_expression, String topic_expression);

} // interface DomainParticipantOperations
