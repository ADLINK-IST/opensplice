Latency Budget QoS              {#DCPS_QoS_LatencyBudget}
==================

Specifies the maximum acceptable additional delay to the typical transport delay
from the time the data is written until the data is delivered at the
\ref DCPS_Modules_Subscription_DataReader "DataReader" and the application is notified of this fact.

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
            duration
        </td>
        <td>
            Specifies the maximum acceptable
            delay from the time the data is
            written until the data is inserted in
            the receiver's application-cache and
            the receiving application is notified
            of the fact. This policy is a hint to
            the Service, not something that must
            be monitored or enforced. The
            Service is not required to track or
            alert the user of any violation. The
            default value of the duration is zero
            indicating that the delay should be
            minimised.
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

This policy provides a means for the application to indicate to the middleware the “urgency” of the data-communication. By having a non-zero duration the Service can optimise its internal operation.

This policy is considered a hint. There is no specified mechanism as to how the service should take advantage of this hint.

The value offered is considered compatible with the value requested if and only if the inequality “offered duration <=
requested duration” evaluates to ‘TRUE.’


