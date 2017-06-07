History QoS              {#DCPS_QoS_History}
===========

This QosPolicy controls which samples will be stored when the value of an
instance changes (one or more times) before it is finally communicated.

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
            A HistoryQosPolicyKind:<br/>
            kind
        </td>
        <td>
            Specifies specifies the type of history, which may be KEEP_LAST or KEEP_ALL.
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
            KEEP_LAST
            and optional
            long "depth"
        </td>
        <td>
            On the publishing side, the Service
            will only attempt to keep the most
            recent “depth” samples of each
            instance of data (identified by its
            key) managed by the \ref DCPS_Modules_Publication_DataWriter "DataWriter".
            On the subscribing side, the
            \ref DCPS_Modules_Subscription_DataReader "DataReader" will only attempt to
            keep the most recent “depth”
            samples received for each instance
            (identified by its key) until the
            application “takes” them via the
            \ref DCPS_Modules_Subscription_DataReader "DataReader"’s take operation.
            KEEP_LAST is the default kind.
            The default value of depth is 1.
            If a value other than 1 is specified, it
            should be consistent with the
            settings of the
            RESOURCE_LIMITS QoS policy.
        </td>
    </tr>
    <tr>
        <td>
            KEEP_ALL
        </td>
        <td>
            On the publishing side, the Service
            will attempt to keep all samples
            (representing each value written) of
            each instance of data (identified by
            its key) managed by the \ref DCPS_Modules_Publication_DataWriter "DataWriter"
            until they can be delivered to all
            subscribers. On the subscribing side,
            the Service will attempt to keep all
            samples of each instance of data
            (identified by its key) managed by
            the \ref DCPS_Modules_Subscription_DataReader "DataReader". These samples are
            kept until the application “takes”
            them from the Service via the take
            operation. The setting of depth has
            no effect. Its implied value is
            LENGTH_UNLIMITED.
        </td>
    </tr>
</table>

-# This policy controls the behaviour of the Service when the value of an instance changes before it is finally communicated to some of its existing \ref DCPS_Modules_Subscription_DataReader "DataReader" entities.

-# If the kind is set to KEEP_LAST, then the Service will only attempt to keep the latest values of the instance and discard the older ones. In this case, the value of depth regulates the maximum number of values (up to and
including the most current one) the Service will maintain and deliver. The default (and most common setting) for depth is one, indicating that only the most recent value should be delivered.

-# If the kind is set to KEEP_ALL, then the Service will attempt to maintain and deliver all the values of the instance to existing subscribers. The resources that the Service can use to keep this history are limited by the settings of the RESOURCE_LIMITS QoS. If the limit is reached, then the behaviour of the Service will depend on the RELIABILITY QoS.
If the reliability kind is BEST_EFFORT, then the old values will be discarded. If reliability is RELIABLE, then the Service will block the \ref DCPS_Modules_Publication_DataWriter "DataWriter" until it can deliver the necessary old values to all subscribers.

The setting of HISTORY depth must be consistent with the RESOURCE_LIMITS max_samples_per_instance. For these two QoS to be consistent, they must verify that depth <= max_samples_per_instance.
