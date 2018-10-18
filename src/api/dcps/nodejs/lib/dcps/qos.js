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
 * @module qos
 */

const qc99 = require('./qosc99');
const path = require('path');

/**
 * Throws an error
 * @param {string} err - error message
 */
function errorHandler(err) {
  throw err;
}

/**
 * Class representing qos
*/
class QoS {
  /**
   * Creates a qos handle
   * @param {object} c99qos qos handle
   */
  constructor() {
    this._c99Qos = qc99.qosCreate();
  }

  /**
   * Deletes the qos
  */
  delete() {
    /** Delete the qos only if it hasn't been deleted */
    if (this._c99Qos !== null){
      qc99.qosDelete(this._c99Qos);
      this._c99Qos = null;
    }
  }

  /**
   * Get a qos handle
   * @returns {object} a qos handle
   */
  get cqos() {
    return this._c99Qos;
  }

  /**
   * Retrieves the default participant qos
   * @returns {QoS} default participant qos handle
   */
  static participantDefault(){
    let qos = new QoS();
    qc99.defaultParticipantQos(qos.cqos, errorHandler);
    return qos;
  }

  /**
   * Retrieves the default topic qos
   * @returns {QoS} default topic qos handle
   */
  static topicDefault(){
    let qos = new QoS();
    qc99.defaultTopicQos(qos.cqos, errorHandler);
    return qos;
  }

  /**
   * Retrieves the default subscriber qos
   * @returns {QoS} default subscriber qos handle
   */
  static subscriberDefault(){
    let qos = new QoS();
    qc99.defaultSubscriberQos(qos.cqos, errorHandler);
    return qos;
  }

  /**
   * Retrieves the default reader qos
   * @returns {QoS} default reader qos handle
   */
  static readerDefault(){
    let qos = new QoS();
    qc99.defaultReaderQos(qos.cqos, errorHandler);
    return qos;
  }

  /**
   * Retrieves the default publisher qos
   * @returns {QoS} default publisher qos handle
   */
  static publisherDefault(){
    let qos = new QoS();
    qc99.defaultPublisherQos(qos.cqos, errorHandler);
    return qos;
  }

  /**
   * Retrieves the default writer qos
   * @returns {QoS} default writer qos handle
   */
  static writerDefault(){
    let qos = new QoS();
    qc99.defaultWriterQos(qos.cqos, errorHandler);
    return qos;
  }

  /**
   * Get the userdata qos policy.
   * @returns {string} an ISO-8859-1 encoded string,
   * null on failure if the callback does not throw an exception.
   */
  get userdata() {
    let buf = qc99.getUserdata(this._c99Qos, errorHandler);
    if (buf === null){
      return buf;
    }
    return buf.toString('binary');
  }

  /**
   * Get the userdata qos policy.
   * @returns {Buffer} a Buffer which wraps the userdata qos policy string,
   * null on failure if the callback does not throw an exception.
   */
  get userdataRaw(){
    return qc99.getUserdata(this._c99Qos, errorHandler);
  }

  /**
   * Get the topicdata qos policy.
   * @returns {string} an ISO-8859-1 encoded string,
   * null on failure if the callback does not throw an exception.
   */
  get topicdata() {
    let buf = qc99.getTopicdata(this._c99Qos, errorHandler);
    if (buf === null){
      return buf;
    }
    return buf.toString('binary');
  }

  /**
   * Get the topicdata qos policy.
   * @returns {Buffer} A buffer which wraps the topic data qos policy string,
   * null on failure if the callback does not throw an exception.
   */
  get topicdataRaw(){
    return qc99.getTopicdata(this._c99Qos, errorHandler);
  }

  /**
   * Get the groupdata qos policy.
   * @returns {string} an ISO-8859-1 encoded string,
   * null on failure if the callback does not throw an exception.
   */
  get groupdata() {
    let buf = qc99.getGroupdata(this._c99Qos, errorHandler);
    if (buf === null){
      return buf;
    }
    return buf.toString('binary');
  }

