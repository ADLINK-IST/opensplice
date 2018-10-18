.. _`QoS Provider`:


########################
Quality of Service (QoS)
########################

The following section explains how to set the Quality of Service (QoS) for a DDS entity.

Users have two options available to set the QoS for an entity or entities.  They can define the QoS settings using an XML file, or they can use the Node.js DCPS APIs.  Both of these options are explained.

If a QoS setting for an entity is not set using an xml file or the Node.js DCPS APIs, the defaults will be used. This allows a user the ability to override only those settings that require non-default values.

The code snippets referenced are taken from the runnable examples.

.. note:: 

    - The :ref:`Examples` section provides the examples directory location, example descriptions and running instructions.


Setting QoS Using QoS Provider XML File
***************************************

QoS for DDS entities can be set using XML files based on the XML schema file QoSProfile.xsd_. 
These XML files contain one or more QoS profiles for DDS entities. OSPL includes an XSD (XML schema), that is located in $OSPL_HOME/etc/DDS_QoSProfile.xml. This can be used with XML schema core editors to help create valid XML files.

Sample QoS Profile XML files can be found in the examples directory. Typically you will place the qos files in a subdirectory of your Node.js application.

.. _QoSProfile:

QoS Profile
===========

A QoS profile consists of a name and optionally a base_name attribute. The base_name attribute allows a 
QoS or a profile to inherit values from another QoS or profile in the same file. The file contains QoS 
elements for one or more DDS entities.

A skeleton file without any QoS values is displayed below to show the structure of the file.

.. code-block:: xml
    
    <dds xmlns="http://www.omg.org/dds/" 
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
     xsi:schemaLocation="file:DDS_QoSProfile.xsd">
        <qos_profile name="DDS QoS Profile Name">
            <datareader_qos></datareader_qos>
            <datawriter_qos></datawriter_qos>
            <domainparticipant_qos></domainparticipant_qos>
            <subscriber_qos></subscriber_qos>
            <publisher_qos></publisher_qos>
            <topic_qos></topic_qos>
        </qos_profile>
    </dds>


**Example: Persistent QoS XML file**

The example below specifies the persistent QoS XML file which is used by the QoSProvider.

