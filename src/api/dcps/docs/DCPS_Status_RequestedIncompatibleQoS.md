Requested Incompatible QoS Status              {#DCPS_Status_RequestedIncompatibleQoS}
=================================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative number of times the concerned DataReader
            discovered a DataWriter for the same Topic with an offered QoS that
            was incompatible with that requested by the DataReader.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The change in total_count since the last time the listener was called or
            the status was read.</td>
    </tr>
    <tr>
        <td>last_policy_id</td>
        <td>The QosPolicyId_t of one of the policies that was found to be
            incompatible the last time an incompatibility was detected.</td>
    </tr>
    <tr>
        <td>policies</td>
        <td>A list containing for each policy the total number of times that the
            concerned DataReader discovered a DataWriter for the same Topic
            with an offered QoS that is incompatible with that requested by the
            DataReader.</td>
    </tr>
</table>

A QoS policy requested is incompatible with the offered QoS policy by DataWriter.