  /**
   * Get the groupdata qos policy.
   * @returns {Buffer} a buffer which wraps the groupdata qos policy string,
   * null on failure if the callback does not throw an exception.
   */
  get groupdataRaw(){
    return qc99.getGroupdata(this._c99Qos, errorHandler);
  }

  /**
   * Get the durability qos policy.
   * @returns {number} durability policy kind,
   * null on failure if the callback does not throw an exception.
   * Refer to DurabilityKind for the durability policy kind values.
   */
  get durability() {
    return qc99.getDurability(this._c99Qos, errorHandler);
  }

  /**
   * Get the history qos policy.
   * @returns {object} a javascript object {kind, depth},
   * null on failure if the callback does not throw an exception.
   * Refer to HistoryKind for the history policy kind values.
   */
  get history() {
    return qc99.getHistory(this._c99Qos, errorHandler);
  }

  /**
   * Get the resource limits qos policy.
   * @returns {object} a javascript object {maxSamples,
   * maxInstances, maxSamplesPerInstance},
   * null on failure if the callback does not throw an exception.
   */
  get resourceLimits() {
    return qc99.getResourceLimits(this._c99Qos, errorHandler);
  }

  /**
   * Get the presentaion qos policy.
   * @returns {object} a javascript object {accessScope,
   * coherentAccess, orderedAccess},
   * null on failure if the callback does not throw an exception.
   * Refer to PresentationAccessScopeKind for the presentation
   * access scope kind values.
   */
  get presentation() {
    return qc99.getPresentation(this._c99Qos, errorHandler);
  }

  /**
   * Get the lifespan qos policy.
   * @returns {number} the lifespan duration,
   * null on failure if the callback does not throw an exception.
   */
  get lifespan() {
    return qc99.getLifespan(this._c99Qos, errorHandler);
  }

  /**
   * Get the deadline qos policy.
   * @returns {number} deadline,
   * null on failure if the callback does not throw an exception.
   */
  get deadline() {
    return qc99.getDeadline(this._c99Qos, errorHandler);
  }

  /**
   * Get the latency budget qos policy.
   * @returns {number} duration,
   * null on failure if the callback does not throw an exception.
   */
  get latencyBudget() {
    return qc99.getLatencyBudget(this._c99Qos, errorHandler);
  }

  /**
   * Get the ownership qos policy.
   * @returns {number} ownership policy kind,
   * null on failure if the callback does not throw an exception.
   * Refer to OwnershipKind for the ownership policy kind values.
   */
  get ownership() {
    return qc99.getOwnership(this._c99Qos, errorHandler);
  }

  /**
   * Get the ownership strength qos policy.
   * @returns {number} the ownership strength value,
   * null on failure if the callback does not throw an exception.
   */
  get ownershipStrength() {
    return qc99.getOwnershipStrength(this._c99Qos, errorHandler);
  }

  /**
   * Get the liveliness qos policy.
   * @returns {object} a javascript object {livelinessKind, leaseDuration},
   * null on failure if the callback does not throw an exception.
   * Refer to LivelinessKind for the liveliness policy kind values.
   */
  get liveliness() {
    return qc99.getLiveliness(this._c99Qos, errorHandler);
  }

  /**
   * Get the timebased filter qos policy.
   * @returns {number} minimum separation duration,
   * null on failure if the callback does not throw an exception.
   */
  get timebasedFilter() {
    return qc99.getTimebasedFilter(this._c99Qos, errorHandler);
  }

  /**
   * Get the partition qos policy.
   * @returns {array} an array of partition names (strings),
   * null on failure if the callback does not throw an exception.
   */
  get partition() {
    return qc99.getPartition(this._c99Qos, errorHandler);
  }

