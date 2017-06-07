.. _`Secure Networking Configuration`:


###############################
Secure Networking Configuration
###############################

*This section provides an in-depth description of the OpenSplice 
security configuration by describing the most important 
configuration parameters for the Secure Networking Service.*

Each configuration parameter will be explained by means of an 
extensive description together with the tabular summary that 
contains the following information:

**Full path** - 
  Describes the location of the item within a 
  complete configuration. Because the configuration is in XML 
  format, an XPath expression is used to point out the name and 
  location of the configuration item.

**Format** - 
  Describes the format of the value of the configuration item.

**Dimension** - 
  Describes the unit for the configuration item 
  (for instance *seconds* or *bytes*).

**Default value** - 
  Describes the default value that is used by 
  the service when the configuration item is not 
  set in the configuration.

**Valid values** - 
  Describes the valid values for the configuration item. 
  This can be a range or a set of values.

If the configuration parameter is an XML *attribute*, the 
table also contains the following information:

**Required** - 
  Describes whether the attribute is required or if 
  it is optional.

If the configuration parameter is an XML *element*, the table 
also contains the following information:

**Occurrences** - 
  Describes the range of the possible number of 
  occurrences of the element in the configuration by specifying 
  the minimum and maximum number of occurrences.


Activating Secure Networking
****************************

The OpenSplice Security Service comes as a separate service, 
replacing the regular networking service. 

The secure networking service executable is named ``snetworking`` 
on Linux and Solaris and ``snetworking.exe`` on Windows. 

It needs to be configured in the OpenSplice configuration XML 
file (see below). In addition, a license is required to enable 
the OpenSplice Secure Networking feature; if a license is not 
found then an error message is printed and the service will not start.

Configuring the Secure Networking Service
=========================================

OpenSplice services are configured in an OpenSplice 
configuration file, which is located in 
``<OSPL_HOME>/etc/configs/ospl.xml`` by default.

Within this configuration file the ``<Domain>`` element contains 
a set of Service child elements, which are responsible for 
starting services like durability or networking. The default 
configuration file already contains a ``<Service>`` element starting 
the networking service. To configure secure networking only the 
name of the service’s executable in the ``<Command>`` child element 
needs to be replaced with the value ``snetworking``.

The following snippet shows an example of an OpenSplice 
configuration starting secure networking service:

:: 

   <OpenSplice>
       <Domain>
           <Name>OpenSplice Security</Name>
           <Database>
               <Size>10485670</Size>
           </Database>
           <Lease>
               <ExpiryTime update_factor="0.5">5.0</ExpiryTime>
           </Lease>
           <Service enabled="true" name="networking">
               <Command>snetworking</Command>
           </Service>
       </Domain>
       ...
   </OpenSplice>


To check if the service starts correctly the info logfile 
``ospl-info.log`` can be inspected (see also 
:ref:`How to Diagnose Problems - Logging <How to Diagnose Problems - Logging>`
).


Secure Networking Configuration Elements
****************************************

The secure networking configuration expects a root element named 
``OpenSplice/NetworkingService``. Within this root element, the 
networking daemon will look for several sub-elements. 

The configuration elements for secure networking are listed and 
explained below. 

These elements are an extension of the configuration elements for 
the Networking Service described in the 
*Vortex OpenSplice Deployment Guide* and the
*Vortex OpenSplice Configuration Guide*.


Element GlobalPartition
=======================

This element specifies the global or default networking 
partition.

Attribute SecurityProfile
-------------------------

In the context of secure networking, the ``GlobalPartition`` element 
provides support for the attribute ``SecurityProfile``. The 
attribute is referencing a security profile declared in the 
context of the ``Security`` element.

If the given reference is invalid, the global partition 
configuration is invalid. In this case, the partition will be 
blocked to prevent unwanted information leaks. A configuration 
error message will be logged to the ``ospl-error.log`` file. If the 
security feature has been enabled, but no profile is declared, 
then the ``NULL`` profile is used by default: this means that no 
security is added to the transport 
(see :ref:`Attribute Enabled <Security ENABLED>`).

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | OpenSplice/NetworkService/Partitioning/\               |
|             | GlobalPartition[@SecurityProfile]                      |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any                                                    |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Required    | no                                                     |
+-------------+--------------------------------------------------------+
| Remarks     | This attribute is referencing a security profile       |
|             | declared in the context of the Security element.       |
+-------------+--------------------------------------------------------+


Element NetworkPartition
========================

The Networking configuration can contain a set of networking 
partitions, which are defined in the context of the 
``NetworkPartitions`` element.

