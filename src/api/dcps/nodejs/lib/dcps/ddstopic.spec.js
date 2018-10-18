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

const ddstopic = require('./ddstopic');
const expect = require('chai').expect;
const dcps = require('./dcps');
const path = require('path');

const simple_xml = '<MetaData version="1.0.0">' +
'<Module name="HelloWorldData">' +
    '<Struct name="Msg">' +
        '<Member name="userID"><Long/></Member>' +
        '<Member name="message"><Long/></Member>' +
    '</Struct>' +
'</Module>' +
'</MetaData>';
const simple_typename = 'HelloWorldData::Msg';
const simple_keys = 'userID';

const shape_xml = '<MetaData version="1.0.0">' +
    '<Struct name="ShapeType">' +
        '<Member name="color"><String/></Member>' +
        '<Member name="x"><Long/></Member>' +
        '<Member name="y"><Long/></Member>' +
        '<Member name="shapesize"><Long/></Member>' +
    '</Struct>' +
'</MetaData>';
const shape_typename = 'ShapeType';
const shape_keys = 'color';

const nestedStruct_xml = '<MetaData version="1.0.0">' +
'<Module name="Test">' +
'<Struct name="Inner"><Member name="foo"><String/></Member></Struct>' +
'<Struct name="Outer">' +
'<Member name="id"><Long/></Member>' +
'<Member name="inner"><Type name="Inner"/></Member>' +
'</Struct>' +
'</Module></MetaData>';
const nestedStruct_typename = 'Test::Outer';
const nestedStruct_keys = 'id';

const simpleTypes_xml = '<MetaData version="1.0.0">' +
'<Module name="basic"><Module name="module_SimpleTypes">' +
'<Struct name="SimpleTypes_struct"><Member name="long1"><Long/>' +
'</Member><Member name="ulong1"><ULong/></Member>' +
'<Member name="longlong1"><LongLong/></Member>' +
'<Member name="ulonglong1"><ULongLong/></Member>' +
'<Member name="float1"><Float/></Member>' +
'<Member name="short1"><Short/></Member>' +
'<Member name="ushort1"><UShort/></Member>' +
'<Member name="char1"><Char/></Member>' +
'<Member name="octet1"><Octet/></Member>' +
'<Member name="double1"><Double/></Member>' +
'<Member name="bool1"><Boolean/></Member>' +
'<Member name="string1"><String/></Member>' +
'</Struct></Module></Module></MetaData>';
const simpleTypes_typename = 'basic::module_SimpleTypes::SimpleTypes_struct';
const simpleTypes_keys = 'long1';

const enum_xml = '<MetaData version="1.0.0">' +
  '<Module name="basic">' +
    '<Module name="enumZ">' +
      '<Enum name="Color">' +
        '<Element name="Red" value="0"/>' +
        '<Element name="Green" value="1"/>' +
        '<Element name="Blue" value="2"/></Enum>' +
    '</Module>' +
    '<Module name="module_Enum2">' +
      '<Enum name="Color">' +
        '<Element name="Red" value="0"/>' +
        '<Element name="Green" value="1"/>' +
        '<Element name="Blue" value="2"/></Enum>' +
      '<TypeDef name="ZColor"><Type name="basic::enumZ::Color"/></TypeDef>' +
      '<TypeDef name="AColor"><Type name="Color"/></TypeDef>' +
      '<Struct name="Enum2_struct">' +
        '<Member name="long1"><Long/></Member>' +
        '<Member name="color1"><Type name="AColor"/></Member>' +
        '<Member name="color2"><Type name="ZColor"/></Member>' +
        '<Member name="array">' +
        '<Array size="2"><Type name="ZColor"/></Array></Member>' +
      '</Struct>' +
    '</Module>' +
  '</Module>' +
'</MetaData>';
const enum_typename = 'basic::module_Enum2::Enum2_struct';
const enum_keys = 'long1';

