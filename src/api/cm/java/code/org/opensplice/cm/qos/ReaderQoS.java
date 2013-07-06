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
 * Represents the set of policies that apply to a DataReader Entity. It 
 * consists of:
 * - DurabilityPolicy
 * - DeadlinePolicy
 * - LatencyPolicy
 * - LivelinessPolicy
 * - ReliabilityPolicy
 * - OrderbyPolicy
 * - HistoryPolicy
 * - ResourcePolicy
 * - UserDataPolicy
 * - OwnershipPolicy
 * - PacingPolicy
 * - ReaderLifecyclePolicy 
 * - ReaderLifespanPolicy
 * - SharePolicy
 * - UserKeyPolicy
 * 
 * @date Jan 10, 2005 
 */
public class ReaderQoS extends QoS {
    /**
     * The DURABILITY policy.
     */
    private DurabilityPolicy durability;
    
    /**
     * The DEADLINE policy.
     */
    private DeadlinePolicy deadline;
    
    /**
     * The LATENCY_BUDGET policy.
     */
    private LatencyPolicy latency;
    
    /**
     * The LIVELINESS policy.
     */
    private LivelinessPolicy liveliness;
    
    /**
     * The RELIABILITY policy.
     */
    private ReliabilityPolicy reliability;
    
    /**
     * The DESTINATION_ORDER policy.
     */
    private OrderbyPolicy orderby;
    
    /**
     * The HISTORY policy.
     */
    private HistoryPolicy history;
    
    /**
     * The RESOURCE_LIMITS policy.
     */
    private ResourcePolicy resource;
    
    /**
     * The USER_DATA policy.
     */
    private UserDataPolicy userData;
    
    private OwnershipPolicy ownership;
    
    /**
     * The TIME_BASED_FILTER policy.
     */
    private PacingPolicy pacing;
    
    /**
     * The READER_DATA_LIFECYCLE policy.
     */
    private ReaderLifecyclePolicy lifecycle;
    
    /**
     * The READER_DATA_LIFESPAN policy.
     */
    private ReaderLifespanPolicy lifespan;
    
    private SharePolicy share;
    
    private UserKeyPolicy userKey;
    
    /**
     * Constructs a new ReaderQoS.
     *  
     *
     * @param _durability The DURABILITY policy.
     * @param _deadline The DEADLINE policy.
     * @param _latency The LATENCY_BUDGET policy.
     * @param _liveliness The LIVELINESS policy.
     * @param _reliability The RELIABILITY policy.
     * @param _orderby The DESTINATION_ORDER policy.
     * @param _history The HISTORY policy.
     * @param _resource The RESOURCE_LIMITS policy.
     * @param _userData The USER_DATA policy.
     * @param _ownership The OWNERSHIP policy
     * @param _pacing The TIME_BASED_FILTER policy.
     * @param _lifecycle The READER_DATA_LIFECYCLE policy.
     */
    public ReaderQoS(
            DurabilityPolicy _durability,
            DeadlinePolicy _deadline,
            LatencyPolicy _latency,
            LivelinessPolicy _liveliness,
            ReliabilityPolicy _reliability,
            OrderbyPolicy _orderby,
            HistoryPolicy _history,
            ResourcePolicy _resource,
            UserDataPolicy _userData,
            OwnershipPolicy _ownership,
            PacingPolicy _pacing,
            ReaderLifecyclePolicy _lifecycle,
            ReaderLifespanPolicy _lifespan,
            SharePolicy _share,
            UserKeyPolicy _userKey)
    {
        durability = _durability;
        deadline = _deadline;
        latency = _latency;
        liveliness = _liveliness;
        reliability = _reliability;
        orderby = _orderby;
        history = _history;
        resource = _resource; 
        userData = _userData;
        ownership = _ownership;
        pacing = _pacing;
        lifecycle = _lifecycle;
        lifespan = _lifespan;
        share = _share;
        userKey = _userKey;
    }
    
    public static ReaderQoS getDefault(){
        return new ReaderQoS(
                            DurabilityPolicy.DEFAULT, DeadlinePolicy.DEFAULT, 
                            LatencyPolicy.DEFAULT, LivelinessPolicy.DEFAULT,
                            ReliabilityPolicy.DEFAULT, OrderbyPolicy.DEFAULT,
                            HistoryPolicy.DEFAULT, ResourcePolicy.DEFAULT, 
                            UserDataPolicy.DEFAULT, OwnershipPolicy.DEFAULT,
                            PacingPolicy.DEFAULT, ReaderLifecyclePolicy.DEFAULT, 
                            ReaderLifespanPolicy.DEFAULT, SharePolicy.DEFAULT,
                            UserKeyPolicy.DEFAULT).copy();
    }
    
    public static ReaderQoS copyFromTopicQoS(TopicQoS tqos){
        ReaderQoS rqos = ReaderQoS.getDefault();
        
        if(tqos != null){
            rqos.setDurability(tqos.getDurability());
            rqos.setDeadline(tqos.getDeadline());
            rqos.setLatency(tqos.getLatency());
            rqos.setLiveliness(tqos.getLiveliness());
            rqos.setReliability(tqos.getReliability());
            rqos.setOrderby(tqos.getOrderby());
            rqos.setHistory(tqos.getHistory());
            rqos.setResource(tqos.getResource());
            rqos.setOwnership(tqos.getOwnership());
        }
        return rqos;
    }
    
