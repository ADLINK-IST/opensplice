.. _`|product_name| RMI over DDS`:

###########################
|product_name| RMI over DDS
###########################


As in traditional service-oriented applications, communication from
client to server is performed through a well-defined service model. The
RMI module enables a user to build a service model with remote method
invocation capabilities and completely hides the DDS DCPS API. Of
course, using RMI does not prevent the application from using the DDS
API as shown by the following figure:


.. _`RMI Relationship to DDS`:

.. figure:: /images/RMI-diag-02.png
   :height: 50mm
   :alt: RMI Relationship to DDS

**RMI Relationship to DDS**


A service model is defined by one or more object-oriented interfaces. A
DDS RMI interface is an IDL interface having a name and a set of
operations. Each operation has a fixed set of typed parameters. The RMI
module provides:

+ A service invocation framework that maps the different services
  operations onto a set of DDS topics that hold the operation's invocation
  requests and replies. A set of mapping rules have been defined for this
  purpose. At runtime, this framework sets up the underlying DDS
  environment and handles the remote interface invocations using the basic
  DDS read/write operations.

+ A simple and intuitive programming model for both the server application
  side implementing the interface, and the client application side
  invoking that interface. The server programming model is as simple as
  implementing an interface, and the client programming model is as simple
  as calling a local interface.

+ A powerful feature to enable tuning of the invocation request and reply
  QoS by setting their corresponding DDS QoS policies. This feature
  enables developers to improve the invocations quality with real-time and
  high-performance features. For instance, priorities and validity
  durations (lifespan) could be set on the different operation
  requests/replies.

+ Synchronous, asynchronous and oneway invocation modes. The synchronous
  mode is the invocation mode that blocks the client thread until the
  reply is sent back to him by the server. The asynchronous mode is
  similar to the CORBA Asynchronous Messaging Interface (AMI) callback
  model. It is a non-blocking mode where the client does not wait for the
  reply from the server, but rather provides a callback object that will
  be invoked by the middleware to deliver the request return values when
  they are received. Finally, the oneway mode is a fire-and-forget
  invocation mode where the client does not care about the success or
  failure of the invocation. A oneway method cannot return values and no
  reply message will ever return to the client once the request is sent to
  the server.

+ |rmi_langs| implementation.

**************
Key components
**************

The |product_name| RMI module includes the following components:

**RMI Pre-processor** (``rmipp``)
  Generates the interface-type-specific
  requests/replies topics and invocation handling classes.

**Core library**
  Provides the runtime setup operations and a generic
  invocation framework.

*****************
Binding Languages
*****************

.. ifconfig:: 'Java' in (rmi_languages)

  The |product_name| RMI module is available for both **Java** and **C++** languages.

.. ifconfig:: 'Java' not in (rmi_languages)

  The |product_name| RMI module is available for the **C++** language.









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

         
