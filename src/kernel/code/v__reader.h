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

#ifndef V__READER_H
#define V__READER_H

#include "v_reader.h"
#include "v_entity.h"
#include "v__subscriberQos.h"

#define v__readerIsGroupCoherent(reader_) v_subscriberQosIsGroupCoherent((reader_)->subQos)
#define v__readerIsGroupOrderedNonCoherent(reader_) \
    ((reader_->subQos->presentation.v.access_scope == V_PRESENTATION_GROUP) && \
     (reader_->subQos->presentation.v.coherent_access == FALSE) && \
     (reader_->subQos->presentation.v.ordered_access == TRUE))

#define v_readerAccessScope(_this) \
        v_reader(_this)->subQos->presentation.v.access_scope

void
v_readerInit(
    _Inout_ v_reader _this,
    _In_z_ const c_char *name,
    _In_ v_subscriber s,
    _In_ v_readerQos qos);

void
v_readerDeinit(
    v_reader _this);

void
v_readerFree(
    v_reader _this);

void
v_readerAddEntry(
    _Inout_ v_reader _this,
    _In_ v_entry e);

void
v_readerAddTransactionAdmin(
    _Inout_ v_reader r,
    _In_opt_ v_transactionGroupAdmin a);

void
v__readerNotifyStateChange_nl(
    v_reader _this,
    c_bool complete);

v_topic
v_readerGetTopic(
    v_reader _this);

v_topic
v_readerGetTopic_nl(
    v_reader _this);

void
v_readerPublishBuiltinInfo(
    v_reader _this);

v_subscriber
v_readerGetSubscriber(
    v_reader _this);

#endif
