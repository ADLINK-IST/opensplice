.. _`Appendix A`:

##########
Appendix A
##########

Appendix A provides a description of the Simulink Bus to DDS Mapping implementation.

Simulink Bus to DDS Mapping
***************************

Simulink data is represented in buses whose types are not compatable with DDS types. 
Therfore sending Simulink data to DDS requires a conversion from Simulink types to DDS types. 
Conversly sending DDS data to Simulink requires a conversion from DDS types to Simulink types.
This document describes the mapping between these types, the type descriptors generated for Topic
registration and any annotations needed to describe keys, namespaces, ... for two different workflows.



Workflow 1: Using idlpp to Generate Simulink Bus and Type Descriptor
====================================================================

DDS Topic Types are described in IDL. To be compatible with DDS, these types must be 
represented in Sumlink as Bus Types. On writes, these Simulink Bus types are converted 
to DDS types and on read DDS types are converted to Simulink Bus types.

IDL can be defined in an IDL file and the corresponding Simulink Bus Types can be created 
by ``Vortex.idlImportSl`` at the MATLAB command line. This will create the necessary Simulink Bus 
Types and Elements in a Simulink dictionary. It also creates the XML type descriptor for
the Simulink Bus types that is used to create the Topic. The type descriptor is a Simulink variable
of the form <bus-name>TypeDescriptor. The type descriptor is necessary for the block set to work correctly,
but you will not directly address this when creating Simulink blocks.


**DDS IDL to Simulink Bus Mapping**

The table below describes the generated Simulink artifacts when ``Vortex.idlImportSl`` is invoked from MATLAB.


.. table:: 

    +--------------------------+----------------------------+----------------+--------------------------------+
    |**DDS IDL**               |**Simulink Type**           |**Annotation**  |**Comments**                    |
    +==========================+============================+================+================================+
    |struct B                  |Simulink.Bus B              |                | Creates Simulink Bus           |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |enum E                    | Simulink Enum E            |                | Creates Simulink Enum type     |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |module A                  |                            |@Scope(A)       |Added to each enum or struct    |
    |                          |                            |                |contained in the module.        |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |#pragma keylist           |                            |@Key            |Every topic bus has @Key in its |
    |                          |                            |                |description.                    |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |boolean                   |boolean                     |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |char                      |int8                        |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |octet                     |uint8                       |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |short                     |int16                       |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |unsigned short            |uint16                      |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |long                      |int32                       |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |unsigned long             |uint32                      |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |long long                 |double                      |                |No int64 in Simulink. Copied    |
    |                          |                            |                |into the memory allocated for   |
    |                          |                            |                |a double.                       |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |unsigned long long        |double                      |                |No int64 in Simulink. Copied    |
    |                          |                            |                |into the memory allocated for   |
    |                          |                            |                |a double.                       |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |float                     |single                      |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |double                    |double                      |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |string                    |int8                        |@String         | Default dimension is 256       |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |string<N>                 | int8                       |@BString        |                                |
    |                          | Dimension N                |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |T field[N]                | Mapped type for T.         |                |Multidimensional arrays are     |
    |                          | Dimension N                |                |supported.                      |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |sequence<T,N>             | Bus: seqN_T.               |                |E.g. sequence<long,3> becomes   |
    |                          | Dimension: N               |                |"Bus: seq3_int32"               |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |sequence<T>               | Bus: seq_T                 |                |E.g. sequence<long> becomes     |
    |                          | Dimension: 16              |                |"Bus: seq_int32" with default   |
    |                          |                            |                |dimension of 16                 |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |typedef                   |                            |                |expanded in place               |
    +--------------------------+----------------------------+----------------+--------------------------------+
    | **Unsupported DDS data types**                                                                          |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |wchar                     |*not supported*             |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |wstring                   |*not supported*             |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |any                       |*not supported*             |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |long double               |*not supported*             |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |union                     |*not supported*             |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+
    |inheritance               |*not supported*             |                |                                |
    +--------------------------+----------------------------+----------------+--------------------------------+



Workflow 2: Manually Modeling DDS data in the Simulink Bus Editor
=================================================================

DDS IDL is not necessary to interact with DDS applicatoins.
You can also model the Simulink buses directly. 
In this case, the block set will infer the DDS data types from the Simulink types.

Defining Simulink busses without first defining the IDL is not recommended; it has the following limitations:

* fewer IDL concepts are supported. In particular, sequences are unsupported.
* it will be difficult for your Simulink application to interact with applications written in other languages, as those languages will required IDL to define the topic data.


**IDL-less mapping of Simulink bus IDL concepts**

The table below describes how a Simulink bus that was not created from an IDL file
is mapped to IDL concepts.

.. table:: 

    +-------------------------------+------------------+---------------------------------+
    |**Simulink Type**              |**IDL equivalent**|**Description**                  |
    +===============================+==================+=================================+
    |Simulink.Bus B                 |struct B          |Defines DDS topic type B         |
    +-------------------------------+------------------+---------------------------------+
    | bus annotation: @Scope(A::B)  |module            |Creates DDS namespace for struct |
    +-------------------------------+------------------+---------------------------------+
    |bus annotation: @Key(f1,f2)    |#pragma keylist   |Defines topic key field(s)       |
    +-------------------------------+------------------+---------------------------------+
    |boolean                        |boolean           |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |int8                           |char              |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |uint8                          |octet             |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |int16                          |short             |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |uint16                         |unsigned short    |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |int32                          |long              |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |uint32                         |unsigned long     |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |single                         |float             |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    |double                         |double            |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
    | | int8, annotated @String     |string            | | max length of read strings is |
    | | uint8, annotated @String    |                  | | determined by field dimension |
    +-------------------------------+------------------+---------------------------------+
    | | int8, annotated @BString    |string<N>         |                                 |
    | | uint8, annotated @BString   |                  |                                 |
    | | Dimension N                 |                  |                                 |
    +-------------------------------+------------------+---------------------------------+
    |Simulink enumeration, E        |enum E            |IDL array if Dimensions > 1      |
    +-------------------------------+------------------+---------------------------------+
 
 
