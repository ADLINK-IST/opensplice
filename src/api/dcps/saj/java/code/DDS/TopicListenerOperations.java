
package DDS;


public interface TopicListenerOperations  extends DDS.ListenerOperations
{
  void on_inconsistent_topic (DDS.Topic the_topic, DDS.InconsistentTopicStatus status);
} // interface TopicListenerOperations
