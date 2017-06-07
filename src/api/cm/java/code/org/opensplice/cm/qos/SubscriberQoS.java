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
 * Represents the set of policies that apply to a Subscriber Entity. It 
 * consists of:
 * - PresentationPolicy
 * - Partition
 * - GroupDataPolicy
 * - EntityFactoryPolicy
 * 
 * @date Jan 10, 2005 
 */
public class SubscriberQoS extends QoS {
    /**
     * The PRESENTATION policy.
     */
    private PresentationPolicy presentation;
    
    /**
     * The PARTITION policy.
     */
    private String partition;
    
    /**
     * The GROUP_DATA policy.
     */
    private GroupDataPolicy groupData;
    
    /**
     * The ENTITY_FACTORY policy.
     */
    private EntityFactoryPolicy entityFactory;
    
    private SharePolicy share;
    /**
     * Constructs a new SubscriberQoS that can be applied to a Subscriber.
     *
     * @param _presentation The PRESENTATION policy.
     * @param _partition The PARTITION policy.
     * @param _groupData The GROUP_DATA policy.
     * @param _entityFactory The ENTITY_FACTORY policy.
     */
    public SubscriberQoS(
            PresentationPolicy _presentation,
            String _partition,
            GroupDataPolicy _groupData,
            EntityFactoryPolicy _entityFactory,
            SharePolicy _share)
    {
        presentation = _presentation;
        partition = _partition;
        groupData = _groupData;
        entityFactory = _entityFactory;
        share = _share;
    }
    
    public static SubscriberQoS getDefault(){
        return new SubscriberQoS(
                        PresentationPolicy.DEFAULT, "", 
                        GroupDataPolicy.DEFAULT, EntityFactoryPolicy.DEFAULT,
                        SharePolicy.DEFAULT).copy();
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
    public void setEntityFactory(EntityFactoryPolicy entityFactory) {
        this.entityFactory = entityFactory;
    }
    /**
     * Provides access to groupData.
     * 
     * @return Returns the groupData.
     */
    public GroupDataPolicy getGroupData() {
        return groupData;
    }
    /**
     * Sets the groupData to the supplied value.
     *
     * @param groupData The groupData to set.
     */
    public void setGroupData(GroupDataPolicy groupData) {
        this.groupData = groupData;
    }
    /**
     * Provides access to partition.
     * 
     * @return Returns the partition.
     */
    public String getPartition() {
        return partition;
    }
    /**
     * Sets the partition to the supplied value.
     *
     * @param partition The partition to set.
     */
    public void setPartition(String partition) {
        this.partition = partition;
    }
    /**
     * Adds the supplied value to the partition list.
     *
     * @param partition The partition to add.
     */
    public void addPartition(String partition) {
        if (this.partition == null) {
            this.partition = partition;
        } else {
            this.partition += ","+partition;
        }
    }
    /**
     * Provides access to presentation.
     * 
     * @return Returns the presentation.
     */
    public PresentationPolicy getPresentation() {
        return presentation;
    }
    /**
     * Sets the presentation to the supplied value.
     *
     * @param presentation The presentation to set.
     */
    public void setPresentation(PresentationPolicy presentation) {
        this.presentation = presentation;
    }

    public SharePolicy getShare() {
        return this.share;
    }

    public void setShare(SharePolicy share) {
        this.share = share;
    }
    
    public SubscriberQoS copy(){
        String str;
        if(this.partition != null){
            str = new String(this.partition);
        } else {
            str = null;
        }
        return new SubscriberQoS(
                this.presentation.copy(), 
                str, 
                this.groupData.copy(), 
                this.entityFactory.copy(), 
                this.share.copy());
    }
}
