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
const cico = require('./cico');
const ref = require('ref');
const Enum = require('enum');

describe('representing dds types', function() {

  it('describes a struct with basic types', function() {
    const type = new cico.Type({
      i8: ref.types.int8, // 0
      u8: ref.types.uint8, // 1
      i16: ref.types.int16, // 2
      u16: ref.types.uint16, // 4
      i32: ref.types.int32, // 8
      u32: ref.types.uint32, // 16
      i64: ref.types.int64, // 24
      u64: ref.types.uint64, // 32
      f32: ref.types.float, // 40
      f64: ref.types.double, // 48
      b: ref.types.bool, // 56
    });
    const expectedInfo = {
      i8: { offset: 0, size: 1, align: 1 },
      u8: { offset: 1, size: 1, align: 1 },
      i16: { offset: 2, size: 2, align: 2 },
      u16: { offset: 4, size: 2, align: 2 },
      i32: { offset: 8, size: 4, align: 4 },
      u32: { offset: 12, size: 4, align: 4 },
      i64: { offset: 16, size: 8, align: 8 },
      u64: { offset: 24, size: 8, align: 8 },
      f32: { offset: 32, size: 4, align: 4 },
      f64: { offset: 40, size: 8, align: 8 },
      b: { offset: 48, size: 1, align: 1 },
    };

    expect(type.fields).to.be.length(11);
    for (var f of type.fields) {
      expect(f).to.have.property('size', expectedInfo[f.name].size);
      expect(f).to.have.property('alignment', expectedInfo[f.name].align);
      expect(f).to.have.property('offset', expectedInfo[f.name].offset);
    }

    expect(type.size).to.be.equal(56);
  });

  it('describes a struct with string fields', function() {
    const type = new cico.Type({
      i8: ref.types.int8, // 0
      s: ref.types.CString, // 8
      b: ref.types.bool, // 16
    });
    const expectedInfo = {
      i8: { offset: 0, size: 1, align: 1 },
      s: { offset: 8, size: 8, align: 8 },
      b: { offset: 16, size: 1, align: 1 },
    };
    expect(type.fields).to.be.length(3);
    for (var f of type.fields) {
      expect(f).to.have.property('size', expectedInfo[f.name].size);
      expect(f).to.have.property('alignment', expectedInfo[f.name].align);
      expect(f).to.have.property('offset', expectedInfo[f.name].offset);
    }

    expect(type.size).to.be.equal(24);
  });

  it('describes a struct with an array field', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(ref.types.int32, 4),
      e: ref.types.bool,
    });
    const expectedInfo = {
      b: {offset: 0, size: 1, align: 1},
      a: {offset: 4, size: 16, align: 4},
      e: {offset: 20, size: 1, align: 1},
    };
    expect(type.fields).to.be.length(3);
    for (var f of type.fields) {
      expect(f).to.have.property('size', expectedInfo[f.name].size);
      expect(f).to.have.property('alignment', expectedInfo[f.name].align);
      expect(f).to.have.property('offset', expectedInfo[f.name].offset);
    }
    expect(type.size).to.be.equal(24);
  });

  it('describes a sequence field', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      su: new cico.Sequence(ref.types.int32),
      sb: new cico.Sequence(ref.types.int32, 2),
      e: ref.types.bool,
    });
    const expectedInfo = {
      b: {offset: 0, size: 1, align: 1},
      su: {offset: 8, size: 24, align: 8},
      sb: {offset: 32, size: 24, align: 8},
      e: {offset: 56, size: 1, align: 1},
    };
    expect(type.fields).to.be.length(4);
    for (var f of type.fields) {
      expect(f).to.have.property('size', expectedInfo[f.name].size);
      expect(f).to.have.property('alignment', expectedInfo[f.name].align);
      expect(f).to.have.property('offset', expectedInfo[f.name].offset);
    }
    expect(type.size).to.be.equal(64);

  });

  it('describes an enum field', function() {
    const colors = new Enum({None: 0, Black: 1, Red: 2,
      Red2: 3, Green: 4, Blue: 5});
    const type = new cico.Type({
      b: ref.types.bool,
      c: colors,
      e: ref.types.bool,
    });
    const expectedInfo = {
      b: {offset: 0, size: 1, align: 1},
      c: {offset: 4, size: 4, align: 4},
      e: {offset: 8, size: 1, align: 1},
    };
    expect(type.fields).to.be.length(3);
    for (var f of type.fields) {
      expect(f).to.have.property('size', expectedInfo[f.name].size);
      expect(f).to.have.property('alignment', expectedInfo[f.name].align);
      expect(f).to.have.property('offset', expectedInfo[f.name].offset);
    }
    expect(type.size).to.be.equal(12);
  });

  it('throws an error on unsupported types', function() {
    expect(function() {
      const type = new cico.Type({ // eslint-disable-line no-unused-vars
        b: ref.types.bool,
        u: {},
        e: ref.types.bool,
      });
    }).to.throw(Error);
  });
});

