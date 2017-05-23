Destination Order QoS              {#DCPS_QoS_DestinationOrder}
=====================

This QosPolicy controls the order in which the \ref DCPS_Modules_Subscription_DataReader "DataReader" stores the data.

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
            A DestinationOrderQosPolicyKind:<br/>
            kind
        </td>
        <td>
            controls the order in which the \ref DCPS_Modules_Subscription_DataReader "DataReader" stores the data. This can be BY_RECEPTION_TIMESTAMP or BY_SOURCE_TIMESTAMP.
        </td>
        <td rowspan="3">
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="3">Yes</td>
        <td rowspan="3">No</td>
    </tr>
    <tr>
        <td>
            BY_RECEPTION_TIMESTAMP
        </td>
        <td>
            Indicates that data is ordered based
            on the reception time at each
            Subscriber. Since each subscriber
            may receive the data at different
            times there is no guaranteed that the
            changes will be seen in the same
            order. Consequently, it is possible
            for each subscriber to end up with a
            different final value for the data.
        </td>
    </tr>
    <tr>
        <td>
            BY_SOURCE_TIMESTAMP
        </td>
        <td>
            Indicates that data is ordered based
            on a timestamp placed at the source
            (by the Service or by the
            application). In any case this
            guarantees a consistent final value
            for the data in all subscribers.
        </td>
    </tr>
</table>

This policy controls how each subscriber resolves the final value of a data instance that is written by multiple \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects (which may be associated with different \ref DCPS_Modules_Publication "Publisher" objects) running on different nodes.

The setting BY_RECEPTION_TIMESTAMP indicates that, assuming the OWNERSHIP policy allows it, the latest received value for the instance should be the one whose value is kept. This is the default value.

The setting BY_SOURCE_TIMESTAMP indicates that, assuming the OWNERSHIP policy allows it, a timestamp placed at the source should be used. This is the only setting that, in the case of concurrent same-strength \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects updating the same instance, ensures all subscribers will end up with the same final value for the instance. The mechanism to set the source timestamp is middleware dependent.

The value offered is considered compatible with the value requested if and only if the inequality “offered kind >= requested kind” evaluates to ‘TRUE.’ For the purposes of this inequality, the values of DESTINATION_ORDER kind are considered ordered such that BY_RECEPTION_TIMESTAMP < BY_SOURCE_TIMESTAMP.
