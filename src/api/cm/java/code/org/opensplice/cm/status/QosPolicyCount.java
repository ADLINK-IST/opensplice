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
package org.opensplice.cm.status;

/**
 * Only used in PublicationMatchInfo and SubscriptionMatchInfo, which are 
 * deprecated.
 * 
 * @date Oct 12, 2004 
 */
public class QosPolicyCount {
    long policy_id;
    long count;
    
    public QosPolicyCount(long _policy_id, long _count){
        policy_id = _policy_id;
        count = _count;
    }
    /**
     * Provides access to policy_id.
     * 
     * @return Returns the policy_id.
     */
    public long getPolicyId() {
        return policy_id;
    }
    /**
     * Provides access to count.
     * 
     * @return Returns the count.
     */
    public long getCount() {
        return count;
    }
}
