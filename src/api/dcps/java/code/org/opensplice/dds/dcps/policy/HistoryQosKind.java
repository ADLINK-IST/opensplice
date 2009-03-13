package org.opensplice.dds.dcps.policy;

/**@brief Specifies the behaviour of the Service in the case where the value of a sample changes.
 * 
 * -KEEP_LAST
 *  On the publishing side, the Service will only attempt to keep the most recent
 *  "depth" samples of each instance of data managed by the DataWriter. On the Subscribing
 *  side, the DataReader will only attempt to to keep the most recent "depth" samples
 *  received from each instance until the application 'takes' them.  
 * -KEEP_ALL
 *  On the publishing side, the Service will only attempt to keep all
 *  samples of each instance of data managed by the DataWriter until they can be 
 *  delivered to each Subscriber. On the Subscribing side, the DataReader will attempt 
 *  to keep all samples of each instance of data managed by the DataReader. These samples
 *  are kept until the application 'takes' them.
 */
public class HistoryQosKind {
    public static final int _KEEP_LAST              = 0;
    public static final int _KEEP_ALL               = 1;

    public static final HistoryQosKind KEEP_LAST    = new HistoryQosKind(_KEEP_LAST);
    public static final HistoryQosKind KEEP_ALL     = new HistoryQosKind(_KEEP_ALL);

    protected HistoryQosKind(int rc){}

    public static HistoryQosKind from_int(int i){
        HistoryQosKind rc = null;
        switch(i){
        case _KEEP_LAST:
            rc = KEEP_LAST;
            break;
        case _KEEP_ALL:
            rc = KEEP_ALL;
            break; 
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(HistoryQosKind rc){
        int result = 0;

        if(rc.equals(KEEP_LAST)){
            result = _KEEP_LAST;
        } else if(rc.equals(KEEP_ALL)){
            result = _KEEP_ALL;
        } 
        return result;
    }
}

