Lifespan QoS              {#DCPS_QoS_Lifespan}
============

This QosPolicy specifies the duration of the validity of the data written by the
DataWriter.

Attributes
----------
<table>
    <tr>
        <th>Value</th>
        <th>Meaning</th>
        <th>Concerns</th>
        <th>RxO</th>
        <th>Changeable</th>
    </tr>
    <tr>
        <td>
            A duration:<br/>
            duration
        </td>
        <td>
            Specifies the maximum duration of
            validity of the data written by the
            \ref DCPS_Modules_Publication_DataWriter "DataWriter". The default value of the
            lifespan duration is infinite.
        </td>
        <td>
             \ref DCPS_Modules_TopicDefinition "Topic",
             \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td>N/A</td>
        <td>Yes</td>
    </tr>
</table>

The purpose of this QoS is to avoid delivering "stale" data to the application.
Each data sample written by the \ref DCPS_Modules_Publication_DataWriter "DataWriter" has an associated ‘expiration time’ beyond which the data should not be
delivered to any application. Once the sample expires, the data will be removed from the DataReader caches as well as
from the transient and persistent information caches.
The ‘expiration time’ of each sample is computed by adding the duration specified by the LIFESPAN QoS to the source
timestamp.
This QoS relies on the sender and receiving applications having their clocks sufficiently synchronized. If this is not the
case and the Service can detect it, the DataReader is allowed to use the reception timestamp instead of the source
timestamp in its computation of the ‘expiration time.’

