/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

'use strict';
/* eslint-env node, mocha */
/* eslint no-new: 0 */ // --> turn off no-new

const dcps = require('./dcps');
const ddserr = require('./ddserr');
const expect = require('chai').expect;

/* Topic Descriptor Information */
const sampleTypeName = 'HelloWorldData::Msg';
const sampleTypeKey = 'userID';
const sampleTopicXML = '<MetaData version="1.0.0">' +
  '<Module name="HelloWorldData">' +
  '<Struct name="Msg">' +
  '<Member name="userID"><Long/></Member>' +
  '<Member name="message"><Long/></Member>' +
  '</Struct>' +
  '</Module>' +
  '</MetaData>';
const sampleTopicName = 'HelloWorldData_Msg';


describe('DCPS create and delete tests', function() {

  it('participantCreate create a domain participant within 10s', function() {
    this.timeout(10000);
    var dp = new dcps.Participant();
    expect(dp.handle).to.not.be.null;
  });

  it('participantCreate fails with invalid domainId', function() {
    expect(function() {
      new dcps.Participant(500);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);
  });

  it('delete twice test', function() {
    this.timeout(10000);
    var dp = new dcps.Participant();
    dp.delete();
    expect(function() {
      dp.delete();
    }).not.to.throw(Error);
  });

  /* TopicDescriptor class tests */
  it('create a topic descriptor', function() {
    var topicDescriptor = new dcps.TopicDescriptor(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    expect(topicDescriptor.cAddr).to.not.be.null;
  });

  it('delete a topic descriptor', function() {
    var topicDescriptor = new dcps.TopicDescriptor(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    expect(function() {
      topicDescriptor.delete();
    }).not.to.throw(Error);
  });

  it('delete a topic descriptor twice', function() {
    var topicDescriptor = new dcps.TopicDescriptor(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    topicDescriptor.delete();
    expect(function() {
      topicDescriptor.delete();
    }).not.to.throw(Error);
  });

  it('accessing the c address after a delete throws an error', function() {
    var topicDescriptor = new dcps.TopicDescriptor(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    topicDescriptor.delete();
    expect(function() {
      topicDescriptor.cAddr;
    }).to.throw(ReferenceError);
  });

  it('get entity qos test', function() {
    var dp = new dcps.Participant();
    var pqos = dp.qos;
    expect(pqos).to.not.be.null;
    dp.delete();
    pqos.delete();
  });

  it('get topic descriptor name,key and metadescriptor', function() {
    var topicDescriptor = new dcps.TopicDescriptor(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    expect(topicDescriptor.typeName).to.equal(sampleTypeName);
    expect(topicDescriptor.keyList).to.equal(sampleTypeKey);
    expect(topicDescriptor.metaDescriptor).to.equal(sampleTopicXML);
  });

  /* End of TopicDescriptor class tests */
});

describe('DCPS tests', function() {

  function createSampleTopic(topicname) {
    return dp.createTopic(
      topicname,
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
  }
  /*
  function createSampleTopicWithQos(topicname) {
    let tqos = ddsqos.QoS.topicDefault();
    tqos.durability = ddsqos.DurabilityKind.Transient;
    tqos.reliability = ddsqos.ReliabilityKind.Reliable;
    console.log('tqos = ', tqos);
    return dp.createTopic(
      topicname,
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML,
      tqos
    );
  }
*/
  var dp = null;

  before(function() {
    dp = new dcps.Participant();
  });

  after(function() {
    dp.delete();
  });

  it('createTopic function test', function() {
    var topic = createSampleTopic('SampleTopic');
    expect(topic).to.not.be.null;
  });

  it('createReader function test', function() {
    var topic = createSampleTopic('SampleTopic');
    var reader = dp.createReader(topic);
    expect(reader).to.not.be.null;
  });

  it('createWriter function test', function() {
    var topic = createSampleTopic('SampleTopic');
    var writer = dp.createWriter(topic);
    expect(writer).to.not.be.null;
  });

  it('createPublisher function test', function() {
    var publisher = dp.createPublisher();
    expect(publisher).to.not.be.null;
  });

  it('createSubscriber function test', function() {
    var subscriber = dp.createSubscriber();
    expect(subscriber).to.not.be.null;
  });

  /* Topic class tests */
  it('Topic create fails with null topicHandle', function() {
    expect(function() {
      new dcps.Topic(dp, 'HelloWorldData_Msg', null, null, null);
    }).to.throw(Error);
  });

  it('create a topic', function() {
    var topic = new dcps.Topic(
      dp,
      'HelloWorldData_Msg',
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    expect(topic.handle).to.not.be.null;
  });

  it('findTopic finds a topic on the wire', function() {
    expect(dp.findTopic('HelloWorldData_Msg')).to.not.be.null;
  });

  it('gets the descriptor and topic name', function() {
    var topic = createSampleTopic('SampleTopic');
    expect(topic.name).to.equal('SampleTopic');
    expect(topic.descriptor).to.not.be.null;
  });

  /* End of Topic class tests */

  /* Writer class tests */
  it('Writer create fails with null topic', function() {
    expect(function() {
      new dcps.Writer(dp, null, null, null);
    }).to.throw(TypeError);
  });

  it('create a writer', function() {
    var topic = createSampleTopic('HelloWorldData_Msg');
    var writer = new dcps.Writer(dp, topic);
    expect(writer.handle).to.not.be.null;
  });

  it('write data', function() {
    var topic = createSampleTopic('HelloWorldData_Msg');
    var writer = new dcps.Writer(dp, topic);
    var sampleJSObj = {
      userID: 5,
      message: 14,
    };
    var status = writer.write(sampleJSObj);
    expect(status).to.be.equal(0);
  });

  /* End of Wrtier class tests */

  /* Reader class tests */
  it('create a reader', function() {
    var topic = createSampleTopic('HelloWorldData_Msg');
    var reader = new dcps.Reader(dp, topic);
    expect(reader.handle).to.not.be.null;
  });

  it('Reader create fails with null topic', function() {
    expect(function() {
      new dcps.Reader(dp, null);
    }).to.throw(TypeError);
  });

  it('read sample', function() {
    var topic = createSampleTopic('HelloWorldData_Msg');
    var reader = new dcps.Reader(dp, topic);
    var writer = new dcps.Writer(dp, topic);

    var numberOfSample = 15;

    // write 15 messages
    for (let i = 0; i < numberOfSample; i++) {
      let sampleJSObj = {
        userID: i,
        message: i + 1,
      };
      writer.write(sampleJSObj);
    }

    var readArray = reader.read(numberOfSample);
    expect(readArray.length).to.equal(numberOfSample);
  });

  it('read sample with condition', function() {
    var topic = createSampleTopic('HelloWorldData_Msg');
    var reader = new dcps.Reader(dp, topic);
    var writer = new dcps.Writer(dp, topic);

    var numberOfSample = 15;

    // write 15 messages
    for (let i = 0; i < numberOfSample; i++) {
      let sampleJSObj = {
        userID: i,
        message: i + 1,
      };
      writer.write(sampleJSObj);
    }

    var readArray = reader.readCond(
      numberOfSample,
      new dcps.ReadCondition(reader, dcps.StateMask.any)
    );
    expect(readArray.length).to.equal(numberOfSample);
    expect(readArray[0].info.valid_data).to.equal(true);
  });

  it('take invalid data should return a null sample', function() {
    const topicDesc = '<MetaData version="1.0.0">' +
      '<Struct name="PingPongData3">' +
      '<Member name="userID"><Long/></Member>' +
      '<Member name="message"><Long/></Member>' +
      '</Struct>' +
      '</MetaData>';
    var participant = new dcps.Participant();
    var participant2 = new dcps.Participant();
    var topic = new dcps.Topic(participant,
      'pingpong_topic2',
      'PingPongData3',
      'userID',
      topicDesc);
    var writer = new dcps.Writer(participant, topic);
    var reader = new dcps.Reader(participant2, topic);
    var numberOfSample = 10;

    for (let i = 0; i < numberOfSample; i++) {
      writer.write({ userID: i + 1, message: i + 2 });
    }
    reader.take(numberOfSample);
    participant.delete();
    var readArray = reader.take(numberOfSample);
    expect(readArray[0].info.valid_data).to.equal(false);
    expect(readArray[0].sample.userID).to.equal(1);
    expect(readArray[0].sample.message).to.equal(null);
    participant2.delete();
  });

  it('take sample', function() {
    var topic = createSampleTopic('HelloWorldData_Msg2');
    var reader = new dcps.Reader(dp, topic);
    var writer = new dcps.Writer(dp, topic);

    var numberOfSample = 15;

    // write 15 messages
    for (let i = 0; i < numberOfSample; i++) {
      let sampleJSObj = {
        userID: i,
        message: i + 1,
      };
      writer.write(sampleJSObj);
    }

    var readArray = reader.take(numberOfSample);
    expect(readArray.length).to.equal(numberOfSample);
  });

  it('take sample with condition', function() {
    var topic = createSampleTopic('HelloWorldData_Msg2');
    var reader = new dcps.Reader(dp, topic);
    var writer = new dcps.Writer(dp, topic);

    var numberOfSample = 15;

    // write 15 messages
    for (let i = 0; i < numberOfSample; i++) {
      let sampleJSObj = {
        userID: i,
        message: i + 1,
      };
      writer.write(sampleJSObj);
    }

    var readArray = reader.takeCond(
      numberOfSample,
      new dcps.ReadCondition(reader, dcps.StateMask.any)
    );
    expect(readArray.length).to.equal(numberOfSample);
  });
  /* End of Reader class tests */

  /* Subscriber class tests */
  it('create a subscriber', function() {
    var subscriber = new dcps.Subscriber(dp);
    expect(subscriber.handle).to.not.be.null;
  });

  it('create a reader using subscriber', function() {
    var subscriber = new dcps.Subscriber(dp);
    var topic = createSampleTopic('HelloWorldData_Msg');
    var reader = subscriber.createReader(topic);
    expect(reader.handle).to.not.be.null;
  });

  it('Subscriber create fails with null participant', function() {
    expect(function() {
      new dcps.Subscriber(null);
    }).to.throw(TypeError);
  });
  /* End of Subscriber class tests */

  /* Publisher class tests */
  it('create a publisher', function() {
    var publisher = new dcps.Publisher(dp);
    expect(publisher.handle).to.not.be.null;
  });

  it('create a writer using publisher', function() {
    var publisher = new dcps.Publisher(dp);
    var topic = createSampleTopic('HelloWorldData_Msg');
    var writer = publisher.createWriter(topic);
    expect(writer.handle).to.not.be.null;
  });

  it('Publisher create fails with null participant', function() {
    expect(function() {
      new dcps.Publisher(null);
    }).to.throw(TypeError);
  });
  /* End of Publisher class tests */

  it('Wait for historical data test (blocking)', function() {
    let rd = new dcps.Reader(dp, createSampleTopic('HelloWorldData_Msg'));
    expect(function() {
      rd.waitForHistoricalDataBlocking(0);
    }).to.not.throw(Error);
  });

  it('Can asynchronously wait for historical data', function(done) {
    // let topic = createSampleTopic('HelloWorldData_Msg');
    let rd = new dcps.Reader(dp, createSampleTopic('HelloWorldData_Msg'));
    // expect to complete right away with no error
    rd.waitForHistoricalData(0, function(err, res) {
      expect(err).to.be.null;
      expect(res).to.equal(0);
      done();
    });
  });


});

describe('Conditions and Waitset tests', function() {
  var dp = null;
  var reader = null;
  var topic = null;

  function createHelloWorldTopic() {

    return dp.createTopic(
      sampleTopicName,
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML);
  }

  before(function() {
    dp = new dcps.Participant();
    topic = createHelloWorldTopic();
    reader = new dcps.Reader(dp, topic);
  });

  after(function() {
    dp.delete();
  });

  it('Read condition tests', function() {
    var cond = new dcps.ReadCondition(reader, dcps.StateMask.any);
    expect(cond).to.not.be.null;
    expect(cond.handle).to.not.be.null;
    expect(cond.triggered()).to.equal(false);
    expect(function() {
      cond.delete();
    }).to.not.throw(Error);
  });

  it('Guard condition tests', function() {
    var guardCond = new dcps.GuardCondition();
    expect(guardCond).to.not.be.null;
    expect(guardCond.triggered()).to.equal(false);
    guardCond.trigger();
    expect(guardCond.triggered()).to.equal(true);
    guardCond.reset();
    expect(guardCond.triggered()).to.equal(false);
  });

  it('StatusCondition tests', function() {
    let statCond = new dcps.StatusCondition(dp);
    expect(statCond).to.not.be.null;
    statCond.enable(dcps.StatusMask.data_available);
    expect(statCond.mask).to.equal(dcps.StatusMask.data_available);

    let statCond2 = new dcps.StatusCondition(dp,
      dcps.StatusMask.data_available);
    expect(statCond2.mask).to.equal(dcps.StatusMask.data_available);
  });

  it('Query Condition tests', function() {
    var queryCond = new dcps.QueryCondition(
      reader,
      dcps.StateMask.any,
      'userID = %0', ['10'],
      1
    );
    expect(queryCond).to.not.be.null;
  });

  it('Can wait on a waitset and timeout', function() {
    var ws = new dcps.Waitset();
    var cond1 = new dcps.ReadCondition(reader, dcps.StateMask.any);
    var cond2 = new dcps.GuardCondition();
    var cond3 = new dcps.GuardCondition();
    var cond4 = new dcps.GuardCondition();

    expect(function() {
      ws.attach(cond1);
      ws.attach(cond2);
      ws.attach(cond3);
      ws.attach(cond4);
    }).to.not.Throw(Error);

    expect(ws.conditions.size).to.equal(4);

    expect(function() {
      ws.detach(cond1);
      ws.detach(cond2);
    }).to.not.Throw(Error);

    expect(ws.conditions.size).to.equal(2);

    // wait for 1.5 seconds and time out
    expect(function() {
      ws.waitBlocking(1.5 * 1000000000);
    }).to.not.throw(Error);

    // delete ws and expect no error
    expect(function() {
      ws.delete();
    }).to.not.throw(Error);
  });

  it('Can call the Waitset constructor with a condition', function() {
    let ws = new dcps.Waitset(new dcps.GuardCondition());
    expect(ws.conditions.size).to.equal(1);
  });

  it('Can wait on a waitset and unblock when condition is met', function() {
    let waitset = new dcps.Waitset();
    let statCond = new dcps.StatusCondition(reader);
    statCond.enable(dcps.StatusMask.data_available);
    waitset.attach(statCond, reader);

    let writer = dp.createWriter(topic);
    writer.write({
      userID: 1,
      message: 'foo',
    });

    // expect the waitset to unblock immediately
    let handles = waitset.waitBlocking(0.20 * Math.pow(10, 9));
    expect(handles).to.be.an.instanceof(Array);
    expect(handles[0]).to.be.an.instanceof(dcps.Reader);

    // test wait with default arg
    writer.write({
      userID: 1,
      message: 'foo',
    });
    expect(function() {
      waitset.waitBlocking();
    }).to.not.throw(Error);
  });

  // can asyncronously wait and timeout
  it('Can asynchronously wait on a waitset and timeout', function(done) {
    let ws = new dcps.Waitset();
    ws.wait(Math.pow(10, 9), function(err, res) {
      expect(err).to.be.instanceof(Error);
      expect(err.message).to.equal('10'); // 10 = DDS_RETCODE_TIMEOUT
      expect(res).to.equal(0); // ddsc99.waitsetWait returns 0 on timeout
      done();
    });
  });

  it('Can asynchronously wait on a waitset and unblock when '
    + 'the condition is met', function(done) {
    let ws = new dcps.Waitset();
    let cond = new dcps.GuardCondition();
    cond.trigger();
    ws.attach(cond, 'guard');
    ws.wait(Math.pow(10, 9), function(err, res) {
      expect(err).to.be.null;
      expect(res).to.equal(1);
      done();
    });
  });

  it('Can create a status condition on each type of entity.', function() {
    // for a participant
    let dp = new dcps.Participant();
    let statCond = new dcps.StatusCondition(dp);
    expect(function() {
      statCond.enable(dcps.StatusMask.data_available);
    }).to.not.throw(Error);

    // for a publisher
    let pub = dp.createPublisher();
    statCond = new dcps.StatusCondition(pub);
    expect(function() {
      statCond.enable(dcps.StatusMask.publication_matched);
    }).to.not.throw(Error);

    // for a writer
    let wr = pub.createWriter(topic);
    statCond = new dcps.StatusCondition(wr);
    expect(function() {
      statCond.enable(dcps.StatusMask.publication_matched);
    }).to.not.throw(Error);

    // for a topic
    statCond = new dcps.StatusCondition(topic);
    expect(function() {
      statCond.enable(dcps.StatusMask.inconsistent_topic);
    }).to.not.throw(Error);

    // for a subscriber
    let sub = dp.createSubscriber();
    statCond = new dcps.StatusCondition(sub);
    expect(function() {
      statCond.enable(dcps.StatusMask.data_on_readers);
    }).to.not.throw(Error);

    // for a reader
    let rd = sub.createReader(topic);
    statCond = new dcps.StatusCondition(rd);
    expect(function() {
      statCond.enable(dcps.StatusMask.subscription_matched);
    }).to.not.throw(Error);

    // get an error as the mask does not correspond to the entity
    // inconsistent topic is not valid for a reader
    expect(function() {
      statCond.enable(dcps.StatusMask.inconsistent_topic);
    }).to.throw(Error);
  });
});

describe('Listener tests', function() {
  let dp = null;
  let topic = null;
  let reader = null;
  let writer = null;

  function createHelloWorldTopic() {
    return dp.createTopic(
      sampleTopicName + '55',
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML);
  }

  before(function() {
    dp = new dcps.Participant();
    topic = createHelloWorldTopic();
    reader = new dcps.Reader(dp, topic);
    writer = new dcps.Writer(dp, topic);
  });

  after(function() {
    dp.delete();
  });

  it('Can add a listener to a reader', function(done) {
    let count = 0;
    // if we don't reference writer, lint will complain
    // we need the writer to get subscription matched
    expect(writer).to.not.be.null;
    reader.addListener({
      subscription_matched: function() { count++; done(); /* done(); */ },
    }, function() { return (count > 0); });
  });

  it('Cannot attach listener of wrong type to entity', function() {
    expect(() => reader.addListener(
      { inconsistent_topic: undefined })).to.throw(Error);
  });

  it('Cannot attach listener whose name is not a valid communication status',
    function() {
      expect(() => reader.addListener({
        foo: undefined,
      })).to.throw(Error);
    });

});

describe('readStatus, takeStatus, enabledStatuses, statusChanges tests',
  function() {
    let dp = null;

    before(function() {
      dp = new dcps.Participant();
    });

    after(function() {
      dp.delete();
    });

    it('Can get the status of an entity', function() {
      expect(dp.readStatus(0)).to.be.a('number');
      expect(dp.takeStatus(0)).to.be.a('number');
      expect(dp.enabledStatuses()).to.be.a('number');
      expect(dp.statusChanges()).to.be.a('number');
    });
  });
