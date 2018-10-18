Presentation QoS              {#DCPS_QoS_Presentation}
================

This QosPolicy controls the extent to which changes to data-instances can be
made dependent on each other, the order in which they need to be presented to the
user and also the kind of dependencies that can be propagated and maintained by the
Data Distribution Service.

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
            A
            PresentationQosPolicyAccessScopeKind:<br/>
            access_scope
        </td>
        <td>
            Specifies how the samples
            representing changes to data
            instances are presented to the
            subscribing application. This policy
            affects the application’s ability to
            specify and receive coherent
            changes and to see the relative order
            of changes. access_scope
            determines the largest scope
            spanning the entities for which the
            order and coherency of changes can
            be preserved. Options are INSTANCE,TOPIC and GROUP where the default is INSTANCE
        </td>
        <td rowspan="6">\ref DCPS_Modules_Publication "Publisher", \ref DCPS_Modules_Subscription "Subscriber"</td>
        <td rowspan="6">Yes</td>
        <td rowspan="6">No</td>
    </tr>
    <tr>
        <td>INSTANCE</td>
        <td>
            Scope spans only a single instance.
            Indicates that changes to one
            instance need not be coherent nor
            ordered with respect to changes to
            any other instance. In other words,
            order and coherent changes apply to
            each instance separately. This is the
            default access_scope.
        </td>
    </tr>
    <tr>
        <td>TOPIC</td>
        <td>
            Scope spans to all instances within
            the same \ref DCPS_Modules_Publication_DataWriter "DataWriter" (or \ref DCPS_Modules_Subscription_DataReader "DataReader"), but not across instances in
            different \ref DCPS_Modules_Publication_DataWriter "DataWriter" (or \ref DCPS_Modules_Subscription_DataReader "DataReader").
        </td>
    </tr>
    <tr>
        <td>GROUP</td>
        <td>
            Scope spans to all
            instances belonging to \ref DCPS_Modules_Publication_DataWriter "DataWriter"
            (or \ref DCPS_Modules_Subscription_DataReader "DataReader") entities within the
            same \ref DCPS_Modules_Publication "Publisher" (or \ref DCPS_Modules_Subscription "Subscriber").
        </td>
    </tr>
    <tr>
        <td>
            A boolean:<br/>
            coherent_access</td>
        <td>
            Specifies support coherent access.
            That is, the ability to group a set of
            changes as a unit on the publishing
            end such that they are received as a
            unit at the subscribing end. The
            default setting of coherent_access is
            FALSE.
        </td>
    </tr>
    <tr>
        <td>
            A boolean:<br/>
            ordered_access</td>
        <td>
            Specifies support for ordered access
            to the samples received at the
            subscription end. That is, the ability
            of the \ref DCPS_Modules_Subscription "Subscriber" to see changes in
            the same order as they occurred on
            the publishing end. The default
            setting of ordered_access is FALSE.
        </td>
    </tr>
</table>

This QoS policy controls the extent to which changes to data-instances can be made dependent on each other,
the order in which they need to be presented to the user  and also the kind of dependencies that can be propagated
and maintained by the Service.

The support for ‘coherent changes’ enables a publishing application to change the value of several data-instances that
could belong to the same or different topics and have those changes be seen ‘atomically’ by the readers. This is useful
in cases where the values are inter-related. For example, if there are two data-instances representing the ‘altitude’
and ‘velocity vector’ of the same aircraft and both are changed, it may be useful to communicate those values in a way
the reader can see both together; otherwise it may erroneously interpret that the aircraft is on a collision course.
Basically this QosPolicy allows a \ref DCPS_Modules_Publication "Publisher" to group a number of samples by writing them
\if isocpp2
within a scope that holds a CoherentSet object
\else
within calls to begin_coherent_change and end_coherent_change
\endif
and treat them as if they are to be communicated as a single message. That is, the receiver will only be able to access the data after all
the modifications in the set are available at the receiver end.

Samples that belong to a (yet) unfinished coherent update consume resource limits from the receiving \ref DCPS_Modules_Subscription_DataReader "DataReader", but are not
(yet) accessible through its history, and cannot (yet) push samples out of its history. In order for the \ref DCPS_Modules_Subscription_DataReader "DataReader" to store
samples outside its history administration, its ResourceLimitsQosPolicy should have a value for max_samples_per_instance that
is bigger than the depth value of its HistoryQosPolicy.

If not enough resources are available to hold an incoming sample that belongs to an unfinshed transaction, one of the following
things may happen.
- When some of the resources are in use by the history of the \ref DCPS_Modules_Subscription_DataReader "DataReader", the incoming sample will be rejected, and the \ref DCPS_Modules_Subscription_DataReader "DataReader"
will be notified of a SAMPLE_REJECTED event. This will cause the delivery mechanism to retry delivery of the rejected sample at a
later moment in time, in the expectation that the application will free resources by actively taking samples out of the reader history.
- When all the available resources are in use by samples belonging to unfinished coherent updates, the application has no way to free
up resources and the transaction is either ‘deadlocked’ by itself (i.e. it is too big for the amount of available resources) or by one
or more other incomplete transactions. To break out of this deadlock, all samples belonging to the same transaction as the currently
incoming sample will be dropped, and the \ref DCPS_Modules_Subscription_DataReader "DataReader" will be notified of a SAMPLE_LOST event. No attempt will be made to retransmit
the dropped transaction. To avoid this scenario, it is important to make sure the \ref DCPS_Modules_Subscription_DataReader "DataReader" has set its ResourceLimits to accommodate
for the worst case history size PLUS the worst case transaction size. In other words, if Sh represents the worst case size of the
required history, St represents the worst case size of single transaction and Nt represents the worst case number of concurrent
transactions, then the ResourceLimits should accomodate for Sh + (St * Nt).

A connectivity change may occur in the middle of a set of coherent changes; for example, the set of partitions used by the \ref DCPS_Modules_Publication "Publisher" or
one of its Subscribers may change, a late-joining \ref DCPS_Modules_Subscription_DataReader "DataReader" may appear on the network, or a communication failure may occur. In the
event that such a change prevents an entity from receiving the entire set of coherent changes, that entity must behave as if it had
received none of the set.

The support for ‘ordered_access’ enables a subscribing application to view changes in the order in which they occurred. Ordering is always
determined according to the applicable DestinationOrderQosPolicy setting. Depending on the selected access_scope, ordering is either on a
per instance basis (this is the default behaviour, even when ordered_access is set to FALSE), on a per \ref DCPS_Modules_Subscription_DataReader "DataReader" basis or across all
DataReaders that span the \ref DCPS_Modules_Subscription "Subscriber". In case of ordered_access with an acces_scope of GROUP, the \ref DCPS_Modules_Subscription "Subscriber" will enforce that all its
DataReaders share the same DestinationOrderQosPolicy setting. The DestinationOrderQosPolicy setting of the first \ref DCPS_Modules_Subscription_DataReader "DataReader" created for
that \ref DCPS_Modules_Subscription "Subscriber" will then determine the DestinationOrderQosPolicy setting that is allowed for all subsequent DataReaders. Conflicting
settings will result in an INCONSISTENT_POLICY error.

The PresentationQosPolicy is applicable to both \ref DCPS_Modules_Publication "Publisher" and \ref DCPS_Modules_Subscription "Subscriber", but behaves differently on the publishing side and the subscribing
side. The setting of coherent_access on a \ref DCPS_Modules_Publication "Publisher" controls whether that \ref DCPS_Modules_Publication "Publisher" will preserve the coherency of changes
\if isocpp2
 within a scope that holds a CoherentSet object
\else
 enclosed by calls to begin_coherent_change and end_coherent_change
\endif
, as indicated by its access_scope and as made available by its embedded DataWriters. However, the \ref DCPS_Modules_Subscription "Subscriber" settings determine whether a coherent set of samples will actually be delivered to the subscribing application in a coherent way.

- If a \ref DCPS_Modules_Publication "Publisher" or \ref DCPS_Modules_Subscription "Subscriber" sets coherent_access to FALSE, it indicates that it does not want to maintain coherency between the different
samples in a set: a \ref DCPS_Modules_Subscription "Subscriber" that receives only a part of this set may still deliver this partial set of samples to its embedded DataReaders.
- If both \ref DCPS_Modules_Publication "Publisher" and \ref DCPS_Modules_Subscription "Subscriber" set coherent_access to TRUE, they indicate that they want to maintain coherency between the different samples
in a set: a \ref DCPS_Modules_Subscription "Subscriber" that receives only a part of this set may not deliver this partial set of samples to its embedded DataReaders; it needs
to wait for the set to become complete, and it will flush this partial set when it concludes that it will never be able to complete it.

Coherency is implemented on top of a transaction mechanism between individual DataWriters and DataReaders; completeness of a coherent set is
determined by the successful completion of each of its participating transactions. The value of the access_scope attribute determines which
combination of transactions constitute the contents of a coherent set.

The setting of ordered_access has no impact on the way in which a \ref DCPS_Modules_Publication "Publisher" transmits its samples (although it does influence the RxO properties
of this \ref DCPS_Modules_Publication "Publisher"), but basically it determines whether a \ref DCPS_Modules_Subscription "Subscriber" will preserve the ordering of samples when the subscribing application uses
its embedded DataReaders to read or take samples:
- If a \ref DCPS_Modules_Subscription "Subscriber" sets ordered_access to FALSE, it indicates that it does not want to maintain ordering between the different samples it receives:
a subscribing application that reads or takes samples will receive these samples ordered by their key-values, which does probably not resemble the
order they were written in.
- If a \ref DCPS_Modules_Subscription "Subscriber" sets ordered_access to TRUE, it indicates that it does want to maintain ordering within the specified access_scope between the
different samples it receives: a subscribing application that reads or takes samples will receives these samples sorted by the order in which they
were written.

The access_scope determines the maximum extent of coherent and/or ordered changes:
- If access_scope is set to INSTANCE and coherent_access is set to TRUE, then the \ref DCPS_Modules_Subscription "Subscriber" will behave, with respect to
maintaining coherency, in a way similar to an access_scope that is set to TOPIC. This is caused by the fact that coherency
is defined as the successful completion of all participating transactions. If a \ref DCPS_Modules_Publication_DataWriter "DataWriter" writes a transaction containing samples from different
instances, and a connected \ref DCPS_Modules_Subscription_DataReader "DataReader" misses one of these samples, then the transaction failed and the coherent set is considered incomplete by the
receiving \ref DCPS_Modules_Subscription_DataReader "DataReader". It doesn’t matter that all the other instances have received their samples successfully; an unsuccessful transaction by
definition results in an incomplete coherent set. In that respect the DDS can offer no  granularity that is more fine-grained with respect to
coherency than that described by the TOPIC.
If access_scope is set to INSTANCE and ordered_access is set to TRUE, then the \ref DCPS_Modules_Subscription "Subscriber" will maintain ordering between
samples belonging to the same instance. Samples belonging to different instances will still be grouped by their key-values instead of by the order
in which they were received.
- If access_scope is set to TOPIC and coherent_access is set to TRUE, then the DDS will define the scope of a coherent set
on individual transactions. So a coherent set that spans samples coming from multiple DataWriters
\if isocpp2
 (indicated by the scope of the CoherentSet object),
\else
 (indicated by its enclosure within calls to begin_coherent_change and end_coherent_change on their shared \ref DCPS_Modules_Publication "Publisher"),
\endif
is chopped up into separate and disjunct transactions (one for each participating \ref DCPS_Modules_Publication_DataWriter "DataWriter"), where each transaction is processed separately.
On the subscribing side this may result in the successful completion of some of these transactions, and the unsuccessful completion of some others.
In such cases all DataReaders that received successful transactions will deliver the embedded content to their applications, without waiting for the
completion of other transactions in other DataReaders connected to the same \ref DCPS_Modules_Subscription "Subscriber".
If access_scope is set to TOPIC and ordered_access is set to TRUE, then the \ref DCPS_Modules_Subscription "Subscriber" will maintain ordering between samples
belonging to the same \ref DCPS_Modules_Subscription_DataReader "DataReader". This means that samples belonging to the same instance in the same \ref DCPS_Modules_Subscription_DataReader "DataReader" may no longer be received consecutively
if samples belonging to different instances were written in between. It is possible to read/take a limited batch of ordered samples (where max_samples
!= LENGTH_UNLIMITED). In that case the \ref DCPS_Modules_Subscription_DataReader "DataReader" will keep a bookmark, so that in subsequent read/take operations your application can start where the
previous read/take call left off. There are two ways for the middleware to indicate that you completed a full iteration:
  + The amount of samples returned is smaller than the amount of samples requested. In that case the bookmark is automatically reset and the next
read/take will begin a new iteration right from the start of the ordered list.
  + Your read/take call returns RETCODE_NO_DATA, indicating that there are no more samples matching your criteria after the current bookmark. In that
case the bookmark is reset as well.
The bookmark is also reset in the following cases:
  + A read/take call uses different masks than the previous invocation of read/take.
  + A read/take call uses a different query than the previous invocation of read/take.
- If access_scope is set to GROUP and coherent_access is set to TRUE, then the DDS will define the scope of a coherent set on the
sum of all participating transactions. So a coherent set that spans samples coming from multiple DataWriters
\if isocpp2
 (indicated by the scope of the CoherentSet object),
\else
 (indicated by its enclosure within calls to begin_coherent_change and end_coherent_change on their shared \ref DCPS_Modules_Publication "Publisher"),
\endif
is chopped up into separate and disjunct transactions (one for each participating \ref DCPS_Modules_Publication_DataWriter "DataWriter"), where each transactions is processed separately. On the
subscribing side this may result in the successful completion of some of these transactions, and the unsuccessful completion of some others. However,
each \ref DCPS_Modules_Subscription_DataReader "DataReader" is only allowed to deliver the embedded content when all participating transactions completed successfully. This means that DataReaders
that received successful transactions will need to wait for all other DataReaders attached to the same \ref DCPS_Modules_Subscription "Subscriber" to also complete their transactions
successfully. If one or more DataReaders conclude that they will not be able to complete their transactions successfully, then all DataReaders that
participate in the original coherent set will flush the content of their transactions. In order for the application to access the state of all
DataReaders that span the coherent update, a separate read/take operation will need to be performed on each of the concerned DataReaders. To keep the
history state of the DataReaders consistent in between the successive invocations of the read/take operations on the various readers,
\if isocpp2
 the DataReaders should be locked for incoming updates by starting a new scope that holds a CoherentAccess object.
If all concerned  DataReaders have been accessed properly, they can be unlocked for incoming updates by ending the scope that holds the CoherentAccess object or by explicitly invoking its operation named "end".
\else
 the DataReaders should be locked for incoming updates by invoking the begin_access on the \ref DCPS_Modules_Subscription "Subscriber" prior to accessing the first \ref DCPS_Modules_Subscription_DataReader "DataReader".
If all concerned  DataReaders have been accessed properly, they can be unlocked for incoming updates by invoking the end_access on the \ref DCPS_Modules_Subscription "Subscriber".
\endif
Note that in this case a \ref DCPS_Modules_Subscription "Subscriber" is created in a disabled state. This allows the application to create all concerned DataReaders, preventing any transactions from completing prematurely
before all DataReaders have been created. The application must explicitly enable the subscriber after it has finished creating DataReaders.

If access_scope is set to GROUP and ordered_access is set to TRUE, then ordering is maintained between samples that are written
by DataWriters attached to a common \ref DCPS_Modules_Publication "Publisher" and received by DataReaders attached to a common \ref DCPS_Modules_Subscription "Subscriber". This way the subscribing application can
access the changes as a unit and/or in the proper order. However, this does not necessarily imply that the subscribing application will indeed access
the changes as a unit and/or in the correct order. For that to occur, the subscribing application must use the proper logic in accessing its datareaders:
  + Upon notification by the callback operation on_data_on_readers of the SubscriberListener or when triggered by the similar DATA_ON_READERS status of
the \ref DCPS_Modules_Subscription "Subscriber"’s StatusCondition,
\if isocpp2
the application starts a new scope holding a CoherentAccess object
\else
the application uses begin_access on the \ref DCPS_Modules_Subscription "Subscriber"
\endif
to indicate it will be accessing data through the \ref DCPS_Modules_Subscription "Subscriber". This will lock the embedded datareaders for any incoming messages during the
coherent data access.
  + Then it
\if isocpp2
 invokes dds::sub::find function
\else
 calls get_datareaders on the \ref DCPS_Modules_Subscription "Subscriber"
\endif
to get the list of \ref DCPS_Modules_Subscription_DataReader "DataReader" objects
where data samples are available. Note that when ordered_access is TRUE, then the list of DataReaders may contain the same reader several times. In
this manner the correct sample order can be maintained among samples in different \ref DCPS_Modules_Subscription_DataReader "DataReader" objects.
  + Following this it calls read or take on each \ref DCPS_Modules_Subscription_DataReader "DataReader" in the same order returned to access all the relevant changes in the \ref DCPS_Modules_Subscription_DataReader "DataReader". Note that when
ordered_access is TRUE, you should only read or take one sample at a time.
  + Once it has called read or take on all the readers,
\if isocpp2
 it ends the scope that holds the CoherentAccess object or it explicitly invokes its operation named "end"
\else
 it calls end_access on the \ref DCPS_Modules_Subscription "Subscriber".
\endif
This will unlock the embedded datareaders again.

The value offered is considered compatible with the value requested if and only if the following conditions are met:

-# The inequality “offered access_scope >= requested access_scope” evaluates to ‘TRUE.’ For the purposes of this inequality, the values of PRESENTATION access_scope are considered ordered such that INSTANCE < TOPIC < GROUP.

-# Requested coherent_access is FALSE, or else both offered and requested coherent_access are TRUE.

-# Requested ordered_access is FALSE, or else both offered and requested ordered _access are TRUE.

For a DataWriter that is attached to a Publisher which has coherent-access set to TRUE and the access-scope set to TOPIC or GROUP there is a constraint on the HistoryQosPolicy. In that case the HistoryQosPolicy should be set to KEEP-ALL. This constraint is applied because for a DataWriter with a HistoryQosPolicy set to KEEP-LAST the samples in the DataWriter history may be overwritten by new samples which causes that the corresponding transaction will not become complete anymore because of the missing samples.
