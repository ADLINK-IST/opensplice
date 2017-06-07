.. _`Introduction`:

############
Introduction
############


*******************************
Google Protocol Buffers for DDS
*******************************

*Vortex OpenSplice* is capable of using the **Google Protocol Buffer**
(*GPB*) system for publishing and subscribing GPB messages in a DDS
system. This makes it possible to use GPB as an alternative to OMG-IDL
for those who prefer to use GPB rather than IDL. With the
seamless integration of GPB and DDS technologies there is no need for
OMG-IDL knowledge or visibility when working with GPB data models, and
no OMG-DDS data-types are needed in the application (no explicit
type-mapping between GPB and DDS types is required).

This results in an easy migration of GPB users to DDS(-based
data-sharing) with data-centric GPB with support for keys, filters
and (future) QoS-annotations (ony a few DDS calls are needed).
Also easy migration of DDS applications to GPB(-based data-modeling),
only the field accessors change.

This Tutorial will describe how this is done for the language bindings
**Java5** and **ISO-C++** by defining a GPB message layout which is
compiled into proper interfaces for the Vortex DDS system.

GPB Installation and usage with DDS
===================================
Google Protocol Buffers (GPB) can be downloaded from the following locations:

Linux:
https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.tar.gz

Windows:
https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.zip

After unpacking follow the install instructions located in install.txt in the unpacked directory.
For windows there is a visual studio solution file that will build everything that is needed.

|info|
  | In order for GPB to work with DDS an environment variable **PROTOBUF_HOME** needs to be
  | set that points to the unpacked directory.
  | For windows also another environment variable **PROTOBUF_LIB_HOME** needs to be set that
  | points to the directory that contains the generated libprotobuf.lib.

IDL usage in a DDS system
=========================

In a Data Distributed System (DDS) as a Global DataSpace (GDS) for
ubiquitous information-sharing in distributed systems as specified by
the Object Management Group (OMG), the data is traditionally
captured in the platform- and language-independent OMG-IDL language.
The relational model of DDS is supported by the notion of
identifying key fields in these data structures
where structure/content-awareness by the middleware allows for
dynamic querying and filtering of data.

Google Protocol Buffers
=======================

Google Protocol Buffers (GPB) are a flexible, efficient, automated
mechanism for serializing structured data; think XML, but smaller,
faster, and simpler. One can define how data needs
to be structured once, after which language-specific source code can
be generated to easily write and read this structured data to and
from a variety of data streams using a variety of
languages. The information structure is defined in so-called *protocol
buffer* message types in ``.proto`` files. Each protocol buffer message is
a small logical record of information, containing a series of name-value
pairs. This approach is quite similar to using IDL for data modeling in
combination with an IDL compiler (as available in OpenSplice and DDS
implementations in general).

Additionally, the GPB data structure can be updated without breaking
deployed programs that are compiled against the 'old' format,
similar to the *xTypes* concept as defined for DDS.

Using a GPB data-model instead of an IDL data-model
---------------------------------------------------

For an IDL-OMG based application, the IDL file is compiled with the
IDL-PP compiler to generate the needed classes.

For Java as an example, ``Address.idl`` will (among others)
be compiled into:

  - ``Address.java``
  - ``AddressTypesSupport.java``
  - ``AddressDataWriter.java``
  - ``AddressDataReader.java``
  - ...

Using a GPB data-model, it is not necessary to create IDL files. The
``protoc_gen_ddsJava`` plug-in in OpenSplice will create them from the
given ``.proto`` data-model.

For the GPB ``.proto`` based application, the ``.proto`` file is first
compiled by the Google ``protoc`` compiler. This compiler will call the
``protoc_gen_ddsJava`` plug-in in OpenSplice with the
``.proto`` data parsed into a ``CodeGeneratorRequest`` protocol
buffer.

The OpenSplice plug-in will generate an IDL file from this data. Any
field member that is marked as key or filterable is explicitly mapped
to a member in the IDL type.

