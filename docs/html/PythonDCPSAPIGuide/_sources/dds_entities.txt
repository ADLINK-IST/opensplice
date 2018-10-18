.. _`Python API for Vortex DDS`:

#########################
Python API for Vortex DDS 
#########################

The Python DCPS API provides users with Python classes to model DDS communication using Python and pure
DDS applications.

The Python DCPS API consists of 2 modules.

* dds module
* ddsutil module

This section provides an overview of the main DDS concepts and Python API examples for these DDS concepts.

.. note:: 

    - The Python DCPS API can be found in the following directory:

          *$OSPL_HOME/tools/python/docs/html*

API Usage Patterns
******************

The typical usage pattern for the Python DCPS API for Vortex DDS is the following:

* optional - Model your DDS topics using IDL.  Alternatively, the python scripts can use topics that already exist in the DDS system.
* optional - Statically generate Python topic classes using ``idlpp -l python``.  If using IDL, the user also has the option to dynamically generate Python topic classes in python scripts.  See :ref:`Python Generation from IDL`.
* Start writing your Python program using the Python API for Vortex DDS.

The core classes you must use are ``dds.Topic``, ``dds.DomainParticipant`` and either ``dds.DataReader`` or ``dds.DataWriter``.
Other classes may be required, especially if you need to adjust the Quality of Service (QoS) defaults.
For details on setting QoS values with the API, see :ref:`QoS Provider`.

The following list shows the sequence in which you would use the Vortex classes:

* Create a ``dds.DomainParticipant`` instance. 
* Create one or more ``dds.Topic`` instances for the IDL topics your program will read or write.
* If you require publisher or subscriber level non-default QoS settings, create ``dds.Publisher`` and/or ``dds.Subscriber`` instances. (The most common reason for changing publisher/subscriber QoS is to define non-default partitions.)
* Create ``dds.DataReader`` and/or ``dds.DataWriter`` classes using the ``dds.Topic`` instances that you created.
* If you required data filtering, create ``dds.QueryCondition`` objects.
* Create the core of program, creating instances of your topic classes and writing them; or, reading data and processing it.

dds.Topic
*********

The Python Topic class represents a DDS topic type.  The DDS topic corresponds to a single data type.  In DDS, data is distributed by publishing and subscribing topic data samples.

For a DDS Topic type definition, a corresponding Python class must be defined.
These topic classes are either a class created statically using ``idlpp``, dynamically using an idl file or dynamically for an existing topic discovered in the system.  (see :ref:`Topic Generation and Discovery`)

**API Examples**

Create a Vortex DDS topic named 'Msg1' based on the DDS Topic type ``Msg``.

.. code-block:: python

    TOPIC_NAME = 'Msg1'
    TOPIC_TYPE = 'HelloWorldData::Msg'
    IDL_FILE = 'idl/HelloWorldData.idl'
    
    # Create domain participant
    dp = DomainParticipant()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl(IDL_FILE, TOPIC_TYPE)

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), 
        ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, TOPIC_NAME, qos) 

dds.DomainParticipant
*********************

The Python``dds.DomainParticipant`` class represents a DDS domain participant entity. 
  
In DDS - “A domain participant represents the local membership of the application in a domain.  A domain is a distributed concept that links all the applications able to communicate with each other.  It represents a communication plane: only the publishers and subscribers attached to the same domain may interact.”

The dds.DomainParticipant has two optional parameters on creation.  If these parameters are not provided, defaults are used.

Parameters:	
    
* qos (Qos) – Participant QoS. Default: None
* listener (Listener) – Participant listener Default: None


**API Examples**

Create a Vortex DDS domain participant. Returns participant or throws a ``dds.DDSException`` if the participant cannot be created.

Create a domain participant in the default DDS domain (the one specified by the OSLP_URI environment variable).

.. code-block:: python

    # Create domain participant
    dp = DomainParticipant()
    
Create a participant on default domain with QoS profile.
    
