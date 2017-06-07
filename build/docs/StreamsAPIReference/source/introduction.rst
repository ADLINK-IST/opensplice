.. _`Introduction`:


############
Introduction
############


Features
********

*Vortex OpenSplice Streams API* supports a common data-distribution pattern 
where continuous flows or *streams* of data have to be transported with
minimal overhead and therefore maximal achievable throughput.

Vortex OpenSplice Streams API implements this *streams pattern* by transparent
packing and queuing of data samples using auto-generated *containers*,
thus minimizing the overhead normally associated with the management and
distribution of individual DDS samples.

Getting Started
***************

The Vortex OpenSplice Streams API is divided in two main components:

+ type-specific code that can be generated using the OpenSplice 
  IDL Pre-Processor
  
+ a Streams library. 

Applications that wish to use the Streams API are required to do two things:

1. Link against *one* of the OpenSplice Streams libraries available
   within the Vortex OpenSplice distribution. There are separate libraries for
   either *CORBA-Cohabitation* mode or *Standalone C++* mode.

2. Annotate the data-model IDL file with ``#pragma`` stream directives for
   each data type for which a Stream needs to be created.

The Vortex OpenSplice Streams API is built on the DCPS API. Since the C++
bindings of Vortex OpenSplice are available in two flavours, so is the Streams
API. In the following paragraphs the steps will be discussed to build a
simple application that uses the following data-model:

:: 
   
   Space.idl:

   module Space {
       struct Foo {
           long long_1;
           long long_2;
       };
   #pragma stream Foo

       struct Type2 {
         long long_1;
         long long_2;
         long long_3;
       };
   #pragma stream Type2
   #pragma keylist Type2 long_1

   };


Using this model, both ``Foo`` and ``Type2`` can be used with the 
Streams API. In addition ``Type2`` can also be used as a regular 
DDS topic, with ``long_1`` as key.

The following relevant Streams API classes are generated based on this
model for ``Foo``:

:: 
   
   Space::FooStreamDataWriter
   Space::FooStreamDataReader
   Space::FooStreamBuf


It is recommended to use smart references to the ``StreamDataWriter`` and
``StreamDataReader`` classes in applications. The regular Vortex OpenSplice C++
smart-pointer ``<class>_var`` types are available for this purpose. See
the section on *Memory Management* in the Vortex OpenSplice DDS *C++ Reference
Guide* for more information.

CORBA Cohabitation Mode
=======================

In *CORBA Co-habitation* mode, ``idlpp`` generates code that can be processed
with any of the supported ORB compilers (OpenFusion TAO, Mico, *etc.*).

First ``idlpp`` is executed on the ``Space.idl`` file:

:: 

   $ idlpp -I$OSPL_HOME/etc/idl -l cpp -C Space.idl

The standard Vortex OpenSplice DDS IDL directory is referenced as ``include-path``,
since it contains definitions of some basic data-types and interfaces
that are required if DDS Topics are created for any of the types in the
IDL file. The other parameters are used to put ``idlpp`` in C++
CORBA-Cohabitation mode.

As usual when DDS topics are created, the above command generates, among
other files, a file called ``SpaceDcps.idl``. The file ``SpaceStreams.idl``
is also generated.

To proceed, ``idlpp`` should be executed on the ``ExampleStreams.idl`` file:

:: 

   $ idlpp -I$OSPL_HOME/etc/idl -l cpp -C SpaceStreams.idl

This creates the descriptions of the DCPS entities that are required to
manage the DDS topics that will be used for the Streams types, just like
with the original IDL file, in a file called ``SpaceStreamsDcps.idl``.

Now all four IDL files should be processed with the appropriate (ORB-specific)
CORBA IDL processor. After this step all code and header files are generated
to start using the Streams API in application code.  

Standalone Mode
===============

In *Standalone C++* mode, the generated interfaces are *not* required to be
processed by an IDL compiler. Instead, ``idlpp`` will use the ``cppgen``
code-generator that is part of the Vortex OpenSplice DDS distribution. ``idlpp``
will automatically call ``cppgen`` to process certain files; the user is
only required to execute ``idlpp``, first on the original IDL file:

:: 

   $ idlpp -I$OSPL_HOME/etc/idl -l cpp -S Space.idl

This creates ``SpaceStreams.idl``, which in turn also needs to be 
processed by ``idlpp``:

:: 

   $ idlpp -I$OSPL_HOME/etc/idl -l cpp -S -i SpaceStreams.idl

The ``-i`` parameter is required because normally no code is generated for
interfaces (for DDS topics, only datatypes are generated). In the case of
streams, interfaces should not be ignored.






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

         