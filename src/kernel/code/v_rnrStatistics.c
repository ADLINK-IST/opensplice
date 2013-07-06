/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "v_rnrStatistics.h"
#include "v_statistics.h"
#include "v__statistics.h"
#include "v_maxValue.h"

v_rnrStatistics
v_rnrStatisticsNew(
    v_kernel k,
    const c_char *name)
{
    v_rnrStatistics _this;
    c_type type;
    c_string serviceName;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    type = c_resolve(c_getBase(k), "kernelModule::v_rnrStatistics");
    _this = v_rnrStatistics(v_new(k, type));

    serviceName = c_stringNew(c_getBase(c_object(k)), name);

    v_rnrStatisticsInit(_this, k, serviceName);
    return _this;
}

void
v_rnrStatisticsInit(
    v_rnrStatistics _this,
    v_kernel k,
    c_string name)
{
    c_type type;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_rnrStatistics));

    v_statisticsInit(v_statistics(_this));

    type = c_resolve(c_getBase(k), "kernelModule::v_rnrStorageStatistics");
    _this->storages = c_tableNew(type, "name");
    _this->name = name;
}

void
v_rnrStatisticsDeinit(
    v_rnrStatistics _this)
{
    assert(_this!=NULL);
    assert(C_TYPECHECK(_this, v_rnrStatistics));
    OS_UNUSED_ARG(_this);
    c_free(_this->name);
    /* todo free _this->storage */
}

void
v_rnrStatisticsFree(v_rnrStatistics _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_rnrStatistics));
    v_rnrStatisticsDeinit(_this);
    c_free(_this);
}

struct checkStorageIsNewHelper {
    const char *name;
    v_rnrStorageStatistics stats;
};

static c_bool
checkStorageIsNew(
    c_object o /* v_rnrStorageStatistics */,
    c_voidp arg /* c_char* */)
{
    v_rnrStorageStatistics stats = v_rnrStorageStatistics(o);
    struct checkStorageIsNewHelper *helper = (struct checkStorageIsNewHelper*)arg;

    if (strcmp(helper->name, stats->name) == 0) {
        helper->stats = stats;
        return FALSE;
    }
    return TRUE;
}

/* service is required because v_statistics does not extend entity */
v_rnrStorageStatistics
v_rnrStatisticsStorageStatistics(
    v_rnrStatistics _this,
    v_service service,
    const c_char* storageName)
{
    v_kernel kernel;
    struct checkStorageIsNewHelper helper;

    assert(_this);
    assert(C_TYPECHECK(_this, v_rnrStatistics));
    helper.name = storageName;
    helper.stats = NULL;
    kernel = v_objectKernel(service);

    if (c_walk(_this->storages, checkStorageIsNew, &helper)) {
        helper.stats = v_rnrStorageStatisticsNew(kernel, storageName);
        c_tableInsert(_this->storages, helper.stats);
    }
    assert(helper.stats);
    return helper.stats;
}

c_bool
v_rnrStatisticsReset(
    v_rnrStatistics _this,
    const c_char* fieldName)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(fieldName);
    return TRUE;
#if 0
    c_bool result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_rnrStatistics));

    result = FALSE;
    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(_this), fieldName);
    } else {
        /*
        v_maxValueReset(&(_this->maxConnectedClients));
        v_maxValueReset(&(_this->maxClientThreads));
        _this->requestsHandled = 0;
        _this->connectedClients = 0;
        _this->clientThreads = 0;
        */
        result = TRUE;
    }
    return result;
#endif
}
