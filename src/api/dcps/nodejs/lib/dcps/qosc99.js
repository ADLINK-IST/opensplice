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

/**
 * QoS for Node.js
 * @module qosc99
 */

const ffi = require('ffi');
const ref = require('ref');
const ddserr = require('./ddserr');
const libs = require('./libs');
const ArrayType = require('ref-array');

//  Special types used in C functions
const pvoid = ref.refType(ref.types.void);
const ppvoid = ref.refType(pvoid);
const puint32 = ref.refType(ref.types.uint32);
const pint32 = ref.refType(ref.types.int32);
const pint64 = ref.refType(ref.types.int64);
const pbool = ref.refType(ref.types.bool);
const psize_t = ref.refType(ref.types.int32);
const pCString = ref.refType(ref.types.CString);
const ppCString = ref.refType(pCString);
const stringArr = ArrayType(ref.types.CString);

// Selected types from qos.h
const dds_qos_t = ref.types.void;
const pdds_qos_t = ref.refType(dds_qos_t);

// Selected types from dds.h
const dds_entity_t = pvoid;
const pdds_entity_t = ref.refType(dds_entity_t);

// QoS bindings for libdcpsc99.so
const qos_bindings = {
  dds_qos_create: [pdds_qos_t, [
  ]],

  dds_qos_delete: ['void', [
    pdds_qos_t,
  ]],

  dds_qosprovider_create: ['int', [
    pdds_entity_t,
    ref.types.CString,
    ref.types.CString,
  ]],

  dds_qosprovider_get_participant_qos: ['int', [
    dds_entity_t,
    pdds_qos_t,
    ref.types.CString,
  ]],

  dds_qosprovider_get_topic_qos: ['int', [
    dds_entity_t,
    pdds_qos_t,
    ref.types.CString,
  ]],

  dds_qosprovider_get_subscriber_qos: ['int', [
    dds_entity_t,
    pdds_qos_t,
    ref.types.CString,
  ]],

  dds_qosprovider_get_reader_qos: ['int', [
    dds_entity_t,
    pdds_qos_t,
    ref.types.CString,
  ]],

  dds_qosprovider_get_writer_qos: ['int', [
    dds_entity_t,
    pdds_qos_t,
    ref.types.CString,
  ]],

  dds_qosprovider_get_publisher_qos: ['int', [
    dds_entity_t,
    pdds_qos_t,
    ref.types.CString,
  ]],

  dds_qos_get: ['void', [
    dds_entity_t,
    pdds_qos_t,
  ]],

  dds_get_default_participant_qos: ['void', [
    pdds_qos_t,
  ]],

  dds_get_default_topic_qos: ['void', [
    pdds_qos_t,
  ]],

  dds_get_default_publisher_qos: ['void', [
    pdds_qos_t,
  ]],

  dds_get_default_subscriber_qos: ['void', [
    pdds_qos_t,
  ]],

  dds_get_default_writer_qos: ['void', [
    pdds_qos_t,
  ]],

  dds_get_default_reader_qos: ['void', [
    pdds_qos_t,
  ]],

  dds_qget_userdata: ['void', [
    pdds_qos_t,
    ppvoid,
    psize_t,
  ]],

  dds_qget_topicdata: ['void', [
    pdds_qos_t,
    ppvoid,
    psize_t,
  ]],

  dds_qget_groupdata: ['void', [
    pdds_qos_t,
    ppvoid,
    psize_t,
  ]],

  dds_qget_durability: ['void', [
    pdds_qos_t,
    pint32,
  ]],

  dds_qget_history: ['void', [
    pdds_qos_t,
    pint32,
    pint32,
  ]],

  dds_qget_resource_limits: ['void', [
    pdds_qos_t,
    pint32,
    pint32,
    pint32,
  ]],

  dds_qget_presentation: ['void', [
    pdds_qos_t,
    pint32,
    pbool,
    pbool,
  ]],

  dds_qget_lifespan: ['void', [
    pdds_qos_t,
    pint64,
  ]],

  dds_qget_deadline: ['void', [
    pdds_qos_t,
    pint64,
  ]],

  dds_qget_latency_budget: ['void', [
    pdds_qos_t,
    pint64,
  ]],

  dds_qget_ownership: ['void', [
    pdds_qos_t,
    pint32,
  ]],

  dds_qget_ownership_strength: ['void', [
    pdds_qos_t,
    pint32,
  ]],

  dds_qget_liveliness: ['void', [
    pdds_qos_t,
    pint32,
    pint64,
  ]],

  dds_qget_time_based_filter: ['void', [
    pdds_qos_t,
    pint64,
  ]],

  dds_qget_partition: ['void', [
    pdds_qos_t,
    puint32,
    ppCString,
  ]],

  dds_qget_reliability: ['void', [
    pdds_qos_t,
    pint32,
    pint64,
  ]],

  dds_qget_transport_priority: ['void', [
    pdds_qos_t,
    pint32,
  ]],

  dds_qget_destination_order: ['void', [
    pdds_qos_t,
    pint32,
  ]],

  dds_qget_writer_data_lifecycle: ['void', [
    pdds_qos_t,
    pbool,
  ]],

  dds_qget_reader_data_lifecycle: ['void', [
    pdds_qos_t,
    pint64,
    pint64,
  ]],

  dds_qget_durability_service: ['void', [
    pdds_qos_t,
    pint64,
    pint32,
    pint32,
    pint32,
    pint32,
    pint32,
  ]],

  dds_qset_userdata: ['void', [
    pdds_qos_t,
    pvoid,
    ref.types.size_t,
  ]],

  dds_qset_topicdata: ['void', [
    pdds_qos_t,
    pvoid,
    ref.types.size_t,
  ]],

  dds_qset_groupdata: ['void', [
    pdds_qos_t,
    pvoid,
    ref.types.size_t,
  ]],

  dds_qset_durability: ['void', [
    pdds_qos_t,
    ref.types.int32,
  ]],

  dds_qset_history: ['void', [
    pdds_qos_t,
    ref.types.int32,
    ref.types.int32,
  ]],

  dds_qset_resource_limits: ['void', [
    pdds_qos_t,
    ref.types.int32,
    ref.types.int32,
    ref.types.int32,
  ]],

  dds_qset_presentation: ['void', [
    pdds_qos_t,
    ref.types.int32,
    ref.types.bool,
    ref.types.bool,
  ]],

  dds_qset_lifespan: ['void', [
    pdds_qos_t,
    ref.types.int64,
  ]],

  dds_qset_deadline: ['void', [
    pdds_qos_t,
    ref.types.int64,
  ]],

  dds_qset_latency_budget: ['void', [
    pdds_qos_t,
    ref.types.int64,
  ]],

  dds_qset_ownership: ['void', [
    pdds_qos_t,
    ref.types.int32,
  ]],

  dds_qset_ownership_strength: ['void', [
    pdds_qos_t,
    ref.types.int32,
  ]],

  dds_qset_liveliness: ['void', [
    pdds_qos_t,
    ref.types.int32,
    ref.types.int64,
  ]],

  dds_qset_time_based_filter: ['void', [
    pdds_qos_t,
    ref.types.int64,
  ]],

  dds_qset_partition: ['void', [
    pdds_qos_t,
    ref.types.uint32,
    stringArr,
  ]],

  dds_qset_reliability: ['void', [
    pdds_qos_t,
    ref.types.int32,
    ref.types.int64,
  ]],

  dds_qset_transport_priority: ['void', [
    pdds_qos_t,
    ref.types.int32,
  ]],

  dds_qset_destination_order: ['void', [
    pdds_qos_t,
    ref.types.int32,
  ]],

  dds_qset_writer_data_lifecycle: ['void', [
    pdds_qos_t,
    ref.types.bool,
  ]],

  dds_qset_reader_data_lifecycle: ['void', [
    pdds_qos_t,
    ref.types.int64,
    ref.types.int64,
  ]],

  dds_qset_durability_service: ['void', [
    pdds_qos_t,
    ref.types.int64,
    ref.types.int32,
    ref.types.int32,
    ref.types.int32,
    ref.types.int32,
    ref.types.int32,
  ]],
};

