.. _`MATLAB DDS Classes`:

#################################
MATLAB API for Vortex DDS Classes
#################################

The DDS MATLAB Integration provides a class library with custom classes to read and write data with DDS.

The MATLAB DDS Classes are included in a **Vortex** package.   

API Usage patterns
******************

The typical usage pattern for the MATLAB API for Vortex DDS is the following:

* model your DDS topics using IDL
* using ``idlpp -l matlab`` to compile your IDL into MATLAB topic classes. See :ref:`MATLAB Generation from IDL`.
* start writting your MATLAB program using the MATLAB API for Vortex DDS.

The core classes you must use are ``Vortex.Topic`` and either ``Vortex.Reader`` or ``Vortex.Writer``.
Other classes may be required, especially if you need to adjust the Quality of Service (QoS) defaults.
For details on setting QoS values with the API, see :ref:`QoS Provider`.
The following list shows the sequence in which you would use the Vortex classes:

* If you require participant-level non-default QoS settings, create a ``Vortex.Participant`` instance. Pass the participant to subsequently created Vortex entities.
* Create one or more ``Vortex.Topic`` instances for the IDL topics your program will read or write.
* If you require publisher or subscriber level non-default QoS settings, create ``Vortex.Publisher`` and/or ``Vortex.Subscriber`` instances. Pass these to any created reader or writers. (The most common reason for changing publisher/subscriber QoS is to define non-default partitions.)
* Create ``Vortex.Reader`` and/or ``Vortex.Writer`` classes from the ``Vortex.Topic`` instances that you created.
* If you required data filtering, create ``Vortex.Query`` objects.
* Create the core of program, creating instances of your topic classes and writing them; or, reading data and processing it.

Vortex.Topic
************

The MATLAB Topic class represents a DDS topic type.  The DDS topic corresponds to a single data type.  In DDS, data is distributed by publishing and subscribing topic data samples.

For a DDS Topic type definition, a corresponding MATLAB class must be defined in the MATLAB workspace.
This is either a class created by ``idlpp`` (see :ref:`MATLAB Generation from IDL`) or a manually
created MATLAB class (see :ref:`MATLAB without IDL`).
It is recommend that you create DDS Topic type definitions via IDL and `idlpp`.

**API Examples**

Create a Vortex DDS domain topic. Returns a topic instance, or throws a ``Vortex.DDSException`` if the topic cannot be created.

Create a topic named 'Circle' based on the DDS Topic type ``ShapeType`` with default participant and QoS::
    
   topic = Vortex.Topic('Circle', ?ShapeType);

**Note**: In MATLAB, references to classes such as ShapeType are created by prefixing them with a
question mark (?). If the class is in a MATLAB package, then the fully qualified name must be used.
For example: ``?ShapesDemo.Topics.ShapeType``.
       
Create the 'Circle' topic with an explicitly created participant::

   % dp: a Vortex.DomainParticipant instance  
   topic = Vortex.Topic(dp, 'Circle', ?ShapeType);
       
Create the 'Circle' topic with default participant and QoS profile::

   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   topic = Vortex.Topic('Circle', ?ShapeType, qosFileURI, qosProfile); 
       
Create the 'Circle' topic with explicit participant and QoS profile::
    
   % dp: a Vortex.DomainParticipant instance  
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   topic = Vortex.Topic(dp, 'Circle', ?ShapeType, qosFileURI, qosProfile);

Vortex.DomainParticipant
************************

The ``Vortex.DomainParticipant`` class represents a DDS domain participant entity. 
  
In DDS - “A domain participant represents the local membership of the application in a domain.  A domain is a distributed concept that links all the applications able to communicate with each other.  It represents a communication plane: only the publishers and subscribers attached to the same domain may interact.”

Use of the ``Vortex.DomainParticipant`` class is optional.
The API provides a 'default participant', which is used if no explicit domain participant is provided.
The default participant is created on first usage, and is disposed when MATLAB exits.
Reasons for using an explicitly created domain participant are:

