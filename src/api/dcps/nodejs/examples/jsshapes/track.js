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

/* This script demonstrates status conditions, query conditions and waitsets.
   It reads a blue circle from demo_ishapes and publishes a red square to the
   same position it read the blue circle from.
   Usage: 1. start demo_ishapes.
          2. run nodejs track.js
          3. publish a blue circle.
          4. subscribe to a square
*/

'use strict';

const dds = require('vortexdds');
const ShapeTopicHelper = require('./ShapeTopicHelper');
const util = require('util');

// we are reading 100 samples of a blue circle
const shapeName = 'Circle';
const numSamples = 100;
const mask = dds.StateMask.sample.not_read;
const sqlExpression = 'color=%0';
const params = ['BLUE'];


main();

async function main() {
  const participant = new dds.Participant();

  // set up topic
  const circleTopic = await ShapeTopicHelper.create(participant, 'Circle');
  const squareTopic = await ShapeTopicHelper.create(participant, 'Square');

  // our blue circle reader reader has volatile qos
  let readerqosprovider = new dds.QoSProvider(
    './DDS_VolatileQoS_All.xml',
    'DDS VolatileQosProfile'
  );

  // set up reader for the blue circle
  const circleReader = participant.createReader(
    circleTopic,
    readerqosprovider.getReaderQos()
  );

  // our square writer has default QoS
  const squareWriter = participant.createWriter(squareTopic);

  // set up a waitset on our circle reader for the query condition
  // (shape read = blue circle).
  const queryCond = new dds.QueryCondition(
    circleReader,
    mask,
    sqlExpression,
    params);
  const queryWaitset = new dds.Waitset(queryCond);

  // set up our status condition, which is for matching the publication
  // with the square topic
  const pubWaitset = new dds.Waitset(new dds.StatusCondition(squareWriter,
    dds.StatusMask.publication_matched));

  console.log('Waiting for demo_ishapes to subscribe to a square...');
  pubWaitset.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
    if (err) throw err;
    console.log('Waiting for demo_ishapes to publish a blue circle...');
    onDataAvailable(
      participant,
      queryWaitset,
      queryCond,
      circleReader,
      squareWriter,
      0
    );
  });
}

/* onDataAvailable waits until we have new data (with the query condition).
 * when we do, we read it, print the data (shape data), track the
 * circle with a red square, and then call onDataAvailable with iterLower
 * incremented by one, to wait for more data. This repeats until
 * iterLower === numSamples (100 samples in this case).
 * When iterLower === numSamples, we delete the participant and quit.
*/
function onDataAvailable(
  participant,
  ws,
  queryCond,
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
    let sampleArray = reader.takeCond(1, queryCond);
    if (sampleArray.length > 0 && sampleArray[0].info.valid_data) {
      let sample = sampleArray[0].sample;
      console.log(
        util.format(
          '%s %s of size %d at (%d,%d)',
          sample.color,
          shapeName,
          sample.shapesize,
          sample.x,
          sample.y
        )
      );
      writer.write({
        color: 'RED',
        x: sample.x,
        y: sample.y,
        shapesize: 45,
      });
    }
    onDataAvailable(participant, ws, queryCond, reader, writer, iterLower + 1);
  });
}

