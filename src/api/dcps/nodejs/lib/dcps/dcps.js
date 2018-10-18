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

/**
 * DCPS for Node.js
 * @module dcps
 */
const ddsc99 = require('./ddsc99');
const qos = require('./qos');
const qosc99 = require('./qosc99');
const ddstopic = require('./ddstopic');

/**
 * State mask for samples.
 */
const StateMask = {
  sample: {
    read: 1,
    not_read: 2,
    any: 1 | 2,
  },
  view: {
    new: 4,
    not_new: 8,
    any: 4 | 8,
  },
  instance: {
    alive: 16,
    not_alive_disposed: 32,
    not_alive_no_writers: 64,
    any: 16 | 32 | 64,
  },
  any: (1 | 2) | (4 | 8) | (16 | 32 | 64),
};

/**
 * Communication statuses.
 */
const StatusMask = {
  data_available: 1024,
  data_on_readers: 512,
  inconsistent_topic: 1,
  liveliness_changed: 4096,
  liveliness_lost: 2048,
  offered_deadline_missed: 2,
  offered_incompatible_qos: 32,
  publication_matched: 8192,
  requested_deadline_missed: 4,
  requested_incompatible_qos: 64,
  sample_lost: 128,
  sample_rejected: 256,
  subscription_matched: 16384,
};

/**
 * Reasons for a sample being rejected.
 */
const SampleRejectedStatusKind = {
  not: 0,
  instance_limit: 1,
  samples_limit: 2,
  samples_per_instance: 3,
};

/**
 * Miscellaneous dds constants used in this file. Lifted from dds.h.
 */
const DDSConstants = {
  DDS_LENGTH_UNLIMITED: -1,
  DDS_DOMAIN_DEFAULT: 0x7FFFFFFF,
  DDS_HANDLE_NIL: 0,
  DDS_INFINITY: 0x7FFFFFFFFFFFFFFF,
  DDS_RETCODE_TIMEOUT: 10,
};

/**
 * Throw a JavaScript error.
 * @param {string} err error message
 */
function errorHandler(err) {
  throw err;
}

/**
 * Base Class to create and delete a dds entity.
 */
class Entity {
  /**
   * Create an Entity handle.
   * @param {Buffer} c_handle buffer for entity C99 handle
   */
  constructor(c_handle) {
    this._c_handle = c_handle;
  }

  /**
   * Get Buffer representing the C99 handle to the entity.
   * @returns {Buffer} buffer for entity C99 handle
   */
  get handle() {
    return this._c_handle;
  }

  /**
   * Get the existing set of QoS policies for the entity.
   * QoS must be deleted by user.
   * @returns {QoS}
   */
  get qos() {
    var entityQos = new qos.QoS();
    qosc99.qosGet(this._c_handle, entityQos.cqos, errorHandler);
    return entityQos;
  }

  /**
   * Delete this entity.
   */
  delete() {
    /** Delete the entity only if it hasn't been deleted */
    if (this._c_handle !== null) {
      ddsc99.entityDelete(this._c_handle);
      this._c_handle = null;
    }
  }

  /**
   * Method used by addListener. Listens for the status condition
   * in ws and calls cb when the status is triggered. We stop listening when
   * stopCondition evaluates to true.
   *
   * In the default case, stopCondition() is
   * always false so we listen forever.
   * @param {Waitset} ws
   * @param {function} cb
   */
  listen(ws, listenerDict, stopCondition) {
    let self = this;
    let listenerKeys = Object.keys(listenerDict);

    ws.wait(DDSConstants.DDS_INFINITY, function(err, res) {
      if (err) throw err;
      for (let i = 0; i < Object.keys(listenerDict).length; i++) {
        // take the status
        let stat = self.takeStatus(StatusMask[listenerKeys[i]]);
        if (stat > 0) {
          listenerDict[listenerKeys[i]]();
        }
      }
      if (!stopCondition()) {
        self.listen(ws, listenerDict, stopCondition);
      }
    });
  }