* to provide non-default QoS settings.
* to control the timing of participant creation and destruction.

**API Examples**

Create a Vortex DDS domain participant. Returns participant or throws a ``Vortex.DDSException`` if the participant cannot be created.

Create a domain participant in the default DDS domain (the one specified by the OSLP_URI environment variable)::
    
   dp = Vortex.DomainParticipant(); 
      
Create a domain participant on domain, specifying a domain id::

   % domainId: an integer value
   dp = Vortex.DomainParticipant(domainId);

**Note**: The underlying DCPS C99 API used by `Vortex.DomainParticipant` does not currently support this operation, and will result in a ``Vortex.DDSException`` being raised.
      
Create a participant on default domain with QoS profile::
    
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   dp = Vortex.DomainParticipant(qosFileURI, qosProfile); 
      
Create a participant on domain with QoS profile::
    
   % domainId: an integer value
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   dp = Vortex.DomainParticipant(domainId, qosFileURI, qosProfile);

Vortex.Publisher
****************

The MATLAB ``Vortex.Publisher`` class represents a DDS publisher entity.

In DDS, a publisher is “an object responsible for data distribution.  It may publish data of different data types.”

Use of the ``Vortex.Publisher`` class is optional.
In it's place, you can use a ``Vortex.DomainParticipant`` instance, or default to the *default domain participant*.
Reasons for explicitly creating a ``Vortex.Publisher`` instance are:

* to specify non-default QoS settings, including specifying the DDS *partition* upon which samples are written.
* to control the timing of publisher creation and deletion.

**API Examples**

Create a DDS Publisher entity. Returns publisher or throws a ``Vortex.DDSException`` if the publisher cannot be created.

Create a default publisher with default participant::
    
   pub = Vortex.Publisher(); 
    
Create a publisher with an explicit participant::

   % dp: a Vortex.DomainParticipant instance  
   pub = Vortex.Publisher(dp);

Create default publisher with default participant and QoS profile::
    
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   pub = Vortex.Publisher(qosFileURI, qosProfile); 

Create a publisher with participant and QoS profile::
    
   % dp: a Vortex.DomainParticipant instance  
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   pub = Vortex.Publisher(dp, qosFileURI, qosProfile);

Vortex.Writer
*************

The MATLAB ``Vortex.Writer`` class represents a DDS data writer entity.

In DDS - “The DataWriter is the object the application must use to communicate to a publisher the existence and value of data-objects of a given type.”

A ``Vortex.Writer`` class is required in order to write data to a DDS domain.
It may be explicitly attached to a DDS publisher or a DDS domain participant; or, it is implicitly attached to the *default domain participant*.

A ``Vortex.Writer`` class instance references an existing ``Vortex.Topic`` instance.

**API Examples**

Create a Vortex DDS domain writer. Returns writer or throws a ``Vortex.DDSException`` if the writer cannot be created.
    
Create a writer for a topic, in the default domain participant and default QoS settings::

   % topic: a Vortex.Topic instance
   writer = Vortex.Writer(topic); 
     
Create a writer within an explicitly specified publisher or domain participant::

   % pubOrDp: a Vortex.Publisher or Vortex.DomainParticipant instance   
   % topic: a Vortex.Topic instance
   writer = Vortex.Writer(pubOrDp, topic);
     
Create writer for a topic with explicit QoS profile::
    
   % topic: a Vortex.Topic instance
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   writer = Vortex.Writer(topic, qosFileURI, qosProfile); 
     
Create a writer with publisher or participant, topic and QoS profile::
    
   % pubOrDp: a Vortex.Publisher or Vortex.DomainParticipant instance   
   % topic: a Vortex.Topic instance
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   writer = Vortex.Writer(pubOrDp, topic, qosFileURI, qosProfile);

