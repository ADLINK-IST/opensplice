/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef V__GROUPINSTANCE_H
#define V__GROUPINSTANCE_H

#include "v_kernel.h"
#include "v_writerInstance.h"
#include "v_state.h"
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

#define v_groupInstanceMessageCount(_this) \
        (v_groupInstance(_this)->messageCount)

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
    v_message message,
    v_message *regMsg);

v_writeResult
v_groupInstanceRemoveRegistration(
    v_groupInstance instance,
    v_registration registration,
    c_time timestamp);

v_writeResult
v_groupInstanceUnregister (
    v_groupInstance _this,
    v_message message);

v_writeResult
v_groupInstanceInsert (
    v_groupInstance _this,
    v_message message);

void
v_groupInstanceRemove (
    v_groupSample sample);
void
v_groupInstancePurge(
    v_groupInstance _this);
c_time
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
    c_time purgeTime);

void
v_groupInstanceDisconnect(
    v_groupInstance _this);

v_writeResult
v_groupInstanceDispose (
    v_groupInstance instance,
    c_time timestamp);

void
v_groupInstancecleanup(
    v_groupInstance _this,
    v_registration registration,
    c_time timestamp,
    c_bool isImplicit);

#endif
