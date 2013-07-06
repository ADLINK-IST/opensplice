/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
