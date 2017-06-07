Ownership QoS              {#DCPS_QoS_Ownership}
==================

This QosPolicy specifies whether a \ref DCPS_Modules_Publication_DataWriter "DataWriter" exclusively owns an instance.

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
            A OwnershipQosPolicyKind:<br/>
            kind
        </td>
        <td>Specifies whether it is allowed for multiple DataWriters to write the same instance of the data and if so , how these modifications should be arbitrated. This can be SHARED or EXCLUSIVE</td>
        <td rowspan="3">\ref DCPS_Modules_TopicDefinition "Topic", \ref DCPS_Modules_Subscription_DataReader "DataReader", \ref DCPS_Modules_Publication_DataWriter "DataWriter"</td>
        <td rowspan="3">Yes</td>
        <td rowspan="3">No</td>
    </tr>
    <tr>
        <td>
            SHARED
        </td>
        <td>
        Indicates shared ownership for each instance. Multiple writers are allowed to update the same instance and all the updates are made available to the readers. In other words there is no concept of an "owner" for the instances. This is the default behaviour
        </td>
    </tr>
    <tr>
        <td>
            EXCLUSIVE
        </td>
        <td>
            Indicates each instance can only be owned by one \ref DCPS_Modules_Publication_DataWriter "DataWriter", but the owner of an instance can change dynamically.
            The selection of the owner is controlled by the setting of the OWNERSHIP_STRENGTH QoS policy.
            The owner is always set to be the highest-strength \ref DCPS_Modules_Publication_DataWriter "DataWriter" object among the ones currently “active” (as determined by the LIVELINESS QoS).
        </td>
    </tr>
</table>

This policy controls whether the Service allows multiple \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects to update the same instance (identified by
\ref DCPS_Modules_TopicDefinition "Topic" + key) of a data-object.
There are two kinds of OWNERSHIP selected by the setting of the kind: SHARED and EXCLUSIVE.

SHARED kind
-------------
This setting indicates that the Service does not enforce unique ownership for each instance. In this case, multiple writers
can update the same data-object instance. The \ref DCPS_Modules_Subscription "Subscriber" to the \ref DCPS_Modules_TopicDefinition "Topic" will be able to access modifications from all
\ref DCPS_Modules_Publication_DataWriter "DataWriter" objects, subject to the settings of other QoS that may filter particular samples (e.g., the
TIME_BASED_FILTER or HISTORY QoS policy). In any case there is no “filtering” of modifications made based on the
identity of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" that causes the modification.



EXCLUSIVE kind
--------------
This setting indicates that each instance of a data-object can only be modified by one \ref DCPS_Modules_Publication_DataWriter "DataWriter". In other words, at any
point in time a single \ref DCPS_Modules_Publication_DataWriter "DataWriter" “owns” each instance and is the only one whose modifications will be visible to the
\ref DCPS_Modules_Subscription_DataReader "DataReader" objects. The owner is determined by selecting the \ref DCPS_Modules_Publication_DataWriter "DataWriter" with the highest value of the strength that is
both “alive” as defined by the LIVELINESS QoS policy and has not violated its DEADLINE contract with regards to the
data-instance. Ownership can therefore change as a result of (a) a \ref DCPS_Modules_Publication_DataWriter "DataWriter" in the system with a higher value of the
strength that modifies the instance, (b) a change in the strength value of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" that owns the instance, (c) a
change in the liveliness of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" that owns the instance, and (d) a deadline with regards to the instance that is
missed by the \ref DCPS_Modules_Publication_DataWriter "DataWriter" that owns the instance.

The behaviour of the system is as if the determination was made independently by each \ref DCPS_Modules_Subscription_DataReader "DataReader". Each \ref DCPS_Modules_Subscription_DataReader "DataReader"
may detect the change of ownership at a different time. It is not a requirement that at a particular point in time all the
\ref DCPS_Modules_Subscription_DataReader "DataReader" objects for that \ref DCPS_Modules_TopicDefinition "Topic" have a consistent picture of who owns each instance.
It is also not a requirement that the \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects are aware of whether they own a particular instance. There is no
error or notification given to a \ref DCPS_Modules_Publication_DataWriter "DataWriter" that modifies an instance it does not currently own.
The requirements are chosen to (a) preserve the decoupling of publishers and \ref DCPS_Modules_Subscription "Subscriber", and (b) allow the policy to be
implemented efficiently.

It is possible that multiple \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects with the same strength modify the same instance. If this occurs the Service
will pick one of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects as the “owner.” It is not specified how the owner is selected. However, it is
required that the policy used to select the owner is such that all \ref DCPS_Modules_Subscription_DataReader "DataReader" objects will make the same choice of the
particular \ref DCPS_Modules_Publication_DataWriter "DataWriter" that is the owner. It is also required that the owner remains the same until there is a change in
strength, liveliness, the owner misses a deadline on the instance, a new \ref DCPS_Modules_Publication_DataWriter "DataWriter" with higher strength modifies the
instance, or another \ref DCPS_Modules_Publication_DataWriter "DataWriter" with the same strength that is deemed by the Service to be the new owner modifies the
instance.

Exclusive ownership is on an instance-by-instance basis. That is, a \ref DCPS_Modules_Subscription "Subscriber" can receive values written by a lower
strength \ref DCPS_Modules_Publication_DataWriter "DataWriter" as long as they affect instances whose values have not been set by the higher-strength \ref DCPS_Modules_Publication_DataWriter "DataWriter".
The value of the OWNERSHIP kind offered must exactly match the one requested or else they are considered
incompatible.
