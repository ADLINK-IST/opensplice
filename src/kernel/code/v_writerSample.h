/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef V_WRITERSAMPLE_H
#define V_WRITERSAMPLE_H

#include "v_kernel.h"
#include "v_message.h"
#include "v_writer.h"

#define v_writerSample(_this)         (C_CAST(_this,v_writerSample))
#define v_writerSampleTemplate(_this) ((v_writerSampleTemplate)(_this))

typedef c_bool (*v_writerSampleAction)(v_writerSample sample, c_voidp arg);

typedef enum v_writerSampleStatus {
    V_SAMPLE_ANY,
    V_SAMPLE_RESEND,
    V_SAMPLE_KEEP,
    V_SAMPLE_RELEASE,
    V_SAMPLE_DISPOSED
} v_writerSampleStatus;

/* Precondition for these functions: protect the sample yourself */

#define v_writerSampleMessage(_this) \
        (v_writerSampleTemplate(_this)->message)

#define v_writerSampleTestState(_this,mask) \
        (v_stateTest(v_nodeState(v_writerSampleMessage(_this)),mask))

#define v_writerSampleNew(writer,message) \
        _v_writerSampleNew(v_writer(writer),v_message(message))

#define v_writerSampleGetStatus(_this) \
        _v_writerSampleGetStatus(v_writerSample(_this))
     
#define v_writerSampleClear(_this) \
        _v_writerSampleClear(v_writerSample(_this))

#define v_writerSampleRelease(_this) \
        _v_writerSampleRelease(v_writerSample(_this))

#define v_writerSampleKeep(_this,decayCount) \
        _v_writerSampleKeep(v_writerSample(_this),decayCount)

#define v_writerSampleResend(_this, resendScope) \
        _v_writerSampleResend(v_writerSample(_this), resendScope)

#define v_writerSampleSetSentBefore(_this, sentBefore) \
        _v_writerSampleSetSentBefore(_this, sentBefore)

#define v_writerSampleHasBeenSentBefore(_this) \
        _v_writerSampleHasBeenSentBefore(_this)

v_writerSample
_v_writerSampleNew(
    v_writer w,
    v_message m);

v_writerSampleStatus
_v_writerSampleGetStatus(
    v_writerSample _this);

void
_v_writerSampleClear(
    v_writerSample _this);                         

void
_v_writerSampleRelease(
    v_writerSample _this);

void
_v_writerSampleKeep(
    v_writerSample _this,
    c_long decayCount);

void
_v_writerSampleResend(
    v_writerSample _this,
    v_resendScope resendScope);

void
_v_writerSampleSetSentBefore(
    v_writerSample _this,
    c_bool sentBefore);

c_bool
_v_writerSampleHasBeenSentBefore(
    v_writerSample _this);

#endif
