Ownership Strength QoS              {#DCPS_QoS_OwnershipStrength}
======================

This QosPolicy specifies the value of the ownership strength of a \ref DCPS_Modules_Publication_DataWriter "DataWriter"
used to determine the ownership of an instance.

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
            A long:<br/>
            value
        </td>
        <td>
            Specifies the value of the
            "strength" used to arbitrate among
             multiple \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects that
             attempt to modify the same instance
             of a data-object(identified by Topic + key). This policy only applies if the OWNERSHIP QoS policy is of kind EXCLUSIVE. The default value of the ownership_strength is zero.
        </td>
        <td>
             \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td>N/A</td>
        <td>Yes</td>
    </tr>
</table>

This QoS policy should be used in combination with the OWNERSHIP policy. It only applies to the situation case where
OWNERSHIP kind is set to EXCLUSIVE. The value of the OWNERSHIP_STRENGTH is used to determine the ownership of a data-instance
(identified by the key). The arbitration is performed by the \ref DCPS_Modules_Subscription_DataReader "DataReader".


