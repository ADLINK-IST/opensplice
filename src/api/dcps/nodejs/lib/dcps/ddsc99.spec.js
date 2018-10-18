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

const dds = require('./ddsc99');
const ddserr = require('./ddserr');
const ddstopic = require('./ddstopic');
const ddsqos = require('./qos');
const expect = require('chai').expect;
const ref = require('ref');
const os = require('os');

/* sample topic data */
const sampleTopicXML = '<MetaData version="1.0.0">' +
  '<Module name="HelloWorldData">' +
  '<Struct name="Msg">' +
  '<Member name="userID"><Long/></Member>' +
  '<Member name="message"><Long/></Member>' +
  '</Struct>' +
  '</Module>' +
  '</MetaData>';
const sampleTypeName = 'HelloWorldData::Msg';
const sampleTypeKey = 'userID';
const sampleTopicName = 'HelloWorldData_Msg';

const pvoid = ref.refType(ref.types.void);

function emptycallback() { };

// test the environment
describe('OSPL environment', function() {
  it('Has non-empty OSPL_HOME', function(done) {
    this.timeout(10000);
    expect(process.env.OSPL_HOME).to.not.be.a('null');
    done();
  });
});


describe('FFI interface to C99 - participant', function() {
  if (os.platform() === 'linux') {
    it('participantCreate fails with invalid OSPL_URI', function() {

      this.timeout(10000);
      const savedOsplURI = process.env.OSPL_URI;
      process.env.OSPL_URI = savedOsplURI + 'XXX';

      // expect failure with the default callback
      expect(function() { dds.participantCreate(); }).to.throw(ddserr.DDSError)
        .with.property('ddsErrCode').to.be.within(1, 12); ;

      // expect failure with the empty callback
      expect(dds
        .participantCreate(dds.DDS_DOMAIN_DEFAULT, null, null, emptycallback))
        .to.be.a('null');

      // change the URI back
      process.env.OSPL_URI = savedOsplURI;

    });
  }
});