  /**
   * Get the reliability qos policy.
   * @returns {number} a javascript object {reliabilityKind, maxBlockingTime},
   * null on failure if the callback does not throw an exception.
   * Refer to ReliabilityKind for the reliability policy kind values.
   */
  get reliability() {
    return qc99.getReliability(this._c99Qos, errorHandler);
  }

  /**
   * Get the transport priority qos policy.
   * @returns {number} transport priority value,
   * null on failure if the callback does not throw an exception.
   */
  get transportPriority() {
    return qc99.getTransportPriority(this._c99Qos, errorHandler);
  }

  /**
   * Get the destination order qos policy.
   * @returns {number} destination order kind,
   * null on failure if the callback does not throw an exception.
   * Refer to DestinationOrderKind for the destination order
   * policy kind values.
   */
  get destinationOrder() {
    return qc99.getDestinationOrder(this._c99Qos, errorHandler);
  }

  /**
   * Get the writer data lifecycle qos policy.
   * @returns {boolean} true or false,
   * null on failure if the callback does not throw an exception.
   */
  get writerDataLifecycle() {
    return qc99.getWriterDataLifecycle(this._c99Qos, errorHandler);
  }

  /**
   * Get the reader data lifecycle qos policy.
   * @returns {object} a javascript object {autopurgeNoWriterSamples,
   * autopurgeDisposedSamplesDelay},
   * null on failure if the callback does not throw an exception.
   */
  get readerDataLifecycle() {
    return qc99.getReaderDataLifecycle(this._c99Qos, errorHandler);
  }

  /**
   * Get the durability service qos policy.
   * @returns {object} a javascript object {serviceCleanupDelay, historyKind,
   * historyDepth, maxSamples, maxInstances, maxSamplesPerInstance},
   * null on failure if the callback does not throw an exception.
   * Refer to HistoryKind for the history kind values.
   */
  get durabilityService() {
    return qc99.getDurabilityService(this._c99Qos, errorHandler);
  }

  /**
   * Set the userdata policy in the qos structure.
   *
   * @param {string|Buffer} data
   */
  set userdata(data) {
    let buf = data;
    if (typeof data === 'string'){
      buf = Buffer.from(data, 'binary');
    }

    qc99.setUserdata(this._c99Qos, buf, errorHandler);
  }

  /**
   * Set the topicdata policy in the qos structure.
   *
   * @param {string|Buffer} data
   */
  set topicdata(data) {
    let buf = data;
    if (typeof data === 'string'){
      buf = Buffer.from(data, 'binary');
    }

    qc99.setTopicdata(this._c99Qos, buf, errorHandler);
  }

  /**
   * Set the groupdata policy in the qos structure.
   *
   * @param {string|Buffer} data
   */
  set groupdata(data) {
    let buf = data;
    if (typeof data === 'string'){
      buf = Buffer.from(data, 'binary');
    }

    qc99.setGroupdata(this._c99Qos, buf, errorHandler);
  }

  /**
   * Set the durability policy in the qos structure.
   *
   * @param {number} kind durability policy kind value.
   * Refer to DurabilityKind for the durability policy kind values.
   */
  set durability(kind) {
    qc99.setDurability(this._c99Qos, kind, errorHandler);
  }

  /**
   * Set the history policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * kind and depth. E.g., {kind: 0, depth: 1}.
   * You only need to provide the fields that you want to change.
   * Refer to HistoryKind for the history policy kind values.
   */
  set history(jsobj) {
    let updatedPolicy = Object.assign(this.history, jsobj);
    qc99.setHistory(
      this._c99Qos,
      updatedPolicy.kind,
      updatedPolicy.depth,
      errorHandler
    );
  }

