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
const qos = require('./qos');
const expect = require('chai').expect;
const ddserr = require('./ddserr');
const path = require('path');
const DDS_DOMAIN_DEFAULT = 0x7fffffff;

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

const dirpath = 'test_data' + path.sep;
const relqospath = dirpath + 'DDS_PersistentQoS_All.xml';
const absqospath = path.resolve(relqospath);
const qosuri = 'file://' + absqospath;
const emptyqp = path.resolve(dirpath + 'Empty_Entity_QoS.xml');
const profile = 'DDS PersistentQosProfile';

const entityQosUri = 'file://'
  + path.resolve(dirpath
    + 'DDS_EntityQoS.xml');
const entityQosProfile = 'DDS EntityQosProfile';

describe('QoS Provider class tests', function() {
  var dp = null;
  before(function(){
    this.timeout(20000);
    dp = new dcps.Participant();
  });

  after(function(){
    this.timeout(20000);
    dp.delete();
  });

  it('qos provider fails with invalid qospath', function() {
    expect(function() {
      new qos.QoSProvider(
        'home/dds.xml',
        'DDS PersistentQosProfile'
      );
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);
  });

  it('qos provider with absolute qospath', function() {
    let qosprovide = new qos.QoSProvider(absqospath, profile);
    expect(qosprovide).to.not.be.null;
  });

  it('qos provider with relative qospath', function() {
    let qosprovide = new qos.QoSProvider(relqospath, profile);
    expect(qosprovide).to.not.be.null;
  });

  it('qos provider with file uri qospath', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    expect(qosprovide).to.not.be.null;
  });

  it('invalid type for qospath', function() {
    expect(function() { new qos.QoSProvider(1234, profile); })
      .to.throw(TypeError);
  });

  it('try to delete qos twice', function(){
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let pqos = qosprovide.getParticipantQos();
    pqos.delete();
    expect(function() {
      pqos.delete();
    }).to.not.throw(Error);
  });

  it('getParticipantQos test when no participant qos is available', function() {
    let qosprovide = new qos.QoSProvider(emptyqp, profile);
    expect(function() { qosprovide.getParticipantQos(); })
      .to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);
  });

  it('getSubscriberQoS test when no subscriber qos is available', function() {
    let qosprovide = new qos.QoSProvider(emptyqp, profile);
    expect(function() { qosprovide.getSubscriberQos(); })
      .to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);
  });

  it('getPublisherQos test when no publisher qos is available', function() {
    let qosprovide = new qos.QoSProvider(emptyqp, profile);
    expect(function() { qosprovide.getPublisherQos(); })
      .to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);
  });

  it('getTopicQos test when no topic qos is available', function() {
    let qosprovide = new qos.QoSProvider(emptyqp, profile);
    expect(function() { qosprovide.getTopicQos(); })
      .to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);
  });

  it('getReaderQos test id = b', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let rqos = qosprovide.getReaderQos('b');
    let topic = new dcps.Topic(
      dp,
      'sampleTopic3',
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let reader = new dcps.Reader(dp, topic, rqos);
    rqos.delete();
    expect(reader).to.not.be.null;
  });

  it('getReaderQos test null id', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    expect(function() {
      qosprovide.getReaderQos();
    }).to.throw(Error);
  });

  it('getReaderQos test when no reader qos is available', function() {
    let qosprovide = new qos.QoSProvider(emptyqp, profile);
    expect(function() { qosprovide.getReaderQos(); })
      .to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);
  });

  it('getWriterQos test when no writer qos is available', function() {
    let qosprovide = new qos.QoSProvider(emptyqp, profile);
    expect(function() { qosprovide.getWriterQos(); })
      .to.throw(ddserr.DDSError)
      .with.property('ddsErrCode').to.be.within(1, 12);
  });

  it('get participantDefault qos test', function() {
    let pqos = qos.QoS.participantDefault();
    expect(pqos).to.not.be.null;
    pqos.delete();
  });

  it('get topicDefault qos test', function() {
    let tqos = qos.QoS.topicDefault();
    expect(tqos).to.not.be.null;
    tqos.delete();
  });

  it('get publisherDefault qos test', function() {
    let pubqos = qos.QoS.publisherDefault();
    expect(pubqos).to.not.be.null;
    pubqos.delete();
  });

  it('get subscriberDefault qos test', function() {
    let subqos = qos.QoS.subscriberDefault();
    expect(subqos).to.not.be.null;
    subqos.delete();
  });

  it('get readerDefault qos test', function() {
    let rqos = qos.QoS.readerDefault();
    expect(rqos).to.not.be.null;
    rqos.delete();
  });

  it('get writerDefault qos test', function() {
    let wqos = qos.QoS.writerDefault();
    expect(wqos).to.not.be.null;
    wqos.delete();
  });

  it('getWriterQos test', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let wqos = qosprovide.getWriterQos();
    let kind = wqos.durability;
    expect(kind).to.be.equal(qos.DurabilityKind.Persistent);
    let topic = new dcps.Topic(
      dp,
      'sampleTopic3',
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let writer = new dcps.Writer(dp, topic, wqos);
    wqos.delete();
    expect(writer).to.not.be.null;
  });

  it('getReaderQos test', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let rqos = qosprovide.getReaderQos('a');
    let kind = rqos.durability;
    expect(kind).to.be.equal(qos.DurabilityKind.Persistent);
    let topic = new dcps.Topic(
      dp,
      'sampleTopic3',
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML
    );
    let reader = new dcps.Reader(dp, topic, rqos);
    rqos.delete();
    expect(reader).to.not.be.null;
  });

  it('getTopicQos test', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let tqos = qosprovide.getTopicQos();
    let kind = tqos.durability;
    expect(kind).to.be.equal(qos.DurabilityKind.Persistent);
    let topic = new dcps.Topic(
      dp,
      'sampleTopic1',
      sampleTypeName,
      sampleTypeKey,
      sampleTopicXML,
      tqos
    );
    tqos.delete();
    expect(topic).to.not.be.null;
  });

  it('getPublisherQos test', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let pubqos = qosprovide.getPublisherQos();
    let scope = pubqos.presentation;
    expect(scope.accessScope).to.be.equal(
      qos.PresentationAccessScopeKind.Instance);
    let pub = new dcps.Publisher(dp, pubqos);
    pubqos.delete();
    expect(pub).to.not.be.null;
  });

  it('getSubscriberQos test', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let subqos = qosprovide.getSubscriberQos();
    let scope = subqos.presentation;
    expect(scope.accessScope).to.be.equal(
      qos.PresentationAccessScopeKind.Instance);
    let sub = new dcps.Subscriber(dp, subqos);
    subqos.delete();
    expect(sub).to.not.be.null;
  });

  it('getParticipantQos test', function() {
    let qosprovide = new qos.QoSProvider(qosuri, profile);
    let pqos = qosprovide.getParticipantQos();
    this.timeout(10000);
    let participant = new dcps.Participant(DDS_DOMAIN_DEFAULT, pqos);
    pqos.delete();
    expect(participant).to.not.be.null;
    participant.delete();
  });

});

