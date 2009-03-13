package org.opensplice.dds.dcps;

/**@brief DCPS ReturnCode object that is used to provide information about
 * the execution of several operations.
 * 
 * @date Mar 17, 2004
 *
 */
public final class ReturnCode {
    public static final int _OK                     = 0;
    public static final int _ERROR                  = 1;
    public static final int _BAD_PARAMETER          = 2;
    public static final int _UNSUPPORTED            = 3;
    public static final int _ALREADY_DELETED        = 4;
    public static final int _OUT_OF_RESOURCES       = 5;
    public static final int _NOT_ENABLED            = 6;
    public static final int _IMMUTABLE_POLICY       = 7;
    public static final int _INCONSISTENT_POLICY    = 8;
    public static final int _PRE_CONDITION_NOT_MET  = 9;
    
    public static final ReturnCode OK                       = new ReturnCode(_OK);
    public static final ReturnCode ERROR                    = new ReturnCode(_ERROR);
    public static final ReturnCode BAD_PARAMETER            = new ReturnCode(_BAD_PARAMETER);
    public static final ReturnCode UNSUPPORTED              = new ReturnCode(_UNSUPPORTED);
    public static final ReturnCode ALREADY_DELETED          = new ReturnCode(_ALREADY_DELETED);
    public static final ReturnCode OUT_OF_RESOURCES         = new ReturnCode(_OUT_OF_RESOURCES);
    public static final ReturnCode NOT_ENABLED              = new ReturnCode(_NOT_ENABLED);
    public static final ReturnCode IMMUTABLE_POLICY         = new ReturnCode(_IMMUTABLE_POLICY);
    public static final ReturnCode INCONSISTENT_POLICY      = new ReturnCode(_INCONSISTENT_POLICY);
    public static final ReturnCode PRE_CONDITION_NOT_MET    = new ReturnCode(_PRE_CONDITION_NOT_MET);
    
    /**@brief Constructs new ReturnCode.
     * 
     * @param rc The int representation of the ReturnCode.
     */
    protected ReturnCode(int rc){}
    
    /**@brief Does the same as the from_int(int i) operation.
     * 
     * @param i The int representation of the ReturnCode.
     * @return The newly created ReturnCode.
     */
    public static ReturnCode from_user_result(int i){
       return ReturnCode.from_int(i);
    }
    
    /**@brief Creates a ReturnCode from user layer int results.
     * 
     * @param i The int representation of the ReturnCode.
     * @return The newly created ReturnCode.
     */
    public static ReturnCode from_int(int i){
        ReturnCode rc = null;
        switch(i){
        case 0:
            rc = OK;
            break;
        case 1:
            rc = ERROR;
            break; 
        case 2:
            rc = BAD_PARAMETER;
            break;
        case 3:
            rc = UNSUPPORTED;
            break;
        case 4:
            rc = ALREADY_DELETED;
            break;
        case 5:
            rc = OUT_OF_RESOURCES;
            break; 
        case 6:
            rc = NOT_ENABLED;
            break;
        case 7:
            rc = IMMUTABLE_POLICY;
            break;   
        case 8:
            rc = INCONSISTENT_POLICY;
            break;
        case 9:
            rc = PRE_CONDITION_NOT_MET;
            break;  
        default:
            System.err.println("Unknown ReturnCode" + i);
            rc = null;
            break;
        }
        return rc;
    }
    
    /**@brief Returns the int representation of the ReturnCode.
     * 
     * @param rc The ReturnCode where to get the int representation of.
     * @return The int representation of the provided ReturnCode.
     */
    public static int value(ReturnCode rc){
        int result = 9;
        
        if(rc.equals(OK)){
            result = 0;
        } else if(rc.equals(ERROR)){
            result = _ERROR;
        } else if(rc.equals(BAD_PARAMETER)){
            result = _BAD_PARAMETER;
        } else if(rc.equals(UNSUPPORTED)){
            result = _UNSUPPORTED;
        } else if(rc.equals(ALREADY_DELETED)){
            result = _ALREADY_DELETED;
        } else if(rc.equals(OUT_OF_RESOURCES)){
            result = _OUT_OF_RESOURCES;
        } else if(rc.equals(NOT_ENABLED)){
            result = _NOT_ENABLED;
        } else if(rc.equals(IMMUTABLE_POLICY)){
            result = _IMMUTABLE_POLICY;
        } else if(rc.equals(INCONSISTENT_POLICY)){
            result = _INCONSISTENT_POLICY;
        } else if(rc.equals(PRE_CONDITION_NOT_MET)){
            result = _PRE_CONDITION_NOT_MET;
        } 
        return result;
    }
}

