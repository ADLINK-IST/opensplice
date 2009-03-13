package org.opensplice.dds.dcps.policy;

/**@brief Specifies how the samples representing changes to to data instances are presented to
 * the subscribing application. 
 * 
 * -INSTANCE
 * Scope spans only a single instance. Indicates that changes to one instance need not be
 * coherent nor ordered with respect to changes to any other instance.
 * -TOPIC
 * Scope spans to all instances whitin the same DataWriter (or DataReader) but not
 * across instances in different DataWriters/DataReaders
 * -GROUP
 * Scope spans to all instances of a DataReader or DataWriter within the same Publisher or
 * Subscriber.
 */
public class PresentationQosAccessScopeKind {
    public static final int _INSTANCE                           = 0;
    public static final int _TOPIC                              = 1;
    public static final int _GROUP                              = 2;

    public static final PresentationQosAccessScopeKind INSTANCE = new PresentationQosAccessScopeKind(_INSTANCE);
    public static final PresentationQosAccessScopeKind TOPIC    = new PresentationQosAccessScopeKind(_TOPIC);
    public static final PresentationQosAccessScopeKind GROUP    = new PresentationQosAccessScopeKind(_GROUP);

    protected PresentationQosAccessScopeKind(int rc){}
    
    public static PresentationQosAccessScopeKind from_int(int i){
        PresentationQosAccessScopeKind rc = null;
        switch(i){
        case 0:
            rc = INSTANCE;
            break;
        case 1:
            rc = TOPIC;
            break; 
        case 2:
            rc = GROUP;
            break;
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(PresentationQosAccessScopeKind rc){
        int result = 0;

        if(rc.equals(INSTANCE)){
            result = _INSTANCE;
        } else if(rc.equals(TOPIC)){
            result = _TOPIC;
        } else if(rc.equals(GROUP)){
            result = _GROUP;
        }
        return result;
    }
}