  /**
   * Create a listener on the entity.
   *
   * @param {number} mask Communication Status that we are listening for
   * @param {function} cb callback to run when the communication status is
   * triggered
   * @throws {Error} Throws an error when the mask does not correspond to the
   * entity
   */
  addListener(listenerDict, stopCondition = () => false) {
    let listenerKeys = Object.keys(listenerDict);
    let statCond = new StatusCondition(this);

    let mask = 0;
    for (let i = 0; i < listenerKeys.length; i++) {
      mask |= StatusMask[listenerKeys[i]];
      if (!statCond.checkMask(mask)) {
        throw new Error('Cannot attach listener ' +
          listenerDict[listenerKeys[i]] + ' to entity of type ' +
          this.constructor.name);
      }
      if (!Object.keys(StatusMask).includes(listenerKeys[i])) {
        throw new Error('No listener named ' + listenerKeys[i]);
      }
    }
    statCond.enable(mask);

    let ws = new Waitset();
    ws.attach(statCond, this);

    this.listen(ws, listenerDict, stopCondition);
  }

  /**
   * Reads the status(es) set for the entity based on the enabled status
   * and mask set. Clears the status after reading.
   * @param {number} mask communication status
   */
  takeStatus(mask) {
    return ddsc99.statusTake(this.handle, mask);
  }
  /**
  * Reads the status(es) set for the entity based on the enabled status
  * and mask set. Clears the status after reading.
  * @param {number} mask communication status
  */
  readStatus(mask) {
    return ddsc99.statusRead(this.handle, mask);
  }

  /**
   * Gets the enabled statuses.
   */
  enabledStatuses() {
    return ddsc99.statusGetEnabled(this.handle);
  }

  /**
   * Gets the status changes.
   */
  statusChanges() {
    return ddsc99.statusChanges(this.handle);
  }
}

/**
 * Class representing a dds domain participant.
 */
class Participant extends Entity {

  /**
   * Create a handle to a Participant.
   * @param {number} domainId 0 <= domainId <= 230
   * (default DDSConstants.DDS_DOMAIN_DEFAULT)
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   */
  constructor(
    domainId = DDSConstants.DDS_DOMAIN_DEFAULT,
    qos = null,
    listener = null
  ) {
    let c_handle = ddsc99.participantCreate(
      domainId,
      qos !== null ? qos.cqos : null,
      listener,
      errorHandler
    );
    super(c_handle);
  }

  /**
   * Creates a Topic on this participant.
   * @param {string} topicName
   * @param {string} typeName
   * @param {string} keys (comma separated string)
   * @param {string} xml topic metadata
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Topic} topic instance
   */
  createTopic(
    topicName,
    typeName,
    keys,
    xml,
    qos = null,
    listener = null
  ) {

    return new Topic(
      this,
      topicName,
      typeName,
      keys,
      xml,
      null,
      null,
      qos,
      listener
    );
  }

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

    return new Topic(
      this,
      topicName,
      typeSupport.getTypename(),
      typeSupport.getKeys(),
      typeSupport.getXML(),
      null,
      typeSupport,
      qos,
      listener
    );
  }

  /**
   * Finds a topic over the wire.
   * Hangs until it's found.
   * Calls callback() when done (which defaults to a function
   * which does nothing).
   *
   * @param {string} topicName
   * @param {function} callback Callback to call when topic is found
   * @returns {Topic} Topic instance
   */
  findTopic(topicName, callback = () => { }) {
    // get the topic and run the callback
    let topicHandle = ddsc99.topicFind(this.handle, topicName);
    callback();

    let typeName = ddsc99.topicGetTypeName(topicHandle);
    let keys = ddsc99.topicGetKeylist(topicHandle);
    let xml = ddsc99.topicGetMetadescriptor(topicHandle);

    return new Topic(
      this,
      topicName,
      typeName,
      keys,
      xml,
      topicHandle,
      null,
      null
    );
  }

  /**
   * Creates a Reader on this participant.
   * @param {Topic} topic
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Reader} reader instance
   */
  createReader(topic, qos = null, listener = null) {
    return new Reader(this, topic, qos, listener);
  }

  /**
   * Creates a Writer on this participant.
   * @param {Topic} topic topic
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Writer} writer instance
   */
  createWriter(topic, qos = null, listener = null) {
    return new Writer(this, topic, qos, listener);
  }

  /**
   * Creates a Publisher on this participant.
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Publisher} publisher instance
   */
  createPublisher(qos = null, listener = null) {
    return new Publisher(this, qos, listener);
  }

  /**
   * Creates a Subscriber on this participant.
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Subscriber} subscriber instance
   */
  createSubscriber(qos = null, listener = null) {
    return new Subscriber(this, qos, listener);
  }

};

/**
 * Class representing a dds topic descriptor
 */