const libqos = ffi.Library(libs.libdds, qos_bindings);

/**
 * Callback function to throw error
 *
 * @param {object} err
 */
function defaultErrorHandler(err) {
  throw err;
}

// qos wrapper functions
/**
 * Wrapper for dds_qos_create.
 * Allocate memory and initializes to default values for qos
 *
 * @returns {pdds_qos_t} qos
*/
module.exports.qosCreate = function() {
  let qos = libqos.dds_qos_create();
  return qos;
};

/**
 * Wrapper for dds_qos_delete.
 * Delete the memory allocated to qos.
 * The qos must be deleted once. Calling the qosDelete() to delete an already
 * deleted qos can corrupt the memory.
 *
 * @param {pdds_qos_t} qos
 */
module.exports.qosDelete = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qos_delete(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qosprovider_create.
 * Create a dds qos provider using the uri and profile
 *
 * @param {string} uri
 * @param {string} profile
 * @param {function} callback
 * @returns {dds_entity_t} qos provider
 */
module.exports.qosProviderCreate = function(
  uri,
  profile,
  callback = defaultErrorHandler
) {

  let qp = ref.alloc(dds_entity_t);
  let status = 0;

  try {
    status = libqos.dds_qosprovider_create(qp, uri, profile);
  } catch (e) {
    callback(e);
    return null;
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to create qosprovider: '));
  }

  return qp.deref();
};

/**
 * Wrapper for dds_qosprovider_get_participant_qos.
 * Given the QoSProvider, retrieve the value of the domain participant qos.
 *
 * @param {dds_entity_t} qp
 * @param {pdds_qos_t} qos
 * @param {string} id
 * @param {function} callback
 */
module.exports.qosProviderGetParticipantQos = function(
  qp,
  qos,
  id = null,
  callback = defaultErrorHandler
) {

  let status = 0;

  try {
    status = libqos.dds_qosprovider_get_participant_qos(qp, qos, id);
  } catch (e) {
    callback(e);
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to get participant qos using qosprovider: '));
  }
};

/**
 * Wrapper for dds_qosprovider_get_topic_qos.
 * Given the QoSProvider, retrieve the value of the topic qos.
 *
 * @param {dds_entity_t} qp
 * @param {pdds_qos_t} qos
 * @param {string} id
 * @param {function} callback
 */
module.exports.qosProviderGetTopicQos = function(
  qp,
  qos,
  id = null,
  callback = defaultErrorHandler
) {

  let status = 0;

  try {
    status = libqos.dds_qosprovider_get_topic_qos(qp, qos, id);
  } catch (e) {
    callback(e);
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to get topic qos using qosprovider: '));
  }
};

/**
 * Wrapper for dds_qosprovider_get_publisher_qos.
 * Given the QoSProvider, retrieve the value of the publisher qos.
 *
 * @param {dds_entity_t} qp
 * @param {pdds_qos_t} qos
 * @param {string} id
 * @param {function} callback
 */
module.exports.qosProviderGetPublisherQos = function(
  qp,
  qos,
  id = null,
  callback = defaultErrorHandler
) {

  let status = 0;

  try {
    status = libqos.dds_qosprovider_get_publisher_qos(qp, qos, id);
  } catch (e) {
    callback(e);
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to get publisher qos using qosprovider: '));
  }
};

/**
 * Wrapper for dds_qosprovider_get_subscriber_qos.
 * Given the QoSProvider, retrieve the value of the subscriber qos.
 *
 * @param {dds_entity_t} qp
 * @param {pdds_qos_t} qos
 * @param {string} id
 * @param {function} callback
 */
module.exports.qosProviderGetSubscriberQos = function(
  qp,
  qos,
  id = null,
  callback = defaultErrorHandler
) {

  let status = 0;

  try {
    status = libqos.dds_qosprovider_get_subscriber_qos(qp, qos, id);
  } catch (e) {
    callback(e);
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to get subscriber qos using qosprovider: '));
  }
};

/**
 * Wrapper for dds_qosprovider_get_reader_qos.
 * Given the QoSProvider, retrieve the value of the reader qos.
 *
 * @param {dds_entity_t} qp
 * @param {pdds_qos_t} qos
 * @param {string} id
 * @param {function} callback
 */
module.exports.qosProviderGetReaderQos = function(
  qp,
  qos,
  id = null,
  callback = defaultErrorHandler
) {

  let status = 0;

  try {
    status = libqos.dds_qosprovider_get_reader_qos(qp, qos, id);
  } catch (e) {
    callback(e);
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to get reader qos using qosprovider: '));
  }
};

/**
 * Wrapper for dds_qosprovider_get_writer_qos.
 * Given the QoSProvider, retrieve the value of the writer qos.
 *
 * @param {dds_entity_t} qp
 * @param {pdds_qos_t} qos
 * @param {string} id
 * @param {function} callback
 */
module.exports.qosProviderGetWriterQos = function(
  qp,
  qos,
  id = null,
  callback = defaultErrorHandler
) {

  let status = 0;

  try {
    status = libqos.dds_qosprovider_get_writer_qos(qp, qos, id);
  } catch (e) {
    callback(e);
  }

  if (status !== 0) {
    callback(new ddserr.DDSError(status,
      'Failed to get writer qos using qosprovider: '));
  }
};

/**
 * Wrapper for dds_qos_get. Get the existing set of QoS policies
 * for the entity
 *
 * @param {dds_entity_t} entity
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.qosGet = function(
  entity,
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qos_get(entity, qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_get_default_participant_qos.
 * Retrieves the default value of the domain participant qos.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.defaultParticipantQos = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_get_default_participant_qos(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_get_default_topic_qos.
 * Retrieves the default value of the topic qos.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.defaultTopicQos = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_get_default_topic_qos(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_get_default_publisher_qos.
 * Retrieves the default value of the publisher qos.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.defaultPublisherQos = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_get_default_publisher_qos(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_get_default_subscriber_qos.
 * Retrieves the default value of the subscriber qos.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.defaultSubscriberQos = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_get_default_subscriber_qos(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_get_default_writer_qos.
 * Retrieves the default value of the writer qos.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.defaultWriterQos = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_get_default_writer_qos(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_get_default_reader_qos.
 * Retrieves the default value of the reader qos.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 */
module.exports.defaultReaderQos = function(
  qos,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_get_default_reader_qos(qos);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qget_userdata.
 * Get the userdata policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {buffer} a buffer
 */
module.exports.getUserdata = function(
  qos,
  callback = defaultErrorHandler
) {
  let value = ref.alloc(pvoid);
  let size = ref.alloc(ref.types.size_t);
  try {
    libqos.dds_qget_userdata(qos, value, size);
  } catch (e) {
    callback(e);
    return null;
  }

  let pointer = ref.alloc('pointer', value.deref());
  let dataBuffer = ref.readPointer(pointer, 0, size.deref());

  return dataBuffer;
};

/**
 * Wrapper for dds_qget_topicdata.
 * Get the topicdata policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {buffer} a buffer
 */
module.exports.getTopicdata = function(
  qos,
  callback = defaultErrorHandler
) {
  let value = ref.alloc(pvoid);
  let size = ref.alloc(ref.types.size_t);
  try {
    libqos.dds_qget_topicdata(qos, value, size);
  } catch (e) {
    callback(e);
    return null;
  }

  let pointer = ref.alloc('pointer', value.deref());
  let dataBuffer = ref.readPointer(pointer, 0, size.deref());

  return dataBuffer;
};

/**
 * Wrapper for dds_qget_groupdata.
 * Get the groupdata policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {buffer} a buffer
 */
module.exports.getGroupdata = function(
  qos,
  callback = defaultErrorHandler
) {
  let value = ref.alloc(pvoid);
  let size = ref.alloc(ref.types.size_t);
  try {
    libqos.dds_qget_groupdata(qos, value, size);
  } catch (e) {
    callback(e);
    return null;
  }

  let pointer = ref.alloc('pointer', value.deref());
  let dataBuffer = ref.readPointer(pointer, 0, size.deref());

  return dataBuffer;
};

/**
 * Wrapper for dds_qget_durability.
 * Get the durability qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} durability policy kind,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getDurability = function(
  qos,
  callback = defaultErrorHandler
) {
  let kind = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_durability(qos, kind);
  } catch (e) {
    callback(e);
    return null;
  }
  return kind.deref();
};

/**
 * Wrapper for dds_qget_history.
 * Get the history qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {kind, depth},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getHistory = function(
  qos,
  callback = defaultErrorHandler
) {
  let kind = ref.alloc(ref.types.int32);
  let depth = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_history(qos, kind, depth);
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    kind: kind.deref(),
    depth: depth.deref(),
  };
};

/**
 * Wrapper for dds_qget_resource_limits.
 * Get the resource limits qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {maxSamples, maxInstances,
 * maxSamplesPerInstance},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getResourceLimits = function(
  qos,
  callback = defaultErrorHandler
) {
  let maxSamples = ref.alloc(ref.types.int32);
  let maxInstances = ref.alloc(ref.types.int32);
  let maxSamplesPerInstance = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_resource_limits(
      qos,
      maxSamples,
      maxInstances,
      maxSamplesPerInstance
    );
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    maxSamples: maxSamples.deref(),
    maxInstances: maxInstances.deref(),
    maxSamplesPerInstance: maxSamplesPerInstance.deref(),
  };
};

/**
 * Wrapper for dds_qget_presentation.
 * Get the presentation qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {accessScope,
 * coherentAccess, orderedAccess},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getPresentation = function(
  qos,
  callback = defaultErrorHandler
) {
  let accessScope = ref.alloc(ref.types.int32);
  let coherentAccess = ref.alloc(ref.types.bool);
  let orderedAccess = ref.alloc(ref.types.bool);
  try {
    libqos.dds_qget_presentation(
      qos,
      accessScope,
      coherentAccess,
      orderedAccess
    );
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    accessScope: accessScope.deref(),
    coherentAccess: coherentAccess.deref(),
    orderedAccess: orderedAccess.deref(),
  };
};

/**
 * Wrapper for dds_qget_lifespan.
 * Get the lifespan qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} lifespan duration,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getLifespan = function(
  qos,
  callback = defaultErrorHandler
) {
  let lifespan = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_lifespan(qos, lifespan);
  } catch (e) {
    callback(e);
    return null;
  }

  return lifespan.deref();
};

/**
 * Wrapper for dds_qget_deadline.
 * Get the deadline qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} deadline,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getDeadline = function(
  qos,
  callback = defaultErrorHandler
) {
  let deadline = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_deadline(qos, deadline);
  } catch (e) {
    callback(e);
    return null;
  }

  return deadline.deref();
};

/**
 * Wrapper for dds_qget_latency_budget.
 * Get the latency budget qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} latency budget duration,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getLatencyBudget = function(
  qos,
  callback = defaultErrorHandler
) {
  let duration = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_latency_budget(qos, duration);
  } catch (e) {
    callback(e);
    return null;
  }

  return duration.deref();
};

/**
 * Wrapper for dds_qget_ownership.
 * Get the ownership qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} ownership policy kind,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getOwnership = function(
  qos,
  callback = defaultErrorHandler
) {
  let kind = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_ownership(qos, kind);
  } catch (e) {
    callback(e);
    return null;
  }

  return kind.deref();
};

/**
 * Wrapper for dds_qget_ownership_strength.
 * Get the ownership strength qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} value of the ownership strength,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getOwnershipStrength = function(
  qos,
  callback = defaultErrorHandler
) {
  let value = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_ownership_strength(qos, value);
  } catch (e) {
    callback(e);
    return null;
  }

  return value.deref();
};

/**
 * Wrapper for dds_qget_liveliness.
 * Get the liveliness qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {livelinessKind, leaseDuration},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getLiveliness = function(
  qos,
  callback = defaultErrorHandler
) {
  let kind = ref.alloc(ref.types.int32);
  let leaseDuration = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_liveliness(qos, kind, leaseDuration);
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    kind: kind.deref(),
    leaseDuration: leaseDuration.deref(),
  };
};

/**
 * Wrapper for dds_qget_time_based_filter.
 * Get the timebased filter qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} minimum separation,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getTimebasedFilter = function(
  qos,
  callback = defaultErrorHandler
) {
  let minSeparation = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_time_based_filter(qos, minSeparation);
  } catch (e) {
    callback(e);
    return null;
  }

  return minSeparation.deref();
};

/**
 * Wrapper for dds_qget_partition.
 * Get the logical partition qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {string[]} an array of partition names,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getPartition = function(
  qos,
  callback = defaultErrorHandler
) {

  let pn = ref.alloc(ref.types.uint32);
  let partitions = ref.alloc(pCString);
  try {
    libqos.dds_qget_partition(qos, pn, partitions);
  } catch (e) {
    callback(e);
    return null;
  }

  let pBuffer = null;
  let partitionArray = [];

  for (let i = 0; i < pn.deref(); i++) {
    pBuffer = ref.get(partitions.deref(), 8 * i, ref.getType(partitions));
    partitionArray.push(ref.readCString(pBuffer, 0));
  }

  return partitionArray;
};

/**
 * Wrapper for dds_qget_reliability.
 * Get the reliability qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {reliabilityKind, maxBlockingTime},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getReliability = function(
  qos,
  callback = defaultErrorHandler
) {
  let kind = ref.alloc(ref.types.int32);
  let maxBlockingTime = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_reliability(qos, kind, maxBlockingTime);
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    kind: kind.deref(),
    maxBlockingTime: maxBlockingTime.deref(),
  };
};

/**
 * Wrapper for dds_qget_transport_priority.
 * Get the transport priority qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} transport priority value,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getTransportPriority = function(
  qos,
  callback = defaultErrorHandler
) {
  let value = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_transport_priority(qos, value);
  } catch (e) {
    callback(e);
    return null;
  }

  return value.deref();
};

/**
 * Wrapper for dds_qget_destination_order.
 * Get the destination order qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {number} destination order policy kind,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getDestinationOrder = function(
  qos,
  callback = defaultErrorHandler
) {
  let kind = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_destination_order(qos, kind);
  } catch (e) {
    callback(e);
    return null;
  }

  return kind.deref();
};

/**
 * Wrapper for dds_qget_writer_data_lifecycle.
 * Get the writer data lifecycle qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {boolean} true or false,
 * null on failure if the callback does not throw an exception.
 */
module.exports.getWriterDataLifecycle = function(
  qos,
  callback = defaultErrorHandler
) {
  let autodisposeUnregisterInstances = ref.alloc(ref.types.bool);
  try {
    libqos.dds_qget_writer_data_lifecycle(qos, autodisposeUnregisterInstances);
  } catch (e) {
    callback(e);
    return null;
  }

  return autodisposeUnregisterInstances.deref();
};

/**
 * Wrapper for dds_qget_reader_data_lifecycle.
 * Get the reader data lifecycle qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {autopurgeNoWriterSamples,
 * autopurgeDisposedSamplesDelay},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getReaderDataLifecycle = function(
  qos,
  callback = defaultErrorHandler
) {
  let autopurgeNoWriterSamples = ref.alloc(ref.types.int64);
  let autopurgeDisposedSamplesDelay = ref.alloc(ref.types.int64);
  try {
    libqos.dds_qget_reader_data_lifecycle(
      qos,
      autopurgeNoWriterSamples,
      autopurgeDisposedSamplesDelay
    );
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    autopurgeNoWriterSamples: autopurgeNoWriterSamples.deref(),
    autopurgeDisposedSamplesDelay: autopurgeDisposedSamplesDelay.deref(),
  };
};

/**
 * Wrapper for dds_qget_durability_service.
 * Get the durability service qos policy.
 *
 * @param {pdds_qos_t} qos
 * @param {function} callback
 * @returns {object} a javascript object {serviceCleanupDelay, historyKind,
 * historyDepth, maxSamples, maxInstances, maxSamplesPerInstance},
 * null on failure if the callback does not throw an exception.
 */
module.exports.getDurabilityService = function(
  qos,
  callback = defaultErrorHandler
) {

  let serviceCleanupDelay = ref.alloc(ref.types.int64);
  let historyKind = ref.alloc(ref.types.int32);
  let historyDepth = ref.alloc(ref.types.int32);
  let maxSamples = ref.alloc(ref.types.int32);
  let maxInstances = ref.alloc(ref.types.int32);
  let maxSamplesPerInstance = ref.alloc(ref.types.int32);
  try {
    libqos.dds_qget_durability_service(
      qos,
      serviceCleanupDelay,
      historyKind,
      historyDepth,
      maxSamples,
      maxInstances,
      maxSamplesPerInstance
    );
  } catch (e) {
    callback(e);
    return null;
  }

  return {
    serviceCleanupDelay: serviceCleanupDelay.deref(),
    historyKind: historyKind.deref(),
    historyDepth: historyDepth.deref(),
    maxSamples: maxSamples.deref(),
    maxInstances: maxInstances.deref(),
    maxSamplesPerInstance: maxSamplesPerInstance.deref(),
  };
};

/**
 * Wrapper for dds_qset_userdata.
 * Set the userdata policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {buffer} data
 * @param {function} callback
 */
module.exports.setUserdata = function(
  qos,
  data,
  callback = defaultErrorHandler
) {
  try {
    if (data instanceof Buffer) {
      libqos.dds_qset_userdata(qos, data, data.length);
    } else {
      callback(new Error('Error in setUserdata: Buffer ' +
        'data expected'));
    }
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_topicdata.
 * Set the topicdata policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {buffer} data
 * @param {function} callback
 */
module.exports.setTopicdata = function(
  qos,
  data,
  callback = defaultErrorHandler
) {
  try {
    if (data instanceof Buffer) {
      libqos.dds_qset_topicdata(qos, data, data.length);
    } else {
      callback(new Error('Error in setTopicdata: Buffer ' +
        'data expected'));
    }
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_groupdata.
 * Set the groupdata policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {buffer} data
 * @param {function} callback
 */
module.exports.setGroupdata = function(
  qos,
  data,
  callback = defaultErrorHandler
) {
  try {
    if (data instanceof Buffer) {
      libqos.dds_qset_groupdata(qos, data, data.length);
    } else {
      callback(new Error('Error in setGroupdata: Buffer ' +
        'data expected'));
    }
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_durability.
 * Set the durability policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} kind
 * @param {function} callback
 */
module.exports.setDurability = function(
  qos,
  kind,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_durability(qos, kind);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_history.
 * Set the history policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} kind
 * @param {number} depth
 * @param {function} callback
 */
module.exports.setHistory = function(
  qos,
  kind,
  depth,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_history(qos, kind, depth);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_resource_limits.
 * Set the resource limits policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} maxSamples
 * @param {number} maxInstances
 * @param {number} maxSamplesPerInstance
 * @param {function} callback
 */
module.exports.setResourceLimits = function(
  qos,
  maxSamples,
  maxInstances,
  maxSamplesPerInstance,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_resource_limits(
      qos,
      maxSamples,
      maxInstances,
      maxSamplesPerInstance
    );
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_presentation.
 * Set the presentation policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} accessScope
 * @param {boolean} coherentAccess
 * @param {boolean} orderedAccess
 * @param {function} callback
 */
module.exports.setPresentation = function(
  qos,
  accessScope,
  coherentAccess,
  orderedAccess,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_presentation(
      qos,
      accessScope,
      coherentAccess,
      orderedAccess
    );
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_lifespan.
 * Set the lifespan policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} lifespan
 * @param {function} callback
 */
module.exports.setLifespan = function(
  qos,
  lifespan,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_lifespan(qos, lifespan);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_deadline.
 * Set the deadline policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} deadline
 * @param {function} callback
 */
module.exports.setDeadline = function(
  qos,
  deadline,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_deadline(qos, deadline);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_latency_budget.
 * Set the latency budget policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} duration
 * @param {function} callback
 */
module.exports.setLatencyBudget = function(
  qos,
  duration,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_latency_budget(qos, duration);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_ownership.
 * Set the ownership qos policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} kind
 * @param {function} callback
 */
module.exports.setOwnership = function(
  qos,
  kind,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_ownership(qos, kind);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_ownership_strength.
 * Set the ownership strength in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} value
 * @param {function} callback
 */
module.exports.setOwnershipStrength = function(
  qos,
  value,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_ownership_strength(qos, value);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_liveliness.
 * Set the liveliness policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} kind
 * @param {number} leaseDuration
 * @param {function} callback
 */
module.exports.setLiveliness = function(
  qos,
  kind,
  leaseDuration,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_liveliness(qos, kind, leaseDuration);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_time_based_filter.
 * set the time based filter policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} minSeparation
 * @param {function} callback
 */
module.exports.setTimebasedFilter = function(
  qos,
  minSeparation,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_time_based_filter(qos, minSeparation);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_partition.
 * set the partition name in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {string[]} partitionArray
 * @param {function} callback
 */
module.exports.setPartition = function(
  qos,
  partitionArray,
  callback = defaultErrorHandler
) {
  try {
    if (partitionArray instanceof Array) {
      libqos.dds_qset_partition(
        qos,
        partitionArray.length,
        partitionArray
      );
    } else {
      callback(new Error('Error in setParttion: non array input'));
    }
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_reliability.
 * Set the reliability policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} kind
 * @param {number} maxBlockingTime
 * @param {function} callback
 */
module.exports.setReliability = function(
  qos,
  kind,
  maxBlockingTime,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_reliability(qos, kind, maxBlockingTime);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_transport_priority.
 * Set the transport priority policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} value
 * @param {function} callback
 */
module.exports.setTransportPriority = function(
  qos,
  value,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_transport_priority(qos, value);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_destination_order.
 * Set the destination order policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} kind
 * @param {function} callback
 */
module.exports.setDestinationOrder = function(
  qos,
  kind,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_destination_order(qos, kind);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_writer_data_lifecycle.
 * Set the writer data lifecycle policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {boolean} autodisposeUnregisterInstances true : dispose the
 * instance automatically each time when it is unregistered,
 * false : automatic disposition will not happen upon unregistration
 * @param {function} callback
 */
module.exports.setWriterDataLifecycle = function(
  qos,
  autodisposeUnregisterInstances,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_writer_data_lifecycle(qos, autodisposeUnregisterInstances);
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_reader_data_lifecycle.
 * Set the reader data lifecycle policy in the qos structure.
 *
 * @param {pdds_qos_t} qos
 * @param {number} autopurgeNoWriterSamples
 * @param {number} autopurgeDisposedSamplesDelay
 * @param {function} callback
 */
module.exports.setReaderDataLifecycle = function(
  qos,
  autopurgeNoWriterSamples,
  autopurgeDisposedSamplesDelay,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_reader_data_lifecycle(
      qos,
      autopurgeNoWriterSamples,
      autopurgeDisposedSamplesDelay
    );
  } catch (e) {
    callback(e);
  }
};

/**
 * Wrapper for dds_qset_durability_service.
 * Set the durability service qos.
 *
 *
 * @param {pdds_qos_t} qos
 * @param {number} serviceCleanupDelay
 * @param {number} historyKind
 * @param {number} historyDepth
 * @param {number} maxSamples
 * @param {number} maxInstances
 * @param {number} maxSamplesPerInstance
 * @param {function} callback
 */
module.exports.setDurabilityService = function(
  qos,
  serviceCleanupDelay,
  historyKind,
  historyDepth,
  maxSamples,
  maxInstances,
  maxSamplesPerInstance,
  callback = defaultErrorHandler
) {
  try {
    libqos.dds_qset_durability_service(
      qos,
      serviceCleanupDelay,
      historyKind,
      historyDepth,
      maxSamples,
      maxInstances,
      maxSamplesPerInstance
    );
  } catch (e) {
    callback(e);
  }
};

