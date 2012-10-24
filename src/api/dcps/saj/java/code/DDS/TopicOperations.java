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
