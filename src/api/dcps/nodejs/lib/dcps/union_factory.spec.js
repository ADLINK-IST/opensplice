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

const expect = require('chai').expect;
const ref = require('ref');
const cico = require('./cico');
const ddstopic = require('./ddstopic');

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

describe('IoTValue union tests', function(){

  var typeSupport = null;
  var IoTValue = null;
  before(function(){
    typeSupport = new ddstopic.TypeSupport(
      iotData_typename,
      iotData_keys,
      iotData_xml
    );
    IoTValue = typeSupport.getClass('DDS::IoT::IoTValue');
  });

  it('test getter and setter', function() {
    let u = new IoTValue();
    u.b = false;
    expect(u.b).to.be.false;
  });

  it('creates IoTValue instances', function() {
    const type = new cico.Type({
      u: IoTValue,
    });
    expect(type.size).to.be.equal(4 + 4/* padding*/ + 24);
    expect(type.alignment).to.be.equal(8);

  });

  it('embeds IoTValue instances in a struct', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const expectedInfo = {
      name: {offset: 0, size: 8, align: 8},
      value: {offset: 8, size: 32, align: 8},
    };
    expect(IoTNVP.fields).to.be.length(2);
    for (var f of IoTNVP.fields) {
      expect(f).to.have.property('size', expectedInfo[f.name].size);
      expect(f).to.have.property('alignment', expectedInfo[f.name].align);
      expect(f).to.have.property('offset', expectedInfo[f.name].offset);
    }
    expect(IoTNVP.size).to.be.equal(40);
  });
});

describe('IoTValue initialization', function() {

  var typeSupport = null;
  var IoTValue = null;
  before(function(){
    typeSupport = new ddstopic.TypeSupport(
      iotData_typename,
      iotData_keys,
      iotData_xml
    );
    IoTValue = typeSupport.getClass('DDS::IoT::IoTValue');
  });

  it('constructs IoTValue from a case & value', function() {
    const data = new IoTValue({ f64: 32.25 });

    // expect(data).to.have.property('_d', ddsIoT.IoTType.TYPE_IoTF64);
    expect(data).to.have.property('_v', 32.25);
    expect(data.f64).to.equal(32.25);

  });

  it('throws errors on invalid IoTValue arguments', function() {
    expect(function() {
      const data = new IoTValue( // eslint-disable-line no-unused-vars
        { f128: 32.25 }
      );
    }).to.throw(TypeError);
    expect(function() {
      const data = new IoTValue( // eslint-disable-line no-unused-vars
        { f64: 32.25, i8: 1 }
      );
    }).to.throw(TypeError);
  });

  it('initilizes a union field with defaults', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const data = IoTNVP.newInstance();
    data.value.ui8 = 0;

    expect(data.value.ui8).to.equal(0);
    // expect(data.value.discriminator).to.equal(ddsIoT.IoTType.TYPE_IoTUI8);
  });

  // test data for setting & getting IoTValue cases
  const cases = {
    ui8: 1,
    ui16: 0x0101,
    ui32: 0x01010101,
    ui64: 0x0101010101010101,
    i8: 1,
    i16: 0x0101,
    i32: 0x01010101,
    i64: 0x0101010101010101,
    f32: 32.5,
    f64: 64.5,
    b: true,
    str: 'string value',
    ch: 42,
    ui8Seq: [1, 2],
    ui16Seq: [0x0101, 0x0202],
    ui32Seq: [0x01010101, 0x02020202],
    ui64Seq: [0x0101010101010101, 0x0202020202020202],
    i8Seq: [1, 2],
    i16Seq: [0x0101, 0x0202],
    i32Seq: [0x01010101, 0x02020202],
    i64Seq: [0x0101010101010101, 0x0202020202020202],
    f32Seq: [32.5, 16.25],
    f64Seq: [64.5, 128.125],
    bSeq: [true, false, true],
    strSeq: ['string one', 'string two'],
    chSeq: [42, 43, 44],
  };

  for (const case_name of Object.keys(cases)) {
    it('can set and get case ' + case_name, function() {
      const u = new IoTValue();
      u[case_name] = cases[case_name];
      expect(u[case_name]).to.deep.equal(cases[case_name]);
    });

    it('throws exception on reading unselected case ' + case_name, function() {
      const u = new IoTValue(); // selects ui8 be default.
      if (case_name === 'ui8') {
        u.i8 = 0; // select a different case
      }
      expect(function() { return u[case_name]; }).to.throw();
    });
  }
});

