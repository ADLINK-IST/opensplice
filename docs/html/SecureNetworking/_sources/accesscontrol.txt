.. _`Access Control`:


##############
Access Control
##############

*The previous sections described how transport security is 
achieved with the OpenSplice Secure Networking Module. This 
section explains how access control is realized in OpenSplice 
and gives a short introduction to the underlying concepts.*

Overview
********

The OpenSplice Access Control Module is an optional, pluggable 
service to the OpenSplice Secure Networking Module. It 
complements the Secure Networking Module by offering different 
types of access control modules that may be flexibly plugged in 
(for example, mandatory access control, role-based access 
control, *etc.*). 

|info|
OpenSplice currently supports Mandatory Access Control (MAC) 
based on the Bell-LaPadula/Biba model. Other 
Access Control Modules may be available in a future release.

Access Control can be enabled in the OpenSplice secure 
networking configuration (see 
:ref:`Secure Networking Configuration <Secure Networking Configuration>`
). An external file containing the policy rules in 
XML format can be referenced.

The following section, Mandatory Access Control, introduces 
mandatory access control. `MAC in OpenSplice`_ 
explains how Mandatory access control is realized in OpenSplice.

Mandatory Access Control
************************

An access control policy defines whether a user (referred to as 
a *subject*) is allowed to access an object. 

The access control policy is centrally controlled when Mandatory 
Access Control is in force. Users (subjects) may not override 
the policy. For example when Mandatory Access Control is in 
force users may not grant access to objects that would otherwise 
be restricted. 

Mandatory Access Control in OpenSplice ensures data 
confidentiality as well as data integrity: data may only be 
created and accessed by authorized parties. The control of 
information flow is based on the Bell-LaPadula model. This 
ensures that the information received via the network can only 
be retrieved on nodes that:

+ are accredited for the secrecy level of this information, and

+ contain applications that have a need-to-know for the information.

Additionally, care is taken to preserve data integrity (based on 
the Biba model) by:

+ preventing data modification by unauthorized parties

+ preventing unauthorized data modification by authorized parties

+ maintaining internal and external consistency 

The following sections give a short introduction to the 
Bell-LaPadula and Biba models. `MAC in OpenSplice`_ 
explains how mandatory access control is realized in 
OpenSplice.

Bell-LaPadula Confidentiality Model
===================================

The Bell-LaPadula Confidentiality model describes a set of 
access control rules which use clearances for subjects (the 
users) and classifications on objects (resources). The clearance 
for a user and a classification of an object define a secrecy 
level and a set of compartments. Secrecy levels are hierarchical 
and range from the least sensitive, for example “Unclassified” 
or “Public”, to the most sensitive, for example “Top Secret”.

Compartments are a non-hierarchical list of ID’s (strings) 
structuring information to enforce the ‘need-to-know’ principle.

The clearance of the subject is compared to the classification 
of the object in order to determine if a subject is authorized 
to access a certain object. The Bell-LaPadula model defines two 
mandatory access control rules:

+ a subject at a given secrecy level may not read an object at 
  a higher secrecy level (no *read-up*).

+ a subject at a given secrecy level must not write to any 
  object at a lower secrecy level (no *write-down*).

In the Bell-LaPadula model, users can create content only at or 
above their own secrecy level. For example, secret researchers 
can create “Secret” or “Top-Secret” files, but may not create 
“Public” files (no write-down). On the other hand, users can 
view content only at or below their own secrecy level. For 
example, secret researchers can view “Public” or “Secret” files, 
but may not view “Top-Secret” files (no read-up).

Additionally, a subject must have a *‘need-to-know’* for the 
object it wants to access; in other words, the set of 
compartments contained in the user’s clearance must be a subset 
of the set of compartments contained in the object’s 
classification.

Biba Integrity Model
====================

The Biba Integrity Model describes a set of access control rules 
designed to ensure data integrity. Resources (objects) and users 
(subjects) are grouped into ordered levels of integrity. 

The Biba model defines two mandatory access control rules:

+ a subject at a given level of integrity may not read an 
  object at a lower integrity level (no read down).

+ a subject at a given level of integrity must not write to any 
  object at a higher level of integrity (no write up).

