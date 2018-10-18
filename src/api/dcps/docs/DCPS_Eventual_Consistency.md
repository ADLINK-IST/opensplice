Eventual Consistency        {#DCPS_Eventual_Consistency}
====================

Eventual Consistency is an important property for distributed systems:
it guarantees that all subscribers to the same topic will eventually
converge to the same end-state. Of course not every \ref DCPS_Modules_Subscription "Subscriber" will
see the exact same state at every single moment in time (one subscriber
could have lost a sample during transmission and will need to wait for
its retransmission, while others could have received it successfully
already), but eventually all Subscribers will end up with the same end
state for the same instances.

Why is this such an important property for distributed systems? You
could argue that if each instance is updated periodically and some
Subscribers miss out on a sample, then they will eventually re-align
with the rest of the system upon reception of the next update for that
same instance. And that is indeed true for periodic data, but what if the
data is a-periodic in nature, or has a very long update period? Then one
or more Subscribers may feed their processing algorithms with different
input than similar Subscribers on other nodes and therefore reach a different
end state when compared to the others: their state is in that case said to be
not consistent with the state of the other Subscribers until the successful
reception of the next update for the same instance, which depending on the
update frequency of the Writer, may be for a long time to come.

It depends on the nature and type of your system whether it is a problem
if one or more Subscribers have a different view of the world than others
for a potentially long interval of time. In many systems however this is
undesired behavior, and it is the reason why several middleware products
offer things like reliable transmission protocols, and in case of DDS, why
we offer policies like durability: the idea is that any Subscriber no matter
where or when it is started, has the ability to obtain the same instance state
as all the other Subscribers to the same Topic in the same system.

This section will give some more background on which factors come into play
when you want your system to behave in an eventually consistent way and in
the exact mechanisms used to guarantee eventual consistency.

##Configuring for Eventual Consistency
The settings for the following QosPolicies play an important role in determining
whether eventual consistency can be guaranteed:
- \ref DCPS_QoS_Reliability "Reliability"
- \ref DCPS_QoS_Durability "Durability"
- \ref DCPS_QoS_DestinationOrder "Destination Order"

Let's talk about each one of these and their impact on eventual consistency.

###Reliability:
When a system needs to have Eventual Consistency, it either needs to have high
frequency periodic updates for all its instances, or it will need to use a
reliable protocol. Otherwise samples may be lost during transmission and will
cause the system not to be eventually consistent.
Please note that our reliable protocol is order preserving, which means it
will never deliver samples from the same source out of order. However, samples
from different sources may still be received out of order due to non-deterministic
latencies on the network.

###Durability:
When a system needs to have Eventual Consistency, it either needs to have high
frequency periodic updates for all its instances, or it will need to use either
TRANSIENT or PERSISTENT durability. Otherwise late joining Subscribers will not
be able to converge to the same instances state as the ones that were already
running when the data was actually published.

###Destination Order:
When a system needs to have Eventual Consistency, it will have to use a
BY_SOURCE_TIMESTAMP destination order and preferably synchronized clocks.
The latter becomes important when the same instance may be updated by multiple
Writers running on different nodes.

The effects of DestinationOrderQosPolicy especially matter when consecutive
updates are received out of order. The most common way in which is may happen is
if the consecutive updates originate from different Publishers and have different
latencies. If the latency of the first sample is longer than that of the second
sample but the second sample is published sooner than the difference in latencies,
then out of order delivery is a fact. But out of order delivery is even possible
when both samples originate from the same Publisher, even though the reliable protocol
guarantees no out of order delivery. An example of the latter case is a late joining
Transient DataReader who is requesting initial data from the durability service,
but who already receives a newer update directly from the originating \ref DCPS_Modules_Publication_DataWriter "DataWriter"
before the durability service was able to service his request.

So why does BY_RECEPTION_TIMESTAMP not guarantee eventual consistency in case of
out of order delivery? Consider the scenario where two Publishers are updating the
same instance: Publisher A sends its update at t1, Publisher B sends the update at
t2 where t2 > t1. So most Subscribers will receive the update from Publisher B last,
and agree that this represents the latest state. However, some Subscribers may have
received the update from Publisher A last (for example because they were late joiners
and the durability service sent the A sample after the B sample has been received
directly from Publisher B, or because Publisher A's sample was lost during transmission
and its retransmit arrived after the succesful delivery of Publisher's B message) and
will therefore conclude that Publisher's A sample represents the latest state. The
effect of this is that both groups of Subscribers are not guaranteed to be eventual
consistent.

So how does BY_SOURCE_TIMESTAMP guarantee eventual consistency in that case? Let's take the previous
example again and look at the consequences of the out of order delivery of the samples published
at t1 and t2. The latter sample arrived first, and it will be inserted into the Reader cache upon
arrival. Now when the sample with t1 arrives later, one of three things may happen:
-# If the t2 sample is still there and there is enough history depth left (KEEP_LAST with depth > 1
or KEEP_ALL with MAX_SAMPLES_PER_INSTANCE > 1), the t1 sample will be back inserted into the history
so that t2 will remain the latest state.
-# If the t2 sample is still there and there is not enough history depth left (for example KEEP_LAST with
depth = 1 or KEEP_ALL with MAX_SAMPLES_PER_INSTANCE = 1) then the t1 sample will be dropped so that t2 will
remain the latest state. In case of a KEEP_LAST policy this is not considered an issue, since only the latest
'depth' samples need to be stored, but in case of KEEP_ALL this violates the expectation that no samples
will ever be lost. So why choose this approach then? The answer is simple: delivering the sample in this case
would violate the resource limits that may not be exceeded. So in this case there is a direct conflict
between two orthogonal policies, and by dropping the sample we choose to satisfy the \ref DCPS_QoS_ResourceLimits "ResourceLimitsQosPolicy"
over the \ref DCPS_QoS_History "HistoryQosPolicy". However, the \ref DCPS_Modules_Subscription_DataReader "DataReader" will be informed about the dropped sample by notifying
the \ref DCPS_Status_SampleLost "SampleLostStatus", which may be picked up by the DataReader's Listener or its StatusCondition.
-# If the t2 sample has already been consumed (i.e. taken out of the DataReader), then the t1 sample
can no longer be inserted even though there would be enough resources in the history to store it,
as this would violate the eventual consistency. After all, how could the reading application know that the
t1 sample would not represent the latest known state in that case? For that reason each instance keeps track
of the source timestamp of the latest sample consumed: any incoming sample with a source timestamp older
than the latest one consumed will be dropped, even in case of a KEEP_ALL policy, as delivering it would
break eventual consistency. However, the DataReader will be informed about the dropped sample by notifying
its \ref DCPS_Status_SampleLost "SampleLostStatus", which may be picked up by the DataReader's Listener or its StatusCondition.

