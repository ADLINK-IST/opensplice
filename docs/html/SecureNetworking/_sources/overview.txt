.. _`Overview`:


########
Overview
########

*This section gives an overview of the features provided by the 
Vortex OpenSplice Secure Networking Service.*

Vortex OpenSplice Secure Networking Service
*******************************************

Vortex OpenSplice is a suite of software products for the creation 
of a secure, performant, and predictable information backbone 
for distributed systems and systems-of-systems, comprised of a 
high-performance, low-overhead run-time environment, development 
tools for modelling information and applications, and run-time 
tools for monitoring system performance.

The objective of the OpenSplice suite is to reduce 
complexity, shorten time-to-market, raise quality, and ensure 
standards compliance and code correctness; all in a single 
integrated suite of tools and runtime components from a proven 
and trusted vendor. Fast and predictable networking between the 
nodes of the information backbone is an essential part of the 
OpenSplice solution, as is the importance of addressing 
Information Assurance.

The OpenSplice Secure Networking Service is an optional 
pluggable service to the OpenSplice Core infrastructure. The 
Secure Networking Service complements the advanced real-time 
networking features of the OpenSplice Core module by 
offering a dedicated crypto-channel *per* network partition.

The OpenSplice Networking service provides essential 
features for achieving both scalability as well as real-time 
determinism in mission-critical systems where important 
high-priority data must ‘pre-empt’ less-important data. The 
Networking service’s architecture supports multiple 
runtime-configured *network channels* each representing a 
pre-emptive priority band which allows for traffic-shaping and 
data-urgency (``Latency-Budget`` QoS) driven network-packing. The 
proper network channel is automatically chosen based upon the 
actual importance (``Transport_Priority`` QoS) of published data. 
The channel-specific traffic-shaping and bandwidth limitation 
effectively prevent faulty and/or misbehaving applications from 
monopolizing the network resources.

The Secure Networking Service extends this real-time ‘network 
scheduler’ with configurable cryptographic protection 
implementing transport security with the following properties:

+ Information exchanged between nodes of the OpenSplice-based 
  information backbone over unsecure networks cannot be 
  eavesdropped or modified without detection while in transit 
  (data integrity and confidentiality).

+ Complete, reliable, and readily-evaluatable separation 
  between the area in which the information is processed in 
  unencrypted form (*RED*, on the node) and the area to which 
  critical (classified) information is not permitted to flow in 
  unencrypted form (*BLACK*, network), is achieved by means of 
  concentration and restriction of network connectivity to exactly 
  one client on each node.

+ Authenticity of all information exchanged between nodes.

In addition to transport security, the Secure Networking Service 
offers pluggable access control. Different modules may be used 
to enforce access control (for example modules implementing 
mandatory access control or role-based access control, *etc.*). 
Currently, OpenSplice supports Mandatory Access Control (MAC). 
Other modules will be available in a future release. The 
Mandatory Access Control Module is characterized by the 
following features:

+ Information received *via* the network can only be retrieved 
  on nodes that
 
  - are accredited for the security level of this information, and
 
  - host applications that have a need-to-know for the 
    information.

+ Information of different classifications can be 
  cryptographically separated while in transit between different 
  nodes, resulting in stronger separation than only labelling, and 
  performing no infiltration or exfiltration between different 
  classifications while in transit.

This infrastructure security solution ensures Information 
Assurance (IA) for all DDS-based co-operation and information 
exchange between the DDS nodes over untrusted communication 
infrastructures. The Secure Networking Service allows the 
reliable separation of applications with different clearances 
deployed on different nodes in a way that ensures transparency 
to the applications, thus supporting full portability.

The OpenSplice Secure Networking Service is the first 
building block for a complete QoS-enabled IA solution offering 
end-to-end security between all applications (distributed or 
co-located), including mandatory access control for all data 
flowing between applications and detailed security audit of 
application interactions.


Getting Started
***************

The Vortex OpenSplice Secure Networking Service is provided along 
with the OpenSplice commercial edition RTS and HDE product 
installers. 

|info|
To activate the Secure Networking Service an OpenSplice licence 
including the *OPENSPLICE_SECURE_NETWORKING* 
feature is required. Please be sure to read the Release Notes 
for the latest information.

To install OpenSplice, please follow the instructions and 
configuration example given in the *Vortex OpenSplice Getting 
Started Guide*. For an in-depth description of the available 
configuration settings you may also want to read the 
*Vortex OpenSplice Deployment Guide* and the
*Vortex OpenSplice Configuration Guide*.

If you wish to try out the 
:ref:`Secure Networking Tutorial <Secure Networking Tutorial>`
you will need to set up at least two networked hosts with 
OpenSplice. You will also need to build and run the 
Chat Tutorial application which is described in the 
*DCPS C Tutorial Guide*.



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

         