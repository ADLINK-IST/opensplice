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

/*
    Hello World example.
    Instructions: 1. Run nodejs HelloWorldSubscriber.js
                  2. Run nodejs HelloWorldPublisher.js
*/

'use strict';

const dds = require('vortexdds');
const HelloWorldTopic = require('./HelloWorldTopic');


main();

async function main(){

  let participant = new dds.Participant();

  let topic = await HelloWorldTopic.create(participant);

  let pqos = dds.QoS.publisherDefault();
  pqos.partition = 'HelloWorld example';
  let pub = participant.createPublisher(pqos, null);

  let wqos = dds.QoS.writerDefault();
  wqos.durability = dds.DurabilityKind.Transient;
  wqos.reliability = dds.ReliabilityKind.Reliable;
  let writer = pub.createWriter(topic, wqos, null);

  // send one message
  let msg = {userID: 1, message: 'Hello World'};

  console.log('=== HelloWorldPublisher');
  console.log('=== [Publisher] writing a message containing :');
  console.log('    userID  : ' + msg.userID);
  console.log('    Message : ' + msg.message);

  writer.write(msg);

  participant.delete();

};


