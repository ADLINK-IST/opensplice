Publication Module              {#DCPS_Modules_Publication}
==================

![Class model of the DCPS Publication Module] (@ref PublicationModule_UML.png)

The Publication Module is comprised of the following classifiers:

- Publisher
- \subpage DCPS_Modules_Publication_DataWriter "DataWriter"
- \subpage DCPS_Modules_Infrastructure_Listener "Publisher Listener"
- \ref DCPS_Modules_Infrastructure_Listener "DataWriter Listener"

The Publisher  {#DCPS_Modules_Publisher}
===============
The Publisher is responsible for sending the data-update message to subscribers:

- Any change in data associated to one or more DataWriters associated will be published.
- Information like timestamp, writers and QoS set are considered before publishing


### Applicable QoS

Qos      | Brief
---------|---------
\ref DCPS_QoS_GroupData "GROUP_DATA" | Sequence of data distributed by means of built-in topics
\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY" | Auto enable/disable entities on creation
\ref DCPS_QoS_Presentation "PRESENTATION" | Specifies how the samples representing changes to the data instances are presented
\ref DCPS_QoS_Partition "PARTITION" | Set of strings that represent the logical partition among topics

### Default QoS Values

|QoS                                          | Attribute    | Value|
|---------------------------------------------|--------------|------|
|\ref DCPS_QoS_GroupData "GROUP_DATA"         | value.length | 0|
|\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY" | autoenable_created_entities | TRUE |
|\ref DCPS_QoS_Presentation "PRESENTATION"    | access_scope \n coherent_access \n ordered_access | INSTANCE \n FALSE \n FALSE |
|\ref DCPS_QoS_Partition "PARTITION"          | name.length  | 0|



### Related Listeners
The Publisher listens to \ref DCPS_Modules_Publication_DataWriter "DataWriter" defined callbacks, if registered.