describe('copy in values to buffer', function() {

  it('copies basic types', function() {
    const type = new cico.Type({
      i8: ref.types.int8, // 0
      u8: ref.types.uint8, // 1
      i16: ref.types.int16, // 2
      u16: ref.types.uint16, // 4
      i32: ref.types.int32, // 8
      u32: ref.types.uint32, // 16
      i64: ref.types.int64, // 24
      u64: ref.types.uint64, // 32
      f32: ref.types.float, // 40
      f64: ref.types.double, // 48
      b: ref.types.bool, // 56
    });

    const buf = new Buffer(type.size);

    type._copyin(buf, {
      i8: 0x7f,
      u8: 0xef,
      i16: 0x0102,
      u16: 0x0304,
      i32: 0x01020304,
      u32: 0x05060708,
      i64: 0x0002030405060708,
      u64: 0x0092939495969798,
      f32: 3.125,
      f64: 6.25,
      b: true,
    });

    const expected = new Buffer(new Uint8Array([
      0x7f, 0xef, // i8, u8
      0x02, 0x01, 0x04, 0x03, // i16, u16
      0, 0, // padding
      0x04, 0x03, 0x02, 0x01, 0x08, 0x07, 0x06, 0x05, // i32, u32
      0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x00, // i64
      0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x00, // u64
      0, 0, 72, 64, // f32
      0, 0, 0, 0, // padding
      0, 0, 0, 0, 0, 0, 25, 64, // f64
      0x01,
      0, 0, 0, 0, 0, 0, 0, // fill (7)
    ]));

    expect(buf).to.deep.equal(expected);
  });

  it('copies nested structs', function() {
    const nested = new cico.Type({
      b: ref.types.bool,
      l: ref.types.int32,
      e: ref.types.bool,
    });
    const outer = new cico.Type({
      b: ref.types.bool,
      n: nested,
      ll: ref.types.int64,
    });

    const data = {
      b: true,
      n: {
        b: true,
        l: 0x01020304,
        e: true,
      },
      ll: 0x0001020304050607,
    };
    const buf = new Buffer(outer.size);
    outer._copyin(buf, data, 0);

    const expected = new Uint8Array([
      1, // b
      0, 0, 0, // padding
      1, // n.b
      0, 0, 0,
      4, 3, 2, 1, // n.l
      1, // n.e
      0, 0, 0,
      7, 6, 5, 4, 3, 2, 1, 0, // ll
    ]);

    expect(buf.buffer).to.deep.equal(expected.buffer);
  });

  it('copies a struct with strings', function() {
    const type = new cico.Type({
      i8: ref.types.int8, // 0
      s: ref.types.CString, // 8
      b: ref.types.bool, // 16
    });

    const data = {
      i8: 1,
      s: 'can we save this string?',
      b: true,
    };
    const cleanup_list = [];
    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0, cleanup_list);

    expect(cleanup_list).to.be.length(1);
    const s = ref.readCString(cleanup_list[0]);
    expect(s).to.equal(data.s);
  });

  it('it copies a nested struct with strings', function() {
    const inner = new cico.Type({
      b: ref.types.bool,
      s: ref.types.CString,
      e: ref.types.bool,
    });
    const outer = new cico.Type({
      b: ref.types.bool,
      i: inner,
      e: ref.types.bool,
    });

    const data = {
      b: true,
      i: {
        b: true,
        s: 'can you hold onto this, please',
        e: true,
      },
      e: true,
    };

    const buf = Buffer(outer.size);
    const cleanup_list = [];

    outer._copyin(buf, data, 0, cleanup_list);

    expect(cleanup_list).to.be.length(1);
    const s = ref.readCString(cleanup_list[0], 0);
    expect(s).to.equal(data.i.s);

  });

  it('copies an integer array', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(ref.types.int32, 4),
      e: ref.types.bool,
    });

    const data = {
      b: true,
      a: [0x11121314, 0x21222324, 0x31323334, 0x41424344],
      e: true,
    };

    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0);

    const expected = new Uint8Array([
      1, // b
      0, 0, 0, // padding
      0x14, 0x13, 0x12, 0x11,
      0x24, 0x23, 0x22, 0x21,
      0x34, 0x33, 0x32, 0x31,
      0x44, 0x43, 0x42, 0x41,
      1,
      0, 0, 0,
    ]);

    expect(buf.buffer).to.deep.equal(expected.buffer);

  });

  it('copies a 2x3 integer array', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(new cico.Array(ref.types.int32, 3), 2),
      e: ref.types.bool,
    });

    const data = {
      b: true,
      a: [[0x11110001, 0x11110002, 0x11110003],
        [0x22220001, 0x22220002, 0x22220003]],
      e: true,
    };

    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0);

    const expected = new Uint8Array([
      1, // b
      0, 0, 0, // padding
      0x01, 0x00, 0x11, 0x11,
      0x02, 0x00, 0x11, 0x11,
      0x03, 0x00, 0x11, 0x11,
      0x01, 0x00, 0x22, 0x22,
      0x02, 0x00, 0x22, 0x22,
      0x03, 0x00, 0x22, 0x22,
      1,
      0, 0, 0,
    ]);

    expect(buf.buffer).to.deep.equal(expected.buffer);

  });

  it('copies a string array', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(ref.types.CString, 2),
      e: ref.types.bool,
    });

    const data = {
      b: true,
      a: ['hello', 'this is a long message'],
      e: true,
    };

    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);

    expect(cleanup_list).to.be.length(2);
    for (var i = 0; i < 2; i++) {
      expect(ref.readCString(cleanup_list[i])).to.equal(data.a[i]);
    }

  });

  it('copies an incomplete array', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(ref.types.int32, 4),
      e: ref.types.bool,
    });

    const data = {
      b: true,
      a: [0x11121314, 0x21222324, 0x31323334 ],
      e: true,
    };

    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0);

    const expected = new Uint8Array([
      1, // b
      0, 0, 0, // padding
      0x14, 0x13, 0x12, 0x11,
      0x24, 0x23, 0x22, 0x21,
      0x34, 0x33, 0x32, 0x31,
      0, 0, 0, 0,
      1,
      0, 0, 0,
    ]);

    expect(buf.buffer).to.deep.equal(expected.buffer);
  });

  it('copies a sparse integer array', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(ref.types.int32, 4),
      e: ref.types.bool,
    });
    const sparse = [];
    sparse[0] = 0x11121314;
    sparse[3] = 0x41424344;
    const data = {
      b: true,
      a: sparse,
      e: true,
    };

    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0);

    const expected = new Uint8Array([
      1, // b
      0, 0, 0, // padding
      0x14, 0x13, 0x12, 0x11,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0x44, 0x43, 0x42, 0x41,
      1,
      0, 0, 0,
    ]);

    expect(buf.buffer).to.deep.equal(expected.buffer);

  });

  it('copies a too-long integer array', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      a: new cico.Array(ref.types.int32, 4),
      e: ref.types.bool,
    });

    const data = {
      b: true,
      a: [0x11121314, 0x21222324, 0x31323334, 0x41424344, 0x51525354],
      e: true,
    };

    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0);

    const expected = new Uint8Array([
      1, // b
      0, 0, 0, // padding
      0x14, 0x13, 0x12, 0x11,
      0x24, 0x23, 0x22, 0x21,
      0x34, 0x33, 0x32, 0x31,
      0x44, 0x43, 0x42, 0x41,
      1,
      0, 0, 0,
    ]);

    expect(buf.buffer).to.deep.equal(expected.buffer);

  });

  it('writes a sequence', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      su: new cico.Sequence(ref.types.int32),
      e: ref.types.bool,
    });
    const data = {
      b: true,
      su: [0x11111111, 0x22222222],
      e: true,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);

    const expected = new Uint8Array([
      0x11, 0x11, 0x11, 0x11,
      0x22, 0x22, 0x22, 0x22,
    ]);
    expect(cleanup_list).to.be.length(1);
    expect(cleanup_list[0].buffer).to.deep.equal(expected.buffer);
    const l = ref.types.uint32.get(buf, 8);
    expect(l).to.be.equal(2);
    const mx = ref.types.uint32.get(buf, 12);
    expect(mx).to.be.equal(2);
    const pb = ref.readPointer(buf, 16, 2 * ref.types.int32.size);
    expect(pb.buffer).to.deep.equal(expected.buffer);
  });

  it('writes a too long sequence', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      su: new cico.Sequence(ref.types.int32, 2),
      e: ref.types.bool,
    });
    const data = {
      b: true,
      su: [0x11111111, 0x22222222, 0x33333333],
      e: true,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);

    const expected = new Uint8Array([
      0x11, 0x11, 0x11, 0x11,
      0x22, 0x22, 0x22, 0x22,
    ]);
    expect(cleanup_list).to.be.length(1);
    expect(cleanup_list[0].buffer).to.deep.equal(expected.buffer);
    const l = ref.types.uint32.get(buf, 8);
    expect(l).to.be.equal(2);
    const mx = ref.types.uint32.get(buf, 12);
    expect(mx).to.be.equal(2);
    const pb = ref.readPointer(buf, 16, 2 * ref.types.int32.size);
    expect(pb.buffer).to.deep.equal(expected.buffer);
  });

  it('copies an enum value', function() {
    const colors = new Enum({None: 0, Black: 1, Red: 2,
      Red2: 3, Green: 4, Blue: 5});
    const type = new cico.Type({
      b: ref.types.bool,
      c: colors,
      e: ref.types.bool,
    });
    const data = {
      b: true,
      c: colors.Red,
      e: true,
    };
    const buf = new Buffer(type.size);
    type._copyin(buf, data, 0);
    const expected = new Uint8Array([
      1,
      0, 0, 0,
      2, 0, 0, 0,
      1,
      0, 0, 0,
    ]);
    expect(buf.buffer).to.deep.equal(expected.buffer);
  });

  it('simplifies outer struct copy', function() {
    const inner = new cico.Type({
      b: ref.types.int8,
      ss: new cico.Sequence(ref.types.CString),
      e: ref.types.int8,
    });
    const type = new cico.Type({
      b: ref.types.int8,
      i: inner,
      e: ref.types.int8,
    });

    const obj = {
      b: 1,
      i: {
        b: 3,
        ss: ['one un uno', 'two deux dos'],
        e: 4,
      },
      e: 2,
    };
    const buf = type.copyin(obj);
    expect(buf).to.be.an.instanceof(Buffer);
    expect(buf).to.have.property('_refs').of.length(3);
  });

});

