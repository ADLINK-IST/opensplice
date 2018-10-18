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
/* eslint camelcase: 0 */ // --> turn off camel case checking
/* eslint no-console: 0 */ // --> turn off no-console checking

/**
 * ddsc99 for Node.js
 * @module ddsc99
 */


const ffi = require('ffi');
const ref = require('ref');
const StructType = require('ref-struct');
const ArrayType = require('ref-array');
const ddserr = require('./ddserr');
const libs = require('./libs');

//  DDS constants used in this file
const DDS_DOMAIN_DEFAULT = 0x7fffffff;
// const DDS_RETCODE_TIMEOUT = 10;


//  Special types used in C functions
//  general rule: pX means pointer to X
const pvoid = ref.refType(ref.types.void); // = void *

//  general dds types
const dds_entity_t = pvoid;
const pdds_entity_t = ref.refType(dds_entity_t);
const dds_domainid_t = ref.types.uint32;
const dds_duration_t = ref.types.int64;

// dds qos types
const dds_qos_t = ref.types.void;
const pdds_qos_t = ref.refType(dds_qos_t);

// dds topic types
const dds_topic_descriptor_t = pvoid;
const pdds_topic_descriptor_t = ref.refType(dds_topic_descriptor_t);

// dds listener types
const pdds_participantlistener_t = pvoid;
const dds_topiclistener_t = pvoid;
const pdds_topic_listener_t = ref.refType(dds_topiclistener_t);
const dds_publisherlistener_t = pvoid;
const dds_writerlistener_t = pvoid;
const pdds_writerlistener_t = ref.refType(dds_writerlistener_t);
// for reader listener. may be removed later
const pfunc = ffi.Function('void', [dds_entity_t]);
const dds_readerlistener_t = StructType({
  on_requested_deadline_missed: pfunc,
  on_requested_incompatible_qos: pfunc,
  on_sample_rejected: pfunc,
  on_liveliness_changed: pfunc,
  on_data_available: pfunc,
  on_subscription_matched: pfunc,
  on_sample_lost: pfunc,
});
const pdds_readerlistener_t = ref.refType(dds_readerlistener_t);

// dds waitset and condition types
const dds_waitset_t = pvoid;
const dds_condition_t = pvoid;
const pdds_condition_t = ref.refType(dds_condition_t);
const dds_attach_t = pvoid;
const dds_condition_seq = StructType({
  length: ref.types.uint32,
  buffer: pdds_condition_t,
  release: ref.types.int8,
});
const pdds_condition_seq = ref.refType(dds_condition_seq);

const dds_instance_handle_t = ref.types.uint64;

// used in dds_read, dds_take
const dds_sample_info_t = StructType({
  sample_state: ref.types.int32,
  view_state: ref.types.int32,
  instance_state: ref.types.int32,
  valid_data: ref.types.int8,
  source_timestamp: ref.types.int64,
  instance_handle: ref.types.uint64,
  publication_handle: ref.types.uint64,
  disposed_generation_count: ref.types.uint32,
  no_writers_generation_count: ref.types.uint32,
  sample_rank: ref.types.uint32,
  generation_rank: ref.types.uint32,
  absolute_generation_rank: ref.types.uint32,
  reception_timestamp: ref.types.int64,
});

// Array types used in dds functions.
const PVoidArray = ArrayType(pvoid); // dds_read
const StringArray = ArrayType(ref.types.CString); // init, querycondition_create
const SampleInfoArray = ArrayType(dds_sample_info_t); // dds_read,dds_take

