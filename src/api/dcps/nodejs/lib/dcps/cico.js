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
const RefStruct = require('ref-struct');
const Enum = require('enum');

function alignup(offset, alignment) {
  if ((offset % alignment) === 0) {
    return offset;
  } else {
    return offset + (alignment - (offset % alignment));
  }
}
class AbstractField {
  constructor(name) {
    this._name = name;
  }

  get name() {
    return this._name;
  }

  set offset(offset) {
    this._offset = offset;
  }

  get offset() {
    return this._offset;
  }
}

class BasicField extends AbstractField {

  constructor(name, refstruct_type) {
    super(name);
    this._rstype = refstruct_type;
  }

  get size() {
    return this._rstype.size;
  }

  get alignment() {
    return this.size;
  }

  copyin(buffer, value, initial_offset) {
    this._rstype.set(buffer, initial_offset + this._offset, value);
  }

  copyout(buffer, initial_offset) {
    return this._rstype.get(buffer, initial_offset + this._offset);
  }

  defaultValue() {
    if (this._rstype === ref.types.bool) {
      return false;
    } else {
      return 0;
    }
  }
}

class NestedStructField extends AbstractField {

  constructor(name, ddsstruct) {
    super(name);
    this._ddsstruct = ddsstruct;
  }

  get size() {
    return this._ddsstruct.size;
  }

  get alignment() {
    return this._ddsstruct.alignment;
  }

  copyin(buffer, value, initial_offset, cleanup_list) {
    this._ddsstruct._copyin(
      buffer,
      value,
      initial_offset + this.offset,
      cleanup_list
    );
  }

  copyout(buffer, initial_offset) {
    return this._ddsstruct.copyout(
      buffer,
      initial_offset + this.offset
    );
  }

  defaultValue() {
    return this._ddsstruct.newInstance();
  }
}

class StringField extends AbstractField {

  constructor(name, limit = 0) {
    super(name);
    this._rstype = ref.types.CString;
  }

  get size() {
    return this._rstype.size;
  }

  get alignment() {
    return this.size;
  }

  copyin(buffer, value, initial_offset, cleanup_list) {
    const str = ref.allocCString(value);
    cleanup_list.push(str);
    ref._writePointer(buffer, initial_offset + this.offset, str);
  }

  copyout(buffer, initial_offset) {
    const sbuf = ref.readPointer(buffer, initial_offset + this.offset,
      this.size
    );
    return ref.readCString(sbuf);
  }

  defaultValue() {
    return '';
  }
}

class EnumField extends AbstractField {
  constructor(name, enum_type) {
    super(name);
    this._enum_type = enum_type;
  }

  get size() {
    return ref.types.uint32.size;
  }

  get alignment() {
    return this.size;
  }

  copyin(buffer, value, initial_offset, cleanup_list) {
    ref.types.uint32.set(buffer,
      initial_offset + this.offset,
      value.value
    );
  }

  copyout(buffer, initial_offset) {
    const v = ref.types.uint32.get(buffer,
      initial_offset + this.offset
    );
    return this._enum_type.enums[v];
  }

  defaultValue() {
    return this._enum_type.enums[0];
  }
}
class ArrayField extends AbstractField {
  constructor(name, wrapped_field, num_elements) {
    super(name);
    this._wrapped_field = wrapped_field;
    this._wrapped_field.offset = 0;
    this._num_elements = num_elements;
  }

  get size() {
    return this._wrapped_field.size * this._num_elements;
  }

  get alignment() {
    return this._wrapped_field.alignment;
  }

  copyin(buffer, value, initial_offset, cleanup_list) {
    const offset = initial_offset + this.offset;
    for (var i = 0; i < this._num_elements; i++) {
      this._wrapped_field.copyin(buffer, value[i],
        offset + i * this._wrapped_field.size,
        cleanup_list
      );
    }
  }

