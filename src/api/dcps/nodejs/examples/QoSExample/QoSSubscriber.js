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

/* QoSExample Subscriber.
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

/**
* Summary of how this example works:
*  When we call the wait method on a waitset, dds_waitset_wait runs in the
*  node.js thread pool. When it completes, it runs the callback provided in
*  the main node.js thread.
*
* The control flow of this example is as follows.
*  1. Wait for the first reader to match with its writer using
*     firstFloorMatchWs. When it completes, we proceed to step 2.
*  2. Wait for the second reader to match with its writer using
*     secondFloorMatchWs. When it completes, we proceed to step 3.
*  3. Wait for new data on the first and second readers, using
*     firstFloorNewDataWs and secondFloorNewDataWs, respectively. As the wait
*     calls are asynchronous, these two waitsets wait in parallel. When they
*     unblock, they either timeout or read a message and print it.
*/
'use strict';

const dds = require('vortexdds');
const EnvironmentConditionsTopic = require('./EnvironmentConditionsTopic.js');

main();

async function main() {
  // participant with default qos
  let participant = new dds.Participant();

  // set the topic qos, and use this for the reader too.
  // topic qos is only used for matching
  let tqos = dds.QoS.topicDefault();
  tqos.reliability = dds.ReliabilityKind.reliable;
  tqos.durability = dds.DurabilityKind.Volatile;
  let topic = await EnvironmentConditionsTopic.create(participant, tqos);

  // create subscribers for the first and second floors
  let sqos = dds.QoS.subscriberDefault();
  sqos.partition = 'floor1';
  let firstFloorSubscriber = participant.createSubscriber(sqos);
  sqos.partition = 'floor2';
  let secondFloorSubscriber = participant.createSubscriber(sqos);

  // create readers for the first and second floor
  let rqos = dds.QoS.readerDefault();
  rqos.reliability = dds.ReliabilityKind.reliable;
  rqos.durability = dds.DurabilityKind.Volatile;
  let firstFloorReader = firstFloorSubscriber.createReader(topic, rqos);
  let secondFloorReader = secondFloorSubscriber.createReader(topic, rqos);

  // set up waitsets and conditions used in this program
  // first floor publication matched waitset and condition
  let firstFloorMatchWs = new dds.Waitset(new dds.StatusCondition(
    firstFloorReader, dds.StatusMask.subscription_matched));

  // second floor publication matched waitset and condition
  let secondFloorMatchWs = new dds.Waitset(new dds.StatusCondition(
    secondFloorReader, dds.StatusMask.subscription_matched));

  // first floor new data waitset and condition
  let firstFloorNewDataWs = new dds.Waitset(new dds.ReadCondition(
    firstFloorReader, dds.StateMask.sample.not_read),
  'first floor new data');

  // second floor new data waitset and condition
  let secondFloorNewDataWs = new dds.Waitset(new dds.ReadCondition(
    secondFloorReader, dds.StateMask.sample.not_read),
  'second floor new data');

  console.log('Waiting for readers to be matched with their respective ' +
    'writers');
  firstFloorMatchWs.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
    if (err) throw err;
    secondFloorMatchWs.wait(dds.DDSConstants.DDS_INFINITY, function(err, res) {
      if (err) throw err;
      console.log('Readers matched with their respective writers.');
      // this increments when we read data. we quit when dataCount === 2
      let dataCount = 0;
      // asynchronously wait for data from first and second floor writers.
      // have a timeout of 2s for each
      firstFloorNewDataWs.wait(2 * Math.pow(10, 9), function(err, res) {
        if (err) {
          if (parseInt(err.message, 10)
            === dds.DDSConstants.DDS_RETCODE_TIMEOUT) {
            console.log('No data received from first floor writer.');
          }
        } else {
          // if dataCount > 1, then it's time to quit
          if (dataCount > 1) { participant.delete(); return; }
          dataCount++;
          let data = firstFloorReader.take(1);
          console.log('Data from first floor: ' +
            JSON.stringify(data[0].sample));
        }
      });
      secondFloorNewDataWs.wait(2 * Math.pow(10, 9), function(err, res) {
        if (err) {
          if (parseInt(err.message, 10)
            === dds.DDSConstants.DDS_RETCODE_TIMEOUT) {
            console.log('No data received from second floor writer.');
          }
        } else {
          if (dataCount > 1) { participant.delete(); return; }
          dataCount++;
          let data = secondFloorReader.take(1);
          console.log('Data from second floor: ' +
            JSON.stringify(data[0].sample));
        }
      });
    });
  });
};

