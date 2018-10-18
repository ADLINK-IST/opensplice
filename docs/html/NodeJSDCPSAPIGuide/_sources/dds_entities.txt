.. _`Node.js API for Vortex DDS`:

##########################
Node.js API for Vortex DDS 
##########################

The Node.js DCPS API provides users with Node.js classes to model DDS communication using JavaScript and pure
DDS applications.

The Node.js DCPS API consists of one module.

* vortexdds

This section provides an overview of the main DDS concepts and Node.js API examples for these DDS concepts.

.. note:: 

    - The Node.js DCPS API documentation can be found in the following directory:

          *$OSPL_HOME/docs/nodejs/html*


API Usage Patterns
******************

The typical usage pattern for the Node.js DCPS API for Vortex DDS is the following:

* Model your DDS topics using IDL and generate Node.js topic classes from IDL. 
* AND/OR generate Node.js topic classes for topics that already exist in the DDS system.
* Start writing your Node.js program using the Node.js API for Vortex DDS.

The core classes are ``Participant``, ``Topic``, ``Reader`` and ``Writer``.
``Publisher`` and ``Subscriber`` classes can be used to adjust the Quality of Service (QoS) defaults.  

For details on setting QoS values with the API, see :ref:`QoS Provider`.

The following list shows the sequence in which you would use the Vortex classes:

* Create a ``Participant`` instance. 
* Create one or more ``Topic`` instances.
* If you require publisher or subscriber level non-default QoS settings, create ``Publisher`` and/or ``Subscriber`` instances. (The most common reason for changing publisher/subscriber QoS is to define non-default partitions.)
* Create ``Reader`` and/or ``Writer`` classes using the ``Topic`` instances that you created.
* If you required data filtering, create ``QueryCondition`` objects.
* Create the core of program, writing and/or reading data and processing it.


Participant
***********

The Node.js ``Participant`` class represents a DDS domain participant entity. 
  
In DDS - “A domain participant represents the local membership of the application in a domain.  A domain is a distributed concept that links all the applications able to communicate with each other.  It represents a communication plane: only the publishers and subscribers attached to the same domain may interact.”

Parameters:	

* {number} domainId 0 <= domainId <= 230 (Default: DDSConstants.DDS_DOMAIN_DEFAULT)  
* {QoS} qos (Default: undefined)
* {object} listener (Default: undefined)

The optional parameters have defaults if not specified.  If the qos is undefined, the OSPL default QoS is used.


**Examples**

Create a Vortex DDS domain participant. Returns participant or throws a ``DDSError`` if the participant cannot be created.

Create a domain participant in the default DDS domain (the one specified by the OSPL_URI environment variable).

.. code-block:: javascript

    const dds = require('vortexdds');

    // create domain participant
    const participant = new dds.Participant();
    
Create a participant on the default domain with a QoS profile.  Consider the code snippet below taken from example: GetSetQoSExample/GetSetQoSExample.js file. 

.. code-block:: javascript
       
    const QOS_PATH = 'DDS_Get_Set_QoS.xml';
    const QOS_PROFILE = 'DDS GetSetQosProfile';
    const DOMAIN_ID = dds.DDS_DOMAIN_DEFAULT;     

    async function main(){

      // create a qos provider using qos xml file
      let qp = new dds.QoSProvider(QOS_PATH, QOS_PROFILE);

      // get participant qos from qos provider
      let pqos = qp.getParticipantQos();
      let participant = new dds.Participant(DOMAIN_ID, pqos);

    }

.. _Topic:

Topic
*****

The Node.js ``Topic`` class represents a DDS topic type.  The DDS topic corresponds to a single data type.  In DDS, data is distributed by publishing and subscribing topic data samples.

Javascript ``Topic`` instances can be created using an IDL file.

**Step 1 - Generate TypeSupport objects from IDL file**

The function ``getTopicTypeSupportsForIDL(idlPath)`` is provided to generate a ``TypeSupport`` instance for every topic defined in an IDL file.  This function returns a promise, therefore should be called from an async function.      

