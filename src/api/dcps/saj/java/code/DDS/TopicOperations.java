
package DDS;


public interface TopicOperations  extends DDS.EntityOperations, DDS.TopicDescriptionOperations
{

  // Access the status
  int get_inconsistent_topic_status (DDS.InconsistentTopicStatusHolder status);
  int get_qos (DDS.TopicQosHolder qos);
  int set_qos (DDS.TopicQos qos);
  DDS.TopicListener get_listener ();
  int set_listener (DDS.TopicListener a_listener, int mask);
} // interface TopicOperations
