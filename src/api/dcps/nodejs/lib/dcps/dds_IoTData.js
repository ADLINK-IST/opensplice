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
const Enum = require('enum');
const ref = require('ref');
const cico = require('./cico');

const IoTType = new Enum({
  TYPE_IoTUI8: 0,
  TYPE_IoTUI16: 1,
  TYPE_IoTUI32: 2,
  TYPE_IoTUI64: 3,
  TYPE_IoTI8: 4,
  TYPE_IoTI16: 5,
  TYPE_IoTI32: 6,
  TYPE_IoTI64: 7,
  TYPE_IoTF32: 8,
  TYPE_IoTF64: 9,
  TYPE_IoTB: 10,
  TYPE_IoTStr: 11,
  TYPE_IoTCh: 12,
  TYPE_IoTUI8Seq: 13,
  TYPE_IoTUI16Seq: 14,
  TYPE_IoTUI32Seq: 15,
  TYPE_IoTUI64Seq: 16,
  TYPE_IoTI8Seq: 17,
  TYPE_IoTI16Seq: 18,
  TYPE_IoTI32Seq: 19,
  TYPE_IoTI64Seq: 20,
  TYPE_IoTF32Seq: 21,
  TYPE_IoTF64Seq: 22,
  TYPE_IoTBSeq: 23,
  TYPE_IoTStrSeq: 24,
  TYPE_IoTChSeq: 25,
});

const IoTValue_cases = new Map([
  [IoTType.TYPE_IoTUI8, 'ui8'],
  [IoTType.TYPE_IoTUI16, 'ui16'],
  [IoTType.TYPE_IoTUI32, 'ui32'],
  [IoTType.TYPE_IoTUI64, 'ui64'],
  [IoTType.TYPE_IoTI8, 'i8'],
  [IoTType.TYPE_IoTI16, 'i16'],
  [IoTType.TYPE_IoTI32, 'i32'],
  [IoTType.TYPE_IoTI64, 'i64'],
  [IoTType.TYPE_IoTF32, 'f32'],
  [IoTType.TYPE_IoTF64, 'f64'],
  [IoTType.TYPE_IoTB, 'b'],
  [IoTType.TYPE_IoTStr, 'str'],
  [IoTType.TYPE_IoTCh, 'ch'],
  [IoTType.TYPE_IoTUI8Seq, 'ui8Seq'],
  [IoTType.TYPE_IoTUI16Seq, 'ui16Seq'],
  [IoTType.TYPE_IoTUI32Seq, 'ui32Seq'],
  [IoTType.TYPE_IoTUI64Seq, 'ui64Seq'],
  [IoTType.TYPE_IoTI8Seq, 'i8Seq'],
  [IoTType.TYPE_IoTI16Seq, 'i16Seq'],
  [IoTType.TYPE_IoTI32Seq, 'i32Seq'],
  [IoTType.TYPE_IoTI64Seq, 'i64Seq'],
  [IoTType.TYPE_IoTF32Seq, 'f32Seq'],
  [IoTType.TYPE_IoTF64Seq, 'f64Seq'],
  [IoTType.TYPE_IoTBSeq, 'bSeq'],
  [IoTType.TYPE_IoTStrSeq, 'strSeq'],
  [IoTType.TYPE_IoTChSeq, 'chSeq'],
]);

const IoTValue_types = {
  ui8: ref.types.uint8,
  ui16: ref.types.uint16,
  ui32: ref.types.uint32,
  ui64: ref.types.uint64,
  i8: ref.types.int8,
  i16: ref.types.int16,
  i32: ref.types.int32,
  i64: ref.types.int64,
  f32: ref.types.float,
  f64: ref.types.double,
  b: ref.types.bool,
  str: ref.types.CString,
  ch: ref.types.int8,
  ui8Seq: new cico.Sequence(ref.types.uint8),
  ui16Seq: new cico.Sequence(ref.types.uint16),
  ui32Seq: new cico.Sequence(ref.types.uint32),
  ui64Seq: new cico.Sequence(ref.types.uint64),
  i8Seq: new cico.Sequence(ref.types.int8),
  i16Seq: new cico.Sequence(ref.types.int16),
  i32Seq: new cico.Sequence(ref.types.int32),
  i64Seq: new cico.Sequence(ref.types.int64),
  f32Seq: new cico.Sequence(ref.types.float),
  f64Seq: new cico.Sequence(ref.types.double),
  bSeq: new cico.Sequence(ref.types.bool),
  strSeq: new cico.Sequence(ref.types.CString),
  chSeq: new cico.Sequence(ref.types.int8),
};

class IoTValue extends cico.Union {

