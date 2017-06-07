Scheduling QoS              {#DCPS_QoS_Scheduling}
==============

This QosPolicy specifies the scheduling parameters that will be used for a thread that is spawned by the \ref DCPS_Modules_DomainParticipant "DomainParticipant".


*NOTE:* Some scheduling parameters may not be supported by the underlying
Operating System, or that you may need special privileges to select particular
settings.



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
            A SchedulingClassQosPolicyKind:<br>
            scheduling_class.kind
        </td>
        <td>
            Specifies the scheduling class used by the Operating System, which may be SCHEDULE_DEFAULT, SCHEDULE_TIMESHARING or SCHEDULE_REALTIME. Threads can only be spawned within the scheduling classes that are supported by the underlying Operating System.
        </td>
         <td rowspan="3">
            \ref DCPS_Modules_DomainParticipant "DomainParticipant"
        </td>
        <td rowspan="3">N/A</td>
        <td rowspan="3">No</td>
    </tr>
    <tr>
        <td>
            A SchedulingPriorityQosPolicyKind:<br>
            scheduling_priority_kind.kind
        </td>
        <td>
            specifies the priority type, which may be either PRIORITY_RELATIVE or PRIORITY_ABSOLUTE.
        </td>
    </tr>
    <tr>
        <td>
            A long:<br>
            scheduling_priority
        </td>
        <td>
            Specifies the priority that will be assigned to threads spawned by the \ref DCPS_Modules_DomainParticipant "DomainParticipant". Threads can only be spawned with priorities that are supported by the underlying Operating System.
        </td>
    </tr>
</table>

This QosPolicy specifies the scheduling parameters that will be used for threads
spawned by the \ref DCPS_Modules_DomainParticipant "DomainParticipant". Note that some scheduling parameters may
not be supported by the underlying Operating System, or that you may need special
privileges to select particular settings. Refer to the documentation of your OS for
more details on this subject.


Although the behaviour of the scheduling_class is highly dependent on the underlying OS,
in general it can be said that when running in a Timesharing class your thread will have
to yield execution to other threads of equal priority regularly.
In a Realtime class your thread normally runs until completion, and can only be
pre-empted by higher priority threads. Often the highest range of priorities is not
accessible through a Timesharing Class.


The scheduling_priority_kind determines whether the specified scheduling_priority should be
interpreted as an absolute priority, or whether it should be interpreted relative to the priority
of its creator, in this case the priority of the thread that created the \ref DCPS_Modules_DomainParticipant "DomainParticipant".