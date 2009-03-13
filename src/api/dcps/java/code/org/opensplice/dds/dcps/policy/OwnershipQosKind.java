package org.opensplice.dds.dcps.policy;

/**@brief Specifies whether it is allowed for multiple DataWriter entities to write
 * the same instance of data.
 * 
 * -SHARED
 *  Indicates ownership for each instance. Multiple DataWriters are allowed to
 *  update the same instance and all updates are made available to the readers.
 * -EXCLUSIVE
 * Indicates each isntance can only be owned by one DataWriter, but the owner of an instance
 * can change dynamically.
 */
public final class OwnershipQosKind {
    public static final int _SHARED                 = 0;
    public static final int _EXCLUSIVE              = 1;

    public static final OwnershipQosKind SHARED     = new OwnershipQosKind(_SHARED);
    public static final OwnershipQosKind EXCLUSIVE  = new OwnershipQosKind(_EXCLUSIVE);

    protected OwnershipQosKind(int rc){}
    
    public static OwnershipQosKind from_int(int i){
        OwnershipQosKind rc = null;
        switch(i){
        case 0:
            rc = SHARED;
            break;
        case 1:
            rc = EXCLUSIVE;
            break; 
        default:
            rc = null;
            break;
        }
        return rc;
    }

    public int value(OwnershipQosKind rc){
        int result = 0;

        if(rc.equals(SHARED)){
            result = _SHARED;
        } else if(rc.equals(EXCLUSIVE)){
            result = _EXCLUSIVE;
        } 
        return result;
    }
}
