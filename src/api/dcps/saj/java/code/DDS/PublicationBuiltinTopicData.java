package DDS;

public final class PublicationBuiltinTopicData {

    public int[] key = new int[3];
    public int[] participant_key = new int[3];
    public java.lang.String topic_name = "";
    public java.lang.String type_name = "";
    public DDS.DurabilityQosPolicy durability = new DDS.DurabilityQosPolicy();
    public DDS.DeadlineQosPolicy deadline = new DDS.DeadlineQosPolicy();
    public DDS.LatencyBudgetQosPolicy latency_budget = new DDS.LatencyBudgetQosPolicy();
    public DDS.LivelinessQosPolicy liveliness = new DDS.LivelinessQosPolicy();
    public DDS.ReliabilityQosPolicy reliability = new DDS.ReliabilityQosPolicy();
    public DDS.LifespanQosPolicy lifespan = new DDS.LifespanQosPolicy();
    public DDS.UserDataQosPolicy user_data = new DDS.UserDataQosPolicy();
    public DDS.OwnershipQosPolicy ownership = new DDS.OwnershipQosPolicy();
    public DDS.OwnershipStrengthQosPolicy ownership_strength = new DDS.OwnershipStrengthQosPolicy();
    public DDS.PresentationQosPolicy presentation = new DDS.PresentationQosPolicy();
    public DDS.PartitionQosPolicy partition = new DDS.PartitionQosPolicy();
    public DDS.TopicDataQosPolicy topic_data = new DDS.TopicDataQosPolicy();
    public DDS.GroupDataQosPolicy group_data = new DDS.GroupDataQosPolicy();

    public PublicationBuiltinTopicData() {
    }

    public PublicationBuiltinTopicData(
        int[] _key,
        int[] _participant_key,
        java.lang.String _topic_name,
        java.lang.String _type_name,
        DDS.DurabilityQosPolicy _durability,
        DDS.DeadlineQosPolicy _deadline,
        DDS.LatencyBudgetQosPolicy _latency_budget,
        DDS.LivelinessQosPolicy _liveliness,
        DDS.ReliabilityQosPolicy _reliability,
        DDS.LifespanQosPolicy _lifespan,
        DDS.UserDataQosPolicy _user_data,
        DDS.OwnershipQosPolicy _ownership,
        DDS.OwnershipStrengthQosPolicy _ownership_strength,
        DDS.PresentationQosPolicy _presentation,
        DDS.PartitionQosPolicy _partition,
        DDS.TopicDataQosPolicy _topic_data,
        DDS.GroupDataQosPolicy _group_data)
    {
        key = _key;
        participant_key = _participant_key;
        topic_name = _topic_name;
        type_name = _type_name;
        durability = _durability;
        deadline = _deadline;
        latency_budget = _latency_budget;
        liveliness = _liveliness;
        reliability = _reliability;
        lifespan = _lifespan;
        user_data = _user_data;
        ownership = _ownership;
        ownership_strength = _ownership_strength;
        presentation = _presentation;
        partition = _partition;
        topic_data = _topic_data;
        group_data = _group_data;
    }

}
