.. _`RMI Runtime Configuration Options`:

#################################
RMI Runtime Configuration Options
#################################


The RMI runtime can be configured by a set of command line options.
These options are passed directly to the runtime start operation as
described in the section :ref:`Runtime starting and stopping`.

This chapter describes the set of supported options.

******************************
RMIClientThreadingModel option
******************************

``--RMIClientThreadingModel = [ST | MT]``

This option specifies the threading model of a given client.
The ``ST`` and ``MT`` option values set respectively the Single-Threaded
and Multi-Threaded models.


*********************************
RMIServiceDiscoveryTimeout option
*********************************

``--RMIServiceDiscoveryTimeout = <seconds>``

This is a client-side option that specifies the maximum duration
(in seconds) that a client application can wait to find services.
It influences the execution time of the DDS_Service.getServerProxy 
operation that is used to find a given service. The default value
is set to 10 seconds. The need to set this value may come from some
specific deployment environements with bad communication conditions.

******************************
RMIServerThreadingModel option
******************************

``--RMIServerThreadingModel=ST | MT | TPS [,<thread-pool-size>]``

This is a server-side option that specifies the threading policy of 
the server runtime including the threading policy name and the thread pool size.

  **ST**  selects *Single Threaded* policy.

  **MT**  selects *Multi Thread* policy.

  **TPS** selects *Thread Per Service* policy.

These policies are described in detail in the section
:ref:`Server Threading and Scheduling policies`.


*******************************
RMIServerSchedulingModel option
*******************************

.. [[!! DEVELOPMENT NOTE:
   Check status of this feature on each release !!
   !!]]


``--RMIServerSchedulingModel=<priority>``

This is a server-side option that specifies the scheduling
policy of a Java server RMI runtime.

********************
RMIDurability option
********************

|caution|
  **Note**: The ``RMIDurability`` option is
  currently *only* implemented for *C++*.

.. [[!! DEVELOPMENT NOTE:
   Check status of this feature on each release !!
   !!]]
  
|cpp|  

``--RMIDurability = yes | no``

This is a client-side and server-side option that indicates whether the
underlying DDS middleware support the non-default durability Qos policies
(TRANSIENT_LOCAL and above) or not.

By default, this option value is ``yes``.

RMI servers uses non-volatile topics for services advertising to allow 
late-joining clients to discover them. This option is useful for adapting 
services registration and discovery mechanisms when the durability support 
is missing in the underlying DDS middleware.

|caution|
  Note that this feature must be *either* enabled *or* disabled for *all* of 
  the RMI applications in a given DDS domain. It means that durability-*en*abled 
  (option value is ``yes``) RMI applications cannot be deployed with 
  durability-*dis*abled (option value is ``no``) RMI applications in 
  the same DDS domain.


*******************************
RMIClientSchedulingModel option
*******************************
|caution|
  **Note**: The ``RMIClientSchedulingModel`` option is
  currently *only* implemented for *Java*.
  
  
.. [[!! DEVELOPMENT NOTE:
   Check status of this feature on each release !!
   !!]]

|Java|

``--RMIClientSchedulingModel=<priority>``

This is a client-side option that specifies the priority of all the threads created
by OpenSplice RMI at the client side, including the AsyncWaiter thread, which is the
one that waits for asynchronous replies.


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

         
