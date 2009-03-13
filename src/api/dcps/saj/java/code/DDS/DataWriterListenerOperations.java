
package DDS;


public interface DataWriterListenerOperations  extends DDS.ListenerOperations
{
  void on_offered_deadline_missed (DDS.DataWriter writer, DDS.OfferedDeadlineMissedStatus status);
  void on_offered_incompatible_qos (DDS.DataWriter writer, DDS.OfferedIncompatibleQosStatus status);
  void on_liveliness_lost (DDS.DataWriter writer, DDS.LivelinessLostStatus status);
  void on_publication_matched (DDS.DataWriter writer, DDS.PublicationMatchedStatus status);
} // interface DataWriterListenerOperations