class TopicDescriptor {
  /**
   * Create a Topic descriptor.
   * @param {string} typeName
   * @param {string} key (comma separated string)
   * @param {string} xml topic metadata
   * @returns {TopicDescriptor} topic descriptor instance
   */
  constructor(typeName, key, xml) {
    this._typeName = typeName;
    this._keyList = key;
    this._metaDescriptor = xml;
    this._cAddr = ddsc99.topicDescriptorCreate(
      typeName,
      key,
      xml,
      errorHandler
    );
  }

  /**
   * Get a dds topic descriptor handle.
   * @returns {Buffer} buffer for topic descriptor C99 handle
   */
  get cAddr() {
    if (this._cAddr === null) {
      throw new ReferenceError('TopicDescriptor has been deleted');
    }
    return this._cAddr;
  }

  /**
   * Get the type name of the topic descriptor.
   * @returns {string}
   */
  get typeName() {
    return this._typeName;
  }

  /**
   * Get the type key list, a commma separated string.
   * @returns {string}
   */
  get keyList() {
    return this._keyList;
  }

  /**
   * Get the metadescriptor associated with the type.
   * @returns {string}
   */
  get metaDescriptor() {
    return this._metaDescriptor;
  }

  /**
   * Delete a dds topic descriptor.
   */
  delete() {
    /** Delete the topicDescriptor only if it hasn't been deleted */
    if (this._cAddr !== null) {
      ddsc99.topicDescriptorDelete(this._cAddr, errorHandler);
      this._cAddr = null;
    }
  }
};

/**
 * Class representing a dds topic entity.
 */
class Topic extends Entity {
  /**
   * Create a dds topic entity
   * @param {Participant} participant
   * @param {string} topicName
   * @param {string} typeName
   * @param {string} keys (comma separated string)
   * @param {string} xml topic metadata
   * @param {Buffer} topic_handle
   * @param {TypeSupport} typeSupport
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   */
  constructor(
    participant,
    topicName,
    typeName,
    keys,
    xml,
    topic_handle = null,
    typeSupport = null,
    qos = null,
    listener = null
  ) {

    let topicDescriptor = new TopicDescriptor(typeName, keys, xml);
    let c_handle = topic_handle;
    if (c_handle === null){
      c_handle = ddsc99.topicCreate(
        participant.handle,
        topicDescriptor.cAddr,
        topicName,
        qos !== null ? qos.cqos : null,
        listener,
        errorHandler
      );
    }

    super(c_handle);
    this._name = topicName;
    this._typeSupport = typeSupport;
    if (typeSupport === null) {
      this._typeSupport = new ddstopic.TypeSupport(typeName, keys, xml);
    }
    this._descriptor = topicDescriptor;
  }

  /**
   * Get the topic descriptor.
   * @returns {TopicDescriptor}
   */
  get descriptor() {
    return this._descriptor;
  }

  /**
   * Get the topic name.
   * @returns {string}
   */
  get name() {
    return this._name;
  }

  /**
   * Get the type support object.
   * @returns {TypeSupport}
   */
  get typeSupport() {
    return this._typeSupport;
  }
};

/**
 * Class representing a dds publisher entity.
 */
class Publisher extends Entity {
  /**
   * Create a Publisher.
   * @param {Participant} participant
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   */
  constructor(participant, qos = null, listener = null) {
    let c_handle = ddsc99.publisherCreate(
      participant.handle,
      qos !== null ? qos.cqos : null,
      listener,
      errorHandler
    );
    super(c_handle);
  }

  /**
   * Create a Writer using this publisher.
   * @param {Topic} topic topic handle
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Writer} writer instance
   */
  createWriter(topic, qos = null, listener = null) {
    return new Writer(this, topic, qos, listener);
  }

}

/**
 * Class representing a dds writer entity.
 */
class Writer extends Entity {
  /**
   * Create a Writer.
   * @param {Participant|Publisher} dpOrPub Participant or Publisher to create
   * the writer on
   * @param {Topic} topic
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   */
  constructor(dpOrPub, topic, qos = null, listener = null) {
    let c_handle = ddsc99.writerCreate(
      dpOrPub.handle,
      topic.handle,
      qos !== null ? qos.cqos : null,
      listener,
      errorHandler
    );
    super(c_handle);
    this._topic = topic;
  }

  /**
   * Get the topic.
   * @returns {Topic}
   */
  get topic() {
    return this._topic;
  }