describe('IoTData copy in/out', function() {

  var typeSupport = null;
  var IoTValue = null;
  before(function(){
    typeSupport = new ddstopic.TypeSupport(
      iotData_typename,
      iotData_keys,
      iotData_xml
    );
    IoTValue = typeSupport.getClass('DDS::IoT::IoTValue');
  });

  it('can write an IoTValue', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const data = IoTNVP.newInstance();
    data.name = 'hello';
    data.value.ui32 = 0x10203040;

    const buffer = new Buffer(IoTNVP.size);
    const cleanup_list = [];
    IoTNVP._copyin(buffer, data, 0, cleanup_list);

    expect(cleanup_list).to.be.length(1);
    const d = buffer.slice(8, 12);
    const expected_d = new Buffer(
      // new Uint8Array([ddsIoT.IoTType.TYPE_IoTUI32.value, 0, 0, 0])
      new Uint8Array([2, 0, 0, 0])
    );
    expect(d).to.deep.equal(expected_d);
    const v = buffer.slice(16, 20);
    expect(v).to.deep.equal(
      new Buffer(new Uint8Array([0x40, 0x30, 0x20, 0x10]))
    );

  });

  it('can read an IoTValue', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const data = IoTNVP.newInstance();
    data.name = 'this is a long name';
    data.value.ui32Seq = [0x10000001, 0x20000002, 0x30000003, 0x40000004];
    const buf = new Buffer(IoTNVP.size);
    const cleanup_list = [];
    IoTNVP._copyin(buf, data, 0, cleanup_list);
    const data2 = IoTNVP.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('can write and read full IoTData instances', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const IoTData = new cico.Type({
      typeName: ref.types.CString,
      instanceId: ref.types.CString,
      values: new cico.Sequence(IoTNVP),
    });

    const data = {
      typeName: 'MyThermometer',
      instanceId: '00011235813',
      values: [],
    };
    const tempF = IoTNVP.newInstance();
    tempF.name = 'tempF';
    tempF.value.f64 = 59.0;
    data.values.push(tempF);
    const tempC = IoTNVP.newInstance();
    tempC.name = 'tempC';
    tempC.value.i32 = 15;
    data.values.push(tempC);

    const buf = new Buffer(IoTData.size);
    const cleanup_list = [];
    IoTData._copyin(buf, data, 0, cleanup_list);

    const data2 = IoTData.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('can serialize an IoTData to reasonable JSON', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });

    const data = {
      typeName: 'MyThermometer',
      instanceId: '00011235813',
      values: [],
    };
    const tempF = IoTNVP.newInstance();
    tempF.name = 'tempF';
    tempF.value.f64 = 59.0;
    data.values.push(tempF);
    const tempC = IoTNVP.newInstance();
    tempC.name = 'tempC';
    tempC.value.i32 = 15;
    data.values.push(tempC);

    expect(JSON.stringify(data)).to.equal(
      '{' +
        '"typeName":"MyThermometer",' +
        '"instanceId":"00011235813",' +
        '"values":[' +
          '{' +
            '"name":"tempF",' +
            '"value":{"f64":59}' +
          '},' +
          '{' +
            '"name":"tempC",' +
            '"value":{"i32":15}' +
          '}' +
        ']' +
      '}');
  });

  it('copyin', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const IoTData = new cico.Type({
      typeName: ref.types.CString,
      instanceId: ref.types.CString,
      values: new cico.Sequence(IoTNVP),
    });

    const jsData = {
      typeName: 'MyThermometer',
      instanceId: '00011235813',
      values: [
        { name: 'tempF', value: new IoTValue({f64: 59}) },
        { name: 'tempC', value: new IoTValue({i32: 15}) },
      ],
    };

    let buf = IoTData.copyin(jsData);
    expect(buf).is.not.null;
  });

  it('copyin, copyout', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const IoTData = new cico.Type({
      typeName: ref.types.CString,
      instanceId: ref.types.CString,
      values: new cico.Sequence(IoTNVP),
    });

    const jsData = {
      typeName: 'MyThermometer',
      instanceId: '00011235813',
      values: [
        { name: 'tempF', value: new IoTValue({f64: 59}) },
        { name: 'tempC', value: new IoTValue({i32: 15}) },
      ],
    };

    let buf = IoTData.copyin(jsData);
    let copyOutJSObj = IoTData.copyout(buf);
    expect(copyOutJSObj).deep.equal(jsData);
  });

  it('copyin, copyout ui64, i64', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const IoTData = new cico.Type({
      typeName: ref.types.CString,
      instanceId: ref.types.CString,
      values: new cico.Sequence(IoTNVP),
    });

    const jsData = {
      typeName: 'johnnyCash',
      instanceId: 'folsom',
      values: [
        { name: 'walkTheLine',
          value: new IoTValue({ui64: '18446744073709551615'}) },
        { name: 'ringOfFire',
          value: new IoTValue({i64: '9223372036854775807'}) },
      ],
    };

    let buf = IoTData.copyin(jsData);
    let copyOutJSObj = IoTData.copyout(buf);
    expect(copyOutJSObj).deep.equal(jsData);
  });

  it('copyin, ui64 - out of range error', function() {
    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const IoTData = new cico.Type({
      typeName: ref.types.CString,
      instanceId: ref.types.CString,
      values: new cico.Sequence(IoTNVP),
    });

    const jsData = {
      typeName: 'cohen',
      instanceId: 'thePartisan',
      values: [
        { name: 'shelter',
          value: new IoTValue({ui64: '18446744073709551616'}) },
      ],
    };

    // expect an out of range error
    expect(function() {
      IoTData.copyin(jsData);
    }).to.throw(TypeError);

  });

  it('copyin, copyout: all enum types', function() {

    const IoTNVP = new cico.Type({
      name: ref.types.CString,
      value: IoTValue,
    });
    const IoTData = new cico.Type({
      typeName: ref.types.CString,
      instanceId: ref.types.CString,
      values: new cico.Sequence(IoTNVP),
    });

    const jsData = {
      typeName: 'MyThermometer',
      instanceId: '00011235813',
      values: [
        { name: 'ui8_1',
          value: new IoTValue({ui8: 1}) },
        { name: 'ui16_0x0101',
          value: new IoTValue({ui16: 0x0101}) },
        { name: 'ui32',
          value: new IoTValue({ui32: 0x01010101}) },
        { name: 'ui64', // comment out
          value: new IoTValue({ui64: '18446744073709551615'}) },
        { name: 'i18',
          value: new IoTValue({i8: 1}) },
        { name: 'i16',
          value: new IoTValue({i16: 0x0101}) },
        { name: 'i32',
          value: new IoTValue({i32: 0x01010101}) },
        { name: 'i64',
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
            20120516]})},
        { name: 'i18seq',
          value: new IoTValue({i8Seq: [1, 2]}) },
        { name: 'i16seq',
          value: new IoTValue({i16Seq: [0x0101, 0x0202]}) },
        { name: 'i32seq',
          value: new IoTValue({i32Seq: [0x01010101, 0x02020202]})},
        { name: 'i64seq',
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

    let buf = IoTData.copyin(jsData);
    let copyOutJSObj = IoTData.copyout(buf);
    expect(copyOutJSObj).deep.equal(jsData);
  });


});