// Bindings for libdcpsc99.so
const c99_bindings = {

  dds_init: ['int', [
    'int',
    StringArray,
  ]],

  dds_fini: ['void', []],

  dds_topic_descriptor_create: [pdds_topic_descriptor_t, [
    ref.types.CString,
    ref.types.CString,
    ref.types.CString,
  ]],

  dds_topic_descriptor_delete: ['void', [
    pdds_topic_listener_t,
  ]],

  dds_participant_create: ['int', [
    pdds_entity_t,
    dds_domainid_t, // this is a uint32
    pdds_qos_t,
    pdds_participantlistener_t]],

  dds_entity_delete: ['void', [
    dds_entity_t]],

  dds_topic_create: ['int', [
    dds_entity_t,
    pdds_entity_t,
    pdds_topic_descriptor_t,
    ref.types.CString,
    pdds_qos_t,
    pdds_topic_listener_t]],

  dds_publisher_create: ['int', [
    dds_entity_t,
    pdds_entity_t,
    pdds_qos_t,
    dds_publisherlistener_t]],

  dds_writer_create: ['int', [
    dds_entity_t,
    pdds_entity_t,
    dds_entity_t,
    pdds_qos_t,
    pdds_writerlistener_t]],

  dds_subscriber_create: ['int', [
    dds_entity_t,
    pdds_entity_t,
    pdds_qos_t,
    pvoid,
  ]],

  dds_reader_create: ['int', [
    dds_entity_t,
    pdds_entity_t,
    dds_entity_t,
    pvoid,
    pdds_readerlistener_t,
  ]],

  dds_write: ['int', [
    dds_entity_t,
    pvoid]],

  dds_read: ['int', [
    dds_entity_t,
    PVoidArray,
    ref.types.uint32,
    SampleInfoArray,
    ref.types.uint32,
  ]],

  dds_take: ['int', [
    dds_entity_t,
    PVoidArray,
    ref.types.uint32,
    SampleInfoArray,
    ref.types.uint32,
  ]],

  dds_return_loan: ['void', [
    dds_entity_t,
    PVoidArray,
    ref.types.uint32,
  ]],

  dds_free: ['void', [
    pvoid,
  ]],

  dds_topic_find: [dds_entity_t, [
    dds_entity_t,
    ref.types.CString,
  ]],

  dds_topic_get_name: [ref.types.CString, [
    dds_entity_t,
  ]],

  dds_topic_get_metadescriptor: [ref.types.CString, [
    dds_entity_t,
  ]],

  dds_topic_get_keylist: [ref.types.CString, [
    dds_entity_t,
  ]],

  dds_topic_get_type_name: [ref.types.CString, [
    dds_entity_t,
  ]],

  dds_waitset_create: [dds_waitset_t, [],
  ],

  dds_waitset_delete: [ref.types.int, [
    dds_waitset_t,
  ]],

  dds_readcondition_create: [dds_condition_t, [
    dds_entity_t,
    ref.types.uint32,
  ]],

  dds_waitset_attach: [ref.types.int, [
    dds_waitset_t,
    dds_condition_t,
    dds_attach_t,
  ]],

  dds_waitset_detach: [ref.types.int, [
    dds_waitset_t,
    dds_condition_t,
  ]],

  dds_waitset_get_conditions: [ref.types.void, [
    dds_waitset_t,
    pdds_condition_seq,
  ]],

  dds_waitset_wait_until: [ref.types.int, [
    dds_waitset_t,
    PVoidArray,
    ref.types.size_t,
    dds_duration_t,
  ]],

  dds_waitset_wait: [ref.types.int, [
    dds_waitset_t,
    PVoidArray,
    ref.types.size_t,
    dds_duration_t,
  ]],

  dds_guardcondition_create: [dds_condition_t, [
  ]],

  dds_guard_trigger: [ref.types.void, [
    dds_condition_t,
  ]],

  dds_guard_reset: [ref.types.void, [
    dds_condition_t,
  ]],

  dds_condition_triggered: [ref.types.int8, [
    dds_condition_t,
  ]],

  dds_condition_delete: [ref.types.void, [
    dds_condition_t,
  ]],

  dds_querycondition_create_sql: [dds_condition_t, [
    dds_entity_t,
    ref.types.uint32,
    ref.types.CString,
    StringArray,
    ref.types.uint32,
  ]],

  dds_read_cond: [ref.types.int, [dds_entity_t,
    PVoidArray,
    ref.types.uint32,
    SampleInfoArray,
    dds_condition_t,
  ]],

  dds_take_cond: [ref.types.int, [dds_entity_t,
    PVoidArray,
    ref.types.uint32,
    SampleInfoArray,
    dds_condition_t,
  ]],

  dds_reader_wait_for_historical_data: [ref.types.int, [
    dds_entity_t,
    dds_duration_t,
  ]],

  dds_statuscondition_get: [dds_condition_t, [
    dds_entity_t,
  ]],

  dds_status_set_enabled: [ref.types.int, [
    dds_entity_t,
    ref.types.uint32,
  ]],

  dds_status_take: [ref.types.int, [
    dds_entity_t,
    ref.refType(ref.types.uint32),
    ref.types.uint32,
  ]],

  dds_status_read: [ref.types.int, [
    dds_entity_t,
    ref.refType(ref.types.uint32),
    ref.types.uint32,
  ]],

  dds_status_get_enabled: [ref.types.uint32, [
    dds_entity_t,
  ]],

  dds_status_changes: [ref.types.uint32, [
    dds_entity_t,
  ]],

  dds_instance_register: [dds_instance_handle_t, [
    dds_entity_t,
    pvoid,
  ]],

  dds_instance_unregister: [ref.types.int, [
    dds_entity_t,
    pvoid,
    dds_instance_handle_t,
  ]],
};