const enum_xml2 = '<MetaData version="1.0.0">' +
'<Module name="AGraph">' +
  '<Enum name="AEnumType">' +
    '<Element name="Q_AREA" value="0"/>' +
    '<Element name="R_AREA" value="1"/>' +
    '<Element name="S_AREA" value="2"/>' +
    '<Element name="T_AREA" value="3"/>' +
    '<Element name="U_AREA" value="4"/>' +
  '</Enum>' +
  '<Struct name="AGraphEntity">' +
    '<Member name="id"><Long/></Member>' +
    '<Member name="robot_id"><ULong/></Member>' +
    '<Member name="type"><Type name="AEnumType"/></Member>' +
  '</Struct>' +
'</Module>' +
'</MetaData>';
const enum_typename2 = 'AGraph::AGraphEntity';
const enum_keys2 = 'id';

const sequence_xml = '<MetaData version="1.0.0">' +
  '<Module name="basic">' +
    '<Module name="module_Sequence">' +
      '<Struct name="Sequence_struct">' +
        '<Member name="long1"><Long/></Member>' +
        '<Member name="seq1"><Sequence><Long/></Sequence></Member>' +
      '</Struct>' +
    '</Module>' +
  '</Module>' +
'</MetaData>';
const sequence_typename = 'basic::module_Sequence::Sequence_struct';
const sequence_keys = 'long1';

const sequenceOfStructs_xml = '<MetaData version="1.0.0">' +
'<Module name="basic">' +
  '<Module name="module_SequenceOfStruct">' +
    '<Struct name="Inner">' +
      '<Member name="short1"><Short/></Member>' +
      '<Member name="double1"><Double/></Member>' +
    '</Struct>' +
    '<TypeDef name="inner_struct"><Type name="Inner"/></TypeDef>' +
    '<Struct name="SequenceOfStruct_struct">' +
      '<Member name="long1"><Long/></Member>' +
      '<Member name="seq1">' +
        '<Sequence><Type name="inner_struct"/></Sequence>' +
      '</Member>' +
    '</Struct>' +
  '</Module>' +
'</Module>' +
'</MetaData>';
const sequenceOfStructs_typename =
  'basic::module_SequenceOfStruct::SequenceOfStruct_struct';
const sequenceOfStructs_keys = 'long1';

const array_xml = '<MetaData version="1.0.0">' +
  '<Module name="basic">' +
    '<Module name="module_Array">' +
      '<Struct name="Array_struct">' +
        '<Member name="long1"><Long/></Member>' +
        '<Member name="array1"><Array size="3"><Double/></Array></Member>' +
      '</Struct>' +
    '</Module>' +
  '</Module>' +
'</MetaData>';
const array_typename = 'basic::module_Array::Array_struct';
const array_keys = 'long1';

