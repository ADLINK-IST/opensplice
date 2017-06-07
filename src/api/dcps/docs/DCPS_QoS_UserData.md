User Data QoS              {#DCPS_QoS_UserData}
==============

This QosPolicy allows the application to attach additional information to a
\ref DCPS_Modules_DomainParticipant "DomainParticipant", \ref DCPS_Modules_Subscription_DataReader "DataReader" or \ref DCPS_Modules_Publication_DataWriter "DataWriter" entity. This information is
distributed with the BuiltinTopics.

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
            A sequence of octets that holds the application user data. By default, the sequence has length 0.
        </td>
        <td>
            \ref DCPS_Modules_DomainParticipant "DomainParticipant",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td>No</td>
        <td>Yes</td>
    </tr>
</table>

This QosPolicy allows the application to attach additional information to a
\ref DCPS_Modules_DomainParticipant "DomainParticipant", \ref DCPS_Modules_Subscription_DataReader "DataReader" or \ref DCPS_Modules_Publication_DataWriter "DataWriter" entity. This information is
distributed with the Builtin Topics. An application that discovers a new
Entity ofthe listed kind, can use this information to add additional functionality. The
UserDataQosPolicy is changeable and updates of the Builtin Topic instance must
be expected. Note that the Data Distribution Service is not aware of the real
structure of the user data (the Data Distribution System handles it as an opaque
type) and that the application is responsible for correct mapping on structural types
for the specific platform.

