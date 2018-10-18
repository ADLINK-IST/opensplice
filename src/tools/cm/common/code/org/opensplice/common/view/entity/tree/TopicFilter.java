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

package org.opensplice.common.view.entity.tree;

import org.opensplice.cm.Entity;
import org.opensplice.common.util.Config;

public class TopicFilter {
    
    private static TopicFilter INSTANCE = null;
    
    private static final String DCPS = "DCPS";
    private static final String CM = "CM";
    private static final String d_ = "d_";
    private static final String q_ = "q_";
    private static final String rr_ = "rr_";
    
    //want the latest preferences for filter
    
    private boolean topic_filter_DCPS;
    private boolean topic_filter_CM;
    private boolean topic_filter_d;
    private boolean topic_filter_q;
    private boolean topic_filter_rr;
    

    private TopicFilter() {
        super();
    }

    public static TopicFilter getInstance(){
        if (INSTANCE == null){
            INSTANCE = new TopicFilter();
        }
        return INSTANCE;
    }
    
    public void refreshTopicFilters(){
        
        Config config = Config.getInstance();
        
        topic_filter_DCPS = new Boolean(config.getProperty("topic_filter_DCPS")).booleanValue();
        topic_filter_CM = new Boolean(config.getProperty("topic_filter_CM")).booleanValue();
        topic_filter_d = new Boolean(config.getProperty("topic_filter_d")).booleanValue();
        topic_filter_q = new Boolean(config.getProperty("topic_filter_q")).booleanValue();
        topic_filter_rr = new Boolean(config.getProperty("topic_filter_rr")).booleanValue();
        
    }


    public boolean isFilteredOutTopic(Entity topicEntity){
        
        String name = topicEntity.getName();
        
        if (topic_filter_DCPS){
            if (name.startsWith(DCPS)) {
                return true;
            }
        }
        
        if (topic_filter_CM){
            if (name.startsWith(CM)) {
                return true;
            }
        }
        
        if (topic_filter_d){
            if (name.startsWith(d_)){
                return true;
            }
        }
        
        if (topic_filter_q){
            if (name.startsWith(q_)){
                return true;
            }
        }
        
        if (topic_filter_rr){
            if (name.startsWith(rr_)){
                return true;
            }
        }
        
        return false;
        
    }

}
