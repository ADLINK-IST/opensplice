.. _`Command Line Options`:


####################
Command Line Options
####################

The Vortex OpenSplice IDL Pre-processor, ``idlpp``, can be run with the following
command line options:

::

   [ -h ]
   [ -b <ORB-template-path> ]
   [ -n <include-suffix> ]
   [ -I <path> ]
   [ -D <macro>[=<definition>] ]
   < -S | -C >
   < -l (c | c++ | cpp | java | cs | isocpp | isoc++ | c99) >
   [ -F ]
   [ -j [old]:<new>]
   [ -o <dds-types> | <custom-psm> | <no-equality> | <deprecated-c++11-mapping>]
   [ -d <output-directory> ]
   [ -P <dll_macro_name>[,<header_file>] ]
   [ -N ]
   <filename>



+  Options shown between angle brackets, ``<`` and ``>``, 
   are mandatory. 
+  Options shown between square brackets, ``[`` and ``]``, 
   are optional.

All of these options are described in full detail below. 

**-h**
   List the command line options and information.

**-b <ORB-template-path>**
   Specifies the ORB-specific path within the 
   template path for the specialized class templates (in case the template 
   files are ORB specific). The ORB-specific template path can also be set 
   *via* the environment variable ``OSPL_ORB_PATH``, the command line option 
   is however leading. To complete the path to the templates, the value of 
   the environment variable ``OSPL_TMPL_PATH`` is prepended to the ORB path.

**-n <include-suffix>**
   Overrides the suffix that is used to identify the 
   ORB-dependent header file (specifying the data model) that needs to be included.
   Normally the name of this include file is derived from the IDL file name and
   followed by an ORB-dependent suffix (*e.g.* ``C.h`` for ACE-TAO based ORBs).
   This option is only supported in CORBA cohabitation mode for C++; in all
   other cases it is simply ignored.
   Example usage: ``-n .stub.hpp``
   (For a file named ``foo.idl`` this will include ``foo.stub.hpp`` instead of
   ``fooC.h``, which is the default expectation for ACE-TAO.)

**-I <path>** 
   Passes the include path directives to the C pre-processor.

**-D <macro>**
   Passes the specified macro definition to the C pre-processor.

**-S**
   Specifies standalone mode, which allows application programs to be 
   built and run without involvement of any ORB. The name space for standard 
   types will be DDS instead of the name space implied by the IDL language mapping.
   types will be DDS instead of the name space implied by the IDL language mapping.

**-C**
   Specifies ORB integrated mode, which allows application programs to 
   be built and run integrated with an ORB.

**-l (c | c++ | cpp | java | cs | isocpp | isoc++ | isocpp2 | isoc++2 | c99)** 
   Selects the target language. 
   Note that the Vortex OpenSplice IDL Pre-processor does not support
   every combination of modes and languages. This option is mandatory; when no
   language is selected the OpenSplice IDL Pre-processor reports an error.
   
   - For the *c*, *c++*, *cpp*, *java* and *cs* target languages the types 
     will default to the standard types. For the *isocpp* and *isoc++* target 
     languages the types will default to the ISOC++ types that comply with 
     the ISO/IEC C++ 2003 Language DDS PSM. When using *isocpp*, *isoc++* 
     *isocpp2* or *isoc++2* an equality operator will also be generated for
     types unless this feature is explicitly disabled.
   - Please note that *isocpp* and *isoc++* target languages are DEPRECATED
     since V6.6.0. Please use *isocpp2* or *isoc++2* instead.
   - For the Standalone mode in C (when using the ``-S`` flag and the ``c`` 
     language option), ``OSPL_ORB_PATH`` will by default be set to value SAC, 
     which is the default location for the standalone C specialized class 
     template files.
   - For the CORBA cohabitation mode in C++ (when using the ``-C`` flag and 
     the ``c++`` or ``cpp`` language option) the ``OSPL_ORB_PATH`` will, 
     by default, be set to:

     |unix|
       ``CCPP/DDS_OpenFusion_1_6_1`` for Unix-based platforms.

     |windows|
       ``CCPP\DDS_OpenFusion_1_6_1`` for Windows platforms.

     These are the default locations for the IDL to C++ specialized class 
     template files of the OpenSplice-Tao ORB. Class templates for other 
     ORBS are also available in separate sub-directories of the CCPP 
     directory, but for more information about using a different ORB, 
     consult the ``README`` file in the ``custom_lib/ccpp`` directory.
   - For the Standalone mode in C++ (when using the ``-S`` flag and 
     the ``c++`` or ``cpp`` language option), ``OSPL_ORB_PATH`` will 
     by default be set to value ``SACPP``, which is the default location 
     for the standalone C++ specialized class template files.

     |java|
   - For the Standalone mode in Java (when using the ``-S`` flag and the 
     ``java`` language option), ``OSPL_ORB_PATH`` will by default be set 
     to the value of ``SAJ``, which is the default location for the 
     standalone Java specialized class template files.
   - For the CORBA cohabitation mode in Java (when using the ``-C`` flag 
     and the ``java`` language option), ``OSPL_ORB_PATH`` will by default 
     be set to the value of ``SAJ``, which is the default location for 
     the CORBA Java specialized class template files. This means that 
     the CORBA cohabitated Java API and StandAlone Java API share the 
     same template files.

     |csharp|
   - For the Standalone mode in C# (when using the ``-S`` flag and the 
     ``cs`` language option), ``OSPL_ORB_PATH`` will by default be set to 
     the value of ``SACS``, which is the default location for the 
     standalone CSharp specialized class template files.
     
     |c99|
   - For the c99 target language the types will default to the standard
     types. Except that the primitive types are mapped to the corresponding
     c99 types and that bound strings are mapped to char arrays with a
     size one larger than specified in the idl definition to allow for
     the terminating 0 character.

   See also
   :ref:`OpenSplice Modes and Languages <OpenSplice Modes and Languages>`
   for a complete list of supported modes and languages.

