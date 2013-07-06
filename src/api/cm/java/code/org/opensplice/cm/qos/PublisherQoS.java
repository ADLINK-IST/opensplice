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
 * Represents the set of policies that apply to a Publisher Entity. It 
 * consists of:
 * - PresentationPolicy
 * - Partition
 * - GroupDataPolicy
 * - EntityFactoryPolicy 
 * 
 * @date Jan 10, 2005 
 */
public class PublisherQoS extends QoS {
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
    
    /**
     * Constructs a new PublisherQoS that can be applied to a Publisher.
     *
     * @param _presentation The PRESENTATION policy.
     * @param _partition The PARTITION policy.
     * @param _groupData The GROUP_DATA policy.
     * @param _entityFactory The ENTITY_FACTORY policy.
     */
    public PublisherQoS(
            PresentationPolicy _presentation,
            String _partition,
            GroupDataPolicy _groupData,
            EntityFactoryPolicy _entityFactory)
    {
        presentation = _presentation;
        partition = _partition;
        groupData = _groupData;
        entityFactory = _entityFactory;
    }
    
    public static PublisherQoS getDefault(){
        return new PublisherQoS(PresentationPolicy.DEFAULT, "", 
                                GroupDataPolicy.DEFAULT, 
                                EntityFactoryPolicy.DEFAULT).copy();
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
    
    public PublisherQoS copy(){
        String str;
        if(this.partition != null){
            str = new String(this.partition);
        } else {
            str = null;
        }
        return new PublisherQoS(
                this.presentation.copy(),
                str,
                this.groupData.copy(),
                this.entityFactory.copy());
    }
}
