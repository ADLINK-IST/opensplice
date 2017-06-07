.. _`Proto message for a DDS system`:

##############################
Proto message for a DDS system
##############################


Individual declarations in a ``.proto`` file can be annotated with a number
of options. Options do not change the overall meaning of a declaration,
but may affect the way it is handled in a particular context.

Options can be defined at different levels:

  - File-level options: meaning they should be written at the top-level
    scope, not inside any message, enum, or service definition.
  - Message-level options: meaning they should be written inside message
    definitions.
  - Field-level options: meaning they should be written inside field
    definitions. Enum types, enum values, service types, and service
    methods.

Use case: Person
================

In this use case example, a system capable of describing the personal
data of persons must be built using the GPB data-model

The layout can be::

  string name
  integer age
  sequence phone-number + type
  sequence friends

Proto file for the Person example
=================================

This use case is described in this ``.proto`` file:

.. code-block:: protobuf

   import "omg/dds/descriptor.proto";
   package address;

   message Person {
      required string name = 1;
      required int32 age = 2;
      optional string email = 3;

      enum PhoneType {
         UNDEFINED = 0;
         MOBILE    = 1;
         HOME      = 2;
         WORK      = 3;
      }

      message PhoneNumber {
         required string number = 1;
         optional PhoneType type  = 2 [default = HOME];
      }

   repeated PhoneNumber phone = 4;
   repeated Person friend = 5;
   }


GPB labels every field as either a *required* or an *optional* field.
Required fields are *always* used/filled; optional fields may or may not
be.

Different data models are compatible if all *required* fields are the same.
Data models can be extended with extra fields; if those new fields are all
*optional*, then the new model will still be compatible with older applications
using the old data model.

In our example the *name* and *age* are always required. The *email* string
is optional, as extra information for this person. The sequences with phone
numbers and friends are allowed to be empty.

Detailed explanation for the layout of a ``.proto`` file can be found in
the Google Protocol buffer documentation on
https://developers.google.com/protocol-buffers/docs/proto


Annotating a proto message for use as a type in DDS
===================================================

For the GPB message to be able to be handled correctly in a DDS system,
some options are needed in the ``.proto`` file which define how the GPB
message shall behave in the DDS system.

At the message level there is an extra option ``.omg.dds.type``.
This tells the protocol buffer compiler that this message is also a dds type message.
This type option has a optional extra parameter for giving this type a dds type name.
By default it has the same name in the DDS domain as it has in GPB.

The Person example with this option:

.. code-block:: protobuf

   import "omg/dds/descriptor.proto";

   package address;

   message Person {
    option (.omg.dds.type) = {};
    required string name = 1;
    required int32 age = 2;

   enum PhoneType {
     UNDEFINED = 0;
     MOBILE    = 1;
     HOME      = 2;
     WORK      = 3;
   }

   message PhoneNumber {
      required string number = 1;
     optional PhoneType type  = 2 [default = HOME];
    }
    repeated PhoneNumber phone = 4;
    repeated Person friend = 5;
   }


Proto file with omg.dds.member.key option
-----------------------------------------

For support of a key value in the datamodel, the option ``key`` can be given as a
field-member option. One or more fields containing this option will indicate that
these members make a unique key identifier in the data model. A field
indicated as a key field must always be a *required* field for GPB. Also a key
field is automatically a filterable field, as described below.

The Person example with *name* as a unique key (this means that each unique value
of the name will lead to a separate instance in DDS with its own history):

.. code-block:: protobuf

   import "omg/dds/descriptor.proto";

   package address;

   message Person {
      option (.omg.dds.type) = {};
      required string name = 1 [(.omg.dds.member).key = true];
      required int32 age = 2;
      optional string email = 3;
   }
   enum PhoneType {
      UNDEFINED = 0;
      MOBILE = 1;
      HOME = 2;
      WORK = 3;
   }
   message PhoneNumber {
      required string number = 1;
      optional PhoneType type = 2 [default = HOME];
   }

   repeated PhoneNumber phone = 4;
   repeated Person friend = 5;


Proto file with omg.dds.member.filterable option
------------------------------------------------

For support of filterable fields in the datamodel, the option ``filterable`` can be
given as a field-member option.

One or more fields with this option indicates that these members are available
for dynamic querying and filtering by means of a ``QueryCondition`` or
``ContentFilteredTopic`` in DDS.

A field marked as a filterable field must always be a *required* field in GPB.
A key field is always filterable, by definition.

The Person example with *age* as a filterable attribute:

.. code-block:: protobuf

   import "omg/dds/descriptor.proto";

   package address;

   message Person {
    option (.omg.dds.type) = {};
    required string name = 1 [(.omg.dds.member).key = true];
    required int32 age = 2 [(.omg.dds.member).filterable = true];
    optional string email = 3;

   enum PhoneType {
     UNDEFINED = 0;
     MOBILE    = 1;
     HOME      = 2;
     WORK      = 3;
   }

   message PhoneNumber {
      required string number = 1;
     optional PhoneType type  = 2 [default = HOME];
    }
    repeated PhoneNumber phone = 4;
    repeated Person friend = 5;
   }

Proto file with omg.dds.member.name option
------------------------------------------

The previous examples will result in a DDS type with the directly-mapped fields in
IDL with the same name as in proto. (Key fields and filterable fields are directly
mapped.)

If a different name is needed in the DDS domain for a fieldname in the generated
IDL and dds type, a name can be given as an ``omg.dds.member`` option.

Example where the *age* field will be named ``AgeInYears`` in the DDS domain:

.. code-block:: protobuf

   import "omg/dds/descriptor.proto";

   package address;

   message Person {
    option (.omg.dds.type) = {};
    required string name = 1 [(.omg.dds.member).key = true];
    required int32 age = 2   [(.omg.dds.member) = { name: "AgeInYears" filterable: true }];
    optional string email = 3 ;

   enum PhoneType {
     UNDEFINED = 0;
     MOBILE    = 1;
     HOME      = 2;
     WORK      = 3;
   }

   message PhoneNumber {
      required string number = 1;
     optional PhoneType type  = 2 [default = HOME];
    }
    repeated PhoneNumber phone = 4;
    repeated Person friend = 5;
   }



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

