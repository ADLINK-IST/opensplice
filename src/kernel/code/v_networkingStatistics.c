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

#include "v_statistics.h"
#include "v_networkingStatistics.h"

v_networkingStatistics
v_networkingStatisticsNew(
    v_kernel k)
{
    v_networkingStatistics _this;
    c_type networkingStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    networkingStatisticsType = c_resolve(c_getBase(k),
                                         "kernelModuleI::v_networkingStatistics");

    _this = v_networkingStatistics(v_new(k, networkingStatisticsType));
    v_networkingStatisticsInit(_this);
    return _this;
}

void
v_networkingStatisticsInit(
    v_networkingStatistics _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));

    v_statisticsInit(v_statistics(_this));

    _this->numberOfErrors = 0;
    _this->channelsCount = 0;
    _this->channels = c_arrayNew(c_resolve(c_getBase(c_object(_this)),
                                 "kernelModuleI::v_networkChannelStatistics"),64);

}

void
v_networkingStatisticsDeinit(
    v_networkingStatistics _this)
{
    assert(_this!=NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));
    OS_UNUSED_ARG(_this);
}

void
v_networkingStatisticsFree(
    v_networkingStatistics _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));

    v_networkingStatisticsDeinit(_this);
    c_free(_this);
}


