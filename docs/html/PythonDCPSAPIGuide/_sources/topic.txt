.. _`Topic Generation and Discovery`:

##############################
Topic Generation and Discovery
##############################

A DDS Topic represents the unit for information that can be produced or consumed by a DDS application. Topics are defined by a name, a type, and a set of QoS policies. 

The Python DCPS API provides several ways of generating Python classes to represent DDS topics.

    - over the wire discovery
    - dynamic generation of Python Topic classes using parameters IDL file and topic name
    - static generation of Python Topic classes using IDL

.. note:: 

    - The :ref:`Examples` section provides the examples directory location, example descriptions and running instructions.


Over the Wire Discovery
************************

Python topic classes can be generated for existing DDS topics in the DDS system.   These topics are "discovered over the wire".

The Python classes are generated when the topic is requested by name.   

A code snippet is provided from example4.py.  This example finds a topic registered by another process, and reads and writes samples to that topic.

**example4.py**

.. code-block:: python

    ...
    print('Connecting to DDS domain...')
    dp = dds.DomainParticipant()
    
    print('Finding OsplTestTopic...')
    found_topic = dp.find_topic('OsplTestTopic')
    
    print('Registering OsplTestTopic locally')
    local_topic = ddsutil.register_found_topic_as_local(found_topic)
    
    print('Getting Python classes for the found topic...')
    gen_info = ddsutil.get_dds_classes_for_found_topic(found_topic)
    OsplTestTopic = gen_info.get_class(found_topic.type_name)
    Tstate = gen_info.get_class('ospllog::Tstate')

    print('Creating sample data to write...')    
    data = OsplTestTopic(id=11,index=22, x=1.2, y=2.3, z= 3.4, t=9.8, 
        state=Tstate.init, description='Hello from Python')

    print('Creating readers and writers...')    
    pub = dp.create_publisher()
    wr = pub.create_datawriter(local_topic, found_topic.qos)
    sub = dp.create_subscriber()
    rd = sub.create_datareader(local_topic, found_topic.qos)
    
    print('Writing sample data...')
    wr.write(data)
    print('Wrote: %s' % (str(data)))
    ...

Static Generation of Python Topic Classes Using IDL
***************************************************

The Python topic classes can be generated statically using an IDL file.  Please see :ref:`StaticGeneration` for more information.

Dynamic Generation of Python Topic Classes Using IDL and Name
*************************************************************

The Python topic classes can be generated dynamically using an IDL file, and the topic name.  Please see :ref:`DynamicGeneration` for more information.




