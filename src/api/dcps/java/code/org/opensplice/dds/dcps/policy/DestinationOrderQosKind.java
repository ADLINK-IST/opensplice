package org.opensplice.dds.dcps.policy;

/**@brief  This policy is meant to be able to control the logical order of instances.
 * 
 * -DestinationOrderQosKind.BY_SOURCE_TIMESTAMP:
 *  Indicates that data is ordered based on a timestamp placed at the source
 *  (by the Service or the application). In any case this guarantees a consistent
 *  final value for the data in all subscribers.
 * -DestinationOrderQosKind.BY_RECEPTION_TIME
 *  Indicates that data is ordered based on the reception time at each Subscriber.
 *  Since each Subscriber may receive the data at different times, there is no
 *  guarantee that changes will be seen in the same order. Consequently, it is possible
 *  for each subscriber to end up with a different final value for the data.
 */
public class DestinationOrderQosKind {
    public static final int _BY_RECEPTION_TIMESTAMP                     = 0;
    public static final int _BY_SOURCE_TIMESTAMP                        = 1;

    public static final DestinationOrderQosKind BY_RECEPTION_TIMESTAMP  = new DestinationOrderQosKind(_BY_RECEPTION_TIMESTAMP);
    public static final DestinationOrderQosKind BY_SOURCE_TIMESTAMP     = new DestinationOrderQosKind(_BY_SOURCE_TIMESTAMP);

    /**
     * @param rc DestinationOrderQosKind.BY_RECEPTION_TIME or
     *           DestinationOrderQosKind.BY_SOURCE_TIMESTAMP. 
     */
    protected DestinationOrderQosKind(int rc){}

    public static DestinationOrderQosKind from_int(int i){
        DestinationOrderQosKind rc = null;
        switch(i){
        case _BY_RECEPTION_TIMESTAMP:
            rc = BY_RECEPTION_TIMESTAMP;
            break;
        case _BY_SOURCE_TIMESTAMP:
            rc = BY_SOURCE_TIMESTAMP;
            break; 
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(DestinationOrderQosKind rc){
        int result = 0;

        if(rc.equals(BY_RECEPTION_TIMESTAMP)){
            result = _BY_RECEPTION_TIMESTAMP;
        } else if(rc.equals(BY_SOURCE_TIMESTAMP)){
            result = _BY_SOURCE_TIMESTAMP;
        } 
        return result;
    }
}
