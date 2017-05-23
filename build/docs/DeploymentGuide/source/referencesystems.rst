.. _`Example Reference Systems`:


#########################
Example Reference Systems
#########################

*The OpenSplice middleware can be deployed for different kinds of 
systems. This section identifies several different systems that will 
be used as reference systems throughout the rest of this manual. Each 
needs to be configured differently in order to fit its requirements. 
The intention of this section is to give the reader an impression of 
the possible differences in system requirements and the configuration 
aspects induced.*


Zero Configuration System
*************************

The OpenSplice middleware comes with a default configuration file 
that is intended to give a satisfactory out-of-the-box experience. It 
suits the standard situation of a system containing a handful of 
nodes with only a few applications per node (enabling standalone 
‘single-process’ deployment) and where requirements on data 
distribution latencies, volumes and determinism are not too demanding 
(enabling use of the standard DDSI networking service).

Starting and running any systems that satisfy these conditions should 
not be a problem. Nodes can be started and shut down without any extra 
configuration because the default discovery mechanism will keep track 
of the networking topology.

Single Node System
******************

Systems that have to run as a federation on a single node can be 
down-scaled considerably by not starting the networking and 
durability daemons. The networking daemon is obviously not needed 
because its responsibility is forwarding data to and from the 
network, which is not present. The durability daemon is not needed 
because the OpenSplice libraries themselves are capable of handling 
durable data on a single node.

Note that this is not the case for single process deployments. 
Multiple single process applications that are running on the same 
machine node can only communicate when there is a networking service 
running within each process. This is because there is no shared 
administration between the applications, unlike for the shared memory 
deployments when a networking service is not required for a single 
node system.

With a single node system, the OpenSplice services do not have much 
influence on application behaviour. The application has full control 
over its own thread priorities and all OpenSplice activities will be 
executed in the scope of the application threads.

One exception to this is the listener thread. This thread is 
responsible for calling listener functions as described in the DDS 
specification.

Medium Size Static (Near) Real-time System
******************************************

Many medium size systems have highly demanding requirements with 
respect to data distribution latencies, volumes and predictability. 
Such systems require configuration and tuning at many levels. The 
OpenSplice middleware will be an important player in the system and 
therefore is highly configurable in order to meet these requirements. 
Every section reflects on an aspect of the configuration.

High Volumes
============

The OpenSplice middleware architecture is designed for efficiently 
transporting many small messages. The networking service is capable 
of packing messages from different writing applications into one 
network packet. For this, the latency budget quality of service 
should be applied. A latency budget allows the middleware to optimise 
on throughput. Messages will be collected and combined during an 
interval allowed by the latency budget. This concerns networking 
traffic only.

A network channel that has to support high volumes should be 
configured to do so. By default, the ``Resolution`` parameter is set to 
``50 ms``. This means that latency budget values will be truncated to 
multiples of 50 ms, which is a suitable value. For efficient packing, 
the ``FragmentSize`` should be set to a large value, for example ``8000``. 
This means that data will be sent to the network in chunks of 8 
kilobytes. A good value for ``MaxBurstSize`` depends on the speed of the 
attached network and on the networking load. If several writers start 
writing simultaneously at full speed during a longer period, 
receiving nodes could start losing packets. Therefore, the writers 
might need  to be slowed down to a suitable speed.

Note that message with a large latency budget might be overtaken by 
messages with a smaller latency budget, especially if they are 
transported *via* different networking channels.

Low Latencies
=============

If messages are to be transported with requirements on their end to 
end times, a zero latency budget quality of service should be 
attached. This results in an immediate wake-up of the networking 
service at the moment that the message arrives in a networking queue. 
For optimal results with respect to end-to-end latencies, the thread 
priority of the corresponding networkChannel should be higher than 
the thread priority of the writing application. with the current 
implementation, a context switch between the writing application and 
the networking channel is always required. With the correct 
priorities, the induced latency is minimized.

The value of the ``Resolution`` parameter has its consequences for using 
latency budgets. The networking service will ignore any latency 
budgets that have a value smaller than Resolution.

The effect of sending many messages with a zero latency budget is an 
increase of CPU load. The increasing number of context switches 
require extra processing. This quality of service should therefore be 
consciously used.

Responsiveness
==============

Especially with respect to reliable transport over the network, 
responsiveness is an important aspect. Whenever a reliably sent 
message is lost on the network, the sending node has to initiate a 
resend. Since OpenSplice networking uses an acknowledgement protocol, 
it is the up to the sending side to decide when to resend a message. 
This behaviour can be tuned.

First of all, the ``Resolution`` parameter is important. This parameter 
gives the interval at which is checked if any messages have to be 
resent. The ``RecoveryFactor`` parameter indicates how many of these 
checks have to be executed before actually resending a message. If 
Resolution is scaled down, messages will be resent earlier. If 
Recovery factor is scaled down, messages will be resent earlier as 
well.

Topology Discovery
==================

OpenSplice RT-Networking Service implements a discovery protocol for 
discovering other nodes in the system. As long as only one node is 
present, nothing has to be sent to the network. As soon as at least 
two nodes are present, networking starts sending data to the network. 
The node-topology detection also allows for quick reaction to 
topology changes, such as when a publishing node disappears (due to a 
disconnection or a node crash); a ‘tightly-configured’ discovery 
allows for swift detection of such topology changes, and related 
updating of DDS-level liveliness administration.

.. EoF

