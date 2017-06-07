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
package org.opensplice.dds.core.status;

public enum StatusMask {
    NONE                                ((int)(0x0L)),
    ANY                                 ((int)(0x7FE7L)),
    ANY_V1_2                            ((int)(0x7FE7L)),
    INCONSISTENT_TOPIC_STATUS           ((int)(0x0001L << 0L)),
    OFFERED_DEADLINE_MISSED_STATUS      ((int)(0x0001L << 1L)),
    REQUESTED_DEADLINE_MISSED_STATUS    ((int)(0x0001L << 2L)),
    OFFERED_INCOMPATIBLE_QOS_STATUS     ((int)(0x0001L << 5L)),
    REQUESTED_INCOMPATIBLE_QOS_STATUS   ((int)(0x0001L << 6L)),
    SAMPLE_LOST_STATUS                  ((int)(0x0001L << 7L)),
    SAMPLE_REJECTED_STATUS              ((int)(0x0001L << 8L)),
    DATA_ON_READERS_STATUS              ((int)(0x0001L << 9L)),
    DATA_AVAILABLE_STATUS               ((int)(0x0001L << 10L)),
    LIVELINESS_LOST_STATUS              ((int)(0x0001L << 11L)),
    LIVELINESS_CHANGED_STATUS           ((int)(0x0001L << 12L)),
    PUBLICATION_MATCHED_STATUS          ((int)(0x0001L << 13L)),
    SUBSCRIPTION_MATCHED_STATUS         ((int)(0x0001L << 14L)),
    ALL_DATA_DISPOSED_TOPIC_STATUS      ((int)(0x0001L << 31L));
    
    private final int mask;
    
    StatusMask(int mask){
        this.mask = mask;
    }
    
    public int getMask(){
        return this.mask;
    }
}
