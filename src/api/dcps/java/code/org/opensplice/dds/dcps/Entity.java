package org.opensplice.dds.dcps;

import org.opensplice.dds.dcps.policy.*;

/**@brief Base object for all DCPS Entity objects.
 * 
 * @date Mar 17, 2004
 */
public class Entity {
    /**The list of QoS polices.
     */
    protected QosPolicy qos;        /*!<The attached QoS policies.*/
    protected DCPSListener listener;/*!<The attached DCPSListener object.*/
}