Attribute SecurityProfile
-------------------------

In the context of secure networking, the ``NetworkPartition`` 
element provides support for the attribute ``SecurityProfile``. 
The attribute is referencing a security profile declared in the 
context of the ``Security`` element.

If the given reference is invalid, the network partition 
configuration is invalid. In this case the partition will be 
blocked to prevent unwanted information leaks. A configuration 
error message will be logged to the ``ospl-error.log`` file. 
If the security feature has been enabled but no profile is declared, 
the ``NULL`` profile will be used by default.

The ordering of network partition declarations in the OSPL 
configuration file must be the same for all nodes within the 
OpenSplice domain. 

If certain nodes shall not use one of the network partitions, 
the network partition in question must be declared as ``connected="false"``. 
In this case the declared security profile would not be evaluated 
or initialized, and the associated secret cipher keys need not to 
be defined for the OpenSplice node in question.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | OpenSplice/NetworkService/NetworkPartitions/\          |
|             | NetworkPartition[@SecurityProfile]                     |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any                                                    |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Required    | no                                                     |
+-------------+--------------------------------------------------------+
| Remarks     | This attribute is referencing a security profile       |
|             | declared in the context of the Security element.       |
+-------------+--------------------------------------------------------+


Element Security
================

The ``Security`` section defines the parameters relevant for secure 
networking. Declaring this element in the OSPL configuration 
file will activate the secure networking feature. 

Without any additional security settings, all network partitions 
of the node would use the ``NULL`` cipher encoding. If confidentiality 
and integrity is required for a network partition, the network 
partition must be associated with a security profile (see 
`Element SecurityProfile`_).

.. _`Security ENABLED`:

Attribute Enabled
-----------------

This is an optional attribute. 

|caution|
If not defined it defaults to ``true`` and all network partitions, 
if not specified otherwise, will be encoded using the ``NULL`` cipher. 
**The NULL cipher does not provide for any level of integrity 
or confidentiality, and message items will be sent unencrypted.**

If ``enabled="false"`` the security feature will not be 
activated, and the node acts like any other OpenSplice node not 
being security-aware. Security profiles defined in the 
configuration file will not take effect, but will cause the 
system to log warnings. 

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | OpenSplice/NetworkService/Security[@Enabled]           |
+-------------+--------------------------------------------------------+
| Format      | Boolean                                                |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | true                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | true, false                                            |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Required    | no                                                     |
+-------------+--------------------------------------------------------+
| Remarks     | This attribute is a flag to enable or disable          |
|             | the secure networking.                                 |
+-------------+--------------------------------------------------------+

Element SecurityProfile
=======================

This element defines the security profile which can be applied 
to one or more network partitions. This element is optional.

Attribute Name
--------------

This is a mandatory attribute. The name must be unique for all 
Security Profiles being declared. If the name is not specified, 
the security profile will be ignored as it cannot be referenced 
anyway.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | OpenSplice/NetworkService/Security[@Enabled]           |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any, but must be unique amongst all other              |
| Values      | SecurityProfiles                                       |
+-------------+--------------------------------------------------------+
| Required    | yes                                                    |
+-------------+--------------------------------------------------------+
| Remarks     | This is a required attribute. The given name can be    |
|             | referenced by NetworkPartition and GlobalPartition     |
|             | elements.                                              |
+-------------+--------------------------------------------------------+


Attribute Cipher
----------------

This is a mandatory attribute. Depending on the declared cipher, 
the cipher key must have a specific length, 128 bits, 192 bits, 
256 bits or none at all. The following case-insensitive values 
are supported by the current implementation:

+ **aes128**, implements AES cipher with 128 bit cipher-key 
  (16 Bytes, 32 hexadecimal characters). This cipher will 
  occupy 34 bytes of each UDP packet being sent.

+ **aes192**, implements the AES cipher with 192 bit cipher-key 
  (24 Bytes, 48 hexadecimal characters). This cipher will occupy 
  34 bytes of each UDP packet being sent. 