const libdcpsc99 = ffi.Library(libs.libdds, c99_bindings);

// default callback
function errorHandler(err) {
  throw err; // throws an error and kills the program.
}
module.exports.errorHandler = errorHandler;

/**
 * Wrapper for dds_init.
 */
module.exports.init = function(argc, argv) {
  return libdcpsc99.dds_init(argc, argv);
};

/**
 * Wrapper for dds_fini.
 */
module.exports.fini = function() {
  libdcpsc99.dds_fini();
};


// dds wrapper methods
/**
 * Wrapper for dds_participant_create. Returns a handle to a domain
 * particicpant on success.
 *
 * @param {number} domainId An integer 0 <= domainId <= 230
 * @param {pdds_qos_t} qos
 * @param {pdds_participantlistener_t} listener
 * @param {function} callback
 * @returns {dds_entity_t|object} A handle the DDS Domain Participant on
 * success, null on failure if the callback does not throw an exception.
 */
module.exports.participantCreate = function(
  domainId = DDS_DOMAIN_DEFAULT,
  qos = null,
  listener = null,
  callback = errorHandler
) {
  // do all the ffi calls in one big try/catch block
  try {
    var pEntity = ref.alloc(dds_entity_t);
    var status = libdcpsc99.dds_participant_create(
      pEntity,
      domainId,
      qos,
      listener
    );

    if (status !== 0) {
      throw new ddserr.DDSError(status,
        'Failed to create domain participant: ');
    }
  } catch (e) { // catch the TypeError ffi might throw
    callback(e);
    return null;
  }

  return pEntity.deref();
};

/**
 * Wrapper for dds_topic_descriptor_create. Returns a topic descriptor on
 * success.
 *
 * @param {string} typeName
 * @param {string} key
 * @param {string} xml
 * @returns {pdds_topic_descriptor_t|object} The topic descriptor on success,
 * null on failure if the callback does not throw an exception.
 */
