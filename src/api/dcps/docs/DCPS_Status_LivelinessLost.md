Liveliness Lost Status              {#DCPS_Status_LivelinessLost}
======================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative number of times that a previously-alive DataWriter
            became not alive due to a failure to actively signal its liveliness within
            its offered liveliness period. This count does not change when an
            already not alive DataWriter simply remains not alive for another
            liveliness period.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The change in total_count since the last time the listener was called or
            the status was read.</td>
    </tr>
</table>

The liveliness of the DataWriter set by the QoS policy is not respected and DataReader entities will consider
the DataWriter as "inactive"
