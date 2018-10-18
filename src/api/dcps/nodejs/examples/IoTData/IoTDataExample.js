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
    IoT Data Example.
    Instructions: 1. Run nodejs IoTDataExample.js

*/

'use strict';

const dds = require('vortexdds');
const IoTData = require('./IoTTopicHelper');

main();

async function main() {

  let participant = new dds.Participant();
  let topic = await IoTData.create(participant);

  integer64bitExample(participant, topic);

  allUnionTypesExample(participant, topic);

  participant.delete();

}

function allUnionTypesExample(participant, topic){

  console.log('');
  console.log('=== allUnionTypesExample start');

  let writer = new dds.Writer(participant, topic);
  let reader = new dds.Reader(participant, topic);

  let jsData = getAllUnionTypesSample(topic);

  console.log('write 1 sample');
  writer.write(jsData);

  console.log('take 1 sample');
  let takeArray = reader.take(1);
  if (takeArray.length > 0 && takeArray[0].info.valid_data) {
    let sample = takeArray[0].sample;
    console.log('   Sample ');
    console.log('   type name: ' + sample.typeName);
    console.log('   instance id ' + sample.instanceId);
    console.log(takeArray[0].sample);
  }

  reader.delete();
  writer.delete();
  console.log('=== allUnionTypesExample end');
}

function getAllUnionTypesSample(topic){

  let typeSupport = topic.typeSupport;
  let IoTValue = typeSupport.getClass('DDS::IoT::IoTValue');

  const jsData = {
    typeName: 'allUnionEnumTypes',
    instanceId: '00011235813',
    values: [
      {
        name: 'ui8',
        value: new IoTValue({ui8: 1}),
      },
      {
        name: 'ui16_test',
        value: new IoTValue({ui16: 0x0101}),
      },
      {
        name: 'ui32_test',
        value: new IoTValue({ui32: 0x01010101}),
      },
      {
        name: 'ui64_test', // 18446744073709551616 out of range
        value: new IoTValue({ui64: '18446744073709551615'}),
      },
      {
        name: 'i18_test',
        value: new IoTValue({i8: 1}),
      },
      {
        name: 'i16_test',
        value: new IoTValue({i16: 0x0101}),
      },
      {
        name: 'i32_test',
        value: new IoTValue({i32: 0x01010101}),
      },
      {
        name: 'i64_test', // '9223372036854775808' out of range
        value: new IoTValue({i64: '9223372036854775807'}),
      },
      {
        name: 'f32_test',
        value: new IoTValue({f32: 32.5}),
      },
      {
        name: 'f64_test',
        value: new IoTValue({f64: 64.5}),
      },
      {
        name: 'b_test',
        value: new IoTValue({b: true}),
      },
      {
        name: 'Str_test',
        value: new IoTValue({str: 'string value'}),
      },
      {
        name: 'ch_test',
        value: new IoTValue({ch: 42}),
      },
      {
        name: 'ui8seq_test',
        value: new IoTValue({ui8Seq: [1, 2]}),
      },
      {
        name: 'ui16seq_test',
        value: new IoTValue({ui16Seq: [0x0101, 0x0202]}),
      },
      {
        name: 'ui32seq_test',
        value: new IoTValue({ui32Seq: [0x01010101, 0x02020202]}),
      },
      {
        name: 'ui64seq_test',
        value: new IoTValue({ui64Seq: [
          '18446744073709551615',
          20120516,
        ]}),
      },
      {
        name: 'i8seq_test',
        value: new IoTValue({i8Seq: [1, 2]}) },
      {
        name: 'i16seq_test',
        value: new IoTValue({i16Seq: [0x0101, 0x0202]}) },
      {
        name: 'i32seq_test',
        value: new IoTValue({i32Seq: [0x01010101, 0x02020202]})},
      {
        name: 'i64seq_test',
        value: new IoTValue({i64Seq: [
          '9223372036854775807',
          20120516]}),
      },
      {
        name: 'f32seq_test',
        value: new IoTValue({f32Seq: [32.5, 16.25]}),
      },
      {
        name: 'f64seq_test',
        value: new IoTValue({f64Seq: [64.5, 128.125]}),
      },
      {
        name: 'bseq_test',
        value: new IoTValue({bSeq: [true, false, true]}),
      },
      {
        name: 'strseq_test',
        value: new IoTValue({strSeq: ['stringOne', 'stringTwo']}),
      },
      {
        name: 'chseq_test',
        value: new IoTValue({chSeq: [8, 7, 5, 16]}),
      },
    ],
  };

  return jsData;

}

function integer64bitExample(participant, topic){

  /* JavaScript does not currently include standard support for
    64-bit integer values.
    If the value will fit inside a JavaScript Number without losing
    precision, a number can be used, otherwise use a String.

    This example demonstrates the usage and ranges for
    the unsigned and signed 64 bit integers within Javascript.
    */

  console.log('');
  console.log('=== integer64bitExample start');

  let writer = new dds.Writer(participant, topic);
  let reader = new dds.Reader(participant, topic);

  let typeSupport = topic.typeSupport;
  let IoTValue = typeSupport.getClass('DDS::IoT::IoTValue');

  let jsData = {
    typeName: '64bitIntExample',
    instanceId: '001',
    values: [
      {
        name: 'ui64_test',
        value: new IoTValue({ui64: 0}),
      },
      {
        name: 'ui64_test', // 2 to the 53
        value: new IoTValue({ui64: 9007199254740992}),
      },
      { // values from (2 to the 53)+1 to (2 to the 64) must be
        // represented using strings
        name: 'ui64_test',
        value: new IoTValue({ui64: '9007199254740993'}),
      },
      {
        name: 'ui64_test', // 18446744073709551616 out of range
        value: new IoTValue({ui64: '18446744073709551615'}),
      },
      {
        name: 'i64_test', // - (2 to the 53)
        value: new IoTValue({i64: -9007199254740992}),
      },
      {
        name: 'i64_test', // (2 to the 53)
        value: new IoTValue({i64: 9007199254740992}),
      },
      {
        name: 'i64_test', // '9223372036854775808' out of range
        value: new IoTValue({i64: '9223372036854775807'}),
      },

    ],
  };

  console.log('write 1 sample');
  writer.write(jsData);

  console.log('take 1 sample');
  let takeArray = reader.take(1);
  if (takeArray.length > 0 && takeArray[0].info.valid_data) {
    let sample = takeArray[0].sample;
    console.log('   Sample ');
    console.log('   type name: ' + sample.typeName);
    console.log('   instance id ' + sample.instanceId);
    console.log(takeArray[0].sample);
  }

  reader.delete();
  writer.delete();
  console.log('=== integer64bitExample end');

}


