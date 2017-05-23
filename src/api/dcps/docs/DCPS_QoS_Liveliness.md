Liveliness QoS              {#DCPS_QoS_Liveliness}
==============

This QosPolicy controls the way the liveliness of an Entity is being determined.

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
            A LivelinessQosPolicyKind:<br/>
            kind
        </td>
        <td>
            Determines the mechanism and
            parameters used by the application
            to determine whether an Entity is
            “active” (alive). The “liveliness”
            status of an Entity is used to
            maintain instance ownership in
            combination with the setting of the
            OWNERSHIP QoS policy. The
            application is also informed via
            listener when an Entity is no longer
            alive.
            The \ref DCPS_Modules_Subscription_DataReader "DataReader" requests that
            liveliness of the writers is
            maintained by the requested means
            and loss of liveliness is detected
            with delay not to exceed the
            lease_duration.
            The \ref DCPS_Modules_Publication_DataWriter "DataWriter" commits to
            signalling its liveliness using the
            stated means at intervals not to
            exceed the lease_duration.
            Listeners are used to notify the
            \ref DCPS_Modules_Subscription_DataReader "DataReader" of loss of liveliness and
            \ref DCPS_Modules_Publication_DataWriter "DataWriter" of violations to the
            liveliness contract. The kinds can be
            AUTOMATIC,MANUAL_BY_PARTICIPANT,MANUAL_BY_TOPIC
            The default kind is AUTOMATIC.
        </td>
        <td rowspan="6">
            \ref DCPS_Modules_TopicDefinition "Topic",
            \ref DCPS_Modules_Subscription_DataReader "DataReader",
            \ref DCPS_Modules_Publication_DataWriter "DataWriter"
        </td>
        <td rowspan="6">Yes</td>
        <td rowspan="6">No</td>
    </tr>
    </tr>
    <tr>
        <td>
            A duration:<br/>
            lease_duration
        </td>
        <td>
            Specifies the duration of the interval within which the liveliness must be reported. The default is infinite.
        </td>
    </tr>
    <tr>
        <td>AUTOMATIC</td>
        <td>
            The infrastructure will automatically
            signal liveliness for the DataWriters
            at least as often as required by the
            lease_duration
        </td>
    </tr>
    <tr>
        <td>
            MANUAL
        </td>
        <td>
            The user application takes
            responsibility to signal liveliness to
            the Service using one of the
            mechanisms described in
            \ref DCPS_QoS_Liveliness "LIVELINESS".
            Liveliness must be
            asserted at least once every
            lease_duration otherwise the
            Service will assume the
            corresponding Entity is no longer
            “active/alive.”
        </td>
    </tr>
    <tr>
        <td>
            MANUAL_BY_PARTICIPANT
        </td>
        <td>
            The Service will assume that as long
            as at least one Entity within the
            DomainParticipant has asserted its
            liveliness the other Entities in that
            same DomainParticipant are also
            alive.
        </td>
    </tr>
    <tr>
        <td>
            MANUAL_BY_TOPIC
        </td>
        <td>
            The Service will only assume
            liveliness of the \ref DCPS_Modules_Publication_DataWriter "DataWriter" if the
            application has asserted liveliness of
            that \ref DCPS_Modules_Publication_DataWriter "DataWriter" itself.
        </td>
    </tr>
</table>

This policy controls the mechanism and parameters used by the Service to ensure that particular entities on the network are still “alive.” The liveliness can also affect the ownership of a particular instance, as determined by the OWNERSHIP
QoS policy.

This policy has several settings to support both data-objects that are updated periodically as well as those that are changed sporadically. It also allows customising for different application requirements in terms of the kinds of failures that will be
detected by the liveliness mechanism.

The AUTOMATIC liveliness setting is most appropriate for applications that only need to detect failures at the process-level, but not application-logic failures within a process. The Service takes responsibility for renewing the leases at the required rates and thus, as long as the local process where a DomainParticipant is running and the link connecting it to remote participants remains connected, the entities within the DomainParticipant will be considered alive. This requires the lowest overhead.

The MANUAL settings (MANUAL_BY_PARTICIPANT, MANUAL_BY_TOPIC) require the application on the publishing side to periodically assert the liveliness before the lease expires to indicate the corresponding Entity is still alive. The action can be explicit by calling the assert_liveliness operations, or implicit by writing some data.

The two possible manual settings control the granularity at which the application must assert liveliness.

- The setting MANUAL_BY_PARTICIPANT requires only that one Entity within the publisher is asserted to be alive to deduce all other Entity objects within the same DomainParticipant are also alive.

- The setting MANUAL_BY_TOPIC requires that at least one instance within the \ref DCPS_Modules_Publication_DataWriter "DataWriter" is asserted.

The value offered is considered compatible with the value requested if and only if the following conditions are met:

-# the inequality “offered kind >= requested kind” evaluates to ‘TRUE.’ For the purposes of this inequality, the values of LIVELINESS kind are considered ordered such that:
    - AUTOMATIC < MANUAL_BY_PARTICIPANT < MANUAL_BY_TOPIC.


-# the inequality “offered lease_duration <= requested lease_duration” evaluates to TRUE.

Changes in LIVELINESS must be detected by the Service with a time-granularity greater or equal to the lease_duration. This ensures that the value of the LivelinessChangedStatus is updated at least once during each lease_duration and the
related Listeners and WaitSets are notified within a lease_duration from the time the LIVELINESS changed.

