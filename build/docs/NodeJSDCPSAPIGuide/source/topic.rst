.. _`Topic Generation and Discovery`:

##############################
Topic Generation and Discovery
##############################

A DDS Topic represents the unit for information that can be produced or consumed by a DDS application. Topics are defined by a name, a type, and a set of QoS policies. 

The Node.js DCPS API provides several ways of generating Node.js classes to represent DDS topics.

    - over the wire discovery
    - dynamic generation of Node.js classes using parameters IDL file and topic name    

.. note:: 

    - The :ref:`Examples` section provides the examples directory location, example descriptions and running instructions.


Over the Wire Discovery
************************

Node.js topic classes can be generated for existing DDS topics in the DDS system. These topics are "discovered over the wire".

The Node.js classes are generated when the topic is requested by name.   

A code snippet is provided from findTopicExample.js. This example finds a topic registered by another process, and writes a sample to that topic.

**Example**

.. code-block:: javascript

    ...
    const dds = require('vortexdds');

    console.log('Connecting to DDS domain...');
    const participant = new dds.Participant();
    
    console.log('Finding topic...');
    let topic = participant.findTopic('HelloWorldData_Msg');

    console.log('Creating writer and sample data to write...');    
    let writer = participant.createWriter(topic);
    let sample = { userID: 4, message: 7 };

    console.log('Writing sample data...');
    let status = writer.write(sample);
    ...


Dynamic Generation of Node.js Topic Classes Using IDL and Name
**************************************************************

The Node.js DCPS API supports generation of Node.js topic classes from IDL. This section describes the details of the IDL-Node.js binding.

Dynamic Generation
==================

The Node.js DCPS API provides an asynchronous function that returns a Map of ``TypeSupport`` objects.

A ``TypeSupport`` object includes the topic typename, keys and descriptor.    

The structure type representation of a topic is created by the TypeSupport object. However, the usage
of the structure type is internal to the Node.js DCPS API.

In order to create a topic, a topic name and a ``TypeSupport`` are passed into the ``Participant`` createTopicFor function.  (qos and listener parameters are optional)

The code snippet below is taken from the 'IoTTopicHelper.js' file referred in :ref:`IoTData` example.

.. code-block:: javascript

    const dds = require('vortexdds');
    const path = require('path');

    //asynchronous function to create the topic
    module.exports.create = async function(participant) {

      const topicName = 'IoTData';
      const idlName = 'dds_IoTData.idl';
      const idlPath = path.resolve(idlName);

      //wait for dds.getTopicTypeSupportsForIDL to return a map of typeSupports
      let typeSupports = await dds.getTopicTypeSupportsForIDL(idlPath);

      //idl contains 1 topic.
      let typeSupport = typeSupports.get('DDS::IoT::IoTData');

      return participant.createTopicFor(topicName, typeSupport); 
    };

Generated Artifacts
===================

The following table defines the Node.js artifacts generated from IDL concepts:

===========  ====================================== 
IDL Concept  Node.js Concept  
===========  ======================================  
module       N/A          
enum         enum from npm 'enum' package           
enum value   enum value      
struct       object           
field        object property 
union        object (IoTValue from dds_IoTData.idl
             is the only supported union)           
===========  ====================================== 

**Datatype mappings**

The following table shows the Node.js equivalents to IDL primitive types:

=========== ==============
IDL Type    Node.js Type
=========== ==============
boolean     Boolean
char        Number
octet       Number
short       Number
ushort      Number
long        Number
ulong       Number
long long   Number
ulong long  Number
float       Number
double      Number
string      String
wchar       Unsupported
wstring     Unsupported
any         Unsupported
long double Unsupported
=========== ==============


**Implementing Arrays and Sequences in Node.js**

Both IDL arrays and IDL sequences are mapped to JavaScript arrays.


.. _Limitations:

Limitations of Node.js Support
******************************

The Node.js binding has the following limitations:

    - Listeners are not supported

    - Only the IoTValue union from dds_IoTData.idl is supported in the beta

    - JavaScript does not currently include standard support for 64-bit integer values.  64-bit integers with more than 53 bits of data are represented by String values to avoid loss of precision. If the value will fit inside a JavaScript Number without losing precision, a Number can be used, otherwise use a String.  (Refer to :ref:`IoTData` example which demonstrates the usage and ranges for the unsigned and signed 64 bit integers within nodejs.)  




