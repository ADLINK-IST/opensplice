SubscriptionKey QoS              {#DCPS_QoS_SubscriptionKey}
==================

This QosPolicy allows the \ref DCPS_Modules_Subscription_DataReader "DataReader" to define it's own set of keys on the data,
potentially different from the keys defined on the \ref DCPS_Modules_TopicDefinition "Topic".

*NOTE:* This is an OpenSplice-specific QosPolicy, it is not part of the DDS
Specification.

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
            use_key_list
        </td>
        <td>Controls whether the alternative key list is applied on the \ref DCPS_Modules_Subscription_DataReader "DataReader".</td>
        <td rowspan="2">\ref DCPS_Modules_Subscription_DataReader "DataReader"</td>
        <td rowspan="2">N/A</td>
        <td rowspan="2">No</td>
    </tr>
    <tr>
        <td>
            A string sequence: <br/>
            key_list
        </td>
        <td>A sequence of strings with one or more names of \ref DCPS_Modules_TopicDefinition "Topic" fields acting as alternative keys.</td>
    </tr>
</table>

By using the SubscriptionKeyQosPolicy, a \ref DCPS_Modules_Subscription_DataReader "DataReader" can force its own key-list
definition on data samples. The consequences are that the \ref DCPS_Modules_Subscription_DataReader "DataReader" will
internally keep track of instances based on its own key list, instead of the key list
dictated by the \ref DCPS_Modules_TopicDefinition "Topic".

Operations that operate on instances or instance handles, such as
lookup_instance or get_key_value, respect the alternative key-list and work
as expected. However, since the mapping of writer instances to reader instances is
no longer trivial (one writer instance may now map to more than one matching
reader instance and vice versa), a writer instance will no longer be able to fully
determine the lifecycle of its matching reader instance, nor the value its
view_state and instance_state.

In fact, by diverting from the conceptual 1 – 1 mapping between writer instance and
reader instance, the writer can no longer keep an (empty) reader instance
ALIVE by just refusing to unregister its matching writer instance. That means that when a
reader takes all samples from a particular reader instance, that reader instance will
immediately be removed from the reader’s administration. Any subsequent
reception of a message with the same keys will re-introduce the instance into the
reader administration, setting its view_state back to NEW. Compare this to the
default behaviour, where the reader instance will be kept alive as long as the writer
does not unregister it. That causes the view_state in the reader instance to remain
NOT_NEW, even if the reader has consumed all of its samples prior to receiving an
update.

Another consequence of allowing an alternative keylist is that events that are
communicated by invalid samples (i.e. samples that have only initialized their
keyfields) may no longer be interpreted by the reader to avoid situations in which
uninitialized non-keyfields are treated as keys in the alternative keylist. This
effectively means that all invalid samples (e.g. unregister messages and both
implicit and explicit dispose messages) will be skipped and can no longer affect the
instance_state, which will therefore remain ALIVE. The only exceptions to this
are the messages that are transmitted explicitly using the
writedispose() call, which always includes a full and valid sample and can therefore modify the
instance_state to NOT_ALIVE_DISPOSED.

By default, the SubscriptionKeyQosPolicy is not used because use_key_list is set to FALSE.
This QosPolicy is applicable to a \ref DCPS_Modules_Subscription_DataReader "DataReader" only, and cannot be changed after the
\ref DCPS_Modules_Subscription_DataReader "DataReader" is enabled.