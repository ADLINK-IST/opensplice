/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
    
    @Override
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
