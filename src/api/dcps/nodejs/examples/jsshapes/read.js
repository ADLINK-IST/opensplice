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

/* This script demonstrates query conditions and (async) waitsets.
   It reads 100 samples from demo_ishapes.
   Usage: 1. start demo_ishapes.
          2. publish a blue circle.
          3. run nodejs read.js.
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

  // set up our topic
  const circleTopic = await ShapeTopicHelper.create(participant, 'Circle');

  // our reader has volatile qos
  let readerqosprovider = new dds.QoSProvider(
    './DDS_VolatileQoS_All.xml',
    'DDS VolatileQosProfile'
  );

  // set up circle reader
  const circleReader = participant.createReader(circleTopic,
    readerqosprovider.getReaderQos());

  // set up waitset
  const queryCond = new dds.QueryCondition(
    circleReader,
    mask,
    sqlExpression,
    params);
  const newDataWs = new dds.Waitset(queryCond);

  console.log('Waiting for demo_ishapes to publish a blue circle...');
  onDataAvailable(participant, newDataWs, circleReader, queryCond, 0);
};

/* onDataAvailable waits until we have new data (with the query condition).
 * when we do, we read it, print the data (shape data), and then call
 * onDataAvailable with iterLower incremented by one, to wait for more data.
 * This repeats until iterLower === numSamples (100 samples in this case).
 * When iterLower === numSamples, we delete the participant and quit.
 */
function onDataAvailable(
  participant,
  ws,
  reader,
  queryCond,
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
    }
    onDataAvailable(participant, ws, reader, queryCond, iterLower + 1);
  });
}
