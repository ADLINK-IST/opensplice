Status            {#DCPS_Modules_Infrastructure_Status}
================

![Status] (@ref InfrastructureModule_Status_UML.png)

The communication statuses whose changes can be communicated to the application depend on the Entity. The following table shows for each entity the statuses that are relevant.

<table>
    <tr>
        <th>Entity</th>
        <th>Status Name</th>
        <th>Meaning</th>
    </tr>
    <tr>
        <td>Topic</td>
        <td>@subpage DCPS_Status_InconsistentTopic "INCONSISTENT_TOPIC"</td>
        <td>Another topic exists with the same name but different characteristics.</td>
    </tr>
    <tr>
        <td>Subscriber</td>
        <td>DATA_ON_READERS</td>
        <td>New information is available.</td>
    </tr>
    <tr>
        <td rowspan="7">DataReader</td>
        <td>@subpage DCPS_Status_SampleRejected "SAMPLE_REJECTED"</td>
        <td>A received sample has been rejected</td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_LivelinessChanged "LIVELINESS_CHANGED"</td>
        <td>
            The liveliness of one or more **DataWriter** that were writing instances
            read through the **DataReader** has changed. Some **DataWriter** have
            become “active” or “inactive.”
        </td>
    <tr>
        <td>@subpage DCPS_Status_RequestedDeadlineMissed "REQUESTED_DEADLINE_MISSED"</td>
        <td>
            The deadline that the **DataReader** was expecting through its QoSPolicy
            DEADLINE was not respected for a specific instance.
        </td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_RequestedIncompatibleQoS "REQUESTED_INCOMPATIBLE_QOS"</td>
        <td>A QosPolicy value was incompatible with what is offered.</td>
    </tr>
    <tr>
        <td>DATA_AVAILABLE</td>
        <td>New information is available.</td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_SampleLost "SAMPLE_LOST"</td>
        <td>A sample has been lost (never received).</td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_SubscriptionMatched "SUBSCRIPTION_MATCHED"</td>
        <td>
            The **DataReader** has found a **DataWriter** that matches the Topic and
            has compatible QoS, or has ceased to be matched with a **DataWriter**
            that was previously considered to be matched.
        </td>
    </tr>
    <tr>
        <td rowspan="5">DataWriter</td>
        <td>@subpage DCPS_Status_LivelinessLost "LIVELINESS_LOST"</td>
        <td>
            The liveliness that the **DataWriter** has committed through its
            QosPolicy LIVELINESS was not respected; thus **DataReader** entities
            will consider the **DataWriter** as no longer “active.”
        </td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_OfferedDeadlineMissed "OFFERED_DEADLINE_MISSED"</td>
        <td>
            The deadline that the **DataWriter** has committed through its QosPolicy
            DEADLINE was not respected for a specific instance.
        </td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_OfferedIncompatibleQoS "OFFERED_INCOMPATIBLE_QOS"</td>
        <td>
            A QosPolicy value was incompatible with what was requested.
        </td>
    </tr>
    <tr>
        <td>@subpage DCPS_Status_PublicationMatched "PUBLICATION_MATCHED"</td>
        <td>
            The **DataWriter** has found DataReader that matches the Topic and has
            compatible QoS, or has ceased to be matched with a DataReader that
            was previously considered to be matched.
        </td>
    </tr>
</table>


Those statuses may be classified in:

- Read communication statuses: i.e., those that are related to arrival of data, namely DATA_ON_READERS and DATA_AVAILABLE.
- Plain communication statuses: i.e., all the others. Read communication statuses are treated slightly differently than the others for they don’t change independently.
    In other  words, at least two changes will appear at the same time (DATA_ON_READERS + DATA_AVAILABLE) and even several of the last kind may be part of the set. This ‘grouping’ has to be communicated to the application.

Changes in Status
-----------------
Associated with each one of an Entity’s communication status is a logical StatusChangedFlag. This flag indicates
whether that particular communication status has changed since the last time the status was ‘read’ by the application. The
way the status changes is slightly different for the Plain Communication Status and the Read Communication status.

![StatusChangedFlag indicates if status has changed] (@ref InfrastructureModule_Status_CIS_UML.png)

Changes in Plain Communication Status
-------------------------------------
For the plain communication status, the StatusChangedFlag flag is initially set to FALSE. It becomes TRUE whenever the plain communication status changes and it is reset to FALSE each time the application accesses the plain communication status via the proper get_<plain communication status> operation on the Entity.

![Changes in StatusChangedFlag for plain communication status] (@ref InfrastructureModule_Status_CIPS_UML.png)

The communication status is also reset to FALSE whenever the associated listener operation is called as the listener implicitly accesses the status which is passed as a parameter to the operation.
The fact that the status is reset prior to calling the listener means that if the application calls the get_<plain communication status> from inside the listener it will see the status already reset.

An exception to this rule is when the associated listener is the 'nil' listener.
The 'nil' listener is treated as a NO-OP and the act of calling the 'nil' listener does not reset the communication status.

For example, the value of the StatusChangedFlag associated with the REQUESTED_DEADLINE_MISSED status will become TRUE each time new deadline occurs (which increases the total_count field within RequestedDeadlineMissedStatus). The value changes to FALSE when the application accesses the status via the corresponding get_requested_deadline_missed_status method on the proper Entity.


Changes in Read Communication Status
--------------------------------------
For the read communication status, the StatusChangedFlag flag is initially set to FALSE.
The StatusChangedFlag becomes TRUE when either a data-sample arrives or else the ViewState, SampleState, or InstanceState of any existing sample changes for any reason other than a call to DataReader::read, DataReader::take or their variants.
Specifically any of the following events will cause the StatusChangedFlag to become TRUE:

- The arrival of new data.
- A change in the InstanceState of a contained instance. This can be caused by either:
    - The arrival of the notification that an instance has been disposed by:
        - the DataWriter that owns it if OWNERSHIP QoS kind=EXCLUSIVE
        - or by any DataWriter if OWNERSHIP QoS kind=SHARED.
    - The loss of liveliness of the DataWriter of an instance for which there is no other DataWriter.
    - The arrival of the notification that an instance has been unregistered by the only DataWriter that is known to be writing the instance.


Depending on the kind of StatusChangedFlag, the flag transitions to FALSE again as follows:
    - The DATA_AVAILABLE StatusChangedFlag becomes FALSE when either the corresponding listener operation (on_data_available) is called or the read or take operation (or their variants) is called on the associated DataReader.
    - The DATA_ON_READERS StatusChangedFlag becomes FALSE when any of the following events occurs:
    - The corresponding listener operation (on_data_on_readers) is called.
    - The on_data_available listener operation is called on any DataReader belonging to the Subscriber.
    - The read or take operation (or their variants) is called on any DataReader belonging to the Subscriber.
