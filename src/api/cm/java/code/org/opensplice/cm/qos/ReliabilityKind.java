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
    @Override
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
