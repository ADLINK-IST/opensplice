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
const ref = require('ref');
const cico = require('./cico');

const u_factory = require('./union_factory');
const Enum = require('enum');
const ET = require('elementtree');
ET.XML;
const { spawn } = require('child_process');

const _MODULE_TAG = 'Module';
const _TYPEDEF_TAG = 'TypeDef';
const _STRUCT_TAG = 'Struct';
const _MEMBER_TAG = 'Member';
const _ARRAY_TAG = 'Array';
const _SEQUENCE_TAG = 'Sequence';
const _TYPE_TAG = 'Type';
const _ENUM_TAG = 'Enum';
const _ELEMENT_TAG = 'Element';
const _UNION_TAG = 'Union';

const _NAME_ATTRIBUTE = 'name';
const _SIZE_ATTRIBUTE = 'size';
const _VALUE_ATTRIBUTE = 'value';

const _MODULE_SEPARATOR = '::';

const _ENUM_TYPE = ref.types.int32;


class TypeSupport {
  constructor(typename, keys, xml) {

    this._reftype;
    this._typeMap = new Map();

    this._typename = typename;
    this._keys = keys;
    this._xml = xml;

    this._init();
  }

  _init(){
    // parse xml metadata
    this._processTopicDescriptor();

    // set reftype
    this._reftype = this._typeMap.get(this._typename);
  }

  getClass(fullClassName){
    return this._typeMap.get(fullClassName);
  }

  getRefType(){
    return this._reftype;
  }

  getTypename(){
    return this._typename;
  }

  getKeys(){
    return this._keys;
  }

  getXML(){
    return this._xml;
  }

  _processTopicDescriptor() {

    let rootTree = ET.parse(this._xml);
    let rootElement = rootTree.getroot();

    this._processTopicDescriptorElement(rootElement, '');

  }

  _processTopicDescriptorElement(element, module) {
    if (element.tag === 'MetaData'){

      for (let child of element.getchildren()){
        this._processTopicDescriptorElement(child, module);
      }

    } else if (element.tag === _MODULE_TAG) {

      let newModule = element.attrib[_NAME_ATTRIBUTE];
      if (module !== null && module !== ''){
        newModule = module + _MODULE_SEPARATOR + newModule;
      }
      for (let child of element.getchildren()){
        this._processTopicDescriptorElement(child, newModule);
      }

    } else if (element.tag === _ENUM_TAG) {

      this._processEnumElement(element, module);

    } else if (element.tag === _TYPEDEF_TAG) {

      this._processTypeDefElement(element, module);

    } else if (element.tag === _STRUCT_TAG) {

      this._processStructElement(element, module);

    } else if (element.tag === _UNION_TAG) {

      this._processUnionElement(element, module);
    }
  };

  _processEnumElement(element, module){

    let enumJSOjb = {};
    let key = element.attrib[_NAME_ATTRIBUTE];
    let anEnum;

    if (module != null && module !== ''){
      key = module + _MODULE_SEPARATOR + key;
    }

    // iterate through enum children
    for (let child of element.getchildren()){
      if (child.tag === _ELEMENT_TAG){
        let enumLiteralName = child.attrib[_NAME_ATTRIBUTE];
        let enumLiteralValue = child.attrib[_VALUE_ATTRIBUTE];
        enumJSOjb[enumLiteralName] = parseInt(enumLiteralValue, 10);
      }
    }

    anEnum = new Enum(enumJSOjb);
    this._typeMap.set(key, anEnum);
  }

