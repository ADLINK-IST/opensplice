Sample Rejected Status              {#DCPS_Status_SampleRejected}
======================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative count of samples rejected by the DataReader.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>
            The incremental number of samples rejected since the last time the listener was called or the status was read.
        </td>
    </tr>
    <tr>
        <td>last_reason</td>
        <td>
            Reason for rejecting the last sample rejected. If no samples have been rejected, the reason is the special value NOT_REJECTED.
        </td>
    </tr>
    <tr>
        <td>last_instance_handle</td>
        <td>
            Handle to the instance being updated by the last sample that was rejected.
        </td>
    </tr>
</table>

A received sample was rejected. This can happen when the ResourceLimits Qos is active and the History determined by History QoS of
the DataReader is full.
