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
 * Represents the set of policies that apply to a Participant Entity. It 
 * consists of:
 * - EntityFactoryPolicy
 * - UserDataPolicy
 * 
 * @date Jan 10, 2005 
 */
public class ParticipantQoS extends QoS {
    /**
     * The ENTITY_FACTORY policy.
     */
    private EntityFactoryPolicy entityFactory;
    
    /**
     * The USER_DATA policy.
     */
    private UserDataPolicy userData;
    
    /**
     * The WATCHDOG_SCHEDULING policy.
     */
    private SchedulePolicy watchdogScheduling;
    
    /**
     * Constructs a new QoS that can be applied to a Participant.
     *
     * @param _entityFactory The ENTITY_FACTORY policy.
     * @param _userData The USER_DATA policy.
     */
    public ParticipantQoS(EntityFactoryPolicy _entityFactory, UserDataPolicy _userData, SchedulePolicy _watchdogScheduling){
        entityFactory = _entityFactory;
        userData = _userData;
        watchdogScheduling = _watchdogScheduling;
    }
    
    public static ParticipantQoS getDefault(){
        return new ParticipantQoS(EntityFactoryPolicy.DEFAULT, 
                UserDataPolicy.DEFAULT, SchedulePolicy.DEFAULT).copy();
    }
    /**
     * Provides access to entityFactory.
     * 
     * @return Returns the entityFactory.
     */
    public EntityFactoryPolicy getEntityFactory() {
        return entityFactory;
    }
    /**
     * Sets the entityFactory to the supplied value.
     *
     * @param entityFactory The entityFactory to set.
     */
    public void setEntityFactoryPolicy(EntityFactoryPolicy entityFactory) {
        this.entityFactory = entityFactory;
    }
    /**
     * Provides access to userData.
     * 
     * @return Returns the userData.
     */
    public UserDataPolicy getUserData() {
        return userData;
    }
    /**
     * Sets the userData to the supplied value.
     *
     * @param userData The userData to set.
     */
    public void setUserData(UserDataPolicy userData) {
        this.userData = userData;
    }

    public SchedulePolicy getWatchdogScheduling() {
        return this.watchdogScheduling;
    }

    public void setWatchdogScheduling(SchedulePolicy watchdogScheduling) {
        this.watchdogScheduling = watchdogScheduling;
    }
    
    public ParticipantQoS copy(){
        return new ParticipantQoS(
                this.entityFactory.copy(),
                this.userData.copy(),
                this.watchdogScheduling.copy());
    }
}
