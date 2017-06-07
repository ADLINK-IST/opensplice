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

#ifndef C__METABASE_H
#define C__METABASE_H

#include "c_metabase.h"

#define C_META_OFFSET_(t,f) \
        ((os_size_t)(&((t)0)->f))

#define C_META_ATTRIBUTE_(c,o,n,t) \
        c_metaAttributeNew(o,#n,t,C_META_OFFSET_(c,n))

#define C_META_TYPEINIT_(o,t) do { \
        C_ALIGNMENT_C_STRUCT_TYPE(t); \
        c_metaTypeInit(o,C_SIZEOF(t),C_ALIGNMENT_C_STRUCT(t)); \
    } while(0)

#if defined (__cplusplus)
extern "C" {
#endif

c_bool
c_objectIs (
    c_baseObject _this,
    c_metaKind kind);

c_bool
c_objectIsType (
    c_baseObject _this);

void
c_metaAttributeNew (
    c_metaObject scope,
    const c_char *name,
    c_type type,
    os_size_t offset);

void
c_metaTypeInit (
    c_object _this,
    os_size_t size,
    os_size_t alignment);

void
c_metaCopy (
    c_metaObject src,
    c_metaObject dst);

c_result
c__metaFinalize(
    c_metaObject o,
    c_bool normalize);

c_bool
c_isBaseObjectType(
    c_type type);

c_bool
c_isBaseObject(
    c_object obj);

#if defined (__cplusplus)
}
#endif

#endif