  /**
   * Write a dds sample.
   * @param {object} data data as a javascript object
   * @returns {number} status of dds write
   */
  write(data) {
    let buffer = this.topic.typeSupport.copyin(data);
    return ddsc99.write(this.handle, buffer, errorHandler);
  }
};

/**
 * Class representing a dds subscriber entity.
 */
class Subscriber extends Entity {
  /**
   * Create a dds subscriber entity.
   * @param {Participant} participant
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   */
  constructor(participant, qos = null, listener = null) {
    let c_handle = ddsc99.subscriberCreate(
      participant.handle,
      qos !== null ? qos.cqos : null,
      listener,
      errorHandler
    );
    super(c_handle);
  }

  /**
   * Create a Reader using subscriber.
   * @param {Topic} topic
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   * @returns {Reader} reader instance
   */
  createReader(topic, qos = null, listener = null) {
    return new Reader(this, topic, qos, listener);
  }
}

/**
 * Class representing a dds reader entity.
 */
class Reader extends Entity {
  /**
   * Create a dds reader entity.
   * @param {Participant|Subscriber} dpOrSub Participant or Subscriber to create
   * the Reader on.
   * @param {Topic} topic
   * @param {QoS} qos (default undefined)
   * @param {object} listener (default undefined)
   */
  constructor(dpOrSub, topic, qos = null, listener = null) {
    let c_handle = ddsc99.readerCreate(
      dpOrSub.handle,
      topic.handle,
      qos !== null ? qos.cqos : null,
      null,
      errorHandler
    );
    super(c_handle);
    this._topic = topic;
  }

  /**
   * Get the topic on this reader.
   * @returns {Topic}
   */
  get topic() {
    return this._topic;
  }

  /**
   * Nulls out non-keyed fields when valid_data === false. For internal use.
   * @param {Array} dataArray Array returned from read,take,readCond or takeCond
   */
  nullOutNonKeyedFields(dataArray) {
    // key list as an array
    let keyedFields = this.topic.descriptor.keyList.split(',');

    // loop through all samples
    for (let i = 0; i < dataArray.length; i++) {
      // loop through all field names in this sample, if the same is not valid
      if (!dataArray[i].info.valid_data) {
        for (let j = 0; j < Object.keys(dataArray[i].sample).length; j++) {
          // if the field name is not in the key-list, null it out the data
          // for that field
          let fieldName = Object.keys(dataArray[i].sample)[j];
          if (!keyedFields.includes(fieldName)) {
            dataArray[i].sample[fieldName] = null;
          }
        }
      }
    }
    return dataArray;
  }

  /**
   * Read dds sample.
   * @param {number} maxSample number of samples to read
   * @returns {object} array of sample and info pairs
   */
  read(maxSample) {
    let topicStruct = this.topic.typeSupport.getRefType();
    return this.nullOutNonKeyedFields(ddsc99.read(
      this.handle,
      maxSample,
      topicStruct,
      errorHandler
    ));
  }

  /**
   * Read dds sample with condition.
   * @param {number} maxSample number of samples to read
   * @param {Condition} cond condition to read against
   * @returns {object} array of sample and info pairs
   */
  readCond(maxSample, cond) {
    let topicStruct = this.topic.typeSupport.getRefType();
    return this.nullOutNonKeyedFields(ddsc99.readCond(
      this.handle,
      maxSample,
      topicStruct,
      cond.handle
    ));
  }

  /**
   * Take dds sample.
   * @param {number} maxSample number of samples to read
   * @returns {object} array of sample and info pairs
   */
  take(maxSample) {
    let topicStruct = this.topic.typeSupport.getRefType();
    return this.nullOutNonKeyedFields(ddsc99.take(
      this.handle,
      maxSample,
      topicStruct,
      errorHandler
    ));
  }

  /**
   * Take dds sample with condition.
   * @param {number} maxSample number of samples to read
   * @param {Condition} cond condition to take against
   * @returns {object} array of sample and info pairs
   */
  takeCond(maxSample, cond) {
    let topicStruct = this.topic.typeSupport.getRefType();
    return this.nullOutNonKeyedFields(ddsc99.takeCond(
      this.handle,
      maxSample,
      topicStruct,
      cond.handle
    ));
  }

  /**
   * Wait for historical data.
   * @param {number} timeout timeout in nanoseconds
   * @returns {number} 0 on success (throws Error on failure)
   */
  waitForHistoricalDataBlocking(timeout) {
    return ddsc99.readerWaitForHistoricalDataBlocking(this.handle, timeout);
  }