The complete serialized ``.proto`` message is stored in the generic
``ospl_protobuf_data`` attribute as a sequence of bytes (making it
opaque data for DDS). The mapping between data types is
given in the table `Mapping of GPB types to DDS types`_.

As the next step the IDL-PP compiler will generate the previously-named
files from the idl file needed for the DDS domain.
The Google ``protoc`` compiler will generate the classes needed for the
GPB domain.

The dds options for the proto file are given in the
`omg/dds/descriptor.proto`_ file listed below.
This proto file shows how the different dds options on the proto
file are interpreted, and gives the unique id ``1016`` to the dds types.

|info|
  | **Note that the id 1016 has officially been granted to the Vortex product by Google.**
  | This ensures these options are always unique and won't clash with any options used by users.

How mapping is done between the different languages is shown below in
the table `Mapping of GPB types to DDS types`_.


omg/dds/descriptor.proto
------------------------

.. literalinclude:: ../../../../src/tools/protobuf/protos/omg/dds/descriptor.proto
   :language: protobuf


Mapping of GPB types to DDS types
---------------------------------

.. tabularcolumns:: | p{1.6cm} | p{7.5cm} | p{1.5cm} | p{1.5cm} | p{1.5cm} |

+---------------+-------------------------------------+----------+-----------+---------------+
| .proto Type   | Notes                               | C++ Type | Java Type | DDS IDL Type  |
+===============+=====================================+==========+===========+===============+
| double        |                                     | double   | double    | double        |
+---------------+-------------------------------------+----------+-----------+---------------+
| float         |                                     | float    | float     | float         |
+---------------+-------------------------------------+----------+-----------+---------------+
| int32         | Uses variable-length encoding.      | int32    | int       | long          |
|               | Inefficient for encoding            |          |           |               |
|               | negative numbers; if your field     |          |           |               |
|               | is likely to have negative values,  |          |           |               |
|               | use sint32 instead                  |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+
| int64         | Uses variable-length encoding.      | int64    | long      | long long     |
|               | Inefficient for encoding            |          |           |               |
|               | negative numbers; if your field     |          |           |               |
|               | is likely to have negative values,  |          |           |               |
|               | use sint64 instead                  |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+
| uint32        | Uses variable-length encoding       | uint32   | int       | unsigned long |
+---------------+-------------------------------------+----------+-----------+---------------+
| uint64        | Uses variable-length encoding       | uint64   | long      | unsigned long |
|               |                                     |          |           | long          |
+---------------+-------------------------------------+----------+-----------+---------------+
| sint32        | Uses variable-length encoding.      | int32    | int       | long          |
|               | Signed int value.                   |          |           |               |
|               | These encode negative numbers       |          |           |               |
|               | more efficiently than regular       |          |           |               |
|               | int32s.                             |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+
| sint64        | Uses variable-length encoding.      | int64    | long      | long long     |
|               | Signed int value.                   |          |           |               |
|               | These encode negative numbers       |          |           |               |
|               | more efficiently than regular       |          |           |               |
|               | int64s.                             |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+
| fixed32       | Always four bytes.                  | uint32   | int       | unsigned long |
|               | More efficient than uint32 if       |          |           |               |
|               | values are often greater than 2^28. |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+
| fixed64       | Always eight bytes.                 | uint64   | long      | unsigned long |
|               | More efficient than uint64 if       |          |           | long          |
|               | values are often greater than 2^56. |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+
| sfixed32      | Always four bytes.                  | int32    | int       | long          |
+---------------+-------------------------------------+----------+-----------+---------------+
| sfixed64      | Always eight bytes.                 | int64    | long      | long long     |
+---------------+-------------------------------------+----------+-----------+---------------+
| bool          |                                     | bool     | boolean   | bool          |
+---------------+-------------------------------------+----------+-----------+---------------+
| string        | A string must always contain UTF-8  | string   | String    | string        |
|               | encoded or 7-bit ASCII text         |          |           |               |
+---------------+-------------------------------------+----------+-----------+---------------+



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

