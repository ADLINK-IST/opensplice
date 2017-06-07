The DataWriter      {#DCPS_Modules_Publication_DataWriter}
===============
The DataWriter is an accessor to write data into a Domain.
It is attached to the \ref DCPS_Modules_Publication "Publisher", a derived class specific to the associated \ref DCPS_Modules_TopicDefinition "Topic".
The \ref DCPS_Modules_Publication "Publisher" and the \ref DCPS_Modules_TopicDefinition "Topic" associated to a DataWriter must be created with the same \ref DCPS_Modules_DomainParticipant "DomainParticipant".
QoS policies on the associated \ref DCPS_Modules_TopicDefinition "Topic" is retrieved and merged with DataWriter QoS (default or application provided)
during creation.


### Applicable QoS

Qos      | Brief
---------|---------
\ref DCPS_QoS_UserData "USER_DATA" | Sequence of data distributed by means of built-in topics
\ref DCPS_QoS_Durability "DURABILITY" | Specifies if the data should 'outlive' their writing time
\ref DCPS_QoS_Deadline "DEADLINE" | Specifies the time duration for updating each instance periodically
\ref DCPS_QoS_LatencyBudget "LATENCY_BUDGET" | Specifies the "urgency" of the data-communication
\ref DCPS_QoS_Ownership "OWNERSHIP" | Specifies multiple DataWriter objects to update the same instance
\ref DCPS_QoS_OwnershipStrength "OWNERSHIP_STRENGTH" | Specifies the value of the strength to arbitrate among DataWriters to update the same instance
\ref DCPS_QoS_Liveliness "LIVELINESS" | Specifies the mechanism and parameters used by the application to determine the whether the Entity is 'alive'
\ref DCPS_QoS_Reliability "RELIABILITY" | Specifies the level of reliability offered/requested by the service
\ref DCPS_QoS_TransportPriority "TRANSPORT_PRIORITY" | Specifies the priority of the underlying transport used to send the data
\ref DCPS_QoS_Lifespan "LIFESPAN" | Specifies the maximum duration of validity of the data written
\ref DCPS_QoS_DestinationOrder "DESTINATION_ORDER" | Specifies the criteria to determine the logical order among changes made by \ref DCPS_Modules_Publication "Publisher" entities of the same instances
\ref DCPS_QoS_History "HISTORY" | Specifies the behaviour of the service when the value of an instance changes before communication to the DataReaders.
\ref DCPS_QoS_ResourceLimits "RESOURCE_LIMITS" | Specifies the resources that can be consumed by the service
\ref DCPS_QoS_WriterDataLifecycle "WRITER_DATA_LIFECYCLE" | Specifies the behaviour of the DataWriter with respect to the lifecycle of the data-instances.

### Default QoS Values

|QoS                    | Attribute                     | Value|
|-----------------------|-------------------------------|------|
|\ref DCPS_QoS_UserData "USER_DATA"                     | value.length | 0|
|\ref DCPS_QoS_Durability "DURABILITY"                  | kind | VOLATILE|
|\ref DCPS_QoS_Deadline "DEADLINE"                      | period | DURATION_INFINITE|
|\ref DCPS_QoS_LatencyBudget "LATENCY_BUDGET"           | duration | 0|
|\ref DCPS_QoS_Ownership "OWNERSHIP"                    | kind | SHARED |
|\ref DCPS_QoS_OwnershipStrength "OWNERSHIP_STRENGTH"   | value | 0 |
|\ref DCPS_QoS_Liveliness "LIVELINESS"                  | kind \n  lease_duration | AUTOMATIC \n DURATION_INFINITE|
|\ref DCPS_QoS_Reliability "RELIABILITY"                | kind \n max_blocking_time  \n synchronous | RELIABLE \n 100 ms \n FALSE |
|\ref DCPS_QoS_TransportPriority "TRANSPORT_PRIORITY"   | value | 0 |
|\ref DCPS_QoS_Lifespan "LIFESPAN"                      | duration | DURATION_INFINITE |
|\ref DCPS_QoS_DestinationOrder "DESTINATION_ORDER"     | kind | BY_RECEPTION_TIMESTAMP |
|\ref DCPS_QoS_History "HISTORY"                        | kind \n depth | KEEP_LAST \n 1|
|\ref DCPS_QoS_ResourceLimits "RESOURCE_LIMITS"         | max_samples \n max_instances \n max_samples_per_instance  | LENGTH_UNLIMITED \n LENGTH_UNLIMITED \n LENGTH_UNLIMITED|
|\ref DCPS_QoS_WriterDataLifecycle "WRITER_DATA_LIFECYCLE" | autodispose_unregistered_instances | TRUE |


### Related Status Conditions

Status   | Brief
---------|---------
\ref DCPS_Status_LivelinessLost "LIVELINESS_LOST" | Liveliness QoS set is not respected & DataReaders will consider DataWriters are no longer active
\ref DCPS_Status_OfferedDeadlineMissed "OFFERED_DEADLINE_MISSED" | Deadline QoS set is not respected for a specific instance.
\ref DCPS_Status_OfferedIncompatibleQoS "OFFERED_INCOMPATIBLE_QOS " | QoS policies not compatible with the DataReaders
\ref DCPS_Status_PublicationMatched "PUBLICATION_MATCHED" | The DataWriter has found a DataReader that matches the \ref DCPS_Modules_TopicDefinition "Topic" and Compatible QoS

### Related Listeners
Listener | Brief
---------|---------
on_liveliness_lost | LiveLiness set by DataWriter is not respected
on_offered_deadline_missed | Deadline duration set by DataWriter is not respected
on_offered_incompatible_qos | QoS set by the DataWriter is not compatible with the DataReaders
on_publication_matched | The DataWriter has found a DataReader with a same \ref DCPS_Modules_TopicDefinition "Topic" and Compatible QoS
