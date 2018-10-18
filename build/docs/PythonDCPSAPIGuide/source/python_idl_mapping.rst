.. _`Python Generation from IDL`:

##########################
Python Generation from IDL
##########################

The Python DCPS API supports generation of Python topic classes from IDL. This chapter describes the details of the IDL-Python binding.

Running IDLPP
*************

.. _StaticGeneration:

Static Generation
=================

The Python topic classes can be generated statically using an IDL file.

Compiling IDL into python code is done using the ``-l python`` switch on idlpp::

    idlpp -l python idl-file-to-compile.idl

.. note:: 

    - A Python package with the same name as the idl file (without the .idl extension) is always created.

    - It defines types not included in an IDL module.  IDL modules become Python packages within this base package.

.. _DynamicGeneration:

Dynamic Generation
==================

The Python topic classes can be generated dynamically using an IDL file, and the topic name.   An api, ``ddsutil.get_dds_classes_from_idl``, is provided to generate the topic classes at runtime from within a Python script.

**HelloWorldDataPublisher.py**

.. code-block:: python

    ...
    TOPIC_NAME = 'Msg1'
    TOPIC_TYPE = 'HelloWorldData::Msg'
    IDL_FILE = 'idl/HelloWorldData.idl'

    # Create domain participant
    dp = DomainParticipant()

    # Create publisher
    pub = dp.create_publisher()

    # Generate python topic classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl(IDL_FILE, TOPIC_TYPE)

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
           ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])

    # Register topic
    topic = gen_info.register_topic(dp, TOPIC_NAME, qos)     
  
    # Create a writer
    writer = pub.create_datawriter(topic)
    
    # Topic data class
    idMessage = 1
    message1 = 'Hello World' 
    s = gen_info.topic_data_class(userID = idMessage, message = message1)
    
    # Write data
    writer.write(s)
    
    #output to console
    print('=== [Publisher] writing a message containing : ')
    print('userID  :',  idMessage)
    print('message :', message1)
    ...

Generated Artifacts
===================

The following table defines the Python artifacts generated from IDL concepts:

===========  ==============  ===============================================
IDL Concept  Python Concept  Comment
===========  ==============  ===============================================
module       package         Folder with module name, plus __init__.py that 
                             defines types defined within the module.
enum         class           Defined in __init__.py files.
enum value   enum value      Defined in __init__.py files.
struct       class           Defined in __init__.py files.
field        class property  Defined in __init__.py files.
union        union           Defined in __init__.py files. (Only statically
                             generated supported)
===========  ==============  ===============================================

**Datatype mappings**

The following table shows the Python equivalents to IDL primitive types:

=========== ==============
IDL Type    Python Type
=========== ==============
boolean     bool
char        str, length==1
octet       int
short       int
ushort      int
long        int
ulong       int
long long   int
ulong long  int
float       float
double      float
string      str
wchar       Unsupported
wstring     Unsupported
any         Unsupported
long double Unsupported
=========== ==============

**Implementing Arrays and Sequences in Python**

Both IDL arrays and IDL sequences are mapped to Python lists.

The constructors for generated classes always fully allocate any array fields. Sequences are always initialized to the empty list.

Limitations of Python Support
*****************************

The IDL-to-Python binding has the following limitations:

* IDL unions are Supported by statically generated Python, but not by dynamic or over-the-wire.
* The following IDL data types are not supported: wchar, wstring, any and long double . 

