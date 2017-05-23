Subscription Module              {#DCPS_Modules_Subscription}
====================

![Class model of the DCPS Subscription Module] (@ref SubscriptionModule_UML.png)

The Subscription Module is comprised of the following classifiers:

- Subscriber
- \subpage DCPS_Modules_Subscription_DataReader "DataReader"
- \subpage DCPS_Modules_Subscription_DataSample "DataSample"
- \subpage DCPS_Modules_Subscription_SampleInfo "SampleInfo"
- \subpage DCPS_Modules_Infrastructure_Listener "Subscriber Listener"
- \ref DCPS_Modules_Infrastructure_Listener "DataReader Listener"
- Read Condition
- Query Condition


The Subscriber  {#DCPS_Modules_Subscriber}
===============

The Subscriber is responsible for collecting data from various publications:

- Each datatype must be accessed using its own typed \ref DCPS_Modules_Subscription_DataReader "DataReader".
- A \ref DCPS_Modules_Subscription_DataReader "DataReader" gives access to samples and their corresponding \ref DCPS_Modules_Subscription_SampleInfo "SampleInfo".
- SampleInfo describes relevant information about the state of a sample and the instance to which it relates, for example about its lifecycle.
- The Subscriber acts as a factory for typed DataReaders.
- DataReaders do not read their data directly from the network:
    - it is the Subscriber that decides when and how the data is transmitted to its DataReaders:
- Groups of messages (possibly coming from different DataWriters) can be received as a whole, allowing for a sort of transaction. (See \ref DCPS_QoS_Presentation "PresentationQoSPolicy").
- A Subscriber only collects samples that are published in the selected Partitions (See \ref DCPS_QoS_Partition PartitionQosPolicy.)

### Applicable QoS

QoS      | Brief
---------|---------
\ref DCPS_QoS_GroupData "GROUP_DATA"                | User data
\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY"        | Auto enable/disable entities on creation
\ref DCPS_QoS_Presentation "PRESENTATION"           | Specifies how the samples representing changes to the data instances are presented
\ref DCPS_QoS_Partition "PARTITION"                 | Set of strings that represent the logical partition among topics
\ref DCPS_QoS_Share "SHARE"                         | Used to share a Subscriber between multiple processes.

### Default QoS Values

|QoS                                          | Attribute    | Value|
|---------------------------------------------|--------------|------|
|\ref DCPS_QoS_GroupData "GROUP_DATA"         | value.length | 0|
|\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY" | autoenable_created_entities | TRUE |
|\ref DCPS_QoS_Presentation "PRESENTATION"    | access_scope \n coherent_access \n ordered_access | INSTANCE \n FALSE \n FALSE |
|\ref DCPS_QoS_Partition "PARTITION"          | name.length  | 0|
|\ref DCPS_QoS_Share "SHARE"                  | name \n enable | NULL \n FALSE |


### Related Status Conditions

Status   | Brief
---------|---------
DATA_ON_READERS | New information is available

### Related Listeners
Listener | Brief
---------|---------
on_data_on_readers() | Data available on one of the attached DataReaders
