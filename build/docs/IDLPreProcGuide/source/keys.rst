.. _`Keys`:


####
Keys
####

.. _`Defining Keys`:

Defining Keys
*************

The Vortex OpenSplice IDL Pre-processor also provides a mechanism to define
a list of keys (space- or comma-separated) with a specific data type. The
syntax for that definition is:

:: 
   
   #pragma keylist <data-type-name> <key>*


The identifier ``<data-type-name>`` is the identification of a struct 
or a union definition.

The identifier ``<key>`` is the member of a struct. For a struct either no key list is
defined, in which case no specialized interfaces (``TypeSupport``, ``DataReader`` and
``DataWriter``) are generated for the struct, or a key list with or without keys is
defined, in which case the specialized interfaces are generated for the struct. For a
union either no key list is defined, in which case no specialized interfaces are
generated for the union, or a key list without keys is defined, in which case the
specialized interfaces are generated for the union. It is not possible to define keys
for a union because a union case may only be addressed when the discriminant is set
accordingly, nor is it possible to address the discriminant of a union. The keylist
must be defined in the same name scope or module as the referred struct or union.

.. _`Supported types for keys`:

Supported types for keys
========================

Vortex OpenSplice supports following types as keys:

+ short
+ long
+ long long
+ unsigned short
+ unsigned long
+ unsigned long long
+ float
+ double
+ char
+ boolean
+ octet
+ string
+ bounded string
+ enum
+ char array (provided that ``#pragma cats`` is specified;
  see `Character arrays as Keys`_ below)

Vortex OpenSplice also supports typedef for these types.


.. _`Character arrays as Keys`:

Character arrays as Keys
========================

By default Vortex OpenSplice does not support using a character array as a key. Using
an (character) array as a key field is not desirable or supported, because:

1. Every index in the array must be considered a separate key in this situation. 
   This is the only way that arrays can be compared to each other in a correct manner.
   An array of ten characters would have to be treated as a ten-dimensional storage
   structure, leading to poor performance compared with the processing of a
   (bounded) string of ten characters.

2. An array always has a fixed length and therefore the whole array is sent over the
   wire even if only a small part of it is needed. When using a (bounded) string,
   only the actual string is sent and not the maximum length.

However, in certain scenarios a character array is the logical key for a topic, either
from an information modeling perspective or simply due to a legacy data model. To
facilitate such scenarios Vortex OpenSplice introduces the following pragma which
allows for character arrays to be used as a key.

:: 
   
   #pragma cats <data-type-name> <char-array-field-name>*


The identifier ``<data-type-name>`` is the identification of a struct definition.
The identifier ``<char-array-field-name>`` is the member of a struct with the type
character array. The ``cats`` pragma *must* be defined in the same name scope or
module as the referred struct.

This pragma ensures that each character array listed for the specified struct
definition is treated as a string type internally within Vortex OpenSplice and operates
exactly like a regular string. This allows the character array to be used as a key for
the data type, because as far as Vortex OpenSplice is concerned the character array is
in fact a string. On the API level (*e.g.*, generated code) the character array is
maintained so that applications will be able to use the field as a regular character
array as normal. Be aware that listing a character array here does *not* promote the
character array to a key of the data type; the regular keylist pragma must still be
used for that. In effect this pragma can be used to let any character array be treated
as a string internally, although that is not by definition desirable.

When a character array is mapped to a string internally by using the cats pragma,
the product behaves as follows:

1. If the character array does not have a '\0' terminator, the middleware will add a
   ``\0`` terminator internally and then remove it again in the character array that 
   is presented to a subscribing application. In other words, a character array used 
   in combination with the cats pragma does not need to define a ``\0`` terminator as
   one of its elements.

2. If the character array does have a ``\0`` terminator, the middleware will only
   process the characters up to the first element containing the ``\0`` character; 
   all other characters are ignored. The middleware will present the character array
   with the same ``\0`` terminator to a subscribing application and any array
   elements following that ``\0`` terminator will contain ``\0`` terminators as well;
   *i.e.*, any array elements following a ``\0`` element are ignored.

The following table shows some examples using the *cats* pragma for a character
array with a size of ``4``.

+-------------------------+-------------------------+--------------------------+
| *Character array*       | *Internal string*       | *Character array*        |
| *written*               | *representation*        | *read*                   |
+-------------------------+-------------------------+--------------------------+
| (by publishing          | (Internal OpenSplice    | (By subscribing          |
| application)            | data)                   | application)             |
+=========================+=========================+==========================+
| ``['a','b','c','d']``   | ``"abcd"``              | ``['a','b','c','d']``    |
+-------------------------+-------------------------+--------------------------+
| ``['a','b','c','\0']``  | ``"abc"``               | ``['a','b','c','\0']``   |
+-------------------------+-------------------------+--------------------------+
| ``['a','b','\0','d']``  | ``"ab"``                | ``['a','b','\0','\0']``  |
+-------------------------+-------------------------+--------------------------+



.. _`Bounded strings as character arrays`:

Bounded strings as character arrays
***********************************

In some use cases a large number of (relatively small) strings may be
used in the data model, and because each string is a reference type, it
means that it is not stored inline in the data model but instead as a
pointer. This will result in separate allocations for each string (and
thus a performance penalty when writing data) and a slight increase in
memory usage due to pointers and (memory storage) headers for each
string.

The Vortex OpenSplice IDL Pre-Processor features a special pragma called
``stac`` which can be utilized in such use cases. This pragma enables you
to indicate that Vortex OpenSplice should store strings internally as
character arrays (but on the API level they are still bounded strings).
Because a character array has a fixed size, the pragma ``stac`` only
affects bounded strings. By storing the strings internally as a
character array the number of allocations is reduced and less memory is
used. This is most effective in a scenario where a typical string has a
relatively small size, *i.e.* fewer than 100 characters.

|caution|

  Using the pragma ``stac`` on bounded strings results in the limitation
  that those strings can no longer be utilized in queries. It also results
  in the maximum size of the bounded string to be used each time,
  therefore the pragma ``stac`` is less suitable when the string has a large
  bound and does not always use up the maximum space when filled with
  data. A bounded string that is also mentioned in the pragma ``keylist``
  can not be listed for pragma ``stac``, as transforming those strings to
  an array would violate the rule that an array can not be a keyfield.

::
   
   #pragma stac <data-type-name> [[!]bounded-string-fieldname]\*
                                                             
The identifier *<data-type-name>* is the identification of a struct
definition. The identifier *[[!]bounded-string-field-name]* is the
member of a struct with the type bounded string. The ``stac`` pragma must
be defined in the same name scope or module as the referred struct. If
no field names are listed, then all bounded strings will be treated as
character arrays internally. If only a subset of the struct members is
targeted for transformation then these members can be listed explicitly
one by one. Preceeding a field name with a ``!`` character indicates
that the listed member should not be considered for transformation from
bounded string to character array. 

|caution|

  Member names with and without the ``!`` character may not be mixed 
  within a ``stac`` pragma for a specific struct as this has no relevant 
  meaning. This pragma ensures that each bounded string listed for the 
  specified struct definition is treated as a character array type 
  internally within Vortex OpenSplice and operates  exactly like a regular 
  bounded string. On the API level ( *i.e.* , generated code) the bounded 
  string is maintained so that applications will be able to use the 
  field as a regular bounded string.






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

.. EoF
