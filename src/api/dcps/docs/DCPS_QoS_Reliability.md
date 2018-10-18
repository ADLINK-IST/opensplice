Reliability QoS              {#DCPS_QoS_Reliability}
===============

This QosPolicy controls the level of reliability of the data distribution offered or
requested by the \ref DCPS_Modules_Publication_DataWriter "DataWriter"s and \ref DCPS_Modules_Subscription_DataReader "DataReader"s.

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
            A ReliabilityQosPolicyKind:<br/>
            kind
        </td>
        <td>
            Specifies the type of reliability which may be BEST_EFFORT or RELIABLE, the default is BEST_EFFORT
        </td>
        <td rowspan="4">
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="4">Yes</td>
        <td rowspan="4">No</td>
    </tr>
    <tr>
        <td>RELIABLE</td>
        <td>
            Specifies the Service will attempt to
            deliver all samples in its history.
            Missed samples may be retried. In
            steady-state (no modifications
            communicated via the \ref DCPS_Modules_Publication_DataWriter "DataWriter")
            the middleware guarantees that all
            samples in the \ref DCPS_Modules_Publication_DataWriter "DataWriter" history
            will eventually be delivered to all
            the \ref DCPS_Modules_Subscription_DataReader "DataReader" objects. Outside
            steady state the HISTORY and
            RESOURCE_LIMITS policies will
            determine how samples become part
            of the history and whether samples
            can be discarded from it. This is the
            default value for \ref DCPS_Modules_Publication_DataWriter "DataWriter"s.
        </td>
    </tr>
    <tr>
        <td>BEST_EFFORT</td>
        <td>
            Indicates that it is acceptable to not
            retry propagation of any samples.
            Presumably new values for the
            samples are generated often enough
            that it is not necessary to re-send or
            acknowledge any samples. This is
            the default value for \ref DCPS_Modules_Subscription_DataReader "DataReader"s
            and \ref DCPS_Modules_TopicDefinition "Topic"s.
        </td>
    </tr>
    <tr>
        <td>
            A duration:<br/>
            max_blocking_time</td>
        <td>
            The value of the max_blocking_time
            indicates the maximum time the
            operation \ref DCPS_Modules_Publication_DataWriter "DataWriter" write is
            allowed to block if the \ref DCPS_Modules_Publication_DataWriter "DataWriter"
            does not have space to store the
            value written. The default
            max_blocking_time=100ms.
        </td>
    </tr>
    <tr>
        <td>
            A boolean:<br/>
            synchronous
        </td>
        <td>
            Specifies whether a \ref DCPS_Modules_Publication_DataWriter "DataWriter" should wait for acknowledgements by all connected \ref DCPS_Modules_Subscription_DataReader "DataReader"s that also have set a synchronous ReliabilityQosPolicy. <br/>It is advised only to use this policy in combination with reliability, if used in combination with best effort data may not arrive at the \ref DCPS_Modules_Subscription_DataReader "DataReader" resulting in a timeout at the \ref DCPS_Modules_Publication_DataWriter "DataWriter" indicating that the data has not been received. Acknoledgments are always sent reliable so when the \ref DCPS_Modules_Publication_DataWriter "DataWriter" encounters a timeout it is guaranteed that the \ref DCPS_Modules_Subscription_DataReader "DataReader" hasn't received the data.<br/> **Note:** The synchronous option is an OpenSplice specific QoS!
        </td>
        <td>
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td>No</td>
        <td>No</td>
     </tr>
</table>

This policy indicates the level of reliability requested by a \ref DCPS_Modules_Subscription_DataReader "DataReader" or offered by a \ref DCPS_Modules_Publication_DataWriter "DataWriter". These levels are ordered, BEST_EFFORT being lower than RELIABLE. A \ref DCPS_Modules_Publication_DataWriter "DataWriter" offering a level is implicitly offering all levels
below.

The setting of this policy has a dependency on the setting of the RESOURCE_LIMITS policy. In case the RELIABILITY kind is set to RELIABLE the write operation on the \ref DCPS_Modules_Publication_DataWriter "DataWriter" may block if the modification would cause data to be lost or else cause one of the limits specified in the RESOURCE_LIMITS to be exceeded. Under these circumstances, the RELIABILITY max_blocking_time configures the maximum duration the write operation may block.

If the RELIABILITY kind is set to RELIABLE, data-samples originating from a single \ref DCPS_Modules_Publication_DataWriter "DataWriter" cannot be made available to the \ref DCPS_Modules_Subscription_DataReader "DataReader" if there are previous data-samples that have not been received yet due to a communication error. In other words, the service will repair the error and re-transmit data-samples as needed in order to reconstruct a correct snapshot of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" history before it is accessible by the \ref DCPS_Modules_Subscription_DataReader "DataReader".

If the RELIABILITY kind is set to BEST_EFFORT, the service will not re-transmit missing data-samples. However for data-samples originating from any one \ref DCPS_Modules_Publication_DataWriter "DataWriter" the service will ensure they are stored in the \ref DCPS_Modules_Subscription_DataReader "DataReader" history in the same order they originated in the \ref DCPS_Modules_Publication_DataWriter "DataWriter". In other words, the \ref DCPS_Modules_Subscription_DataReader "DataReader" may miss some data-samples but it will never see the value of a data-object change from a newer value to an order value.

The value offered is considered compatible with the value requested if and only if the inequality "offered kind >= requested kind" evaluates to 'TRUE'
For the purposes of this inequality, the values of RELIABILITY kind are considered ordered such that BEST_EFFORT < RELIABLE.

This QoS plays a role within the scope of \ref DCPS_Eventual_Consistency "Eventual Consistency"



