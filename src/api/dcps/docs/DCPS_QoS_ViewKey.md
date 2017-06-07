ViewKey QoS              {#DCPS_QoS_ViewKey}
==================

This QosPolicy is used to define a set of keys on a DataReaderView.



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
            A boolean: <br/>
            use_key_list
        </td>
        <td> Controls whether the alternative key list is applied to the DataReaderView.</td>
        <td rowspan="2">DataReaderView</td>
        <td rowspan="2">N/A</td>
        <td rowspan="2">No</td>
    </tr>
    <tr>
        <td>
            A sequence of strings: <br/>
            key_list
        </td>
        <td>A sequence of strings with one or more names of \ref DCPS_Modules_TopicDefinition "Topic" fields acting as alternative keys.</td>
    </tr>
</table>

This QosPolicy is used to set the key list of a DataReaderView. A DataReaderView
allows a different view, defined by this key list, on the data set of the \ref DCPS_Modules_Subscription_DataReader "DataReader"
from which it is created.


Operations that operate on instances or instance handles, such as
lookup_instance or get_key_value, respect the alternative key-list and work
as expected. However, since the mapping of writer instances to reader instances is
no longer trivial (one writer instance may now map to more than one matching
reader instance and vice versa), a writer instance will no longer be able to fully
determine the lifecycle of its matching reader instance, nor the value its
view_state and instance_state.


In fact, the view sample will always copy the view_state and instance_state
values from the reader sample to which it is slaved. If both samples preserve a 1 – 1
correspondence with respect to their originating instances (this may sometimes be
the case even when an alternative keylist is provided, i.e. when one reader instance
never maps to more than one view instance and vice versa) then the resulting
instance_state and view_state still have a valid semantical meaning. If this
1 – 1 correspondence cannot be guaranteed, the resulting
instance_state and view_state are semantically meaningless and should not be used to derive any
conclusion regarding the lifecycle of a view instance.


By default, the ViewKeyQosPolicy is disabled because use_key_list is set to FALSE.


This QosPolicy is applicable to a DataReaderView only, and cannot be changed
after the DataReaderView is created.