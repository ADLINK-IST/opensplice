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
// const Enum = require('enum');
// const ref = require('ref');
const cico = require('./cico');
const ET = require('elementtree');
ET.XML;
const _SWITCHTYPE_TAG = 'SwitchType';
const _CASE_TAG = 'Case';
const _NAME_ATTRIBUTE = 'name';
const _VALUE_ATTRIBUTE = 'value';
const _TYPE_TAG = 'Type';
const _LABEL_TAG = 'Label';
// const _ENUM_TAG = 'Enum';

const _MODULE_SEPARATOR = '::';

module.exports.generateUnionClass = function(element, module, typeSupport){

  const MyUnion = class extends cico.Union {

    constructor(init_obj) {
      super();
      this._setData(init_obj);
    }

    _setData(init_obj){ // TODO
      if (typeof init_obj === 'undefined') {
        // this.ui8 = 0;
        // do nothing
      } else if (Object.keys(init_obj).length === 1) {
        const case_name = Object.keys(init_obj)[0];
        let types = this.constructor['v.types'];
        if (types.get(case_name) !== undefined) {
          this[case_name] = init_obj[case_name];
        } else {
          throw new TypeError('argument property must be a valid case name');
        }
      } else {
        throw new TypeError('argument has have one property, ' +
            'which must be a case name');
      }
    }

    toJSON(arg_ignored) {
      const obj = {};
      const cases = this.constructor['d.cases'];
      const case_name = cases.get(this._d);
      obj[case_name] = this[case_name];
      return obj;
    }

    get discriminator() {
      return this._d;
    }

  };

  _processXMLElement(element, module, typeSupport, MyUnion);

  return MyUnion;

};

function _processXMLElement(element, module, typeSupport, MyUnion){

  let _EnumType = null;
  let _cases = new Map();
  let _types = new Map();

  /* let key = element.attrib[_NAME_ATTRIBUTE];
  if (module != null && module !== ''){
    key = module + _MODULE_SEPARATOR + key;
  }*/

  for (let child of element.getchildren()){
    if (child.tag === _SWITCHTYPE_TAG){
      if (child.getItem(0).tag === _TYPE_TAG){
        let typeName = child.getItem(0).attrib[_NAME_ATTRIBUTE];
        if ((module !== null)
          && (module !== '')
          && (!typeName.includes(_MODULE_SEPARATOR))){
          typeName = module + _MODULE_SEPARATOR + typeName;
        }
        let referenceType = typeSupport.getClass(typeName);
        _EnumType = referenceType;
        // TODO check that is is a valid enum
      }
    } else if (child.tag === _CASE_TAG){
      // for each case we need to:
      // add a getter and setter
      // add entry to _cases
      // add entry to _types

      let caseName = child.attrib[_NAME_ATTRIBUTE]; // "ui8"
      let caseType;
      let caseEnum;

      for (let subChild of child.getchildren()){
        let tag = subChild.tag;

        if (tag === _TYPE_TAG){
          let name = subChild.attrib[_NAME_ATTRIBUTE];
          if ((module !== null)
                && (module !== '')
                && (!name.includes(_MODULE_SEPARATOR))){
            name = module + _MODULE_SEPARATOR + name;
          }
          caseType = typeSupport.getClass(name);
        } else if (tag === _LABEL_TAG){
          let valueName = subChild.attrib[_VALUE_ATTRIBUTE];
          caseEnum = _EnumType.get(valueName);
        }

      }

      // TODO check that we have a case name, type, and label
      // for each case we need to :
      //   add a getter and setter
      //   add entry to _cases
      //   add entry to _types

      _cases.set(caseEnum, caseName);
      _types.set(caseName, caseType);

      Object.defineProperty(MyUnion.prototype, caseName, {
        get() {
          if (this._d !== caseEnum) {
            throw ReferenceError('not selected by discriminator');
          } else {
            return this._v;
          }
        },
        set(v) {
          this._d = caseEnum;
          this._v = v;
        },
      });
    }
  }

  MyUnion['d.type'] = _EnumType;
  MyUnion['v.types'] = _types;
  MyUnion['d.cases'] = _cases;

}
