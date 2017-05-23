Time Based Filter QoS              {#DCPS_QoS_TimeBasedFilter}
=====================

This QosPolicy specifies a period after receiving a sample for a particular instance
during which a \ref DCPS_Modules_Subscription_DataReader "DataReader" will drop new samples for the same instance.Effectively the \ref DCPS_Modules_Subscription_DataReader "DataReader" will receive at most one sample per period for each
instance.

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
            minimum_separation
        </td>
        <td>
            Filter that allows a \ref DCPS_Modules_Subscription_DataReader "DataReader" to
            specify that it is interested only in
            (potentially) a subset of the values
            of the data. The filter states that the
            \ref DCPS_Modules_Subscription_DataReader "DataReader" does not want to
            receive more than one value each
            minimum_separation period, regardless of
            how fast the changes occur.
            At the end of the period the latest state
            of the instance will be notified and a
            new filter period will start.
            In the case there are no new samples
            in a period the filter will not notify
            the same latest already notified state and
            starts waiting for a new sample on this
            particular instance to start a new period.
            In the case where the reliability QoS kind
            is RELIABLE the system guarantees that the
            latest state is notified.
            Effectively the \ref DCPS_Modules_Subscription_DataReader "DataReader" will receive at
            most one sample with the latest state per
            period for each instance.
            It is inconsistent for a \ref DCPS_Modules_Subscription_DataReader "DataReader" to
            have a minimum_separation longer
            than its DEADLINE period. By
            default minimum_separation=0
            indicating \ref DCPS_Modules_Subscription_DataReader "DataReader" is potentially
            interested in all values.
        </td>
        <td>\ref DCPS_Modules_Subscription_DataReader "DataReader"</td>
        <td>N/A</td>
        <td>Yes</td>
    </tr>
</table>

This policy allows a \ref DCPS_Modules_Subscription_DataReader "DataReader" to indicate that it does not necessarily want to see all values of each instance published under the \ref DCPS_Modules_TopicDefinition "Topic". Rather, it wants to see at most one change every minimum_separation period.

The TIME_BASED_FILTER applies to each instance separately, that is, the constraint is that the \ref DCPS_Modules_Subscription_DataReader "DataReader" does not want to see more than one sample of each instance per minimum_separation period.

This setting allows a \ref DCPS_Modules_Subscription_DataReader "DataReader" to further decouple itself from the \ref DCPS_Modules_Publication_DataWriter "DataWriter" objects. It can be used to protect applications that are running on a heterogeneous network where some nodes are capable of generating data much faster than others can consume it. It also accommodates the fact that for fast-changing data different \ref DCPS_Modules_Subscription "Subscriber"s may have different requirements as to how frequently they need to be notified of the most current values.

The setting of a TIME_BASED_FILTER, that is, the selection of a minimum_separation with a value greater than zero is compatible with all settings of the HISTORY and RELIABILITY QoS. The TIME_BASED_FILTER specifies the samples that are of interest to the \ref DCPS_Modules_Subscription_DataReader "DataReader". The HISTORY and RELIABILITY QoS affect the behaviour of the middleware with respect to the samples that have been determined to be of interest to the \ref DCPS_Modules_Subscription_DataReader "DataReader", that is, they apply after the TIME_BASED_FILTER has been applied.

In the case where the reliability QoS kind is RELIABLE then in steady-state, defined as the situation where the \ref DCPS_Modules_Publication_DataWriter "DataWriter" does not write new samples for a period “long” compared to the minimum_separation, the system should guarantee delivery the last sample to the \ref DCPS_Modules_Subscription_DataReader "DataReader".

The setting of the TIME_BASED_FILTER minimum_separation must be consistent with the DEADLINE period. For these two QoS policies to be consistent they must verify that “period >= minimum_separation.” An attempt to set these policies in an inconsistent manner when an entity is created via a set_qos operation will cause the operation to fail.
