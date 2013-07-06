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
package org.opensplice.cm.qos;

/**
 * Determines the mechanism  used by the application to determine whether an 
 * Entity is "active" (alive).
 * 
 * @date Jan 10, 2005 
 */
public class LivelinessKind {
    public static final int _AUTOMATIC          = 0;
    public static final int _PARTICIPANT        = 1;
    public static final int _TOPIC              = 2;
    
    /**
     * The infrastructure will automatically signal liveliness for the writers 
     * at least as often as required by the lease_duration in the 
     * LivelinessPolicy.
     */
    public static final LivelinessKind AUTOMATIC      = new LivelinessKind(_AUTOMATIC);
    
    /**
     * The service will assume that as long as at least one Entity within the
     * Participant has asserted its liveliness the other Entities in that same
     * Participant are also alive.
     */
    public static final LivelinessKind PARTICIPANT    = new LivelinessKind(_PARTICIPANT);
    
    /**
     * The service will only assume liveliness of the Writer if the application 
     * has asserted liveliness of that Writer itself.
     */
    public static final LivelinessKind TOPIC          = new LivelinessKind(_TOPIC);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(AUTOMATIC)){
            result = _AUTOMATIC;
        } else if(this.equals(PARTICIPANT)){
            result = _PARTICIPANT;
        } else if(this.equals(TOPIC)){
            result = _TOPIC;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static LivelinessKind from_int(int value){
        LivelinessKind result = null;
        
        if(value == _AUTOMATIC){
            result = AUTOMATIC;
        } else if(value == _PARTICIPANT){
            result = PARTICIPANT;
        } else if( value == _TOPIC){
            result = TOPIC;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static LivelinessKind from_string(String value){
        LivelinessKind result = null;
        
        if("V_LIVELINESS_AUTOMATIC".equals(value)){
            result = AUTOMATIC;
        } else if("V_LIVELINESS_PARTICIPANT".equals(value)){
            result = PARTICIPANT;
        } else if("V_LIVELINESS_TOPIC".equals(value)){
            result = TOPIC;
        } else if("AUTOMATIC".equals(value)){
            result = AUTOMATIC;
        } else if("PARTICIPANT".equals(value)){
            result = PARTICIPANT;
        } else if("TOPIC".equals(value)){
            result = TOPIC;
        }
        
        return result;
    }
    
    /**
     * Resolves the string representation of the kind.
     * 
     * @return The string representation of the kind.
     */
    public String toString(){
        String result = "UNKNOWN";
        
        if(this.equals(AUTOMATIC)){
            result = "AUTOMATIC";
        } else if(this.equals(PARTICIPANT)){
            result = "PARTICIPANT";
        } else if(this.equals(TOPIC)){
            result = "TOPIC";
        }
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(AUTOMATIC)){
            result = "V_LIVELINESS_AUTOMATIC";
        } else if(this.equals(PARTICIPANT)){
            result = "V_LIVELINESS_PARTICIPANT";
        } else if(this.equals(TOPIC)){
            result = "V_LIVELINESS_TOPIC";
        }
        return result;
    }
    
    protected LivelinessKind(int value){}
    
    
}
