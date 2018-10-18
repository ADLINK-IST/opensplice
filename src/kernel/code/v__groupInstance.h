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

#ifndef V__GROUPINSTANCE_H
#define V__GROUPINSTANCE_H

#include "v_kernel.h"
#include "v_writerInstance.h"
#include "v_state.h"
#include "v__policy.h"
#include "v__group.h"
#include "v_groupInstance.h"

#define v_groupInstance(_this) (C_CAST(_this,v_groupInstance))
#define v_groupInstanceTemplate(_this) \
        ((v_groupInstanceTemplate)v_groupInstance(_this))

#define v_groupInstanceHead(_this) \
        v_groupSample(v_groupInstanceTemplate(_this)->newest)

#define v_groupInstanceSetHead(_this,_sample) \
        v_groupInstanceTemplate(_this)->newest = \
        c_keep(v_groupSample(_sample))

#define v_groupInstanceSetHeadNoRefCount(_this,_sample) \
        v_groupInstanceTemplate(_this)->newest = \
        v_groupSampleTemplate(_sample)

#define v_groupInstanceTail(_this) \
        v_groupSample(v_groupInstance(_this)->oldest)

#define v_groupInstanceSetTail(_this,_sample) \
        v_groupInstance(_this)->oldest = \
        v_groupSample(_sample)

#define v_groupInstanceGroup(_this) \
        v_group(v_groupInstance(_this)->group)

#define v_groupInstanceOwner(_this) \
        v_group(v_groupInstance(_this)->group)

#define v_groupInstanceHistorySampleCount(_this) \
        (v_groupInstance(_this)->historySampleCount)

#define v_groupInstanceResourceSampleCount(_this) \
        (v_groupInstance(_this)->resourceSampleCount)

#define v_groupInstanceStateTest(_this,_mask) \
        v_stateTest(v_groupInstance(_this)->state,_mask)

#define v_groupInstanceWrite(_this,instance,msg, resendScope) \
            v_groupWrite(v_groupInstanceGroup(_this), \
                         msg, \
                         (v_groupInstance *)instance, \
                         V_NETWORKID_LOCAL, \
                         resendScope)\

#define v_groupInstanceResend(_this,instance,msg,resendScope) \
        v_groupResend(v_groupInstanceGroup(_this), \
                     msg, \
                     (v_groupInstance *)instance, \
                     resendScope, \
                     V_NETWORKID_LOCAL)

#define v_groupInstanceSetEpoch(_this,_epoch) \
        v_groupInstance(_this)->epoch = _epoch

typedef c_bool (*v_groupInstanceWalkSampleAction) \
               (v_groupSample s, c_voidp arg);

typedef c_bool (*v_groupInstanceWalkRegistrationAction) \
               (v_registration r, c_voidp arg);

v_groupInstance
v_groupInstanceNew (
    v_group group,
    v_message message);

void
v_groupInstanceInit (
    v_groupInstance _this,
    v_message message);

void
v_groupInstanceFree(
    v_groupInstance _this);

v_writeResult
v_groupInstanceRegister (
    v_groupInstance _this,
    v_message message);

v_writeResult
v_groupInstanceUnregister (
    v_groupInstance _this,
    v_message message,
    v_transaction transaction);

v_writeResult
v_groupInstanceInsert (
    v_groupInstance _this,
    v_message message);

v_writeResult
v_groupInstanceFlushTransaction (
    v_groupInstance _this,
    v_message message,
    v_transaction transaction);

void
v_groupInstanceRemove (
    v_groupSample sample);
void
v_groupInstancePurge(
    v_groupInstance _this);

os_timeW
v_groupInstanceDisposeTime (
    v_groupInstance _this);

c_bool
v_groupInstanceWalkRegistrations (
    v_groupInstance _this,
    v_groupInstanceWalkRegistrationAction action,
    c_voidp arg);

c_bool
v_groupInstanceWalkUnregisterMessages (
    v_groupInstance _this,
    v_groupInstanceWalkRegistrationAction action,
    c_voidp arg);

c_bool
v_groupInstanceWalkSamples (
    v_groupInstance _this,
    v_groupInstanceWalkSampleAction action,
    c_voidp arg);

void
v_groupInstanceGetRegisterMessages(
    v_groupInstance _this,
    c_ulong systemId,
    c_iter *messages);

c_bool
v_groupInstanceHasRegistration(
    v_groupInstance instance,
    v_registration registration);

c_bool
v_groupInstanceAcceptMessage(
    v_groupInstance _this,
    v_message message);

void
v_groupInstancePurgeTimed(
    v_groupInstance _this,
    os_timeE purgeTime);

void
v_groupInstanceDisconnect(
    v_groupInstance _this);

v_writeResult
v_groupInstanceDispose (
    v_groupInstance instance,
    os_timeW timestamp);

void
v_groupInstancecleanup(
    v_groupInstance _this,
    v_registration registration,
    os_timeW timestamp,
    c_bool isImplicit);

c_bool
v_groupInstanceClaimResource(
    v_groupInstance instance,
    v_message message);

void
v_groupInstanceReleaseResource(
    v_groupInstance _this);

v_ownershipResult
v_groupInstanceTestOwnership(
    v_groupInstance instance,
    v_message message);

#endif
