.. _`Installation and Configuration`:

##############################
Installation and Configuration
##############################

*Follow the instructions in this chapter to install and configure
Vortex OpenSplice and its tools. Information on running the OpenSplice
examples are provided at the end of the chapter under* `Examples`_.


******************************************
Vortex OpenSplice Development and Run-Time
******************************************

Vortex OpenSplice is provided in two installers. The **HDE** (Host
Development Environment) is the standard and it requires approximately
60 Mb of disk space after installation; the **RTS** (Run Time System)
requires approximately 35 Mb of disk space.

The HDE contains all of the services, libraries, header files and tools
needed to develop applications using OpenSplice, and the RTS is a subset
of the HDE which contains all of the services, libraries and tools
needed to deploy applications using OpenSplice.

.. _`Installation for UNIX and Windows Platforms`:

*******************************************
Installation for UNIX and Windows Platforms
*******************************************

This section describes the normal procedure to install Vortex OpenSplice
on a UNIX or Windows platform. The exception is the procedure to install
Vortex OpenSplice on a UNIX ARM platform which is described in section :ref:`UNIX ARM platform` 

**Step 1**

  *Install Vortex OpenSplice.*

  Run the installation wizard for your
  particular installation, using:

  ``P<code>-VortexOpenSplice<version>-<E>-<platform>.<os>-<comp>-<type>-<target>-installer.<ext>``

  where, some being optional,

  *<code>* - PrismTech's code for the platform
  *<version>* - the Vortex OpenSplice version number,
  for example ``V6.0``

  *<E>* - the environment, either ``HDE`` or ``RTS``

  *<platform>* - the platform architecture,
  for example ``sparc`` or ``x86``

  *<os>* - the operating system
  for example ``solaris8`` or ``linux2.6``

  *<comp>* - the compiler or glibc version

  *<type>* - release, debug or dev, which is release with symbols

  *<target>* - the target architecture for host/target builds.

  *<ext>* - the platform executable extension,
  either ``run`` or ``exe``

  The directories in the Vortex OpenSplice distribution are named after the
  installation package they contain. Each package consists of an archive
  and its installation procedure.

**Step 2**

|unix|

  *Configure the Vortex OpenSplice environment variables.*

  (This is only necessary on UNIX, as the Windows environment
  is configured by the OpenSplice installer.)

  Go to the *<install_dir>/<E>/<platform>* directory, where *<E>* is ``HDE``
  or ``RTS`` and *<platform>* is, for example, ``x86.linux2.6``.

  Source the ``release.com`` file from the shell command line.

  This step performs all the required environment configuration.


**Step 3**

  Usually not required, but install your desired ORB when the C++ language mapping is used with
  CORBA cohabitation. Ensure your chosen ORB and compiler is appropriate
  for the CCPP library being used (either OpenSplice's default library or
  other custom-built library). Refer to the *Release Notes* for ORB and
  compiler information pertaining to Vortex OpenSplice' default CCPP library.


*******************************
Installation on Other Platforms
*******************************

Please refer to :ref:`Platform-specific Information`
for information about using Vortex OpenSplice on platforms
other than Unix/Linux and Windows.

.. _`Configuration`:

*************
Configuration
*************

Vortex OpenSplice is configured using an XML configuration file, as
shown under `Example XML Configuration Settings`_ .
It is advisable to use the ``osplconf`` tool (UNIX) 
or *OpenSplice DDS Configurator Tool* (Windows *Start Menu* ) to
edit your xml files. The configurator tool provides
explanations of each attribute and also validates the input.

The default configuration file is ``ospl.xml`` located in
``$OSPL_HOME/etc/config`` (alternative configuration files may also be
available in this directory, to assist in other scenarios). The default
value of the environment variable ``OSPL_URI`` is set to this
configuration file.

The configuration file defines and configures the following OpenSplice
services:

*spliced*
  The default service, also called the domain service; the
  domain service is responsible for starting and monitoring all other
  services.

*durability*
  This service is responsible for storing non-volatile data and
  keeping it consistent within the domain (optional).

*networking*
  This service realizes user-configured communication between the nodes
  in a domain.

*tuner*
  This service provides a SOAP interface for the OpenSplice Tuner to connect
  to the node remotely from any other reachable node.

The default deployment specified by the XML configuration file is for a
*Single Process* deployment. This means that the OpenSplice Domain
Service, database administration and associated services are all started
within the DDS application process. This is implicitly done when the
user's application invokes the DDS *create\_participant* operation.

