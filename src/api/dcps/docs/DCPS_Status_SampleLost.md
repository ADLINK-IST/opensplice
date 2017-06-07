Sample Lost Status              {#DCPS_Status_SampleLost}
==================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative count of all samples lost across of instances of data
            published under the Topic.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>The incremental number of samples lost since the last time the listener
             was called or the status was read.</td>
    </tr>
</table>

A sample never reached the DataReader queue.
The following events can lead to a sample lost status:
<ul>
<li>When the Presentation QoS is used with coherentAccess set to true and ResourceLimits Qos is active
 a sample lost can occur when all resources are consumed by incomplete transactions. In order to prevent deadlocks the
 the current transaction is dropped which causes a SampleLost event.
</li>
<li>When the DestinationOrder QoS is set to BY_SOURCE_TIMESTAMP It can happen that older data is inserted after newer
data is already taken by the DataReader. In this case the older data is not inserted into the DataReader and a Sample Lost is reported.</li>
<li>When the networking service detects a gap between sequence numbers of incoming data it will report a Sample Lost event</li>
</ul>
