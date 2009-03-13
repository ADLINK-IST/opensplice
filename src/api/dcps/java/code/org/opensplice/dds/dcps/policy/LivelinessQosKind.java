package org.opensplice.dds.dcps.policy;

/**@brief Determines the mechanism and parameters used by the application to determine
 * whether an Entity is alive.
 * 
 * -AUTOMATIC
 *  The infrastructure will automaticly signal liveliness for the DataWriters at 
 *  least as often as required by a duration.
 * -MANUAL_BY_PARTICIPANT
 *  The user application takes responsability to signal liveliness to the Service.
 *  The Service will assume that as at least one Entity within the domain has asserted
 *  its liveliness, the Entity is also alive.
 * -MANUAL_BY_TOPIC
 *  The user application takes responsability to signal liveliness to the Service.
 *  The Service will only assume liveliness of the DataWriter if the application has
 *  asserted the liveliness of the DataWriter itself.
 */
public final class LivelinessQosKind {
    public static final int _AUTOMATIC                          = 0;
    public static final int _MANUAL_BY_PARTICIPANT              = 1;
    public static final int _MANUAL_BY_TOPIC                    = 2;

    public static final LivelinessQosKind AUTOMATIC             = new LivelinessQosKind(_AUTOMATIC);
    public static final LivelinessQosKind MANUAL_BY_PARTICIPANT = new LivelinessQosKind(_MANUAL_BY_PARTICIPANT);
    public static final LivelinessQosKind MANUAL_BY_TOPIC       = new LivelinessQosKind(_MANUAL_BY_TOPIC);

    protected LivelinessQosKind(int rc){}
    
    public static LivelinessQosKind from_int(int i){
        LivelinessQosKind rc = null;
        switch(i){
        case 0:
            rc = AUTOMATIC;
            break;
        case 1:
            rc = MANUAL_BY_PARTICIPANT;
            break; 
        case 2:
            rc = MANUAL_BY_TOPIC;
            break;
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(LivelinessQosKind rc){
        int result = 0;

        if(rc.equals(AUTOMATIC)){
            result = _AUTOMATIC;
        } else if(rc.equals(MANUAL_BY_PARTICIPANT)){
            result = _MANUAL_BY_PARTICIPANT;
        } else if(rc.equals(MANUAL_BY_TOPIC)){
            result = _MANUAL_BY_TOPIC;
        }
        return result;
    }
}

