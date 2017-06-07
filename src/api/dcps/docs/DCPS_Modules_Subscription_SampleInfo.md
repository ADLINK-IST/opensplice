The SampleInfo      {#DCPS_Modules_Subscription_SampleInfo}
===============

The SampleInfo contains information pertaining to the associated Data value:

- The sample_state of the Data value (i.e., if the sample has already been READ or NOT_READ by that same DataReader).
- The view_state of the related instance (i.e., if the instance is NEW, or NOT_NEW for that DataReader) – see below.
- The instance_state of the related instance (i.e., if the instance is ALIVE, NOT_ALIVE_DISPOSED, or NOT_ALIVE_NO_WRITERS) – see below.
- The valid_data flag. This flag indicates whether there is data associated with the sample. Some samples do not contain data indicating only a change on the instance_state of the corresponding instance – see below.
- The values of disposed_generation_count and no_writers_generation_count for the related instance at the time the sample was received. These counters indicate the number of times the instance had become ALIVE (with instance_state= ALIVE) at the time the sample was received – see below.
- The sample_rank and generation_rank of the sample within the returned sequence. These ranks provide a preview of the samples that follow within the sequence returned by the read or take operations.
- The absolute_generation_rank of the sample within the DataReader. This rank provides a preview of what is available within the DataReader.
- The source_timestamp of the sample. This is the timestamp provided by the DataWriter at the time the sample was produced.

### Interpretation of the SampleInfo instance_state

For each instance the middleware internally maintains an instance_state. The instance_state can be ALIVE, NOT_ALIVE_DISPOSED, or NOT_ALIVE_NO_WRITERS.

- ALIVE indicates that (a) samples have been received for the instance, (b) there are live DataWriter entities writing the instance, and (c) the instance has not been explicitly disposed (or else more samples have been received after it was
disposed).
- NOT_ALIVE_DISPOSED indicates the instance was explicitly disposed by a DataWriter by means of the dispose operation.
- NOT_ALIVE_NO_WRITERS indicates the instance has been declared as not-alive by the DataReader because it detected that there are no live DataWriter entities writing that instance.

The precise behavior events that cause the instance_state to change depends on the setting of the OWNERSHIP QoS:

- If OWNERSHIP is set to EXCLUSIVE, then the instance_state becomes NOT_ALIVE_DISPOSED only if the DataWriter that “owns” the instance16 explicitly disposes it. The instance_state becomes ALIVE again only if the DataWriter that owns the instance writes it.

- If OWNERSHIP is set to SHARED, then the instance_state becomes NOT_ALIVE_DISPOSED if any DataWriter explicitly disposes the instance. The instance_state becomes ALIVE as soon as any DataWriter writes the instance again.

The instance_state available in the SampleInfo is a snapshot of the instance_state of the instance at the time the collection was obtained (i.e., at the time read or take was called). The instance_state is therefore the same for all samples in the returned collection that refer to the same instance.

### Interpretation of the SampleInfo valid_data

Normally each DataSample contains both a SampleInfo and some Data. However there are situations where a DataSample contains only the SampleInfo and does not have any associated data. This occurs when the Service notifies the application of a change of state for an instance that was caused by some internal mechanism (such as a timeout) for which there is no associated data. An example of this situation is when the Service detects that an instance has no writers and changes the corresponding instance_state to NOT_ALIVE_NO_WRITERS.

The actual set of scenarios under which the middleware returns DataSamples containing no Data is implementation dependent. The application can distinguish whether a particular DataSample has data by examining the value of the valid_data flag. If this flag is set to TRUE, then the DataSample contains valid Data. If the flag is set to FALSE, the DataSample contains no Data.

To ensure correctness and portability, the valid_data flag must be examined by the application prior to accessing the Data associated with the DataSample and if the flag is set to FALSE, the application should not access the Data associated with the DataSample, that is, the application should access only the SampleInfo.

### Interpretation of the SampleInfo disposed_generation_count and no_writers_generation_count

For each instance the middleware internally maintains two counts: the disposed_generation_count and
no_writers_generation_count, relative to each DataReader:

- The disposed_generation_count and no_writers_generation_count are initialized to zero when the DataReader first detects the presence of a never-seen-before instance.
- The disposed_generation_count is incremented each time the instance_state of the corresponding instance changes from NOT_ALIVE_DISPOSED to ALIVE.
- The no_writers_generation_count is incremented each time the instance_state of the corresponding instance changes from NOT_ALIVE_NO_WRITERS to ALIVE.

The disposed_generation_count and no_writers_generation_count available in the SampleInfo capture a snapshot of the corresponding counters at the time the sample was received.

### Interpretation of the SampleInfo sample_rank, generation_rank, and absolute_generation_rank
The sample_rank and generation_rank available in the SampleInfo are computed based solely on the actual samples in the ordered collection returned by read or take.

- The sample_rank indicates the number of samples of the same instance that follow the current one in the collection.
- The generation_rank available in the SampleInfo indicates the difference in ‘generations’ between the sample (S) and the **M** ost **R** ecent **S** ample of the same instance that appears **I** n the returned **C** ollection ( **MRSIC** ). That is, it counts the number of times the instance transitioned from not-alive to alive in the time from the reception of the S to the reception of MRSIC.

The generation_rank is computed using the formula:

*generation_rank = (MRSIC.disposed_generation_count + MRSIC.no_writers_generation_count) - (S.disposed_generation_count + S.no_writers_generation_count)*

The absolute_generation_rank available in the SampleInfo indicates the difference in ‘generations’ between the sample (S) and the Most Recent Sample of the same instance that the middleware has received (MRS). That is, it counts the number of times the instance transitioned from not-alive to alive in the time from the reception of the S to the time when the read or take was called.

*absolute_generation_rank = (MRS.disposed_generation_count + MRS.no_writers_generation_count) - (S.disposed_generation_count + S.no_writers_generation_count)*

### Interpretation of the SampleInfo counters and ranks

These counters and ranks allow the application to distinguish samples belonging to different ‘generations’ of the instance. Note that it is possible for an instance to transition from not-alive to alive (and back) several times before the application accesses the data by means of read or take. In this case the returned collection may contain samples that cross generations (i.e., some samples were received before the instance became not-alive, others after the instance reappeared again).
Using the information in the SampleInfo the application can anticipate what other information regarding the same instance appears in the returned collection, as well as, in the infrastructure and thus make appropriate decisions. For example, an application desiring to only consider the most current sample for each instance would only look at samples with sample_rank==0. Similarly an application desiring to only consider samples that correspond to the latest generation in the collection will only look at samples with generation_rank==0. An application desiring only samples pertaining to the latest generation available will ignore samples for which absolute_generation_rank != 0. Other application-defined criteria may also be used.

### Interpretation of the SampleInfo view_state
For each instance (identified by the key), the middleware internally maintains a view_state relative to each DataReader.
The view_state can either be NEW or NOT_NEW.

-  NEW indicates that either this is the first time that the DataReader has ever accessed samples of that instance, or else that the DataReader has accessed previous samples of the instance, but the instance has since been reborn (i.e., become not-alive and then alive again). These two cases are distinguished by examining the disposed_generation_count and the no_writers_generation_count.
- NOT_NEW indicates that the DataReader has already accessed samples of the same instance and that the instance has not been reborn since.

The view_state available in the SampleInfo is a snapshot of the view_state of the instance relative to the DataReader used to access the samples at the time the collection was obtained (i.e., at the time read or take was called). The view_state is therefore the same for all samples in the returned collection that refer to the same instance.

Once an instance has been detected as not having any “live” writers and all the samples associated with the instance are ‘taken’ from the DataReader, the middleware can reclaim all local resources regarding the instance. Future samples will be treated as ‘never seen’.

![Statechart of the instance_state and view_state for a single instance] (@ref SubscriptionModule_SampleInfo_InstanceState_UML.png)

### Data access patterns

The application accesses data by means of the operations read or take on the DataReader. These operations return an ordered collection of DataSamples consisting of a SampleInfo part and a Data part. The way the middleware builds this collection depends on QoS policies set on the DataReader and Subscriber, as well as the source timestamp of the samples, and the parameters passed to the read/take operations, namely:

- The desired sample states (i.e., READ, NOT_READ, or both).
- The desired view states (i.e., NEW, NOT_NEW, or both).
- The desired instance states (ALIVE, NOT_ALIVE_DISPOSED, NOT_ALIVE_NO_WRITERS, or a combination of these).

The read and take operations are non-blocking and just deliver what is currently available that matches the specified states.

The read_w_condition and take_w_condition operations take a ReadCondition object as a parameter instead of sample, view, and instance states. The behavior is that the samples returned will only be those for which the condition is TRUE. These operations, in conjunction with ReadCondition objects and a WaitSet, allow performing waiting reads (see below).
Once the data samples are available to the data readers, they can be read or taken by the application. The basic rule is that the application may do this in any order it wishes. This approach is very flexible and allows the application ultimate control. However, the application must use a specific access pattern in case it needs to retrieve samples in the proper order received, or it wants to access a complete set of coherent changes.

To access data coherently, or in order, the PRESENTATION QoS on must be set properly and the application must conform to the access pattern described below. Otherwise, the application will still access the data but will not necessarily see all coherent changes together, nor will it see the changes in the proper order.

There is a general pattern that will provide both ordered and coherent access across multiple DataReaders. This pattern will work for any settings of the PRESENTATION QoS. Simpler patterns may also work for specific settings of the QoS as described below.

1 . General pattern to access samples as a coherent set and/or in order across DataReader entities. This case applies when PRESENTATION QoS specifies “access_scope=GROUP.”
- Upon notification to the SubscriberListener or following the similar StatusCondition enabled, the application uses begin_access on the Subscriber to indicate it will be accessing data through the Subscriber.
    - Then it calls get_datareaders on the Subscriber to get the list of DataReader objects where data samples are available.
    - Following this it calls read or take on each DataReader in the same order returned to access all the relevant changes in the DataReader.
    - Once it has called read or take on all the readers, it calls end_access.

Note that if the PRESENTATION QoS policy specifies ordered_access=TRUE, then the list of DataReaders may return the same reader several times. In this manner the correct sample order can be maintained among samples in different DataReader objects.

2 . Specialized pattern if no order or coherence needs to be maintained across DataReader entities.This case applies if PRESENTATION QoS policy specifies access_scope something other than GROUP.
    - In this case, it is not required for the application to call begin_access and end_access. However, doing so is not an error and it will have no effect.
    - The application accesses the data by calling read or take on each DataReader in any order it wishes.
    - The application can still call get_datareaders to determine which readers have data to be read, but it does not need to read all of them, nor read them in a particular order. Furthermore, the return of get_datareaders will be logically a “set” in the sense that the same reader will not appear twice, and the order of the readers returned is not specified.

3 . Specialized pattern if the application accesses the data within the SubscriberListener. This case applies regardless of the PRESENTATION QoS policy when the application accesses the data inside the listener’s implementation of the on_data_on_readers operation.
    - Similar to the previous case (2 above), it is not required for the application to call begin_access and end_access, but doing so has no effect.
    - The application can access data by calling read or take20 on each DataReader in any order it wishes.
    - The application can also delegate the accessing of the data to the DataReaderListener objects installed on each DataReader by calling notify_datareaders.
    - Similar to the previous case (2 above), the application can still call get_datareaders to determine which readers have data to be read, but it does not need to read all of them, nor read them in a particular order. Furthermore, the return of get_datareaders will be logically a ‘set.’
