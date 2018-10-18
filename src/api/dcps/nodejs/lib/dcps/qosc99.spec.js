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

const qosc99 = require('./qosc99');
const ddsc99 = require('./ddsc99');
const ddserr = require('./ddserr');
const expect = require('chai').expect;
const path = require('path');

const qospath = 'test_data' + path.sep + 'DDS_PersistentQoS_All.xml';
const uri = 'file://' + path.resolve(qospath);
const profile = 'DDS PersistentQosProfile';

const entityQosFilePath = 'test_data' + path.sep + 'DDS_EntityQoS.xml';
const entityQosUri = 'file://' + path.resolve(entityQosFilePath);
const entityQosProfile = 'DDS EntityQosProfile';

function emptyCallback() {
  /* Does nothing */
}

describe('qosc99: qos create, delete and get default entity qos tests',
  function() {
    it('create a qos', function() {
      let qos = qosc99.qosCreate();
      expect(qos).to.not.be.null;
      qosc99.qosDelete(qos);
    });

    it('ffi throws error calling qos delete for invalid qos', function() {
      expect(function() {
        qosc99.qosDelete(111);
      }).to.throw(TypeError);
    });

    it('qos_get test', function() {
      this.timeout(10000);
      let dp = ddsc99.participantCreate();
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.qosGet(dp, qos);
      }).not.to.throw(Error);

      qosc99.qosDelete(qos);
      ddsc99.entityDelete(dp);
    });

    it('ffi throws error calling qos_get for invalid argument', function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.qosGet(0, qos);
      }).to.throw(Error);
      qosc99.qosDelete(qos);
    });

    it('ffi throws error for calling default participant qos without argument',
      function() {
        expect(function() {
          qosc99.defaultParticipantQos();
        }).to.throw(TypeError);
      });

    it('ffi throws error for calling default topic qos without argument',
      function() {
        expect(function() {
          qosc99.defaultTopicQos();
        }).to.throw(TypeError);
      });

    it('ffi throws error for calling default publisher qos without argument',
      function() {
        expect(function() {
          qosc99.defaultPublisherQos();
        }).to.throw(TypeError);
      });

    it('ffi throws error for calling default subscriber qos without argument',
      function() {
        expect(function() {
          qosc99.defaultSubscriberQos();
        }).to.throw(TypeError);
      });

    it('ffi throws error for calling default reader qos without argument',
      function() {
        expect(function() {
          qosc99.defaultReaderQos();
        }).to.throw(TypeError);
      });

    it('ffi throws error for calling default writer qos without argument',
      function() {
        expect(function() {
          qosc99.defaultWriterQos();
        }).to.throw(TypeError);
      });
  });

