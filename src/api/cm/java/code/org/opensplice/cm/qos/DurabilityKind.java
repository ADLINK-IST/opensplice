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
 * This kind expresses if the data should 'outlive' their writing time.
 * 
 * @date Jan 10, 2005 
 */
public class DurabilityKind {
    public static final int _VOLATILE                   = 0;
    public static final int _TRANSIENT_LOCAL            = 1;
    public static final int _TRANSIENT                  = 2;
    public static final int _PERSISTENT                 = 3;
    
    /**
     * The Service does not need to keep any samples of data-instances on behalf 
     * of any DataReader that is not known by the Writer at the time the 
     * instance is written. In other words the Service will only attempt to 
     * provide the data to existing subscribers. This is the default kind.
     */
    public static final DurabilityKind VOLATILE         = new DurabilityKind(_VOLATILE);
    
    /**
     * The Service will attempt to keep some samples so that they can be 
     * delivered to any potential late-joining DataReader. Which particular 
     * samples are kept depends on other QoS such as HISTORY and 
     * RESOURCE_LIMITS. For TRANSIENT_LOCAL, the service is only required to 
     * keep the data in the memory of the Writer that wrote the data and the 
     * data is not required to survive the Writer.
     */
    public static final DurabilityKind TRANSIENT_LOCAL  = new DurabilityKind(_TRANSIENT_LOCAL);
    
    /**
     * The Service will attempt to keep some samples so that they can be 
     * delivered to any potential late-joining DataReader. Which particular 
     * samples are kept depends on other QoS such as HISTORY and 
     * RESOURCE_LIMITS. For TRANSIENT, the service is only required to keep the 
     * data in memory and not in permanent storage; but the data is not tied to 
     * the lifecycle of the DataWriter and will, in general, survive it.
     */
    public static final DurabilityKind TRANSIENT        = new DurabilityKind(_TRANSIENT);
    
    /**
     * Data is kept on permanent storage, so that they can outlive a system
     * session.
     */
    public static final DurabilityKind PERSISTENT       = new DurabilityKind(_PERSISTENT);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(VOLATILE)){
            result = _VOLATILE;
        } else if(this.equals(TRANSIENT_LOCAL)){
            result = _TRANSIENT_LOCAL;
        } else if(this.equals(TRANSIENT)){
            result = _TRANSIENT;
        } else if(this.equals(PERSISTENT)){
            result = _PERSISTENT;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static DurabilityKind from_int(int value){
        DurabilityKind result = null;
        
        if(value == _VOLATILE){
            result = VOLATILE;
        } else if(value == _TRANSIENT_LOCAL){
            result = TRANSIENT_LOCAL;
        } else if( value == _TRANSIENT){
            result = TRANSIENT;
        } else if( value == _PERSISTENT){
            result = PERSISTENT;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static DurabilityKind from_string(String value){
        DurabilityKind result = null;
        
        if("V_DURABILITY_VOLATILE".equals(value)){
            result = VOLATILE;
        } else if("V_DURABILITY_TRANSIENT_LOCAL".equals(value)){
            result = TRANSIENT_LOCAL;
        } else if("V_DURABILITY_TRANSIENT".equals(value)){
            result = TRANSIENT;
        } else if("V_DURABILITY_PERSISTENT".equals(value)){
            result = PERSISTENT;
        } else if("VOLATILE".equals(value)){
            result = VOLATILE;
        } else if("TRANSIENT_LOCAL".equals(value)){
            result = TRANSIENT_LOCAL;
        } else if("TRANSIENT".equals(value)){
            result = TRANSIENT;
        } else if("PERSISTENT".equals(value)){
            result = PERSISTENT;
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
        
        if(this.equals(VOLATILE)){
            result = "VOLATILE";
        } else if(this.equals(TRANSIENT_LOCAL)){
            result = "TRANSIENT_LOCAL";
        } else if(this.equals(TRANSIENT)){
            result = "TRANSIENT";
        } else if(this.equals(PERSISTENT)){
            result = "PERSISTENT";
        }
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(VOLATILE)){
            result = "V_DURABILITY_VOLATILE";
        } else if(this.equals(TRANSIENT_LOCAL)){
            result = "V_DURABILITY_TRANSIENT_LOCAL";
        } else if(this.equals(TRANSIENT)){
            result = "V_DURABILITY_TRANSIENT";
        } else if(this.equals(PERSISTENT)){
            result = "V_DURABILITY_PERSISTENT";
        }
        return result;
    }
    
    protected DurabilityKind(int value){}
}