describe('JS object initilization from dds type', function() {
  it('creates basic types', function() {
    const type = new cico.Type({
      i8: ref.types.int8,
      u8: ref.types.uint8,
      i16: ref.types.int16,
      u16: ref.types.uint16,
      i32: ref.types.int32,
      u32: ref.types.uint32,
      i64: ref.types.int64,
      u64: ref.types.uint64,
      f32: ref.types.float,
      f64: ref.types.double,
      b: ref.types.bool,
    });

    const data = type.newInstance();

    expect(data).to.deep.equal({
      i8: 0,
      u8: 0,
      i16: 0,
      u16: 0,
      i32: 0,
      u32: 0,
      i64: 0,
      u64: 0,
      f32: 0.0,
      f64: 0.0,
      b: false,
    });
  });

  it('creates string types', function() {
    const type = new cico.Type({
      s: ref.types.CString,
    });
    const data = type.newInstance();

    expect(data).to.deep.equal({
      s: '',
    });
  });

  it('creates enum types', function(){
    const colors = new Enum({None: 0, Black: 1, Red: 2,
      Red2: 3, Green: 4, Blue: 5});
    const type = new cico.Type({
      c: colors,
    });
    const data = type.newInstance();

    expect(data).to.deep.equal({
      c: colors.None,
    });
  });

  it('creates nested structs', function() {
    const inner = new cico.Type({
      i: ref.types.int32,
      s: ref.types.CString,
    });
    const type = new cico.Type({
      b: ref.types.bool,
      i: inner,
      e: ref.types.bool,
    });
    const data = type.newInstance();

    expect(data).to.deep.equal({
      b: false,
      i: {
        i: 0,
        s: '',
      },
      e: false,
    });
  });

  it('creates default arrays', function() {
    const type = new cico.Type({
      a: new cico.Array(ref.types.int32, 2),
      m23: new cico.Array(new cico.Array(ref.types.int32, 3), 2),
    });
    const data = type.newInstance();

    expect(data).to.deep.equal({
      a: [0, 0],
      m23: [[0, 0, 0], [0, 0, 0]],
    });
  });

  it('creates default sequences', function() {
    const type = new cico.Type({
      sq: new cico.Sequence(ref.types.int32),
    });
    const data = type.newInstance();

    expect(data).to.deep.equal({
      sq: [],
    });
  });
});