.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8"?>
    <dds xmlns="http://www.omg.org/dds/" 
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
    xsi:schemaLocation="file:DDS_QoSProfile.xsd">
    <qos_profile name="DDS PersistentQosProfile">
         <domainparticipant_qos>
              <user_data>
                   <value></value>
              </user_data>
              <entity_factory>
                   <autoenable_created_entities>true</autoenable_created_entities>
              </entity_factory>
         </domainparticipant_qos>
         <subscriber_qos name="subscriber1">
              <presentation>
                   <access_scope>INSTANCE_PRESENTATION_QOS</access_scope>
                   <coherent_access>true</coherent_access>
                   <ordered_access>true</ordered_access>
              </presentation>
              <partition>
                   <name>partition1</name>
              </partition>
              <group_data>
                   <value></value>
              </group_data>
              <entity_factory>
                   <autoenable_created_entities>true</autoenable_created_entities>
              </entity_factory>
         </subscriber_qos>
         <subscriber_qos name="subscriber2">
              <presentation>
                   <access_scope>INSTANCE_PRESENTATION_QOS</access_scope>
                   <coherent_access>false</coherent_access>
                   <ordered_access>false</ordered_access>
              </presentation>
              <partition>
                   <name></name>
              </partition>
              <group_data>
                   <value></value>
              </group_data>
              <entity_factory>
                   <autoenable_created_entities>true</autoenable_created_entities>
              </entity_factory>
         </subscriber_qos>
         <publisher_qos>
              <presentation>
                   <access_scope>INSTANCE_PRESENTATION_QOS</access_scope>
                   <coherent_access>true</coherent_access>
                   <ordered_access>true</ordered_access>
              </presentation>
              <partition>
                   <name>partition1</name>
              </partition>
              <group_data>
                   <value></value>
              </group_data>
              <entity_factory>
                   <autoenable_created_entities>true</autoenable_created_entities>
              </entity_factory>
         </publisher_qos>
         <datawriter_qos>
              <durability>
                   <kind>PERSISTENT_DURABILITY_QOS</kind>
              </durability>         
              <deadline>
                   <period>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </period>
              </deadline>
              <latency_budget>
                   <duration>
                        <sec>0</sec>
                        <nanosec>0</nanosec>
                   </duration>
              </latency_budget>
              <liveliness>
                   <kind>AUTOMATIC_LIVELINESS_QOS</kind>
                   <lease_duration>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </lease_duration>
              </liveliness>
              <reliability>
                   <kind>RELIABLE_RELIABILITY_QOS</kind>
                   <max_blocking_time>
                        <sec>0</sec>
                        <nanosec>100000000</nanosec>
                   </max_blocking_time>
             </reliability>
             <destination_order>
                  <kind>BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS</kind>
              </destination_order>
              <history>
                   <kind>KEEP_LAST_HISTORY_QOS</kind>
                   <depth>100</depth>
              </history>
              <resource_limits>
                   <max_samples>LENGTH_UNLIMITED</max_samples>
                   <max_instances>LENGTH_UNLIMITED</max_instances>
                   <max_samples_per_instance>LENGTH_UNLIMITED</max_samples_per_instance>
              </resource_limits>
              <transport_priority>
                   <value>0</value>
              </transport_priority>
              <lifespan>
                   <duration>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </duration>
              </lifespan>
              <user_data>
                   <value></value>
             </user_data>
             <ownership>
                  <kind>SHARED_OWNERSHIP_QOS</kind>
              </ownership>
              <ownership_strength>
                   <value>0</value>
              </ownership_strength>
              <writer_data_lifecycle>
                   <autodispose_unregistered_instances>true</autodispose_unregistered_instances>
              </writer_data_lifecycle>
         </datawriter_qos>
         <datareader_qos>
              <durability>
                  <kind>PERSISTENT_DURABILITY_QOS</kind>
              </durability>
              <deadline>
                   <period>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </period>
              </deadline>
              <latency_budget>
                   <duration>
                        <sec>0</sec>
                        <nanosec>0</nanosec>
                   </duration>
              </latency_budget>
              <liveliness>
                   <kind>AUTOMATIC_LIVELINESS_QOS</kind>
                   <lease_duration>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </lease_duration>
              </liveliness>
              <reliability>
                   <kind>BEST_EFFORT_RELIABILITY_QOS</kind>
                   <max_blocking_time>
                        <sec>0</sec>
                        <nanosec>100000000</nanosec>
                   </max_blocking_time>
              </reliability>
              <destination_order>
                    <kind>BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS</kind>
              </destination_order>
              <history>
                   <kind>KEEP_ALL_HISTORY_QOS</kind>
              </history>
              <resource_limits>
                   <max_samples>LENGTH_UNLIMITED</max_samples>
                   <max_instances>LENGTH_UNLIMITED</max_instances>
                   <max_samples_per_instance>LENGTH_UNLIMITED</max_samples_per_instance>
              </resource_limits>
              <user_data>
                   <value></value>
              </user_data>
              <ownership>
                   <kind>SHARED_OWNERSHIP_QOS</kind>
              </ownership>
              <time_based_filter>
                   <minimum_separation>
                        <sec>0</sec>
                        <nanosec>0</nanosec>
                   </minimum_separation>
              </time_based_filter>
              <reader_data_lifecycle>
                   <autopurge_nowriter_samples_delay>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </autopurge_nowriter_samples_delay>
                   <autopurge_disposed_samples_delay>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </autopurge_disposed_samples_delay>
              </reader_data_lifecycle>
         </datareader_qos>
         <topic_qos>
              <topic_data>
                   <value></value>
              </topic_data>
              <durability>
                   <kind>PERSISTENT_DURABILITY_QOS</kind>
              </durability>
              <durability_service>
                   <service_cleanup_delay>
                        <sec>3600</sec>
                        <nanosec>0</nanosec>
                   </service_cleanup_delay>
                   <history_kind>KEEP_LAST_HISTORY_QOS</history_kind>
                   <history_depth>100</history_depth>
                   <max_samples>8192</max_samples>
                   <max_instances>4196</max_instances>
                   <max_samples_per_instance>8192</max_samples_per_instance>
              </durability_service>
              <deadline>
                   <period>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </period>
              </deadline>
              <latency_budget>
                   <duration>
                        <sec>0</sec>
                        <nanosec>0</nanosec>
                   </duration>
              </latency_budget>
              <liveliness>
                   <kind>AUTOMATIC_LIVELINESS_QOS</kind>
                   <lease_duration>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                   </lease_duration>
              </liveliness>
              <reliability>
                   <kind>BEST_EFFORT_RELIABILITY_QOS</kind>
                   <max_blocking_time>
                        <sec>0</sec>
                        <nanosec>100000000</nanosec>
                        </max_blocking_time>
              </reliability>
              <destination_order>
                   <kind>BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS</kind>
              </destination_order>
              <history>
                   <kind>KEEP_LAST_HISTORY_QOS</kind>
                   <depth>1</depth>
              </history>
                  <resource_limits>
                   <max_samples>LENGTH_UNLIMITED</max_samples>
                   <max_instances>LENGTH_UNLIMITED</max_instances>
                   <max_samples_per_instance>LENGTH_UNLIMITED</max_samples_per_instance>
              </resource_limits>
              <transport_priority>
                   <value>0</value>
              </transport_priority>
              <lifespan>
                   <duration>
                        <sec>DURATION_INFINITE_SEC</sec>
                        <nanosec>DURATION_INFINITE_NSEC</nanosec>
                  </duration>
              </lifespan>
              <ownership>
                   <kind>SHARED_OWNERSHIP_QOS</kind>
              </ownership>
         </topic_qos>
    </qos_profile>
    </dds>
    
