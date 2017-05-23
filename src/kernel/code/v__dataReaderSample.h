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

#ifndef V__DATAREADERSAMPLE_H
#define V__DATAREADERSAMPLE_H

#include "v_kernel.h"
#include "v_dataReaderSample.h"

v_dataReaderSample
v_dataReaderSampleNew(
    v_dataReaderInstance instance,
    v_message message);

void
v_dataReaderSampleRemoveFromLifespanAdmin(
    v_dataReaderSample _this);

void
v_dataReaderSampleWipeViews(
    v_dataReaderSample _this);
void
v_dataReaderSampleEmptyViews(
    v_dataReaderSample _this);

void
v_dataReaderSampleAddViewSample(
    v_readerSample sample,
    v_dataViewSample viewSample);

#endif