The deployment mode and other configurable properties can be changed by
using a different ``OSPL_URI`` file. Several sample configuration files
are provided at the same location.

|info| 
  If using a shared memory configuration, a *<Database>* attribute
  is specified in the XML configuration.
  The default Database Size that is mapped on
  a shared memory segment is 10 Megabytes.


|caution|
  *Note:* The maximum user-creatable shared-memory segment is limited
  on certain machines, including Solaris, so it must either be 
  adjusted or OpenSplice must be started as root.

A complete configuration file that enables durability as well as
Real Time Native Networking is shown below. (The relevant lines are not enabled in
the default configuration file, but editing them will allow you to
enable support for *PERSISTENT* data (instead of just *TRANSIENT* or
*VOLATILE* data) and to use multicast instead of broadcast.)

Adding support for *PERSISTENT* data requires you to add the
*<Persistent>* element to the *<DurabilityService>* content (see the
relevant lines in the XML example shown below). In this *<Persistent>*
element you can then specify the actual path to the directory for
persistent-data storage (if it does not exist, the directory will be
created). In the example below this directory is ``/tmp/Pdata``.

For the networking service, the network interface-address that is to be
used is specified by the *<NetworkInterfaceAddress>* element. The
default value is set to ``first available`` , meaning that OpenSplice will
determine the first available interface that is broadcast or multicast
enabled. However, an alternative address may be specified as well
(specify as ``a.b.c.d``).

The network service may use separate channels, each with their own name
and their own parameters (for example the port-number, the queue size,
and, if multicast enabled, the multicast address). Channels are either
reliable (all data flowing through them is delivered reliably on the
network level, regardless of QoS settings of the corresponding writers)
or not reliable (all data flowing through them is delivered at most
once, regardless of QoS settings of the corresponding writers). The idea
its that the network service chooses the most appropriate channel for
each DataWriter, *i.e.* the channel that fits its QoS settings the best.

Usually, networking shall be configured to support at least one reliable
and one non-reliable channel. Otherwise, the service might not be
capable of offering the requested reliability. If the service is not
capable of selecting a correct channel, the message is sent through the
default channel. The example configuration defines both a reliable and
a non-reliable channel.

The current configuration uses broadcast as the networking distribution
mechanism. This is achieved by setting the *Address* attribute in the
*GlobalPartition* element to broadcast, which happens to be the default
value anyway. This *Address* attribute can be set to any multicast
address in the notation ``a.b.c.d`` in order to use multicast.

|caution|
  If multicast is required to be used instead of broadcast, then the
  operating system's multicast routing capabilities must be configured
  correctly.

See the *Vortex OpenSplice Deployment Manual* for more advanced configuration
settings.

.. _`Example XML Configuration Settings`:

**Example XML Configuration Settings**

.. code-block:: xml

  <OpenSpliceDDS>
      <Domain>
          <Name>OpenSpliceDDSV6.6</Name>
          <Database>
              <Size>10485670</Size>
          </Database>
          <Lease>
              <ExpiryTime update_factor=”0.05”>60.0</ExpiryTime>
          </Lease>
          <Service name="networking">
              <Command>networking</Command>
          </Service>
          <Service name="durability">
              <Command>durability</Command>
          </Service>
      </Domain>
      <NetworkService name="networking">
          <General>
              <NetworkInterfaceAddress>
                  first available
              </NetworkInterfaceAddress>
          </General>
          <Partitioning>
            <GlobalPartition Address="broadcast"/>
          </Partitioning>
          <Channels>
             <Channel name="BestEffort" reliable="false"
                default="true">
                  <PortNr>3340</PortNr>
              </Channel>
              <Channel name="Reliable" reliable="true">
                  <PortNr>3350</PortNr>
              </Channel>
          </Channels>
      </NetworkService>
      <DurabilityService name="durability">
          <Network>
              <InitialDiscoveryPeriod>2.0</InitialDiscoveryPeriod>
              <Alignment>
                  <RequestCombinePeriod>
                      <Initial>2.5</Initial>
                      <Operational>0.1</Operational>
                  </RequestCombinePeriod>
              </Alignment>
              <WaitForAttachment maxWaitCount="10">
                  <ServiceName>networking</ServiceName>
              </WaitForAttachment>
          </Network>
          <NameSpaces>
              <NameSpace durabilityKind="Durable"
                  alignmentKind="Initial_and_Aligner">
                  <Partition>*</Partition>
              </NameSpace>
          </NameSpaces>
          <Persistent>
              <StoreDirectory>/tmp/Pdata</StoreDirectory>
          </Persistent>
      </DurabilityService>
  </OpenSplice>




