package org.opensplice.dds.dcps.policy;

/**@brief QoS that specifies the behaviour of the time the data 'lives' in the system. 
 * 
 * This enumeration contains:
 * -VOLATILE
 *  The Service does not need to keep any samples of data-instances on behalf of any
 *  DataReader that is not known by the  DataWriterat the time the instance is written.
 *  In other words; the Service will only attempt to provide the data to existing Subscribers.
 * -TRANSIENT
 *  The Service will attempt to keep some samples so that they can be delivered to any
 *  potential late joining DataReader. Which particular samples are kept depend on other
 *  QoS like HistoryQosPolicy and ResourceLimitsQosPolicy. The Service is only required to
 *  keep the data in memory and not in permanent storage.
 * -PERSISTENT
 *  Data is kept on permanent storage, so that it can outlive a system session.
 */
public final class DurabilityQosKind {
    public static final int _VOLATILE                   = 0;
    public static final int _TRANSIENT                  = 1;
    public static final int _PERSISTENT                 = 2;

    public static final DurabilityQosKind VOLATILE      = new DurabilityQosKind(_VOLATILE);
    public static final DurabilityQosKind TRANSIENT     = new DurabilityQosKind(_TRANSIENT);
    public static final DurabilityQosKind PERSISTENT    = new DurabilityQosKind(_PERSISTENT);
    
    protected DurabilityQosKind(int rc){}
        
    public static DurabilityQosKind from_int(int i){
        DurabilityQosKind rc = null;
        switch(i){
        case 0:
            rc = VOLATILE;
            break;
        case 1:
            rc = TRANSIENT;
            break; 
        case 2:
            rc = PERSISTENT;
            break;
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(DurabilityQosKind rc){
        int result = 0;
    
        if(rc.equals(VOLATILE)){
            result = _VOLATILE;
        } else if(rc.equals(TRANSIENT)){
            result = _TRANSIENT;
        } else if(rc.equals(PERSISTENT)){
            result = _PERSISTENT;
        }
        return result;
    }
}
