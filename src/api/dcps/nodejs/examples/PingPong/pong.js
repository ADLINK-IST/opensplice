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
    Ping Pong pong.
    Instructions: run nodejs ping.js
                  run nodejs pong.js
                  You may run these in any order.
*/

'use strict';

const dds = require('vortexdds');
const PingPongTopic = require('./PingPongTopic');

// we iterate for 20 samples
const numSamples = 20;
main();

async function main() {
  // create our entities
  let participant = new dds.Participant();
  let topic = await PingPongTopic.create(participant);

  // create a subscriber on partition Ping
  // on ping, we have a publisher with the same partition
  let sqos = dds.QoS.subscriberDefault();
  sqos.partition = 'Ping';
  let subscriber = participant.createSubscriber(sqos, null);
  let reader = subscriber.createReader(topic);

  // create a publisher on partition Pong
  // on ping, we have a subscriber on the same partition
  let pqos = dds.QoS.publisherDefault();
  pqos.partition = 'Pong';
  let publisher = participant.createPublisher(pqos);
  let writer = publisher.createWriter(topic);

  // create waitset which waits until ping writer from ping.js is found
  let subMatchedWaitset = new dds.Waitset(new dds.StatusCondition(reader,
    dds.StatusMask.subscription_matched));

  // waitset for new data
  let newDataWaitset = new dds.Waitset(new dds.ReadCondition(reader,
    dds.StateMask.sample.not_read));

  // block until ping has been found
  console.log('Waiting for ping publisher to be found...');
  subMatchedWaitset.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
    if (err) throw err;
    console.log('Found ping publisher.');
    onDataAvailable(participant, newDataWaitset, reader, writer, 0);
  });

}

/* onDataAvailable waits for new data. When we have new data, it
 * reads it, prints it, and sends back a message with the same userID field
 * and 'from pong' in the message field. It then calls onDataAvailable again.
 * We quit when iterLower === numSamples (20 iterations in this example).
 */
function onDataAvailable(
  participant,
  ws,
  reader,
  writer,
  iterLower
) {
  if (iterLower === numSamples) {
    console.log('Done.');
    participant.delete();
    return;
  }
  ws.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
    if (err) throw err;
    let takeArray = reader.take(1);
    if (takeArray.length > 0 && takeArray[0].info.valid_data) {
      let sample = takeArray[0].sample;
      console.log('received: ' + JSON.stringify(sample));
      let msg = {userID: sample.userID, message: 'from pong'};
      console.log('replying with: ' + JSON.stringify(msg));
      writer.write(msg);
    }
    onDataAvailable(participant, ws, reader, writer, iterLower + 1);
  });
};
