/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.cm.status;

/**
 * Represents the reason, why a Sample was rejected. 
 * 
 * @date Oct 12, 2004 
 */
public class SampleRejectedKind {
    private static final int _NOT_REJECTED                           = 0;
    private static final int _REJECTED_BY_INSTANCES_LIMIT            = 1;
    private static final int _REJECTED_BY_SAMPLES_LIMIT              = 2;
    private static final int _REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = 3;
    
    public static final SampleRejectedKind NOT_REJECTED =
        new SampleRejectedKind(_NOT_REJECTED);
    
    /**
     * Sample was rejected because max number of instances is reached.
     */
    public static final SampleRejectedKind REJECTED_BY_INSTANCES_LIMIT = 
                    new SampleRejectedKind(_REJECTED_BY_INSTANCES_LIMIT);
    
    /**
     * Sample was rejected because max number of samples is reached.
     */
    public static final SampleRejectedKind REJECTED_BY_SAMPLES_LIMIT =
                    new SampleRejectedKind(_REJECTED_BY_SAMPLES_LIMIT);
    
    public static final SampleRejectedKind REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT =
        new SampleRejectedKind(_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT);
    
    private SampleRejectedKind(int kind){}
    
    public static SampleRejectedKind from_int(int i){
        SampleRejectedKind kind = null;
        
        switch(i){
        case _NOT_REJECTED:
            kind = NOT_REJECTED;
            break;
        case _REJECTED_BY_INSTANCES_LIMIT:
            kind = REJECTED_BY_INSTANCES_LIMIT;
            break;
        case _REJECTED_BY_SAMPLES_LIMIT:
            kind = REJECTED_BY_SAMPLES_LIMIT;
            break;
        case _REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            kind = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
            break;
        default:
            break;
        }
        return kind;
    }
    
    /**
     * Creates a SampleRejectedKind from its string representation.
     * 
     * @param s The string representation of the SampleRejectedKind.
     * @return The SampleRejectedKind that matches the supplied String or null
     *         if the string is invalid.
     */
    public static SampleRejectedKind from_string(String s){
        SampleRejectedKind kind = null;
            
        if("S_NOT_REJECTED".equals(s)){
            kind = NOT_REJECTED;
        } else if("S_REJECTED_BY_INSTANCES_LIMIT".equals(s)){
            kind = REJECTED_BY_INSTANCES_LIMIT;
        } else if("S_REJECTED_BY_SAMPLES_LIMIT".equals(s)){
            kind = REJECTED_BY_SAMPLES_LIMIT;
        } else if("S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT".equals(s)){
            kind = REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
        } 
        return kind;
    }
    
    /**
     * Creates a string representation of the SampleRejectedKind.
     * 
     * @param s The SampleRejectedKind to convert.
     * @return The string that matches the supplied SampleRejectedKind or null
     *         if null is supplied.
     */
    public static String get_string(SampleRejectedKind s){
        String result = null;
        
        if(s != null){
            if(NOT_REJECTED.equals(s)){
                result = "NOT_REJECTED";
            } else if(REJECTED_BY_INSTANCES_LIMIT.equals(s)){
                result = "REJECTED_BY_INSTANCES_LIMIT";
            } else if(REJECTED_BY_SAMPLES_LIMIT.equals(s)){
                result = "REJECTED_BY_SAMPLES_LIMIT";
            } else if(REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT.equals(s)){
                result = "REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT";
            }
        }
        return result;
    }
}
