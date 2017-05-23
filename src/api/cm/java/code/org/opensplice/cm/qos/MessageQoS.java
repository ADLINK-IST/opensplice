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

/**
 * Represents the set of policies that apply to a Message.
 *
 * @date Mar 29, 2005
 */
public class MessageQoS {
    private static final int MQ_BYTE0_RELIABILITY_OFFSET     = 0;
    private static final int MQ_BYTE0_RELIABILITY_MASK       = (0x01);

    private final int[] value;
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
            int rel = rshift((value[0] & MQ_BYTE0_RELIABILITY_MASK),
                    MQ_BYTE0_RELIABILITY_OFFSET);

            if(rel == 0){
                reliability = ReliabilityKind.from_int(ReliabilityKind._BESTEFFORT);
            } else {
                reliability = ReliabilityKind.from_int(ReliabilityKind._RELIABLE);
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
