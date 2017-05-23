Publication Matched Status              {#DCPS_Status_PublicationMatched}
==========================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative count the concerned DataWriter discovered a
            “match” with a DataReader. That is, it found a DataReader for the
            same Topic with a requested QoS that is compatible with that offered
            by the DataWriter.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The change in total_count since the last time the listener was called or
            the status was read.</td>
    </tr>
    <tr>
        <td>last_subscription_handle</td>
        <td>Handle to the last DataReader that matched the DataWriter causing the
            status to change.</td>
    </tr>
     <tr>
        <td>current_count</td>
        <td>The number of DataReaders currently matched to the concerned
            DataWriter.</td>
     </tr>
     <tr>
        <td>current_count_change</td>
        <td>The change in current_count since the last time the listener was called
            or the status was read.</td>
     </tr>
</table>

The DataWriter has found a DataReader that matches the Topic and has compatible QoS or
ceased to be matched with a DataReader.
