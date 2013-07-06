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
#include "v_rnrGroupStatistics.h"
#include "v_statistics.h"
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

    type = c_resolve(c_getBase(k), "kernelModule::v_rnrGroupStatistics");

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