  /**
   * Set the resource limits policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * maxSamples, maxInstances and maxSamplesPerInstance values.
   * E.g., {maxSamples: 1, maxInstances: 1, maxSamplesPerInstance: 1}.
   * You only need to provide the fields that you want to change.
   */
  set resourceLimits(jsobj) {
    let updatedPolicy = Object.assign(this.resourceLimits, jsobj);
    qc99.setResourceLimits(
      this._c99Qos,
      updatedPolicy.maxSamples,
      updatedPolicy.maxInstances,
      updatedPolicy.maxSamplesPerInstance,
      errorHandler
    );
  }

  /**
   * Set the presentation policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * accessScope, coherentAccess and orderedAccess values.
   * E.g., {accessScope: 2, coherentAccess: true, orderedAccess: true}.
   * You only need to provide the fields that you want to change.
   * Refer to PresentationAccessScopeKind for the presentation
   * access scope kind values.
   */
  set presentation(jsobj) {
    let updatedPolicy = Object.assign(this.presentation, jsobj);
    qc99.setPresentation(
      this._c99Qos,
      updatedPolicy.accessScope,
      updatedPolicy.coherentAccess,
      updatedPolicy.orderedAccess,
      errorHandler
    );
  }

  /**
   * Set the lifespan policy in the qos structure.
   *
   * @param {number} lifespan Expiration time relative to source timestamp
   * beyond which the sample shall be removed from the caches.
   */
  set lifespan(lifespan) {
    qc99.setLifespan(this._c99Qos, lifespan, errorHandler);
  }

  /**
   * Set the deadline policy in the qos structure.
   *
   * @param {number} deadline
   */
  set deadline(deadline) {
    qc99.setDeadline(this._c99Qos, deadline, errorHandler);
  }

  /**
   * Set the latency budget policy in the qos structure.
   *
   * @param {number} duration
   */
  set latencyBudget(duration) {
    qc99.setLatencyBudget(this._c99Qos, duration, errorHandler);
  }

  /**
   * Set the ownership policy in the qos structure.
   *
   * @param {number} kind ownership policy kind value.
   * Refer to OwnershipKind for the ownership policy kind values.
   */
  set ownership(kind) {
    qc99.setOwnership(this._c99Qos, kind, errorHandler);
  }

  /**
   * Set the ownership strength in the qos structure.
   *
   * @param {number} value determines the ownership of a data
   * instance.
   */
  set ownershipStrength(value) {
    qc99.setOwnershipStrength(this._c99Qos, value, errorHandler);
  }

  /**
   * Set the liveliness policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * kind and leaseDuration. E.g., {kind: 1, leaseDuration: 100}.
   * You only need to provide the fields that you want to change.
   * Refer to LivelinessKind for the liveliness policy kind values.
   */
  set liveliness(jsobj) {
    let updatedPolicy = Object.assign(this.liveliness, jsobj);
    qc99.setLiveliness(
      this._c99Qos,
      updatedPolicy.kind,
      updatedPolicy.leaseDuration,
      errorHandler
    );
  }

  /**
   * Set the time based filter policy in the qos structure.
   *
   * @param {number} minimumSeparation the duration that determines the
   * rate at which the data reader want to see the sample per instance.
   */
  set timebasedFilter(minimumSeparation) {
    qc99.setTimebasedFilter(
      this._c99Qos,
      minimumSeparation,
      errorHandler
    );
  }

  /**
   * Set the logical partition name in the qos structure.
   *
   * @param {string|array} partitions a string of partiton name or a
   * string array of the partition names.
   * E.g., ['part1','part2'] or 'part'.
   */
  set partition(partitions) {
    let partitionArray = [];

    if (partitions instanceof Array) {
      partitionArray = partitions;
    } else if (typeof (partitions) === 'string') {
      partitionArray.push(partitions);
    }

    qc99.setPartition(this._c99Qos, partitionArray, errorHandler);
  }

  /**
   * Set the reliability policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * kind and maxBlockingTime. E.g., {kind: 1, maxBlockingTime: 100}.
   * You only need to provide the fields that you want to change.
   * Refer to ReliabilityKind for the reliability policy kind values.
   */
  set reliability(jsobj) {
    let updatedPolicy = Object.assign(this.reliability, jsobj);
    qc99.setReliability(
      this._c99Qos,
      updatedPolicy.kind,
      updatedPolicy.maxBlockingTime,
      errorHandler
    );
  }

