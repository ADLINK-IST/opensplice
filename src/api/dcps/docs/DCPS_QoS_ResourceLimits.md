Resource Limits QoS              {#DCPS_QoS_ResourceLimits}
===================

This QosPolicy will specify the maximum amount of resources, which can be used
by a \ref DCPS_Modules_Publication_DataWriter "DataWriter" or \ref DCPS_Modules_Subscription_DataReader "DataReader".

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
            A long:<br/>
            max_samples</td>
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
        <td rowspan="3">
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="3">No</td>
        <td rowspan="3">No</td>
    </tr>
    <tr>
        <td>
            A long:<br/>
            max_instances</td>
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
            max_samples_per_instance</td>
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

This policy controls the resources that the Service can use in order to meet the requirements imposed by the application and other QoS settings.

If the \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects are communicating samples faster than they are ultimately taken by the \ref DCPS_Modules_Subscription_DataReader "DataReader" objects, the middleware will eventually hit against some of the QoS-imposed resource limits. Note that this may occur when just a single \ref DCPS_Modules_Subscription_DataReader "DataReader" cannot keep up with its corresponding \ref DCPS_Modules_Publication_DataWriter "DataWriter". The behaviour in this case depends on the setting for the RELIABILITY QoS. If reliability is BEST_EFFORT, then the Service is allowed to drop samples. If the reliability is RELIABLE, the Service will block the \ref DCPS_Modules_Publication_DataWriter "DataWriter" or discard the sample at the \ref DCPS_Modules_Subscription_DataReader "DataReader" in order not to lose existing samples.

The constant LENGTH_UNLIMITED may be used to indicate the absence of a particular limit. For example setting max_samples_per_instance to LENGH_UNLIMITED will cause the middleware to not enforce this particular limit.

The setting of RESOURCE_LIMITS max_samples must be consistent with the max_samples_per_instance. For these two values to be consistent they must verify that “max_samples >= max_samples_per_instance.”

The setting of RESOURCE_LIMITS max_samples_per_instance must be consistent with the HISTORY depth. For these two QoS to be consistent, they must verify that “depth <= max_samples_per_instance.” An attempt to set this policy to inconsistent values when an entity is created via a set_qos operation will cause the
operation to fail.
