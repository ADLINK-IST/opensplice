package org.opensplice.dds.dcps;


import java.util.*;

import org.opensplice.dds.dcps.jni.*;
import org.opensplice.dds.dcps.policy.*;

/**@class DomainParticipantFactory
 * @brief The sole purpose of this class is to allow the creation and destruction of
 * DomainParticipant objects. 
 * 
 * DomainParticipantFactory itself has no factory. It is a singleton object that can 
 * be accessed by means of the get_instance operation.
 */
public class DomainParticipantFactory extends Entity{
    private static DomainParticipantFactory dpf; /*!<Singleton instance of this class.*/
    private DCPSCommunicationHandler ch;         /*!<Handles communication to other components.*/
    private HashSet domainParticipants;          /*!<Contains all DomainParticipants.*/
    private HashSet domains;                     /*!<Contains all Domains.*/
    
    /**@brief Creates a new DomainParticipantFactory.
     * 
     * @param _ch The communication handler that handles the actual communcation
     * with the Splice2v3 system.
     * 
     * @exception UnsatisfiedLinkError Exception that will be thrown if the communication
     * handler cannot load its libraries.
     * @exception Exception Will be thrown if any other exception occurs when loading the
     * communication handler.
     * 
     * @see org.opensplice.dds.dcps.jni.DCPSCommunicationHandler
     */
    private DomainParticipantFactory(String _ch) throws UnsatisfiedLinkError, Exception{
        try{
            if(_ch.equals("jni")){
            }
            ch = new DCPSJNIHandler();
        }
        catch(UnsatisfiedLinkError ule){
            throw new UnsatisfiedLinkError("Could not load dcpsjni library.");
        }
        ReturnCode rc = ReturnCode.from_user_result(ch.init_participant_factory());
        
        if(!rc.equals(ReturnCode.OK)){
            throw new Exception("DomainParticipantFactory could not be initialized.");
        }
        
        domainParticipants = new HashSet();
        domains = new HashSet();
    }
    
    /**@brief This operation returns the DomainParticipantFactory singleton. 
     * 
     * This operation
     * is idempotent, that is, it can be called multiple times without side effects and
     * it will return the same DomainParticipantFactory.
     * 
     * @return The DomainParticipantFactory singleton.
     */
    public static DomainParticipantFactory get_instance() {
        if(dpf == null){
            try {
                dpf = new DomainParticipantFactory("jni");
            }
            catch(UnsatisfiedLinkError ule){
                return null;
            }
            catch (Exception e) {
                return null;
            }
        }
        return dpf;
    }
    
    /**@brief Explicitly deletes the DomainParticipantFactory. 
     * 
     * @return ReturnCode.OK if succeeded. If the operation fails it will return any other
     * ReturnCode.
     */
    public ReturnCode delete(){
        ReturnCode rc;

        if(domainParticipants.size() != 0){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        else{
            rc = ReturnCode.from_user_result(ch.delete_participant_factory());
            if(rc.equals(ReturnCode.OK)){
                dpf = null;
            }
        }
        return rc;
    }
        
    /**@brief This operation creates a new DomainParticipant object.
     * The DomainParticipant signifies that the calling application intends to join the Domain 
     * identified by the _domainId argument.
     * 
     * @param _domainId The domainId of the Domain to participate in.
     * @param _qos The desired QoS policies.
     * @param _listener The listener that will be attached to the DomainParticipant.
     * 
     * @return The newly created DomainParticipant. In case of failure the operation
     * will return null.
     */
    public DomainParticipant create_participant(long _domainId, DomainParticipantQos _qos, DomainParticipantListener _listener){
        DomainParticipant part = ch.create_participant(this, _domainId, _qos, _listener);
        if(part != null){
            domainParticipants.add(part);
        }
        return part;
    }
    
    /**@brief This operation deletes an existing DomainParticipant.
     * 
     * This operation can only be invoked if all domain entities belonging to the 
     * participant have already been deleted. Otherwise ReturnCode.PRECONDITION_NOT_MET will be 
     * returned.
     * 
     * @param dp The DomainParticipant to delete.
     * 
     * @return ReturnCode.OK if succeeded, ReturnCode.PRECONDITION_NOT_MET otherwise.
     */
    public ReturnCode delete_participant(DomainParticipant dp){
        ReturnCode rc;
        if(dp == null){
            rc = ReturnCode.BAD_PARAMETER;
        }
        else if(!(domainParticipants.contains(dp))){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        else if(dp.get_publishers().size() != 0 ||
                dp.get_subscribers().size() != 0 ||
                dp.get_topics().size() != 0){
            rc = ReturnCode.PRE_CONDITION_NOT_MET;
        }
        else{
            rc =  ReturnCode.from_user_result(ch.delete_participant(dp));
        
            if(rc.equals(ReturnCode.OK)){
                domainParticipants.remove(dp);    
            }
        }
        return rc;
    }
    
    /**@brief This operation gives access to the current communication handler of the 
     * DomainParticipantFactory.
     * 
     * @return The DCPSCommunicationHandler that is currently used.
     */
    public DCPSCommunicationHandler getCommunicationHandler(){
        return ch;
    }
    
    /**@brief This operation changes the communication to the specified handler in the
     * _ch argument.
     * 
     * @param _ch The new communication handler.
     * 
     * @return true if succeeded, false otherwise.
     */
    public boolean setCommunicationHandler(DCPSCommunicationHandler _ch){
        if(_ch != null){
            ch = _ch;
            return true;
        }
        return false;
    }
}

