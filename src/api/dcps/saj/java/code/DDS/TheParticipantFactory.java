
package DDS;

public interface TheParticipantFactory
{
  public static final DDS.DomainParticipantFactory value = DDS.DomainParticipantFactory.get_instance();
}