  copyout(buffer, initial_offset) {
    const offset = initial_offset + this.offset;
    const arr = [];
    for (var i = 0; i < this._num_elements; i++) {
      arr.push(this._wrapped_field.copyout(
        buffer,
        offset + i * this._wrapped_field.size
      ));
    }
    return arr;
  }

  defaultValue() {
    const obj = [];
    for (var i = 0; i < this._num_elements; i++) {
      obj.push(this._wrapped_field.defaultValue());
    }
    return obj;
  }
}

/*
 * It seems counter intuitive to employ ref-struct in a module
 * that is replacing it, but...
 *
 * This is the easiest way to figure out the sizes/alignments
 * of various sequence fields in a machine independent way.
 *
 * To be clear: copy-in/copy-out WILL NOT USE ref-struct.
 */
const seqRS = RefStruct({
  len: ref.types.int32,
  max: ref.types.int32,
  buffer: ref.refType(ref.types.void),
  release: ref.types.bool,
});
const seq_len_offset = seqRS.fields.len.offset;
const seq_max_offset = seqRS.fields.max.offset;
const seq_ptr_offset = seqRS.fields.buffer.offset;
const seq_rel_offset = seqRS.fields.release.offset;
const seq_alignment = seqRS.alignment;
const seq_size = seqRS.size;

class SequenceField extends AbstractField {
  constructor(name, wrapped_field, max_elements) {
    super(name);
    this._wrapped_field = wrapped_field;
    this._wrapped_field.offset = 0;
    this._max_elements = max_elements;
  }

  get size() {
    return seq_size;
  }

  get alignment() {
    return seq_alignment;
  }

  copyin(buffer, value, initial_offset, cleanup_list) {
    var len = value.length;
    if (this._max_elements > 0 && len > this._max_elements) {
      len = this._max_elements;
    }
    const sz = this._wrapped_field.size;
    const contents = new Buffer(len * sz);
    for (var i = 0; i < len; i++) {
      this._wrapped_field.copyin(contents, value[i],
        i * sz, cleanup_list);
    }
    ref.types.uint32.set(buffer,
      initial_offset + this.offset + seq_len_offset,
      len
    );
    ref.types.uint32.set(buffer,
      initial_offset + this.offset + seq_max_offset,
      len
    );
    ref.types.bool.set(buffer,
      initial_offset + this.offset + seq_rel_offset,
      false
    );
    ref._writePointer(buffer,
      initial_offset + this.offset + seq_ptr_offset,
      contents
    );
    cleanup_list.push(contents);
  }

  copyout(buffer, initial_offset) {
    const seq = [];
    const sz = this._wrapped_field.size;
    const len = ref.types.uint32.get(buffer,
      initial_offset + this.offset + seq_len_offset
    );
    if (len > 0) {
      const sbuf = ref.readPointer(buffer,
        initial_offset + this.offset + seq_ptr_offset,
        len * sz
      );
      for (var i = 0; i < len; i++) {
        seq.push(this._wrapped_field.copyout(
          sbuf,
          i * sz
        ));
      }
    }
    return seq;
  }

  defaultValue() {
    return [];
  }
}

class UnionField extends AbstractField {
  constructor(name, union_type) {
    super(name);
    this._union = union_type;
    this.init();
  }

  init(){
    this._d_field = fieldFactory('_d', this._union['d.type']);
    this._d_field.offset = 0;
    this._v_fields = {};
    // calcalculate value size & offset
    let valign = 1;
    let vsize = 0;
    let typesMap = this._union['v.types'];
    for (const name of typesMap.keys()) {
      this._v_fields[name] = fieldFactory(name, typesMap.get(name));
      valign = Math.max(valign, this._v_fields[name].alignment);
      vsize = Math.max(vsize, this._v_fields[name].size);
    }
    const voffset = alignup(this._d_field.size, valign);
    for (const name of Object.keys(this._v_fields)) {
      this._v_fields[name].offset = voffset;
    }
    this._alignment = Math.max(valign, this._d_field.alignment);
    this._size = alignup(voffset + vsize, this._alignment);
  }

