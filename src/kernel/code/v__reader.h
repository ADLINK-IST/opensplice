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

#ifndef V__READER_H
#define V__READER_H

#include "v_reader.h"
#include "v_entity.h"

#define v_readerEntrySetLock(_this) \
        c_mutexLock(&_this->entrySet.mutex)

#define v_readerEntrySetUnlock(_this) \
        c_mutexUnlock(&_this->entrySet.mutex)

#define v__readerIsGroupCoherent(reader_) \
    ((reader_->subQos->presentation.v.access_scope == V_PRESENTATION_GROUP) && \
     (reader_->subQos->presentation.v.coherent_access))

#define v__readerIsGroupOrderedNonCoherent(reader_) \
    ((reader_->subQos->presentation.v.access_scope == V_PRESENTATION_GROUP) && \
     (reader_->subQos->presentation.v.coherent_access == FALSE) && \
     (reader_->subQos->presentation.v.ordered_access == TRUE))

void
v_readerInit(
    v_reader _this,
    const c_char *name,
    v_subscriber s,
    v_readerQos qos,
    c_bool enable);

void
v_readerDeinit(
    v_reader _this);

void
v_readerFree(
    v_reader _this);

v_result
v_readerSubscribe(
    v_reader _this,
    v_partition d);

c_bool
v_readerUnSubscribe(
    v_reader _this,
    v_partition d);

v_result
v_readerGetHistoricalData(
    v_reader _this);

c_bool
v_readerSubscribeGroup (
    v_reader _this,
    v_group group);

c_bool
v_readerUnSubscribeGroup (
    v_reader _this,
    v_group g);

v_entry
v_readerAddEntry(
    v_reader _this,
    v_entry e);

v_entry
v_readerRemoveEntry(
    v_reader _this,
    v_entry e);

#endif
