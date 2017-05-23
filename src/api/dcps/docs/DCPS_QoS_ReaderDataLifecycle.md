Reader Data Lifecycle QoS              {#DCPS_QoS_ReaderDataLifecycle}
=========================

This QosPolicy specifies the maximum duration for which the \ref DCPS_Modules_Subscription_DataReader "DataReader" will
maintain information regarding a data instance for which the instance_state
becomes either NOT_ALIVE_NO_WRITERS_INSTANCE_STATE or NOT_ALIVE_DISPOSED_INSTANCE_STATE.

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
            autopurge_nowriter_samples_delay
        </td>
        <td>
            specifies the duration for which the \ref DCPS_Modules_Subscription_DataReader "DataReader" will maintain information regarding a data instance for which the instance_state becomes NOT_ALIVE_NO_WRITERS_INSTANCE_STATE. By default the duration value is DURATION_INFINITE. When the delay time has expired, the data instance is marked so that it can be purged in the next garbage collection sweep.
        </td>
        <td rowspan="5">\ref DCPS_Modules_Subscription_DataReader "DataReader"</td>
        <td rowspan="5">N/A</td>
        <td rowspan="5">Yes</td>
    </tr>
    <tr>
        <td>
            A duration:<br/>
            autopurge_disposed_samples_delay
        </td>
        <td>
            specifies the duration for which the \ref DCPS_Modules_Subscription_DataReader "DataReader" will maintain information regarding a data instance for which the instance_state becomes NOT_ALIVE_DISPOSED_INSTANCE_STATE. By default the duration value is DURATION_INFINITE. When the delay time has expired, the data instance is marked so that it can be purged in the next garbage collection sweep.
        </td>
    </tr>
    <tr>
        <td>
            A boolean:<br/>
            autopurge_dispose_all
        </td>
        <td>
            Determines whether all samples in the \ref DCPS_Modules_Subscription_DataReader "DataReader" will be purged automatically when a dispose_all_data() call is performed on the \ref DCPS_Modules_TopicDefinition "Topic" that is associated with the \ref DCPS_Modules_Subscription_DataReader "DataReader". If this attribute set to TRUE, no more samples will exist in the \ref DCPS_Modules_Subscription_DataReader "DataReader" after the dispose_all_data has been processed. Because all samples are purged, no data available events will be notified to potential Listeners or Conditions that are set for the \ref DCPS_Modules_Subscription_DataReader "DataReader". If this attribute is set to FALSE, the dispose_all_data() behaves as if each individual instance was disposed separately.
        </td>
    </tr>
    <tr>
        <td>
            A boolean:<br/>
            enable_invalid_samples
        </td>
        <td>
           Insert dummy samples if no data sample is available to notify readers of an instance state change. By default the value is TRUE. <br/><b>NOTE:</b> This feature is deprecated. It is recommended that you use invalid_sample_visibility instead
        </td>
    </tr>
    <tr>
        <td>
            An InvalidSampleVisibilityQosPolicy:<br/>
            invalid_sample_visibility
        </td>
        <td>
          Insert dummy samples if no data sample is available, to notify readers of an instance state change. By default the value is MINIMUM_INVALID_SAMPLES. Options are NO_INVALID_SAMPLES, MINIMUM_INVALID_SAMPLES, ALL_INVALID_SAMPLES
        </td>
    </tr>
</table>

This QosPolicy specifies the maximum duration for which the \ref DCPS_Modules_Subscription_DataReader "DataReader" will maintain information regarding a data instance for which the
instance_state becomes either NOT_ALIVE_NO_WRITERS_INSTANCE_STATE or NOT_ALIVE_DISPOSED_INSTANCE_STATE. The \ref DCPS_Modules_Subscription_DataReader "DataReader" manages resources
for instances and samples of those instances. The amount of resources managed
depends on other QosPolicies like the HistoryQosPolicy and the ResourceLimitsQosPolicy. The \ref DCPS_Modules_Subscription_DataReader "DataReader" can only release resources for data instances for which all samples have been taken and the instance_state has become NOT_ALIVE_NO_WRITERS_INSTANCE_STATE or NOT_ALIVE_DISPOSED_INSTANCE_STATE. If an application does not take the samples belonging to a data instance with such an instance_state, the \ref DCPS_Modules_Subscription_DataReader "DataReader" will never be able to release the maintained resources. The application can use this QosPolicy to instruct the \ref DCPS_Modules_Subscription_DataReader "DataReader" to release all resources related to the relevant data instance after a specified duration.


There is one exception to this rule. If the autopurge_dispose_all attribute is TRUE, the maintained resources in the \ref DCPS_Modules_Subscription_DataReader "DataReader" are cleaned up immediately in case dispose_all_data() is called on the \ref DCPS_Modules_TopicDefinition "Topic" that is associated with the \ref DCPS_Modules_Subscription_DataReader "DataReader". Instance state changes are communicated to a
\ref DCPS_Modules_Subscription_DataReader "DataReader" by means of the SampleInfo accompanying a data sample. If no samples are available in the \ref DCPS_Modules_Subscription_DataReader "DataReader", a so-called ‘invalid sample’ can be injected with the sole purpose of notifying applications of the instance state. This behaviour is configured by the InvalidSampleVisibilityQosPolicy.


- If invalid_sample_visibility is set to NO_INVALID_SAMPLES, applications will be notified of instance_state changes only if there is a sample
available in the \ref DCPS_Modules_Subscription_DataReader "DataReader". The SampleInfo belonging to this sample will contain the updated instance state.
- If invalid_sample_visibility is set to MINIMUM_INVALID_SAMPLES, the middleware will try to update the instance_state on available samples in the
\ref DCPS_Modules_Subscription_DataReader "DataReader". If no sample is available, an invalid sample will be injected. These samples contain only the key values of the instance. The SampleInfo for invalid samples will have the ‘valid_data’ flag disabled, and contain the updated instance state.
- If invalid_sample_visibility is set to ALL_INVALID_SAMPLES, every change in the instance_state will be communicated by a separate invalid sample.
**NOTE:** This value (ALL_INVALID_SAMPLES) is not yet implemented. It is scheduled for a future release.


An alternative but deprecated way to determine the visibility of state changes is to set a boolean value for the enable_invalid_samples field.
- When TRUE, the behavior is similar to the MINIMUM_INVALID_SAMPLES value of the InvalidSampleVisibilityQosPolicy field.
- When FALSE, the behavior is similar to the NO_INVALID_SAMPLES value of the InvalidSampleVisibilityQosPolicy field.


You cannot set both the the enable_invalid_samples field AND the invalid_sample_visibility field. If both deviate from their factory default, this is considered a RETCODE_INCONSISTENT_POLICY. If only one of the fields deviates from its factory default, then that setting will be leading. However, modifying the default value of the enable_invalid_samples field will automatically result in a warning message stating that you are using deprecated functionality. This QosPolicy is applicable to a \ref DCPS_Modules_Subscription_DataReader "DataReader" only. After enabling the relevant \ref DCPS_Modules_Subscription_DataReader "DataReader", this QosPolicy can be changed using the set_qos operation.

