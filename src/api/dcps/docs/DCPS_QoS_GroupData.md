Group Data QoS              {#DCPS_QoS_GroupData}
==============

This QosPolicy allows the application to attach additional information to a
\ref DCPS_Modules_Publication "Publisher" or \ref DCPS_Modules_Subscription "Subscriber" Entity. This information is distributed with the
BuiltinTopics.

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
            A sequence of octects:<br/>
            value
        </td>
        <td>
            User data not known by the
            middleware, but distributed by
            means of built-in topics.
            The default value is an empty (zero-
            sized) sequence.
        </td>
        <td>
            \ref DCPS_Modules_Publication "Publisher",
            \ref DCPS_Modules_Subscription "Subscriber"
        </td>
        <td>No</td>
        <td>Yes</td>
    </tr>
</table>

The purpose of this QoS is to allow the application to attach additional information to the created \ref DCPS_Modules_Publication "Publisher" or \ref DCPS_Modules_Subscription "Subscriber". The value of the GROUP_DATA is available to the application on the \ref DCPS_Modules_Subscription_DataReader "DataReader" and \ref DCPS_Modules_Publication_DataWriter "DataWriter" entities and is propagated by means of the built-in topics.

This QoS can be used by an application combination with the DataReaderListener and DataWriterListener to implement matching policies similar to those of the PARTITION QoS except the decision can be made based on an application-defined policy.