  /**
   * Asynchronously wait for historical data.
   * @param {number} timeout yimeout in nanoseconds
   * @param {function} cb callback to call when
   * dds_reader_wait_for_historical_data completes
   */
  waitForHistoricalData(timeout, cb) {
    ddsc99.readerWaitForHistoricalData(this.handle, timeout, cb);
  }
};

/**
 * Abstract class for Conditions. Holds the handle to the C99 condition.
 */
class Condition {
  /**
   * Sets the handle to the condition handle we pass in.
   * @param {object} cond
   */
  constructor(cond = null) {
    this._handle = cond;
  }

  /**
   * Returns the C99 handle of the condition.
   * @returns {Buffer} Buffer for condition C99 handle
   */
  get handle() {
    return this._handle;
  }

  /**
   * Checks whether the condition is triggered or not.
   * @returns {boolean}
   */
  triggered() {
    return ddsc99.conditionTriggered(this._handle);
  }

  /**
   * Deletes the C99 handle and sets it to null.
   */
  delete() {
    // only delete the condition if it has been added and not deleted
    if (this._handle !== null) {
      ddsc99.conditionDelete(this._handle);
      this._handle = null;
    }
  }
};

/**
 * Wrapper class for ReadConditions.
 */
class ReadCondition extends Condition {
  /**
   * Creates a read condition.
   * @param {Reader} reader
   * @param {number} mask (see StateMask)
   */
  constructor(reader, mask) {
    let condition = ddsc99.readConditionCreate(reader.handle, mask);
    super(condition);
  }
};

/**
 * Wrapper class for GuardConditions.
 */
class GuardCondition extends Condition {
  /**
   * Creates a guard condition.
   */
  constructor() {
    super(ddsc99.guardConditionCreate());
  }

  /**
   * Sets the guard condition to true.
   */
  trigger() {
    ddsc99.guardTrigger(this.handle);
  }

  /**
   * Sets the guard condition to false.
   */
  reset() {
    ddsc99.guardReset(this.handle);
  }
};

/**
 * Wrapper class for QueryConditions.
 */
class QueryCondition extends Condition {
  constructor(reader, mask, expression, params) {
    let condition = ddsc99.queryConditionCreateSql(
      reader.handle,
      mask,
      expression,
      params,
      params.length
    );
    super(condition);
  }
};

/**
 * Class for a Status Condition.
 */
class StatusCondition extends Condition {
  constructor(entity, mask = null) {
    super(ddsc99.statusCondition(entity.handle));
    this._entity = entity;
    this._mask = 0;
    if (mask !== null) {
      if (this.checkMask(mask)) {
        this._mask = mask;
        this.enable(mask);
      }
    }

    this._participantMasks = Object.values(StatusMask);
    this._topicMasks = [StatusMask.inconsistent_topic];
    this._subscriberMasks = [StatusMask.data_on_readers];
    this._readerMasks = [StatusMask.sample_rejected,
      StatusMask.liveliness_changed, StatusMask.requested_deadline_missed,
      StatusMask.requested_incompatible_qos, StatusMask.data_available,
      StatusMask.sample_lost, StatusMask.subscription_matched];
    this._writerMasks = [StatusMask.liveliness_lost,
      StatusMask.offered_deadline_missed, StatusMask.offered_incompatible_qos,
      StatusMask.publication_matched];
  }

  // returns true if the mask corresponds to the entity
  // false otherwise
  checkMask(mask) {
    let entityMasks = [];
    switch (this._entity.constructor.name) {
      case 'Participant':
        entityMasks = Object.values(StatusMask);
        break;
      case 'Topic':
        entityMasks = [StatusMask.inconsistent_topic];
        break;
      case 'Subscriber':
        entityMasks = [StatusMask.data_on_readers];
        break;
      case 'Reader':
        entityMasks = [StatusMask.sample_rejected,
          StatusMask.liveliness_changed, StatusMask.requested_deadline_missed,
          StatusMask.requested_incompatible_qos, StatusMask.data_available,
          StatusMask.sample_lost, StatusMask.subscription_matched];
        break;
      case 'Writer':
        entityMasks = [StatusMask.liveliness_lost,
          StatusMask.offered_deadline_missed,
          StatusMask.offered_incompatible_qos, StatusMask.publication_matched];
        break;
      default:
        entityMasks = Object.values(StatusMask);
        break;
    }

    // make sure the mask is a combination of the masks for the entity
    let m = 0;
    for (let i = 0; i < entityMasks.length; i++) {
      // use toString here as LHS is a 32 bit # and RHS is a 64 bit #
      if ((entityMasks[i] & mask).toString(10)
        === entityMasks[i].toString(10)) {
        m = m | entityMasks[i];
      }
    }
    if (m !== mask) {
      return false;
    }
    return true;
  }

