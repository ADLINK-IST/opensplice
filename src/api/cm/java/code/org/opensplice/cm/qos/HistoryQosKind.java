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
 * Specifies the behavior of the Service in the case where the value of a sample
 * changes (one or more times) before it can be successfully communicated to one 
 * or more existing subscribers.
 * 
 * @date Jan 10, 2005 
 */
public class HistoryQosKind {
    public static final int _KEEPLAST         = 0;
    public static final int _KEEPALL           = 1;
    
    /**
     * On the publishing side, the service will only attempt to keep the most 
     * recent "depth" samples of each instance of data (identified by its key) 
     * managed by the Writer. On the subscribing side, the DataReader will only 
     * attempt to keep the most recent "depth" samples received for each 
     * instance (identified by its key) until the application "takes" them via 
     * the DataReader's take operation. KEEPLAST is the default kind. The 
     * default value of depth is 1. If a value other than 1 is specified, it
     * should be consistent with the settings of the ResourcePolicy.
     */
    public static final HistoryQosKind KEEPLAST    = new HistoryQosKind(_KEEPLAST);
    
    /**
     * On the publishing side, the service will attempt to keep all samples 
     * (representing each value written) of each instance of data (identified by 
     * its key) managed by the Writer until they can be delivered to all 
     * subscribers. On the subscribing side, the service will attempt to keep 
     * all samples of each instance of data (identified by its key) managed by 
     * the DataReader. These samples are kept until the application "takes" them 
     * from the service via the take operation. The setting of depth has no 
     * effect. Its implied value is "INFINITE"
     */
    public static final HistoryQosKind KEEPALL      = new HistoryQosKind(_KEEPALL);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(KEEPLAST)){
            result = _KEEPLAST;
        } else if(this.equals(KEEPALL)){
            result = _KEEPALL;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static HistoryQosKind from_int(int value){
        HistoryQosKind result = null;
        
        if(value == _KEEPLAST){
            result = KEEPLAST;
        } else if(value == _KEEPALL){
            result = KEEPALL;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static HistoryQosKind from_string(String value){
        HistoryQosKind result = null;
        
        if("V_HISTORY_KEEPLAST".equals(value)){
            result = KEEPLAST;
        } else if("V_HISTORY_KEEPALL".equals(value)){
            result = KEEPALL;
        } else if("KEEPLAST".equals(value)){
            result = KEEPLAST;
        } else if("KEEPALL".equals(value)){
            result = KEEPALL;
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
        
        if(this.equals(KEEPLAST)){
            result = "KEEPLAST";
        } else if(this.equals(KEEPALL)){
            result = "KEEPALL";
        } 
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(KEEPLAST)){
            result = "V_HISTORY_KEEPLAST";
        } else if(this.equals(KEEPALL)){
            result = "V_HISTORY_KEEPALL";
        } 
        return result;
    }
    
    protected HistoryQosKind(int value){}
}
