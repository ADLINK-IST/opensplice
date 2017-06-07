Liveliness Changed Status              {#DCPS_Status_LivelinessChanged}
=========================


<table>
    <tr>
        <th>Attribute</th>
        <th>Attribute Meaning</th>
    </tr>
    <tr>
        <td>alive_count</td>
        <td>The total number of currently active DataWriters that write the Topic read by the DataReader.
            This count increases when a newly matched DataWriter asserts its liveliness for the first time
            or when a DataWriter previously considered to be not alive reasserts its liveliness. The count
            decreases when a DataWriter considered alive fails to assert its liveliness and becomes not alive,
            whether because it was deleted normally or for some other reason.</td>
    </tr>
    <tr>
        <td>not_alive_count</td>
        <td>The total count of currently DataWriters that write the Topic read by the DataReader that are no
            longer asserting their liveliness. This count increases when a DataWriter considered alive fails
            to assert its liveliness and becomes not alive for some reason other than the normal deletion of
            that DataWriter. It decreases when a previously not alive DataWriter either reasserts its liveliness
            or is deleted normally.
        </td>
    </tr>
    <tr>
        <td>alive_count_change</td>
        <td>The change in the alive_count since the last time the listener was called or the status was read.</td>
    </tr>
    <tr>
        <td>not_alive_count_change</td>
        <td>The change in the not_alive_count since the last time the listener was called or the status was read.</td>
    </tr>
    <tr>
        <td>last_publication_handle</td>
        <td>Handle to the last DataWriter whose change in liveliness caused this status to change.</td>
    </tr>
</table>

The liveliness of one or more **DataWriter** that were writing instances have become "active" or "inactive"