describe('Get and set userdata, topicdata and groupdata QoS tests', function() {

  it('userdata qos policy', function() {
    let dpqos = qos.QoS.participantDefault();
    dpqos.userdata = 'test';
    let ud = dpqos.userdata;
    expect(ud).to.be.equal('test');
    dpqos.delete();
  });

  it('userdataRaw qos policy', function() {
    let dpqos = qos.QoS.participantDefault();
    dpqos.userdata = 'test';
    let ud = dpqos.userdataRaw; // expect a buffer
    expect(ud.toString('binary')).to.be.equal('test');
    dpqos.delete();
  });

  it('userdata qos policy with non buffer input', function() {
    let dpqos = qos.QoS.participantDefault();
    expect(function() {
      dpqos.userdata = 1234;
    }).to.throw(Error);
    dpqos.delete();
  });

  it('topicdata qos policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.topicdata = 'test';
    let td = tqos.topicdata;
    expect(td).to.be.equal('test');
    tqos.delete();
  });

  it('topicdataRaw qos policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.topicdata = 'test';
    let td = tqos.topicdataRaw; // expect a buffer
    expect(td.toString('binary')).to.be.equal('test');
    tqos.delete();
  });

  it('topicdata qos policy with non buffer input', function() {
    let tqos = qos.QoS.topicDefault();
    expect(function() {
      tqos.topicdata = 1234;
    }).to.throw(Error);
    tqos.delete();
  });

  it('groupdata qos policy', function() {
    let subqos = qos.QoS.subscriberDefault();
    subqos.groupdata = 'test';
    let gd = subqos.groupdata;
    expect(gd).to.be.equal('test');
    subqos.delete();
  });

  it('groupdataRaw qos policy', function() {
    let subqos = qos.QoS.subscriberDefault();
    subqos.groupdata = 'test';
    let gd = subqos.groupdataRaw; // expect a buffer
    expect(gd.toString('binary')).to.be.equal('test');
    subqos.delete();
  });

  it('groupdata qos policy with non buffer input', function() {
    let subqos = qos.QoS.subscriberDefault();
    expect(function() {
      subqos.groupdata = 1234;
    }).to.throw(Error);
    subqos.delete();
  });
});