+ **aes256**, implements the AES cipher with 256 bit cipher-key 
  (32 Bytes, 64 hexadecimal characters. This cipher will occupy 
  34 bytes of each UDP packet being sent.

+ **blowfish**, implements the Blowfish cipher with 128 bit 
  cipher-key (16 Bytes, 32 hexadecimal characters). This cipher 
  will occupy 26 bytes of each UDP packet being sent.

+ **null**, implements the ``NULL`` cipher. The only cipher that 
  does not require a cipher-key. This cipher will occupy 4 bytes 
  of each UDP packet being sent.

All ciphers except for the ``NULL`` cipher are combined with SHA1 to 
achieve data integrity. Also, the *rsa-* prefix can be added to 
the ciphers. In this case, digital signatures using RSA will be 
available.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | OpenSplice/NetworkingService/Security/\                |
|             | SecurityProfile[@Cipher]                               |
+-------------+--------------------------------------------------------+
| Format      | enumeration                                            |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | aes128, aes192, aes256, blowfish, rsa-aes128,          |
| Values      | rsa-eas192, rsa-eas256, rsa-blowfish,                  |
|             | rsa-null, NULL                                         |
+-------------+--------------------------------------------------------+
| Required    | yes                                                    |
+-------------+--------------------------------------------------------+
| Remarks     | All but NULL cipher require attribute CipherKey        |
|             | to be set with matching key length.                    |
+-------------+--------------------------------------------------------+


Attribute CipherKey
-------------------

The ``CipherKey`` attribute is used to define the secret key 
required by the declared cipher. The value can be a URI 
referencing an external file containing the secret key, or the 
secret key can be defined in-place directly as a string value. 
The key must be defined as a hexadecimal string, each character 
representing 4 bits of the key; for example, ``1ABC`` represents 
the 16-bit key ``0001 1010 1011 1100``. The key must not follow a 
well-known pattern and must match *exactly* the key length 
required by the chosen cipher. If the cipher-keys are malformed, 
the security profile in question will be marked as invalid. 
Moreover, each network partition referring to the invalid 
Security Profile will not be operational and thus traffic will 
be blocked to prevent information leaks.

|caution|
As all OpenSplice applications require read access to the XML 
configuration file, for security reasons it is recommended to 
store the secret key in an external file in the file system, 
referenced by the URI in the configuration file. The file must 
be protected against read and write access from other users on 
the host. Verify that access rights are not given to any other 
user or group on the host.

|caution|
Alternatively, storing the secret key in-place in the XML 
configuration file will give read/write access to all DDS 
applications joining the same OpenSplice node. Because of this, 
the ‘in-place’ method is strongly discouraged.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | OpenSplice/NetworkingService/Security/\                |
|             | SecurityProfile[@CipherKey]                            |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | Hexadecimal string or file URI matching the            |
| Values      | pattern: ``file://.*``.                                |
+-------------+--------------------------------------------------------+
| Required    | yes                                                    |
+-------------+--------------------------------------------------------+
| Remarks     | All but NULL cipher require attribute CipherKey        |
|             | to be set with matching key length.                    |
+-------------+--------------------------------------------------------+


Element AccessControl
=====================

The optional ``AccessControl`` element defines settings for access 
control enforcement and which access control module shall be 
used. 

Attribute enabled
-----------------

The access control feature will be activated when ``enabled="true"``.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/AccessControl[@enabled]                       |
+-------------+--------------------------------------------------------+
| Format      | Boolean                                                |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | false                                                  |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | false, true                                            |
| Values      |                                                        |
+-------------+--------------------------------------------------------+


Attribute policy
----------------

The ``policy`` attribute references a file containing the access 
control policy. Configuration elements of this file are 
explained in detail in 
:ref:`Access Control Policy Elements <Access Control Policy Elements>`.


.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/AccessControl[@policy]                        |
+-------------+--------------------------------------------------------+
| Format      | file URI, also see  `Attribute CipherKey`_             |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any                                                    |
| Values      |                                                        |
+-------------+--------------------------------------------------------+


Element AccessControlModule
===========================

The ``AccessControlModule`` element defines which access control 
module will be used. More than one module may be defined. All 
defined and enabled modules will be used to determine if access 
should be granted.

Attribute enabled
-----------------

The module specified in the ``type`` attribute is used to evaluate 
access control rules when ``enabled="true"``.  

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/AccessControl/\                               |
|             | AccessControlModule[@enabled]                          |
+-------------+--------------------------------------------------------+
| Format      | Boolean                                                |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | true                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | true, false                                            |
| Values      |                                                        |
+-------------+--------------------------------------------------------+


Attribute type
--------------

The ``type`` attribute defines the access control model type. 

|info|
OpenSplice currently only supports mandatory access control, 
accordingly the only valid value for this attribute is ``"MAC"``. 

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/AccessControl/\                               |
|             | AccessControlModule[@enabled]                          |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | "MAC"                                                  |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | This value is referencing an access control module.    |
+-------------+--------------------------------------------------------+


Element Authentication
======================

The optional ``Authentication`` element defines whether additional 
sender authorization shall be performed. Enabling Authentication 
requires that a cipher, including RSA (such as *rsa-aes256*), is 
used.

Attribute enabled
-----------------

Authentication is performed when ``enabled`` is set to ``true``.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/Authentication[@enabled]                      |
+-------------+--------------------------------------------------------+
| Format      | Boolean                                                |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | true, false                                            |
| Values      |                                                        |
+-------------+--------------------------------------------------------+

.. _`Element X509Authentication`:

Element X509Authentication
--------------------------

The ``X509Authentication`` element defines where keys and 
certificates required for X509 authentication may be found.

Element Credentials
...................

The ``Credentials`` element is an optional element. If it is 
missing, then the node does not sign messages (in other words, 
does not send credentials).

Element Key
'''''''''''

The ``Key`` element references the file containing the key. 

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/Authentication/X509Authentication/\           |
|             | Credentials/Key                                        |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any file URI                                           |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 1 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | It is recommended that the absolute path is used.      |
|             | A relative path will be interpreted relative to the    |
|             | directory from which the OpenSplice daemon is started. |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element Cert
''''''''''''

The ``Cert`` element references the file containing the certificate.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/Authentication/X509Authentication/\           |
|             | Credentials/Cert                                       |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any file URI                                           |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 1 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | It is recommended that the absolute path is used.      |
|             | A relative path will be interpreted relative to the    |
|             | directory from which the OpenSplice daemon is started. |
|             |                                                        |
+-------------+--------------------------------------------------------+

.. _`Element TrustedCertificates`:

Element TrustedCertificates
...........................

The ``TrustedCertificates`` element references a file containing the 
trusted certificates.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | Security/Authentication/X509Authentication/\           |
|             | TrustedCertificates                                    |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any file URI                                           |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 1 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | It is recommended that the absolute path is used.      |
|             | A relative path will be interpreted relative to the    |
|             | directory from which the OpenSplice daemon is started. |
|             |                                                        |
+-------------+--------------------------------------------------------+


Secure Networking Example Configuration
***************************************

The following XML is an example of a secure networking 
configuration.

::

   <OpenSplice>
       <Domain>
           <Name>OpenSplice Security</Name>
           <Database>
               <Size>10485670</Size>
           </Database>
           <Lease>
               <ExpiryTime update_factor="0.5">5.0</ExpiryTime>
           </Lease>
           <Service enabled="true" name="networking">
               <Command>snetworking</Command>
           </Service>    
       </Domain>

       <NetworkService name="networking">
           <Partitioning>
               <GlobalPartition Address="broadcast"
                  SecurityProfile="GlobalProfile"/>
               <NetworkPartitions>
                   <NetworkPartition Name="ChatRoomPartition"
                      Address="230.230.230.1"
                      SecurityProfile="ChatRoomProfile" />
               </NetworkPartitions>
               <PartitionMappings>
                   <PartitionMapping DCPSPartitionTopic="ChatRoom.*"
                      NetworkPartition="ChatRoomPartition"/>
               </PartitionMappings>
           </Partitioning>

         <Security enabled="true">
            <SecurityProfile Name="GlobalProfile" 
               Cipher="aes128"
               CipherKey="000102030405060708090a0b0c0d0e0f" />
            <SecurityProfile Name="ChatRoomProfile"
               Cipher="blowfish"
               CipherKey="000102030405060708090a0b0c0d0e0f" />
            <SecurityProfile Name="OtherProfile"
               Cipher="rsa-aes128"
               CipherKey=
                  "file:///my/shared/secrets/aes128.key" />

            <AccessControl enabled="true" policy="file://.....">
               <AccessControlModule enabled="true" type="MAC"/>
            <AccessControl/>

            <Authentication enabled="true"> 
               <X509Authentication>
                  <Credentials> 
                     <Key>
                       file:///usr/osp/securityConfig/ProxyKey.pem
                     </Key>
                     <Cert>
                        file:///usr/osp/securityConfig/ \
                           ProxyCert.pem
                     </Cert>
                  </Credentials>
                  <TrustedCertificates>
                     file://../../securityConfig/ \
                        trustedCerts.pem
                  </TrustedCertificates>
               </X509Authentication>
            <Authentication>
         </Security>
         <Channels>
               <Channel 
                     enabled="true" name="BestEffort" 
                     reliable="false" default="true">
                   <PortNr>3340</PortNr>
               </Channel>
               <Channel enabled="true" name="Reliable" reliable="true">
                   <PortNr>3350</PortNr>
               </Channel>
           </Channels>
           <Discovery enabled="true">
               <PortNr>3360</PortNr>
           </Discovery>
       </NetworkService>
   </OpenSplice>


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
