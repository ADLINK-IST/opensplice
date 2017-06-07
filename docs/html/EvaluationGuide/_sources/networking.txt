.. _`OpenSplice Networking Options`:

#############################
OpenSplice Networking Options
#############################

OpenSplice Enterprise provides several networking options for the 
delivery of DDS data between nodes. The networking service selection 
is largely transparent to the user; the difference is observed 
in the CPU consumption, networking load, and ultimately how fast and 
efficiently the data is delivered between nodes. The most applicable 
service is dependent on the requirements of the use case. 

**OpenSplice DDSI** is the industry standard protocol providing vendor 
interoperability that operates using a typed 'pull' style model. 

**OpenSplice RTNetworking** is an alternative to the DDSI wire protocol. 
RTNetworking uses a type-less 'push' style model in 
contrast to DDSI and is often the more performant, scalable option. 
RTNetworking also offers prioritization of network traffic via 
&lsquo;channels&rsquo;, partitioning to separate data flows and 
optional compression for low-bandwidth environments. 
**OpenSplice SecureRTNetworking** provides these features together 
with encryption and access control. 

**OpenSplice DDSI2E** is the 'enhanced' version of the 
interoperable service. DDSI2E offers the benefits of the DDSI 
protocol (such as its automatic unicast delivery in the case of there 
being a single subscribing endpoint), together with some of the 
performance features of the RTNetworking service such as channels, 
partitioning and encryption. 

*************************************
How to select the Networking Protocol
*************************************

As with the architectural deployment choice, the selection of the 
networking service is described by the XML configuration file. Note 
that this choice is independent of and orthogonal to the 
architectural deployment: you can have single process or shared 
memory with any of the networking service protocols. 

+ To run with a **DDSI** service, set the ``OSPL_URIvariable`` to refer 
  to a DDSI xml file such as ``ospl_sp_ddsi.xml`` or ``ospl_shmem_ddsi.xml``. 


+ To run with an **RTNetworking** service, set the ``OSPL_URI`` variable to 
  refer to an RTNetworking xml file such as ``ospl_sp_nativeRT.xml`` or 
  ``ospl_shmem_nativeRT.xml``. 


+ To run with a **SecureRTNetworking** service, set the ``OSPL_URIvariable`` to 
  refer to the ``ospl_shem_secure_nativeRT.xml`` SecureRTNetworking xml 
  file. 


+ To run with a  **DDSI2E** deployment, set the ``OSPL_URI`` variable to refer 
  to a DDSI2E xml file such as ``ospl_sp_ddsi2e.xml`` or 
  ``ospl_shmem_ddsi2e.xml``. 


|caution|
  Note that by default, the ``OSPL_URI`` environment variable refers to a 
  *DDSI* configuration, so to see the extra performance and scalability 
  benefits of OpenSplice DDS's RTNetworking or DDSI2E it is 
  necessary to switch from the default. 


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