describe('Get QoS policy tests', function() {

  it('get durability policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let kind = tqos.durability;
    expect(kind).to.be.equal(qos.DurabilityKind.Persistent);
    tqos.delete();
  });

  it('get history policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let wqos = qp.getWriterQos();
    let history = wqos.history;
    expect(history.kind).to.be.equal(qos.HistoryKind.KeepLast);
    wqos.delete();
  });

  it('get resource limits policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let wqos = qp.getWriterQos();
    let rlPolicy = wqos.resourceLimits;
    expect(rlPolicy.maxSamples).to.be.equal(-1);
    expect(rlPolicy.maxInstances).to.be.equal(-1);
    expect(rlPolicy.maxSamplesPerInstance).to.be.equal(-1);
    wqos.delete();
  });

  it('get presentation policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let subqos = qp.getSubscriberQos();
    let pPolicy = subqos.presentation;
    expect(pPolicy.accessScope).to
      .be.equal(qos.PresentationAccessScopeKind.Instance);
    subqos.delete();
  });

  it('get lifespan policy', function() {
    let qp = new qos.QoSProvider(entityQosUri, entityQosProfile);
    let tqos = qp.getTopicQos();
    let lifespan = tqos.lifespan;
    expect(lifespan).to.be.equal(3600);
    tqos.delete();
  });

  it('get deadline policy', function() {
    let qp = new qos.QoSProvider(entityQosUri, entityQosProfile);
    let tqos = qp.getTopicQos();
    let deadline = tqos.deadline;
    expect(deadline).to.be.equal(5000);
    tqos.delete();
  });

  it('get latency budget policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let duration = tqos.latencyBudget;
    expect(duration).to.be.equal(0);
    tqos.delete();
  });

  it('get ownership policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let kind = tqos.ownership;
    expect(kind).to.be.equal(qos.OwnershipKind.Shared);
    tqos.delete();
  });

  it('get ownership strength policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let wqos = qp.getWriterQos();
    let value = wqos.ownershipStrength;
    expect(value).to.be.equal(0);
    wqos.delete();
  });

  it('get liveliness policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let jsobj = tqos.liveliness;
    expect(jsobj.kind).to.be.equal(qos.LivelinessKind.Automatic);
    tqos.delete();
  });

  it('get timebased filter policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let rqos = qp.getReaderQos('a');
    let minSeparation = rqos.timebasedFilter;
    expect(minSeparation).to.be.equal(0);
    rqos.delete();
  });

  it('get partition policy', function() {
    let qp = new qos.QoSProvider(entityQosUri, entityQosProfile);
    let pubqos = qp.getPublisherQos();
    let partArr = pubqos.partition;
    expect(partArr.length).to.equal(2);
    expect(partArr[0]).to.be.equal('partition1');
    expect(partArr[1]).to.be.equal('partition2');
    pubqos.delete();
  });

  it('get reliability policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let jsobj = tqos.reliability;
    expect(jsobj.kind).to.be.equal(qos.ReliabilityKind.BestEffort);
    tqos.delete();
  });

  it('get transport priority policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let value = tqos.transportPriority;
    expect(value).to.be.equal(0);
    tqos.delete();
  });

  it('get destination order policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let kind = tqos.destinationOrder;
    expect(kind).to.be.equal(qos.DestinationOrderKind.ByReceptionTimestamp);
    tqos.delete();
  });

  it('get writer data lifecycle policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let wqos = qp.getWriterQos();
    let value = wqos.writerDataLifecycle;
    expect(value).to.be.true;
    wqos.delete();
  });

  it('get reader data lifecycle policy', function() {
    let qp = new qos.QoSProvider(entityQosUri, entityQosProfile);
    let rqos = qp.getReaderQos();
    let jsobj = rqos.readerDataLifecycle;
    expect(jsobj.autopurgeNoWriterSamples).to.equal(1000);
    expect(jsobj.autopurgeDisposedSamplesDelay).to.equal(500);
    rqos.delete();
  });

  it('get durability service policy', function() {
    let qp = new qos.QoSProvider(qosuri, profile);
    let tqos = qp.getTopicQos();
    let jsobj = tqos.durabilityService;
    expect(jsobj.historyKind).to.equal(qos.HistoryKind.KeepLast);
    tqos.delete();
  });
});

