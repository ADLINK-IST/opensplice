
package DDS;


public interface SubscriberListenerOperations  extends DDS.DataReaderListenerOperations
{
  void on_data_on_readers (DDS.Subscriber subs);
} // interface SubscriberListenerOperations
