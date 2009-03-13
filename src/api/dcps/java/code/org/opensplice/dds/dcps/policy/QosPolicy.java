package org.opensplice.dds.dcps.policy;

/**@brief Base class for a QoS policies.
 */
public abstract class QosPolicy {
    String name; //!<The name of the QoSPolicy
    
    /**@brief Gets the default policy.
     * 
     * @return The default policy
     */
    public static QosPolicy getDefault(){
        return null; 
    }
}

