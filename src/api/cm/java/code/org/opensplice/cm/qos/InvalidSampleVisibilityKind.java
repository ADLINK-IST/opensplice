/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

public class InvalidSampleVisibilityKind {
    public static final int _NO_INVALID_SAMPLES           = 0;
    public static final int _MINIMUM_INVALID_SAMPLES      = 1;
    public static final int _ALL_INVALID_SAMPLES          = 2;

    public static final InvalidSampleVisibilityKind NO_INVALID_SAMPLES      = new InvalidSampleVisibilityKind(_NO_INVALID_SAMPLES);
    public static final InvalidSampleVisibilityKind MINIMUM_INVALID_SAMPLES = new InvalidSampleVisibilityKind(_MINIMUM_INVALID_SAMPLES);
    public static final InvalidSampleVisibilityKind ALL_INVALID_SAMPLES     = new InvalidSampleVisibilityKind(_ALL_INVALID_SAMPLES);

    /**
     * Resolves the integer representation of the kind.
     * 
     * @return The integer representation of the kind.
     */
    public int value(){
        int result = -1;
        
        if(this.equals(NO_INVALID_SAMPLES)){
            result = _NO_INVALID_SAMPLES;
        } else if(this.equals(MINIMUM_INVALID_SAMPLES)){
            result = _MINIMUM_INVALID_SAMPLES;
        } else if(this.equals(ALL_INVALID_SAMPLES)){
            result = _ALL_INVALID_SAMPLES;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its integer representation.
     * 
     * @return The created kind.
     */
    public static InvalidSampleVisibilityKind from_int(int value){
        InvalidSampleVisibilityKind result = null;
        
        if(value == _NO_INVALID_SAMPLES){
            result = NO_INVALID_SAMPLES;
        } else if(value == _MINIMUM_INVALID_SAMPLES){
            result = MINIMUM_INVALID_SAMPLES;
        } else if( value == _ALL_INVALID_SAMPLES){
            result = ALL_INVALID_SAMPLES;
        }
        return result;
    }
    
    /**
     * Constructs the kind from its string representation.
     * 
     * @return The created kind.
     */
    public static InvalidSampleVisibilityKind from_string(String value){
        InvalidSampleVisibilityKind result = null;
        
        if("V_VISIBILITY_NO_INVALID_SAMPLES".equals(value)){
            result = NO_INVALID_SAMPLES;
        } else if("V_VISIBILITY_MINIMUM_INVALID_SAMPLES".equals(value)){
            result = MINIMUM_INVALID_SAMPLES;
        } else if("V_VISIBILITY_ALL_INVALID_SAMPLES".equals(value)){
            result = ALL_INVALID_SAMPLES;
        } else if("NO_INVALID_SAMPLES".equals(value)){
            result = NO_INVALID_SAMPLES;
        } else if("MINIMUM_INVALID_SAMPLES".equals(value)){
            result = MINIMUM_INVALID_SAMPLES;
        } else if("ALL_INVALID_SAMPLES".equals(value)){
            result = ALL_INVALID_SAMPLES;
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
        
        if(this.equals(NO_INVALID_SAMPLES)){
            result = "NO_INVALID_SAMPLES";
        } else if(this.equals(MINIMUM_INVALID_SAMPLES)){
            result = "MINIMUM_INVALID_SAMPLES";
        } else if(this.equals(ALL_INVALID_SAMPLES)){
            result = "ALL_INVALID_SAMPLES";
        }
        return result;
    }
    
    public String toKernelString(){
        String result = "UNKNOWN";
        
        if(this.equals(NO_INVALID_SAMPLES)){
            result = "V_VISIBILITY_NO_INVALID_SAMPLES";
        } else if(this.equals(MINIMUM_INVALID_SAMPLES)){
            result = "V_VISIBILITY_MINIMUM_INVALID_SAMPLES";
        } else if(this.equals(ALL_INVALID_SAMPLES)){
            result = "V_VISIBILITY_ALL_INVALID_SAMPLES";
        }
        return result;
    }
    
    
    protected InvalidSampleVisibilityKind(int value){}
}
