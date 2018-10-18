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

package org.opensplice.cm;

public class Event {
    private int value;
    
    public static final int UNDEFINED            = (0x00000000);
    public static final int OBJECT_DESTROYED     = (0x00000001);
    public static final int INCONSISTENT_TOPIC   = (0x00000001 << 1);
    public static final int SAMPLE_REJECTED      = (0x00000001 << 2);
    public static final int SAMPLE_LOST          = (0x00000001 << 3);
    public static final int DEADLINE_MISSED      = (0x00000001 << 4);
    public static final int INCOMPATIBLE_QOS     = (0x00000001 << 5);
    public static final int LIVELINESS_ASSERT    = (0x00000001 << 6);
    public static final int LIVELINESS_CHANGED   = (0x00000001 << 7);
    public static final int LIVELINESS_LOST      = (0x00000001 << 8);
    public static final int TOPIC_MATCHED        = (0x00000001 << 9);
    public static final int SERVICES_CHANGES     = (0x00000001 << 10);
    public static final int NEW_GROUP            = (0x00000001 << 11);
    public static final int DATA_AVAILABLE       = (0x00000001 << 12);
    public static final int SERVICESTATE_CHANGED = (0x00000001 << 13);
    public static final int LEASE_RENEWED        = (0x00000001 << 14);
    public static final int LEASE_EXPIRED        = (0x00000001 << 15);
    public static final int TRIGGER              = (0x00000001 << 16);
    public static final int TIMEOUT              = (0x00000001 << 17);
    public static final int TERMINATE            = (0x00000001 << 18);
    public static final int HISTORY_DELETE       = (0x00000001 << 19);
    public static final int HISTORY_REQUEST      = (0x00000001 << 20);
    public static final int ALL                  = (0xffffffff);
    
    public Event(int event){
        this.setValue(event);
    }
    
    public void setValue(int event){
        this.value = event;
    }
    
    public int getValue() {
        return value;
    }
    
    public boolean test(int mask){
        return (((value) & (mask)) == mask);
    }
    
    @Override
    public String toString(){
        StringBuffer buf = new StringBuffer();
        
        if(value == ALL){
            buf.append("ALL");
        } else if(value == UNDEFINED){
            buf.append("UNDEFINED");
        } else {
            if((value & OBJECT_DESTROYED) == OBJECT_DESTROYED){
                buf.append("OBJECT_DESTROYED | ");
            }
            if((value & INCONSISTENT_TOPIC) == INCONSISTENT_TOPIC){
                buf.append("INCONSISTENT_TOPIC | ");
            }
            if((value & SAMPLE_REJECTED) == SAMPLE_REJECTED){
                buf.append("SAMPLE_REJECTED | ");
            }
            if((value & SAMPLE_LOST) == SAMPLE_LOST){
                buf.append("SAMPLE_LOST | ");
            }
            if((value & DEADLINE_MISSED) == DEADLINE_MISSED){
                buf.append("DEADLINE_MISSED | ");
            }
            if((value & INCOMPATIBLE_QOS) == INCOMPATIBLE_QOS){
                buf.append("INCOMPATIBLE_QOS | ");
            }
            if((value & LIVELINESS_ASSERT) == LIVELINESS_ASSERT){
                buf.append("LIVELINESS_ASSERT | ");
            }
            if((value & LIVELINESS_CHANGED) == LIVELINESS_CHANGED){
                buf.append("LIVELINESS_CHANGED | ");
            }
            if((value & LIVELINESS_LOST) == LIVELINESS_LOST){
                buf.append("LIVELINESS_LOST | ");
            }
            if((value & TOPIC_MATCHED) == TOPIC_MATCHED){
                buf.append("TOPIC_MATCHED | ");
            }
            if((value & SERVICES_CHANGES) == SERVICES_CHANGES){
                buf.append("SERVICES_CHANGES | ");
            }
            if((value & NEW_GROUP) == NEW_GROUP){
                buf.append("NEW_GROUP | ");
            }
            if((value & DATA_AVAILABLE) == DATA_AVAILABLE){
                buf.append("DATA_AVAILABLE | ");
            }
            if((value & SERVICESTATE_CHANGED) == SERVICESTATE_CHANGED){
                buf.append("SERVICESTATE_CHANGED | ");
            }
            if((value & LEASE_RENEWED) == LEASE_RENEWED){
                buf.append("LEASE_RENEWED | ");
            }
            if((value & LEASE_EXPIRED) == LEASE_EXPIRED){
                buf.append("LEASE_EXPIRED | ");
            }
            if((value & TRIGGER) == TRIGGER){
                buf.append("TRIGGER | ");
            }
            if((value & TIMEOUT) == TIMEOUT){
                buf.append("TIMEOUT | ");
            }
            if((value & TERMINATE) == TERMINATE){
                buf.append("TERMINATE | ");
            }
            if((value & HISTORY_DELETE) == HISTORY_DELETE){
                buf.append("HISTORY_DELETE | ");
            }
            if((value & HISTORY_REQUEST) == HISTORY_REQUEST){
                buf.append("HISTORY_REQUEST | ");
            }
            if(buf.length() > 2){
                buf.delete(buf.length()-2, buf.length()-1);
            }
        }
        return buf.toString();
      
    }
}
