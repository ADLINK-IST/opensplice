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
 * 
 * 
 * @date Oct 12, 2004
 * @deprecated Use TopicMatchStatus instead. 
 */
@Deprecated
public class SubscriptionMatchInfo {
    private long total_count;
    private long total_count_change;
    private String last_subscription_handle;
    
    public SubscriptionMatchInfo(
            long _total_count, 
            long _total_count_change, 
            String _last_subscription_handle)
    {
        total_count = _total_count;
        total_count_change = _total_count_change;
        last_subscription_handle = _last_subscription_handle;
    }
    
    /**
     * Provides access to total_count.
     * 
     * @return Returns the total_count.
     */
    public long getTotalCount() {
        return total_count;
    }
    
    /**
     * Provides access to total_count_change.
     * 
     * @return Returns the total_count_change.
     */
    public long getTotalCountChange() {
        return total_count_change;
    }
    /**
     * Provides access to last_subscription_handle.
     * 
     * @return Returns the last_subscription_handle.
     */
    public String getLastSubscriptionHandle() {
        return last_subscription_handle;
    }
}