.. code-block:: python
       
    qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')

    # Create participant
    dp = DomainParticipant(qos = qp.get_participant_qos())        
      
Create a participant on domain with QoS profile and listener.  TODO
    
.. code-block:: python
       
    qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')

    # Create participant
    dp = DomainParticipant(qos = qp.get_participant_qos())

dds.Publisher
*************

The Python ``dds.Publisher`` class represents a DDS publisher entity.

In DDS, a publisher is “an object responsible for data distribution.  It may publish data of different data types.”

Use of the ``dds.Publisher`` class is optional.
In it's place, you can use a ``dds.DomainParticipant`` instance.
Reasons for explicitly creating a ``dds.Publisher`` instance are:

* to specify non-default QoS settings, including specifying the DDS *partition* upon which samples are written.
* to control the timing of publisher creation and deletion.

**API Examples**

Create a DDS Publisher entity. Returns publisher or throws a ``dds.DDSException`` if the publisher cannot be created.

Create a publisher with participant.
    
.. code-block:: python
       
    # Create participant
    dp = DomainParticipant()

    # Create publisher
    pub = dp.create_publisher()
        
Create a publisher with participant and QoS profile.
    
.. code-block:: python
       
    qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')

    # Create participant
    dp = DomainParticipant(qos = qp.get_participant_qos())

    # Create publisher
    pub = dp.create_publisher(qos = qp.get_publisher_qos())
    
dds.DataWriter
**************

The Python ``dds.Writer`` class represents a DDS data writer entity.

In DDS - “The DataWriter is the object the application must use to communicate to a publisher the existence and value of data-objects of a given type.”

A ``dds.DataWriter`` class is required in order to write data to a DDS domain.
It is attached to a DDS publisher or a DDS domain participant.

A ``dds.DataWriter`` class instance references an existing ``dds.Topic`` instance.

**API Examples**

Create a Vortex DDS domain writer. Returns writer or throws a ``dds.DDSException`` if the writer cannot be created.
    
Create a writer within a domain participant, and with default QoS.

.. code-block:: python
    
    # Create domain participant
    dp = DomainParticipant()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl('idl/HelloWorldData.idl', 'HelloWorldData::Msg')

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, 'Msg1', qos)  
 
    # Create a writer
    writer = dp.create_datawriter(topic)

Create a writer within a publisher, and with default QoS.

.. code-block:: python

    # Create domain participant
    dp = DomainParticipant()

    # Create publisher
    pub = dp.create_publisher()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl('idl/HelloWorldData.idl', 'HelloWorldData::Msg')

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, 'Msg1', qos)  
 
    # Create a writer
    writer = pub.create_datawriter(topic)
     
Create a writer with publisher or participant, topic and QoS profile.
    
 .. code-block:: python

    # Create domain participant
    dp = DomainParticipant()

    # Create publisher
    pub = dp.create_publisher()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl('idl/HelloWorldData.idl', 'HelloWorldData::Msg')

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, 'Msg1', qos)  
 
    # Create a writer
    qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')
    writer = pub.create_datawriter(topic)
    writer = pub.create_datawriter(topic, qp.get_writer_qos())

Write a Msg topic class instance to a writer.

.. code-block:: python

    # Topic data class
    idMessage = 1
    message1 = 'Hello World' 
    s = gen_info.topic_data_class(userID = idMessage, message = message1)
    
    # Write data
    writer.write(s)

Dispose a DDS topic instance.

.. code-block:: python

    # dispose instance
    writer.dispose_instance(s)

Unregister a DDS topic instance.  TODO no equivalent in Python API

   % writer: a Vortex.Writer instance
   % ShapeType: a 'topic class' created manually or via IDLPP
   data = ShapeType(); % create an object instance
   % set data key values...
   data.color = 'RED';

   ddsStatus = writer.unregister(data);

dds.Subscriber
**************

The Python ``dds.Subscriber`` class represents a DDS subscriber entity.

In DDS, a subscriber is “an object responsible for receiving published data and making it available to the receiving application.  It may receive and dispatch data of different specified types.”