Applying QoS Profile 
====================

To set the QoS profile for a DDS entity using the Node.js DCPS API and an XML file, the user specifies the qos file URI or file path and the QoS profile name as parameters.

**Example**
 
Create a QoS provider and get the respective entity QoS. The example below uses the QoS XML file referred in :ref:`QoSProfile`. For a detailed example refer to 'GetSetQoSExample.js' file referred in :ref:`GetSetQoSExample` example.

.. code-block:: javascript
       
    const dds = require('vortexdds');
    const HelloWorldTopic = require('./HelloWorldTopic.js');

    /**
     * Path of the qos xml file
     * The DDS_PersistentQoS.xml file is used for this example
     * It is located in the same directory of this example
    * */
    const QOS_PATH = 'DDS_PersistentQoS.xml';
    const QOS_PROFILE = 'DDS PersistentQosProfile';
    const DOMAIN_ID = dds.DDS_DOMAIN_DEFAULT;

    main();

    async function main(){

      /**
       * Set QoS using QoS XML file
       */

      // create a qos provider using qos xml file
      let qp = new dds.QoSProvider(QOS_PATH, QOS_PROFILE);

      // get participant qos from qos provider and create a participant
      let pqos = qp.getParticipantQos();
      let participant = new dds.Participant(DOMAIN_ID, pqos);

      // get publisher qos from qos provider and create a publisher
      let pubqos = qp.getPublisherQos();
      let publisher = new dds.Publisher(participant, pubqos);

      let subqos = qp.getSubscriberQos('subscriber1');
      let subscriber = new dds.Subscriber(participant, subqos);

      // get topic qos from qos provider and create a topic
      let tqos = qp.getTopicQos();
      let topic = await HelloWorldTopic.create(participant, tqos);

      // get reader qos from qos provider and create a reader
      let rqos = qp.getReaderQos();
      let reader = new dds.Reader(subscriber, topic, rqos);

      // get writer qos from qos provider and create a writer
      let wqos = qp.getWriterQos();
      let writer = new dds.Writer(publisher, topic, wqos);

      // delete the participant
      participant.delete();
    }

Setting QoS Using Node.js DCPS API Classes
******************************************

QoS settings can also be set by using the Node.js classes alone.  (No XML files required.) 

**Example**

Below is a code snippet similar to the 'GetSetQoSExample/GetSetQoSExample.js' file. It demonstrates how to specify the QoS settings for a topic, writer and reader using the Node.js DCPS APIs.