  get size() {
    return this._size;
  }

  get alignment() {
    return this._alignment;
  }

  defaultValue() {
    return new this._union();
  }

  copyin(buffer, value, initial_offset, cleanup_list) {
    this._d_field.copyin(
      buffer,
      value._d,
      initial_offset + this.offset,
      cleanup_list
    );

    let case_name = this._union['d.cases'].get(value._d);
    this._v_fields[case_name].copyin(
      buffer,
      value._v,
      initial_offset + this.offset,
      cleanup_list
    );
  }

  copyout(buffer, initial_offset) {

    let u = new this._union();

    u._d = this._d_field.copyout(
      buffer,
      initial_offset + this.offset
    );

    let cases = this._union['d.cases'];
    const case_name = cases.get(u._d);
    u._v = this._v_fields[case_name].copyout(
      buffer,
      initial_offset + this.offset
    );
    return u;
  }
}

const basicTypes = new Set([
  ref.types.bool,
  ref.types.int8,
  ref.types.int16,
  ref.types.int32,
  ref.types.int64,
  ref.types.uint8,
  ref.types.uint16,
  ref.types.uint32,
  ref.types.uint64,
  ref.types.float,
  ref.types.double,
]);

class Type {
  constructor(fields) {
    this._fields = [];
    this._size = 0;
    this._alignment = 1;
    for (const fn of Object.keys(fields)) {
      let f = fieldFactory(fn, fields[fn]);
      this._fields.push(f);
      this._size = alignup(this._size, f.alignment);
      f.offset = this._size;
      this._size += f.size;
      this._alignment = Math.max(this._alignment, f.alignment);
    }
    this._size = alignup(this._size, this._alignment);
  }

  get size() {
    return this._size;
  }

  get alignment() {
    return this._alignment;
  }

  get fields() {
    return this._fields;
  }

  _copyin(buffer, obj, initial_offset = 0, cleanup_list) {
    for (var f of this.fields) {
      f.copyin(buffer, obj[f.name], initial_offset, cleanup_list);
    }
  }

  copyin(obj) {
    const buf = new Buffer(this.size);
    const cleanup_list = [];
    this._copyin(buf, obj, 0, cleanup_list);
    for (const sbuf of cleanup_list) {
      ref._attach(buf, sbuf);
    }
    return buf;
  }

  copyout(buffer, initial_offset = 0) {
    const obj = {};
    for (var f of this.fields) {
      obj[f.name] = f.copyout(buffer, initial_offset);
    }
    return obj;
  }

  asField(name) {
    return new NestedStructField(name, this);
  }

  newInstance() {
    const obj = {};
    for (var f of this.fields) {
      obj[f.name] = f.defaultValue();
    }
    return obj;
  }
}

class DDSArray {
  constructor(type, num_elements) {
    this._type = type;
    this._num_elements = num_elements;
  }

  asField(name) {
    return new ArrayField(name, fieldFactory(name, this._type),
      this._num_elements);
  }
}

class DDSSequence {
  constructor(type, max_elements = 0) {
    this._type = type;
    this._max_elements = max_elements;
  }

  asField(name) {
    return new SequenceField(name,
      fieldFactory(name, this._type),
      this._max_elements);
  }
}

class DDSUnion {

  constructor() {

  }
}

function fieldFactory(name, type) {
  try {
    return type.asField(name);
  } catch (error) {
    // try the alternatives...
  }
  if (basicTypes.has(type)) {
    return new BasicField(name, type);
  } else if (type === ref.types.CString) {
    return new StringField(name);
  } else if (type instanceof Enum) {
    return new EnumField(name, type);
  } else if (DDSUnion.isPrototypeOf(type)) {
    return new UnionField(name, type);
  } else {
    throw new Error('Unsupported type for field ' + name);
  }
}

module.exports = {
  Type: Type,
  Array: DDSArray,
  Sequence: DDSSequence,
  Union: DDSUnion,
};
