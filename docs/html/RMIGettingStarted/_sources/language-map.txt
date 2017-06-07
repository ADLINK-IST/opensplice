.. _`Language mapping for |product_name| RMI`:

#######################################
Language mapping for |product_name| RMI
#######################################

.. ifconfig:: 'Java' in (rmi_languages)

  Rmipp compilation follows a set of mapping rules to generate language-specific
  source code. Most of these rules come from the standard OMG IDL-to-C++ and
  IDL-to-Java mapping specifications but with some specific differences.
  This chapter focuses on specific parts of this mapping. For more information,
  please refer to the relevant OMG specifications.

.. ifconfig:: 'Java' not in (rmi_languages)

  Rmipp compilation follows a set of mapping rules to generate language-specific
  source code. Most of these rules come from the standard OMG IDL-to-C++
  mapping specification but with some specific differences.
  This chapter focuses on specific parts of this mapping. For more information,
  please refer to the relevant OMG specifications.

The following figure shows the language mapping of the ``HelloService`` 
IDL interface previously defined.

.. _`IDL Interface Mapping`:

.. figure:: /images/RMI-diag-05.png
   :height: 100mm
   :alt: IDL Interface Mapping

**IDL Interface Mapping**


**********************
Mapping for interfaces
**********************

.. ifconfig:: 'Java' in (rmi_languages)

  An interface is mapped to two C++ (or Java) classes that contain public
  definitions of the operations defined in the interface.

.. ifconfig:: 'Java' not in (rmi_languages)

  An interface is mapped to two C++ classes that contain public
  definitions of the operations defined in the interface.

The ``HelloServiceInterface`` abstract class is the base class of the
``HelloService`` implementation class. The ``HelloServiceInterfaceProxy``
class is the proxy object that represents locally the remote service.
The client application should get a reference to this class to be able
to invoke the remote service.

**********************
Mapping for operations
**********************

.. ifconfig:: 'Java' in (rmi_languages)

  Each IDL operation, if not oneway, is mapped to two C++ functions
  (Java methods). The first one, having the same name as the IDL operation,
  is used for synchronous invocations. The second one, having ``async_``
  concatenated to the IDL operation, is used for asynchronous invocations.
  A oneway operation maps only to the synchronous form of the operations.

  The operations parameters and return types obey the same parameter
  passing rules as for the standard OMG IDL-to-C++ and IDL-to-Java mapping.
  The asynchronous functions (methods) will return void and take only the
  in/inout parameters of the IDL operation, as well as a callback object
  used as a reply handler. This handler class is also generated for each
  non-void operation as an inner abstract class of the proxy class as
  depicted in the diagram with the ``greet_Reply_Handler`` class. This
  latter should be implemented by the user to handle the asynchronous
  invocation reply. Hence, the ``greet_Reply`` function (method) provides
  all the inout/out/return parameters of the corresponding IDL operation.

.. ifconfig:: 'Java' not in (rmi_languages)

  Each IDL operation, if not oneway, is mapped to two C++ functions.
  The first one, having the same name as the IDL operation,
  is used for synchronous invocations. The second one, having ``async_``
  concatenated to the IDL operation, is used for asynchronous invocations.
  A oneway operation maps only to the synchronous form of the operations.

  The operations parameters and return types obey the same parameter
  passing rules as for the standard OMG IDL-to-C++ mapping.
  The asynchronous functions (methods) will return void and take only the
  in/inout parameters of the IDL operation, as well as a callback object
  used as a reply handler. This handler class is also generated for each
  non-void operation as an inner abstract class of the proxy class as
  depicted in the diagram with the ``greet_Reply_Handler`` class. This
  latter should be implemented by the user to handle the asynchronous
  invocation reply. Hence, the ``greet_Reply`` function (method) provides
  all the inout/out/return parameters of the corresponding IDL operation.

***********************
Mapping for basic types
***********************

The table below shows the |rmi_langs| mapping of the IDL types that
can be declared in the RMI services description file.

IDL sequences are mapped as specified by the DDS standard.

**Mapping for basic types**

.. ifconfig:: 'Java' in (rmi_languages)

  ===================  ==============  =======
  IDL type             C++             Java
  ===================  ==============  =======
  boolean              DDS::Boolean    boolean
  char                 DDS::Char       char
  octet                DDS::Octet      byte
  short                DDS::Short      short
  unsigned short       DDS::UShort     short
  long                 DDS::Long       int
  unsigned long        DDS::ULong      int
  long long            DDS::LongLong   long
  unsigned long long   DDS::ULongLong  long
  float                DDS::Float      float
  double               DDS::Double     double
  string               DDS::String     String
  ===================  ==============  =======

.. ifconfig:: 'Java' not in (rmi_languages)

  ===================  ==============
  IDL type             C++
  ===================  ==============
  boolean              DDS::Boolean
  char                 DDS::Char
  octet                DDS::Octet
  short                DDS::Short
  unsigned short       DDS::UShort
  long                 DDS::Long
  unsigned long        DDS::ULong
  long long            DDS::LongLong
  unsigned long long   DDS::ULongLong
  float                DDS::Float
  double               DDS::Double
  string               DDS::String
  ===================  ==============





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