##Eventual Consistency in case of a disconnect/reconnect cycle.
Now what happens to a system when one part is temporarily disconnected from another
part? During this disconnect period both parts will basically continue to operate independently
and their states will slowly start to diverge when new updates can't flow from one part of
the system to the other part. Of course Eventual Consistency cannot be maintained in a system
that is physically disconnected (we call this the split-brain syndrome), but what happens when
the two parts are reconnected after they have diverged already? Can Eventual Consistency be
restored in these cases as well?

For the answer to the above question we need to dive a little bit deeper into the mechanics
of the instance state machine and of the merge policies configured for the durability service.
The next section will explain the consequences of a disconnect for your instance state, and the
section after that will explain the consequences of a re-connect for the same instance state.

###Visible effects of a disconnect.
A DataReader may become physically disconnected from the DataWriter that conceived a number
of the topic instances stored in its history. In such a case the DataReader acts as if the
disconnected DataWriter unregistered all its instances. That might impact the state of these
topic instances in the following way:
- If the \ref DCPS_Modules_Publication_DataWriter "DataWriter" has set its \ref DCPS_QoS_WriterDataLifecycle "DataWriterLifecycleQosPolicy"  field auto_dispose_unregistered_instances
to TRUE (the default setting), then the instance state of the concerned instances will go to
NOT_ALIVE_DISPOSED, no matter how many other (currently connected) DataWriters have still registered
the same instances.
- If the DataWriter has set its \ref DCPS_QoS_WriterDataLifecycle "DataWriterLifecycleQosPolicy" field auto_dispose_unregistered_instances
to FALSE, and no other (currently connected) DataWriters have still registered the concerned instances,
then the instance state of those instance will go to NOT_ALIVE_NO_WRITERS.
- If the DataWriter has set its \ref DCPS_QoS_WriterDataLifecycle "DataWriterLifecycleQosPolicy" field auto_dispose_unregistered_instances
to FALSE, but one or more (currently connected) DataWriters still have registered the concerned instances,
then their instance state will not be impacted and remain ALIVE.

