.. _`Access Control Configuration`:


############################
Access Control Configuration
############################

*This section provides a detailed description of the OpenSplice 
access control policy configuration.* 


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

.. _`Access Control Policy Elements`:

Access Control Policy Elements
******************************

When access control is enabled a file containing the access 
control policy configuration is referenced in the secure 
networking configuration.

The access control policy configuration expects a root element 
named ``accessControlPolicy``. Elements defined in an access control 
policy are listed and explained in the following sections.

Element secrecyLevels/secrecyLevel
==================================

The access control policy contains a hierarchical list of 
secrecy levels which are grouped under the ``secrecyLevels`` 
element. Typical secrecy levels would be: ``UNCLASSIFIED``, 
``RESTRICTED``, ``CONFIDENTIAL``, ``SECRET``, and ``TOP_SECRET``. 

|caution|
Note that the order of defined secrecy levels is important: 
secrecy levels are listed from weakest to strongest.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/secrecyLevels/secrecyLevel         |
|             |                                                        |
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
| Occurrences | 0 - *                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | Listed from the weakest to the strongest level.        |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element integrityLevels/integrityLevel
======================================

The access control policy contains a list of integrity levels 
which are grouped under the ``integrityLevels`` element. 

|caution|
The order of defined integrity levels is important. Integrity 
levels are listed from the weakest to the strongest. 

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/integrityLevels/integrityLevel     |
|             |                                                        |
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
| Occurrences | 0 - *                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | Listed from the weakest to the strongest level.        |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element users/user
==================

The ``users`` section contains a set of users. A user has an *id*, a 
*clearance*, and a list of *authentication mechanisms*. 

Element id
----------

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/users/user/id                      |
|             |                                                        |
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
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element Clearance
-----------------

A clearance consists of this user’s secrecy level, integrity 
level, and a set of compartments.

Element secrecyLevel
....................

Defines this user’s secrecy level.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/users/user/clearance/\             |
|             | secrecyLevel                                           |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any, defined in accessControlPolicy/secrecyLevels      |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element integrityLevel
......................

Defines this user’s integrity level.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/users/user/clearance/\             |
|             | integrityLevel                                         |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any, defined in accessControlPolicy/integrityLevels    |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element compartments/compartment
................................

The ``compartments`` section contains a set of compartments this 
user is entitled to access.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/users/user/clearance/\             |
|             | compartments/compartment                               |
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
| Occurrences | 0 - *                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
|             |                                                        |
+-------------+--------------------------------------------------------+


Element authentication
----------------------

This element contains a list of authentication mechanisms for 
this user. 

|info|
Currently, OpenSplice supports SSL X.509 Certificate 
Authentication. Other authentication mechanisms (such as user 
ID/password authentication) may be available in a future release.

Element x509Authentication
..........................

Defines properties of x509 (SSL certificate) authentication. 

**Element subject**

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/users/user/authentication/\        |
|             | x509Authentication/subject                             |
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
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | The distinguished name (DN) of the certificate the     |
|             | user transmits to authenticate to the system (single   |
|             | elements of the distinguished name have to be          |
|             | separated by a comma).                                 |
|             |                                                        |
+-------------+--------------------------------------------------------+


|caution|
Note that the user’s certificate DN must be unique: ensure that 
multiple users do not share the same client certificate DN.

Element resources/resource
==========================

The ``resources`` section contains a set of resources, in other 
words, the objects to be protected. 

A resource has a resource identification (made up of the resource’s 
type, id, and topic or partitions, respectively) and a classification 
(containing the resource’s secrecy and integrity level and a list of 
compartments). The classification is used for mandatory access 
control.

Element type
------------

Defines the type of this resource. A resource can have the type 
``PARTITION`` or ``TOPIC``.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/type            |
|             |                                                        |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | "PARTITION" or "TOPIC"                                 |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
+-------------+--------------------------------------------------------+


Element id
----------

Defines this resource’s id.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/id              |
|             |                                                        |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any valid topic or partition name of a DDS domain      |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | This is related to the type element value.             |
+-------------+--------------------------------------------------------+


Element topics/topic
--------------------

The ``topics`` section contains a set of topics. This element is 
only valid if the type of the resource is ``TOPIC``. It lists all 
valid topics that may be part of this partition.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/topics/topic    |
|             |                                                        |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any valid topic of a DDS domain                        |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | Exists only if element type = PARTITION                |
+-------------+--------------------------------------------------------+

