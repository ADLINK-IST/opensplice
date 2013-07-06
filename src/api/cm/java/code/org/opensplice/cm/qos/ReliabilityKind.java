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
 * Indicates the level of reliability offered/requested by the service.
 * 
 * @date Jan 10, 2005 
 */
public class ReliabilityKind {
    public static final int _BESTEFFORT         = 0;
    public static final int _RELIABLE           = 1;
    
    /**
     * Indicates that it is acceptable to not retry propagation of any samples. 
     * Presumably new values for the samples are generated often enough that it 
     * is not necessary to resend or acknowledge any samples. This is the 
     * default value.
     */
    public static final ReliabilityKind BESTEFFORT    = new ReliabilityKind(_BESTEFFORT);
    
    /**
     * Specifies the service will attempt to deliver all samples in its history. 
     * Missed samples may be retried. In steady-state (no modifications 
     * communicated via the Writer) the middleware guarantees that all samples 
     * in the DataWriter history will eventually be delivered to the all 
     * DataReader objects. Outside steady state the HISTORY and RESOURCE_LIMITS 
     * policies will determine how samples become part of the history and 
     * whether samples can be discarded from it.
     */
    public static final ReliabilityKind RELIABLE      = new ReliabilityKind(_RELIABLE);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(BESTEFFORT)){
            result = _BESTEFFORT;
        } else if(this.equals(RELIABLE)){
            result = _RELIABLE;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static ReliabilityKind from_int(int value){
        ReliabilityKind result = null;
        
        if(value == _BESTEFFORT){
            result = BESTEFFORT;
        } else if(value == _RELIABLE){
            result = RELIABLE;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static ReliabilityKind from_string(String value){
        ReliabilityKind result = null;
        
        if("V_RELIABILITY_BESTEFFORT".equals(value)){
            result = BESTEFFORT;
        } else if("V_RELIABILITY_RELIABLE".equals(value)){
            result = RELIABLE;
        } else if("BESTEFFORT".equals(value)){
            result = BESTEFFORT;
        } else if("RELIABLE".equals(value)){
            result = RELIABLE;
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
        
        if(this.equals(BESTEFFORT)){
            result = "BESTEFFORT";
        } else if(this.equals(RELIABLE)){
            result = "RELIABLE";
        } 
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(BESTEFFORT)){
            result = "V_RELIABILITY_BESTEFFORT";
        } else if(this.equals(RELIABLE)){
            result = "V_RELIABILITY_RELIABLE";
        } 
        return result;
    }
    
    protected ReliabilityKind(int value){}
}