.. code-block:: javascript

    const dds = require('vortexdds');
    const HelloWorldTopic = require('./HelloWorldTopic.js');

    main();

    async function main(){	  
  
      let participant = new dds.Participant();
      
      // get the default topic qos
      let tqos = dds.QoS.topicDefault();

      // modify the topic qos policies

      // All of the policy variables can be set
      tqos.durabilityService = {
        serviceCleanupDelay: 5000,
        historyKind: dds.HistoryKind.KeepLast,
        historyDepth: 10,
        maxSamples: 10,
        maxInstances: 5,
        maxSamplesPerInstance: -1,
      };

      // Or a subset of the policy variables can be set
      tqos.durabilityService = {
        historyKind: dds.HistoryKind.KeepLast,
      };

      tqos.reliability = dds.ReliabilityKind.reliable;
      tqos.durability = dds.DurabilityKind.Volatile;
      tqos.topicdata = 'Hello world topic';
      tqos.userdata = 'connected to hello world';
      tqos.groupdata = 'hello world group';
      tqos.history = {
        kind: dds.HistoryKind.KeepLast,
        depth: 10,
      };
      tqos.resourceLimits = {
        maxSamples: 10,
        maxInstances: 5,
      };
      tqos.lifespan = 5000;
      tqos.deadline = 3000;
      tqos.latencyBudget = 4000;
      tqos.ownership = dds.OwnershipKind.Exclusive;
      tqos.liveliness = {
        kind: dds.LivelinessKind.ManualByTopic,
        leaseDuration: 10,
      };
      tqos.reliability = {
        kind: dds.ReliabilityKind.Reliable,
        maxBlockingTime: 100,
      };
      tqos.transportPriority = 5;
      tqos.destinationOrder = dds.DestinationOrderKind.BySourceTimestamp;

      // create topic with qos
      let topic = await HelloWorldTopic.create(participant, tqos);

      // get topic qos policies
      console.log('\n', '** Topic QoS **');
      console.log('Reliability: ', tqos.reliability);
      console.log('Durability: ', tqos.durability);
      console.log('Topic data: ', tqos.topicdata);
      console.log('User data: ', tqos.userdata);
      console.log('Group data: ', tqos.groupdata);
      console.log('History: ', tqos.history);
      console.log('Resource Limits: ', tqos.resourceLimits);
      console.log('Lifespan: ', tqos.lifespan);
      console.log('Deadline: ', tqos.deadline);
      console.log('Latency Budget: ', tqos.latencyBudget);
      console.log('Ownership: ', tqos.ownership);
      console.log('Liveliness: ', tqos.liveliness);
      console.log('Transport Priority: ', tqos.transportPriority);
      console.log('Destination Order: ', tqos.destinationOrder);
      console.log('Durability Service: ', tqos.durabilityService);

      // get the default writer qos
      let wqos = dds.QoS.writerDefault();

      // modify the writer qos policies
      wqos.writerDataLifecycle = false;

      // create a writer with qos
      let writer = new dds.Writer(publisher, topic, wqos);

      // get writer qos policies
      console.log('\n', '** Writer QoS **');
      console.log('Writer data lifecyle: ', wqos.writerDataLifecycle);

      // get the default reader qos
      let rqos = dds.QoS.readerDefault();

      // modify the reader qos policies
      rqos.partition = 'partition1';
      rqos.timebasedFilter = 60000;
      rqos.readerDataLifecycle = {
        autopurgeNoWriterSamples: 100,
        autopurgeDisposedSamplesDelay: 500,
      };

      // create a reader with qos
      let reader = new dds.Reader(subscriber, topic, rqos);

      // get reader qos policies
      console.log('\n', '** Reader QoS **');
      console.log('Partition: ', rqos.partition);
      console.log('Time based filter: ', rqos.timebasedFilter);
      console.log('Reader data lifecycle: ', rqos.readerDataLifecycle);

      // delete the participant
      participant.delete();

      // NOTE: Actual reading and writing data  is not demonstrated in this example
    }



.. external links
.. _QoSProfile.xsd: http://www.omg.org/spec/dds4ccm/20110201/DDS_QoSProfile.xsd
.. _DDS_DefaultQoS.xml: http://www.omg.org/spec/dds4ccm/20110201/DDS_DefaultQoS.xml