  _processTypeDefElement(element, module){

    let key = element.attrib[_NAME_ATTRIBUTE];
    let child = element.getItem(0);

    if (module != null && module !== ''){
      key = module + _MODULE_SEPARATOR + key;
    }

    if (child.tag === _TYPE_TAG){
      let name = child.attrib[_NAME_ATTRIBUTE];
      let referenceType;
      if ((!name.includes(_MODULE_SEPARATOR))
        && (module != null)
        && module !== ''){
        name = module + _MODULE_SEPARATOR + name;
      }
      referenceType = this._typeMap.get(name);
      this._typeMap.set(key, referenceType);

    } else if (child.tag === _SEQUENCE_TAG){
      let seqTypeElement = child.getItem(0);
      let referenceType;
      let memberType;
      if (seqTypeElement.tag === _TYPE_TAG){
        // TODO  handle array types
        let typeName = seqTypeElement.attrib[_NAME_ATTRIBUTE];
        if ((module !== null)
            && (module !== '')
            && (!typeName.includes(_MODULE_SEPARATOR))){
          typeName = module + _MODULE_SEPARATOR + typeName;
        }
        referenceType = this._typeMap.get(typeName);
        if (referenceType instanceof Enum){
          memberType = this._getSequenceType(_ENUM_TYPE);
        } else {
          memberType = this._getSequenceType(referenceType);
        }
      } else {
        // type is a primitive ex. long
        let sequenceType =
            this._getPrimitiveRefTypeFor(seqTypeElement.tag);
        memberType = this._getSequenceType(sequenceType);
      }
      this._typeMap.set(key, memberType);

    } else if (child.tag === _ARRAY_TAG){
      // TODO

    } else {
      // primitive types
      let primitiveType = this._getPrimitiveRefTypeFor(child.tag);
      this._typeMap.set(key, primitiveType);
    }

  }

  _processStructElement(element, module){

    let jsObj = {};
    let key = element.attrib[_NAME_ATTRIBUTE];

    if (module != null && module !== ''){
      key = module + _MODULE_SEPARATOR + key;
    }

    // iterate through struct children
    for (let child of element.getchildren()){
      if (child.tag !== _MEMBER_TAG){
        continue;
      }

      let memberName = child.attrib[_NAME_ATTRIBUTE];
      let memberType = null;

      // Handle inner struct types
      if (child.getItem(0).tag === _TYPE_TAG){
        let typeName = child.getItem(0).attrib[_NAME_ATTRIBUTE];

        if ((module !== null)
          && (module !== '')
          && (!typeName.includes(_MODULE_SEPARATOR))){
          typeName = module + _MODULE_SEPARATOR + typeName;
        }
        let referenceType = this._typeMap.get(typeName);
        if (referenceType instanceof Enum){
          memberType = _ENUM_TYPE;
        } else {
          memberType = referenceType;
        }

      } else if (child.getItem(0).tag === _ARRAY_TAG){
        let arrayElement = child.getItem(0);
        let size = arrayElement.attrib[_SIZE_ATTRIBUTE];
        let length = parseInt(size, 10);
        let arrayTypeElement = arrayElement.getItem(0);

        if (arrayTypeElement.tag === _TYPE_TAG){
          // TODO  handle array types
          let typeName = arrayTypeElement.attrib[_NAME_ATTRIBUTE];
          if ((module !== null)
              && (module !== '')
              && (!typeName.includes(_MODULE_SEPARATOR))){
            typeName = module + _MODULE_SEPARATOR + typeName;
          }
          let referenceType = this._typeMap.get(typeName);
          if (referenceType instanceof Enum){
            memberType = new cico.Array(_ENUM_TYPE, length);
          } else {
            memberType = new cico.Array(referenceType, length);
          }
        } else {
          // type is a primitive ex. long
          memberType = this._getArrayType(arrayTypeElement.tag, length);
        }

      } else if (child.getItem(0).tag === _SEQUENCE_TAG){
        let seqElement = child.getItem(0);
        let seqTypeElement = seqElement.getItem(0);
        if (seqTypeElement.tag === _TYPE_TAG){
          // TODO  handle array types
          let typeName = seqTypeElement.attrib[_NAME_ATTRIBUTE];
          if ((module !== null)
            && (module !== '')
            && (!typeName.includes(_MODULE_SEPARATOR))){
            typeName = module + _MODULE_SEPARATOR + typeName;
          }
          let referenceType = this._typeMap.get(typeName);
          if (referenceType instanceof Enum){
            memberType = this._getSequenceType(_ENUM_TYPE);
          } else {
            memberType = this._getSequenceType(referenceType);
          }
        } else {
          // type is a primitive ex. long
          let sequenceType =
              this._getPrimitiveRefTypeFor(seqTypeElement.tag);
          memberType = this._getSequenceType(sequenceType);
        }
        this._typeMap.set(key, memberType);

      } else if (child.getItem(0).tag != null){
        memberType = this._getPrimitiveRefTypeFor(child.getItem(0).tag);
      }

      jsObj[memberName] = memberType;

    }
    // using json object as argument,
    // create new cico.Type class
    let cicoType = new cico.Type(jsObj);
    this._typeMap.set(key, cicoType);

  }

