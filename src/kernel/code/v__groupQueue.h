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

#ifndef V__GROUPQUEUE_H
#define V__GROUPQUEUE_H

#include "v_groupQueue.h"
#include "kernelModuleI.h"

#if defined (__cplusplus)
extern "C" {
#endif

v_writeResult
v_groupQueueWrite (
    v_groupQueue _this,
    v_groupAction action);

void
v_groupQueueInit (
    v_groupQueue _this,
    v_subscriber subscriber,
    const c_char *name,
    c_ulong size,
    v_readerQos qos,
    v_statistics qstat,
    c_iter expr);

void
v_groupQueueDeinit (
    v_groupQueue _this);

#if defined (__cplusplus)
}
#endif

#endif
