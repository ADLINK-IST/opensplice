.. _`QoS Provider`:


########################
Quality of Service (QoS)
########################

The following section explains how to set the Quality of Service (QoS) for a DDS entity.

Users have two options available to set the QoS for an entity or entities.  They can define the QoS settings using an XML file, or they can use the Python DCPS APIs.  Both of these options are explained.

If a QoS setting for an entity is not set using an xml file or the Python DCPS APIs, the defaults will be used. This allows a user the ability to override only those settings that require non-default values.

The code snippets referenced are taken from the runnable examples.

.. note:: 

    - The :ref:`Examples` section provides the examples directory location, example descriptions and running instructions.


Setting QoS Using QoS Provider XML File
***************************************

QoS for DDS entities can be set using XML files based on the XML schema file QoSProfile.xsd_. 
These XML files contain one or more QoS profiles for DDS entities. 

Sample QoS Profile XML files can be found in the examples directory.

QoS Profile
===========

A QoS profile consists of a name and optionally a base_name attribute. The base_name attribute allows a 
QoS or a profile to inherit values from another QoS or profile in the same file. The file contains QoS 
elements for one or more DDS entities. A skeleton file without any QoS values is displayed below to show 
the structure of the file.

.. code-block:: xml
    
    <dds xmlns="http://www.omg.org/dds/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="file:DDS_QoSProfile.xsd">
        <qos_profile name="DDS QoS Profile Name">
            <datareader_qos></datareader_qos>
            <datawriter_qos></datawriter_qos>
            <domainparticipant_qos></domainparticipant_qos>
            <subscriber_qos></subscriber_qos>
            <publisher_qos></publisher_qos>
            <topic_qos></topic_qos>
        </qos_profile>
    </dds>

**Example: Specify Publisher Partition**

The example below specifies the publisher's partitions as A and B.

.. code-block:: xml

    <publisher_qos>
        <partition>
            <name>
                <element>A</element>
                <element>B</element>
            </name>
        </partition>
    </publisher_qos>
    
Applying QoS Profile 
====================

To set the QoS profile for a DDS entity using the Python DCPS API and an XML file, the user specifies the File URI and the QoS profile name as parameters.

**example1.py**

.. code-block:: python
       
       ...
       qp = QosProvider('file://DDS_DefaultQoS_All.xml', 'DDS DefaultQosProfile')

       # Create participant
       dp = DomainParticipant(qos = qp.get_participant_qos())

       # Create publisher
       pub = dp.create_publisher(qos = qp.get_publisher_qos())

       # Create Subscriber
       sub = dp.create_subscriber(qos = qp.get_subscriber_qos())
       ...


Setting QoS Using Python DCPS API Classes
*****************************************

QoS settings can also be set by using the python classes alone.  (No XML files required.) 

Below is a code snippet that demonstrates how to specify the QoS settings for a writer using the python DCPS apis.  In this example, all the QoS settings for the writer are set and all of the default QoS settings are overridden.  If a QoS setting for an entity is not set, the default is used.

**qos_example.py**

.. code-block:: python

       ...
       writer_qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
                      DeadlineQosPolicy(DDSDuration(500)),
                      LatencyBudgetQosPolicy(DDSDuration(3000)),
                      LivelinessQosPolicy(DDSLivelinessKind.MANUAL_BY_PARTICIPANT),
                      ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE, DDSDuration.infinity()),
                      DestinationOrderQosPolicy(DDSDestinationOrderKind.BY_SOURCE_TIMESTAMP),
                      HistoryQosPolicy(DDSHistoryKind.KEEP_ALL),
                      ResourceLimitsQosPolicy(10,10,10),
                      TransportPriorityQosPolicy(700),
                      LifespanQosPolicy(DDSDuration(10, 500)),
                      OwnershipQosPolicy(DDSOwnershipKind.EXCLUSIVE),
                      OwnershipStrengthQosPolicy(100),
                      WriterDataLifecycleQosPolicy(False)
                      ])
       ...


In the next example, a topic QoS is created that overrides only a subset of the QoS settings.

**HelloWorldDataSubscriber.py**

.. code-block:: python

    ...
    # Create domain participant
    dp = DomainParticipant()

    # Create subscriber
    sub = dp.create_subscriber()

    # Generate python classes from IDL file
    gen_info = ddsutil.get_dds_classes_from_idl(IDL_FILE, TOPIC_TYPE)

    # Create a topic QoS that overrides defaults for durability and reliability
    qos = Qos([DurabilityQosPolicy(DDSDurabilityKind.TRANSIENT),
           ReliabilityQosPolicy(DDSReliabilityKind.RELIABLE)])
    topic = gen_info.register_topic(dp, TOPIC_NAME, qos)  
    ...


.. external links
.. _QoSProfile.xsd: http://www.omg.org/spec/dds4ccm/20110201/DDS_QoSProfile.xsd
.. _DDS_DefaultQoS.xml: http://www.omg.org/spec/dds4ccm/20110201/DDS_DefaultQoS.xml
