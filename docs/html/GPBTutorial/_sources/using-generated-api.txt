.. _`Using the generated API in applications`:

#######################################
Using the generated API in applications
#######################################


The DDS API implementation will allow the use of GPB types for DDS
transparently, and the generated underlying DDS type will be invisible
to the application.

Protobuf data model
===================

For the comming example the following proto file is used:

.. literalinclude:: ../../../../examples/protobuf/proto/address.proto
   :language: protobuf


Java
====
|java|

In this example we will publish a person *Jane Doe* with one friend,
*John Doe*.

The Subscriber example will read this data and print it to the ``stdout``.
This example is delivered with OpenSplice, and is located in
``examples/protobuf/java5``.

Publisher
---------
|java|

Example Publisher for the generated Person data:

.. literalinclude:: ../../../../examples/protobuf/java5/src/ProtobufPublisher.java
   :language: java


Subscriber
----------
|java|

Example Subscriber for the generated Person data:

.. literalinclude:: ../../../../examples/protobuf/java5/src/ProtobufSubscriber.java
   :language: java


ISO-C++
=======
|cpp|

In this example the publisher and subscriber are embedded into one file.

The publisher part will publish a person *Jane Doe* with one friend,
*John Doe*.

The Subscriber part in this example will read this data and print it to the ``stdout``.

This example is delivered with Vortex OpenSplice, and is located in
``examples/protobuf/isocpp2``.

.. literalinclude:: ../../../../examples/protobuf/isocpp2/src/implementation.cpp
   :language: cpp

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