In the Biba model, users can only create content at or below 
their own integrity level (no write up). On the other hand, 
users can only view content at or above their own integrity 
level (no read down).

Access Control in OpenSplice
****************************

In OpenSplice, access control is enforced on the network-level 
for inter-node communication, in other words, the communication 
between different nodes. One node (host) corresponds to one 
user. When running multiple DDS applications on one host these 
will all run under the same User ID.

There are two access control enforcement points:

+ for inbound traffic: when reading data from the network, the 
  following checks are carried out:
 
  - is the reader allowed to receive the data, 
 
  - was the data published by a trusted node (in other words, was 
    the sender allowed to send the data)

+ for outbound traffic: when writing data to the network, the 
  following check is carried out:
 
  - is the user allowed to write data to the network

|info|
Note that access control is not enforced for intra-node 
communication, in other words communication between DDS 
applications running on the same node.

Objects to be Protected
=======================

One access control policy applies to one DDS Domain. The objects 
to be protected by the access control policy in OpenSplice apply 
to DDS topics and DDS partitions. 

For example, a classification can be attached to a topic. This 
may result in restricting access in a way that only users with a 
matching clearance may use this topic for publication of data or 
for subscription to data of this topic, respectively. The same 
applies to partitions which are referenced by a resource element 
of the access control policy. 

Secure *vs.* Insecure
=====================

The *‘secure’* world is strictly separated from the *‘insecure’* 
world in OpenSplice. If a resource does not have access control 
information (for example a classification), then no access 
control will be enforced for this resource. If a user has no 
access control information (for example a clearance) they may 
only access ‘insecure’ resources (resources that do not have 
access control information like a classification). If a user has 
access control information, then they may not access ‘insecure’ 
resources that don’t have access control information.

User Authentication
===================

The system authenticates the user’s identity. 

|info|
OpenSplice currently supports SSL X.509 Certificate 
Authentication. Other authentication mechanisms (such as user ID 
- password authentication) may be available in a future release.

X.509 is a standard for digital certificates used by SSL. 
Certificates include, among other things, information about the 
identity of the certificate holder (in other words: the user). 
The user authenticates to the system presenting a certificate. 
The system accepts the user’s certificate if it was issued by a 
trusted authority. A list of trusted certificates can be defined 
in the secure networking configuration (see  
:ref:`Element X509Authentication <Element X509Authentication>` and
:ref:`Element TrustedCertificates <Element TrustedCertificates>`.

The user’s identity is determined by a distinguished name (DN) 
contained in the user’s certificate. The system retrieves the 
corresponding user from the access control policy (each user 
entry in the policy defines the subject’s DN) and applies the 
defined access control information to this user.

MAC in OpenSplice
*****************

Mandatory Access Control (MAC) in Open Splice combines the 
Bell-LaPadula and Biba models to ensure confidentiality and data 
integrity. Each resource (object) has a classification which 
comprises a secrecy level, an integrity level and a set of 
compartments that this resource is intended for. Each user 
(subject) has a clearance which comprises a secrecy level, an 
integrity level and a set of compartments this user has a 
‘need-to-know’ for. 

In order to determine if a user is authorized to access a 
certain resource, in other words, if they can publish a certain 
topic or subscribe to a topic, the clearance of the user is 
compared to the classification of the resource. This process 
comprises the evaluation of the:

+ secrecy level
 
  - subscribing is permitted if the resource’s secrecy level is 
    identical or lower than the user’s secrecy level
 
  - publishing is permitted if the resource’s secrecy level is 
    identical or higher than the user’s secrecy level

+ integrity level
 
  - subscribing is permitted if the resource’s integrity level is 
    identical or higher
 
  - publishing is permitted if the resource’s integrity level is 
    identical or lower

+ need to know (compartment)
 
  - publish/subscribe is permitted if the user’s set of 
    compartments is a subset of the resource’s set of compartments

Access is only granted if *all three parts* of the user’s 
clearance (secrecy level, integrity level, and need-to-know) are 
evaluated positively against the resource’s classification.

The following section, 
:ref:`Access Control Configuration <Access Control Configuration>`
describes access control policy configuration elements for MAC 
in OpenSplice.

.. EoF


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