describe('Set QoS policy tests', function() {

  it('set durability policy', function() {
    let q = new qos.QoS();
    q.durability = qos.DurabilityKind.Persistent;
    let kind = q.durability;
    expect(kind).to.equal(qos.DurabilityKind.Persistent);
    q.delete();
  });

  it('set durability policy ignores inapplicable input value',
    function() {
      let q = new qos.QoS();
      let defaultKind = q.durability;
      q.durability = null;
      let kind = q.durability;
      expect(kind).to.equal(defaultKind);
      q.delete();
    });

  it('set history policy kind and depth', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.history = {
      kind: qos.HistoryKind.KeepLast,
      depth: 10,
    };
    let jsobj = tqos.history;
    expect(jsobj.kind).to.equal(qos.HistoryKind.KeepLast);
    expect(jsobj.depth).to.equal(10);
    tqos.delete();
  });

  it('set history policy kind only', function() {
    let tqos = qos.QoS.topicDefault();
    let defaultPolicy = tqos.history;
    tqos.history = { kind: qos.HistoryKind.KeepAll };
    let jsobj = tqos.history;
    expect(jsobj.kind).to.equal(qos.HistoryKind.KeepAll);
    expect(jsobj.depth).to.equal(defaultPolicy.depth);
    tqos.delete();
  });

  it('set history policy ignores inapplicable values', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.history = {kind: qos.HistoryKind.KeepAll, extraField: 1};
    let jsobj = tqos.history;
    expect(jsobj.kind).to.equal(qos.HistoryKind.KeepAll);
    expect(jsobj.extraField).to.be.undefined;
    tqos.delete();
  });

  it('set history policy uses default values for empty input', function() {
    let tqos = qos.QoS.topicDefault();
    let defaultPolicy = tqos.history;
    tqos.history = {};
    let jsobj = tqos.history;
    expect(jsobj.kind).to.equal(defaultPolicy.kind);
    expect(jsobj.depth).to.equal(defaultPolicy.depth);
    tqos.delete();
  });

  it('set resource limits policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.resourceLimits = {
      maxSamples: 1,
      maxInstances: 2,
      maxSamplesPerInstance: 2,
    };
    let jsobj = tqos.resourceLimits;
    expect(jsobj.maxSamples).to.equal(1);
    expect(jsobj.maxInstances).to.equal(2);
    expect(jsobj.maxSamplesPerInstance).to.equal(2);
    tqos.delete();
  });

  it('set resource limits policy maxInstances only', function() {
    let tqos = qos.QoS.topicDefault();
    let defaultPolicy = tqos.resourceLimits;
    tqos.resourceLimits = {maxInstances: 2};
    let jsobj = tqos.resourceLimits;
    expect(jsobj.maxSamples).to.equal(defaultPolicy.maxSamples);
    expect(jsobj.maxInstances).to.equal(2);
    expect(jsobj.maxSamplesPerInstance).to.equal(
      defaultPolicy.maxSamplesPerInstance);
    tqos.delete();
  });

  it('set resource limits policy ignores inapplicable values', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.resourceLimits = {maxInstances: 2, kind: 1};
    let jsobj = tqos.resourceLimits;
    expect(jsobj.maxInstances).to.equal(2);
    expect(jsobj.kind).to.be.undefined;
    tqos.delete();
  });

  it('set resource limits policy uses default values for empty input',
    function() {
      let tqos = qos.QoS.topicDefault();
      let defaultPolicy = tqos.resourceLimits;
      tqos.resourceLimits = {};
      let jsobj = tqos.resourceLimits;
      expect(jsobj.maxSamples).to.equal(defaultPolicy.maxSamples);
      expect(jsobj.maxInstances).to.equal(defaultPolicy.maxInstances);
      expect(jsobj.maxSamplesPerInstance).to.equal(
        defaultPolicy.maxSamplesPerInstance);
      tqos.delete();
    });

  it('set presentation policy', function() {
    let subqos = qos.QoS.subscriberDefault();
    subqos.presentation = {
      accessScope: qos.PresentationAccessScopeKind.Group,
      coherentAccess: false,
      orderedAccess: true };
    let jsobj = subqos.presentation;
    expect(jsobj.accessScope).to
      .equal(qos.PresentationAccessScopeKind.Group);
    expect(jsobj.coherentAccess).to.be.false;
    expect(jsobj.orderedAccess).to.be.true;
    subqos.delete();
  });

  it('set presentation policy accessScope kind only', function() {
    let subqos = qos.QoS.subscriberDefault();
    let defaultPolicy = subqos.presentation;
    subqos.presentation = {
      accessScope: qos.PresentationAccessScopeKind.Group,
    };
    let jsobj = subqos.presentation;
    expect(jsobj.accessScope).to
      .equal(qos.PresentationAccessScopeKind.Group);
    expect(jsobj.coherentAccess).to.be.equal(defaultPolicy.coherentAccess);
    expect(jsobj.orderedAccess).to.be.equal(defaultPolicy.orderedAccess);
    subqos.delete();
  });

  it('set presentation policy ignores inapplicable values', function() {
    let subqos = qos.QoS.subscriberDefault();
    subqos.presentation = {
      accessScope: qos.PresentationAccessScopeKind.Group,
      extraField: 1,
    };
    let jsobj = subqos.presentation;
    expect(jsobj.accessScope).to
      .equal(qos.PresentationAccessScopeKind.Group);
    expect(jsobj.extraField).to.be.undefined;
    subqos.delete();
  });

  it('set presentation policy uses default values for empty input',
    function() {
      let subqos = qos.QoS.subscriberDefault();
      let defaultPolicy = subqos.presentation;
      subqos.presentation = {};
      let jsobj = subqos.presentation;
      expect(jsobj.accessScope).to
        .equal(defaultPolicy.accessScope);
      expect(jsobj.coherentAccess).to.be.equal(defaultPolicy.coherentAccess);
      expect(jsobj.orderedAccess).to.be.equal(defaultPolicy.orderedAccess);
      subqos.delete();
    });

  it('set lifespan policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.lifespan = 5000;
    let lifespan = tqos.lifespan;
    expect(lifespan).to.equal(5000);
    tqos.delete();
  });

  it('set lifespan policy callback throws error for invalid input type',
    function() {
      let tqos = qos.QoS.topicDefault();
      expect(function() {
        tqos.lifespan = 'aaa';
      }).to.throw(TypeError);
      tqos.delete();
    });

  it('set deadline policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.deadline = 3000;
    let deadline = tqos.deadline;
    expect(deadline).to.equal(3000);
    tqos.delete();
  });

  it('set deadline policy callback throws error for invalid input type',
    function() {
      let tqos = qos.QoS.topicDefault();
      expect(function() {
        tqos.deadline = 'aaa';
      }).to.throw(TypeError);
      tqos.delete();
    });

  it('set latency budget policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.latencyBudget = 4000;
    let duration = tqos.latencyBudget;
    expect(duration).to.equal(4000);
    tqos.delete();
  });

  it('set latency budget policy callback throws error for invalid input type',
    function() {
      let tqos = qos.QoS.topicDefault();
      expect(function() {
        tqos.latencyBudget = 'aaa';
      }).to.throw(TypeError);
      tqos.delete();
    });

  it('set ownership policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.ownership = qos.OwnershipKind.Exclusive;
    let kind = tqos.ownership;
    expect(kind).to.equal(qos.OwnershipKind.Exclusive);
    tqos.delete();
  });

  it('set ownership policy ignores inapplicable input value',
    function() {
      let tqos = qos.QoS.topicDefault();
      let defaultKind = tqos.ownership;
      tqos.ownership = null;
      let kind = tqos.ownership;
      expect(kind).to.equal(defaultKind);
      tqos.delete();
    });

  it('set ownership strength policy', function() {
    let wqos = qos.QoS.writerDefault();
    wqos.ownershipStrength = 3;
    let value = wqos.ownershipStrength;
    expect(value).to.equal(3);
    wqos.delete();
  });

  it('set ownership strength policy ignores inapplicable input value',
    function() {
      let wqos = qos.QoS.writerDefault();
      let defaultValue = wqos.ownershipStrength;
      wqos.ownershipStrength = null;
      let value = wqos.ownershipStrength;
      expect(value).to.equal(defaultValue);
      wqos.delete();
    });

  it('set liveliness policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.liveliness = {
      kind: qos.LivelinessKind.ManualByTopic,
      leaseDuration: 10,
    };
    let jsobj = tqos.liveliness;
    expect(jsobj.kind).to.equal(qos.LivelinessKind.ManualByTopic);
    expect(jsobj.leaseDuration).to.equal(10);
    tqos.delete();
  });

  it('set liveliness policy kind only', function() {
    let tqos = qos.QoS.topicDefault();
    let defaultPolicy = tqos.liveliness;
    tqos.liveliness = {kind: qos.LivelinessKind.ManualByTopic};
    let jsobj = tqos.liveliness;
    expect(jsobj.kind).to.equal(qos.LivelinessKind.ManualByTopic);
    expect(jsobj.leaseDuration).to.be.equal(defaultPolicy.leaseDuration);
    tqos.delete();
  });

  it('set liveliness policy ignores inapplicable values', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.liveliness = {
      kind: qos.LivelinessKind.ManualByTopic,
      extraField: 1,
    };
    let jsobj = tqos.liveliness;
    expect(jsobj.kind).to.equal(qos.LivelinessKind.ManualByTopic);
    expect(jsobj.extraField).to.be.undefined;
    tqos.delete();
  });

  it('set liveliness policy uses default values for empty input',
    function() {
      let tqos = qos.QoS.topicDefault();
      let defaultPolicy = tqos.liveliness;
      tqos.liveliness = {};
      let jsobj = tqos.liveliness;
      expect(jsobj.kind).to.equal(defaultPolicy.kind);
      expect(jsobj.leaseDuration).to.equal(defaultPolicy.leaseDuration);
      tqos.delete();
    });

  it('set time based filter policy', function() {
    let rqos = qos.QoS.readerDefault();
    rqos.timebasedFilter = 60000;
    let minSeparation = rqos.timebasedFilter;
    expect(minSeparation).to.equal(60000);
    rqos.delete();
  });

  it('set timebased filter policy callback throws error for invalid input type',
    function() {
      let rqos = qos.QoS.readerDefault();
      expect(function() {
        rqos.timebasedFilter = 'aaa';
      }).to.throw(TypeError);
      rqos.delete();
    });

  it('set partition qos', function() {
    let pubqos = qos.QoS.publisherDefault();
    pubqos.partition = ['partition1', 'partition2'];
    let partition = pubqos.partition;
    expect(partition.length).to.equal(2);
    expect(partition[0]).to.be.equal('partition1');
    expect(partition[1]).to.be.equal('partition2');
    pubqos.delete();
  });

  it('set partition to an input string', function() {
    let pubqos = qos.QoS.publisherDefault();
    pubqos.partition = 'partition1';
    let partition = pubqos.partition;
    expect(partition.length).to.equal(1);
    expect(partition[0]).to.be.equal('partition1');
    pubqos.delete();
  });

  it('set partition to an empty string', function() {
    let pubqos = qos.QoS.publisherDefault();
    pubqos.partition = '';
    let partition = pubqos.partition;
    expect(partition.length).to.equal(1);
    expect(partition[0]).to.be.equal('');
    pubqos.delete();
  });

  it('set partition to an empty array', function() {
    let pubqos = qos.QoS.publisherDefault();
    pubqos.partition = [''];
    let partition = pubqos.partition;
    expect(partition.length).to.equal(1);
    expect(partition[0]).to.be.equal('');
    pubqos.delete();
  });

  it('set partition input number', function() {
    let pubqos = qos.QoS.publisherDefault();
    pubqos.partition = 48;
    let partition = pubqos.partition;
    expect(partition.length).to.equal(0);
    pubqos.delete();
  });

  it('set reliability policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.reliability = {
      kind: qos.ReliabilityKind.Reliable,
      maxBlockingTime: 100,
    };
    let jsobj = tqos.reliability;
    expect(jsobj.kind).to.equal(qos.ReliabilityKind.Reliable);
    expect(jsobj.maxBlockingTime).to.equal(100);
    tqos.delete();
  });

  it('set reliability policy kind only', function() {
    let tqos = qos.QoS.topicDefault();
    let defaultPolicy = tqos.reliability;
    tqos.reliability = {kind: qos.ReliabilityKind.Reliable};
    let jsobj = tqos.reliability;
    expect(jsobj.kind).to.equal(qos.ReliabilityKind.Reliable);
    expect(jsobj.maxBlockingTime).to.be.equal(defaultPolicy.maxBlockingTime);
    tqos.delete();
  });

  it('set reliability policy ignores inapplicable values', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.reliability = {
      kind: qos.ReliabilityKind.Reliable,
      extraField: 1};
    let jsobj = tqos.reliability;
    expect(jsobj.kind).to.equal(qos.ReliabilityKind.Reliable);
    expect(jsobj.extraField).to.be.undefined;
    tqos.delete();
  });

  it('set reliability policy uses default values for empty input',
    function() {
      let tqos = qos.QoS.topicDefault();
      let defaultPolicy = tqos.reliability;
      tqos.reliability = {};
      let jsobj = tqos.reliability;
      expect(jsobj.kind).to.equal(defaultPolicy.kind);
      expect(jsobj.maxBlockingTime).to.equal(defaultPolicy.maxBlockingTime);
      tqos.delete();
    });

  it('set transport priority policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.transportPriority = 5;
    let value = tqos.transportPriority;
    expect(value).to.equal(5);
    tqos.delete();
  });

  it('set transport priority policy ignores inapplicable input value',
    function() {
      let tqos = qos.QoS.topicDefault();
      let defaultValue = tqos.transportPriority;
      tqos.transportPriority = 'aaaaaa';
      let value = tqos.transportPriority;
      expect(value).to.equal(defaultValue);
      tqos.delete();
    });

  it('set destination order policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.destinationOrder = qos.DestinationOrderKind.BySourceTimestamp;
    let kind = tqos.destinationOrder;
    expect(kind).to.equal(qos.DestinationOrderKind.BySourceTimestamp);
    tqos.delete();
  });

  it('set destination order policy ignores inapplicable input value',
    function() {
      var tqos = qos.QoS.topicDefault();
      var defaultKind = tqos.destinationOrder;
      tqos.destinationOrder = null;
      var kind = tqos.destinationOrder;
      expect(kind).to.equal(defaultKind);
      tqos.delete();
    });

  it('set writer data lifecycle policy', function() {
    let wqos = qos.QoS.writerDefault();
    wqos.writerDataLifecycle = false;
    let value = wqos.writerDataLifecycle;
    expect(value).to.be.false;
    wqos.delete();
  });

  it('set writer data lifecycle callback throws error for invalid input type',
    function() {
      let wqos = qos.QoS.writerDefault();
      expect(function() {
        wqos.writerDataLifecycle = -99;
      }).to.throw(TypeError);
      wqos.delete();
    });

  it('set reader data lifecycle qos policy', function() {
    let rqos = qos.QoS.readerDefault();
    rqos.readerDataLifecycle = {
      autopurgeNoWriterSamples: 100,
      autopurgeDisposedSamplesDelay: 500,
    };
    let jsobj = rqos.readerDataLifecycle;
    expect(jsobj.autopurgeNoWriterSamples).to.equal(100);
    expect(jsobj.autopurgeDisposedSamplesDelay).to.equal(500);
    rqos.delete();
  });

  it('set reader data lifecycle qos policy autopurgeNoWriterSamples only',
    function() {
      let rqos = qos.QoS.readerDefault();
      let defaultPolicy = rqos.readerDataLifecycle;
      rqos.readerDataLifecycle = {autopurgeNoWriterSamples: 300};
      let jsobj = rqos.readerDataLifecycle;
      expect(jsobj.autopurgeNoWriterSamples).to.equal(300);
      expect(jsobj.autopurgeDisposedSamplesDelay).to
        .equal(defaultPolicy.autopurgeDisposedSamplesDelay);
      rqos.delete();
    });

  it('set reader data lifecycle qos policy ignores inapplicable values',
    function() {
      let rqos = qos.QoS.readerDefault();
      rqos.readerDataLifecycle = {
        autopurgeNoWriterSamples: 500,
        extraField: 1,
      };
      let jsobj = rqos.readerDataLifecycle;
      expect(jsobj.autopurgeNoWriterSamples).to.equal(500);
      expect(jsobj.extraField).to.be.undefined;
      rqos.delete();
    });

  it('set reader data lifecycle qos policy uses default values for empty input',
    function() {
      let rqos = qos.QoS.readerDefault();
      let defaultPolicy = rqos.readerDataLifecycle;
      rqos.readerDataLifecycle = {};
      let jsobj = rqos.readerDataLifecycle;
      expect(jsobj.autopurgeNoWriterSamples).to
        .equal(defaultPolicy.autopurgeNoWriterSamples);
      expect(jsobj.autopurgeDisposedSamplesDelay).to
        .equal(defaultPolicy.autopurgeDisposedSamplesDelay);
      rqos.delete();
    });

  it('set durability service qos policy', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.durabilityService = {
      serviceCleanupDelay: 5000,
      historyKind: qos.HistoryKind.KeepLast,
      historyDepth: 10,
      maxSamples: 10,
      maxInstances: 5,
      maxSamplesPerInstance: 2,
    };
    let jsobj = tqos.durabilityService;
    expect(jsobj.serviceCleanupDelay).to.equal(5000);
    expect(jsobj.historyKind).to.equal(qos.HistoryKind.KeepLast);
    expect(jsobj.historyDepth).to.equal(10);
    expect(jsobj.maxSamples).to.equal(10);
    expect(jsobj.maxInstances).to.equal(5);
    expect(jsobj.maxSamplesPerInstance).to.equal(2);
    tqos.delete();
  });

  it('set durability service policy historyKind only', function() {
    let tqos = qos.QoS.topicDefault();
    let defaultPolicy = tqos.durabilityService;
    tqos.durabilityService = { historyKind: qos.HistoryKind.KeepAll };
    let jsobj = tqos.durabilityService;
    expect(jsobj.serviceCleanupDelay).to.equal(
      defaultPolicy.serviceCleanupDelay
    );
    expect(jsobj.historyKind).to.equal(qos.HistoryKind.KeepAll);
    expect(jsobj.historyDepth).to.equal(defaultPolicy.historyDepth);
    expect(jsobj.maxSamples).to.equal(defaultPolicy.maxSamples);
    expect(jsobj.maxInstances).to.equal(defaultPolicy.maxInstances);
    expect(jsobj.maxSamplesPerInstance).to.equal(
      defaultPolicy.maxSamplesPerInstance);
    tqos.delete();
  });

  it('set durability service policy ignores inapplicable values', function() {
    let tqos = qos.QoS.topicDefault();
    tqos.durabilityService = {
      historyKind: qos.HistoryKind.KeepAll,
      extraField: 1,
    };
    let jsobj = tqos.durabilityService;
    expect(jsobj.historyKind).to.equal(qos.HistoryKind.KeepAll);
    expect(jsobj.extraField).to.be.undefined;
    tqos.delete();
  });

  it('set durability service policy uses default values for empty input',
    function() {
      let tqos = qos.QoS.topicDefault();
      let defaultPolicy = tqos.durabilityService;
      tqos.durabilityService = {};
      let jsobj = tqos.durabilityService;
      expect(jsobj.serviceCleanupDelay).to.equal(
        defaultPolicy.serviceCleanupDelay
      );
      expect(jsobj.historyKind).to.equal(defaultPolicy.historyKind);
      expect(jsobj.historyDepth).to.equal(defaultPolicy.historyDepth);
      expect(jsobj.maxSamples).to.equal(defaultPolicy.maxSamples);
      expect(jsobj.maxInstances).to.equal(defaultPolicy.maxInstances);
      expect(jsobj.maxSamplesPerInstance).to.equal(
        defaultPolicy.maxSamplesPerInstance);
      tqos.delete();
    });

});
