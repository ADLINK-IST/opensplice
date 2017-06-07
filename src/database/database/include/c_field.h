/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef C_FIELD_H
#define C_FIELD_H

/** \file c_base.h
 *  \brief This file specifies the database c_field class interface.
 *
 * The c_field class implements a helper object to access the field value
 * of a structured type.
 */

#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Meta data definitions:
 */
C_CLASS(c_field);

OS_API c_type
c_field_t(
    c_base base);

/**
 * c_field constructors:
 */
OS_API c_field
c_fieldNew(
    c_type type,
    const c_char *fieldName);

OS_API c_field
c_fieldConcat(
    c_field head,
    c_field tail);

/**
 * c_field methods:
 */
OS_API c_string
c_fieldName(
    c_field _this);

OS_API c_array
c_fieldPath(
    c_field _this);

OS_API c_type
c_fieldType(
    c_field _this);

OS_API c_valueKind
c_fieldValueKind(
    c_field _this);

OS_API void
c_fieldFreeRef (
    c_field field,
    c_object o);

OS_API void
c_fieldAssign(
    c_field _this,
    c_object o,
    c_value value);

OS_API c_value
c_fieldValue(
    c_field _this,
    c_object o);

OS_API void
c_fieldCopy(
    c_field srcfield,
    c_object src,
    c_field dstfield,
    c_object dst);

OS_API void
c_fieldClone(
    c_field srcfield,
    c_object src,
    c_field dstfield,
    c_object dst);

OS_API c_equality
c_fieldCompare (
    c_field field1,
    c_object src1,
    c_field field2,
    c_object src2);

OS_API c_size
c_fieldBlobSize(
    c_field field,
    c_object o);

OS_API c_size
c_fieldBlobCopy(
    c_field field,
    c_object o,
    void *dst);

OS_API void *
c_fieldGetAddress(
    c_field field,
    c_object o);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
