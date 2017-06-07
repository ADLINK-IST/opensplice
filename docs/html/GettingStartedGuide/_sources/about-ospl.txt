.. _`About OpenSplice`:

#######################
About Vortex Opensplice
#######################


*********************
Why Vortex OpenSplice
*********************

What is Vortex OpenSplice?
==========================

The purpose of Vortex OpenSplice is to provide an infrastructure and
middleware layer for real-time distributed systems. This is a
realisation of the OMG-DDS-DCPS Specification for a Data Distribution
Service based upon a Data Centric Publish Subscribe architecture.


Why Use It?
===========

Vortex OpenSplice provides an infrastructure for real-time data
distribution and offers middleware services to applications. It provides
a real-time data distribution service that aims at:

+ reducing the complexity of the real-time distributed systems

+ providing an infrastructure upon which fault-tolerant real-time systems
  can be built

+ supporting incremental development and deployment of systems



Vortex OpenSplice Summary
=========================

*Vortex OpenSplice* is the leading (commercial and Open Source) implementation
of the Object Management Group's (OMG)
*Data Distribution Service (DDS) for Real-Time Systems* datasharing
middleware standard. Vortex OpenSplice is an advanced and proven data-centric solution that enables seamless,
timely, scalable and dependable distributed data sharing. Vortex OpenSplice
delivers the right data, in the right place, at the right time, every
time--even in the *largest-scale mission- and business-critical systems*.


**Key features and benefits are:**

+ Genuinely the fastest, most scalable and most reliable Open Source
  integration technology

+ Deployed in the most challenging business- and mission-critical systems

+ Genuine Open Source Apache 2.0 licensing for both the Community and
  Commercial editions

+ Field-proven interoperability with other DDS implementations

+ Largest ecosystem of plug-ins and tools for modeling, deployment
  and testing

+ Richest set of QoS policies for controlling efficiency, determinism and
  fault-tolerance

+ Supported by world-renowned professional services expertise from the
  developers of the DDS standard

+ Unprecedented throughput of millions of samples/sec for
  typical 'stream' data


Please go to http://www.prismtech.com to obtain evaluation copies of
Vortex OpenSplice, and http://www.opensplice.org for free downloads of the
community version.


******************************
Vortex OpenSplice Architecture
******************************


Overall
=======

