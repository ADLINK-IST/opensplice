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

#ifndef V_WRITERSAMPLE_H
#define V_WRITERSAMPLE_H

#include "v_kernel.h"
#include "v_message.h"
#include "v_writer.h"

#define v_writerSample(_this)         (C_CAST(_this,v_writerSample))
#define v_writerSampleTemplate(_this) ((v_writerSampleTemplate)(_this))

typedef c_bool (*v_writerSampleAction)(v_writerSample sample, c_voidp arg);

/* Precondition for these functions: protect the sample yourself */

#define v_writerSampleMessage(_this) \
        (v_writerSampleTemplate(_this)->message)

#define v_writerSampleTestState(_this,mask) \
        (v_stateTest(v_nodeState(v_writerSampleMessage(_this)),mask))

v_writerSample
v_writerSampleNew(
    v_writer w,
    v_message m);

void
v_writerSampleSetResendScope(
    v_writerSample _this,
    v_resendScope resendScope);

c_bool
v_writerSampleHasBeenSentBefore(
    v_writerSample _this);

#endif
