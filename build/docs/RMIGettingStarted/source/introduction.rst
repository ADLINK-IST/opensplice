.. _`Introduction`:

############
Introduction
############

********
Features
********

|product_name| RMI provides an implementation of the general concept of
invoking a remote method over DDS. It enhances |product_name| with a
service-oriented interaction pattern that can be used with combination
with the native data-centric pattern. |product_name| RMI is a service
invocation framework on top of DDS DCPS that uses DDS mechanisms to
export, find and invoke services. It maps all the application-exchanged
requests/replies into DDS data exchanges, and gives the ability to
configure the associated QoS policies according to the application
needs. Finally, |product_name| RMI enables the definition of a distributed
services space over a DDS data space with all the known DDS benefits,
such as discovery, fault tolerance, performance and real-time features.


.. _`RMI Communication Scheme`:

.. figure:: /images/RMI-diag-01.png
   :height: 85mm
   :alt: RMI Communication Scheme

**RMI Communication Scheme**


|product_name| RMI targets service-oriented applications needing a
request/reply communication scheme while they need to have a very
fine control over the data and the underlying network quality of
service. Typically, |product_name| RMI can be used in systems to issue
commands. Commands are a kind of stimulus that express the ability of
the system to do something. As commands have the \`do-something'
connotation, it is often useful to be informed synchronously that the
command has been executed. Thanks to the various DDS QoSs, applications
can associate expiration time, prioritities, persistency and so on to
those commands.

********
Benefits
********

As a complementary paradigm to data centricity, |product_name| RMI provides
these benefits:

+ A more productive and higher abstraction level than can be achieved
  manually through topic exchanges and applications synchronization.

+ A unique middleware technology for mixing Global Services and Data
  Spaces with an easy and dynamic services registration, data declaration,
  and the same discovery mechanisms.

+ Enables data-centric applications to use RMI without the burden of an
  additional middleware technology (*e.g.* CORBA).

+ Strong services location transparency. Thanks to the connectionless
  nature of DDS, service identities do not need to include any
  network-related information. In |product_name| RMI, a service is identified
  by a simple name. Services' identities are exported naturally *via* a
  DDS publication on specific topics. Services can even move from one
  location to another without any impact on client applications.

+ Simple API.

+ Easy deployment process.






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

         
