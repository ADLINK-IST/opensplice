Supported Quality of Service (QoS)      {#DCPS_QoS}
========================
The Data-Distribution Service (DDS) relies on the use of QoS. A QoS (Quality of Service) is a set of characteristics that
controls some aspect of the behavior of the DDS Service. QoS is comprised of individual QoS policies (objects of type
deriving from QosPolicy).

The QosPolicy objects that need to be set in a compatible manner between the publisher and subscriber ends are indicated by the setting of the "RxO" (Requested/Offered) property:

- An "RxO" setting of "Yes" indicates that the policy can be set both at the publishing and subscribing ends and the values must be set in a compatible manner. In this case the compatible values are explicitly defined.
- An "RxO" setting of "No" indicates that the policy can be set both at the publishing and subscribing ends but the two settings are independent. That is, all combinations of values are compatible.
- An "RxO" setting of "N/A" indicates that the policy can only be specified at either the publishing or the subscribing end, but not at both ends. So compatibility does not apply.


The "Changeable" property determines whether the QosPolicy can be changed after the Entity is enabled. In other words, a policy with "Changeable" setting of "NO" is considered "immutable" and can only be specified either at Entity creation time or else prior to calling the enable() operation.

- \subpage DCPS_QoS_UserData "User Data"
- \subpage DCPS_QoS_TopicData "Topic Data"
- \subpage DCPS_QoS_GroupData "Group Data"
- \subpage DCPS_QoS_Durability "Durability"
- \subpage DCPS_QoS_DurabilityService "Durability Service"
- \subpage DCPS_QoS_Presentation "Presentation"
- \subpage DCPS_QoS_Deadline "Deadline"
- \subpage DCPS_QoS_LatencyBudget "Latency Budget"
- \subpage DCPS_QoS_Ownership "Ownership"
- \subpage DCPS_QoS_OwnershipStrength "Ownership Strength"
- \subpage DCPS_QoS_Liveliness "Liveliness"
- \subpage DCPS_QoS_TimeBasedFilter "Time Based Filter"
- \subpage DCPS_QoS_Partition "Partition"
- \subpage DCPS_QoS_Reliability "Reliability"
- \subpage DCPS_QoS_TransportPriority "Transport Priority"
- \subpage DCPS_QoS_Lifespan "Lifespan"
- \subpage DCPS_QoS_DestinationOrder "Destination Order"
- \subpage DCPS_QoS_History "History"
- \subpage DCPS_QoS_ResourceLimits "Resource Limits"
- \subpage DCPS_QoS_EntityFactory "Entity Factory"
- \subpage DCPS_QoS_WriterDataLifecycle "Writer Data Lifecycle"
- \subpage DCPS_QoS_ReaderDataLifecycle "Reader Data Lifecycle"
- \subpage DCPS_QoS_SubscriptionKey "Subscription Key"
- \subpage DCPS_QoS_ReaderLifespan "Reader Lifespan"
- \subpage DCPS_QoS_Share "Share"
- \subpage DCPS_QoS_ViewKey "View Key"
- \subpage DCPS_QoS_Scheduling "Scheduling"

### Default QoS values

- \ref DCPS_Modules_DomainModule "DomainParticipant"
- \ref DCPS_Modules_TopicDefinition "Topic"
- \ref DCPS_Modules_Publication "Publisher "
- \ref DCPS_Modules_Publication_DataWriter "DataWriter"
- \ref DCPS_Modules_Subscription "Subscriber"
- \ref DCPS_Modules_Subscription_DataReader "DataReader"
