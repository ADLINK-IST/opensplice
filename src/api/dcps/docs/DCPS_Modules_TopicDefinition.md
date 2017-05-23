Topic-Definition Module                 {#DCPS_Modules_TopicDefinition}
========================

![Class model of the DCPS Topic-Definition Module] (@ref TopicModule_UML.png)
The Topic-Definition Module is comprised of the following classes:

- TopicDescription
- @subpage DCPS_Modules_Topic "Topic"
- ContentFilteredTopic
- MultiTopic
- TopicListener
- TypeSupport

TopicDescription represents the fact that both publications and subscriptions are tied to a single data-type. Its attribute
type_name defines a unique resulting type for the publication or the subscription and therefore creates an implicit
association with a TypeSupport. TopicDescription has also a name that allows it to be retrieved locally.

Topic                    {#DCPS_Modules_Topic}
======
Topic is identified by its name, which must be unique in the whole Domain. In addition (by virtue of extending TopicDescription) it fully specifies the type of the data that can be communicated when publishing or subscribing to the Topic.

Topic is the only TopicDescription that can be used for publications and therefore associated to a DataWriter. All operations except for the base-class operations set_qos, get_qos, set_listener, get_listener, enable, and get_status_condition may return the value NOT_ENABLED.

### Applicable QoS

QoS      | Brief
---------|---------
\ref DCPS_QoS_TopicData "TOPIC_DATA"                    | User data
\ref DCPS_QoS_Durability "DURABILITY"                   | Expresses the lifetime of a sample
\ref DCPS_QoS_DurabilityService "DURABILITY_SERVICE"    | Specifies the configuration of the durability service
\ref DCPS_QoS_Deadline "DEADLINE"                       | Sets the period in which a sample must be sent/received
\ref DCPS_QoS_LatencyBudget "LATENCY_BUDGET"            | Sets the maximum delay the sample is in transit
\ref DCPS_QoS_Liveliness "LIVELINESS"                   | Set the method by which an instance is considered alive
\ref DCPS_QoS_Ownership "OWNERSHIP"                     | Specifies if the DataWriters is shared or exclusive
\ref DCPS_QoS_Reliability "RELIABILITY"                 | Set the required reliability of a DataReader/DataWriter pair, reliable or best effort
\ref DCPS_QoS_DestinationOrder "DESTINATION_ORDER"      | Controls the logical order of the samples by reception time-stamp or source time-stamp
\ref DCPS_QoS_History "HISTORY"                         | Specify how many generations of the same instance to keep
\ref DCPS_QoS_ResourceLimits "RESOURCE_LIMITS"          | Specify the amount of resources the DataReader/DataWriter pair can consume
\ref DCPS_QoS_TransportPriority "TRANSPORT_PRIORITY"    | A hint to the underlying transport for prioritization
\ref DCPS_QoS_Lifespan "LIFESPAN"                       | Specifies the maximum duration of validity of the data written

### Default QoS values

|QoS                    | Attribute                 | Value|
|-----------------------|---------------------------|------|
|\ref DCPS_QoS_TopicData "TOPIC_DATA"               | value.length              | 0|
|\ref DCPS_QoS_Durability "DURABILITY"              | kind                      | VOLATILE|
|\ref DCPS_QoS_DurabilityService "DURABILITY_SERVICE"| service_cleanup_delay \n history_kind  \n history_depth \n max_samples \n max_instances \n max_samples_per_instance  |0 \n KEEP_LAST \n 1 \n  LENGTH_UNLIMITED \n LENGTH_UNLIMITED \n LENGTH_UNLIMITED|
|\ref DCPS_QoS_Deadline "DEADLINE"                  | period                    | DURATION_INFINITE|
|\ref DCPS_QoS_LatencyBudget "LATENCY_BUDGET"       | duration                  | 0|
|\ref DCPS_QoS_Liveliness "LIVELINESS"              | kind \n  lease_duration   | AUTOMATIC \n DURATION_INFINITE|
|\ref DCPS_QoS_Ownership "OWNERSHIP"                | kind                      | SHARED|
|\ref DCPS_QoS_Reliability "RELIABILITY"            | kind \n  max_blocking_time \n synchronous | BEST_EFFORT \n 100 ms \n FALSE|
|\ref DCPS_QoS_DestinationOrder "DESTINATION_ORDER" | kind                      | BY_RECEPTION_TIMESTAMP|
|\ref DCPS_QoS_History "HISTORY"                    | kind \n depth             | KEEP_LAST \n 1|
|\ref DCPS_QoS_ResourceLimits "RESOURCE_LIMITS"     | max_samples \n max_instances \n max_samples_per_instance  | LENGTH_UNLIMITED \n LENGTH_UNLIMITED \n LENGTH_UNLIMITED|
|\ref DCPS_QoS_TransportPriority "TRANSPORT_PRIORITY"| value | 0 |
|\ref DCPS_QoS_Lifespan "LIFESPAN"                  | duration  | DURATION_INFINITE |

### Related Status Conditions

Status   | Brief
---------|---------
\ref DCPS_Status_InconsistentTopic "INCONSISTENT_TOPIC"     |   Another topic exists with the same name but different characteristics

### Related Listeners
Listener | Brief
---------|---------
\ref DCPS_Status_InconsistentTopic "on_inconsistent_topic()"     |   Another topic exists with the same name but different characteristics

Content Filtered Topic
======================
ContentFilteredTopic is a specialization of TopicDescription that allows for content-based subscriptions.
ContentFilteredTopic describes a more sophisticated subscription that indicates the subscriber does not want to
necessarily see all values of each instance published under the Topic. Rather, it wants to see only the values whose
contents satisfy certain criteria. This class therefore can be used to request content-based subscriptions.

The selection of the content is done using the filter_expression with parameters expression_parameters:

- The filter_expression attribute is a string that specifies the criteria to select the data samples of interest. It is similar to the WHERE part of an SQL clause.

- The expression_parameters attribute is a sequence of strings that give values to the ‘parameters’ (i.e., "%n" tokens) in the filter_expression. The number of supplied parameters must fit with the requested values in the filter_expression (i.e., the number of %n tokens).
