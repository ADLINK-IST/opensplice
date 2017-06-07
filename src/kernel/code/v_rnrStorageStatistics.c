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
/* TODO this file should be removed!! */

#include "v_statistics.h"
#include "v__statistics.h"
#include "v_rnrStorageStatistics.h"
#include "os_report.h"

v_rnrStorageStatistics
v_rnrStorageStatisticsNew(
    v_kernel k,
    const c_char *name)
{
    v_rnrStorageStatistics _this;
    c_string storageName;
    c_type type;

    assert(k && name);
    assert(C_TYPECHECK(k, v_kernel));

    type = c_resolve(c_getBase(k), "kernelModuleI::v_rnrStorageStatistics");

    _this = v_rnrStorageStatistics(c_new(type));
    storageName = c_stringNew(c_getBase(c_object(k)), name);
    c_free(type);

    v_rnrStorageStatisticsInit(_this, k, storageName);
    return _this;
}

void
v_rnrStorageStatisticsInit(
    v_rnrStorageStatistics _this,
    v_kernel k,
    c_string name)
{
    c_type type;

    assert(_this && name);
    assert(C_TYPECHECK(_this, v_rnrStorageStatistics));

    v_statisticsInit(v_statistics(_this));

    _this->name = name;

    type = c_resolve(c_getBase(k), "kernelModuleI::v_rnrGroupStatistics");
    _this->topics = c_tableNew(type, "name");
}

void
v_rnrStorageStatisticsDeinit(
    v_rnrStorageStatistics _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this, v_rnrStorageStatistics));
    c_free(_this->name);
    c_free(_this->topics);
}

void
v_rnrStorageStatisticsFree(
    v_rnrStorageStatistics _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this, v_rnrStorageStatistics));
    v_rnrStorageStatisticsDeinit(_this);
    c_free(_this);
}

static c_bool
resetGroupStatistic(
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    v_rnrGroupStatisticsReset(v_rnrGroupStatistics(o), NULL);
    return TRUE;
}

void
v_rnrStorageStatisticsReset(
    v_rnrStorageStatistics _this,
    c_string fieldName)
{
    assert(_this);
    assert(C_TYPECHECK(_this, v_rnrStorageStatistics));

    if (fieldName) {
        /* TODO Reset individual group statistic ('part.topic' matching 'fieldName') */
        /*  v_statisticsResetField(v_statistics(_this), fieldName); */
    } else {
        /* Reset all group statistics */
        /* Todo use fieldname to reset a specific statistic? */
        c_tableWalk(_this->topics, resetGroupStatistic, NULL);
    }
}

struct checkGroupExistsHelper {
    const char *name;
    v_rnrGroupStatistics stats;
};

static c_bool
checkGroupExists(
    c_object o /* v_rnrStorageStatistics */,
    c_voidp arg /* c_char* */)
{
    v_rnrGroupStatistics stats = v_rnrGroupStatistics(o);
    struct checkGroupExistsHelper *helper = (struct checkGroupExistsHelper*)arg;

    if (strcmp(helper->name, stats->name) == 0) {
        helper->stats = stats;
        return FALSE;
    }
    return TRUE;
}

v_rnrGroupStatistics
v_rnrStorageStatisticsGroup(
        v_rnrStorageStatistics _this,
        v_service service,
        const c_char* name)
{
    v_kernel kernel;
    struct checkGroupExistsHelper helper;

    assert(_this);
    assert(C_TYPECHECK(_this, v_rnrStorageStatistics));
    assert(service && name);
    helper.name = name;
    helper.stats = NULL;

    if (c_walk(_this->topics, checkGroupExists, &helper)) {
        kernel = v_objectKernel(service);
        helper.stats = v_rnrGroupStatisticsNew(kernel, name);
        assert(helper.stats);
        c_tableInsert(_this->topics, helper.stats);
    }

    return helper.stats;
}