**Step 2 - Create Topic instance using TypeSupport**

The ``createTopicFor`` method in the ``Participant`` class can then be used to create a topic instance.

.. code-block:: javascript

	/**
	* Create a Topic on this participant given a TypeSupport object.
	* @param {string} topicName
	* @param {TypeSupport} typeSupport
	* @param {QoS} QoS (default undefined)
	* @param {object} listener (default undefined)
	* @returns {Topic} topic instance
	*/
	createTopicFor(
		topicName,
		typeSupport,
		qos = null,
		listener = null
	) {




**Example**

Create a Vortex DDS topic named 'HelloWorldData_Msg' based on the DDS Topic type ``Msg`` from the ``HelloWorldData.idl`` file.  This topic is created using the getTopicTypeSupportsForIDL function, and the ``Participant`` createTopicFor method.

Consider the code snippet below taken from example: HelloWorld/HelloWorldTopic.js file. 

.. code-block:: javascript

    const dds = require('vortexdds');
    const path = require('path');

    module.exports.create = async function(participant) {

      const topicName = 'HelloWorldData_Msg';
      const idlName = 'HelloWorldData.idl';
      const idlPath = path.resolve(idlName);

      //wait for dds.getTopicTypeSupportsForIDL to return a Map of typeSupports
      let typeSupports = await dds.getTopicTypeSupportsForIDL(idlPath);

      //HelloWorldData.idl contains 1 topic
      let typeSupport = typeSupports.get('HelloWorldData::Msg');

      //create topic qos
      let tqos = dds.QoS.topicDefault();
      tqos.durability = dds.DurabilityKind.Transient;
      tqos.reliability = dds.ReliabilityKind.Reliable;

      //create topic
      return participant.createTopicFor(topicName, typeSupport, tqos);

    };


Publisher
*********

The Node.js ``Publisher`` class represents a DDS publisher entity.

In DDS, a publisher is “an object responsible for data distribution.  It may publish data of different data types.”

Use of the ``Publisher`` class is optional.
In its place, you can use a ``Participant`` instance.
Reasons for explicitly creating a ``Publisher`` instance are:

* to specify non-default QoS settings, including specifying the DDS *partition* upon which samples are written.
* to control the timing of publisher creation and deletion.

**Examples**

Create a DDS Publisher entity. Returns publisher or throws a ``DDSError`` if the publisher cannot be created.

Create a publisher with participant.
    
.. code-block:: javascript

    const dds = require('vortexdds');
       
    //create participant
    const participant = new dds.Participant();

    //create publisher
    const pub = participant.createPublisher();
        
Create a publisher with a participant and QoS profile. Consider the code snippet below taken from example: GetSetQoSExample/GetSetQoSExample.js file. 
    
.. code-block:: javascript
       
    const QOS_PATH = 'DDS_Get_Set_QoS.xml';
    const QOS_PROFILE = 'DDS GetSetQosProfile';
    const DOMAIN_ID = dds.DDS_DOMAIN_DEFAULT;     

    async function main(){

      // create a qos provider using qos xml file
      let qp = new dds.QoSProvider(QOS_PATH, QOS_PROFILE);

      // get participant qos from qos provider
      let pqos = qp.getParticipantQos();
      let participant = new dds.Participant(DOMAIN_ID, pqos);

      // get publisher qos from qos provider and create a publisher
      let pubqos = qp.getPublisherQos();
      let publisher = new dds.Publisher(participant, pubqos);
    }
    
Writer
******

The Node.js ``Writer`` class represents a DDS data writer entity.

In DDS - “The writer is the object the application must use to communicate to a publisher the existence and value of data-objects of a given type.”

A ``Writer`` class is required in order to write data to a DDS domain.
It is attached to a DDS publisher or a DDS domain participant.

A ``Writer`` class instance references an existing ``Topic`` instance.

**Examples**

Create a Vortex DDS domain writer and write data. Returns writer or throws a ``DDSError`` if the writer cannot be created. The example below uses the 'HelloWorldData_Msg' topic from the 'HelloWorldTopic.js' file as shown in :ref:`Topic` example.  Consider the code snippet below taken from example: HelloWorld/HelloWorldPublisher.js file. 

.. code-block:: javascript

    const dds = require('vortexdds');    
    const HelloWorldTopic = require('./HelloWorldTopic');

    main();

    async function main(){

        //create domain participant
        let participant = new dds.Participant();

        //wait for topic to be created
        let topic = await HelloWorldTopic.create(participant);

        //create a publisher with publisher QoS
        let pqos = dds.QoS.publisherDefault();
        pqos.partition = 'HelloWorld example';
        let pub = participant.createPublisher(pqos, null);

        //create a writer with writer QoS
        let wqos = dds.QoS.writerDefault();
        wqos.durability = dds.DurabilityKind.Transient;
        wqos.reliability = dds.ReliabilityKind.Reliable;
        let writer = pub.createWriter(topic, wqos, null);

        //write one sample
        let msg = {userID: 1, message: 'Hello World'};

        console.log('=== HelloWorldPublisher');
        console.log('=== [Publisher] writing a message containing :');
        console.log('    userID  : ' + msg.userID);
        console.log('    Message : ' + msg.message);

        writer.write(msg);

    }


Subscriber
**********

The Node.js ``Subscriber`` class represents a DDS subscriber entity.

In DDS, a subscriber is “an object responsible for receiving published data and making it available to the receiving application.  It may receive and dispatch data of different specified types.”

Use of the ``Subscriber`` class is optional.
In its place, you can use a ``Participant`` instance.
Reasons for explicitly creating a ``Subscriber`` instance are:

* to specify non-default QoS settings, including specifying the DDS *partition* upon which samples are written.
* to control the timing of subscriber creation and deletion.

**Examples**


Create a Vortex DDS domain subscriber. Returns subscriber or throw a ``DDSError`` if the subscriber cannot be created.
    
    
Create a subscriber with participant.
    
.. code-block:: javascript

    const dds = require('vortexdds');
       
    //create participant
    consr participant = new dds.Participant();

    //create Subscriber
    const sub = participant.createSubscriber();    

Create a subscriber with participant and QoS profile, where the DDS_DefaultQoS_All.xml file is located in the project directory.
    
.. code-block:: javascript
       
    const QOS_PATH = 'DDS_Get_Set_QoS.xml';
    const QOS_PROFILE = 'DDS GetSetQosProfile';
    const DOMAIN_ID = dds.DDS_DOMAIN_DEFAULT;     

    async function main(){

      // create a qos provider using qos xml file
      let qp = new dds.QoSProvider(QOS_PATH, QOS_PROFILE);

      // get participant qos from qos provider
      let pqos = qp.getParticipantQos();
      let participant = new dds.Participant(DOMAIN_ID, pqos);

      /*
         Get subscriber qos from qos provider and create a subscriber
         In the qos xml file provided with this example, there are
         two subscriber qos. The subscriber qos with
         id 'subscriber1' is chosen below
        
         NOTE: If there are more than one qos entries in the xml file
         for a dds entity, then the entity qos id must be specified.
         Otherwise an error will be thrown while trying to get the
         respective entity qos from the qos provider
       */
      let subqos = qp.getSubscriberQos('subscriber1');
      let subscriber = new dds.Subscriber(participant, subqos);
      }
    

Reader
******

The Node.js ``Reader`` class represents a DDS data reader entity.

In DDS - “To access the received data, the application must use a typed reader attached to the subscriber.”

A ``Reader`` class is required in order to write data to a DDS domain.
It is attached to a DDS subscriber or a DDS participant.

A ``Reader`` class instance references an existing ``Topic`` instance.

**Examples**

Create a Vortex DDS domain reader with a subscriber and read/take data. Returns reader or throw a ``DDSError`` instance if the reader cannot be created. Consider the code snippet below taken from example: HelloWorld/HelloWorldSubscriber.js file. 

.. code-block:: javascript

    const dds = require('vortexdds');
    const HelloWorldTopic = require('./HelloWorldTopic');

    main();

    async function main(){

        //create domain participant
        let participant = new dds.Participant();

        //wait for topic to be created
        let topic = await HelloWorldTopic.create(participant);

        //create subscriber with subscriber QoS
        let sqos = dds.QoS.subscriberDefault();
        sqos.partition = 'HelloWorld example';
        let sub = participant.createSubscriber(sqos, null);

        //create reader with reader QoS
        let rqos = dds.QoS.readerDefault();
        rqos.durability = dds.DurabilityKind.Transient;
        rqos.reliability = dds.ReliabilityKind.Reliable;
        let reader = sub.createReader(topic, rqos, null);

        console.log('=== [Subscriber] Ready ...');

        attemptTake(participant, reader, 0);

        participant.delete();
       }

       /* Attempt to take from the reader every 200 ms.
        * If a successful take occurs, we are done.
        * If a successful take does not occur after 100 attempts,
        * return.
        */
       function attemptTake(participant, reader, numberAttempts){

       	setTimeout(function() {
        	let takeArray = reader.take(1);
        	if (takeArray.length > 0 && (takeArray[0].info.valid_data === 1)) {
          	console.log('=== [Subscriber] message received :');
          	console.log('    userID  : ' + takeArray[0].sample.userID);
          	console.log('    Message : ' + takeArray[0].sample.message);
        	} else {
          	if (numberAttempts < 100){
            	return attemptTake(participant, reader, numberAttempts + 1);
          	}
        	}
        	participant.delete();
        	return;
      	}, 200);
       }

Take data with read condition from a data reader.

.. code-block:: javascript

    const numberOfSamples = 10;
    const cond = new dds.ReadCondition(reader, dds.StateMask.any);
    let readArray = reader.takeCond(numberOfSamples, cond);

Read data with read condition from a data reader. 

.. code-block:: javascript

    const numberOfSamples = 10;
    const cond = new dds.ReadCondition(reader, dds.StateMask.any);
    let readArray = reader.readCond(numberOfSamples, cond);

WaitSet
*******

The Node.js ``WaitSet`` class represents a DDS wait set.

A WaitSet object allows an application to wait until one or more of the attached Condition objects evaluates to true or until the timeout expires.

**Example**

Create a ``WaitSet`` with a read condition to wait until publication is matched.  Consider the code snippet below taken from example: PingPong/ping.js file. 

.. code-block:: javascript

    const dds = require('vortexdds');
    const PingPongTopic = require('./PingPongTopic');

    // we iterate for 20 samples
    const numSamples = 20;
    main();

    async function main() {
      // create our entities
      let participant = new dds.Participant();
      let topic = await PingPongTopic.create(participant);

      // create publisher on partition 'Ping' and writer on the publisher
      // on pong, we have a subscriber on the same partition and topic
      let pqos = dds.QoS.publisherDefault();
      pqos.partition = 'Ping';
      let publisher = participant.createPublisher(pqos, null);
      let writer = publisher.createWriter(topic);

      // create a subscriber on partition 'Pong' and a reader on the subscriber
      let sqos = dds.QoS.subscriberDefault();
      sqos.partition = 'Pong';
      let subscriber = participant.createSubscriber(sqos);
      let reader = subscriber.createReader(topic);

      // create waitset which waits until pong subscriber on partition
      // 'Ping' is found
      let pubMatchedWaitset = new dds.Waitset();
      let statCond = new dds.StatusCondition(writer);
      statCond.enable(dds.StatusMask.publication_matched);
      pubMatchedWaitset.attach(statCond, writer);

      // waitset for new data
      let newDataWaitset = new dds.Waitset();
      let newDataCond = new dds.ReadCondition(reader,
        dds.StateMask.sample.not_read);
      newDataWaitset.attach(newDataCond, reader);

      // block until pong has been found
      console.log('Waiting for pong subscriber to be found...');
      pubMatchedWaitset.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
        if (err) throw err;
        console.log('Found pong subscriber.');
        // Ping writes the first message
        let msg = {userID: 0, message: 'from ping'};
        console.log('sending ' + JSON.stringify(msg));
        writer.write(msg);
        onDataAvailable(participant, newDataWaitset, reader, writer, 0);
      });
    }

    /* onDataAvailable waits for new data. When we have new data, it
     * reads it, prints it, and sends back a message with the userID
     * field incremented by one and 'from ping' in the message field.
     * It then calls onDataAvailable again.
     * We quit when iterLower === numSamples (20 iterations in this example).
     */
    function onDataAvailable(
      participant,
      ws,
      reader,
      writer,
      iterLower
    ) {
      if (iterLower === numSamples) {
        console.log('Done');
        participant.delete();
        return;
      }
      // when we get a message from pong, read it, then send it back
      // with userID+1 and message 'from ping'
      ws.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
        if (err) throw err;
        let takeArray = reader.take(1);
        if (takeArray.length > 0 && (takeArray[0].info.valid_data === 1)) {
          let sample = takeArray[0].sample;
          console.log('received: ' + JSON.stringify(sample));
          let msg = {userID: sample.userID + 1, message: 'from ping'};
          console.log('sending ' + JSON.stringify(msg));
          writer.write(msg);
        }
        onDataAvailable(participant, ws, reader, writer, iterLower + 1);
      });
    }


