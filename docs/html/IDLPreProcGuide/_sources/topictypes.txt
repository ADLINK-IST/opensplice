.. _`Extensible and Dynamic Topic Types`:


#############################################################
Extensible and Dynamic Topic Types for DDS annotation support
#############################################################

The specification defines an annotation language as extension to IDL. Even though 
this  specification has not been implemented in Vortex OpenSplice, its IDL pre-processor 
is already able to parse these extensions even though it does not generate 
anything special for them yet. This allows users to write future-proof 
IDL definitions with annotations already in them.

The specification describes *two* notations for defining annotations in IDL, a prefix
and a suffix notation. An annotation type is defined by prefixing a local interface
definition with the new token ``@Annotation``. The members of these types shall be
represented using IDL attributes, as shown in the following example using the
prefix notation:

:: 
   
   1  @Annotation
   2  local interface MyAnnotation {
   3     attribute long my_annotation_member_1;
   4     attribute double my_annotation_member_2;
   5  };
   

Alternately and equivalently, an annotation can be defined by suffixing the interface
with the new annotation token using *slash-slash-at* (``//@Annotation``) instead,
like this:

:: 
   
   1  local interface MyAnnotation {
   2     attribute long my_annotation_member_1;
   3     attribute double my_annotation_member_2;
   4  }; //@Annotation


An annotation can be applied to a type or type member by prefixing it with an
*at* sign (``@``) and the name of the annotation type to apply. To specify the 
values of any members of the annotation type, include them in name=value syntax 
between parentheses; for example:

:: 
   
   1  @MyTypeAnnotation
   2  struct Gadget {
   3     @MyAnnotation(my_annotation_member_1=5,
      my_annotation_member_2=3.4) long my_integer;
   4  };

Alternately and equivalently, an annotation can also be applied to a type 
or type member by suffixing it with an annotation type name using 
*slash-slash-at* (``//@``) instead of the *at* sign by itself; for example:

:: 
   
   1  struct Gadget {
   2  long my_integer;//@MyAnnotation(my_annotation_member_1=5,
      my_annotation_member_2=3.4)
   3  }; //@MyTypeAnnotation


|cpp| |java|

  Please note that the IDL Pre-Processor *only* supports the *suffix* notation 
  when selecting C++ (``-l cpp``, ``-l c++``, ``-l isocpp``, ``-l isoc++``,
  ``-l isocpp2`` or ``-l isoc++2``) or CORBA-cohabitated java (``-l java -C``)
  as language.


For other languages both prefix and suffix notations are supported. The 
:ref:`Extensible and Dynamic Topic Types for DDS specification <OMG DDS XTYPES 2012>`
also defines a number of annotations for use by applications. These types 
do not appear as annotations at runtime; they exist at runtime only in order 
to extend the capabilities of IDL. The following annotations have been 
defined and are accepted by the Vortex OpenSplice IDL pre-processor as well:

+ ID
+ Optional
+ Key
+ Shared
+ BitBound
+ Value
+ BitSet
+ Nested
+ Extensibility
+ MustUnderstand
+ Verbatim

For more details on built-in annotations and annotations in general please
refer to section 7.3.1 of the
:ref:`OMG Extensible and Dynamic Topic Types for DDS specification <OMG DDS XTYPES 2012>`.




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
