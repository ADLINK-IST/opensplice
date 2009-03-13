package DDS;

public final class TopicBuiltinTopicData {

    public int[] key = new int[3];
    public java.lang.String name = "";
    public java.lang.String type_name = "";
    public DDS.DurabilityQosPolicy durability = new DDS.DurabilityQosPolicy();
    public DDS.DurabilityServiceQosPolicy durability_service = new DDS.DurabilityServiceQosPolicy();
    public DDS.DeadlineQosPolicy deadline = new DDS.DeadlineQosPolicy();
    public DDS.LatencyBudgetQosPolicy latency_budget = new DDS.LatencyBudgetQosPolicy();
    public DDS.LivelinessQosPolicy liveliness = new DDS.LivelinessQosPolicy();
    public DDS.ReliabilityQosPolicy reliability = new DDS.ReliabilityQosPolicy();
    public DDS.TransportPriorityQosPolicy transport_priority = new DDS.TransportPriorityQosPolicy();
    public DDS.LifespanQosPolicy lifespan = new DDS.LifespanQosPolicy();
    public DDS.DestinationOrderQosPolicy destination_order = new DDS.DestinationOrderQosPolicy();
    public DDS.HistoryQosPolicy history = new DDS.HistoryQosPolicy();
    public DDS.ResourceLimitsQosPolicy resource_limits = new DDS.ResourceLimitsQosPolicy();
    public DDS.OwnershipQosPolicy ownership = new DDS.OwnershipQosPolicy();
    public DDS.TopicDataQosPolicy topic_data = new DDS.TopicDataQosPolicy();

    public TopicBuiltinTopicData() {
    }

    public TopicBuiltinTopicData(
        int[] _key,
        java.lang.String _name,
        java.lang.String _type_name,
        DDS.DurabilityQosPolicy _durability,
        DDS.DurabilityServiceQosPolicy _durability_service,
        DDS.DeadlineQosPolicy _deadline,
        DDS.LatencyBudgetQosPolicy _latency_budget,
        DDS.LivelinessQosPolicy _liveliness,
        DDS.ReliabilityQosPolicy _reliability,
        DDS.TransportPriorityQosPolicy _transport_priority,
        DDS.LifespanQosPolicy _lifespan,
        DDS.DestinationOrderQosPolicy _destination_order,
        DDS.HistoryQosPolicy _history,
        DDS.ResourceLimitsQosPolicy _resource_limits,
        DDS.OwnershipQosPolicy _ownership,
        DDS.TopicDataQosPolicy _topic_data)
    {
        key = _key;
        name = _name;
        type_name = _type_name;
        durability = _durability;
        durability_service = _durability_service;
        deadline = _deadline;
        latency_budget = _latency_budget;
        liveliness = _liveliness;
        reliability = _reliability;
        transport_priority = _transport_priority;
        lifespan = _lifespan;
        destination_order = _destination_order;
        history = _history;
        resource_limits = _resource_limits;
        ownership = _ownership;
        topic_data = _topic_data;
    }

}