  /**
   * Enables the mask on the entity.
   * @param {number} mask
   */
  enable(mask) {
    if (!this.checkMask(mask)) {
      throw new Error('Mask ' + mask + ' does not correspond to an entity' +
        ' of type ' + this._entity.constructor.name);
    }
    this._mask = mask;
    ddsc99.enableStatus(this._entity.handle, mask);
  }

  get mask() {
    return this._mask;
  }
};

/**
 * Wrapper class for waitsets.
 */
class Waitset {
  constructor(cond = null) {
    this._handle = ddsc99.waitsetCreate();
    // _attachedConditions consists pairs (cond,entity that cond is attached
    // to) using attach()
    this._attachedConditions = new Map();
    if (cond !== null) {
      this.attach(cond, cond);
    }
  }

  /**
   * Deletes all the conditions attached to the waitset and then deletes the
   * waitset.
   */
  delete() {
    ddsc99.waitsetDelete(this.handle);
    this._handle = null;
    this._attachedConditions = null;
  }

  /**
   * Returns the Buffer representing C99 handle to this waitset.
   * @returns {Buffer}
   */
  get handle() {
    return this._handle;
  }

  /**
   * Returns the dictionary of conditions attached to this waitset.
   * @returns {object}
   */
  get conditions() {
    return this._attachedConditions;
  }

  /**
   * Attaches cond to this waitset. Adds the condition to a dictionary
   * of conditions.
   * @param {Condition} cond condition to attach to the waitset
   * @param {object} entity object (of any type) to attach to the condition
   */
  attach(cond, entity) {
    ddsc99.waitsetAttach(this.handle, cond.handle, cond.handle);
    // Map cannot handle a buffer key. Use the string version of &cond.handle.
    this._attachedConditions.set(cond.handle.address().toString(16), entity);
  }

  /**
   * Detaches the condition from the waitset and removes it from the dictionary.
   * @param {Condition} cond handle to the condition to detach from the waitset
   */
  detach(cond) {
    this._attachedConditions.delete(cond.handle.address().toString(16));
    return ddsc99.waitsetDetach(this.handle, cond.handle);
  }

  /**
   * Waits on the waitset for timeout nanoseconds.
   * @param {number} timeout
   * Deprecated -- do not use. Use wait instead.
   */
  waitBlocking(timeout = DDSConstants.DDS_INFINITY) {
    let triggeredHandles = ddsc99.waitsetWaitBlocking(
      this.handle,
      this._attachedConditions.size,
      timeout
    );
    // if we've timed out
    if (triggeredHandles === 0) {
      return 0;
    }

    return triggeredHandles.toArray().map(
      (h) => this._attachedConditions.get(h.address().toString(16))
    );
  }

  /**
   * Waits on the waitset asyncronously. When it unblocks, the callback
   * cb is called. cb should have the form
   * function cb(err,res) {
   *    if (err) { code to run if dds_waitset_wait threw an error }
   *    code to run otherwise.
   * }
   * @param {number} timeout timeout in nanoseconds
   * @param {function} cb callback to call on completion of dds_waitset_wait
   */
  wait(timeout = DDSConstants.DDS_INFINITY, cb) {
    ddsc99.waitsetWait(this.handle, this._attachedConditions.size,
      timeout, function(err, result) {
        if (result === 0) {
          err = new Error(DDSConstants.DDS_RETCODE_TIMEOUT);
        }
        cb(err, result);
      });
  };
};

module.exports = {
  Participant: Participant,
  Topic: Topic,
  Publisher: Publisher,
  Writer: Writer,
  Subscriber: Subscriber,
  Reader: Reader,
  TopicDescriptor: TopicDescriptor,
  Waitset: Waitset,
  ReadCondition: ReadCondition,
  GuardCondition: GuardCondition,
  QueryCondition: QueryCondition,
  StateMask: StateMask,
  SampleRejectedStatusKind: SampleRejectedStatusKind,
  StatusCondition: StatusCondition,
  StatusMask: StatusMask,
  DDSConstants: DDSConstants,
  // Instance: Instance,
};
