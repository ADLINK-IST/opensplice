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
    Ping Pong ping.
    Instructions: run nodejs ping.js
                  run nodejs pong.js
                  You may run these in any order.
*/
/**
 * Summary: Ping sends a message on the "ping" partition with a userID of 0,
 * which pong reads. Pong sends the message back on the "pong" partition with
 * the same userID field. Ping reads it, and sends the message back with 1
 * added to the userID field. This is repeated until userID = 20.
 *
 * All the messages from ping have 'from ping' in the message field, and all the
 * messages from pong have 'from pong' in the message field.
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

  // create publisher on partition 'Ping' and writer on the publisher
  // on pong, we have a subscriber on the same partition and topic
  let pqos = dds.QoS.publisherDefault();
  pqos.partition = 'Ping';
  let publisher = participant.createPublisher(pqos, null);
  let writer = publisher.createWriter(topic);

  // create a subscriber on partition 'Pong' and a reader on the subscriber
  let sqos = dds.QoS.subscriberDefault();
  sqos.partition = 'Pong';
  let subscriber = participant.createSubscriber(sqos);
  let reader = subscriber.createReader(topic);

  // create waitset which waits until pong subscriber on partition
  // 'Ping' is found
  let pubMatchedWaitset = new dds.Waitset(new dds.StatusCondition(writer,
    dds.StatusMask.publication_matched));

  // waitset for new data
  let newDataWaitset = new dds.Waitset(new dds.ReadCondition(reader,
    dds.StateMask.sample.not_read));

  // block until pong has been found
  console.log('Waiting for pong subscriber to be found...');
  pubMatchedWaitset.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
    if (err) throw err;
    console.log('Found pong subscriber.');
    // Ping writes the first message
    let msg = {userID: 0, message: 'from ping'};
    console.log('sending ' + JSON.stringify(msg));
    writer.write(msg);
    onDataAvailable(participant, newDataWaitset, reader, writer, 0);
  });
}

/* onDataAvailable waits for new data. When we have new data, it
 * reads it, prints it, and sends back a message with the userID
 * field incremented by one and 'from ping' in the message field.
 * It then calls onDataAvailable again.
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
    console.log('Done');
    participant.delete();
    return;
  }
  // when we get a message from pong, read it, then send it back
  // with userID+1 and message 'from ping'
  ws.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
    if (err) throw err;
    let takeArray = reader.take(1);
    if (takeArray.length > 0 && takeArray[0].info.valid_data) {
      let sample = takeArray[0].sample;
      console.log('received: ' + JSON.stringify(sample));
      let msg = {userID: sample.userID + 1, message: 'from ping'};
      console.log('sending ' + JSON.stringify(msg));
      writer.write(msg);
    }
    onDataAvailable(participant, ws, reader, writer, iterLower + 1);
  });
}

