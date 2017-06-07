Subscription Matched Status              {#DCPS_Status_SubscriptionMatched}
===========================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative count the concerned DataReader discovered a
            “match” with a DataWriter. That is, it found a DataWriter for the
            same Topic with a requested QoS that is compatible with that offered
            by the DataReader.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The change in total_count since the last time the listener was called or
            the status was read.</td>
    </tr>
    <tr>
        <td>last_subscription_handle</td>
        <td>Handle to the last DataWriter that matched the DataReader causing the
            status to change.</td>
    </tr>
     <tr>
        <td>current_count</td>
        <td>The number of DataWriters currently matched to the concerned
            DataReader.</td>
     </tr>
     <tr>
        <td>current_count_change</td>
        <td>The change in current_count since the last time the listener was called
            or the status was read.</td>
     </tr>
</table>

The DataReader has found a DataWriter that matches the Topic and has compatible QoS or
ceased to be matched with a DataWriter.