Write a ShapeType topic class instance to a writer::

   % writer: a Vortex.Writer instance
   % ShapeType: a 'topic class' created manually or via IDLPP
   data = ShapeType(); % create an object instance
   % set data values...
   data.color = 'RED';
   % ... set other values ...

   ddsStatus = writer.write(data);

**Note**: the returned status value is 0 for success, and negative for failure. Use the ``Vortex.DDSException`` class to decode an failure status.

Dispose a DDS topic instance::

   % writer: a Vortex.Writer instance
   % ShapeType: a 'topic class' created manually or via IDLPP
   data = ShapeType(); % create an object instance
   % set data key values...
   data.color = 'RED';

   ddsStatus = writer.dispose(data);

**Note**: the returned status value is 0 for success, and negative for failure. Use the ``Vortex.DDSException`` class to decode an failure status.

Unregister a DDS topic instance::

   % writer: a Vortex.Writer instance
   % ShapeType: a 'topic class' created manually or via IDLPP
   data = ShapeType(); % create an object instance
   % set data key values...
   data.color = 'RED';

   ddsStatus = writer.unregister(data);

**Note**: the returned status value is 0 for success, and negative for failure. Use the ``Vortex.DDSException`` class to decode an failure status.

Vortex.Subscriber
*****************

The MATLAB ``Vortex.Subscriber`` class represents a DDS subscriber entity.

In DDS, a subscriber is “an object responsible for receiving published data and making it available to the receiving application.  It may receive and dispatch data of different specified types.”

Use of the ``Vortex.Subscriber`` class is optional.
In it's place, you can use a ``Vortex.DomainParticipant`` instance, or default to the *default domain participant*.
Reasons for explicitly creating a ``Vortex.Subscriber`` instance are:

* to specify non-default QoS settings, including specifying the DDS *partition* upon which samples are written.
* to control the timing of subscriber creation and deletion.

**API Examples**


Create a Vortex DDS domain subscriber. Returns subscriber or throw a ``Vortex.DDSException`` if the subscriber cannot be created.
    
Create a subscriber within the default domain participant::
    
   sub = Vortex.Subscriber(); 
     
Create a subscriber within an explicit participant::
    
   % dp: a Vortex.DomainParticipant instance  
   sub = Vortex.Subscriber(dp);
     
Create subscriber within the default domain participant and with a QoS profile::
    
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   sub = Vortex.Subscriber(qosFileURI, qosProfile); 
      
Create a subscriber with participant and QoS profile::
    
   % dp: a Vortex.DomainParticipant instance  
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   sub = Vortex.Subscriber(dp, qosFileURI, qosProfile);


Vortex.Reader
*************

The MATLAB ``Vortex.Reader`` class represents a DDS data reader entity.

In DDS - “To access the received data, the application must use a typed DataReader attached to the subscriber.”

A ``Vortex.Reader`` class is required in order to write data to a DDS domain.
It may be explicitly attached to a DDS subscriber or a DDS domain participant; or, it is implicitly attached to the *default domain participant*.

A ``Vortex.Reader`` class instance references an existing ``Vortex.Topic`` instance.

**API Examples**

Create a Vortex DDS domain reader. Returns reader or throw a ``Vortex.DDSException`` instance if the reader cannot be created.
    
Create a reader for a topic within the default domain participant, and with default QoS::
    
   % topic: a Vortex.Topic instance
   reader = Vortex.Reader(topic); 
     
Create a reader for a topic within a subscriber or participant, and with default QoS::
    
   % subOrDp: a Vortex.Subscriber or Vortex.DomainParticipant instance   
   % topic: a Vortex.Topic instance
   reader = Vortex.Reader(subOrDp, topic);
      
Create reader for a topic within the default domain participant and with a QoS profile::
    
   % topic: a Vortex.Topic instance
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   reader = Vortex.Reader(topic, qosFileURI, qosProfile); 
     
