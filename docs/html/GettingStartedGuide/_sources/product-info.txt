.. _`Product Details`:

###############
Product Details
###############

**************
Key Components
**************

Vortex OpenSplice includes the key components listed here.


Services
========

+ **Domain Service** (``spliced``)
  Manages a DDS domain.

+ **Durability Service**
  Responsible for handling non-volatile data.

+ **Networking Service** (``RTNetworking or DDSI2``)
  Responsible for handling communication between a
  node and the rest of the nodes on \`the network'.

+ **Tuner Service** (``cmsoap``)
  Responsible for providing remote SOAP-based access to a deployed 
  DDS system by providing a control-and-monitoring (C&M) API

+ **Networking Bridge Service**
  Responsible for bridging DDS traffic between multiple 
  networks (each with a related Networking Service)
  
+ **Recording and Replay Service**
  Responsible for recording and replaying of real-time DDS
  data, controlled a topic-based API (as also used by the 
  Vortex RnR Manager tool)
  
+ **DBMS Connect Service**
  Responsible for bi-directional bridging between DDS and a
  relational database (RDBMS) system 

Tools
=====

+ **IDL Preprocessor**
  Generates topic types, type-specific readers and writers.

+ **OpenSplice Tuner**
  Allows local and/or remote 'white-box' inspection and tuning 
  of a deployed Vortex OpenSplice federation and/or appliciation.

+ **OpenSplice Tester**
  Allows for script-based 'black-box' regression-testing
  of locally or remotely deployed Vortex OpenSplice Systems.
  
+ **OpenSplice Configurator**
  Simplifies the process for configuring the services.

+ **mmstat**
  Helps to monitor the memory usage within Vortex OpenSplice.



Key Features
============

+ Vortex OpenSplice is the most complete second generation OMG DDS
  implementation that supports all DCPS profiles.

+ It is proven in the field for a decade of deployment in
  mission critical environments.

+ It targets both real-time embedded and large-scale fault-tolerant systems.

+ It is Highly optimised implementation from DDS users for DDS users.

+ It offers total lifecycle support from prototyping through to remote maintenance.

+ Vortex OpenSplice supports both Single Process (library) and Shared Memory (federated)
  deployment architectures, targeting either ease of use or advanced scalability 
  and determinism scenarios.


******************************
Language and Compiler Bindings
******************************

The Vortex OpenSplice DCPS API is available for the following languages:

+ C

+ C++ (including OMG's latest ISO-C++ API)

+ C#

+ Java (including OMG's latest Java5 API)

With Vortex OpenSplice, there is also the ability to use the DDS and DCPS
APIs in CORBA cohabitation mode. Cohabitation allows you to use
objects in both DCPS and CORBA without copying them from one
representation to the other. This means that CORBA objects can be
published directly in DCPS and the other way around. There is no
difference in the DDS API, only in the generated code produced by the
``idlpp`` tool in the development process. Vortex OpenSplice has CORBA
cohabitation for C++ and Java, using (by default) OpenFusion TAO and
OpenFusion JacORB. Other variations are available, please check the
*Release Notes* for full platform and language ORB coverage.

The full range of language bindings available is:

+ C (Standalone only ) - sac

+ C++ (Standalone and CORBA Cohabitation) - isocpp / isocpp2 / sacpp / ccpp

+ C# (Standalone only) - sacs

+ Java (Standalone and CORBA Cohabitation) - saj / saj5 / cj

Vortex OpenSplice is delivered with a preset compiler for C++. Details of
this can be found in the *Release Notes*.

These compilers are the officially-supported set, but we have experience
of customers who will use the delivered libraries with slight variants
of the compiler. In most cases this works, but PrismTech has provided
the source code so that customers can rebuild the C++ APIs for their
compiler of choice.

|caution|
  **NOTE:** PrismTech only provides support on the officially-supported
  platforms due to difficult-to-fix issues with compiler-generated code,
  but some customers will fund us to qualify OpenSplice on their platform.
  If you wish to use a variant of an official platform, then as long as
  the issue can be recreated on the official platform it will be covered
  under an Vortex OpenSplice support contract. If you wish to request support
  on a specific platform then please contact PrismTech
  (http://www.prismtech.com/contact-us)

  
********************
Interaction patterns
********************

Apart from the DDS pub/sub interaction pattern (DCPS), Vortex OpenSplice also supports
the following additional patterns modeled on top of DDS:

+ RMI (a request-reply API based on modeled interfaces in IDL)

+ Streams (a streaming-data API based on transparent batching of samples)

************************************
Support for evolutionary data models
************************************

Apart from the OMG IDL based data-modeling, Vortex OpenSplice also supports modeling of
evolutionary types using the popular Google Protocol Buffers (GPB) technology (and the 
related .proto files).


  
*********************
Building your own C++ 
*********************

Building your own ISO C++ API
=============================

The Vortex OpenSplice DCPS API for the ISO C++ language binding without
CORBA cohabitation is delivered using a specific compiler.

To be able to use a different compiler with the Vortex OpenSplice ISO C++
API, we deliver the source code for this language with the OpenSplice
DDS distribution. This is contained in a directory
``<Vortex OpenSplice Installation directory>/custom_lib/isocpp``
together with a ``README`` file describing how to generate the custom library.

Building your own Standalone C++ API
====================================

The Vortex OpenSplice DCPS API for the C++ language binding without CORBA
cohabitation is delivered using a specific compiler.

To be able to use a different compiler with the Vortex OpenSplice
Standalone C++ API, we deliver the source code for this language with
the Vortex OpenSplice distribution. This is contained in a directory
``<Vortex OpenSplice Installation directory>/custom_lib/sacpp``
together with a ``README`` file describing how to generate the custom library.


*********
Platforms
*********

The platforms supported by Vortex OpenSplice are listed in the Release
Notes.

Please refer to :ref:`Platform-specific Information` for information
about using Vortex OpenSplice on specific platforms.




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

   