**-F**
   Specifies FACE mode, generate FACE API type specific functions in addition to 
   the target language specific ones. *Only applicable for the java and isocpp2 
   target languages.*

|java|

**-j [old]:<new>**
   Specifies that the (partial) package name which matches *[old]* will 
   be replaced by the package name which matches *<new>* (the package 
   *<new>* is substituted for the package *[old]*). If *[old]* is not 
   included then the package name defined by *<new>* is prefixed to
   all Java packages. The package names may only be separated by 
   ``.`` (period) characters.
   A trailing ``.`` character is not required, but may be used.
   Example: ``-j :org.opensplice`` (prefixes all Java packages).
   Example: ``-j com.opensplice.:org.opensplice.`` (substitutes).
   *Only applicable for the Java language.* 

**-o dds-types**
   Enables the built-in DDS data types. 
   In the default mode, the built-in DDS data types are not available 
   to the application IDL definitions. When this option is activated, 
   the built-in DDS data types will become available. 
   Refer to Section 1.9, Built-in DDS data types, on page 28.

**-o custom-psm**
   Enables support for alternative IDL language mappings.
   Currently CSharp offers an alternative language mapping where 
   IDL names are translated to their PascalCase representation and 
   where ``@`` instead of ``_`` is used to escape reserved C#-keywords.

**-o no-equality**
   Disables support for the automatically-generated 
   equality operator on ISOC++ types.

**-o deprecated-c++11-mapping**
   Generates the ISOC++2 types using the deprecated C++11 mapping
   implementation as used in the past by the also deprecated
   isocpp/isoc++ PSM. This option only makes sense when migrating
   from isocpp/isoc++ to isocpp2/isoc++2.

**-d <output-directory>**
   Specifies the output directory for the generated code.

**-P <dll_macro_name>[,<header_file>]**
   This option controls the signature for every external function/class 
   interface. If you want to use the generated code for creating a DLL, 
   then interfaces that need to be accessible from the outside need to 
   be exported. When accessing these operations outside of the DLL, then 
   these external interfaces need to be imported. 
   If the generated code is statically linked, this option can be omitted.
   The first argument *<dll_macro_name>* specifies the text that is prepended 
   to the signature of every external function and/or class. 
   For example: defining DDS_API as the macro, the user can define this macro 
   as ``__declspec(dllexport)`` when building the DLL containing the generated
   code, and define the macro as ``__declspec(dllimport)`` when using the DLL
   containing the generated code.

   Addtionally a header file can be specified, which contains controls to 
   define the macro. For example the external interface of the generated code 
   is exported when the macro ``BUILD_MY_DLL`` is defined, then this file 
   could look like:

::
   
   #ifdef BUILD_MY_DLL
   #define DDS_API __declspec(dllexport)
   #else /* !BUILD_MY_DLL */
   #define DDS_API __declspec(dllimport)
   #endif /* BUILD_MY_DLL */


|c| |cpp|

**-N**
   This option disables type caching in the copy-in routines. 
   The copy-in routines cache the type to improve the performance 
   of copying sequences. This option disables this feature to allow 
   the use of sequences within multi-domain applications. 
   *Only applicable for the C and C++ languages.*

**<filename>**
   Specifies the IDL input file to process.




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
