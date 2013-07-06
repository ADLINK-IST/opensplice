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
 * Determines the largest scope spanning the entities for which the order and 
 * coherency of changes can be preserved.
 * 
 * @date Jan 10, 2005 
 */
public class PresentationKind {
    public static final int _INSTANCE           = 0;
    public static final int _TOPIC              = 1;
    public static final int _GROUP              = 2;
    
    /**
     * Scope spans only a single instance. Indicates that changes to one 
     * instance need not be coherent nor ordered with respect to changes to any 
     * other instance. In other words, order and coherent changes apply to each 
     * instance separately.
     */
    public static final PresentationKind INSTANCE     = new PresentationKind(_INSTANCE);
    
    /**
     * Scope spans to all instances within the same Writer (or DataReader), but 
     * not across instances in different Writer (or DataReader).
     */
    public static final PresentationKind TOPIC        = new PresentationKind(_TOPIC);
    
    /**
     * Scope spans to all instances belonging to Writer (or DataReader) entities 
     * within the same Publisher (or Subscriber).
     */
    public static final PresentationKind GROUP        = new PresentationKind(_GROUP);
    
    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(INSTANCE)){
            result = _INSTANCE;
        } else if(this.equals(TOPIC)){
            result = _TOPIC;
        } else if(this.equals(GROUP)){
            result = _GROUP;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static PresentationKind from_int(int value){
        PresentationKind result = null;
        
        if(value == _INSTANCE){
            result = INSTANCE;
        } else if(value == _TOPIC){
            result = TOPIC;
        } else if( value == _GROUP){
            result = GROUP;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static PresentationKind from_string(String value){
        PresentationKind result = null;
        
        if("V_PRESENTATION_INSTANCE".equals(value)){
            result = INSTANCE;
        } else if("V_PRESENTATION_TOPIC".equals(value)){
            result = TOPIC;
        } else if("V_PRESENTATION_GROUP".equals(value)){
            result = GROUP;
        } else if("INSTANCE".equals(value)){
            result = INSTANCE;
        } else if("TOPIC".equals(value)){
            result = TOPIC;
        } else if("GROUP".equals(value)){
            result = GROUP;
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
        
        if(this.equals(INSTANCE)){
            result = "INSTANCE";
        } else if(this.equals(TOPIC)){
            result = "TOPIC";
        } else if(this.equals(GROUP)){
            result = "GROUP";
        }
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(INSTANCE)){
            result = "V_PRESENTATION_INSTANCE";
        } else if(this.equals(TOPIC)){
            result = "V_PRESENTATION_TOPIC";
        } else if(this.equals(GROUP)){
            result = "V_PRESENTATION_GROUP";
        }
        return result;
    }
    
    
    protected PresentationKind(int value){}
}
