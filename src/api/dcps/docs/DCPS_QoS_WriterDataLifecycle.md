Writer Data Lifecycle QoS              {#DCPS_QoS_WriterDataLifecycle}
=========================

This QosPolicy drives the behaviour of a \ref DCPS_Modules_Publication_DataWriter "DataWriter" concerning the life-cycle of
the instances and samples that have been written by it.

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
            A boolean:<br/>
            autodispose_unregistered_instances
        </td>
        <td>
            Specifies the behavior of the
            \ref DCPS_Modules_Publication_DataWriter "DataWriter" with regards to the
            lifecycle of the data-instances it
            manages.
            Controls whether a \ref DCPS_Modules_Publication_DataWriter "DataWriter" will
            automatically dispose instances each
            time they are unregistered. The
            setting autodispose_unregistered_instances = TRUE indicates that
            unregistered instances will also be
            considered disposed. By default,
            TRUE.
        </td>
        <td rowspan="3">
             \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="3">N/A</td>
        <td rowspan="3">Yes</td>
    </tr>
    <tr>
        <td>
            A duration:<br/>
            autopurge_suspended_samples_delay
        </td>
        <td>
            Specifies the duration after which the \ref DCPS_Modules_Publication_DataWriter "DataWriter" will automatically remove a sample from its history during periods in which its \ref DCPS_Modules_Publication "Publisher" is suspended. This duration is calculated based on the source timestamp of the written sample. By default the duration value is set to DURATION_INFINITE and therefore no automatic purging of samples occurs.
        </td>
    </tr>
    <tr>
        <td>
            A duration:<br/>
            autounregister_instance_delay
        </td>
        <td>
            Specifies the duration after which the \ref DCPS_Modules_Publication_DataWriter "DataWriter" will automatically unregister an instance after the application wrote a sample for it and no further action is performed on the same instance by this \ref DCPS_Modules_Publication_DataWriter "DataWriter" afterwards. This means that when the application writes a new sample for this instance, the duration is recalculated from that action onwards. By default the duration value is DURATION_INFINITE and therefore no automatic unregistration occurs.
        </td>
    </tr>
</table>

This QosPolicy controls the behaviour of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" with regards to the lifecycle of the data-instances it manages; that is, those data-instances that have been registered, either explicitly using one of the register operations, or implicitly by directly writing the data using the special HANDLE_NIL parameter. The autodispose_unregistered_instances flag controls what happens when an instance gets unregistered by the \ref DCPS_Modules_Publication_DataWriter "DataWriter":

- If the \ref DCPS_Modules_Publication_DataWriter "DataWriter" unregisters the instance explicitly using either unregister_instance or unregister_instance_w_timestamp, then the autodispose_unregistered_instances flag is currently ignored and the instance is never disposed automatically.
- If the \ref DCPS_Modules_Publication_DataWriter "DataWriter" unregisters its instances implicitly because it is deleted, or if a \ref DCPS_Modules_Subscription_DataReader "DataReader" detects a loss of liveliness of a connected \ref DCPS_Modules_Publication_DataWriter "DataWriter", or if autounregister_instance_delay expires, then the autodispose_unregistered_instances flag determines whether the concerned instances are automatically disposed (TRUE) or not (FALSE).


The default value for the autodispose_unregistered_instances flag is TRUE. For TRANSIENT and PERSISTENT \ref DCPS_Modules_TopicDefinition "Topic"s this means that all instances that
are not explicitly unregistered by the application will by default be removed from the Transient and Persistent stores when the \ref DCPS_Modules_Publication_DataWriter "DataWriter" is deleted or when a loss of its liveliness is detected. For \ref DCPS_Modules_Publication_DataWriter "DataWriter"s associated with TRANSIENT and PERSISTENT \ref DCPS_Modules_TopicDefinition "Topic"s setting the autodispose_unregister_instances attribute to TRUE would mean that all instances that are not explicitly unregistered by the application will by default be removed from the Transient and Persistent stores when the \ref DCPS_Modules_Publication_DataWriter "DataWriter" is deleted, when a loss of liveliness is detected, or when the
autounregister_instance_delay expires.