module.exports.topicDescriptorCreate = function(
  typeName,
  key,
  xml,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_topic_descriptor_create(typeName, key, xml);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_topic_descriptor_delete.
 *
 * @param {pdds_topic_descriptor_t} topicDesc The topic_descriptor to free
 * @param {function} callback
 */
module.exports.topicDescriptorDelete = function(
  topicDesc,
  callback = errorHandler
) {
  try {
    libdcpsc99.dds_topic_descriptor_delete(topicDesc);
  } catch (e) {
    callback(e);
  }
};

/**
 *  Wrapper for dds_topic_create. Returns a handle to the topic on success.
 *
 *  @param {dds_entity_t} domainParticipant
 *  @param {string} topicName
 *  @param {pdds_topic_descriptor_t} descriptor
 *  @param {pdds_qos_t} qos
 *  @param {pdds_topic_listener_t} listener
 *  @param {function} callback
 *  @returns {dds_entity_t|object} A handle to the sample topic on success,
 *  null on failure if the callback does not throw an exception.
 */
module.exports.topicCreate = function(
  domainParticipant,
  descriptor,
  topicName,
  qos = null,
  listener = null,
  callback = errorHandler
) {
  try {
    var suiteTopic = ref.alloc(dds_entity_t);
    var status = libdcpsc99.dds_topic_create(
      domainParticipant,
      suiteTopic,
      descriptor,
      topicName,
      qos,
      listener);

    if (status !== 0) {
      throw new ddserr.DDSError(status, 'Failed to create topic: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }
  return suiteTopic.deref();
};

/**
 * Wrapper for dds_writer_create. Returns a handle to a writer on success.
 *
 * @param {dds_entity_t} dpOrPub Our domain participant.
 * @param {dds_entity_t} topic
 * @param {pdds_qos_t} qos (default null)
 * @param {function} callback
 * @param {pdds_writerlistener_t} listener (default null)
 * @returns {dds_entity_t|object} A handle the writer on success, null on
 * failure if the callback does not throw an exception.
 */
module.exports.writerCreate = function(
  dpOrPub,
  topic,
  qos = null,
  listener = null,
  callback = errorHandler
) {

  try {
    var writer = ref.alloc(dds_entity_t);
    var status = libdcpsc99.dds_writer_create(
      dpOrPub,
      writer,
      topic,
      qos,
      listener
    );

    if (status !== 0) {
      throw new ddserr.DDSError(status,
        'Failed to create writer: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  return writer.deref();
};

/**
 * Wrapper for dds_subscriber_create. Returns a a handle to a subscriber on
 * success.
 *
 * @param {dds_entity_t} participant domain participant
 * @param {pdds_qos_t} qos
 * @param {pdds_subscriberlistener_t} listener
 * @param {function} callback
 * @returns {dds_entity_t|object} A handle the subscriber on success, null on
 * failure if the callback does not throw an exception.
 */
module.exports.subscriberCreate = function(
  participant,
  qos = null,
  listener = null,
  callback = errorHandler
) {

  try {
    var subscriber = ref.alloc(dds_entity_t);
    var status = libdcpsc99.dds_subscriber_create(
      participant,
      subscriber,
      qos,
      listener
    );

    if (status !== 0) {
      throw new ddserr.DDSError(status,
        'Failed to create subscriber: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  return subscriber.deref();
};

/**
 * Wrapper for dds_reader_create. Returns a handle to the reader on success.
 *
 * @param {dds_entity_t} sub
 * @param {dds_entity_t} topic
 * @param {pdds_qos_t} qos
 * @param {pdds_readerlistener_t} listener
 * @param {function} callback
 * @returns {dds_entity_t|object} A handle the reader on success, null on
 * failure if the callback does not throw an exception.
 */
module.exports.readerCreate = function(
  ppOrSub,
  topic,
  qos = null,
  listener = null,
  callback = errorHandler
) {

  try {
    var reader = ref.alloc(dds_entity_t);
    var status = libdcpsc99.dds_reader_create(
      ppOrSub,
      reader,
      topic,
      qos,
      listener
    );

    if (status !== 0) {
      throw new ddserr.DDSError(status, 'Failed to create reader: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  return reader.deref();
};

/**
 * Wrapper for dds_publisher_create. Returns a handle to a publisher on
 * success.
 *
 * @param {dds_entity_t} participant
 * @param {pdds_qos_t} qos
 * @param {pdds_publisherlistener_t} listener
 * @param {function} callback
 * @returns {dds_entity_t|object} A handle the publisher on success, null on
 * failure if the callback does not throw an exception.
 */
module.exports.publisherCreate = function(
  participant,
  qos = null,
  listener = null,
  callback = errorHandler
) {

  try {
    var publisher = ref.alloc(dds_entity_t);
    var status = libdcpsc99.dds_publisher_create(
      participant,
      publisher,
      qos,
      listener
    );

    if (status !== 0) {
      throw new ddserr.DDSError(status, 'Failed to create publisher: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  return publisher.deref();
};


/**
 * Wrapper for dds_write. Returns 0 on success.
 *
 * Example: If we are writing on the Msg topic type, then we would write as
 * dds.write(writer, {userID: 1, message:2 }).
 *
 * @param {dds_entity_t} writer
 * @param {object} buf
 * @param {function} callback
 * @returns {number} 0 on success, null on failure if the callback does not
 * throw an exception.
 */
module.exports.write = function(
  writer,
  buf,
  callback = errorHandler
) {
  try {
    var status = libdcpsc99.dds_write(writer, buf);

    if (status !== 0) {
      throw new ddserr.DDSError(status, 'Failed to write: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  return status;
};

function copyOutSamples(
  samples,
  infos,
  topicStruct,
  status
) {

  let arrayOfPairs = [];
  for (let i = 0; i < status; i++) {
    let data = topicStruct.copyout(
      ref.readPointer(
        samples.buffer,
        i * topicStruct.size,
        topicStruct.size
      )
    );
    let infoCopy = Object.assign({}, infos[i]);
    infoCopy.valid_data = (infos[i].valid_data === 1);
    const pair = {
      sample: data,
      info: infoCopy,
    };

    arrayOfPairs.push(pair);
  }
  return arrayOfPairs;

}

/**
 * Wrapper for dds_read.
 *
 * @param {dds_entity_t} reader
 * @param {number} maxSamples maximum number of samples to be read.
 * @param {object} topicStruct A ref-struct instance which indicates the topic
 * type.
 * @returns {object}  array of pairs (sample[i], sample_info[i])}.
 * @throws {Error} on failure.
 */
module.exports.read = function(
  reader,
  maxSamples,
  topicStruct,
  callback = errorHandler
) {
  try {
    var infos = new SampleInfoArray(maxSamples);
    var samples = new PVoidArray(maxSamples);
    var status = libdcpsc99.dds_read(
      reader,
      samples,
      maxSamples,
      infos,
      0
    ); // status is also the # of samples read

    if (status < 0) {
      throw new ddserr.DDSError(status, 'Failed to read: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  var arrayOfPairs = copyOutSamples(
    samples,
    infos,
    topicStruct,
    status
  );

  if (status > 0) {
    libdcpsc99.dds_return_loan(reader, samples, maxSamples);
  }
  return arrayOfPairs;
};

/**
 * Wrapper for dds_take.
 *
 * @param {dds_entity_t} reader
 * @param {number} maxSamples maximum number of samples to be read.
 * @param {object} topicStruct A ref-struct instance which indicates the topic
 * type.
 * @returns {object}  array of pairs (sample[i], sample_info[i])}.
 * @throws {Error} on failure.
 */
module.exports.take = function(
  reader,
  maxSamples,
  topicStruct,
  callback = errorHandler
) {
  try {
    var infos = new SampleInfoArray(maxSamples);
    var samples = new PVoidArray(maxSamples);
    var status = libdcpsc99.dds_take(
      reader,
      samples,
      maxSamples,
      infos,
      0
    ); // status is also the # of samples read

    if (status < 0) {
      throw new ddserr.DDSError(status, 'Failed to take: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  var arrayOfPairs = copyOutSamples(samples, infos, topicStruct, status);

  if (status > 0) {
    libdcpsc99.dds_return_loan(reader, samples, maxSamples);
  }

  return arrayOfPairs;
};

/**
 * Wrapper for dds_read_cond.
 *
 * @param {dds_entity_t} reader
 * @param {number} maxSamples maximum number of samples to be read.
 * @param {object} topicStruct A ref-struct instance which indicates the topic
 * type.
 * @param {dds_condition_t} condition to read with
 * @returns {object}  array of pairs (sample[i], sample_info[i])}.
 * @throws {Error} on failure.
 */
module.exports.readCond = function(
  reader,
  maxSamples,
  topicStruct,
  cond,
  callback = errorHandler
) {
  try {
    var infos = new SampleInfoArray(maxSamples);
    var samples = new PVoidArray(maxSamples);
    var status = libdcpsc99.dds_read_cond(
      reader,
      samples,
      maxSamples,
      infos,
      cond
    ); // status is also the # of samples read

    if (status < 0) {
      throw new ddserr.DDSError(status, 'Failed to read with condition: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  var arrayOfPairs = copyOutSamples(samples, infos, topicStruct, status);

  if (status > 0) {
    libdcpsc99.dds_return_loan(reader, samples, maxSamples);
  }

  return arrayOfPairs;
};

/**
 * Wrapper for dds_take_cond.
 *
 * @param {dds_entity_t} reader
 * @param {number} maxSamples maximum number of samples to be read.
 * @param {object} topicStruct A ref-struct instance which indicates the topic
 * type.
 * @param {dds_condition_t} condition to read with
 * @returns {object}  array of pairs (sample[i], sample_info[i])}.
 * @throws {Error} on failure.
 */
module.exports.takeCond = function(
  reader,
  maxSamples,
  topicStruct,
  condition,
  callback = errorHandler
) {

  try {
    var infos = new SampleInfoArray(maxSamples);
    var samples = new PVoidArray(maxSamples);
    var status = libdcpsc99.dds_take_cond(
      reader,
      samples,
      maxSamples,
      infos,
      condition
    ); // status is also the # of samples read

    if (status < 0) {
      throw new ddserr.DDSError(status, 'Failed to take with condition: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }

  var arrayOfPairs = copyOutSamples(samples, infos, topicStruct, status);

  if (status > 0) {
    libdcpsc99.dds_return_loan(reader, samples, maxSamples);
  }

  return arrayOfPairs;
};

/**
 * Wrapper for dds_entity_delete. Deletes the given entity and returns nothing
 * on success.
 *
 * @param {dds_entity_t} entity The domain participant handle returned by
 * connect.
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.entityDelete = function(
  entity,
  callback = errorHandler
) {
  try {
    libdcpsc99.dds_entity_delete(entity);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_free. Frees the memory at ptr and returns nothing on success.
 *
 * @param {pvoid} ptr A pointer to allocated memory
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.free = function(
  ptr,
  callback = errorHandler
) {
  try {
    libdcpsc99.dds_free(ptr);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_topic_find. Returns a handle to a named topic on success.
 *
 * @param {dds_entity_t} pp
 * @param {string} name
 * @returns {dds_entity_t} A topic handle on success.
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.topicFind = function(
  pp,
  name,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_topic_find(pp, name);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_topic_get_name. Returns the topic name attached to a given
 * topic handle on success.
 *
 * @param {dds_entity_t} topic
 * @returns {string} The topic name on success.
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.topicGetName = function(
  topic,
  callback = errorHandler
) {
  try {
    var name = libdcpsc99.dds_topic_get_name(topic);
  } catch (e) {
    callback(e);
    return null;
  }
  if (name === null) {
    throw new Error('Failed to get topic name');
  }
  return name;
};

/**
 * Wrapper for dds_topic_get_metadescriptor. Returns the metadescriptor attached
 * to a given topic handle on success.
 *
 * @param {dds_entity_t} topic
 * @returns {string} The topic metadescriptor on success.
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.topicGetMetadescriptor = function(
  topic,
  callback = errorHandler
) {
  try {
    var meta = libdcpsc99.dds_topic_get_metadescriptor(topic);
  } catch (e) {
    callback(e);
    return null;
  }
  if (meta === null) {
    throw new Error('Failed to get topic metadescriptor');
  }
  return meta;
};

/**
 * Wrapper for dds_topic_get_type_name. Returns the type name attached to a
 * given topic handle on success.
 *
 * @param {dds_entity_t} topic
 * @returns {string} The topic type name on success. null on failure if the
 * callback does not throw an exception.
 */
module.exports.topicGetTypeName = function(
  topic,
  callback = errorHandler
) {
  try {
    var name = libdcpsc99.dds_topic_get_type_name(topic);
  } catch (e) {
    callback(e);
    return null;
  }
  if (name === null) {
    throw new Error('Failed to get topic type name');
  }
  return name;
};

/**
 * Wrapper for dds_topic_get_keylist. Returns the key list attached to a given
 * topic handle on success.
 *
 * @param {dds_entity_t} topic
 * @returns {string} The keylist associated with topic on success. null on
 * failure if the callback does not throw an exception.
 */
module.exports.topicGetKeylist = function(
  topic,
  callback = errorHandler
) {
  try {
    var keylist = libdcpsc99.dds_topic_get_keylist(topic);
  } catch (e) {
    callback(e);
    return null;
  }
  if (keylist === null) {
    throw new Error('Failed to get topic keylist');
  }
  return keylist;
};

/**
 * Wrapper for dds_waitset_create. Returns a pointer to a waitset.
 *
 * @returns {dds_waitset_t} Pointer to a waitset.
 */
module.exports.waitsetCreate = function() {
  return libdcpsc99.dds_waitset_create();
};

/**
 * Wrapper for dds_waitset_delete.
 *
 * @param {dds_waitset_t} pointer to a waitset
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.waitsetDelete = function(
  ws,
  callback = errorHandler
) {
  try {
    var status = libdcpsc99.dds_waitset_delete(ws);

    if (status !== 0) {
      throw new ddserr.DDSError(status, 'Failed to delete waitset: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }
};

/** Wrapper for dds_readcondition_create. Returns a read condition object.
 *
 * @param {dds_entity_t} reader
 * @param {number} mask
 * @returns {dds_condition_t} null on failure if the callback does not throw
 * an exception.
 */
module.exports.readConditionCreate = function(
  reader,
  mask,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_readcondition_create(reader, mask);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_condition_delete.
 *
 * @param {dds_condition_t} condition
 * @returns {object} null on failure if the callback does not throw an
 * exception.
 */
module.exports.conditionDelete = function(
  condition,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_condition_delete(condition);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_guardcondition_create. Returns a guard condition object.
 *
 * @returns {dds_condition_t}
 */
module.exports.guardConditionCreate = function() {
  return libdcpsc99.dds_guardcondition_create();
};

/**
 * Wrapper for dds_guard_trigger. Triggers the guard.
 *
 * @param {dds_condition_t} guard
 * @returns {object} null on failure, if the callback does not throw an
 * exception.
 */
module.exports.guardTrigger = function(
  guard,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_guard_trigger(guard);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_guard_reset. Resets the guard.
 *
 * @param {dds_condition_t} guard
 * @returns {object} null on failure, if the callback does not throw an
 * exception.
 */
module.exports.guardReset = function(
  guard,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_guard_reset(guard);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_condition_triggered.
 *
 * @param {dds_condition_t} condition
 * @returns {boolean}
 */
module.exports.conditionTriggered = function(
  condition,
  callback = errorHandler
) {
  try {
    return (libdcpsc99.dds_condition_triggered(condition) === 1);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_waitset_attach.
 *
 * @param {dds_waitset_t} ws
 * @param {dds_condition_t} condition
 * @param {dds_attach_t} attach
 * @returns {object} null on failure, if the callback does not throw an
 * exception.
 */
module.exports.waitsetAttach = function(
  ws,
  condition,
  attach,
  callback = errorHandler
) {
  try {
    var status = libdcpsc99.dds_waitset_attach(ws, condition, attach);
    if (status !== 0) {
      throw new ddserr
        .DDSError(status, 'Failed to attach condition to waitset: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_waitset_detach.
 *
 * @param {dds_waitset_t} ws
 * @param {dds_condition_t} condition
 * @returns {object} null on failure, if the callback does not throw an
 * exception.
 */
module.exports.waitsetDetach = function(
  ws,
  condition,
  callback = errorHandler
) {
  try {
    var status = libdcpsc99.dds_waitset_detach(ws, condition);

    if (status !== 0) {
      throw new ddserr
        .DDSError(status, 'Failed to detach condition from waitset');
    }
    return status;
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_waitset_wait. Returns 0 on timeout, else number of signaled
 * waitset conditions. Returns 0 on timeout. Otherwise, it returns the array
 * of triggered conditions.
 *
 * Deprecated -- do not use. Use waitsetWait instead.
 *
 * @param {dds_waitset_t} waitset
 * @param {AttachArray} attachedConds
 * @param {number} numConds
 * @param {number} abstimeout
 * @returns {array|number} array of triggered conditions when unblocked, or
 * 0 on timeout.
 */
module.exports.waitsetWaitBlocking = function(
  waitset,
  numConds,
  abstimeout,
  callback = errorHandler
) {
  try {
    var attachedConds = new PVoidArray(numConds);
    let ret = libdcpsc99.dds_waitset_wait(
      waitset,
      attachedConds,
      numConds,
      abstimeout
    );
    // return 0 if we timeout
    if (ret === 0) {
      return 0;
    }
    return attachedConds;
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_querycondition_create_sql.
 *
 * @param {dds_entity_t} reader
 * @param {number} mask
 * @param {string} expr - sql expression
 * @param {array} params - parameters in sql expression
 * @param {number} maxp - number of parameters
 * @returns {dds_condition_t} handle to the condition
 * @throws {Error} on failure
 */
module.exports.queryConditionCreateSql = function(
  reader,
  mask,
  expr,
  params,
  maxp,
  callback = errorHandler
) {
  try {
    let paramArray = new StringArray(maxp);
    for (let i = 0; i < maxp; i++) {
      paramArray[i] = params[i];
    }
    let cond = libdcpsc99.dds_querycondition_create_sql(
      reader,
      mask,
      expr,
      paramArray,
      maxp
    );
    if (cond.deref() !== null) {
      // console.log(stat.deref());
      // -1 translates into an error of 1 in ddserr
      throw new ddserr
        .DDSError(-1, 'Failed to create querycondition with SQL expression: ');
    }
    return cond;
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_reader_wait_for_historical_data.
 *
 * @param {dds_entity_t} reader
 * @param {number} maxWait
 * @returns {number} 0 on success, DDS_RETCODE_TIMEOUT on timeout. Throws error
 * on failure.
 *
 */
module.exports.readerWaitForHistoricalDataBlocking = function(
  reader,
  maxWait,
  callback = errorHandler
) {
  try {
    let status = libdcpsc99
      .dds_reader_wait_for_historical_data(reader, maxWait);

    if (status < 0) {
      throw new ddserr.DDSError(status,
        'Failed to wait for historical data (blocking call): ');
    }
    return status;
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_status_set_enable
 *
 * @param {dds_entity_t} e - entity to enable the status on
 * @param {number} status mask
 * @returns {number} 0 on success
 */
module.exports.enableStatus = function(
  e,
  stat,
  callback = errorHandler
) {
  try {
    let status = libdcpsc99.dds_status_set_enabled(e, stat);
    /* c99 api note: this status does not seem to be nonzero when stat does
    not correspond to the entity */
    /*
    if (status !== 0) {
      throw new ddserr.DDSError(status, 'dds_status_set_enabled failed: ');
    } */
    return status;
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_statuscondition_get
 *
 * @param {dds_entity_t} e
 * @return {dds_condition_t}
 */
module.exports.statusCondition = function(
  e,
  callback = errorHandler
) {
  return libdcpsc99.dds_statuscondition_get(e);
};

/**
 * Wrapper for dds_instance_register
 *
 * @param {dds_entity_t} wr
 * @param {object} data - serialized data
 * @returns {dds_instance_handle_t}
 */
module.exports.instanceRegister = function(
  wr,
  data,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_instance_register(wr, data);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_instance_unregister
 * @param {dds_entity_t} wr
 * @param {object} data
 * @param {dds_instance_handle_t} handle
 */
module.exports.instanceUnregister = function(
  wr,
  data,
  handle,
  callback = errorHandler) {
  try {
    let status = libdcpsc99.dds_instance_unregister(wr, data, handle);
    if (status !== 0) {
      throw new ddserr.DDSError(status, 'Failed to unregister instance: ');
    }
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Asyncronous wrapper for dds_waitset_wait.
 * @param {dds_waitset_t} waitset
 * @param {number} numConds
 * @param {number} abstimeout
 * @param {function} cb
 */
module.exports.waitsetWait = function(
  waitset,
  numConds,
  abstimeout,
  cb
) {
  let attachedConds = new PVoidArray(numConds);
  libdcpsc99.dds_waitset_wait.async(waitset, attachedConds, numConds,
    abstimeout, cb);
};

/**
 * Wrapper for dds_status_read.
 * @param {dds_entity_t} entity
 * @param {number} mask
 */
module.exports.statusRead = function(
  entity,
  mask,
  callback = errorHandler
) {
  try {
    let statPointer = ref.alloc(ref.types.uint32);
    libdcpsc99.dds_status_read(entity, statPointer, mask);
    /* c99 api note: the above call does not seem to return a nonzero number
    when mask does not correspond to the entity */
    /*
    if (status !== 0) {
      throw new ddserr.DDSError(status, 'dds_status_read failed ');
    } */
    return statPointer.deref();
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_status_take.
 * @param {dds_entity_t} entity
 * @param {number} mask
 */
module.exports.statusTake = function(
  entity,
  mask,
  callback = errorHandler
) {
  try {
    let statPointer = ref.alloc(ref.types.uint32);
    libdcpsc99.dds_status_take(entity, statPointer, mask);
    /* c99 api note: the above call does not seem to return a nonzero number
    when mask does not correspond to the entity */
    /*
    if (status !== 0) {
      throw new ddserr.DDSError(status, 'dds_status_read failed ');
    } */
    return statPointer.deref();
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Asynchronous wrapper for dds_wait_for_historical_data
 * @param {dds_entity} reader
 * @param {dds_duration_t} timeout nanoseconds to wait until timeout
 * @param {function} cb callback to call when dds_wait_for_historical_data
 * completes
 */
module.exports.readerWaitForHistoricalData = function(
  reader,
  timeout,
  cb
) {
  libdcpsc99.dds_reader_wait_for_historical_data.async(reader, timeout,
    function(err, res) {
      if (res < 0) {
        err = new ddserr.DDSError(res,
          'async dds_wait_for_historical_data failed: ');
      }
      cb(err, res);
    });
};

/**
 * Wrapper for dds_status_get_enabled.
 */
module.exports.statusGetEnabled = function(
  entity,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_status_get_enabled(entity);
  } catch (e) {
    callback(e);
    return null;
  }
};

/**
 * Wrapper for dds_status_changes.
 */
module.exports.statusChanges = function(
  entity,
  callback = errorHandler
) {
  try {
    return libdcpsc99.dds_status_changes(entity);
  } catch (e) {
    callback(e);
    return null;
  }
};
