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
