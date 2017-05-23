Durability QoS              {#DCPS_QoS_Durability}
==============

This QosPolicy controls whether the data should be stored for late joining readers.

Attributes
----------
<table>
    <tr>
        <th>Value</th>
        <th>Meaning</th>
        <th>Concerns</th>
        <th>RxO</th>
        <th>Changeable</th>
    </tr>
    <tr>
        <td>
            A DurabilityQosPolicyKind:<br/>
            kind
        </td>
        <td>
            Specifies the type of durability from VOLATILE_DURABILITY_QOS (short life) to PERSISTENT_DURABILITY_QOS (long life).
        </td>
        <td rowspan="4">
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="4">Yes</td>
        <td rowspan="4">No</td>
    </tr>
    <tr>
        <td>
            VOLATILE
        </td>
        <td>
            The Data Distribution Service does not need to keep
            any samples of data-instances on
            behalf of any \ref DCPS_Modules_Subscription_DataReader "DataReader" that is not
            known by the \ref DCPS_Modules_Publication_DataWriter "DataWriter" at the
            time the instance is written. In other
            words the Data Distribution Service will only attempt
            to provide the data to existing
            subscribers. This is the default kind.
        </td>
    </tr>
    <tr>
        <td>
            TRANSIENT_LOCAL,
            TRANSIENT
        </td>
        <td>
            The Data Distribution Service does not need to keep
            any samples of data-instances on
            behalf of any \ref DCPS_Modules_Subscription_DataReader "DataReader" that is not
            known by the \ref DCPS_Modules_Publication_DataWriter "DataWriter" at the
            time the instance is written. In other
            words the Service will only attempt
            to provide the data to existing
            subscribers. This is the default kind.
        </td>
    </tr>
    <tr>
        <td>
            PERSISTENT
        </td>
        <td>
            Data is kept on permanent
            storage, so that they can outlive a
            system session.
        </td>
    </tr>
</table>


The decoupling between \ref DCPS_Modules_Subscription_DataReader "DataReader" and \ref DCPS_Modules_Publication_DataWriter "DataWriter" offered by the Publish/Subscribe paradigm allows an application to write data even if there are no current readers on the network. Moreover, a \ref DCPS_Modules_Subscription_DataReader "DataReader" that joins the network after some data has been written could potentially be interested in accessing the most current values of the data as well as potentially some history.

This QoS policy controls whether the Data Distribution Service will actually make data available to late-joining readers. Note that although related, this does not strictly control what data the Service will maintain internally. That is, the Data Distribution Service may choose to maintain some data for its own purposes (e.g., flow control) and yet not make it available to late-joining readers if the DURABILITY QoS policy is set to VOLATILE.

The value offered is considered compatible with the value requested if and only if the inequality “offered kind >= requested kind evaluates to ‘TRUE.’ For the purposes of this inequality, the values of DURABILITY kind are considered ordered such that VOLATILE < TRANSIENT_LOCAL < TRANSIENT < PERSISTENT.

For the purpose of implementing the DURABILITY QoS kind TRANSIENT or PERSISTENT, the service behaves “as if” for each \ref DCPS_Modules_TopicDefinition "Topic" that has TRANSIENT or PERSISTENT DURABILITY kind there was a corresponding “built-in” \ref DCPS_Modules_Subscription_DataReader "DataReader" and \ref DCPS_Modules_Publication_DataWriter "DataWriter" configured to have the same DURABILITY kind.
In other words, it is “as if” somewhere in the system (possibly on a remote node) there was a “built-in durability \ref DCPS_Modules_Subscription_DataReader "DataReader"” that subscribed to that \ref DCPS_Modules_TopicDefinition "Topic" and a “built-in durability \ref DCPS_Modules_Publication_DataWriter "DataWriter"” that published that \ref DCPS_Modules_TopicDefinition "Topic" as needed for the new subscribers that join the system.

For each \ref DCPS_Modules_TopicDefinition "Topic", the built-in fictitious “persistence service” \ref DCPS_Modules_Subscription_DataReader "DataReader" and \ref DCPS_Modules_Publication_DataWriter "DataWriter" has its QoS configured from the \ref DCPS_Modules_TopicDefinition "Topic" QoS of the corresponding \ref DCPS_Modules_TopicDefinition "Topic". In other words, it is “as-if” the service first did find_topic to access the \ref DCPS_Modules_TopicDefinition "Topic", and then used the QoS from the \ref DCPS_Modules_TopicDefinition "Topic" to configure the fictitious built-in entities.
A consequence of this model is that the transient or persistence serviced can be configured by means of setting the proper QoS on the \ref DCPS_Modules_TopicDefinition "Topic". For a given \ref DCPS_Modules_TopicDefinition "Topic", the usual request/offered semantics apply to the matching between any \ref DCPS_Modules_Publication_DataWriter "DataWriter" in the system that writes the \ref DCPS_Modules_TopicDefinition "Topic" and the built-in transient/persistent \ref DCPS_Modules_Subscription_DataReader "DataReader" for that \ref DCPS_Modules_TopicDefinition "Topic"; similarly for the built-in transient/persistent \ref DCPS_Modules_Publication_DataWriter "DataWriter" for a \ref DCPS_Modules_TopicDefinition "Topic" and any \ref DCPS_Modules_Subscription_DataReader "DataReader" for the \ref DCPS_Modules_TopicDefinition "Topic".
As a consequence, a \ref DCPS_Modules_Publication_DataWriter "DataWriter" that has an incompatible QoS with respect to what the \ref DCPS_Modules_TopicDefinition "Topic" specified will not send its data to the transient/persistent service, and a \ref DCPS_Modules_Subscription_DataReader "DataReader" that has an incompatible QoS with respect to the specified in the \ref DCPS_Modules_TopicDefinition "Topic" will not get data from it. Incompatibilities between local \ref DCPS_Modules_Subscription_DataReader "DataReader"/\ref DCPS_Modules_Publication_DataWriter "DataWriter" entities and the corresponding fictitious “built-in transient/persistent entities” cause the REQUESTED_INCOMPATIBLE_QOS/OFFERED_INCOMPATIBLE_QOS status to change and the corresponding Listener invocations and/or signalling of Condition and WaitSet objects as they would with non-fictitious entities.

The setting of the service_cleanup_delay controls when the TRANSIENT or PERSISTENT service is able to remove all information regarding a data-instances. Information on a data-instances is maintained until the following conditions are
met:

-# the instance has been explicitly disposed (instance_state = NOT_ALIVE_DISPOSED),
-# and while in the NOT_ALIVE_DISPOSED state the system detects that there are no more “live” \ref DCPS_Modules_Publication_DataWriter "DataWriter" entities writing the instance, that is, all existing writers either unregister the instance (call unregister) or lose their
liveliness,
-# and a time interval longer that service_cleanup_delay has elapsed since the moment the service detected that the
previous two conditions were met.

The utility of the service_cleanup_delay is apparent in the situation where an application disposes an instance and it crashes before it has a chance to complete additional tasks related to the disposition. Upon restart the application may ask for initial data to regain its state and the delay introduced by the service_cleanup_delay will allow the restarted application to receive the information on the disposed instance and complete the interrupted tasks.
