.. _`Compiling the datamodel with the GPB compiler`:

#############################################
Compiling the datamodel with the GPB compiler
#############################################

Once you've defined your messages, you run the protocol buffer
compiler for your application's language on your ``.proto`` file to
generate data access classes. These provide simple accessors
for each field so, for instance, if your chosen language is ISO-C++,
running the compiler on the above example will generate a class
called *Person*. You can then use this class in your
application to populate, serialize, and retrieve *Person* protocol
buffer messages.

DDS-specific GPB-compiler plugin to generate code.
==================================================

The GPB compiler can be extended to support new languages *via*
so-called plugins. The compiler invokes the plugin while providing
the GPB type definition to it in the form of a GPB message.
For DDS support the OpenSplice GPB-compiler is delivered with
Vortex OpenSplice.

The Vortex OpenSplice IDL compiler is invoked by the OpenSplice
GPB-compiler plugin to generate the DDS type including typed
DataWriter and DataReader code. Additionally, code is generated to
convert an instance of the DDS type to the GPB type and *vice versa*,
which hides the DDS type from the application entirely.

Java 5 example
==============
|java|

In this example we assume that a correct OpenSplice environment is
set (``release.bat`` has been run).

For creating the DDS-specific code by the GPB compiler the option
``--ddsjava_out`` must be given to the compiler. Also the path to the
OpenSplice GPB-compiler must be supplied.

Example::

   protoc --java_out    =outputPath
          --ddsjava_out =outputPath
          --proto_path  =PathToProtoFile
          --proto_path  =PathToProtoSelf
          --proto_path  =PathToOpenSpliceProtoCompiler
          protoFileToCompile

..

  - ``--java_out`` gives the path where the GDP generated code will be stored.
  - ``--ddsjava_out`` gives the path where the DDS-specific generated code will be stored.
  - first ``--proto_path``: the protoc compiler needs the path where the ``.proto`` file is located.
  - second ``--proto_path``: the path where the GPB environment is installed on your local machine.
  - third ``--proto_path``: specifies the path to the OpenSplice proto descriptor.
    This is normally ``$OSPL_HOME/include/protobuf``.
  - ``protoFileToCompile`` the last option is the ``.proto`` file.

Assuming that we need the generated code in the ``./generated`` directory
and the previous ``address.proto`` example is in the current directory,
the command will be::

   protoc --java_out=./generated
          --ddsjava_out=./generated
          --proto_path=./
          --proto_path=$PROTOBUF_HOME/src
          --proto_path=$OSPL_HOME/include/protobuf
          ./address.proto

The generated code, in the ``./generated`` directory, can be
compiled normally with the Java compiler together with your own
written applications.

The only pre-requisite is that ``$OSPL_HOME/jar/dcpssaj5.jar`` and
``$OSPL_HOME/jar/dcpsprotobuf.jar`` are in the classpath so that the
Java compiler can find the included OpenSplice ``jar`` files.

This example is delivered with OpenSplice, and is located in
``examples/protobuf/java5``.

If the generated ``.idl`` file is needed by other applications, this file
will also be generated in the ``--ddsjava_out`` path if the environment
variable ``OSPL_PROTOBUF_INCLUDE_IDL`` is set to true.

C++ example
===========
|cpp|

In this example we assume that a correct OpenSplice environment is
set (so release.bat has been run)
For creating the DDS specific code by the GPB compiler the option
--ddscpp_out must be given to the compiler. Also the path to the
OpenSplice GPB-compiler must be given.
Example::

   protoc --cpp_out     =outputPath
          --ddscpp_out  =outputPath
          --proto_path  =PathToProtoFile
          --proto_path  =PathToProtoSelf
          --proto_path  =PathToOpenSpliceProtoCompiler
          protoFileToCompile

..

  - ``--cpp_out`` gives the path where the GDP generated code will be stored.
  - ``--ddscpp_out`` gives the path where the DDS-specific generated code will be stored.
  - first ``--proto_path``: the protoc compiler needs the path where the ``.proto`` file is located.
  - second ``--proto_path``: the path where the GPB environment is installed on your local machine.
  - third ``--proto_path``: specifies the path to the OpenSplice proto descriptor.
    This is normally ``$OSPL_HOME/include/protobuf``.
  - ``protoFileToCompile`` the last option is the ``.proto`` file.

Assuming that we need the generated code in the ``./generated`` directory
and the previous ``address.proto`` example is in the current directory,
the command will be::

   protoc --cpp_out=./generated
          --ddscpp_out=./generated
          --proto_path=./
          --proto_path=$PROTOBUF_HOME/src
          --proto_path=$OSPL_HOME/include/protobuf
          ./address.proto

The generated code, in the ``./generated`` directory, can be
compiled normally with the C++ compiler together with your own
written applications.

This example is delivered with OpenSplice, and is located in
``examples/protobuf/isocpp2``.

If the generated ``.idl`` file is needed by other applications, this file
will also be generated in the ``--ddscpp_out`` path if the environment
variable ``OSPL_PROTOBUF_INCLUDE_IDL`` is set to true.


Tempory IDL file created by the GPB data-model
==============================================

The IDL file created for the previous example will contain:

.. code-block:: idl

   module org {
      module omg {
         module dds {
            module protobuf {
               typedef sequence<octet> gpb_payload_t;
            };
         };
      };
   };

   module address {
       module dds {
           struct Person {
               string name;
               long age;
               string worksFor_name;
               string worksFor_address;
               ::org::omg::dds::protobuf::gpb_payload_t ospl_protobuf_data;
           };
           #pragma keylist Person name worksFor_name
       };
   };


This idl file is deleted after the idl-pp compiler is finished. If the
temporary idl file is needed in other DDS applications (it also usable for
other DDS vendors), then the environment variable ``OSPL_PROTOBUF_INCLUDE_IDL``
must be set to true to prevent the idl file from being deleted.


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

