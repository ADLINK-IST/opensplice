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
 * Represents the set of policies that apply to a Writer Entity. It 
 * consists of:
 * - DurabilityPolicy
 * - DeadlinePolicy
 * - LatencyPolicy
 * - LivelinessPolicy
 * - ReliabilityPolicy
 * - OrderbyPolicy
 * - HistoryPolicy
 * - ResourcePolicy
 * - TransportPolicy
 * - UserDataPolicy
 * - OwnershipPolicy
 * - StrengthPolicy
 * - WriterLifecyclePolicy
 * 
 * @date Jan 10, 2005 
 */
public class WriterQoS extends QoS {
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
     * The TRANSPORT_PRIORITY policy.
     */
    private TransportPolicy transport;
    
    /**
     * The LIFESPAN policy.
     */
    private LifespanPolicy lifespan;
    
    /**
     * The USER_DATA policy.
     */
    private UserDataPolicy userData;
    
    
    private OwnershipPolicy ownership;
    
    /**
     * The OWNERSHIP_STRENGTH policy.
     */
    private StrengthPolicy strength;
    
    /**
     * The WRITER_DATA_LIFECYCLE policy.
     */
    private WriterLifecyclePolicy lifecycle;
    
    public static int             NUMBEROFWRITERQOSPOLICIES = 13;

    /**
     * Constructs a new WriterQoS
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
     * @param _transport The TRANSPORT_PRIORITY policy.
     * @param _lifespan The LIFESPAN policy.
     * @param _userData The USER_DATA policy.
     * @param _strength The OWNERSHIP_STRENGTH policy.
     * @param _lifecycle The WRITER_DATA_LIFECYCLE policy.
     */
    public WriterQoS(
            DurabilityPolicy _durability,
            DeadlinePolicy _deadline,
            LatencyPolicy _latency,
            LivelinessPolicy _liveliness,
            ReliabilityPolicy _reliability,
            OrderbyPolicy _orderby,
            HistoryPolicy _history,
            ResourcePolicy _resource,
            TransportPolicy _transport,
            LifespanPolicy _lifespan,
            UserDataPolicy _userData,
            OwnershipPolicy _ownership,
            StrengthPolicy _strength,
            WriterLifecyclePolicy _lifecycle)
    {
        durability = _durability;
        deadline = _deadline;
        latency = _latency;
        liveliness = _liveliness;
        reliability = _reliability;
        orderby = _orderby;
        history = _history;
        resource = _resource; 
        transport = _transport;
        lifespan = _lifespan;
        userData = _userData;
        ownership = _ownership;
        strength = _strength;
        lifecycle = _lifecycle;
    }
    
    public static WriterQoS getDefault(){
        return new WriterQoS(
                        DurabilityPolicy.DEFAULT, DeadlinePolicy.DEFAULT, 
                        LatencyPolicy.DEFAULT, LivelinessPolicy.DEFAULT, 
                        ReliabilityPolicy.DEFAULT, OrderbyPolicy.DEFAULT, 
                        HistoryPolicy.DEFAULT, ResourcePolicy.DEFAULT, 
                        TransportPolicy.DEFAULT, LifespanPolicy.DEFAULT, 
                        UserDataPolicy.DEFAULT, OwnershipPolicy.DEFAULT, 
                        StrengthPolicy.DEFAULT, 
                        WriterLifecyclePolicy.DEFAULT).copy();
    }
    
    public static WriterQoS copyFromTopicQoS(TopicQoS tqos){
        WriterQoS wqos = WriterQoS.getDefault();
        
        if(tqos != null){
            wqos.setDurability(tqos.getDurability());
            wqos.setDeadline(tqos.getDeadline());
            wqos.setLatency(tqos.getLatency());
            wqos.setLiveliness(tqos.getLiveliness());
            wqos.setReliability(tqos.getReliability());
            wqos.setOrderby(tqos.getOrderby());
            wqos.setHistory(tqos.getHistory());
            wqos.setResource(tqos.getResource());
            wqos.setTransport(tqos.getTransport());
            wqos.setLifespan(tqos.getLifespan());
            wqos.setOwnership(tqos.getOwnership());
            
        }
        return wqos;
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
    public WriterLifecyclePolicy getLifecycle() {
        return lifecycle;
    }
    /**
     * Sets the lifecycle to the supplied value.
     *
     * @param lifecycle The lifecycle to set.
     */
    public void setLifecycle(WriterLifecyclePolicy lifecycle) {
        this.lifecycle = lifecycle;
    }
    /**
     * Provides access to lifespan.
     * 
     * @return Returns the lifespan.
     */
    public LifespanPolicy getLifespan() {
        return lifespan;
    }
    /**
     * Sets the lifespan to the supplied value.
     *
     * @param lifespan The lifespan to set.
     */
    public void setLifespan(LifespanPolicy lifespan) {
        this.lifespan = lifespan;
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
     * Provides access to strength.
     * 
     * @return Returns the strength.
     */
    public StrengthPolicy getStrength() {
        return strength;
    }
    /**
     * Sets the strength to the supplied value.
     *
     * @param strength The strength to set.
     */
    public void setStrength(StrengthPolicy strength) {
        this.strength = strength;
    }
    /**
     * Provides access to transport.
     * 
     * @return Returns the transport.
     */
    public TransportPolicy getTransport() {
        return transport;
    }
    /**
     * Sets the transport to the supplied value.
     *
     * @param transport The transport to set.
     */
    public void setTransport(TransportPolicy transport) {
        this.transport = transport;
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
    
    public WriterQoS copy(){
        return new WriterQoS(
                this.durability.copy(),
                this.deadline.copy(),
                this.latency.copy(),
                this.liveliness.copy(),
                this.reliability.copy(),
                this.orderby.copy(),
                this.history.copy(),
                this.resource.copy(),
                this.transport.copy(),
                this.lifespan.copy(),
                this.userData.copy(),
                this.ownership.copy(),
                this.strength.copy(),
                this.lifecycle.copy());
    }
}