describe('read values from buffer', function() {
  it('reads basic types', function(){
    const type = new cico.Type({
      i8: ref.types.int8, // 0
      u8: ref.types.uint8, // 1
      i16: ref.types.int16, // 2
      u16: ref.types.uint16, // 4
      i32: ref.types.int32, // 8
      u32: ref.types.uint32, // 16
      i64: ref.types.int64, // 24
      u64: ref.types.uint64, // 32
      // Note: hard to test floats, because you have to
      // do fuzzy comparisons. Use powers of 2 in
      // fractional values. E.g 0.5, 0.25, 0.125, ...
      f32: ref.types.float, // 40
      f64: ref.types.double, // 48
      b: ref.types.bool, // 56
    });
    const data = {
      i8: 13,
      u8: 14,
      i16: 15,
      u16: 16,
      i32: 17,
      u32: 18,
      i64: 19,
      u64: 20,
      f32: 3.125,
      f64: 6.25,
      b: false,
    };
    const buf = new Buffer(type.size);
    type._copyin(buf, data);
    const data_out = type.copyout(buf);

    expect(data_out).to.deep.equal(data);

  });

  it('reads string types', function() {
    const type = new cico.Type({
      b: ref.types.bool,
      s: ref.types.CString,
      e: ref.types.bool,
    });
    const data = {
      b: true,
      s: 'this is a string',
      e: true,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data_out = type.copyout(buf);

    expect(data_out).to.deep.equal(data);
  });

  it('reads enum types', function() {
    const colors = new Enum({None: 0, Black: 1, Red: 2,
      Red2: 3, Green: 4, Blue: 5});
    const type = new cico.Type({
      b: ref.types.bool,
      c: colors,
      e: ref.types.bool,
    });
    const data = {
      b: true,
      c: colors.Red2,
      e: true,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data_out = type.copyout(buf);

    expect(data_out).to.deep.equal(data);
  });

  it('reads nested structs', function() {
    const inner = new cico.Type({
      b: ref.types.int8,
      s: ref.types.CString,
      e: ref.types.int8,
    });
    const type = new cico.Type({
      b: ref.types.int8,
      i: inner,
      e: ref.types.int8,
    });
    const data = {
      b: 1,
      i: {
        b: 2,
        s: 'this is a string field',
        e: 3,
      },
      e: 4,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads integer arrays', function() {
    const type = new cico.Type({
      b: ref.types.int8,
      a: new cico.Array(ref.types.int32, 3),
      e: ref.types.int8,
    });
    const data = {
      b: 1,
      a: [0x11121314, 0x51525354, 0x71929394],
      e: 2,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads a 2x3 integer array', function() {
    const type = new cico.Type({
      b: ref.types.int8,
      a: new cico.Array(new cico.Array(ref.types.int32, 3), 2),
      e: ref.types.int8,
    });
    const data = {
      b: 1,
      a: [[0x10010203, 0x10040506, 0x10070809],
        [0x20010203, 0x20040506, 0x20070809]],
      e: 2,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads a string array', function() {
    const type = new cico.Type({
      b: ref.types.int8,
      sa: new cico.Array(ref.types.CString, 3),
      e: ref.types.int8,
    });
    const data = {
      b: 1,
      sa: ['hello', 'this is a long one',
        'for you to read'],
      e: 2,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads a struct array', function() {
    const inner = new cico.Type({
      b: ref.types.int8,
      s: ref.types.CString,
      e: ref.types.int8,
    });
    const type = new cico.Type({
      b: ref.types.int8,
      ia: new cico.Array(inner, 3),
      e: ref.types.int8,
    });
    const data = {
      b: 1,
      ia: [
        { b: 3, s: 'one/un/uno', e: 4},
        { b: 5, s: 'two/deux/dos', e: 6},
        { b: 7, s: 'three/trois/tres', e: 8},
      ],
      e: 2,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads an integer sequence', function() {
    const type = new cico.Type({
      b: ref.types.int8,
      si: new cico.Sequence(ref.types.int32),
      e: ref.types.int8,
    });
    const data = {
      b: 1,
      si: [0x10000001, 0x20000002, 0x30000003, 0x40000004],
      e: 5,
    };
    const buf = new Buffer(type.size);
    const cleanup_list = [];
    type._copyin(buf, data, 0, cleanup_list);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads an zero-length sequence', function() {
    const type = new cico.Type({
      b: ref.types.int8,
      si: new cico.Sequence(ref.types.int32),
      e: ref.types.int8,
    });
    const data = {
      b: 0,
      si: [],
      e: 0,
    };
    const buf = new Buffer(type.size);
    const data2 = type.copyout(buf);

    expect(data2).to.deep.equal(data);
  });

  it('reads a sequence of struct', function() {

  });

});
