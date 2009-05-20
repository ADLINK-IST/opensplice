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

#include "v_statistics.h"
#include "v__statistics.h"

#include "v_networkingStatistics.h"

static c_type networkingStatisticsType = NULL;

v_networkingStatistics v_networkingStatisticsNew(v_kernel k)
{
    v_networkingStatistics _this;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    if (networkingStatisticsType == NULL) {
        networkingStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_networkingStatistics");
    }
    _this = v_networkingStatistics(v_new(k, networkingStatisticsType));
    v_networkingStatisticsInit(_this);
    return _this;
}

void v_networkingStatisticsInit(v_networkingStatistics _this)
{
    v_kernel kernel;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));

    v_statisticsInit(v_statistics(_this));
    kernel = v_objectKernel(_this);

    _this->numberOfErrors = 0;
    _this->channelsCount = 0;
    _this->channels = NULL;
}

void v_networkingStatisticsDeinit(v_networkingStatistics _this)
{
    assert(_this!=NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));
}

c_bool v_networkingStatisticsReset(v_networkingStatistics _this, const c_char* fieldName)
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

void v_networkingStatisticsFree(v_networkingStatistics _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_networkingStatistics));

    v_networkingStatisticsDeinit(_this);
    c_free(_this);
}


