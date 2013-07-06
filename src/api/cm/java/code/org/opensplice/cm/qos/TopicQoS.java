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
 * Represents the set of policies that apply to a Topic Entity. It 
 * consists of:
 * - TopicDataPolicy
 * - DurabilityPolicy
 * - DeadlinePolicy
 * - LatencyPolicy
 * - LivelinessPolicy
 * - ReliabilityPolicy
 * - OrderbyPolicy
 * - HistoryPolicy
 * - ResourcePolicy
 * - TransportPolicy
 * - LifespanPolicy
 * - OwnershipPolicy 
 * 
 * @date Jan 10, 2005 
 */
public class TopicQoS extends QoS{
    /**
     * The TOPIC_DATA policy.
     */
    private TopicDataPolicy topicData;
    
    /**
     * The DURABILITY policy.
     */
    private DurabilityPolicy durability;
    
    /**
     * The DURABILITY_SERVICE policy.
     */
    private DurabilityServicePolicy durabilityService;
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
     * The OWNERSHIP_POLICY.
     */
    private OwnershipPolicy ownership;
    
    /**
     * Constructs a new TopicQoS.
     *
     * @param _topicData The TOPIC_DATA policy.
     * @param _durability The DURABILITY policy.
     * @param _durability The DURABILITY_SERVICE policy.
     * @param _deadline The DEADLINE policy.
     * @param _latency The LATENCY_BUDGET policy.
     * @param _liveliness The LIVELINESS policy.
     * @param _reliability The RELIABILITY policy.
     * @param _orderby The DESTINATION_ORDER policy.
     * @param _history The HISTORY policy.
     * @param _resource The RESOURCE_LIMITS policy.
     * @param _transport The TRANSPORT_PRIORITY policy.
     * @param _lifespan The LIFESPAN policy.
     * @param _ownership The OWNERSHIP policy.
     */
    public TopicQoS(
            TopicDataPolicy _topicData,
            DurabilityPolicy _durability,
            DurabilityServicePolicy _durabilityService,
            DeadlinePolicy _deadline,
            LatencyPolicy _latency,
            LivelinessPolicy _liveliness,
            ReliabilityPolicy _reliability,
            OrderbyPolicy _orderby,
            HistoryPolicy _history,
            ResourcePolicy _resource,
            TransportPolicy _transport,
            LifespanPolicy _lifespan,
            OwnershipPolicy _ownership)
    {
        topicData = _topicData;
        durability = _durability;
        durabilityService = _durabilityService;
        deadline = _deadline;
        latency = _latency;
        liveliness = _liveliness;
        reliability = _reliability;
        orderby = _orderby;
        history = _history;
        resource = _resource; 
        transport = _transport;
        lifespan = _lifespan;
        ownership = _ownership;
    }
    
    public static TopicQoS getDefault(){
        return new TopicQoS(
                            TopicDataPolicy.DEFAULT, DurabilityPolicy.DEFAULT, 
                            DurabilityServicePolicy.DEFAULT,
                            DeadlinePolicy.DEFAULT, LatencyPolicy.DEFAULT, 
                            LivelinessPolicy.DEFAULT, ReliabilityPolicy.DEFAULT, 
                            OrderbyPolicy.DEFAULT, HistoryPolicy.DEFAULT, 
                            ResourcePolicy.DEFAULT, TransportPolicy.DEFAULT, 
                            LifespanPolicy.DEFAULT, OwnershipPolicy.DEFAULT).copy();
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
     * Provides access to durability.
     * 
     * @return Returns the durability.
     */
    public DurabilityPolicy getDurability() {
        return durability;
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
     * Provides access to latency.
     * 
     * @return Returns the latency.
     */
    public LatencyPolicy getLatency() {
        return latency;
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
     * Provides access to liveliness.
     * 
     * @return Returns the liveliness.
     */
    public LivelinessPolicy getLiveliness() {
        return liveliness;
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
     * Provides access to ownership.
     * 
     * @return Returns the ownership.
     */
    public OwnershipPolicy getOwnership() {
        return ownership;
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
     * Provides access to resource.
     * 
     * @return Returns the resource.
     */
    public ResourcePolicy getResource() {
        return resource;
    }
    /**
     * Provides access to topicData.
     * 
     * @return Returns the topicData.
     */
    public TopicDataPolicy getTopicData() {
        return topicData;
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
     * Provides access to durabilityService.
     * 
     * @return Returns the durabilityService.
     */
    public DurabilityServicePolicy getDurabilityService() {
        return durabilityService;
    }
    
    public TopicQoS copy(){
        return new TopicQoS(
                this.topicData.copy(), 
                this.durability.copy(), 
                this.durabilityService.copy(),
                this.deadline.copy(),
                this.latency.copy(),
                this.liveliness.copy(),
                this.reliability.copy(),
                this.orderby.copy(),
                this.history.copy(),
                this.resource.copy(),
                this.transport.copy(),
                this.lifespan.copy(),
                this.ownership.copy());
    }
}