Element partitions/partition
----------------------------

The ``partitions`` section contains a set of partitions. This 
element is only valid if the type of the resource is ``PARTITION``. 
It lists all valid partitions that may be part of this partition.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/\               |
|             | partitions/partition                                   |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any valid partition of a DDS domain                    |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - *                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | Exists only if element type = TOPIC                    |
+-------------+--------------------------------------------------------+


Element classification
----------------------

A classification consists of this resource’s secrecy level, 
integrity level, and a set of compartments.

Element secrecyLevel
....................

Defines this resource’s secrecy level.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/\               |
|             | classification/secrecyLevel                            |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any, defined in accessControlPolicy/secrecyLevels      |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
+-------------+--------------------------------------------------------+


Element integrityLevel
......................

Defines this resource’ integrity level.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/\               |
|             | classification/integrityLevel                          |
+-------------+--------------------------------------------------------+
| Format      | string                                                 |
+-------------+--------------------------------------------------------+
| Dimension   | none                                                   |
+-------------+--------------------------------------------------------+
| Default     | none                                                   |
| Value       |                                                        |
+-------------+--------------------------------------------------------+
| Valid       | any, defined in accessControlPolicy/integrityLevels    |
| Values      |                                                        |
+-------------+--------------------------------------------------------+
| Occurrences | 0 - 1                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
+-------------+--------------------------------------------------------+


Element compartments/compartment
................................

The ``compartments`` section contains a set of compartments this 
resource is intended for.

.. tabularcolumns:: | p{3cm} | p{11.9cm} |

+-------------+--------------------------------------------------------+
| Full Path   | accessControlPolicy/resources/resource/\               |
|             | classification/compartments/compartment                |
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
| Occurrences | 0 - *                                                  |
| (min-max)   |                                                        |
+-------------+--------------------------------------------------------+
| Remarks     | none                                                   |
+-------------+--------------------------------------------------------+


Access Control Example Configuration
************************************

The following XML shows an example access control policy.

::

   <accessControlPolicy>
       <secrecyLevels> <!-- for MAC -->
           <secrecyLevel>UNCLASSIFIED</secrecyLevel>
           <secrecyLevel>CONFIDENTIAL</secrecyLevel>
           <secrecyLevel>SECRET</secrecyLevel>
           <secrecyLevel>TOP_SECRET</secrecyLevel>
       </secrecyLevels>

       <integrityLevels> <!-- for MAC -->
           <integrityLevel>LEVEL_0</integrityLevel>
           <integrityLevel>LEVEL_1</integrityLevel>
           <integrityLevel>LEVEL_2</integrityLevel>
       </integrityLevels>
       <users>
           <user>
               <id>user1</id>
               <clearance> <!-- for MAC -->
                   <secrecyLevel>CONFIDENTIAL</secrecyLevel>
                   <integrityLevel>LEVEL_2</integrityLevel>
                   <compartments>
                       <compartment>US Only</compartment>
                       <compartment>Air Force</compartment>
                       <compartment>Radar</compartment>
                   </compartments>
               </clearance>
               <authentication>
                   <x509Authentication>
                       <subject>DN</subject>
                   </x509Authentication>
               </authentication>
           </user>
           <user>
               <id>user2</id>
               <authentication>
                   <x509Authentication>
                       <subject>DN2</subject>
                   </x509Authentication>
               </authentication>
           </user>
       </users>
       <resources>
           <resource>
                <type>PARTITION</type>
                  <id>chat</id>
               <topics>
                 <topic>ChatMessage</topic>
                 <topic>NamedMessage</topic>
               </topics>
               <classification> <!-- for MAC -->
                   <secrecyLevel>CONFIDENTIAL</secrecyLevel>
                   <integrityLevel>LEVEL_1</integrityLevel>
                   <compartments>
                       <compartment>US Only</compartment>
                       <compartment>Air Force</compartment>
                   </compartments>
               </classification>
           </resource>
           <resource>
               <type>TOPIC</type>
                 <id>pingpong</id>
              <partitions>
                 <partition>PING</partition>
                 <partition>PONG</partition>
              </partitions>
           </resource>
           <resource>
               <type>TOPIC</type>
                 <id>topic1</id>
              <partitions>
              </partitions>
           </resource>
       </resources>
   </accessControlPolicy>


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