Example configuration files
===========================

Vortex OpenSplice is delivered with a set of ready-made configuration files
which provide a quick way of achieving different setups.

*ospl\_shmem\_ddsi.xml*
  Federated deployment using shared-memory and
  standard DDSI networking.

*ospl\_shmem\_ddsi2e.xml*
  Federated deployment using shared-memory
  and extended DDSI networking.

*ospl\_shmem\_nativeRT.xml*
  Federated deployment using shared-memory
  and RTNetworking.

*ospl\_shmem\_no\_network.xml*
  Federated deployment using
  shared-memory only (*i.e.* without networking).

*ospl\_shmem\_secure\_nativeRT.xml*
  Federated deployment using
  shared-memory and secure RTNetworking.

*ospl\_sp\_ddsi.xml*
  Stand-alone 'single-process' deployment and
  standard DDSI networking.

*ospl\_sp\_ddsi\_nativeRT\_cohabitation.xml*
  Stand-alone 'single-process' deployment using DDSI
  and RTNetworking in cohabitation mode.

*ospl\_sp\_nativeRT.xml*
  Stand-alone 'single-process' deployment
  using RTNetworking.

*ospl\_sp\_ddsi\_statistics.xml*
  Stand-alone 'single-process'
  deployment with DDSI networking and enabled statistics.

*ospl\_sp\_no\_network.xml*
  Stand-alone 'single-process' deployment
  without networking connectivity.


.. _`Examples`:

********
Examples
********

A great way to get started with Vortex OpenSplice is to try running the
examples provided with the product. There are many examples in different
languages and some with the CORBA cohabitation, showing different
aspects of the DCPS API. To give you a feel for how powerful DDS is then
we recommend trying *PingPong* and the *Tutorial*, as well as the RoundTrip
and Throughput.

The way to build and run the examples is dependent on the Platform you
are using. Each example has HTML documentation explaining how to build
and run it on Unix/Linux and Windows systems.

For VxWorks and Integrity, please refer to :ref:`VxWorks 5.5.1`,
:ref:`VxWorks 6.x RTP`, and :ref:`Integrity` in this *Guide*.

Using the OpenSplice Tools
==========================

|caution|
  *Note:* The following instructions apply only to the *shared memory*
  deployment of Vortex OpenSplice. When deploying in single process
  configuration, there is no need to manually start the OpenSplice
  infrastructure prior to running a DDS application process, as the
  administration will be created within the application process. Please
  refer to the Vortex OpenSplice *Deployment Guide* for a discussion of these
  deployment architectures.


The Vortex OpenSplice infrastructure can be stopped and started from the
Windows *Start Menu* , as well as the Tuner and Configurator.

**Step 1**

  *Manually start the OpenSplice infrastructure*

  1. Enter ``ospl start`` on the command line [#f1]_.
     This starts the OpenSplice services.

  These log files may be created in the current directory when OpenSplice
  is started:

    *ospl-info.log* - contains information and warning reports

    *ospl-error.log* - contains error reports

|info|
  If Vortex OpenSplice is used as a Windows Service then the log files are
  re-directed to the path specified by ``OSPL_LOGPATH``. (Use the *set*
  command in the Vortex OpenSplice command prompt to see the ``OSPL_LOGPATH``
  value.)


**Step 2**

  *Start the OpenSplice Tuner Tool*

  1. Read the *OpenSplice Tuner Guide* (``TunerGuide.pdf``)
     before running the Tuner Tool.

  2. Start the tool by entering ``ospltun`` on the command line.

|caution|
  The URI required to connect is set in the ``OSPL_URI``
  environment variable (the default
  URI is: ``file://$OSPL_HOME/etc/config/ospl.xml``).

  The OpenSplice system can now be monitored.

**Step 3**

  *Experiment with the OpenSplice tools and applications*

  Use the OpenSplice Tuner to monitor all DDS entities and
  their (dynamic) relationships.

**Step 4**

  *Manually stop the OpenSplice infrastructure*

  1. Choose *File > Disconnect* from the OpenSplice Tuner menu.

  2. Enter ``ospl stop`` on the command line; this stops all
     OpenSplice services.

  

 
.. rubric:: Footnotes
 
.. [#f1] ``ospl`` is the command executable for OpenSplice DDS.




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

   