Create a reader for a topic within a subscriber or participant, with with a QoS profile::
    
   % subOrDp: a Vortex.Subscriber or Vortex.DomainParticipant instance   
   % topic: a Vortex.Topic instance
   % qosFileURI: a char array representing a file: URI
   % qosProfile: a char array containing the name of a profile defined in the QoS File   
   reader = Vortex.Reader(subOrDp, topic, qosFileURI, qosProfile);

Take data from a data reader::

   % reader: a Vortex.Reader
   [data, dataState] = reader.take;
   % data: an array of topic class instances (e.g. ShapeType); possibly empty
   % dataState: an struct array; each entry describes the
   %    state of the corresponding data entry

Read data from a data reader::

   % reader: a Vortex.Reader
   [data, dataState] = reader.read;
   % data: an array of topic class instances (e.g. ShapeType); possibly empty
   % dataState: an struct array; each entry describes the
   %    state of the corresponding data entry

Specify a wait timeout, in seconds, before read or take will return without receiving data::

   % reader: a Vortex.Reader
   reader.waitsetTimeout(2.0);

Vortex.Query
************

The MATLAB ``Vortex.Query`` class represents a DDS query entity.

A query is a data reader, restricted to accessing data that matches specific status conditions and/or a filter expression.

A ``Vortex.Query`` class instance references an existing ``Vortex.Reader`` instance.

**API Examples**

Create a ``Vortex.Query`` instance or throw a ``Vortex.DDSException`` if an error occurs.

Create a query based on a state mask only::

   % reader: a Vortex.Reader
   % only receive samples that:
   %   * have not been read by this application
   %   * AND for instances that previously seen by this application
   %   * AND for which there is a live writer
   mask = Vortex.DataState.withNew().withNotRead().withAlive();
   query = Vortex.Query(reader, mask);

Create a query based on a state mask and a filter expression::

   % reader: a Vortex.Reader
   mask = Vortex.DataState.withAnyState();
   filter = 'color = %0 and x > %1';
   % filter for 'RED' shapes with x > 10...
   query = Vortex.Query(reader, mask, filter, {'RED', 10'});

Take data from a query::

   % query: a Vortex.Query
   [data, dataState] = query.take;
   % data: an array of topic class instances (e.g. ShapeType); possibly empty
   % dataState: an struct array; each entry describes the
   %    state of the corresponding data entry

Read data from a query::

   % query: a Vortex.Query
   [data, dataState] = query.read;
   % data: an array of topic class instances (e.g. ShapeType); possibly empty
   % dataState: an struct array; each entry describes the
   %    state of the corresponding data entry

Specify a wait timeout, in seconds, before read or take will return without receiving data::

   % query: a Vortex.Query

   % specify the waitset timeout on the reader
   query.waitsetTimeout(2.0);

   % now, read or take 'query'

Vortex.DDSException
*******************

The ``Vortex.DDSException`` class is thrown in the case of a DDS error arising.
The class can also be used to decode an error status code returned by methods such as ``Vortex.Writer.write``.

**API Examples**

Catch a DDS error while creating a DDS entity::

    % dp: a Vortex.DomainParticipant
    try
      topic = Vortex.topic('Circle', ?SomeAlternateDef.ShapeType);
    catch ex
      switch ex.identifier
        case 'Vortex:DDSError'
          % it's a Vortex Error
          fprintf(['DDS reports error:\n' ...
             '  %s\n' ...
             '    DDS ret code: %s (%d)\n'], ...
             ex.message, char(ex.dds_ret_code), ex.dds_ret_code);
        otherwise
          rethrow ex;
      end
    end

Decode a dds status code returned by ``Vortex.Writer.write``::

   % ddsstatus: a Vortex.Writer.write return value
   ex = Vortex.DDSException('', ddsstatus);
   switch ex.dds_ret_code
     case Vortex.DDSReturnCode.DDS_RETCODE_OK
       % ...
     case Vortex.DDSReturnCode.DDS_BAD_PARAMETER
       % ...
     case Vortex.DDSReturnCode.DDS_RETCODE_INCONSISTENT_POLICY
       % ...
   end