  _processUnionElement(element, module){
    let key = element.attrib[_NAME_ATTRIBUTE];
    if (module != null && module !== ''){
      key = module + _MODULE_SEPARATOR + key;
    }
    // console.log('processUnionElement: ' + key);

    let union = u_factory.generateUnionClass(element, module, this);
    this._typeMap.set(key, union);
  }

  _getArrayType(idlTypeString, length){
    let type = this._getPrimitiveRefTypeFor(idlTypeString);
    return new cico.Array(type, length);
  }

  _getSequenceType(seqType){
    return new cico.Sequence(seqType);
  }

  _getPrimitiveRefTypeFor(idlTypeString){

    let refType;
    let typeString = idlTypeString.toLowerCase();

    switch (typeString) {
      case 'char':
        refType = ref.types.int8;
        break;
      case 'octet':
        refType = ref.types.uint8;
        break;
      case 'short':
        refType = ref.types.int16;
        break;
      case 'ushort':
        refType = ref.types.uint16;
        break;
      case 'long':
        refType = ref.types.int32;
        break;
      case 'ulong':
        refType = ref.types.uint32;
        break;
      case 'longlong':
        refType = ref.types.int64;
        break;
      case 'ulonglong':
        refType = ref.types.uint64;
        break;
      case 'float':
        refType = ref.types.float;
        break;
      case 'double':
        refType = ref.types.double;
        break;
      case 'string':
        refType = ref.types.CString;
        break;
      case 'boolean':
        refType = ref.types.bool;
        break;
      default:
        refType = ref.types.int32;
    }

    return refType;
  }

  copyin(jsobj) {
    return this._reftype.copyin(jsobj);
  }

  copyout(buf) {
    return this._reftype.copyout(buf);
  }

};

module.exports.TypeSupport = TypeSupport;

/**
  * Given an absolute path to an idl file,
  * processes the idl file using IDLPP.
  * Function uses a child process, and
  * therefore must be called asynchrously.
  *
  * Inside promise:
  * IDLPP is called to process idl file.
  * A TypeSupport object is created for
  * each topic in the IDL file and is added to a Map.
  * The Map key is the full topic name, and the value is
  * the topic TypeSupport object.
  *
  * @param {string} idlpath absolute path to idl file
  * @returns {Promise}
  */
module.exports.getTopicTypeSupportsForIDL = function(idlPath){

  const promise = new Promise(function(resolve, reject) {

    const childProcess = spawn('idlpp', ['-l', 'pythondesc', idlPath]);

    var result = '';
    childProcess.stdout.on('data', function(data) {
      result += data.toString();
    });

    childProcess.stderr.on('data', (data) => {
      reject(`stderr: ${data}`);
    });

    childProcess.on('close', function(code) {
      if (code !== 0){
        throw new Error(result);
      }
      let typeSupportMap = processIDLPP_XML(result);
      return resolve(typeSupportMap);
    });

  });

  return promise;
};

function processIDLPP_XML(idlppXML){

  let typeSupportMap = new Map();
  let etree;

  try {
    etree = ET.parse(idlppXML);
  } catch (error){
    return typeSupportMap;
  }

  let topictypeArray = etree.findall('topictype');
  for (let i = 0; i < topictypeArray.length; i++) {
    let typeSupport = processTopicTypeElement(topictypeArray[i]);
    typeSupportMap.set(typeSupport.getTypename(), typeSupport);
  }

  return typeSupportMap;

};

function processTopicTypeElement(topictype){

  const typeName = topictype.findtext('id');
  const keys = topictype.findtext('keys');
  const xml = topictype.findtext('descriptor');

  const typeSupport = new TypeSupport(typeName, keys, xml);

  return typeSupport;

};


