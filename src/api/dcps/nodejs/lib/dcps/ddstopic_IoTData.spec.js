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

'use strict';
/* eslint-env node, mocha */

const dds = require('./ddstopic');
const expect = require('chai').expect;
const dcps = require('./dcps');

const iotData_typename = 'DDS::IoT::IoTData';
const iotData_keys = 'typeName,instanceId';
const iotData_xml = '<MetaData version="1.0.0"><Module name="DDS">' +
  '<Module name="IoT">' +
  '<Enum name="IoTType">' +
  '<Element name="TYPE_IoTUI8" value="0"/>' +
  '<Element name="TYPE_IoTUI16" value="1"/>' +
  '<Element name="TYPE_IoTUI32" value="2"/>' +
  '<Element name="TYPE_IoTUI64" value="3"/>' +
  '<Element name="TYPE_IoTI8" value="4"/>' +
  '<Element name="TYPE_IoTI16" value="5"/>' +
  '<Element name="TYPE_IoTI32" value="6"/>' +
  '<Element name="TYPE_IoTI64" value="7"/>' +
  '<Element name="TYPE_IoTF32" value="8"/>' +
  '<Element name="TYPE_IoTF64" value="9"/>' +
  '<Element name="TYPE_IoTB" value="10"/>' +
  '<Element name="TYPE_IoTStr" value="11"/>' +
  '<Element name="TYPE_IoTCh" value="12"/>' +
  '<Element name="TYPE_IoTUI8Seq" value="13"/>' +
  '<Element name="TYPE_IoTUI16Seq" value="14"/>' +
  '<Element name="TYPE_IoTUI32Seq" value="15"/>' +
  '<Element name="TYPE_IoTUI64Seq" value="16"/>' +
  '<Element name="TYPE_IoTI8Seq" value="17"/>' +
  '<Element name="TYPE_IoTI16Seq" value="18"/>' +
  '<Element name="TYPE_IoTI32Seq" value="19"/>' +
  '<Element name="TYPE_IoTI64Seq" value="20"/>' +
  '<Element name="TYPE_IoTF32Seq" value="21"/>' +
  '<Element name="TYPE_IoTF64Seq" value="22"/>' +
  '<Element name="TYPE_IoTBSeq" value="23"/>' +
  '<Element name="TYPE_IoTStrSeq" value="24"/>' +
  '<Element name="TYPE_IoTChSeq" value="25"/>' +
  '</Enum><TypeDef name="IoTUI8"><Octet/>' +
  '</TypeDef><TypeDef name="IoTUI16"><UShort/></TypeDef>' +
  '<TypeDef name="IoTUI32"><ULong/></TypeDef>' +
  '<TypeDef name="IoTUI64"><ULongLong/></TypeDef>' +
  '<TypeDef name="IoTI8"><Char/></TypeDef>' +
  '<TypeDef name="IoTI16"><Short/></TypeDef>' +
  '<TypeDef name="IoTI32"><Long/></TypeDef>' +
  '<TypeDef name="IoTI64"><LongLong/></TypeDef>' +
  '<TypeDef name="IoTF32"><Float/></TypeDef>' +
  '<TypeDef name="IoTF64"><Double/></TypeDef>' +
  '<TypeDef name="IoTB"><Boolean/></TypeDef>' +
  '<TypeDef name="IoTStr"><String/></TypeDef>' +
  '<TypeDef name="IoTCh"><Char/></TypeDef>' +
  '<TypeDef name="IoTUI8Seq"><Sequence>' +
  '<Type name="IoTUI8"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTUI16Seq"><Sequence>' +
  '<Type name="IoTUI16"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTUI32Seq"><Sequence>' +
  '<Type name="IoTUI32"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTUI64Seq"><Sequence>' +
  '<Type name="IoTUI64"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTI8Seq"><Sequence>' +
  '<Type name="IoTI8"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTI16Seq"><Sequence>' +
  '<Type name="IoTI16"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTI32Seq"><Sequence>' +
  '<Type name="IoTI32"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTI64Seq"><Sequence>' +
  '<Type name="IoTI64"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTF32Seq"><Sequence>' +
  '<Type name="IoTF32"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTF64Seq"><Sequence>' +
  '<Type name="IoTF64"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTBSeq"><Sequence><' +
  'Type name="IoTB"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTStrSeq"><Sequence>' +
  '<Type name="IoTStr"/></Sequence></TypeDef>' +
  '<TypeDef name="IoTChSeq"><Sequence>' +
  '<Type name="IoTCh"/></Sequence></TypeDef>' +
  '<Union name="IoTValue"><SwitchType>' +
  '<Type name="IoTType"/></SwitchType>' +
  '<Case name="ui8"><Type name="IoTUI8"/>' +
  '<Label value="TYPE_IoTUI8"/></Case>' +
  '<Case name="ui16"><Type name="IoTUI16"/>' +
  '<Label value="TYPE_IoTUI16"/></Case>' +
  '<Case name="ui32"><Type name="IoTUI32"/>' +
  '<Label value="TYPE_IoTUI32"/></Case>' +
  '<Case name="ui64"><Type name="IoTUI64"/>' +
  '<Label value="TYPE_IoTUI64"/></Case>' +
  '<Case name="i8"><Type name="IoTI8"/>' +
  '<Label value="TYPE_IoTI8"/></Case>' +
  '<Case name="i16"><Type name="IoTI16"/>' +
  '<Label value="TYPE_IoTI16"/></Case>' +
  '<Case name="i32"><Type name="IoTI32"/>' +
  '<Label value="TYPE_IoTI32"/></Case>' +
  '<Case name="i64"><Type name="IoTI64"/>' +
  '<Label value="TYPE_IoTI64"/></Case>' +
  '<Case name="f32"><Type name="IoTF32"/>' +
  '<Label value="TYPE_IoTF32"/></Case>' +
  '<Case name="f64"><Type name="IoTF64"/>' +
  '<Label value="TYPE_IoTF64"/></Case>' +
  '<Case name="b"><Type name="IoTB"/>' +
  '<Label value="TYPE_IoTB"/></Case>' +
  '<Case name="str"><Type name="IoTStr"/>' +
  '<Label value="TYPE_IoTStr"/></Case>' +
  '<Case name="ch"><Type name="IoTCh"/>' +
  '<Label value="TYPE_IoTCh"/></Case>' +
  '<Case name="ui8Seq"><Type name="IoTUI8Seq"/>' +
  '<Label value="TYPE_IoTUI8Seq"/></Case>' +
  '<Case name="ui16Seq"><Type name="IoTUI16Seq"/>' +
  '<Label value="TYPE_IoTUI16Seq"/></Case>' +
  '<Case name="ui32Seq"><Type name="IoTUI32Seq"/>' +
  '<Label value="TYPE_IoTUI32Seq"/></Case>' +
  '<Case name="ui64Seq"><Type name="IoTUI64Seq"/>' +
  '<Label value="TYPE_IoTUI64Seq"/></Case>' +
  '<Case name="i8Seq"><Type name="IoTI8Seq"/>' +
  '<Label value="TYPE_IoTI8Seq"/></Case>' +
  '<Case name="i16Seq"><Type name="IoTI16Seq"/>' +
  '<Label value="TYPE_IoTI16Seq"/></Case>' +
  '<Case name="i32Seq"><Type name="IoTI32Seq"/>' +
  '<Label value="TYPE_IoTI32Seq"/></Case>' +
  '<Case name="i64Seq"><Type name="IoTI64Seq"/>' +
  '<Label value="TYPE_IoTI64Seq"/></Case>' +
  '<Case name="f32Seq"><Type name="IoTF32Seq"/>' +
  '<Label value="TYPE_IoTF32Seq"/></Case>' +
  '<Case name="f64Seq"><Type name="IoTF64Seq"/>' +
  '<Label value="TYPE_IoTF64Seq"/></Case>' +
  '<Case name="bSeq"><Type name="IoTBSeq"/>' +
  '<Label value="TYPE_IoTBSeq"/></Case>' +
  '<Case name="strSeq"><Type name="IoTStrSeq"/>' +
  '<Label value="TYPE_IoTStrSeq"/></Case>' +
  '<Case name="chSeq"><Type name="IoTChSeq"/>' +
  '<Label value="TYPE_IoTChSeq"/></Case>' +
  '</Union>' +
  '<Struct name="IoTNVP">' +
  '<Member name="name"><String/></Member>' +
  '<Member name="value"><Type name="IoTValue"/></Member>' +
  '</Struct>' +
  '<TypeDef name="IoTNVPSeq">' +
  '<Sequence><Type name="IoTNVP"/></Sequence></TypeDef>' +
  '<Struct name="IoTData">' +
  '<Member name="typeName"><String/></Member>' +
  '<Member name="instanceId"><String/></Member>' +
  '<Member name="values"><Type name="IoTNVPSeq"/></Member>' +
  '</Struct></Module></Module></MetaData>';