  constructor(init_obj) {
    super();
    if (typeof init_obj === 'undefined') {
      this.ui8 = 0;
    } else if (Object.keys(init_obj).length === 1) {
      const case_name = Object.keys(init_obj)[0];
      if (Object.keys(IoTValue_types).indexOf(case_name) !== -1) {
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
    const case_name = IoTValue_cases.get(this._d);
    obj[case_name] = this[case_name];
    return obj;
  }

  get discriminator() {
    return this._d;
  }

  // set discriminator(d) { // no set discriminator necessary

  get ui8() {
    if (this._d !== IoTType.TYPE_IoTUI8) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui16() {
    if (this._d !== IoTType.TYPE_IoTUI16) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui32() {
    if (this._d !== IoTType.TYPE_IoTUI32) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui64() {
    if (this._d !== IoTType.TYPE_IoTUI64) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i8() {
    if (this._d !== IoTType.TYPE_IoTI8) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i16() {
    if (this._d !== IoTType.TYPE_IoTI16) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i32() {
    if (this._d !== IoTType.TYPE_IoTI32) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i64() {
    if (this._d !== IoTType.TYPE_IoTI64) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get f32() {
    if (this._d !== IoTType.TYPE_IoTF32) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get f64() {
    if (this._d !== IoTType.TYPE_IoTF64) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get b() {
    if (this._d !== IoTType.TYPE_IoTB) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get str() {
    if (this._d !== IoTType.TYPE_IoTStr) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ch() {
    if (this._d !== IoTType.TYPE_IoTCh) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui8Seq() {
    if (this._d !== IoTType.TYPE_IoTUI8Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui16Seq() {
    if (this._d !== IoTType.TYPE_IoTUI16Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui32Seq() {
    if (this._d !== IoTType.TYPE_IoTUI32Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get ui64Seq() {
    if (this._d !== IoTType.TYPE_IoTUI64Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i8Seq() {
    if (this._d !== IoTType.TYPE_IoTI8Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i16Seq() {
    if (this._d !== IoTType.TYPE_IoTI16Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i32Seq() {
    if (this._d !== IoTType.TYPE_IoTI32Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get i64Seq() {
    if (this._d !== IoTType.TYPE_IoTI64Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get f32Seq() {
    if (this._d !== IoTType.TYPE_IoTF32Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get f64Seq() {
    if (this._d !== IoTType.TYPE_IoTF64Seq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get bSeq() {
    if (this._d !== IoTType.TYPE_IoTBSeq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get strSeq() {
    if (this._d !== IoTType.TYPE_IoTStrSeq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  get chSeq() {
    if (this._d !== IoTType.TYPE_IoTChSeq) {
      throw ReferenceError('not selected by discriminator');
    } else {
      return this._v;
    }
  }

  set ui8(v) {
    this._d = IoTType.TYPE_IoTUI8;
    this._v = v;
  }

  set ui16(v) {
    this._d = IoTType.TYPE_IoTUI16;
    this._v = v;
  }

  set ui32(v) {
    this._d = IoTType.TYPE_IoTUI32;
    this._v = v;
  }

  set ui64(v) {
    this._d = IoTType.TYPE_IoTUI64;
    this._v = v;
  }

  set i8(v) {
    this._d = IoTType.TYPE_IoTI8;
    this._v = v;
  }

  set i16(v) {
    this._d = IoTType.TYPE_IoTI16;
    this._v = v;
  }

  set i32(v) {
    this._d = IoTType.TYPE_IoTI32;
    this._v = v;
  }

  set i64(v) {
    this._d = IoTType.TYPE_IoTI64;
    this._v = v;
  }

  set f32(v) {
    this._d = IoTType.TYPE_IoTF32;
    this._v = v;
  }

  set f64(v) {
    this._d = IoTType.TYPE_IoTF64;
    this._v = v;
  }

  set b(v) {
    this._d = IoTType.TYPE_IoTB;
    this._v = v;
  }

  set str(v) {
    this._d = IoTType.TYPE_IoTStr;
    this._v = v;
  }

  set ch(v) {
    this._d = IoTType.TYPE_IoTCh;
    this._v = v;
  }

  set ui8Seq(v) {
    this._d = IoTType.TYPE_IoTUI8Seq;
    this._v = v;
  }

  set ui16Seq(v) {
    this._d = IoTType.TYPE_IoTUI16Seq;
    this._v = v;
  }

  set ui32Seq(v) {
    this._d = IoTType.TYPE_IoTUI32Seq;
    this._v = v;
  }

  set ui64Seq(v) {
    this._d = IoTType.TYPE_IoTUI64Seq;
    this._v = v;
  }

  set i8Seq(v) {
    this._d = IoTType.TYPE_IoTI8Seq;
    this._v = v;
  }

  set i16Seq(v) {
    this._d = IoTType.TYPE_IoTI16Seq;
    this._v = v;
  }

  set i32Seq(v) {
    this._d = IoTType.TYPE_IoTI32Seq;
    this._v = v;
  }

  set i64Seq(v) {
    this._d = IoTType.TYPE_IoTI64Seq;
    this._v = v;
  }

  set f32Seq(v) {
    this._d = IoTType.TYPE_IoTF32Seq;
    this._v = v;
  }

  set f64Seq(v) {
    this._d = IoTType.TYPE_IoTF64Seq;
    this._v = v;
  }

  set bSeq(v) {
    this._d = IoTType.TYPE_IoTBSeq;
    this._v = v;
  }

  set strSeq(v) {
    this._d = IoTType.TYPE_IoTStrSeq;
    this._v = v;
  }

  set chSeq(v) {
    this._d = IoTType.TYPE_IoTChSeq;
    this._v = v;
  }
}

IoTValue['union.size'] = 32;
IoTValue['union.alignment'] = 8;
IoTValue['d.type'] = IoTType;
IoTValue['v.types'] = IoTValue_types;
IoTValue['d.cases'] = IoTValue_cases;

module.exports = {
  IoTType: IoTType, // enum
  IoTValue: IoTValue, // iotvalue union
};
