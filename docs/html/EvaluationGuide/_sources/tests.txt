.. _`Performance Tests and Examples`:

##############################
Performance Tests and Examples
##############################

To make the evaluation process as easy as possible, OpenSplice 
Enterprise is shipped with dedicated performance tests that can be 
used to measure latency and throughput. The tests are simple and 
clear, allowing the user to obtain performance results easily. 

The easiest way to build and run the performance tests is to use the 
OpenSplice Enterprise *Launcher* tool. In the *Examples* menu select the 
specific example and the appropriate language and configuration. 
Click the *Compile Example* button and then *Run Example*. This will run 
the DDS applications, and if running with a shared memory 
configuration it will also manage the starting and stopping of 
OpenSplice Enterprise. 

OpenSplice Enterprise also provides dedicated performance testing 
scripts which: 

+ Test multiple API bindings 

+ Use a varying range of payload sizes 

+ Timestamp and append results to a CSV file 

+ Set process priority and CPU affinity 


Please see the ``html`` files for the individual performance tests for details 
of how to run these scripts. 

******************************
Round-Trip Latency Performance
******************************

The latency of a DDS implementation is an expression of how fast data 
can be delivered between two DDS applications. *Round-trip latency* is 
the time taken for an individual DDS data sample to be delivered from 
Application A to Application B and back again, so importantly it 
includes metrics for both data delivery and reception. 

The easiest way to build and run the performance tests is to use the 
OpenSplice Enterprise *Launcher* tool as explained above. 

Alternatively, to manually build and run the round-trip performance 
test, for example for the ISO C++ API: 

|linux|

.. code-block:: bash

   # In an OpenSplice Enterprise environment:
   cd $OSPL_HOME/examples/dcps/RoundTrip/isocpp
   make
    
   cd $OSPL_HOME/examples/dcps/RoundTrip/isocpp
   # If using shared memory do "ospl start"
   ./pong
   # If using shared memory do "ospl stop" 

   # In another OpenSplice Enterprise environment:
   cd $OSPL_HOME/examples/dcps/RoundTrip/isocpp
   # If using shared memory do "ospl start"
   ./ping 20 100
   # If using shared memory do "ospl stop" 




|windows|

.. code-block:: bat

   # Load the OpenSplice DDS examples project solution 
   # into Visual Studio and build the required projects 
   
   # In an OpenSplice Enterprise environment:
   cd %OSPL_HOME%\examples\dcps\RoundTrip\isocpp
   # If using shared memory do "ospl start" 
   pong.exe
   # If using shared memory do "ospl stop" 
   
   # In another OpenSplice Enterprise environment:
   cd %OSPL_HOME%\examples\dcps\RoundTrip\isocpp
   # If using shared memory do "ospl start" 
   ping.exe 20 100     
   # If using shared memory do "ospl stop" 


The ``ping`` application will report the roundtrip time taken to 
send DDS data samples back and forth between the applications. 
The test utilizes the ``ReliabilityQoS`` set to ``RELIABLE`` by default 
in order to show the maximal performance whilst maintaining the guaranteed 
delivery of DDS samples. See the ``README`` file for the test for further 
details. 

**The lowest roundtrip latency may be achieved by tuning the test 
parameters appropriately.**

As mentioned above, the performance testing script described in the 
``html`` for the example is a convenient way to test and record the 
running of this test. 

|caution|
  Note that the default ``OSPL_URI`` value refers to a *Single Process* 
  deployment with *DDSI* networking. 

+ To observe the best performance within a node it is suggested 
  that you use a *Shared Memory* configuration. 

+ To observe the best performance between nodes it is suggested 
  that you use an *RTNetworking* service configuration. 

**********************
Throughput Performance
**********************

The throughput of a DDS implementation is an expression of the rate 
of data delivery through the DDS system. Measured in bits per second, 
it describes the ability of the DDS implementation to effectively 
deliver DDS data without data loss. 

As with the round-trip test, the easiest way to build and run the 
throughput performance test is to use the OpenSplice Enterprise 
*Launcher* tool. 

Alternatively, to manually build and run the throughput performance 
test, for example for the ISO C++ API: 

|linux|

.. code-block:: bash

   # In an OpenSplice Enterprise environment: 
   cd $OSPL_HOME/examples/dcps/Throughput/isocpp 
   make 
   cd $OSPL_HOME/examples/dcps/Throughput/isocpp
   # If using shared memory do "ospl start"
   ./publisher
   # If using shared memory do "ospl stop"
   
   # In another In an OpenSplice Enterprise environment: 
   cd $OSPL_HOME/examples/dcps/Throughput/isocpp
   # If using shared memory do "ospl start"
   ./subscriber
   # If using shared memory do "ospl stop" 



|windows|

.. code-block:: bat

   # Load the OpenSplice DDS examples project solution 
   # into Visual Studio and build the required projects 
   
   # In an OpenSplice Enterprise environment:
   cd %OSPL_HOME%\examples\dcps\Throughput\isocpp
   # If using shared memory do "ospl start"
   publisher.exe
   # If using shared memory do "ospl stop" 

   # In another OpenSplice Enterprise environment:
   cd %OSPL_HOME%\examples\dcps\Throughput\isocpp
   # If using shared memory do "ospl start"
   subscriber.exe 
   # If using shared memory do "ospl stop" 



The subscriber application will report the DDS data throughput by 
default once per second. This and many other aspects of the test can 
be configured on the command line. The test utilizes the 
```ReliabilityQoS`` set to ``RELIABLE`` by default in order to show the 
maximal performance whilst maintaining the guaranteed delivery of DDS 
samples. See the ``README`` file for the test for further details. 

**The maximum throughput may be achieved by tuning the test 
parameters appropriately.** 

As mentioned above, the performance testing script described in the 
``html`` for the example is a convenient way to test and record the 
running of this test. 

|caution|
  Note that the default ``OSPL_URI`` value refers to a *Single Process* 
  deployment with *DDSI* networking. 

+ To observe the best performance within a node it is suggested that 
  you use a *Shared Memory* configuration. 

+ To observe the best performance between nodes it is suggested that 
  you use an *RTNetworking* service configuration. 


Achieving Maximum Throughput
============================

Where there is a requirement to support continuous flows or 
'streams' of data with minimal overhead consider the use 
of OpenSplice Streams. The ability to deliver potentially millions of 
samples per second is realized by the Streams feature transparently 
batching (packing and queuing) the periodic samples. 


.. _`Streams Architecture`:

.. figure:: /images/StreamsArchitecture.png
   :height: 55mm
   :alt: Streams Architecture

   **Streams Architecture**


The streams performance example is located in the ``examples/streams`` 
directory within the installation. 


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