Use of the ``dds.Subscriber`` class is optional.
In it's place, you can use a ``dds.DomainParticipant`` instance.
Reasons for explicitly creating a ``dds.Subscriber`` instance are:

* to specify non-default QoS settings, including specifying the DDS *partition* upon which samples are written.
* to control the timing of subscriber creation and deletion.

**API Examples**


Create a Vortex DDS domain subscriber. Returns subscriber or throw a ``dds.DDSException`` if the subscriber cannot be created.
    
    
Create a subscriber with participant.
    
.. code-block:: python
       
    # Create participant
    dp = DomainParticipant()

    # Create Subscriber
    sub = dp.create_subscriber()    

Create a subscriber with participant and QoS profile.
    
.. code-block:: python
       
    qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')

    # Create participant
    dp = DomainParticipant(qos = qp.get_participant_qos())

    # Create Subscriber
    sub = dp.create_subscriber(qos = qp.get_subscriber_qos())
    

dds.DataReader
**************

The Python ``Vortex.Reader`` class represents a DDS data reader entity.

In DDS - “To access the received data, the application must use a typed DataReader attached to the subscriber.”

A ``dds.DataReader`` class is required in order to write data to a DDS domain.
It is attached to a DDS subscriber or a DDS domain participant.

A ``dds.DataReader`` class instance references an existing ``dds.Topic`` instance.

**API Examples**

Create a Vortex DDS domain reader. Returns reader or throw a ``dds.DDSException`` instance if the reader cannot be created.
    
Create a reader for a topic within a participant, and with default QoS.

.. code-block:: python
    
    # Create domain participant
    dp = DomainParticipant()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl('idl/HelloWorldData.idl', 'HelloWorldData::Msg')

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, 'Msg1', qos)  
    
    # Create a reader
    reader = dp.create_datareader(topic)

Create a reader for a topic within a subscriber, and with default QoS.
    
.. code-block:: python
    
    # Create domain participant
    dp = DomainParticipant()

    # Create subscriber
    sub = dp.create_subscriber()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl(IDL_FILE, TOPIC_TYPE)

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, TOPIC_NAME, qos)  
    
    # Create a reader
    reader = sub.create_datareader(topic)
      
Create a reader for a topic within a subscriber or participant, with with a QoS profile.
    
.. code-block:: python
    
    # Create domain participant
    dp = DomainParticipant()

    # Create subscriber
    sub = dp.create_subscriber()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl(IDL_FILE, TOPIC_TYPE)

    # Type support class
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT), ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, TOPIC_NAME, qos)  
    
    # Create a reader
    qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')
    reader = sub.create_datareader(topic, qp.get_reader_qos())

Take data from a data reader.

.. code-block:: python

     l = reader.take(10)

Read data from a data reader.

.. code-block:: python

     l = reader.read(10)

Specify a wait timeout, in seconds, before read or take will return without receiving data TODO change description the code is wrong

.. code-block:: python

    # Create waitset
    waitset = WaitSet()
    qc = QueryCondition(reader, DDSMaskUtil.all_samples(), 'long1 > 1')

    waitset.attach(qc)

    # Wait for data
    conditions = waitset.wait()

    # Print data
    l = reader.take(10)

dds.QueryCondition
******************

The Python ``dds.QueryCondition`` class represents a DDS query entity.

A query is a data reader, restricted to accessing data that matches specific status conditions and/or a filter expression.

A ``dds.Query`` class instance references an existing ``dds.DataReader`` instance.

**API Examples**

Create a ``dds.QueryCondition`` with a state mask and a filter expression for a reader and take data.

.. code-block:: python

    # Create waitset
    waitset = WaitSet()
    qc = QueryCondition(reader, DDSMaskUtil.all_samples(), 'long1 > 1')

    waitset.attach(qc)

    # Wait for data
    conditions = waitset.wait()

    # Print data
    l = reader.take(10)
    for sd, si in l:
        sd.print_vars()