describe('Type Support tests: copyin, copyout ', function() {

  it('IoTData : typeSupport', function() {
    let typeSupport = new dds.TypeSupport(
      iotData_typename,
      iotData_keys,
      iotData_xml
    );
    expect(typeSupport).is.not.null;
    expect(typeSupport).instanceof(dds.TypeSupport);
  });


});

describe('Type Support: Writing and reading', function() {
  var dp = null;
  before(function(){
    dp = new dcps.Participant();
  });

  after(function(){
    dp.delete();
  });

  function testSampleTopic(topic, sampleJSObj){

    let reader = dp.createReader(topic);
    expect(reader).to.not.be.null;
    let writer = dp.createWriter(topic);
    expect(writer).to.not.be.null;

    // write out 1 sample
    let status = writer.write(sampleJSObj);
    expect(status).to.be.equal(0);

    // read 1 sample
    let numSamples = 1;
    let readArray = reader.read(numSamples);
    expect(readArray).to.not.be.null;
    expect(readArray.length).equals(numSamples);

    // verify that the data read is equivalent to
    // data sample written out
    let sampleItem = readArray[0];
    let sample = sampleItem.sample;
    expect(sample).deep.equal(sampleJSObj);

  }

  it('test creating IoTData topic', function(done) {

    let topic = dp.createTopic(
      'iotDataTest',
      iotData_typename,
      iotData_keys,
      iotData_xml
    );
    expect(topic).to.not.be.null;

    done();
  });

  it('IoTData: test write and read', function(done) {

    let topic = dp.createTopic(
      'IoTData_PW',
      iotData_typename,
      iotData_keys,
      iotData_xml
    );
    expect(topic).to.not.be.null;

    let IoTValue = topic.typeSupport.getClass('DDS::IoT::IoTValue');

    let jsData = {
      typeName: 'MyThermometer',
      instanceId: '00011235813',
      values: [
        { name: 'ui8_1',
          value: new IoTValue({ui8: 1}) },
        { name: 'ui16_0x0101',
          value: new IoTValue({ui16: 0x0101}) },
        { name: 'ui32',
          value: new IoTValue({ui32: 0x01010101}) },
        { name: 'ui64', // 18446744073709551616 out of range
          value: new IoTValue({ui64: '18446744073709551615'}) },
        { name: 'i18',
          value: new IoTValue({i8: 1}) },
        { name: 'i16',
          value: new IoTValue({i16: 0x0101}) },
        { name: 'i32',
          value: new IoTValue({i32: 0x01010101}) },
        { name: 'i64', // '9223372036854775808' out of range
          value: new IoTValue({i64: '9223372036854775807'}) },
        { name: 'f32',
          value: new IoTValue({f32: 32.5}) },
        { name: 'f64',
          value: new IoTValue({f64: 64.5}) },
        { name: 'b_true',
          value: new IoTValue({b: true}) },
        { name: 'Str',
          value: new IoTValue({str: 'string value'}) },
        { name: 'ch_42',
          value: new IoTValue({ch: 42}) },
        { name: 'ui8seq_12',
          value: new IoTValue({ui8Seq: [1, 2]}) },
        { name: 'ui16seq',
          value: new IoTValue({ui16Seq: [0x0101, 0x0202]}) },
        { name: 'ui32seq',
          value: new IoTValue({ui32Seq: [0x01010101, 0x02020202]})},
        { name: 'ui64seq',
          value: new IoTValue({ui64Seq: ['18446744073709551615',
            20120516 ]})},
        { name: 'i18seq',
          value: new IoTValue({i8Seq: [1, 2]}) },
        { name: 'i16seq',
          value: new IoTValue({i16Seq: [0x0101, 0x0202]}) },
        { name: 'i32seq',
          value: new IoTValue({i32Seq: [0x01010101, 0x02020202]})},
        { name: 'i64seq', // comment out
          value: new IoTValue({i64Seq: ['9223372036854775807',
            20120516]})},
        { name: 'f32seq',
          value: new IoTValue({f32Seq: [32.5, 16.25]}) },
        { name: 'f64seq',
          value: new IoTValue({f64Seq: [64.5, 128.125]}) },
        { name: 'bseq',
          value: new IoTValue({bSeq: [true, false, true]}) },
        { name: 'strseq',
          value: new IoTValue({strSeq: ['stringOne', 'stringTwo']})},
        { name: 'chseq_87516',
          value: new IoTValue({chSeq: [8, 7, 5, 16]}) },
      ],
    };

    testSampleTopic(
      topic,
      jsData
    );

    done();
  });


});
