Inconsistent Topic Status              {#DCPS_Status_InconsistentTopic}
======================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>total_count</td>
        <td>Total cumulative count of the Topics discovered whose name matches
            the Topic to which this status is attached and whose type is
            inconsistent with the Topic.</td>
    </tr>
    <tr>
        <td>total_count_change</td>
        <td>
            The incremental number of inconsistent topics discovered since the
            last time the listener was called or the status was read.
        </td>
    </tr>
</table>

Another topic exists with the same name but different characteristics.