  /**
   * Set the transport priority policy in the qos structure.
   *
   * @param {number} value priority value for transporting the
   * messages (higher the number, higher the priority)
   */
  set transportPriority(value) {
    qc99.setTransportPriority(this._c99Qos, value, errorHandler);
  }

  /**
   * Set the destination order policy in the qos structure.
   *
   * @param {number} kind
   * Refer to DestinationOrderKind for the destination order policy kind values.
   */
  set destinationOrder(kind) {
    qc99.setDestinationOrder(this._c99Qos, kind, errorHandler);
  }

  /**
   * Set the writer data lifecycle policy in the qos structure.
   *
   * @param {boolean} autodisposeUnregisterInstances true : dispose the
   * instance automatically each time when it is unregistered,
   * false : automatic disposition will not happen upon unregistration
   */
  set writerDataLifecycle(autodisposeUnregisterInstances) {
    qc99.setWriterDataLifecycle(
      this._c99Qos,
      autodisposeUnregisterInstances,
      errorHandler
    );
  }

  /**
   * Set the reader data lifecycle policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * autopurgeNoWriterSamples and autopurgeDisposedSamplesDelay values.
   * E.g., {autopurgeNoWriterSamples: 100, autopurgeDisposedSamplesDelay: 500}.
   * You only need to provide the fields that you want to change.
   */
  set readerDataLifecycle(jsobj) {
    var updatedPolicy = Object.assign(this.readerDataLifecycle, jsobj);
    qc99.setReaderDataLifecycle(
      this._c99Qos,
      updatedPolicy.autopurgeNoWriterSamples,
      updatedPolicy.autopurgeDisposedSamplesDelay,
      errorHandler
    );
  }

  /**
   * Set the durability service policy in the qos structure.
   *
   * @param {object} jsobj a javascript object containing
   * serviceCleanupDelay, historyKind, historyDepth, maxSamples,
   * maxInstances and maxSamplesPerInstance values.
   * E.g., {serviceCleanupDelay: 100, historyKind: 0, historyDepth: 1,
   * maxSamples: 1, maxInstances: 1, maxSamplesPerInstance: 1}.
   * You only need to provide the fields that you want to change.
   * Refer to HistoryKind for the history kind values.
   */
  set durabilityService(jsobj) {
    let updatedPolicy = Object.assign(this.durabilityService, jsobj);
    qc99.setDurabilityService(
      this._c99Qos,
      updatedPolicy.serviceCleanupDelay,
      updatedPolicy.historyKind,
      updatedPolicy.historyDepth,
      updatedPolicy.maxSamples,
      updatedPolicy.maxInstances,
      updatedPolicy.maxSamplesPerInstance,
      errorHandler
    );
  }

}

/**
 * Class representing dds qos providers
 */
class QoSProvider{
  /** Create a qos provider
     * @param {string} qospath path to the qos file
     * @param {string} profile qos profile
     */
  constructor(qosPath, profile) {
    let uri = null;
    /**
     * check if qospath is a file uri else build uri
     */
    try {
      if (qosPath.startsWith('file://')){
        uri = qosPath;
      } else {
        /**
         * check if qos path is an absolute path else
         * resolve it into an absolute path
         */
        if (path.isAbsolute(qosPath)){
          uri = 'file://' + qosPath;
        } else {
          uri = 'file://' + path.resolve(qosPath);
        }
      }
    } catch (e){
      errorHandler(e);
    }
    this._qp = qc99.qosProviderCreate(uri, profile, errorHandler);
  }

