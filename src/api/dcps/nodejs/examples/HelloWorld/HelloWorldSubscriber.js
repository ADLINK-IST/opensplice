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

  let sqos = dds.QoS.subscriberDefault();
  sqos.partition = 'HelloWorld example';
  let sub = participant.createSubscriber(sqos, null);

  let rqos = dds.QoS.readerDefault();
  rqos.durability = dds.DurabilityKind.Transient;
  rqos.reliability = dds.ReliabilityKind.Reliable;
  let reader = sub.createReader(topic, rqos, null);

  console.log('=== [Subscriber] Ready ...');

  attemptTake(participant, reader, 0);
}

/* Attempt to take from the reader every 200 ms.
 * If a successful take occurs, we are done.
 * If a successful take does not occur after 100 attempts,
 * return.
 */
function attemptTake(participant, reader, numberAttempts){

  setTimeout(function() {
    let takeArray = reader.take(1);
    if (takeArray.length > 0 && takeArray[0].info.valid_data) {
      console.log('=== [Subscriber] message received :');
      console.log('    userID  : ' + takeArray[0].sample.userID);
      console.log('    Message : ' + takeArray[0].sample.message);
    } else {
      if (numberAttempts < 100){
        return attemptTake(participant, reader, numberAttempts + 1);
      }
    }
    participant.delete();
    return;
  }, 200);
}


