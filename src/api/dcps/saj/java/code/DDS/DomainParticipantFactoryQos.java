
package DDS;


public final class DomainParticipantFactoryQos
{
  public DDS.EntityFactoryQosPolicy entity_factory = null;

  public DomainParticipantFactoryQos ()
  {
  } // ctor

  public DomainParticipantFactoryQos (DDS.EntityFactoryQosPolicy _entity_factory)
  {
    entity_factory = _entity_factory;
  } // ctor

} // class DomainParticipantQos

