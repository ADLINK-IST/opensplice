
package DDS;


public final class PublisherQos 
{
  public DDS.PresentationQosPolicy presentation = null;
  public DDS.PartitionQosPolicy partition = null;
  public DDS.GroupDataQosPolicy group_data = null;
  public DDS.EntityFactoryQosPolicy entity_factory = null;

  public PublisherQos ()
  {
  } // ctor

  public PublisherQos (DDS.PresentationQosPolicy _presentation, DDS.PartitionQosPolicy _partition, DDS.GroupDataQosPolicy _group_data, DDS.EntityFactoryQosPolicy _entity_factory)
  {
    presentation = _presentation;
    partition = _partition;
    group_data = _group_data;
    entity_factory = _entity_factory;
  } // ctor

} // class PublisherQos
