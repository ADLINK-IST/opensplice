package org.opensplice.dds.dcps.policy;

/**@brief Indicates the level of reliability offered by the Service.
 * 
 * -RELIABLE
 * Specifies the Service will attempt to deliver all samples in its history. Missed
 * samples mqy be retried. In steady-state the middleware guarantees that all samples
 * will eventually be delivered.
 * -BEST_EFFORT
 * Indicates that it is acceptable to not retry propagation of any samples. Presumably new
 * samples for the samples are generated often enough that it is not necessary to resend or
 * acknowledge any samples. 
 */
public final class ReliabilityQosKind {
    public static final int _RELIABLE                   = 0;
    public static final int _BEST_EFFORT                = 1;

    public static final ReliabilityQosKind RELIABLE      = new ReliabilityQosKind(_RELIABLE);
    public static final ReliabilityQosKind BEST_EFFORT   = new ReliabilityQosKind(_BEST_EFFORT);

    protected ReliabilityQosKind(int rc){}

    public static ReliabilityQosKind from_int(int i){
        ReliabilityQosKind rc = null;
        switch(i){
        case _RELIABLE:
            rc = RELIABLE;
            break;
        case _BEST_EFFORT:
            rc = BEST_EFFORT;
            break; 
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(OwnershipQosKind rc){
        int result = 0;

        if(rc.equals(RELIABLE)){
            result = _RELIABLE;
        } else if(rc.equals(BEST_EFFORT)){
            result = _BEST_EFFORT;
        } 
        return result;
    }
}

