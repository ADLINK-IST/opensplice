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
#include "v_rnrGroupStatistics.h"
#include "v_statistics.h"
#include "v_fullCounter.h"
#include "v__statistics.h"
#include "v_rnrStorageStatistics.h"

v_rnrGroupStatistics
v_rnrGroupStatisticsNew(
    v_kernel k,
    const c_char *name)
{
    v_rnrGroupStatistics _this;
    c_string groupName;
    c_type type;

    assert(k && name);
    assert(C_TYPECHECK(k, v_kernel));

    type = c_resolve(c_getBase(k), "kernelModuleI::v_rnrGroupStatistics");

    _this = v_rnrGroupStatistics(c_new(type));
    groupName = c_stringNew(c_getBase(c_object(k)), name);
    c_free(type);

    v_rnrGroupStatisticsInit(_this, groupName);
    return _this;
}

void
v_rnrGroupStatisticsInit(
    v_rnrGroupStatistics _this,
    c_string name)
{
    assert(_this && name);
    assert(C_TYPECHECK(_this, v_rnrGroupStatistics));

    v_statisticsInit(v_statistics(_this));

    _this->name = name;
    v_fullCounterInit(&(_this->dataRateRecorded));
    v_fullCounterInit(&(_this->dataRateReplayed));
    _this->numberOfBytesRecorded = 0;
    _this->numberOfBytesReplayed = 0;
    _this->numberOfSamplesRecorded = 0;
    _this->numberOfSamplesReplayed = 0;
}

void
v_rnrGroupStatisticsDeinit(
    v_rnrGroupStatistics _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this, v_rnrGroupStatistics));
    c_free(_this->name);
}

void
v_rnrGroupStatisticsReset(
    v_rnrGroupStatistics _this,
    c_string fieldName)
{
    OS_UNUSED_ARG(fieldName);
    v_fullCounterReset(&(_this->dataRateRecorded));
    v_fullCounterReset(&(_this->dataRateReplayed));
    _this->numberOfBytesRecorded = 0;
    _this->numberOfBytesReplayed = 0;
    _this->numberOfSamplesRecorded = 0;
    _this->numberOfSamplesReplayed = 0;
}