  /**
   * Retrieves the domain participant qos
   * @param {string} id qos id
   * @returns {QoS} QoS object that contains
   * values of the policies for participant
   */
  getParticipantQos(id = null){
    let qos = new QoS();
    qc99.qosProviderGetParticipantQos(this._qp, qos.cqos, id, errorHandler);
    return qos;
  }

  /**
   * Retrieves the topic qos
   * @param {string} id qos id
   * @returns {QoS} QoS object that contains
   * values of the policies for topic
   */
  getTopicQos(id = null){
    let qos = new QoS();
    qc99.qosProviderGetTopicQos(this._qp, qos.cqos, id, errorHandler);
    return qos;
  }

  /**
   * Retrieves the publisher qos
   * @param {string} id qos id
   * @returns {QoS} QoS object that contains
   * values of the policies for publisher
   */
  getPublisherQos(id = null){
    let qos = new QoS();
    qc99.qosProviderGetPublisherQos(this._qp, qos.cqos, id, errorHandler);
    return qos;
  }

  /**
   * Retrieves the subscriber qos
   * @param {string} id qos id
   * @returns {QoS} QoS object that contains
   * values of the policies for subscriber
   */
  getSubscriberQos(id = null){
    let qos = new QoS();
    qc99.qosProviderGetSubscriberQos(this._qp, qos.cqos, id, errorHandler);
    return qos;
  }

  /**
   * Retrieves the writer qos
   * @param {string} id qos id
   * @returns {QoS} QoS object that contains
   * values of the policies for writer
   */
  getWriterQos(id = null){
    let qos = new QoS();
    qc99.qosProviderGetWriterQos(this._qp, qos.cqos, id, errorHandler);
    return qos;
  }

  /**
   * Retrieves the reader qos
   * @param {string} id qos id
   * @returns {QoS} QoS object that contains
   * values of the policies for reader
   */
  getReaderQos(id = null){
    let qos = new QoS();
    qc99.qosProviderGetReaderQos(this._qp, qos.cqos, id, errorHandler);
    return qos;
  }
}

/**
   * Durability QoS - Specifies the type of durability
   * Applies to Topic, DataReader, DataWriter
   * @enum {number}
  */
exports.DurabilityKind = Object.freeze({
  Volatile: 0,
  TransientLocal: 1,
  Transient: 2,
  Persistent: 3,
});

/**
   * History QoS - Specifies the number of samples to be stored
   * Applies to Topic, DataReader, DataWriter
   * @enum {number}
  */
exports.HistoryKind = Object.freeze({
  KeepLast: 0,
  KeepAll: 1,
});

/**
   * Ownership QoS - Specifies the type of ownership
   * Applies to Topic, DataReader, DataWriter
   * @enum {number}
  */
exports.OwnershipKind = Object.freeze({
  Shared: 0,
  Exclusive: 1,
});

/**
   * Liveliness QoS - Specifies the type of liveliness
   * Applies to Topic, DataReader, DataWriter
   * @enum {number}
   */
exports.LivelinessKind = Object.freeze({
  Automatic: 0,
  ManualByParticipant: 1,
  ManualByTopic: 2,
});

/**
   * Reliability QoS - Specifies the type of reliability
   * Applies to Topic, DataReader, DataWriter
   * @enum {number}
  */
exports.ReliabilityKind = Object.freeze({
  BestEffort: 0,
  Reliable: 1,
});

/**
 * Destination Order Kind - Specifies the type of destination order
 * Applies to Topic, DataReader, DataWriter
 * @enum {number}
*/
exports.DestinationOrderKind = Object.freeze({
  ByReceptionTimestamp: 0,
  BySourceTimestamp: 1,
});

/**
 * Presentation QoS - Specifies the type of presentation
 * Applies to Publisher, Subscriber
 * @enum {number}
*/
exports.PresentationAccessScopeKind = Object.freeze({
  Instance: 0,
  Topic: 1,
  Group: 2,
});

module.exports.QoSProvider = QoSProvider;
module.exports.QoS = QoS;
