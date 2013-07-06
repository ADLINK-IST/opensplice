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
package org.opensplice.cm.data;

/**
 * 
 * 
 * @date Mar 29, 2005 
 */
public class State {
    private int value;
    private String strState = null;
    
    public static final int ANY        = 0;
    public static final int WRITE      = (0x0001 << 0); /* 1 */
    public static final int NEW        = (0x0001 << 1); /* 2 */
    public static final int DISPOSED   = (0x0001 << 2); /* 4 */
    public static final int NOWRITERS  = (0x0001 << 3); /* 8 */
    public static final int INCOMPLETE = (0x0001 << 4); /* 16 */
    public static final int READ       = (0x0001 << 5); /* 32 */
    public static final int EMPTY      = (0x0001 << 6); /* 64 */
    public static final int LAZYREAD   = (0x0001 << 7); /* 128 */
    public static final int REGISTER   = (0x0001 << 8); /* 256 */
    public static final int UNREGISTER = (0x0001 << 9); /* 512 */
    public static final int VALIDDATA  = (0x0001 << 14);/* 16384 */
    
    public State(int state){
        this.setValue(state);
    }
    
    public void setValue(int state){
        this.value = state;
        strState = null;
    }
    
    public boolean test(int mask){
        return (((value) & (mask)) == mask);
    }
    
    public String toString(){
        if(strState == null){
            StringBuffer buf = new StringBuffer();
            
            if(value == 0){
                buf.append("ANY");
            } else {
                if((value & NEW) == NEW){
                    buf.append("NEW, ");
                } else {
                    buf.append("NOT_NEW, ");
                }
                if((value & READ) == READ){
                    buf.append("READ, ");
                } else {
                    buf.append("NOT_READ, ");
                }
                if((value & WRITE) == WRITE){
                    buf.append("WRITE, ");
                }
                if((value & DISPOSED) == DISPOSED){
                    buf.append("DISPOSED, ");
                }
                if((value & NOWRITERS) == NOWRITERS){
                    buf.append("NOWRITERS, ");
                }
                if((value & INCOMPLETE) == INCOMPLETE){
                    buf.append("INCOMPLETE, ");
                }
                if((value & EMPTY) == EMPTY){
                    buf.append("EMPTY, ");
                }
                if((value & LAZYREAD) == LAZYREAD){
                    buf.append("LAZYREAD, ");
                }
                if((value & REGISTER) == REGISTER){
                    buf.append("REGISTER, ");
                }
                if((value & UNREGISTER) == UNREGISTER){
                    buf.append("UNREGISTER, ");
                }
                buf.delete(buf.length()-2, buf.length()-1);
            }
            strState = buf.toString();
        }
        return strState;
    }
    
    /**
     * Provides access to state.
     * 
     * @return Returns the state.
     */
    public int getValue() {
        return value;
    }
}
