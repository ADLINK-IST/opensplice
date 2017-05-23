.. _`Introduction`:


############
Introduction
############

*The Vortex OpenSplice * Record and Replay Service *is a pluggable service of the 
Vortex OpenSplice middleware which is capable of recording and/or replaying 
DDS data-sets (i.e. topic-samples) in a DDS system.*

As a DDS service, the Record and Replay Service (*RnR Service*, or 
just *RnR*) benefits from the inherent ‘decoupling in time and space’ 
that the DDS architecture offers, with respect to automatic discovery 
of the service’s dynamic interest in subscribing or publishing data 
as well as the transparent routing of information to/from the service.

The RnR Service operates in conjunction with storages, which can be 
configured statically in a configuration file or dynamically through 
interaction with the service. To interact with the service, 
a command-topic and various status-topics are available.





Features
********

You can use the Record and Replay Service to:

+ Control and monitor the service using regular DDS topics.
+ Use expressions with wildcards to select partitions and 
  topics of interest, for record and/or replay.
+ Store data in XML records for easy post-analysis and data-mining.
+ Store data in CDR records for a smaller footprint and high throughput.
+ Create scenarios, grouping multiple commands into a logical set.
+ Use replay filters to replay only the data recorded in a specific time-range.
+ Use conditions to delay the execution of a command.
+ Subscribe to statistics on the data that is replayed and/or recorded.
+ Dynamically change the speed at which data is replayed.
+ Modify the QoS settings of recorded data on-the-fly during replay
+ Modify the partition of recorded data on-the-fly during replay


.. EoF
