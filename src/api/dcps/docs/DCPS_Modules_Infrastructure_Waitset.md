Waitset            {#DCPS_Modules_Infrastructure_Waitset}
=========

![Class model of supported DCPS Waitsets] (@ref InfrastructureModule_Waitset_UML.png)

Conditions (in conjunction with wait-sets) provide an alternative mechanism to allow the middleware to communicate communication status changes (including arrival of data) to the application.

This mechanism is wait-based. Its general use pattern is as follows:

- The application indicates which relevant information it wants to get, by creating Condition objects (StatusCondition, ReadCondition or QueryCondition) and attaching them to a WaitSet.

- It then waits on that WaitSet until the trigger_value of one or several Condition objects become TRUE.

- It then uses the result of the wait (i.e., the list of Condition objects with trigger_value==TRUE) to actually get the information by calling:

    - get_status_changes and then get_<communication_status> on the relevant Entity. If the condition is a Status-Condition and the status changes, refer to plain communication status.

    - get_status_changes and then get_datareaders on the relevant Subscriber. If the condition is a StatusCondition and the status changes, refers to DATA_ON_READERS.

    - get_status_changes and then read/take on the relevant DataReader. If the condition is a StatusCondition and the status changes, refers to DATA_AVAILABLE.

    - Directly read_w_condition/take_w_condition on the DataReader with the Condition as a parameter if it is a ReadCondition or a QueryCondition.

Usually the first step is done in an initialisation phase, while the others are put in the application main loop.
As there is no extra information passed from the middleware to the application when a wait returns (only the list of triggered Condition objects), Condition objects are meant to embed all that is needed to react properly when enabled.
In particular, Entity-related conditions are related to exactly one Entity and cannot be shared.
The result of a wait operation depends on the state of the WaitSet, which in turn depends on whether at least one attached Condition has a trigger_value of TRUE. If the wait operation is called on WaitSet with state BLOCKED, it will block the calling thread.
If wait is called on a WaitSet with state UNBLOCKED, it will return immediately. In addition, when the WaitSet transitions from BLOCKED to UNBLOCKED it wakes up any threads that had called wait on it.

Similar to the invocation of listeners, there is no implied “event queuing” in the awakening of a WaitSet in the sense that, if several Conditions attached to the WaitSet have their trigger_value transition to TRUE in sequence the DCPS implementation needs to only unblock the WaitSet once.

### Trigger State of the StatusCondition

The trigger_value of a StatusCondition is the Boolean OR of the ChangedStatusFlag of all the communication statuses to which it is sensitive. That is, trigger_value==FALSE only if all the values of the ChangedStatusFlags are FALSE.

The sensitivity of the StatusCondition to a particular communication status is controlled by the list of enabled_statuses set on the condition by means of the set_enabled_statuses operation.

### Trigger State of the ReadCondition

Similar to the StatusCondition, a ReadCondition also has a trigger_value that determines whether the attached WaitSet is BLOCKED or UNBLOCKED. However, unlike the StatusCondition, the trigger_value of the ReadCondition is tied to the presence of at least a sample managed by the Data Distribution Service with SampleState, ViewState, and InstanceState matching those of the ReadCondition. Furthermore, for the QueryCondition to have a trigger_value==TRUE, the data associated with the sample must be such that the query_expression evaluates to TRUE.

The fact that the trigger_value of a ReadCondition is dependent on the presence of samples on the associated DataReader implies that a single take operation can potentially change the trigger_value of several ReadCondition or QueryCondition conditions.
For example, if all samples are taken, any ReadCondition and QueryCondition conditions associated with the DataReader that had their trigger_value==TRUE before will see the trigger_value change to FALSE.

Note that this does not guarantee that WaitSet objects that were separately attached to those conditions will not be woken up.
Once we have trigger_value==TRUE on a condition it may wake up the attached WaitSet, the condition transitioning to trigger_value==FALSE does not necessarily ‘unwakeup’ the WaitSet as ‘unwakening’ may not be possible in general.

The consequence is that an application blocked on a WaitSet may return from the wait with a list of conditions some of which are no longer “active.” This is unavoidable if multiple threads are concurrently waiting on separate WaitSet objects and taking data associated with the same DataReader entity.

To elaborate further, consider the following example: A ReadCondition that has a sample_state_mask = {NOT_READ} will have trigger_value of TRUE whenever a new sample arrives and will transition to FALSE as soon as all the newly-arrived samples are either read (so their status changes to READ) or taken (so they are no longer managed by the Data Distribution Service).

However if the same ReadCondition had a sample_state_mask = {READ, NOT_READ}, then the trigger_value would only become FALSE once all the newly-arrived samples are taken (it is not sufficient to read them as that would only change the SampleState to READ which overlaps the mask on the ReadCondition.

### Trigger State of the GuardCondition
The trigger_value of a GuardCondition is completely controlled by the application via operation set_trigger_value.

![Conceptional DataReader WaitSet block] (@ref Waitsets.png)
