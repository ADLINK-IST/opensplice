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

import org.opensplice.cm.Time;
/**
 * Represents the set of policies that apply to a Message.
 * @date Mar 29, 2005 
 */
public class MessageQoS {
    private static final int MQ_BYTE0_RELIABILITY_OFFSET     = 0;
    private static final int MQ_BYTE0_OWNERSHIP_OFFSET       = 1;
    private static final int MQ_BYTE0_ORDERBY_OFFSET         = 2;
    private static final int MQ_BYTE0_AUTODISPOSE_OFFSET     = 3;
    
    private static final int MQ_BYTE0_LATENCY_OFFSET         = 4;
    private static final int MQ_BYTE0_DEADLINE_OFFSET        = 5;
    private static final int MQ_BYTE0_LIVELINESS_OFFSET      = 6;
    private static final int MQ_BYTE0_LIFESPAN_OFFSET        = 7;
    
    private static final int MQ_BYTE1_DURABILITY_OFFSET      = 0;
    private static final int MQ_BYTE1_LIVELINESS_OFFSET      = 2;
    private static final int MQ_BYTE1_PRESENTATION_OFFSET    = 4;
    private static final int MQ_BYTE1_ORDERED_ACCESS_OFFSET  = 6;
    private static final int MQ_BYTE1_COHERENT_ACCESS_OFFSET = 7;
    
    private static final int MQ_BYTE0_RELIABILITY_MASK       = (0x01);
    private static final int MQ_BYTE0_OWNERSHIP_MASK         = (0x02);
    private static final int MQ_BYTE0_ORDERBY_MASK           = (0x04);
    private static final int MQ_BYTE0_AUTODISPOSE_MASK       = (0x08);
    private static final int MQ_BYTE0_LATENCY_MASK           = (0x10);
    private static final int MQ_BYTE0_DEADLINE_MASK          = (0x20);
    private static final int MQ_BYTE0_LIVELINESS_MASK        = (0x40);
    private static final int MQ_BYTE0_LIFESPAN_MASK          = (0x80);
    private static final int MQ_BYTE1_DURABILITY_MASK        = (0x03);
    private static final int MQ_BYTE1_LIVELINESS_MASK        = (0x0c);
    private static final int MQ_BYTE1_PRESENTATION_MASK      = (0x30);
    private static final int MQ_BYTE1_ORDERED_ACCESS_MASK    = (0x40);
    private static final int MQ_BYTE1_COHERENT_ACCESS_MASK   = (0x80);
    
    private int[] value;
    private int curElement = 0;
    
    public MessageQoS(int[] value){
        this.value = new int[value.length];
        
        for(int i=0; i<value.length; i++){
            this.value[i] = value[i];
        }
        this.curElement = 0;
    }
    
    public int[] getValue(){
        int[] result = new int[this.value.length];
        
        for(int i=0; i<value.length; i++){
            result[i] = this.value[i];
        }
        return result;
    }
    
    public void addElement(int e){
        if(curElement < value.length){
            value[curElement++] = e;
        }
    }
    
    private int lshift(int value, int offset){
        return (value << offset);
    }
    
    private int rshift(int value, int offset){
        return (value >> offset);
    }

    /**
     * Provides access to reliability.
     * 
     * @return Returns the reliability.
     */
    public ReliabilityKind getReliabilityKind() {
        ReliabilityKind reliability;
        
        if(this.value.length > 0){
            int rel = rshift(
                    (value[0] & MQ_BYTE0_RELIABILITY_MASK), 
                        MQ_BYTE0_RELIABILITY_OFFSET);
        
            if(rel == 0){
               reliability = ReliabilityKind.from_int(
                                           ReliabilityKind._BESTEFFORT); 
            } else {
               reliability = ReliabilityKind.from_int(
                                            ReliabilityKind._RELIABLE);
            }
        } else {
            reliability = ReliabilityKind.from_int(ReliabilityKind._BESTEFFORT);
        }
        return reliability;
    }
    
    public MessageQoS copy(){
        MessageQoS qos = new MessageQoS(this.value);
        qos.curElement = this.curElement;
        
        return qos;
    }
}
