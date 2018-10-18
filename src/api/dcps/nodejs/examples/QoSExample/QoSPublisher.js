/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to 2018 ADLINK
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

/* QoSExample Publisher.
   Instructions:
    Option 1. i. Run nodejs QoSPublisher.js
              ii. Run nodejs QoSSubscriber.js
              Due to the Durability QoS on the readers and writers being set
              to volatile, we do not expect the subscriber to pick up any data.
    Option 2. i. Run nodejs QoSSubscriber.js
              ii. Run nodejs QoSPublisher.js
              As the readers were created before the messages were sent in this
              option, we expect them to pick up both samples of data.
*/
'use strict';

const dds = require('vortexdds');
const EnvironmentConditionsTopic = require('./EnvironmentConditionsTopic.js');

main();

async function main() {
  // participant with default qos
  let participant = new dds.Participant();

  // set the topic qos, and use this for the writer too.
  // topic qos is only used for matching
  let tqos = dds.QoS.topicDefault();
  tqos.reliability = dds.ReliabilityKind.reliable;
  tqos.durability = dds.DurabilityKind.Volatile;
  let buf = Buffer.from('buffer');
  tqos.topicdata = buf;
  // let dataBuf = tqos.topicdata;
  let topic = await EnvironmentConditionsTopic.create(participant, tqos);

  // create publishers for first and second floor
  let pqos = dds.QoS.publisherDefault();
  pqos.partition = 'floor1';
  let firstFloorPublisher = participant.createPublisher(pqos);
  pqos.partition = 'floor2';
  let secondFloorPublisher = participant.createPublisher(pqos);

  // create the writers for the first and second floor
  let wqos = dds.QoS.writerDefault();
  wqos.reliability = dds.ReliabilityKind.reliable;
  wqos.durability = dds.DurabilityKind.Volatile;
  let firstFloorWriter = firstFloorPublisher.createWriter(topic, wqos);
  let secondFloorWriter = secondFloorPublisher.createWriter(topic, wqos);

  // publish the first floor data
  console.log('Publishing first floor data.');
  let data = {temp: 25, hum: 10};
  firstFloorWriter.write(data);

  // publish the second floor data
  console.log('Publishing second floor data.');
  data = {temp: 26, hum: 11};
  secondFloorWriter.write(data);

  participant.delete();
};
