
package DDS;


public interface TypeSupportOperations 
{
    int
    register_type(
        DDS.DomainParticipant domain,
        String type_name);
    String
    get_type_name();

} // interface TypeSupportOperations
