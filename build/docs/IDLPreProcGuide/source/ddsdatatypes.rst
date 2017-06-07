.. _`Built-in DDS data types`:


#######################
Built-in DDS data types
#######################

The Vortex OpenSplice IDL Pre-processor and the Vortex OpenSplice runtime system
support the following DDS data types to be used in application IDL definitions:

+ ``Duration_t``
+ ``Time_t``

When building C or Java application programs, no special actions have to be taken
other than enabling the Vortex OpenSplice IDL Pre-processor built-in DDS data types
using the ``-o dds-types`` option.

For C++, however, attention must be paid to the ORB IDL compiler, which is also
involved in the application building process. The ORB IDL compiler is not aware of
any DDS data types, so the supported DDS types must be provided by means of
inclusion of an IDL file (``dds_dcps.idl``) that defines these types. This file must
not be included for the Vortex OpenSplice IDL Pre-processor, which has the type
definitions built-in. Therefore ``dds_dcps.idl`` must be included conditionally. The
condition can be controlled *via* the macro definition ``OSPL_IDL_COMPILER``, which
is defined when the Vortex OpenSplice IDL Pre-processor is invoked, but *not* when
the ORB IDL compiler is invoked:

.. code-block:: xml
   
   #ifndef OSPL_IDL_COMPILER
   #include <dds_dcps.idl>
   #endif
   
   module example {
     struct example_struct {
       DDS::Time_ttime;
     };
   };


|caution|

The ORB IDL compiler must be called *with* the ``-I$OSPL_HOME/etc/idlpp``
option in order to define the include path for the ``dds_dcps.idl`` file. 
The Vortex OpenSplice IDL Pre-processor must be called *without* this option.




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
