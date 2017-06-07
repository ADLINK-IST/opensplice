Requested Deadline Missed Status              {#DCPS_Status_RequestedDeadlineMissed}
=================================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative number of missed deadlines detected for any instance
            read by the DataReader. Missed deadlines accumulate; that is, each
            deadline period the total_count will be incremented by one for each
            instance for which data was not received.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The incremental number of deadlines detected since the last time the
            listener was called or the status was read.</td>
    </tr>
    <tr>
        <td>last_instance_handle</td>
        <td>Handle to the last instance in the DataReader for which a deadline was
            detected.</td>
    </tr>
</table>

The deadline that the DataReader was expecting through its QoS policy was not respected for a specific instance.