To ensure scalability, flexibility and extensibility, Vortex OpenSplice has
an internal architecture that, when selected, uses shared memory to
\`interconnect' not only all applications that reside within one
computing node, but also \`hosts' a configurable and extensible set of
services. These services provide \`pluggable' functionality such as
networking (providing QoS driven real-time networking based on multiple
reliable multicast \`channels'), durability (providing fault tolerant
storage for both real-time \`state' data as well as persistent
\`settings'), and remote control & monitoring \`soap service' (providing
remote web based access using the SOAP protocol from the OpenSplice DDS
Tuner tools).


Scalability
===========

Vortex OpenSplice is capable of using a shared-memory architecture where
data is physically present only once on any machine, and where smart
administration still provides each subscriber with his own private
\`view' on this data. This allows a subscriber's data cache to be
perceived as an individual \`database' that can be content-filtered,
queried, etc. (using the content-subscription profile as supported by
OpenSplice DDS). This shared-memory architecture results in an extremely
small footprint, excellent scalability and optimal performance when
compared to implementations where each reader/writer are \`communication
endpoints' each with its own storage (in other words, historical data
both at reader and writer) and where the data itself still has to be
moved, even within the same physical node.


Configuration
=============
            
Vortex OpenSplice is highly configurable, even allowing the architectural
structure of the DDS middleware to be chosen by the user at deployment
time. Vortex OpenSplice can be configured to run using a *shared memory*
architecture, where both the DDS related administration (including the
optional pluggable services) and DDS applications interface directly
with shared memory. Alternatively, Vortex OpenSplice also supports a
*single process* library architecture, where one or more DDS
applications, together with the OpenSplice administration and services,
can all be grouped into a single operating system process. Both
deployment modes support a configurable and extensible set of services,
providing functionality such as:

+ *networking* - providing real-time networking options allowing for
  trade-offs between \`interoperability' (with other DDS vendors) and
  scalability (supporting ultra-large systems).

+ *durability* - providing fault-tolerant storage for both real-time state
  data as well as persistent settings

+ *remote control and monitoring SOAP service* - providing remote web-based
  access using the SOAP protocol from the OpenSplice Tuner tool

+ *dbms service* - providing a connection between the real-time and the
  enterprise domain by bridging data from DDS to DBMS and *vice versa*

The Vortex OpenSplice middleware can be easily configured, on the fly,
using its pluggable architecture: the services that are needed can be
specified together with their optimum configuration for the particular
application domain, including networking parameters, and durability
levels for example).

There are advantages to both the single process and shared memory
deployment architectures, so the most appropriate deployment choice
depends on the user's exact requirements and DDS scenario.


***********************************
Single Process Library Architecture
***********************************

This deployment allows the DDS applications and OpenSplice
administration to be contained together within one single operating
system process. This single process deployment option is most useful in
environments where shared memory is unavailable or undesirable. As
dynamic heap memory is utilized in the single process deployment
environment, there is no need to pre-configure a shared memory segment
which in some use cases is also seen as an advantage of this deployment
option.

Each DDS application on a processing node is implemented as an
individual, self-contained operating system process (*i.e.* all of the
DDS administration and necessary services have been linked into the
application process). This is known as a *single process application*.
Communication between multiple single process applications co-located on
the same machine node is done via the (loop-back) network, since there
is no memory shared between them.

An extension to the single process architecture is the option to
co-locate multiple DDS applications into a single process. This can be
done be creating application libraries rather than application
executables that can be \`linked' into the single process in a similar
way to how the DDS middleware services are linked into the single
process. This is known as a *single process application cluster*.
Communication between clustered applications (that together form a
single process) can still benefit from using the process's heap memory,
which typically is an order of magnitude faster than using a network,
yet the lifecycle of these clustered applications will be tightly
coupled.

The Single Process deployment is the default architecture provided
within Vortex OpenSplice and allows for easy deployment with minimal
configuration required for a running DDS system.

The figure `The OpenSplice Single Process Architecture`_
shows an overview of the single process architecture of Vortex OpenSplice.

.. _`The OpenSplice Single Process Architecture`:

.. figure:: /images/SingleProcessArchitecture.png
   :height: 70mm
   :alt: The OpenSplice Single Process Architecture

   **The OpenSplice Single Process Architecture**


**************************
Shared Memory architecture
**************************

In the shared memory architecture data is physically present only once
on any machine but smart administration still provides each subscriber
with his own private view on this data. Both the DDS applications and
OpenSplice administration interface directly with the shared memory
which is created by the OpenSplice daemon on start up. This architecture
enables a subscriber's data cache to be seen as an individual database
and the content can be filtered, queried, etc. by using the OpenSplice
content subscription profile.

Typically for advanced DDS users, the shared memory architecture is a
more powerful mode of operation and results in extremely low footprint,
excellent scalability and optimal performance when compared to the
implementation where each reader/writer are communication end points
each with its own storage (*i.e.* historical data both at reader and
writer) and where the data itself still has to be moved, even within the
same platform.

The figure `The OpenSplice Shared Memory Architecture`_
shows an overview of the shared memory architecture of 
Vortex OpenSplice on *one* computing node. Typically, there
are *many* nodes within a system.

.. _`The OpenSplice Shared Memory Architecture`:

.. figure:: /images/SharedMemoryArchitecture.png
   :height: 70mm
   :alt: The OpenSplice Shared Memory Architecture
   
   **The OpenSplice Shared Memory Architecture**


************************************
Vortex OpenSplice Features and Benefits
************************************

The table below shows the following
aspects of Vortex OpenSplice, where:

  *Features* are significant characteristics of OpenSplice

  *Advantages* shows why a feature is important

  *Benefits* describes how users of OpenSplice can exploit the advantages


+----------------+-------------------+--------------------------+-------+
| Features       | Advantages        | Benefits                 |  =+!  |
+================+===================+==========================+=======+
| **GENERAL**                                                           |
+----------------+-------------------+--------------------------+-------+
| Information-   | Enable dynamic,   | Simplified and better    |   =   |
| centric        | loosely-\         | scalable architectures   |       |
|                | coupled system    |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Open standard  | 'Off-the-shelf'   | Lower cost, no vendor    |   =   |
|                | solutions         | lock-in                  |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Built on       | Intended for the  | Assured quality and      |  ++   |
| proven         | most demanding    | applicability            |       |
| technology     | situations        |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| TNN/PT         | Decade of  'DDS'  | Proven suitability in    |  !!!  |
| 'inheritance'  | experience        | mission-critical         |       |
|                |                   | domain                   |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Open Source    | Strong and large  | Security of supply of    |  !!!  |
| model          | user community    | most widely-used DDS     |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| **FUNCTIONAL**                                                        |
+----------------+-------------------+--------------------------+-------+
| Real-time      | Dynamic/\         | Autonomous decoupled     |   =   |
| pub/sub        | asynchronous data | applications             |       |
|                | communication     |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Persistence    | Fault-tolerant    | Application fault        |  !!!  |
| profile        | data persistence  | tolerance and data high  |       |
|                |                   | availability             |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Content-sub.   | Reduced           | Easier application       |  !!!  |
| Profile        | complexity and    | design and scalable      |       |
|                | higher            | systems                  |       |
|                | performance       |                          |       |
+----------------+-------------------+--------------------------+-------+
| **PERFORMANCE**                                                       |
+----------------+-------------------+--------------------------+-------+
| Shared         | Small footprint,  | Processor scalability    |  ++   |
| memory         | instant data      |                          |       |
|                | availability      |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Smart          | Efficient data    | Network scalability      |  ++   |
| networking     | transport         |                          |       |
|                |                   |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Extensive IDL  | Includes          | Data scalability         |  ++   |
| support        | unbounded strings |                          |       |
|                | and sequences     |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| **USABILITY**                                                         |
+----------------+-------------------+--------------------------+-------+
| Multiple       | Any (mix) of      | Supports legacy code,    |  ++   |
| languages      | C, C++, Java,     | allows hybrid systems    |       |
|                | C#                |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Multiple       | Any (mix) of      | Intercons, Enterprise    |   =   |
| platforms      | Enterprise and    | and embedded systems     |       |
|                | RTE OSs           |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| **INTEROPERABILITY**                                                  |
+----------------+-------------------+--------------------------+-------+
| DDSI/RTPS      | Interoperability  | Smooth integration with  |  !!!  |
|                | between DDS       | non-OpenSplice (legacy)  |       |
|                | vendors           | DDS systems              |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| **TOOLING AND EASE OF USE**                                           |
+----------------+-------------------+--------------------------+-------+
| All metadata   | Dynamic discovery | Guaranteed data          |   =   |
| at runtime     | of all 'entity    | integrity                |       |
|                | info'             |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Powerful       | Support for       | Enhanced productivity    |  !!!  |
| tooling        | complete system   | and System Integration   |       |
|                | lifecycle         |                          |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
| Remote         | Web-based         | Remote diagnostics       |  !!!  |
| connect        | remote access     | using standard           |       |
|                | and control       | protocols                |       |
|                |                   |                          |       |
+----------------+-------------------+--------------------------+-------+
|                                                                       |
+----------------+-------------------+--------------------------+-------+
| **Legend:**                        | *Equal to the*           |  \=   |
|                                    | *competition*            |       |
|                                    |                          |       |
|                                    | *Better than the*        |  ++   |
|                                    | *competition*            |       |
|                                    |                          |       |
|                                    | *Far superior to the*    |  !!!  |
|                                    | *competition*            |       |
+----------------+-------------------+--------------------------+-------+


**********
Conclusion
**********

PrismTech's Vortex OpenSplice product complemented by its tool support
together encompass the industry's most profound expertise on the OMG's
DDS standard and products.

The result is unrivalled functional DDS coverage and performance in
large-scale mission-critical systems, fault tolerance in information
availability, and total lifecycle support including round-trip
engineering. A complete DDS solution to ensure a customer's successful
adoption of this exciting new technology and to support delivery of the
highest-quality applications with the shortest time to market in the
demanding real-time world.



.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm

