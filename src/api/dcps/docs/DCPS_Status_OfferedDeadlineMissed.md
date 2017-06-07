Offered Deadline Missed              {#DCPS_Status_OfferedDeadlineMissed}
=======================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative number of offered deadline periods elapsed during
            which a DataWriter failed to provide data. Missed deadlines
            accumulate; that is, each deadline period the total_count will be
            incremented by one.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The change in total_count since the last time the listener was called or
            the status was read.</td>
    </tr>
    <tr>
        <td>last_instance_handle</td>
        <td>Handle to the last instance in the DataWriter for which an offered
            deadline was missed.</td>
    </tr>
</table>

The deadline QoS set by the DataWriter was not respected for a specific instance
