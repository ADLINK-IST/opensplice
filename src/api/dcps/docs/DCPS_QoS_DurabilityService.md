Durability Service QoS              {#DCPS_QoS_DurabilityService}
======================

This QosPolicy controls the behaviour of the durability service regarding transient
and persistent data.


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
            service_cleanup_delay
        </td>
        <td>
            Specifies how long the durability service must wait before it is allowed to remove the information on the transient or persistent topic data-instances as a result of incoming dispose messages.
        </td>
        <td rowspan="6">
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="6">No</td>
        <td rowspan="6">No</td>
    </tr>
    <tr>
        <td>
            A HistoryQosPolicyKind:<br/>
            history_kind
        </td>
        <td>
            Specifies the type of history, which may be KEEP_LAST_HISTORY_QOS or KEEP_ALL_HISTORY_QOS, the durability service must apply for the transient or persistent topic data-instances.
        </td>
    </tr>
    <tr>
        <td>
            A long:<br/>
            history_depth
        </td>
        <td>
            Specifies the number of samples of each instance of data (identified by its key) that is managed by the durability service for the transient or persistent topic data-instances. If history_kind is KEEP_LAST_HISTORY_QOS, history_depth must be smaller than or equal to max_samples_per_instance for this QosPolicy to be consistent.
        </td>
    </tr>
    <tr>
        <td>
            A long:<br/>
            max_samples
        </td>
        <td>
            Specifies the maximum number of
            data-samples the \ref DCPS_Modules_Publication_DataWriter "DataWriter" (or
            \ref DCPS_Modules_Subscription_DataReader "DataReader") can manage across all
            the instances associated with it.
            Represents the maximum samples
            the middleware can store for any
            one \ref DCPS_Modules_Publication_DataWriter "DataWriter" (or \ref DCPS_Modules_Subscription_DataReader "DataReader"). It
            is inconsistent for this value to be
            less than
            max_samples_per_instance.
            By default,
            LENGTH_UNLIMITED.
        </td>
    </tr>
    <tr>
        <td>
            A long:<br/>
            max_instances
        </td>
        <td>
            Represents the maximum number of
            instances \ref DCPS_Modules_Publication_DataWriter "DataWriter" (or
            \ref DCPS_Modules_Subscription_DataReader "DataReader") can manage.
            By default,
            LENGTH_UNLIMITED.
        </td>
    </tr>
    <tr>
        <td>
             A long:<br/>
             max_samples_per_instance
        </td>
        <td>
            Represents the maximum number of
            samples of any one instance a
            \ref DCPS_Modules_Publication_DataWriter "DataWriter" (or \ref DCPS_Modules_Subscription_DataReader "DataReader") can
            manage. It is inconsistent for this
            value to be greater than
            max_samples. By default,
            LENGTH_UNLIMITED.
        </td>
    </tr>
</table>

This QosPolicy controls the behaviour of the durability service regarding transient
and persistent data. It controls for the transient or persistent topic; the time at which
information regarding the topic may be discarded, the history policy it must set and
the resource limits it must apply.

The setting of the DurabilityServiceQosPolicy only applies when kind of the
DurabilityQosPolicy is either TRANSIENT_DURABILITY_QOS or PERSISTENT_DURABILITY_QOS.
The service_cleanup_delay setting controls at which time the durability service” is allowed to remove all information
regarding a data-instance. Information on a data-instance is maintained until the
following conditions are met:


- The instance has been explicitly disposed of (instance_state = NOT_ALIVE_DISPOSED_INSTANCE_STATE)
- The system detects that there are no more “live” \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects writing the instance, that is, all \ref DCPS_Modules_Publication_DataWriter "DataWriter" either unregister_instance the instance (call unregister_instance operation) or lose their liveliness
- A time interval longer than service_cleanup_delay has elapsed since the moment the Data Distribution Service detected that the previous two conditions were met.


The use of the attribute service_cleanup_delay is apparent in the situation where an application disposes of an instance and it crashes before having a chance to complete additional tasks related to the disposition. Upon re-start the application may ask for initial data to regain its state and the delay introduced by the service_cleanup_delay allows the re-started application to receive the information on the disposed of instance and complete the interrupted tasks.

The attributes history_kind and history_depth apply to the history settings of the durability service’s internal \ref DCPS_Modules_Publication_DataWriter "DataWriter" and \ref DCPS_Modules_Subscription_DataReader "DataReader" managing the topic. The HistoryQosPolicy behaviour, applies to these attributes.

The attributes max_samples, max_instances and max_samples_per_instance  apply to the resource limits of the Durability Service’s internal \ref DCPS_Modules_Publication_DataWriter "DataWriter" and \ref DCPS_Modules_Subscription_DataReader "DataReader" managing the topic. The ResourceLimitsQosPolicy behaviour, applies to these attributes.


