Topic Data QoS              {#DCPS_QoS_TopicData}
==============

This QosPolicy allows the application to attach additional information to a  \ref DCPS_Modules_TopicDefinition "Topic"
Entity. This information is distributed with the BuiltinTopics.

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
            A sequence of octets:<br/>
            value
        </td>
        <td>
            A sequence of octets that holds the application topic data. By default, the sequence has length 0.
        </td>
        <td>
             \ref DCPS_Modules_TopicDefinition "Topic"
        </td>
        <td>No</td>
        <td>Yes</td>
    </tr>
</table>

The purpose of this QoS is to allow the application to attach additional information to the created  \ref DCPS_Modules_TopicDefinition "Topic" such that when a remote application discovers their existence it can examine the information and use it in an application-defined way. In combination with the listeners on the \ref DCPS_Modules_Subscription_DataReader "DataReader" and \ref DCPS_Modules_Publication_DataWriter "DataWriter" as well as by means of operations such as ignore_topic, these QoS can assist an application to extend the provided QoS.
