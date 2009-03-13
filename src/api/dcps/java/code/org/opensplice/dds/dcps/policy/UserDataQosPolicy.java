package org.opensplice.dds.dcps.policy;

/**@brief User data not known by the middleware, but distributed by means of built-in topics
 * 
 * - Concerns:    DataWriter/DataReader/DomainParticipant
 * - RxO:         No
 * - Changable:   No
 */
public class UserDataQosPolicy {
    private String[] data;
    
    /**@brief Creates new UserDataQosPolicy.
     * 
     * @param _data A sequence of octets not known by the middleware.
     */
    public UserDataQosPolicy(String[] _data){
         data = _data;
    }
}