QueryCondition
**************

The Node.js ``QueryCondition`` class represents a DDS query entity.

A query is a data reader, restricted to accessing data that matches specific status conditions and/or a filter expression.

A ``QueryCondition`` class instance references an existing ``Reader`` instance.

**Example**

Create a ``QueryCondition`` with a state mask and a filter expression for a reader and take data.  Consider the code snippet below taken from example: jsshapes/read.js file. 

.. code-block:: javascript

    const dds = require('vortexdds');
    const ShapeTopicHelper = require('./ShapeTopicHelper');
    const util = require('util');

    // we are reading 100 samples of a blue circle
    const shapeName = 'Circle';
    const numSamples = 100;
    const mask = dds.StateMask.sample.not_read;
    const sqlExpression = 'color=%0';
    const params = ['BLUE'];

    main();

    async function main() {
      const participant = new dds.Participant();

      // set up our topic
      const circleTopic = await ShapeTopicHelper.create(participant, 'Circle');

      // our reader has volatile qos
      let readerqosprovider = new dds.QoSProvider(
        './DDS_VolatileQoS_All.xml',
        'DDS VolatileQosProfile'
      );

      // set up circle reader
      const circleReader = participant.createReader(circleTopic,
        readerqosprovider.getReaderQos());

      // set up waitset
      const newDataWs = new dds.Waitset();
      const queryCond = new dds.QueryCondition(
        circleReader,
        mask,
        sqlExpression,
        params,
        1
      );
      // attach the query condition
      newDataWs.attach(queryCond, circleReader);

      console.log('Waiting for demo_ishapes to publish a blue circle...');
      onDataAvailable(participant, newDataWs, circleReader, queryCond, 0);
    };

    /* onDataAvailable waits until we have new data (with the query condition).
     * when we do, we read it, print the data (shape data), and then call
     * onDataAvailable with iterLower incremented by one, to wait for more data.
     * This repeats until iterLower === numSamples (100 samples in this case).
     * When iterLower === numSamples, we delete the participant and quit.
     */
    function onDataAvailable(
      participant,
      ws,
      reader,
      queryCond,
      iterLower
    ) {
      if (iterLower === numSamples) {
        console.log('Done.');
        participant.delete();
        return;
      }
      ws.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
        if (err) throw err;
        let sampleArray = reader.takeCond(1, queryCond);
        if (sampleArray.length > 0 && (sampleArray[0].info.valid_data === 1)) {
          let sample = sampleArray[0].sample;
          console.log(
            util.format(
              '%s %s of size %d at (%d,%d)',
              sample.color,
              shapeName,
              sample.shapesize,
              sample.x,
              sample.y
            )
          );
        }
        onDataAvailable(participant, ws, reader, queryCond, iterLower + 1);
      });
    }