Please note that in all cases mentioned above the disconnection of the DataWriter will also be
reflected in both the \ref DCPS_Status_LivelinessChanged "LivelinessChangedStatus" and the \ref DCPS_Status_SubscriptionMatched "SubscriptionMatchedStatus".

For Transient/Persistent data that is stored by the durability service, the same instance transitions
will occur. However, keep in mind that when instances are both disposed and unregistered, the durability
service will purge their samples after expiry of the service_cleanup_delay (0 seconds by default). That
means that if data needs outlive the lifespan of its originating DataWriter, such a DataWriter should set
its \ref DCPS_QoS_WriterDataLifecycle "DataWriterLifecycleQosPolicy" field auto_dispose_unregistered_instances to FALSE.

###Visible effects of a re-connect.
If a formerly disconnected DataWriter is re-connected to a DataReader, the state change caused by its
disconnect needs to be reverted. That means that the instance state should go back to ALIVE for all concerned
instances. However, at the same time we cannot undo the past, and just remove the DISPOSE/UNREGISTER sample
that caused the disconnect, for example because it, and/or the samples before it, have already been taken
by the application. Also, the disposed_generation_count or the no_writers_generation_count (depending on whether
the disconnect caused a DISPOSE or NO_WRITERS state) should be increased by one when the instance becomes ALIVE
again. So instead of selectively removing history, we just re-insert historical samples with the correct states
behind the sample that communicated the disconnect, despite the fact that according to their timestamps and
the \ref DCPS_QoS_DestinationOrder "DestinationOrderQosPolicy" the historical samples normally would need to be back-inserted into history before
the sample that communicated the disconnect. In the default case (KEEP_ALL with depth = 1) the newly inserted
historical sample will just push out the original historical sample, and so you might not notice the fact that
normal ordering is not applied here, but in case of KEEP_ALL or depth > 1, you might see the same sample with
the same timestamp listed twice: once before the disconnect with generation_count = n, and one after the disconnect
with generation_count = n + 1. The first occurrence of the sample may have already been set to READ_SAMPLE_STATE,
but the second occurrence of the same sample will always start with a NOT_READ_SAMPLE_STATE, and will set the
instance state back to NEW_INSTANCE_STATE.

Let's look at an example:
At time t1 a sample is being received by a DataReader, and at t2 that DataReader disconnects from the sample's
originating DataWriter. At time t3 the DataReader reconnects to that DataWriter. What would the history timeline
look like in that case if the history depth of our DataReader would be 3? In our timeline S(t1) represents a sample
written at t1, and D(t2) a dispose message at t2, and NW(t2) a NO_WRITERS message at t2.
- For auto_dispose_unregistered_instances = TRUE, the timeline would look like this: S(t1), D(t2), S(t1).
- For auto_dispose_unregistered_instances = FALSE, the timeline would look like this: S(t1), NW(t2), S(t1).

Note that in the above example, if your DataReader has KEEP_LAST with depth = 1, you will eventually see only the
last sample in the timeline. If your DataReader uses a depth > 1 or KEEP_ALL with max_samples_per_instance > 1 you
may see all three samples at the same time.

