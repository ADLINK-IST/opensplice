.. _`Introduction`:


############
Introduction
############

*This section describes the Vortex OpenSplice Tuner.*

General Description
*******************

Vortex OpenSplice Tuner has been implemented in the Java language. It is
possible to use it on every platform where a Java Virtual Machine
(JVM) implementation is available. That means that Vortex OpenSplice Tuner
does not require OpenSplice to be available on the local system.

Vortex OpenSplice Tuner is able to connect to one specific OpenSplice
DDS domain at one specific node, both locally and remotely.  It is possible
to simultaneously connect one or more Vortex OpenSplice Tuners to a specific
domainand node.

The diagram below shows an overview of a Vortex OpenSplice Tuner process
connecting locally to a Vortex OpenSplicefederation (shared memory
deployment) on one computing node.
(Typically, there are many nodes within a system.)

 _`Vortex OpenSplice Tuner Local Connection`:

.. centered:: **Vortex OpenSplice Tuner Local Connection**

.. image:: /images/001a_LocalConnection.png
   :width: 175mm
   :align: center
   :alt: Vortex OpenSplice Tuner Local Connection


.. XXX Figure 1A  Vortex OpenSplice Tuner Local Connection


    \newpage

The diagram below shows an overview of a Vortex OpenSplice Tuner
process connecting remotely to a Vortex OpenSplice application
(single process deployment). The Tuner service (aka the SOAP service)
must be enabled in the Vortex OpenSplice configuration file to allow
remote connection for tools.

_`Vortex OpenSplice Tuner Remote Connection`:

.. centered:: **Vortex OpenSplice Tuner Remote Connection**

.. image:: /images/001b_RemoteConnection.png
   :width: 175mm
   :align: center
   :alt: Vortex OpenSplice Tuner Remote Connection

.. XXX Figure 1B  Vortex OpenSplice Tuner Remote Connection

While the figure details a remote connection to a single process
deployment, one can also use the remote connection capability to
connect to a shared memory deployment in the exact same way.

.. raw:: latex


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

         
