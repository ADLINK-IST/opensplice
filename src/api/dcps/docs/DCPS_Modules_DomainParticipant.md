Domain Module              {#DCPS_Modules_DomainModule}
==============

![Class model of the DCPS Domain Module] (@ref DomainModule_UML.png)

The DCPS Domain Module is comprised of the following classes:
- \ref DCPS_Modules_DomainParticipant "DomainParticipant"
- DomainParticipantFactory
- DomainParticipantListener

The Domain Participant     {#DCPS_Modules_DomainParticipant}
=======================
The DomainParticipant represents the participation of the application on
a communication plane that isolates applications running on the same
set of physical computers from each other. A domain establishes a virtual
network linking all applications that share the same domainId and isolating
them from applications running on different domains. In this way, several
independent distributed applications can coexist in the same physical
network without interfering, or even being aware of each other.

The Domain Participant has several roles:

- It acts as the container for all other \ref DCPS_Modules_Entity "Entities"
- Applications that want to join a **Domain** must create a **DomainParticipant**
- A DomainParticipant acts as a factory for **DomainEntities** i.e. entities that either define, read or write information into the domain.

The DomainParticipant itself is also a specialisation of the Entity (Infrastructure module) class:
- It has QoSPolicies that define its behaviour
- It has a StatusCondition that keeps track of its communication status
- It has a Listener that can handle the events of all embedded DomainEntities that did not handle those events themselves


### Applicable QoS

QoS      | Brief
---------|---------
\ref DCPS_QoS_UserData "USER_DATA"                | User data
\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY"      | Auto enable/disable entities on creation
\ref DCPS_QoS_Scheduling "WATCHDOG_SCHEDULING"    | Used to set the scheduling parameters to create the watchdog thread
\ref DCPS_QoS_Scheduling "LISTENER_SCHEDULING"    | Used to set the scheduling parameters to create the listener thread

### Default QoS Values

|QoS                                                    | Attribute                     | Value|
|-------------------------------------------------------|-------------------------------|------|
|\ref DCPS_QoS_UserData "USER_DATA"                     | value.length                  | 0    |
|\ref DCPS_QoS_EntityFactory "ENTITY_FACTORY"           | autoenable_created_entities   | TRUE |
|\ref DCPS_QoS_Scheduling "WATCHDOG_SCHEDULING"         | scheduling_class.kind \n scheduling_priority_kind.kind \n scheduling_priority | SCHEDULE_DEFAULT \n PRIORITY_RELATIVE \n 0 |
|\ref DCPS_QoS_Scheduling "LISTENER_SCHEDULING"         | scheduling_class.kind \n scheduling_priority_kind.kind \n scheduling_priority | SCHEDULE_DEFAULT \n PRIORITY_RELATIVE \n 0 |

### Related Status Conditions

None directly related to Domain Participant actions

### Related Listeners

The Domain Participant does not possess its own own listeners and instead inherits from all of its child entities

\dot
digraph Listeners
{
    D[label="Domain",shape=box, URL="\ref DCPS_Modules_DomainParticipant"];
    T[label="Topic",shape=box, URL="\ref DCPS_Modules_Topic"];
    P[label="Publisher",shape=box, URL="\ref DCPS_Modules_Publication"];
    S[label="Subscriber",shape=box, URL="\ref DCPS_Modules_Subscription"];
    R[label="DataReader",shape=box, URL="\ref DCPS_Modules_Subscription_DataReader"];
    W[label="DataWriter",shape=box, URL="\ref DCPS_Modules_Publication_DataWriter"];

    D -> S[dir="back"];
    D -> T[dir="back"];
    D -> P[dir="back"];
    P -> W[dir="back"];
    S -> R[dir="back"];
}
\enddot