describe('qosc99: qosprovider tests', function() {

  it('create a qos provider', function() {
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(qp).to.not.be.null;
  });

  it('qos provider create fails with invalid uri', function() {
    expect(function() {
      qosc99.qosProviderCreate(null, profile);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);
  });

  it('ffi throws error from qp create for invalid type', function() {
    expect(function() {
      qosc99.qosProviderCreate(0, profile);
    }).to.throw(TypeError);
  });

  it('catch the ffi thrown error and return null', function() {
    let qp = qosc99.qosProviderCreate(0, profile, emptyCallback);
    expect(qp).to.be.null;
  });

  it('get participant qos from qos provider', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetParticipantQos(qp, qos);
    let value = qosc99.getDurability(qos);
    expect(value).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws type error while getting participantQos from qp', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(function() {
      qosc99.qosProviderGetParticipantQos(qp, qos, 0);
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('get participant qos fails from null qos provider', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.qosProviderGetParticipantQos(null, qos);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    qosc99.qosDelete(qos);
  });

  it('get topic qos from qos provider', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let value = qosc99.getDurability(qos);
    expect(value).to.equal(3);
    qosc99.qosDelete(qos);
  });

  it('ffi throws type error while getting topicQos from qp', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(function() {
      qosc99.qosProviderGetTopicQos(qp, qos, 0);
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('get topic qos fails from null qos provider', function() {
    let qos = qosc99.qosCreate();

    expect(function() {
      qosc99.qosProviderGetTopicQos(null, qos);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    qosc99.qosDelete(qos);
  });

  it('get publisher qos from qos provider', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetPublisherQos(qp, qos);
    let jsobj = qosc99.getPresentation(qos);
    expect(jsobj.accessScope).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws type error while getting publisherQos from qp', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(function() {
      qosc99.qosProviderGetPublisherQos(qp, qos, 0);
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('get publisher qos fails from null qos provider', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.qosProviderGetPublisherQos(null, qos);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    qosc99.qosDelete(qos);
  });

  it('get subscriber qos from qos provider', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetSubscriberQos(qp, qos);
    let jsobj = qosc99.getPresentation(qos);
    expect(jsobj.accessScope).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws type error while getting subscriberQos from qp', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(function() {
      qosc99.qosProviderGetSubscriberQos(qp, qos, 0);
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('get subscriber qos fails from null qos provider', function() {
    let qos = qosc99.qosCreate();

    expect(function() {
      qosc99.qosProviderGetSubscriberQos(null, qos);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    qosc99.qosDelete(qos);
  });

  it('get reader qos from qos provider', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetReaderQos(qp, qos, 'a');
    let value = qosc99.getDurability(qos);
    expect(value).to.equal(3);
    qosc99.qosDelete(qos);
  });

  it('ffi throws type error while getting readerQos from qp', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(function() {
      qosc99.qosProviderGetReaderQos(qp, qos, 0);
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('get reader qos fails from null qos provider', function() {
    let qos = qosc99.qosCreate();

    expect(function() {
      qosc99.qosProviderGetReaderQos(null, qos);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    qosc99.qosDelete(qos);
  });

  it('get writer qos from qos provider', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetWriterQos(qp, qos);
    let value = qosc99.getDurability(qos);
    expect(value).to.equal(3);
    qosc99.qosDelete(qos);
  });

  it('ffi throws type error while getting writerQos from qp', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    expect(function() {
      qosc99.qosProviderGetWriterQos(qp, qos, 0);
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('get writer qos fails from null qos provider', function() {
    let qos = qosc99.qosCreate();

    expect(function() {
      qosc99.qosProviderGetWriterQos(null, qos);
    }).to.throw(ddserr.DDSError).with
      .property('ddsErrCode').to.be.within(1, 12);

    qosc99.qosDelete(qos);
  });
});

describe('qosc99: qget tests', function() {

  it('getUserdata qos policy', function() {
    let qos = qosc99.qosCreate();
    const buf = Buffer.from('hello');
    qosc99.setUserdata(qos, buf);

    let dataBuff = qosc99.getUserdata(qos);
    expect(dataBuff.toString()).to.be.equal('hello');
    qosc99.qosDelete(qos);
  });

  it('getUserdata qos with input buffer', function() {
    let qos = qosc99.qosCreate();
    const buf = Buffer.from([0x62, 0x75, 0x66, 0x66, 0x65, 0x72]);
    qosc99.setUserdata(qos, buf);

    let dataBuff = qosc99.getUserdata(qos);
    expect(dataBuff.toString()).to.be.equal('buffer');
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getUserdata with invalid argument',
    function() {
      expect(function() {
        qosc99.getUserdata(0);
      }).to.throw(TypeError);
    });

  it('getUserdata returns null for invalid argument', function() {
    let value = qosc99.getUserdata(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getTopicdata qos policy', function() {
    let qos = qosc99.qosCreate();
    const buf = Buffer.from('hello');
    qosc99.setTopicdata(qos, buf);

    let dataBuff = qosc99.getTopicdata(qos);
    expect(dataBuff.toString()).to.be.equal('hello');
    qosc99.qosDelete(qos);
  });

  it('getTopicdata qos with input buffer', function() {
    let qos = qosc99.qosCreate();
    const buf = Buffer.from([0x62, 0x75, 0x66, 0x66, 0x65, 0x72]);
    qosc99.setTopicdata(qos, buf);

    let dataBuff = qosc99.getTopicdata(qos);
    expect(dataBuff.toString()).to.be.equal('buffer');
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getTopicdata with invalid argument',
    function() {
      expect(function() {
        qosc99.getTopicdata(0);
      }).to.throw(TypeError);
    });

  it('getTopicdata returns null for invalid argument', function() {
    let value = qosc99.getTopicdata(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getGroupdata qos policy', function() {
    let qos = qosc99.qosCreate();
    const buf = Buffer.from('hello');
    qosc99.setGroupdata(qos, buf);

    let dataBuff = qosc99.getGroupdata(qos);
    expect(dataBuff.toString()).to.be.equal('hello');
    qosc99.qosDelete(qos);
  });

  it('getGroupdata qos with input buffer', function() {
    let qos = qosc99.qosCreate();
    const buf = Buffer.from([0x62, 0x75, 0x66, 0x66, 0x65, 0x72]);
    qosc99.setGroupdata(qos, buf);

    let dataBuff = qosc99.getGroupdata(qos);
    expect(dataBuff.toString()).to.be.equal('buffer');
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getGroupdata with invalid argument',
    function() {
      expect(function() {
        qosc99.getGroupdata(0);
      }).to.throw(TypeError);
    });

  it('getGroupdata returns null for invalid argument', function() {
    let value = qosc99.getGroupdata(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getDurability qos policy kind', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let kind = qosc99.getDurability(qos);
    expect(kind).to.equal(3);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getDurability with invalid argument',
    function() {
      expect(function() {
        qosc99.getDurability(0);
      }).to.throw(TypeError);
    });

  it('getDurability returns null for invalid argument', function() {
    let value = qosc99.getDurability(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getHistory qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetWriterQos(qp, qos);
    let jsobj = qosc99.getHistory(qos);
    expect(jsobj.kind).to.equal(0);
    expect(jsobj.depth).to.equal(100);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getHistory with invalid argument',
    function() {
      expect(function() {
        qosc99.getHistory(0);
      }).to.throw(TypeError);
    });

  it('getHistory returns null for invalid argument', function() {
    let value = qosc99.getHistory(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getResourceLimits qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetWriterQos(qp, qos);
    let jsobj = qosc99.getResourceLimits(qos);
    expect(jsobj.maxSamples).to.equal(-1);
    expect(jsobj.maxInstances).to.equal(-1);
    expect(jsobj.maxSamplesPerInstance).to.equal(-1);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getResourceLimits with invalid argument',
    function() {
      expect(function() {
        qosc99.getResourceLimits(0);
      }).to.throw(TypeError);
    });

  it('getResourceLimits returns null for invalid argument', function() {
    let value = qosc99.getResourceLimits(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getPresentation qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetSubscriberQos(qp, qos);
    let jsobj = qosc99.getPresentation(qos);
    expect(jsobj.accessScope).to.equal(0);
    expect(jsobj.coherentAccess).to.be.false;
    expect(jsobj.orderedAccess).to.be.false;
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getPresentation with invalid argument',
    function() {
      expect(function() {
        qosc99.getPresentation(0);
      }).to.throw(TypeError);
    });

  it('getPresentation returns null for invalid argument', function() {
    let value = qosc99.getPresentation(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getLifespan qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(entityQosUri, entityQosProfile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let lifespan = qosc99.getLifespan(qos);
    expect(lifespan).to.equal(3600);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getLifespan with invalid argument',
    function() {
      expect(function() {
        qosc99.getLifespan(0);
      }).to.throw(TypeError);
    });

  it('getLifespan returns null for invalid argument', function() {
    let value = qosc99.getLifespan(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getDeadline qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(entityQosUri, entityQosProfile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let deadline = qosc99.getDeadline(qos);
    expect(deadline).to.equal(5000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getDeadline with invalid argument',
    function() {
      expect(function() {
        qosc99.getDeadline(0);
      }).to.throw(TypeError);
    });

  it('getDeadline returns null for invalid argument', function() {
    let value = qosc99.getDeadline(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getLatencyBudget qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let duration = qosc99.getLatencyBudget(qos);
    expect(duration).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getLatencyBudget with invalid argument',
    function() {
      expect(function() {
        qosc99.getLatencyBudget(0);
      }).to.throw(TypeError);
    });

  it('getLatencyBudget returns null for invalid argument', function() {
    let value = qosc99.getLatencyBudget(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getOwnership qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let kind = qosc99.getOwnership(qos);
    expect(kind).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getOwnership with invalid argument',
    function() {
      expect(function() {
        qosc99.getOwnership(0);
      }).to.throw(TypeError);
    });

  it('getOwnership returns null for invalid argument', function() {
    let value = qosc99.getOwnership(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getOwnershipStrength qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetWriterQos(qp, qos);
    let value = qosc99.getOwnershipStrength(qos);
    expect(value).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getOwnershipStrength with invalid argument',
    function() {
      expect(function() {
        qosc99.getOwnershipStrength(0);
      }).to.throw(TypeError);
    });

  it('getOwnershipStrength returns null for invalid argument', function() {
    let value = qosc99.getOwnershipStrength(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getLiveliness qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(entityQosUri, entityQosProfile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let jsobj = qosc99.getLiveliness(qos);
    expect(jsobj.kind).to.equal(0);
    expect(jsobj.leaseDuration).to.equal(500);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getLiveliness with invalid argument',
    function() {
      expect(function() {
        qosc99.getLiveliness(0);
      }).to.throw(TypeError);
    });

  it('getLiveliness returns null for invalid argument', function() {
    let value = qosc99.getLiveliness(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getTimebasedFilter qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetReaderQos(qp, qos, 'a');
    let minSeparation = qosc99.getTimebasedFilter(qos);
    expect(minSeparation).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getTimebasedFilter with invalid argument',
    function() {
      expect(function() {
        qosc99.getTimebasedFilter(0);
      }).to.throw(TypeError);
    });

  it('getTimebasedFilter returns null for invalid argument', function() {
    let value = qosc99.getTimebasedFilter(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getPartition qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(entityQosUri, entityQosProfile);
    qosc99.qosProviderGetPublisherQos(qp, qos);
    let partArr = qosc99.getPartition(qos);
    expect(partArr.length).to.equal(2);
    expect(partArr[0]).to.be.equal('partition1');
    expect(partArr[1]).to.be.equal('partition2');
    qosc99.qosDelete(qos);
  });

  it('getPartition returns empty array', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(entityQosUri, entityQosProfile);
    qosc99.qosProviderGetSubscriberQos(qp, qos);
    let partArr = qosc99.getPartition(qos);
    expect(partArr.length).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getPartition with invalid argument',
    function() {
      expect(function() {
        qosc99.getPartition(0);
      }).to.throw(TypeError);
    });

  it('getPartition returns null for invalid argument', function() {
    let value = qosc99.getPartition(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getReliability qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let jsobj = qosc99.getReliability(qos);
    expect(jsobj.kind).to.equal(0);
    expect(jsobj.maxBlockingTime).to.equal(100000000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getReliability with invalid argument',
    function() {
      expect(function() {
        qosc99.getReliability(0);
      }).to.throw(TypeError);
    });

  it('getReliability returns null for invalid argument', function() {
    let value = qosc99.getReliability(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getTransportPriority qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let value = qosc99.getTransportPriority(qos);
    expect(value).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getTransportPriority with invalid argument',
    function() {
      expect(function() {
        qosc99.getTransportPriority(0);
      }).to.throw(TypeError);
    });

  it('getTransportPriority returns null for invalid argument', function() {
    let value = qosc99.getTransportPriority(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getDestinationOrder qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let kind = qosc99.getDestinationOrder(qos);
    expect(kind).to.equal(0);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getDestinationOrder with invalid argument',
    function() {
      expect(function() {
        qosc99.getDestinationOrder(0);
      }).to.throw(TypeError);
    });

  it('getDestinationOrder returns null for invalid argument', function() {
    let value = qosc99.getDestinationOrder(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getWriterDataLifecycle qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetWriterQos(qp, qos);
    let value = qosc99.getWriterDataLifecycle(qos);
    expect(value).to.be.true;
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getWriterDataLifecycle with invalid ' +
  'argument', function() {
    expect(function() {
      qosc99.getWriterDataLifecycle(0);
    }).to.throw(TypeError);
  });

  it('getWriterDataLifecycle returns null for invalid argument', function() {
    let value = qosc99.getWriterDataLifecycle(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getReaderDataLifecycle qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(entityQosUri, entityQosProfile);
    qosc99.qosProviderGetReaderQos(qp, qos);
    let jsobj = qosc99.getReaderDataLifecycle(qos);
    expect(jsobj.autopurgeNoWriterSamples).to.equal(1000);
    expect(jsobj.autopurgeDisposedSamplesDelay).to.equal(500);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getReaderDataLifecycle with invalid ' +
  'argument', function() {
    expect(function() {
      qosc99.getReaderDataLifecycle(0);
    }).to.throw(TypeError);
  });

  it('getReaderDataLifecycle returns null for invalid argument', function() {
    let value = qosc99.getReaderDataLifecycle(0, emptyCallback);
    expect(value).to.be.null;
  });

  it('getDurabilityService qos policy', function() {
    let qos = qosc99.qosCreate();
    let qp = qosc99.qosProviderCreate(uri, profile);
    qosc99.qosProviderGetTopicQos(qp, qos);
    let jsobj = qosc99.getDurabilityService(qos);
    expect(jsobj.serviceCleanupDelay).to.equal(3600000000000);
    expect(jsobj.historyKind).to.equal(0);
    expect(jsobj.historyDepth).to.equal(100);
    expect(jsobj.maxSamples).to.equal(8192);
    expect(jsobj.maxInstances).to.equal(4196);
    expect(jsobj.maxSamplesPerInstance).to.equal(8192);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling getDurabilityService with invalid argument',
    function() {
      expect(function() {
        qosc99.getDurabilityService(0);
      }).to.throw(TypeError);
    });

  it('getDurabilityService returns null for invalid argument', function() {
    let value = qosc99.getDurabilityService(0, emptyCallback);
    expect(value).to.be.null;
  });

});

describe('qosc99: qset tests', function() {

  it('setUserdata throws error for non buffer input', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.setUserdata(qos, 'hello');
    }).to.throw(Error);
    qosc99.qosDelete(qos);
  });

  it('setTopicdata throws error for non buffer input', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.setTopicdata(qos, 'hello');
    }).to.throw(Error);
    qosc99.qosDelete(qos);
  });

  it('setGroupdata throws error for non buffer input', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.setGroupdata(qos, 'hello');
    }).to.throw(Error);
    qosc99.qosDelete(qos);
  });

  it('setDurability policy kind', function() {
    let qos = qosc99.qosCreate();
    qosc99.setDurability(qos, 2);
    let kind = qosc99.getDurability(qos);
    expect(kind).to.equal(2);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setDurability with invalid argument',
    function() {
      expect(function() {
        qosc99.setDurability(0);
      }).to.throw(TypeError);
    });

  it('setHistory qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setHistory(qos, 0, 100);
    let jsobj = qosc99.getHistory(qos);
    expect(jsobj.kind).to.equal(0);
    expect(jsobj.depth).to.equal(100);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setHistory with invalid argument',
    function() {
      expect(function() {
        qosc99.setHistory(0);
      }).to.throw(TypeError);
    });

  it('setResourceLimits qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setResourceLimits(qos, 3, 2, 2);
    let jsobj = qosc99.getResourceLimits(qos);
    expect(jsobj.maxSamples).to.equal(3);
    expect(jsobj.maxInstances).to.equal(2);
    expect(jsobj.maxSamplesPerInstance).to.equal(2);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setResourceLimits with invalid argument',
    function() {
      expect(function() {
        qosc99.setResourceLimits(0);
      }).to.throw(TypeError);
    });

  it('setPresentation qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setPresentation(qos, 2, false, true);
    let jsobj = qosc99.getPresentation(qos);
    expect(jsobj.accessScope).to.equal(2);
    expect(jsobj.coherentAccess).to.be.false;
    expect(jsobj.orderedAccess).to.be.true;
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setPresentation with invalid argument',
    function() {
      expect(function() {
        qosc99.setPresentation(0);
      }).to.throw(TypeError);
    });

  it('setLifespan qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setLifespan(qos, 2000);
    let lifespan = qosc99.getLifespan(qos);
    expect(lifespan).to.equal(2000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setLifespan with invalid argument',
    function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.setLifespan(qos, 'aaa');
      }).to.throw(TypeError);
      qosc99.qosDelete(qos);
    });

  it('setDeadline qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setDeadline(qos, 10000);
    let deadline = qosc99.getDeadline(qos);
    expect(deadline).to.equal(10000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setDeadline with invalid argument',
    function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.setDeadline(qos, 'aaa');
      }).to.throw(TypeError);
      qosc99.qosDelete(qos);
    });

  it('setLatencyBudget qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setLatencyBudget(qos, 70000);
    let duration = qosc99.getLatencyBudget(qos);
    expect(duration).to.equal(70000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setLatencyBudget with invalid argument',
    function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.setLatencyBudget(qos, 'aaa');
      }).to.throw(TypeError);
      qosc99.qosDelete(qos);
    });

  it('setOwnership qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setOwnership(qos, 1);
    let kind = qosc99.getOwnership(qos);
    expect(kind).to.equal(1);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setOwnership with invalid argument',
    function() {
      expect(function() {
        qosc99.setOwnership(0);
      }).to.throw(TypeError);
    });

  it('setOwnershipStrength qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setOwnershipStrength(qos, 2);
    let value = qosc99.getOwnershipStrength(qos);
    expect(value).to.equal(2);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setOwnershipStrength with invalid argument',
    function() {
      expect(function() {
        qosc99.setOwnershipStrength(0);
      }).to.throw(TypeError);
    });

  it('setLiveliness qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setLiveliness(qos, 2, 100);
    let jsobj = qosc99.getLiveliness(qos);
    expect(jsobj.kind).to.equal(2);
    expect(jsobj.leaseDuration).to.equal(100);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setLiveliness with invalid argument',
    function() {
      expect(function() {
        qosc99.setLiveliness(0);
      }).to.throw(TypeError);
    });

  it('setTimebasedFilter qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setTimebasedFilter(qos, 1000);
    let minSeparation = qosc99.getTimebasedFilter(qos);
    expect(minSeparation).to.equal(1000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setTimebasedFilter with invalid argument',
    function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.setTimebasedFilter(qos, 'aaa');
      }).to.throw(TypeError);
      qosc99.qosDelete(qos);
    });

  it('setPartition qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setPartition(qos, ['part1', 'part2']);
    let partArr = qosc99.getPartition(qos);
    expect(partArr.length).to.equal(2);
    qosc99.qosDelete(qos);
  });

  it('setPartition throws exception for non array input', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.setPartition(qos, 'part1');
    }).to.throw(Error);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setPartition with invalid argument',
    function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.setPartition(qos, [0, 1]);
      }).to.throw(TypeError);
      qosc99.qosDelete(qos);
    });

  it('setReliability qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setReliability(qos, 1, 500);
    let jsobj = qosc99.getReliability(qos);
    expect(jsobj.kind).to.equal(1);
    expect(jsobj.maxBlockingTime).to.equal(500);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setReliability with invalid argument',
    function() {
      let qos = qosc99.qosCreate();
      expect(function() {
        qosc99.setReliability(qos, 1, 'aaa');
      }).to.throw(TypeError);
      qosc99.qosDelete(qos);
    });

  it('setTransportPriority qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setTransportPriority(qos, 3);
    let value = qosc99.getTransportPriority(qos);
    expect(value).to.equal(3);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setTransportPriority with invalid argument',
    function() {
      expect(function() {
        qosc99.setTransportPriority(0);
      }).to.throw(TypeError);
    });

  it('setDestinationOrder policy kind', function() {
    let qos = qosc99.qosCreate();
    qosc99.setDestinationOrder(qos, 1);
    let kind = qosc99.getDestinationOrder(qos);
    expect(kind).to.equal(1);
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setDestinationOrder with invalid argument',
    function() {
      expect(function() {
        qosc99.setDestinationOrder(0);
      }).to.throw(TypeError);
    });

  it('setWriterDataLifecycle qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setWriterDataLifecycle(qos, true);
    let value = qosc99.getWriterDataLifecycle(qos);
    expect(value).to.be.true;
    qosc99.qosDelete(qos);
  });

  it('ffi throws error for calling setWriterDataLifecycle with invalid ' +
  'argument', function() {
    expect(function() {
      qosc99.setWriterDataLifecycle(0);
    }).to.throw(TypeError);
  });

  it('setReaderDataLifecycle qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setReaderDataLifecycle(qos, 500, 1000);
    let jsobj = qosc99.getReaderDataLifecycle(qos);
    expect(jsobj.autopurgeNoWriterSamples).to.equal(500);
    expect(jsobj.autopurgeDisposedSamplesDelay).to.equal(1000);
    qosc99.qosDelete(qos);
  });

  it('ffi throws err setReaderDataLifecycle invalid arg', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.setReaderDataLifecycle(qos, 100, 'aaa');
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });

  it('setDurabilityService qos policy', function() {
    let qos = qosc99.qosCreate();
    qosc99.setDurabilityService(qos, 3600, 0, 50, 100, 10, 10);
    let jsobj = qosc99.getDurabilityService(qos);
    expect(jsobj.serviceCleanupDelay).to.equal(3600);
    expect(jsobj.historyKind).to.equal(0);
    expect(jsobj.historyDepth).to.equal(50);
    expect(jsobj.maxSamples).to.equal(100);
    expect(jsobj.maxInstances).to.equal(10);
    expect(jsobj.maxSamplesPerInstance).to.equal(10);
    qosc99.qosDelete(qos);
  });

  it('ffi throws err setDurabilityService invalid arg', function() {
    let qos = qosc99.qosCreate();
    expect(function() {
      qosc99.setDurabilityService(qos, 'aaa');
    }).to.throw(TypeError);
    qosc99.qosDelete(qos);
  });
});
