Infrastructure Module            {#DCPS_Modules_Infrastructure}
=====================

![Class model of the DCPS Infrastructure Module] (@ref InfrastructureModule_UML.png)


The Infrastructure Module defines the abstract classes and the interfaces that are refined by the other modules. It also provides support for the two interaction styles (notification and wait based) with the middleware

The DCPS Infrastructure Module is comprised of the following classifiers:

- Entity
- DomainEntity
- \ref DCPS_QoS "QoS policy"
- @subpage DCPS_Modules_Infrastructure_Listener "Listener"
- @subpage DCPS_Modules_Infrastructure_Status "Status"
- @subpage DCPS_Modules_Infrastructure_Waitset "Waitset"
- Condition
- GuardCondition
- StatusCondition

Entity      {#DCPS_Modules_Entity}
========

- Entities are the basic building blocks of your Data Distribution Service:
- An Entity is the common parent for the most important DDS-elements:

    - DomainParticipant: Application connector to a Network Domain.
    - Topic: Defines the messages that will be transmitted across the network.
    - DataWriter: Accessor to write typed data (Topic) into a Domain.
    - DataReader: Accessor to read typed data (Topic) from a Domain.
    - Publisher: Aggregation of DataWriter objects, used to disseminate data.
    - Subscriber: Aggregation of DataReader objects, used to collect data.