    /**
     * Provides access to deadline.
     * 
     * @return Returns the deadline.
     */
    public DeadlinePolicy getDeadline() {
        return deadline;
    }
    /**
     * Sets the deadline to the supplied value.
     *
     * @param deadline The deadline to set.
     */
    public void setDeadline(DeadlinePolicy deadline) {
        this.deadline = deadline;
    }
    /**
     * Provides access to durability.
     * 
     * @return Returns the durability.
     */
    public DurabilityPolicy getDurability() {
        return durability;
    }
    /**
     * Sets the durability to the supplied value.
     *
     * @param durability The durability to set.
     */
    public void setDurability(DurabilityPolicy durability) {
        this.durability = durability;
    }
    /**
     * Provides access to history.
     * 
     * @return Returns the history.
     */
    public HistoryPolicy getHistory() {
        return history;
    }
    /**
     * Sets the history to the supplied value.
     *
     * @param history The history to set.
     */
    public void setHistory(HistoryPolicy history) {
        this.history = history;
    }
    /**
     * Provides access to latency.
     * 
     * @return Returns the latency.
     */
    public LatencyPolicy getLatency() {
        return latency;
    }
    /**
     * Sets the latency to the supplied value.
     *
     * @param latency The latency to set.
     */
    public void setLatency(LatencyPolicy latency) {
        this.latency = latency;
    }
    /**
     * Provides access to lifecycle.
     * 
     * @return Returns the lifecycle.
     */
    public ReaderLifecyclePolicy getLifecycle() {
        return lifecycle;
    }
    /**
     * Sets the lifecycle to the supplied value.
     *
     * @param lifecycle The lifecycle to set.
     */
    public void setLifecycle(ReaderLifecyclePolicy lifecycle) {
        this.lifecycle = lifecycle;
    }
    /**
     * Provides access to liveliness.
     * 
     * @return Returns the liveliness.
     */
    public LivelinessPolicy getLiveliness() {
        return liveliness;
    }
    /**
     * Sets the liveliness to the supplied value.
     *
     * @param liveliness The liveliness to set.
     */
    public void setLiveliness(LivelinessPolicy liveliness) {
        this.liveliness = liveliness;
    }
    /**
     * Provides access to orderby.
     * 
     * @return Returns the orderby.
     */
    public OrderbyPolicy getOrderby() {
        return orderby;
    }
    /**
     * Sets the orderby to the supplied value.
     *
     * @param orderby The orderby to set.
     */
    public void setOrderby(OrderbyPolicy orderby) {
        this.orderby = orderby;
    }
    /**
     * Provides access to pacing.
     * 
     * @return Returns the pacing.
     */
    public PacingPolicy getPacing() {
        return pacing;
    }
    
    /**
     * Provides access to ownership.
     * 
     * @return Returns the ownership.
     */
    public OwnershipPolicy getOwnership() {
        return ownership;
    }
    /**
     * Sets the ownership to the supplied value.
     *
     * @param ownership The ownership to set.
     */
    public void setOwnership(OwnershipPolicy ownership) {
        this.ownership = ownership;
    }
    
    /**
     * Sets the pacing to the supplied value.
     *
     * @param pacing The pacing to set.
     */
    public void setPacing(PacingPolicy pacing) {
        this.pacing = pacing;
    }
    
    /**
     * Provides access to reliability.
     * 
     * @return Returns the reliability.
     */
    public ReliabilityPolicy getReliability() {
        return reliability;
    }
    /**
     * Sets the reliability to the supplied value.
     *
     * @param reliability The reliability to set.
     */
    public void setReliability(ReliabilityPolicy reliability) {
        this.reliability = reliability;
    }
    /**
     * Provides access to resource.
     * 
     * @return Returns the resource.
     */
    public ResourcePolicy getResource() {
        return resource;
    }
    /**
     * Sets the resource to the supplied value.
     *
     * @param resource The resource to set.
     */
    public void setResource(ResourcePolicy resource) {
        this.resource = resource;
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

    public ReaderLifespanPolicy getLifespan() {
        return this.lifespan;
    }

    public void setLifespan(ReaderLifespanPolicy lifespan) {
        this.lifespan = lifespan;
    }

    public SharePolicy getShare() {
        return this.share;
    }

    public void setShare(SharePolicy share) {
        this.share = share;
    }

    public UserKeyPolicy getUserKey() {
        return this.userKey;
    }

    public void setUserKey(UserKeyPolicy userKey) {
        this.userKey = userKey;
    }
    
    public ReaderQoS copy(){
        return new ReaderQoS(
                this.durability.copy(), 
                this.deadline.copy(), 
                this.latency.copy(), 
                this.liveliness.copy(), 
                this.reliability.copy(), 
                this.orderby.copy(), 
                this.history.copy(), 
                this.resource.copy(), 
                this.userData.copy(),
                this.ownership.copy(),
                this.pacing.copy(),
                this.lifecycle.copy(), 
                this.lifespan.copy(), 
                this.share.copy(), 
                this.userKey.copy());
    }
}