describe('Type Support tests: copyin, copyout ', function() {

  it('simple sequence : copyin, copyout', function(done) {
    let typeSupport = new ddstopic.TypeSupport(
      sequence_typename,
      sequence_keys,
      sequence_xml
    );

    expect(typeSupport).is.not.null;
    expect(typeSupport).instanceof(ddstopic.TypeSupport);

    let jsData = {
      long1: 16,
      seq1: [201, 202, 203, 204],
    };

    let copyInBuf = typeSupport.copyin(jsData);
    let copyOutJSObj = typeSupport.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);

    done();
  });

  it('shape : builds from (typename,keys,xml)', function(done) {

    let shapeTS = new ddstopic.TypeSupport(
      shape_typename,
      shape_keys,
      shape_xml
    );

    expect(shapeTS).is.not.null;
    expect(shapeTS).instanceof(ddstopic.TypeSupport);

    done();
  });

  it('enum complex : creates a ref.buffer on copyin', function() {
    let complexEnumTS2 = new ddstopic.TypeSupport(
      enum_typename,
      enum_keys,
      enum_xml
    );

    expect(complexEnumTS2).is.not.null;
    expect(complexEnumTS2).instanceof(ddstopic.TypeSupport);

    let ColorA = complexEnumTS2.getClass('basic::module_Enum2::Color');
    let ColorZ = complexEnumTS2.getClass('basic::enumZ::Color');
    expect(ColorA).is.not.null;
    expect(ColorZ).is.not.null;

    let jsData = {
      long1: 16,
      color1: ColorA.get('Green').value,
      color2: ColorZ.get('Blue').value,
      array: [ColorZ.get('Red').value, ColorZ.get('Blue').value],
    };

    let copyInBuf = complexEnumTS2.copyin(jsData);
    let copyOutJSObj = complexEnumTS2.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);

  });

  it('enum : creates a ref.buffer on copyin', function() {
    let enumTS2 = new ddstopic.TypeSupport(
      enum_typename2,
      enum_keys2,
      enum_xml2
    );
    let enumA = enumTS2.getClass('AGraph::AEnumType');

    expect(enumTS2).is.not.null;
    expect(enumTS2).instanceof(ddstopic.TypeSupport);
    expect(enumA).is.not.null;

    let jsData = {
      id: 16,
      robot_id: 42,
      type: enumA.get('S_AREA').value,
    };

    let copyInBuf = enumTS2.copyin(jsData);
    let copyOutJSObj = enumTS2.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);

  });

  it('simple : copyin, copyout', function() {
    let typeSupport = new ddstopic.TypeSupport(
      simple_typename,
      simple_keys,
      simple_xml
    );
    expect(typeSupport).is.not.null;
    expect(typeSupport).instanceof(ddstopic.TypeSupport);

    let jsData = {userID: 14, message: 42};

    let copyInBuf = typeSupport.copyin(jsData);
    let copyOutJSObj = typeSupport.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);
  });

  it('Nested : copyin, copyout', function() {
    let typeSupport = new ddstopic.TypeSupport(
      nestedStruct_typename,
      nestedStruct_keys,
      nestedStruct_xml
    );
    expect(typeSupport).is.not.null;
    expect(typeSupport).instanceof(ddstopic.TypeSupport);

    let jsData = {id: 6, inner: {foo: 'abcd'}};
    let copyInBuf = typeSupport.copyin(jsData);
    let copyOutJSObj = typeSupport.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);
  });

  it('Simple types : copyin, copyout', function() {
    let typeSupport = new ddstopic.TypeSupport(
      simpleTypes_typename,
      simpleTypes_keys,
      simpleTypes_xml
    );
    expect(typeSupport).is.not.null;
    expect(typeSupport).instanceof(ddstopic.TypeSupport);

    let jsData = {
      long1: 1,
      ulong1: 2,
      longlong1: 3,
      ulonglong1: 4,
      float1: 5,
      short1: 6,
      ushort1: 7,
      char1: 8,
      octet1: 9,
      double1: 10,
      bool1: true,
      string1: 'abc',
    };

    let copyInBuf = typeSupport.copyin(jsData);
    let copyOutJSObj = typeSupport.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);
  });

  it('simple array : creates a ref.buffer on copyin', function(done) {
    let typeSupport = new ddstopic.TypeSupport(
      array_typename,
      array_keys,
      array_xml
    );
    expect(typeSupport).is.not.null;
    expect(typeSupport).instanceof(ddstopic.TypeSupport);

    let jsData = {long1: 16, array1: [7, 8, 2014]};

    let copyInBuf = typeSupport.copyin(jsData);
    let copyOutJSObj = typeSupport.copyout(copyInBuf);

    expect(copyOutJSObj).deep.equal(jsData);

    done();
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

  function testSampleTopic1(topicName, typename, keys, xml, sampleJSObj) {

    let typeSupport = new ddstopic.TypeSupport(typename, keys, xml);
    testSampleTopic2(typeSupport, topicName, sampleJSObj);

  }

  function testSampleTopic2(typeSupport, topicName, sampleJSObj){

    let topic = dp.createTopic(
      topicName,
      typeSupport.getTypename(),
      typeSupport.getKeys(),
      typeSupport.getXML()
    );
    expect(topic).to.not.be.null;

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

  it('sequenceOfStructs: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    let inner1 = {short1: 999, double1: 222};
    let inner2 = {short1: 777, double1: 333};
    let jsData = {long1: 7, seq1: [inner1, inner2]};

    testSampleTopic1(
      'SequenceOfStructs',
      sequenceOfStructs_typename,
      sequenceOfStructs_keys,
      sequenceOfStructs_xml,
      jsData
    );

    done();
  });

  it('simple sequence: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    // simple message
    let jsData = {
      long1: 16,
      seq1: [601, 602, 603, 604],
    };

    testSampleTopic1('SimpleSequence',
      sequence_typename,
      sequence_keys,
      sequence_xml,
      jsData);

    done();
  });

  it('enum complex: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;
    let typeSupport = new ddstopic.TypeSupport(
      enum_typename,
      enum_keys,
      enum_xml
    );

    let ColorA = typeSupport.getClass('basic::module_Enum2::Color');
    let ColorZ = typeSupport.getClass('basic::enumZ::Color');
    expect(ColorA).is.not.null;
    expect(ColorZ).is.not.null;

    let jsData5 = {
      long1: 16,
      color1: ColorA.get('Green').value,
      color2: ColorZ.get('Blue').value,
      array: [ColorZ.get('Red').value, ColorZ.get('Blue').value],
    };
    testSampleTopic2(typeSupport, 'EnumComplex', jsData5);

    done();
  });

  it('simple message: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    // simple message
    let jsData = {userID: 7, message: 16};
    testSampleTopic1('Msg', simple_typename, simple_keys, simple_xml, jsData);

    done();
  });

  it('simple types: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    // simple types
    let jsData2 = {
      long1: 1,
      ulong1: 2,
      longlong1: 3,
      ulonglong1: 4,
      float1: 5,
      short1: 6,
      ushort1: 7,
      char1: 8,
      octet1: 9,
      double1: 10,
      bool1: true,
      string1: 'erin',
    };
    testSampleTopic1(
      'SimpleTypes',
      simpleTypes_typename,
      simpleTypes_keys,
      simpleTypes_xml,
      jsData2
    );

    done();
  });

  it('nested: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    // nested
    let jsData3 = {id: 6, inner: {foo: 'qrst'}};
    testSampleTopic1(
      'Nested',
      nestedStruct_typename,
      nestedStruct_keys,
      nestedStruct_xml,
      jsData3
    );

    done();
  });

  it('enum: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    let enumTypeSupport = new ddstopic.TypeSupport(
      enum_typename2,
      enum_keys2,
      enum_xml2
    );
    let enumA = enumTypeSupport.getClass('AGraph::AEnumType');
    expect(enumA).is.not.null;

    let jsData4 = {
      id: 16,
      robot_id: 42,
      type: enumA.get('S_AREA').value,
    };
    testSampleTopic2(enumTypeSupport, 'EnumA', jsData4);

    done();
  });

  it('simple array: test writing and reading topic types', function(done) {

    expect(dp.handle).to.not.be.null;

    // simple message
    let jsData = {long1: 16, array1: [7, 8, 2014]};
    testSampleTopic1('SimpleArray',
      array_typename,
      array_keys,
      array_xml,
      jsData);

    done();
  });

  it('IDLPP create Chat.idl topic', function(done) {

    const idlName = 'test_data' + path.sep + 'Chat.idl';
    const idl = path.resolve(idlName);

    ddstopic.getTopicTypeSupportsForIDL(idl).then((result) => {

      expect(result instanceof Map).to.be.true;
      expect(result.size).to.be.equal(3);

      let typeSupport;

      typeSupport = result.get('Chat::ChatMessage');
      expect(typeSupport instanceof ddstopic.TypeSupport).to.be.true;
      dp.createTopicFor('ChatMessageTest', typeSupport);

      typeSupport = result.get('Chat::NameService');
      expect(typeSupport instanceof ddstopic.TypeSupport).to.be.true;
      dp.createTopicFor('NameServiceTest', typeSupport);

      typeSupport = result.get('Chat::NamedMessage');
      expect(typeSupport instanceof ddstopic.TypeSupport).to.be.true;
      dp.createTopicFor('NamedMessageTest', typeSupport);

      done();
    });

  });


});
