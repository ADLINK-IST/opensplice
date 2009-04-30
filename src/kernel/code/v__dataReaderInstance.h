/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef V__DATAREADERINSTANCE_H
#define V__DATAREADERINSTANCE_H

#include "v_dataReaderInstance.h"
#include "v_index.h"

#define v_dataReaderInstance_t(scope) \
        c_type(c_resolve(c_getBase(scope), \
                          "kernelModule::v_dataReaderInstance"))

#define v_dataReaderInstanceDataReader(_this) \
        v_indexDataReader(v_dataReaderInstance(_this)->index)

#define v_dataReaderInstanceSetHead(_this,_sample) \
        (v_dataReaderInstanceTemplate(_this)->sample = \
        v_dataReaderSampleTemplate(_sample))

#define v_dataReaderInstanceSetTail(_this,_sample) \
        (v_dataReaderInstanceTemplate(_this)->tail = \
        (c_voidp)_sample)

#define v_dataReaderInstanceStateSet(instance, state)               \
    assert(((state) == L_NEW) ||                                    \
           ((state) == L_EMPTY) ||                                  \
           ((state) == L_DISPOSED) ||                               \
           ((state) == L_STATECHANGED) ||                           \
           ((state) == L_NOWRITERS));                               \
    v_stateSet(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceStateClear(instance, state)             \
    assert(((state) == L_NEW) ||                                    \
           ((state) == L_EMPTY) ||                                  \
           ((state) == L_DISPOSED) ||                               \
           ((state) == L_NOWRITERS) ||                              \
           ((state) == L_STATECHANGED));                            \
    v_stateClear(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceInNotEmptyList(_this) \
        (v_dataReaderInstance(_this)->inNotEmptyList)

v_dataReaderInstance
v_dataReaderInstanceNew(
    v_dataReader reader,
    v_message message);

void
v_dataReaderInstanceInit(
    v_dataReaderInstance _this,
    v_message message);

#define v_dataReaderInstanceFree(_this) \
        c_free(_this)

void             
v_dataReaderInstanceDeinit(
    v_dataReaderInstance _this);

v_writeResult    
v_dataReaderInstanceWrite (
    v_dataReaderInstance _this,
    v_message message);

v_dataReaderResult 
v_dataReaderInstanceInsert(
    v_dataReaderInstance _this,
    v_message message);

c_bool           
v_dataReaderInstanceReadSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg);

c_bool           
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg);

c_bool             
v_dataReaderInstanceWalkSamples(
    v_dataReaderInstance _this,
    v_readerSampleAction action,
    c_voidp arg);

void             
v_dataReaderInstancePurge(
    v_dataReaderInstance _this);

c_bool           
v_dataReaderInstanceTest(
    v_dataReaderInstance _this,
    c_query query);

void             
v_dataReaderInstanceUnregister (
    v_dataReaderInstance _this,
    c_long count);

void
v_dataReaderInstanceSetEpoch (
    v_dataReaderInstance _this,
    c_time time);

#undef OS_API

#endif


