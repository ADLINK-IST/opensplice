Transport Priority QoS              {#DCPS_QoS_TransportPriority}
======================

This QosPolicy specifies the priority with which the Data Distribution System can
handle the data produced by the \ref DCPS_Modules_Publication_DataWriter "DataWriter".

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
            This policy is a hint to the
            infrastructure as to how to set the
            priority of the underlying transport
            used to send the data. The default
            value of the transport_priority is
            zero.
        </td>
        <td>
             \ref DCPS_Modules_TopicDefinition "Topic",
             \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td>N/A</td>
        <td>Yes</td>
    </tr>
</table>

The purpose of this QoS is to allow the application to take advantage of transports capable of sending messages with
different priorities.
This policy is considered a hint. The policy depends on the ability of the underlying transports to set a priority on the
messages they send. Any value within the range of a 32-bit signed integer may be chosen; higher values indicate higher
priority. However, any further interpretation of this policy is specific to a particular transport and a particular
implementation of the Service.It is expected that during transport configuration the application would provide a mapping
between the values of the TRANSPORT_PRIORITY set on \ref DCPS_Modules_Publication_DataWriter "DataWriter" and the values meaningful to each transport. This
mapping would then be used by the infrastructure when propagating the data written by the \ref DCPS_Modules_Publication_DataWriter "DataWriter".


