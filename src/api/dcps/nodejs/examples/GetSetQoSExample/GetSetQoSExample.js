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

/**
 * GetSetQoSExample
 *
 * This example demonstrates how to get and set qos policies for dds entities.
 * It can be done in two ways
 * 1. Using the QoSProvider API and XML file(s)
 * 2. Using QoS API
 *
 * Instructions: Run nodejs GetSetQoSExample.js
*/
'use strict';

const dds = require('vortexdds');
const HelloWorldTopic = require('./HelloWorldTopic.js');

const QOS_PATH = 'DDS_Get_Set_QoS.xml';
const QOS_PROFILE = 'DDS GetSetQosProfile';
const DOMAIN_ID = dds.DDS_DOMAIN_DEFAULT;

main();

async function main(){

  /*
    1. Set QoS using QoS XML file
   */

  // create a qos provider using qos xml file
  let qp = new dds.QoSProvider(QOS_PATH, QOS_PROFILE);

  // get participant qos from qos provider
  let pqos = qp.getParticipantQos();
  let participant = new dds.Participant(DOMAIN_ID, pqos);

  // get publisher qos from qos provider and create a publisher
  let pubqos = qp.getPublisherQos();
  let publisher = new dds.Publisher(participant, pubqos);

  // get publisher qos policies
  console.log('\n', '** Publisher QoS ** ');
  let pubScope = pubqos.presentation;
  console.log('Presentation Access Scope: ', pubScope.accessScope);
  console.log('Presentation Coherent Access: ', pubScope.coherentAccess);
  console.log('Presentation Ordered Access: ', pubScope.orderedAccess);
  console.log('Partition: ', pubqos.partition);

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

  // get subscriber qos policies
  console.log('\n', '** Subscriber QoS **');
  let subScope = subqos.presentation;
  console.log('Presenation Access Scope: ', subScope.accessScope);
  console.log('Presentation Coherent Access: ', subScope.coherentAccess);
  console.log('Presentation Ordered Access: ', subScope.orderedAccess);
  console.log('Partition: ', subqos.partition);

  /*
    2. Set QoS using QoS API

    To set the qos using the qos api, first get the default qos for the
    respective dds entity. Then modify the qos policies for that entity.
    Create a dds entity using its respective qos.

    Either all qos policies can be set or a subset of policies can be set
   */

  // get the default topic qos
  let tqos = dds.QoS.topicDefault();

  // modify the topic qos policies

  // All of the policy variables can be set
  tqos.durabilityService = {
    serviceCleanupDelay: 5000,
    historyKind: dds.HistoryKind.KeepLast,
    historyDepth: 10,
    maxSamples: 10,
    maxInstances: 5,
    maxSamplesPerInstance: -1,
  };

  // Or a subset of the policy variables can be set
  tqos.durabilityService = {
    historyKind: dds.HistoryKind.KeepLast,
  };

  tqos.reliability = dds.ReliabilityKind.reliable;
  tqos.durability = dds.DurabilityKind.Volatile;
  tqos.topicdata = 'Hello world topic';
  tqos.userdata = 'connected to hello world';
  tqos.groupdata = 'hello world group';
  tqos.history = {
    kind: dds.HistoryKind.KeepLast,
    depth: 10,
  };
  tqos.resourceLimits = {
    maxSamples: 10,
    maxInstances: 5,
  };
  tqos.lifespan = 5000;
  tqos.deadline = 3000;
  tqos.latencyBudget = 4000;
  tqos.ownership = dds.OwnershipKind.Exclusive;
  tqos.liveliness = {
    kind: dds.LivelinessKind.ManualByTopic,
    leaseDuration: 10,
  };
  tqos.reliability = {
    kind: dds.ReliabilityKind.Reliable,
    maxBlockingTime: 100,
  };
  tqos.transportPriority = 5;
  tqos.destinationOrder = dds.DestinationOrderKind.BySourceTimestamp;

  // create topic with qos
  let topic = await HelloWorldTopic.create(participant, tqos);

  // get topic qos policies
  console.log('\n', '** Topic QoS **');
  console.log('Reliability: ', tqos.reliability);
  console.log('Durability: ', tqos.durability);
  console.log('Topic data: ', tqos.topicdata);
  console.log('User data: ', tqos.userdata);
  console.log('Group data: ', tqos.groupdata);
  console.log('History: ', tqos.history);
  console.log('Resource Limits: ', tqos.resourceLimits);
  console.log('Lifespan: ', tqos.lifespan);
  console.log('Deadline: ', tqos.deadline);
  console.log('Latency Budget: ', tqos.latencyBudget);
  console.log('Ownership: ', tqos.ownership);
  console.log('Liveliness: ', tqos.liveliness);
  console.log('Transport Priority: ', tqos.transportPriority);
  console.log('Destination Order: ', tqos.destinationOrder);
  console.log('Durability Service: ', tqos.durabilityService);

  // get the default writer qos
  let wqos = dds.QoS.writerDefault();

  // modify the writer qos policies
  wqos.writerDataLifecycle = false;

  // create a writer with qos
  let writer = new dds.Writer(// eslint-disable-line no-unused-vars
    publisher,
    topic,
    wqos
  );

  // get writer qos policies
  console.log('\n', '** Writer QoS **');
  console.log('Writer data lifecyle: ', wqos.writerDataLifecycle);

  // get the default reader qos
  let rqos = dds.QoS.readerDefault();

  // modify the reader qos policies
  rqos.partition = 'partition1';
  rqos.timebasedFilter = 60000;
  rqos.readerDataLifecycle = {
    autopurgeNoWriterSamples: 100,
    autopurgeDisposedSamplesDelay: 500,
  };

  // create a reader with qos
  let reader = new dds.Reader(// eslint-disable-line no-unused-vars
    subscriber,
    topic,
    rqos
  );

  // get reader qos policies
  console.log('\n', '** Reader QoS **');
  console.log('Partition: ', rqos.partition);
  console.log('Time based filter: ', rqos.timebasedFilter);
  console.log('Reader data lifecycle: ', rqos.readerDataLifecycle);

  // delete the participant
  participant.delete();

  // NOTE: Actual reading and writing data  is not demonstrated in this example
}