describe('FFI interface to C99', function() {

  var dp = null;
  var typeSupport = null;

  before(function() {
    dp = dds.participantCreate();
    typeSupport = new ddstopic.TypeSupport(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
  });

  after(function() {
    dds.entityDelete(dp);
  });

  /* init and fini work as expected */
  it('init and fini work as expected', function() {
    // the only error that we can detect is a TypeError thrown from FFI
    expect(function() { dds.init(null, null); }).to.not.throw(Error);
    expect(function() { dds.fini(); }).to.not.throw(Error);
  });
  /* end of init and fini tests */

  /* topicDescriptorCreate tests */
  it('topicDescriptorCreate creates a topic descriptor',
    function() {
      expect(function() {
        dds.topicDescriptorCreate(
          sampleTypeName,
          sampleTypeKey,
          sampleTopicXML
        );
      }).not.to.throw(Error);
    });

  it('topicDescriptorCreate fails to create topic descriptor with invalid '
    + 'arguments',
  function() {
    // the only way I know how to make this call fail is to put in the wrong
    // type of arguments so FFI fails. So we'll just test with (0,0,0) for
    // now.

    // expect failure with default callback
    expect(function() { dds.topicDescriptorCreate(0, 0, 0); })
      .to.throw(TypeError);
    // expect failure with empty callback.
    expect(dds.topicDescriptorCreate(0, 0, 0, emptycallback))
      .to.be.a('null');
  });

  it('delete a topic descriptor', function() {
    var topicDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    expect(function() {
      dds.topicDescriptorDelete(topicDesc);
    }).not.to.throw(Error);
  });

  it('topicDescriptorDelete throws exception for invalid argument', function() {
    expect(function() {
      dds.topicDescriptorDelete(0);
    }).to.throw(TypeError);
  });
  /* end of topicDescriptorCreate tests */

  /* publisherCreate tests */
  it('publisherCreate creates a publisher', function() {
    var pub = dds.publisherCreate(dp);
    expect(pub).to.not.be.a('null');
  });

  it('publisherCreate fails with invalid arguments',
    function() {
      // We can make publisherCreate fail by passing in a random pointer
      // instead of a valid dp. This results in a negative status
      // which turns into an error.

      // expect failure with default callback
      expect(function() { dds.publisherCreate(ref.alloc(ref.types.void)); })
        .to.throw(Error);
      // expect failure with empty callback
      expect(dds.publisherCreate(ref.alloc(ref.types.void), null, null,
        emptycallback)).to.be.a('null');
    });
  /* end of publisherCreate tests */

  it('entityDelete fails with an invalid argument',
    function() {
      // The only way I can make this fail is by passing in 0. FFI throws an
      // error

      // expect failure with default callback
      expect(function() { dds.entityDelete(0); })
        .to.throw(Error);
      // expect failure with empty callback
      expect(dds.entityDelete(0, emptycallback)).to.be.a('null');
    });

  /* topicFind tests */
  it('topicFind finds a published topic', function() {
    var desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var topic = dds.topicCreate(dp, desc, 'hello');
    expect(topic).to.not.be.a('null');
    var foundTopic = dds.topicFind(dp, 'hello');
    expect(foundTopic).to.not.be.a('null');
  });

  it('topicFind fails to find a topic when there is no topic on the domain '
    + 'participant', function() {
    // for the moment, the only way i'll make the call fail is by passing in 0

    // expect failure with default callback
    expect(function() { dds.topicFind(0, sampleTopicName); }).to.throw(Error);
    // expect failure with empty callback
    expect(dds.topicFind(0, sampleTopicName, emptycallback)).to.be.a('null');
  });
  /* end of topicFind tests */

  /* topicGetTypeName tests */
  it('topicGetTypeName gets the type name of a created topic', function() {
    var desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var top = dds.topicCreate(dp, desc, sampleTopicName);
    expect(dds.topicGetTypeName(top)).to.equal(sampleTypeName);
  });

  it('topicGetTypeName fails when the topic is invalid (0)', function() {
    // pass in 0 to force the call to fail
    expect(function() { dds.topicGetTypeName(0); })
      .to.throw(Error);
    // expect failure with empty callback
    expect(dds.topicGetTypeName(0, emptycallback)).to.be.a('null');
  });
  /* end of topicGetTypeName tests */

  /* topicGetKeylist tests */
  it('topicGetKeylist gets the keylist of a created topic', function() {
    var desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var top = dds.topicCreate(dp, desc, sampleTopicName);
    expect(dds.topicGetKeylist(top)).to.equal('userID');
  });

  it('topicGetKeylist fails when the topic is invalid', function() {
    // expect failure with default callback
    expect(function() { dds.topicGetKeylist(0); }).to.throw(Error);
    // expect failure with empty callback
    expect(dds.topicGetKeylist(0, emptycallback)).to.be.a('null');
  });
  /* end of topigGetKeylist tests */

  /* topicGetMetadescriptor tests */
  it('topicGetMetadescriptor gets the metadescriptor of a created topic',
    function() {
      var desc = dds.topicDescriptorCreate(
        sampleTypeName,
        sampleTypeKey,
        sampleTopicXML
      );
      var top = dds.topicCreate(dp, desc, sampleTopicName);
      expect(dds.topicGetMetadescriptor(top)).to
        .equal(sampleTopicXML);
    });

  it('topicGetMetadescriptor fails when the topic is invalid', function() {
    // expect failure with default callback
    expect(function() { dds.topicGetMetadescriptor(0); }).to.throw(Error);
    // expect failure with empty callback
    expect(dds.topicGetMetadescriptor(0, emptycallback)).to.be.a('null');
  });
  /* end of topicGetMetadecsriptor tests */

  /* topicGetName tests */
  it('topicGetName gets the name of a created topic', function() {
    var desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var top = dds.topicCreate(dp, desc, sampleTopicName);
    expect(dds.topicGetName(top)).to.equal(sampleTopicName);
  });

  it('topicGetName fails when the topic (first argument) is invalid',
    function() {
      // expect failure with default callback
      expect(function() { dds.topicGetName(0); }).to.throw(Error);
      // expect failure with empty callback
      expect(dds.topicGetName(0, emptycallback)).to.be.a('null');
    });
  /* end of topicGetName tests */

  /* end of topic tests */

  /* write tests */
  it('write writes a message successfully', function() {
    var desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var top = dds.topicCreate(dp, desc, sampleTopicName);
    var wr = dds.writerCreate(dp, top);

    var sampleJSObj = { userID: 100, message: 200 };
    var data = typeSupport.copyin(sampleJSObj);
    var status = dds.write(wr, data);

    expect(status).to.not.be.a('null');
  });

  it('write fails with invalid writer (first argument)', function() {
    var sampleJSObj = { userID: 1, message: 2 };
    var data = typeSupport.copyin(sampleJSObj);
    // expect failure with default callback. FFI throws an error here
    expect(function() { dds.write(0, data); })
      .to.throw(TypeError);
    // expect failure with empty callback. dds_write returns an error here.
    // try to write on a domain participant -- dds_write will return an error.
    expect(dds.write(dds.participantCreate(), data,
      emptycallback))
      .to.be.a('null');
  });
  /* end of write tests */

  /* read and take tests */
  it('can read 10 messages successfully', function() {
    var sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    var wr = dds.writerCreate(dp, topic);
    var rd = dds.readerCreate(dp, topic);

    // write 10 messages
    for (let i = 0; i < 10; i++) {
      let sampleJSObj = { userID: i, message: 100 + i };
      let data = typeSupport.copyin(sampleJSObj);
      dds.write(wr, data);
    }

    // read our messages into an array
    var readArray = dds.read(rd, 10, typeSupport.getRefType());
    expect(readArray.length).to.equal(10);
    for (let i = 0; i < 10; i++) {
      expect(readArray[i].sample.userID).to.equal(i);
      expect(readArray[i].sample.message).to.equal(100 + i);
    }
  });

  /* read and take tests */


  it('can take 10 messages successfully', function() {
    var sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    var wr = dds.writerCreate(dp, topic);
    var rd = dds.readerCreate(dp, topic);

    // write 10 messages
    for (let i = 0; i < 10; i++) {
      let sampleJSObj = { userID: i, message: 100 + i };
      let data = typeSupport.copyin(sampleJSObj);
      dds.write(wr, data);
    }

    // read our messages into an array
    var readArray = dds.take(rd, 10, typeSupport.getRefType());
    expect(readArray.length).to.equal(10);
    for (let i = 0; i < 10; i++) {
      expect(readArray[i].sample.userID).to.equal(i);
      expect(readArray[i].sample.message).to.equal(100 + i);
    }
  });

  it('can take 10 messages successfully with condition', function() {
    var sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    var wr = dds.writerCreate(dp, topic);
    var rd = dds.readerCreate(dp, topic);

    // write 10 messages
    for (let i = 0; i < 10; i++) {
      var sampleJSObj = { userID: i, message: 100 + i };
      var data = typeSupport.copyin(sampleJSObj);
      dds.write(wr, data);
    }

    // read our messages into an array
    var readArray = dds.takeCond(
      rd,
      10,
      typeSupport.getRefType(),
      dds.readConditionCreate(rd, 127)
    );
    expect(readArray.length).to.equal(10);
    for (let i = 0; i < 10; i++) {
      expect(readArray[i].sample.userID).to.equal(i);
      expect(readArray[i].sample.message).to.equal(100 + i);
    }

  });


  it('read fails to read with invalid arguments', function() {
    // expect failure with default callback
    expect(function() { dds.read(0, 0, 0); }).to.throw(TypeError);

    var pvoid = ref.refType(ref.types.void);
    expect(function() {
      dds.read(ref.alloc(pvoid), 10, typeSupport.getRefType());
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    // expect failure with empty callback
    expect(dds.read(null, null, null, emptycallback)).to.be.a('null');
  });

  it('readCond fails to read with invalid arguments', function() {
    // expect failure with default callback
    expect(function() { dds.readCond(0, 0, 0, 0); }).to.throw(TypeError);

    var pvoid = ref.refType(ref.types.void);
    expect(function() {
      dds.readCond(ref.alloc(pvoid), 10, typeSupport.getRefType(),
        dds.guardConditionCreate());
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    // expect failure with empty callback
    expect(dds.readCond(null, null, null, null, emptycallback)).to.be.a('null');
  });

  it('takeCond fails to read with invalid arguments', function() {
    // expect failure with default callback
    expect(function() { dds.takeCond(0, 0, 0, 0); }).to.throw(TypeError);

    var pvoid = ref.refType(ref.types.void);
    expect(function() {
      dds.takeCond(
        ref.alloc(pvoid),
        10,
        typeSupport.getRefType(),
        dds.guardConditionCreate()
      );
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    // expect failure with empty callback
    expect(dds.takeCond(null, null, null, null, emptycallback)).to.be.a('null');
  });

  it('take fails to read with invalid arguments', function() {
    var pvoid = ref.refType(ref.types.void);
    expect(function() {
      dds.take(
        ref.alloc(pvoid),
        10,
        typeSupport.getRefType()
      );
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    // expect failure with empty callback
    expect(dds.take(null, null, null, emptycallback)).to.be.a('null');
  });

  it('read, take, readCond, takeCond do not return the loan when they '
    + 'receive nothing', function() {
    var sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    var topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    var rd = dds.readerCreate(dp, topic);
    let cond = dds.readConditionCreate(rd, 127);

    expect(dds.read(rd, 1, typeSupport.getRefType())).to.not.be.null;
    expect(dds.take(rd, 1, typeSupport.getRefType())).to.not.be.null;
    expect(dds.readCond(rd, 1, typeSupport.getRefType(), cond)).to.not.be.null;
    expect(dds.takeCond(rd, 1, typeSupport.getRefType(), cond)).to.not.be.null;
  });
  /* end of read tests */

  it('free frees a topic descriptor successfully', function() {
    expect(function() {
      var desc = dds.topicDescriptorCreate(
        sampleTypeName,
        sampleTypeKey,
        sampleTopicXML
      );
      dds.free(desc);
    }).to.not.throw(Error);
  });

  it('free fails to free given an invalid pointer', function() {
    // expect failure with default callback
    expect(function() { dds.free(0); }).to.throw(TypeError);
    // expect failure with empty callback
    expect(dds.free(0, emptycallback)).to.be.a('null');
  });

  it('can wait for historical data (blocking)', function() {
    let sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    let rd = dds.readerCreate(dp, topic);

    // have it succeed as expected
    expect(dds.readerWaitForHistoricalDataBlocking(rd, 0)).to.not.be.null;

    // have it fail and throw TypeError
    expect(function() {
      dds.readerWaitForHistoricalDataBlocking('foo', 0);
    }).to.throw(TypeError);

    // have it fail and throw a DDSError
    expect(function() {
      dds.readerWaitForHistoricalDataBlocking(rd, -1);
    }).to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);

    // have it fail and get the null value
    expect(dds.readerWaitForHistoricalDataBlocking(rd, -1, emptycallback))
      .to.be.null;
  });

  it('participantCreate fails with invalid arguments',
    function() {
      this.timeout(10000);
      // we need 0 <= domainId <= 230
      // expect non-zero status to turn into an Error
      expect(function() { dds.participantCreate(231); })
        .to.throw(ddserr.DDSError).with
        .property('ddsErrCode').to.be.within(1, 12);
    });
});

describe('FFI interface to C99 - Waitsets', function() {
  var dp = null;
  var reader = null;
  var topic = null;
  var typeSupport = null;

  before(function() {
    dp = dds.participantCreate();
    let sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    reader = dds.readerCreate(dp, topic);
    typeSupport = new ddstopic.TypeSupport(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
  });

  after(function() {
    dds.entityDelete(dp);
  });

  it('Can create and delete a waitset', function() {
    // successfully create and delete a waitset
    var ws = dds.waitsetCreate();
    expect(ws).to.not.be.null;
    expect(dds.waitsetDelete(ws)).to.not.be.null;

    // expect deleting a random pointer to throw an error
    expect(function() { dds.waitsetDelete(null); })
      .to.throw(Error);

    // expect deleting a random pointer be null with no callback
    var ret = dds.waitsetDelete(null, emptycallback);
    expect(ret).to.be.null;
  });

  it('Can create and delete a read condition', function() {
    // create and delete as usual
    var cond = dds.readConditionCreate(reader, 127);
    expect(cond).to.not.be.null;
    expect(function() {
      dds.conditionDelete(cond);
    }).to.not.throw(Error);

    // try to create a read condition with invalid reader. expect error
    expect(function() {
      dds.readConditionCreate('invalid_reader', 127);
    }).to.throw(TypeError);

    // get the null value
    expect(dds.readConditionCreate('invalid_reader', 127, emptycallback))
      .to.be.null;

    // try to delete a string. expect to throw an error
    expect(function() {
      dds.conditionDelete('invalid_input');
    }).to.throw(Error);

    // try to delete a string. expect to throw an error
    expect(dds.conditionDelete('invalid_input', emptycallback)).to.be.null;
  });

  it('Can create and delete a guard condition', function() {
    var cond = dds.guardConditionCreate();
    expect(cond).to.not.be.null;
  });

  it('Can trigger and reset a guard condition', function() {
    var cond = dds.guardConditionCreate();

    // trigger it and expect it to work
    dds.guardTrigger(cond);
    expect(dds.conditionTriggered(cond)).to.equal(true);

    // reset it and expect it to work
    dds.guardReset(cond);
    expect(dds.conditionTriggered(cond)).to.equal(false);

    // try to reset a string. expect it to fail
    expect(function() {
      dds.guardTrigger('invalid_cond');
    }).to.throw(Error);

    // try to reset a string.
    expect(dds.guardTrigger('invalid_cond', emptycallback)).to.be.null;

    // try to reset a string. expect it to fail
    expect(function() {
      dds.guardReset('invalid_cond');
    }).to.throw(Error);

    // try to reset a string.
    expect(dds.guardReset('invalid_cond', emptycallback)).to.be.null;

    // try to check the condition of a string. expect failure
    expect(function() {
      dds.conditionTriggered('invalid_cond');
    }).to.throw(Error);

    // now without the callback
    expect(dds.conditionTriggered('invalid_cond', emptycallback)).to.be.null;
  });

  it('Can attach and detach read conditions', function() {
    var readCond = dds.readConditionCreate(reader, 127);
    var ws = dds.waitsetCreate();

    expect(function() {
      dds.waitsetAttach(ws, readCond, readCond);
    }).to.not.throw(Error);

    expect(function() {
      dds.waitsetDetach(ws, readCond, readCond);
    }).to.not.throw(Error);

    // try to attach an invalid condition and get an error
    expect(function() {
      dds.waitsetAttach(ws, 'invalid_cond', ws);
    }).to.throw(TypeError);

    expect(dds.waitsetAttach(ws, 'invalid_cond', ws, emptycallback)).to.be.null;

    // try to attach a random pointer. expect to get a status error
    expect(dds.waitsetAttach(ws, ref.alloc(pvoid), ws, emptycallback)).to.be
      .null;

    // try to detach a condition that wasn't attached. expect error.
    expect(function() {
      var cond = dds.readConditionCreate(reader, 127);
      dds.waitsetDetach(ws, cond);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    // try to detach an invalid cond
    expect(function() {
      dds.waitsetDetach(ws, 'invalid_cond');
    }).to.throw(Error);

    expect(dds.waitsetDetach(ws, 'invalid_cond', emptycallback)).to.be.null;
  });

  it('Can wait on a condition and timeout', function() {
    var ws = dds.waitsetCreate();
    var cond = dds.readConditionCreate(reader, 127);
    dds.waitsetAttach(ws, cond, cond);
    expect(function() {
      dds.waitsetWaitBlocking(ws, 1, 1000);
    }).to.not.throw(Error);

    // waitset wait until breaks with first invalid arg
    expect(function() {
      dds.waitsetWaitBlocking('invalid_arg', 1, 1000);
    }).to.throw(Error);

    expect(dds.waitsetWaitBlocking('invalid_arg', 1, 1000, emptycallback))
      .to.be.null;
  });

  it('Can wait on a condition and unblock', function() {
    let ws = dds.waitsetCreate();
    let cond = dds.readConditionCreate(reader, 127);
    dds.waitsetAttach(ws, cond, cond);
    let writer = dds.writerCreate(dp, topic);

    let sampleJSObj = { userID: 0, message: 1 };
    let data = typeSupport.copyin(sampleJSObj);
    dds.write(writer, data);

    // 0x7FFFFFFFFFFFFFFF = DDS_INFINITY
    expect(dds.waitsetWaitBlocking(ws, 1, 0x7FFFFFFFFFFFFFFF)).to.not.equal(0);
  });

  // OK
  it('Async waitset wait fails with invalid arguments', function(done) {
    // The error from FFI will get passed into our callback
    dds.waitsetWait('foo', 'foo', 'foo', function(err, res) {
      expect(err).to.be.instanceof(TypeError);
      expect(res).to.be.undefined;
      done();
    });
  });

  it('Async waitset wait can time out', function(done) {
    let ws = dds.waitsetCreate();
    let cond = dds.guardConditionCreate();
    dds.waitsetAttach(ws, cond, cond);

    // when it times out, we will get a return value of 0
    // wait for 1s and timeout
    dds.waitsetWait(ws, 1, Math.pow(10, 9), function(err, res) {
      expect(err).to.be.null;
      expect(res).to.equal(0);
      done();
    });
  });

  it('Async waitset wait can wait and unblock when '
    + 'condition is met', function(done) {
    let ws = dds.waitsetCreate();
    let cond = dds.guardConditionCreate();
    dds.guardTrigger(cond);
    dds.waitsetAttach(ws, cond, cond);

    // when it unblocks, err will be null and res will be the # of conditions
    // triggered
    dds.waitsetWait(ws, 1, Math.pow(10, 9), function(err, res) {
      expect(err).to.be.null;
      expect(res).to.equal(1);
      done();
    });
  });

  // quick test -- fix later
  it('Can create a query condition', function() {
    expect(dds.queryConditionCreateSql(reader, 127, 'color = %0',
      ['BLUE', 'RED', 'GREEN'], 3)).to.not.be.null;

    expect(function() {
      dds.queryConditionCreateSql('invalid_arg', 127, 'name = %0', null, 1);
    }).to.throw(TypeError);

    expect(dds.queryConditionCreateSql('invalid_arg', 127, 'name = %0', null, 1,
      emptycallback)).to.be.null;
  });

  it('Can create a status condition', function() {
    let dp = dds.participantCreate();
    let rd = dds.readerCreate(dp, topic);
    let cond = dds.statusCondition(rd);
    expect(cond).to.not.be.null;
    let ret = dds.enableStatus(rd, 1024); // 1024 = data_available
    expect(ret).to.be.a('number');
    expect(ret).to.equal(0);

    expect(function() {
      dds.enableStatus('foo', 9999);
    }).to.Throw(TypeError);

    expect(dds.enableStatus('foo', 'bar', emptycallback)).to.equal(null);
  });
});

describe('Async dds_reader_wait_for_historical_data tests', function() {
  var reader = null;
  var writer = null;
  var dp = null;
  var typeSupport = null;

  // in order to have wait for historical get called properly, we must
  // make sure reader,writer and topic are created with transient qos
  before(function() {
    dp = dds.participantCreate();
    let sampleDesc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let tqos = ddsqos.QoS.topicDefault();
    tqos.durability = ddsqos.DurabilityKind.Transient;
    tqos.reliability = ddsqos.ReliabilityKind.Reliable;
    let topic = dds.topicCreate(dp, sampleDesc, sampleTopicName + '1',
      tqos.cqos);

    let rqos = ddsqos.QoS.readerDefault();
    rqos.durability = ddsqos.DurabilityKind.Transient;
    rqos.reliability = ddsqos.ReliabilityKind.Reliable;

    reader = dds.readerCreate(dp, topic, rqos.cqos);

    let wqos = ddsqos.QoS.writerDefault();
    wqos.durability = ddsqos.DurabilityKind.Transient;
    wqos.reliability = ddsqos.ReliabilityKind.Reliable;
    writer = dds.writerCreate(dp, topic, wqos.cqos);

    typeSupport = new ddstopic.TypeSupport(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
  });

  after(function() {
    dds.entityDelete(dp);
  });

  it('readerWaitForHistoricalData fails with bad arguments',
    function(done) {
      dds.readerWaitForHistoricalData(reader, 'foo', function(err, res) {
        expect(err).to.be.instanceof(TypeError);
        expect(res).to.be.undefined;
        done();
      });
    });

  it('readerWaitForHistoricalData can timeout',
    function(done) {
      // let the writer write 10000 samples - this should take a bit of time to
      // wait for
      for (let i = 0; i < 10000; i++) {
        dds.write(writer, typeSupport.copyin({ userID: 0, message: 1 }));
      }
      dds.readerWaitForHistoricalData(reader, 1, function(err, res) {
        expect(err).to.be.instanceof(ddserr.DDSError).with
          .property('ddsErrCode', 10 /* timeout */);
        expect(res).to.be.below(0);
        done();
      });
    });
});

describe('Error handling callback ', function() {
  it('errorHandler throws error as expected ', function(done) {
    expect(function() { dds.errorHandler(new Error('error')); })
      .to.throw(Error);
    expect(function() { dds.errorHandler(null); }).to.not.throw(Error);
    done();
  });
});

describe('C99 init and fini ', function() {
  it('init and fini work as expected ', function() {
    // init and fini never fail
    expect(function() { dds.init(null, null); }).to.not.throw(Error);
    expect(function() { dds.fini(); }).to.not.throw(Error);
  });
});

describe('C99 tests for creating topics ', function() {
  // OK
  it('topicDescriptorCreate works as expected ', function() {
    this.timeout(10000);

    // create the topic descriptor as expected
    expect(dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    )).to.not.be.null;

    // catch the FFI TypeError with cb
    expect(
      () => dds.topicDescriptorCreate(0, 0, 0)
    ).to.throw(TypeError);

    // get null value with empty cb
    expect(dds.topicDescriptorCreate(0, 0, 0, emptycallback)).to.be.null;
  });
  // OK
  it('topicCreate works as expected ', function() {
    this.timeout(10000);

    // the descriptor and dp we will create the topic with
    let desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let dp = dds.participantCreate();

    // create the topic as expected
    expect(dds.topicCreate(dp, desc, sampleTopicName)).to.not.be.null;

    // catch the DDSError with cb
    expect(
      () => dds.topicCreate(ref.alloc(pvoid), desc, sampleTopicName)
    ).to.throw(ddserr.DDSError).with.property('ddsErrCode').to.be.within(1, 12);

    // catch the FFI error with cb
    expect(
      () => dds.topicCreate('foo', 'foo', 0)
    ).to.throw(TypeError);

    // get the null value with empty cb
    expect(dds.topicCreate('foo', 'foo', 0, null, null, emptycallback))
      .to.be.null;
  });
});


describe('C99 tests for creating and deleting entities ', function() {
  // OK
  it('participantCreate works as expected ', function() {
    this.timeout(10000);

    // create the dp as expected
    let dp = dds.participantCreate();
    expect(dp).to.not.be.null;
    expect(dds.entityDelete(dp)).to.not.be.null; // clean up

    // catch the DDSError with cb
    expect(
      () => dds.participantCreate(231)
    ).to.throw(ddserr.DDSError).with.property('ddsErrCode').to.be.within(1, 12);

    // catch the FFI TypeError with cb
    expect(
      () => dds.participantCreate('foo', 'foo', 'foo')
    ).to.throw(TypeError);

    // get the null value with empty cb
    expect(dds.participantCreate(231, null, null, emptycallback)).to.be.null;
  });

  // OK
  it('publisherCreate works as expected ', function() {
    this.timeout(10000);

    // the dp we create the publisher on
    let dp = dds.participantCreate();

    // create the publisher as expected
    let pub = dds.publisherCreate(dp);
    expect(pub).to.not.be.null;
    expect(dds.entityDelete(pub)).to.not.be.null;

    // catch the DDSError with cb
    // pass in a random pointer to get DDSError
    expect(
      () => dds.publisherCreate(ref.alloc(pvoid))
    ).to.throw(ddserr.DDSError).with.property('ddsErrCode').to.be.within(1, 12);

    // catch the FFI TypeError with cb
    expect(
      () => dds.publisherCreate('foo')
    ).to.throw(TypeError);

    // get the null value with empty cb
    expect(dds.publisherCreate('foo', null, null, emptycallback)).to.be.null;
  });

  it('writerCreate works as expected ', function() {
    this.timeout(10000);

    // set up our entities
    let dp = dds.participantCreate();
    let pub = dds.publisherCreate(dp);
    let desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let topic = dds.topicCreate(dp, desc, sampleTopicName);

    // create the writer as expected
    expect(dds.writerCreate(pub, topic)).to.not.be.null;

    // catch the DDSError with cb
    expect(
      () => dds.writerCreate(ref.alloc(pvoid), topic)
    ).to.throw(ddserr.DDSError).with.property('ddsErrCode').to.be.within(1, 12);

    // catch the FFI TypeError with cb
    expect(
      () => dds.writerCreate('foo', 'foo')
    ).to.throw(TypeError);

    // get null value with empty cb
    expect(dds.writerCreate('foo', 'foo', null, null, emptycallback))
      .to.be.null;
  });

  it('subscriberCreate works as expected ', function() {
    this.timeout(10000);

    // set up our dp
    let dp = dds.participantCreate();

    // create the subscriber as expected
    expect(dds.subscriberCreate(dp)).to.not.be.null;

    // catch DDSError with cb
    expect(
      () => dds.subscriberCreate(ref.alloc(pvoid))
    ).to.throw(ddserr.DDSError).with.property('ddsErrCode').to.be.within(1, 12);

    // catch the FFI TypeError with cb
    expect(
      () => dds.subscriberCreate('foo')
    ).to.throw(TypeError);

    // get the null value with empty cb
    expect(dds.subscriberCreate('foo', 'foo', 'foo', emptycallback)).to.be.null;
  });

  it('readerCreate works as expected ', function() {
    this.timeout(10000);

    // set up our entities
    let dp = dds.participantCreate();
    let sub = dds.subscriberCreate(dp);
    let desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let topic = dds.topicCreate(dp, desc, sampleTopicName);

    // create the reader on sub as expected
    expect(dds.readerCreate(sub, topic)).to.not.be.null;

    // catch the DDSError with cb
    // pass in a random pointer to get DDSError
    expect(
      () => dds.readerCreate(ref.alloc(pvoid), topic)
    ).to.throw(ddserr.DDSError).with.property('ddsErrCode').to.be.within(1, 12);

    // catch the FFI TypeError with cb
    expect(
      () => dds.readerCreate('foo', 'foo')
    ).to.throw(TypeError);

    // get the null value with empty cb
    expect(dds.readerCreate(null, null, null, null, emptycallback)).to.be.null;
  });

  it('entityDelete works as expected ', function() {
    this.timeout(10000);

    let dp = dds.participantCreate(); // entity to delete

    // delete dp as expected
    expect(dds.entityDelete(dp)).to.not.be.null;

    // get the FFI type error with cb
    expect(
      () => dds.entityDelete('foo')
    ).to.throw(TypeError);

    // get the null value
    expect(dds.entityDelete('foo', emptycallback)).to.be.null;
  });

  it('topicDescriptorDelete works as expected ', function() {
    this.timeout(10000);

    // set up our descriptor
    let desc = dds.topicDescriptorCreate(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );

    // delete desc as expected
    expect(dds.topicDescriptorDelete(desc)).to.not.be.null;

    // get FFI type error with cb
    expect(
      () => dds.topicDescriptorDelete('foo')
    ).to.throw(TypeError);
  });

});

describe('FFI C99 - reading and writing ', function() {
  // these are the entities used in the tests
  // we init. them in before() and delete them in after()
  var dp = null;
  var sampleDesc = null;
  var topic = null;
  var wr = null;
  var rd = null;
  var typeSupport = null;

  before(function() {
    dp = dds.participantCreate();
    sampleDesc = dds.topicDescriptorCreate(sampleTypeName,
      sampleTypeKey, sampleTopicXML);
    topic = dds.topicCreate(dp, sampleDesc, sampleTopicName);
    wr = dds.writerCreate(dp, topic);
    rd = dds.readerCreate(dp, topic);
    typeSupport = new ddstopic.TypeSupport(
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
  });

  after(function() {
    dds.entityDelete(dp);
    dds.topicDescriptorDelete(sampleDesc);
  });

  it('can read 10 messages successfully with condition', function() {
    // write 10 messages
    for (let i = 0; i < 10; i++) {
      let sampleJSObj = { userID: i, message: 100 + i };
      let data = typeSupport.copyin(sampleJSObj);
      dds.write(wr, data);
    }

    // read our messages into an array
    let readArray = dds.readCond(
      rd,
      10,
      typeSupport.getRefType(),
      dds.readConditionCreate(rd, 127)
    );

    expect(readArray.length).to.equal(10);
    for (let i = 0; i < 10; i++) {
      expect(readArray[i].sample.userID).to.equal(i);
      expect(readArray[i].sample.message).to.equal(100 + i);
    }
  });

  it('Can create an instance', function() {
    let fooData = typeSupport.copyin({ userID: 1, message: 2 });
    expect(function() {
      dds.instanceRegister(wr, fooData);
    }).to.not.throw(Error);

    expect(function() {
      dds.instanceRegister(wr, 'foo');
    }).to.throw(Error);

    expect(dds.instanceRegister(wr, 'foo', emptycallback)).to.be.null;
  });

  it('Can unregister an instance', function() {
    let fooData = typeSupport.copyin({ userID: 1, message: 2 });
    let fooInstance = dds.instanceRegister(wr, fooData);

    expect(function() {
      dds.instanceUnregister(wr, fooData, fooInstance);
    }).to.not.Throw(Error);

    expect(function() {
      dds.instanceUnregister(wr, fooData, 0);
    }).to.throw(ddserr.DDSError).with.property('ddsErrCode')
      .to.be.within(1, 12);

    expect(dds.instanceUnregister(wr, fooData, 0, emptycallback)).to.be.null;
  });
});

describe('statusRead/take/getEnabled', function() {
  let dp = null;

  before(function() {
    dp = dds.participantCreate();
  });

  after(function() {
    dds.entityDelete(dp);
  });

  it('Can read a status from an entity', function() {
    expect(dds.statusRead(dp, 0)).to.equal(0);
    expect(() => dds.statusRead('foo', 'foo')).to.throw(TypeError);
    expect(dds.statusRead('foo', 'foo', emptycallback)).to.be.null;
  });

  it('Can take a status from an entity', function() {
    expect(dds.statusTake(dp, 0)).to.equal(0);
    expect(() => dds.statusTake('foo', 'foo')).to.throw(TypeError);
    expect(dds.statusTake('foo', 'foo', emptycallback)).to.be.null;
  });

  it('Can get the enabled statuses on an entity', function() {
    expect(dds.statusGetEnabled(dp)).to.be.a('number');
    expect(() => dds.statusGetEnabled('foo')).to.throw(TypeError);
    expect(dds.statusGetEnabled('foo', emptycallback)).to.be.null;
  });

  it('Can get the changed statuses on an entity', function() {
    expect(dds.statusChanges(dp)).to.be.a('number');
    expect(() => dds.statusChanges('foo')).to.throw(TypeError);
    expect(dds.statusChanges('foo', emptycallback)).to.be.null;
  });
});
