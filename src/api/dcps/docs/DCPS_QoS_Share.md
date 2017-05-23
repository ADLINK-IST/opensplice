Share QoS              {#DCPS_QoS_Share}
==================
This QosPolicy is used to share a \ref DCPS_Modules_Subscription_DataReader "DataReader" between multiple processes.


*NOTE:* This is an OpenSplice-specific QosPolicy, it is not part of the DDS Specification.

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
            A string: <br/>
            name
        </td>
        <td>The label used to identify the shared Entity.</td>
        <td rowspan="2">\ref DCPS_Modules_Subscription_DataReader "DataReader",\ref DCPS_Modules_Subscription "Subscriber"</td>
        <td rowspan="2">N/A</td>
        <td rowspan="2">No</td>
    </tr>
    <tr>
        <td>
            A boolean: <br/>
            enable
        </td>
        <td>Controls whether the entity is shared.</td>
    </tr>
</table>


This QosPolicy allows sharing of entities by multiple processes or threads. When
the policy is enabled, the data distribution service will try to look up an existing
entity that matches the name supplied in the ShareQosPolicy. A new entity will only
be created if a shared entity registered under the specified name doesn’t exist yet.
Shared Readers can be useful for implementing algorithms like the worker pattern,
where a single shared reader can contain samples representing different tasks that
may be processed in parallel by separate processes. In this algorithm each processes
consumes the task it is going to perform (i.e. it takes the sample representing that
task), thus preventing other processes from consuming and therefore performing the
same task.


*NOTE:* Entities can only be shared between processes if OpenSplice is running in
federated mode, because it requires shared memory to communicate between the
different processes.


By default, the ShareQosPolicy is not used and enable is FALSE. Name must be set
to a valid string for the ShareQosPolicy to be valid when enable is set to TRUE.


This QosPolicy is applicable to \ref DCPS_Modules_Subscription_DataReader "DataReader" and \ref DCPS_Modules_Subscription "Subscriber" entities, and cannot be
modified after the \ref DCPS_Modules_Subscription_DataReader "DataReader" or \ref DCPS_Modules_Subscription "Subscriber" is enabled. Note that a \ref DCPS_Modules_Subscription_DataReader "DataReader" can
only be shared if its \ref DCPS_Modules_Subscription "Subscriber" is also shared.