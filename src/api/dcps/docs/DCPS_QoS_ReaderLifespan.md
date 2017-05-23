ReaderLifespan QoS              {#DCPS_QoS_ReaderLifespan}
==================

This QosPolicy automatically remove samples from the \ref DCPS_Modules_Subscription_DataReader "DataReader" after a specified timeout.


*NOTE:* This is an OpenSplice-specific QosPolicy, it is not part of the DDS Specification.

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
            A boolean: <br/>
            use_lifespan
        </td>
        <td>Controls whether the lifespan is applied to the samples in the \ref DCPS_Modules_Subscription_DataReader "DataReader".</td>
        <td rowspan="2">\ref DCPS_Modules_Subscription_DataReader "DataReader"</td>
        <td rowspan="2">N/A</td>
        <td rowspan="2">Yes</td>
    </tr>
    <tr>
        <td>
            A duration: <br/>
            duration
        </td>
        <td>The duration after which data loses validity and is removed.</td>
    </tr>
</table>

This QosPolicy is similar to the \ref DCPS_QoS_Lifespan "LifespanQosPolicy" (applicable to \ref DCPS_Modules_TopicDefinition "Topic" and
\ref DCPS_Modules_Publication_DataWriter "DataWriter"), but limited to the \ref DCPS_Modules_Subscription_DataReader "DataReader" on which the QosPolicy is applied. The
data is automatically removed from the \ref DCPS_Modules_Subscription_DataReader "DataReader" if it has not been taken yet after
the lifespan duration expires. The duration of the ReaderLifespan is added to the
insertion time of the data in the \ref DCPS_Modules_Subscription_DataReader "DataReader" to determine the expiry time.


When both the ReaderLifespanQosPolicy and a \ref DCPS_Modules_Publication_DataWriter "DataWriter"’s \ref DCPS_QoS_Lifespan "LifespanQosPolicy" are applied to the same data,
only the earliest expiry time is taken into account.


By default, the ReaderLifespanQosPolicy is not used and use_lifespan is FALSE.
The duration is set to DURATION_INFINITE.


This QosPolicy is applicable to a \ref DCPS_Modules_Subscription_DataReader "DataReader" only, and is mutable even when the
\ref DCPS_Modules_Subscription_DataReader "DataReader" is already enabled. If modified, the new setting will only be applied to
samples that are received after the modification took place.