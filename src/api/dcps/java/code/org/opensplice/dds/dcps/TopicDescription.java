package org.opensplice.dds.dcps;

/**@brief This class is an abstract class. It is the base class for Topic,
 * ContentFilteredTopic and MultiTopic. 
 * 
 * TopicDescription represents the fact that both publications and subscribers are tied to a 
 * single data-type. Its attribute type_name defies a unique resulting type for the 
 * publication or the subscription and therefore creates an implicit association
 * with a DataType.
 */
public abstract class TopicDescription extends Entity {
    protected String name;                      //!<The name.
    protected String typeName;                  //!<The name of the type.
    protected DomainParticipant participant;    //!<The owning DomainParticipant.
    
    /**@brief Creates a TopicDescription. 
     * 
     * This will never be called, because it is an abstract class.
     * 
     * @param _name The name.
     * @param _typeName The name of the type.
     * @param _participant The participant that ownes the TopicDescription.
     */
    protected TopicDescription(String _name, String _typeName, DomainParticipant _participant){
        name        = _name;
        typeName    = _typeName;
        participant = _participant;
    }
    
    /**@brief Gives access to the DomainParticipant to which to TopicDescription belongs.
     * 
     * @return Returns the DomainParticipant to which the TopicDescription belongs.
     */
    public DomainParticipant get_participant(){
        return participant;
    }
    
    /**@brief Gives access to the name of the TopicDescription.
     * 
     * @return Returns the name used to create the TopicDescription.
     */
    public String get_name(){
        return name;
    }
    
    /**@brief Gives access to the type name of the TopicDescription.
     * 
     * @return Returns the type name used to create the TopicDescription.
     */
    public String get_type_name(){
        return typeName;
    }
}
