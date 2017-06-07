Deadline QoS              {#DCPS_QoS_Deadline}
==============

This QosPolicy defines the period within which a new sample is expected by the \ref DCPS_Modules_Subscription_DataReader "DataReader" or to be written by the \ref DCPS_Modules_Publication_DataWriter "DataWriter".

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
            period
        </td>
        <td>
            \ref DCPS_Modules_Subscription_DataReader "DataReader" expects a new sample
            updating the value of each instance
            at least once every deadline period.
            \ref DCPS_Modules_Publication_DataWriter "DataWriter" indicates that the
            application commits to write a new
            value (using the \ref DCPS_Modules_Publication_DataWriter "DataWriter") for
            each instance managed by the
            \ref DCPS_Modules_Publication_DataWriter "DataWriter" at least once every
            deadline period. It is inconsistent
            for a \ref DCPS_Modules_Subscription_DataReader "DataReader" to have a
            DEADLINE period less than its
            TIME_BASED_FILTER's
            minimum_separation. The default
            value of the deadline period is
            infinite.
        </td>
        <td>
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td>Yes</td>
        <td>Yes</td>
    </tr>
</table>

This policy is useful for cases where a \ref DCPS_Modules_TopicDefinition "Topic" is expected to have each instance updated periodically. On the publishing side this setting establishes a contract that the application must meet. On the subscribing side the setting establishes a
minimum requirement for the remote publishers that are expected to supply the data values.

When the Service ‘matches’ a \ref DCPS_Modules_Publication_DataWriter "DataWriter" and a \ref DCPS_Modules_Subscription_DataReader "DataReader" it checks whether the settings are compatible (i.e., offered deadline period<= requested deadline period) if they are not, the two entities are informed (via the listener or condition
mechanism) of the incompatibility of the QoS settings and communication will not occur.
Assuming that the reader and writer ends have compatible settings, the fulfilment of this contract is monitored by the Service and the application is informed of any violations by means of the proper listener or condition.

The value offered is considered compatible with the value requested if and only if the inequality “offered deadline period
<= requested deadline period” evaluates to ‘TRUE.’

The setting of the DEADLINE policy must be set consistently with that of the TIME_BASED_FILTER. For these two
policies to be consistent the settings must be such that “deadline period>= minimum_separation.”

