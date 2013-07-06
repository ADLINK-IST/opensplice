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

#include "v_statistics.h"
#include "v__statistics.h"

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
                                         "kernelModule::v_networkingStatistics");

    _this = v_networkingStatistics(v_new(k, networkingStatisticsType));
    v_networkingStatisticsInit(_this,k);
    return _this;
}

void
v_networkingStatisticsInit(
    v_networkingStatistics _this,
    v_kernel k)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));

    v_statisticsInit(v_statistics(_this));

    _this->numberOfErrors = 0;
    _this->channelsCount = 0;
    _this->channels = c_arrayNew(c_resolve(c_getBase(c_object(k)),
                                 "kernelModule::v_networkChannelStatistics"),64);

}

void
v_networkingStatisticsDeinit(
    v_networkingStatistics _this)
{
    assert(_this!=NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));
    OS_UNUSED_ARG(_this);
}

c_bool
v_networkingStatisticsReset(
    v_networkingStatistics _this, const c_char* fieldName)
{
    c_bool result;

    assert(_this!=NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));

    result = FALSE;

    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(_this), fieldName);
    } else {
        _this->numberOfErrors = 0;
        result = TRUE;
    }
    return result;
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


