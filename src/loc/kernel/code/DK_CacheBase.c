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
/* C includes */
#include <assert.h>

/* DLRL utilities includes */
#include "DLRL_Report.h"

/* DLRL kernel includes */
#include "DK_CacheBase.h"
#include "DLRL_Kernel_private.h"

#define ENTITY_NAME "DLRL Kernel CacheBase"

DK_Usage
DK_CacheBase_us_getCacheUsage(
    DK_CacheBase* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->usage;
}

/* must lock and ensure the cache is still alive */
DLRL_LS_object
DK_CacheBase_us_getLSCache(
    DK_CacheBase* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_cacheBase;
}

/* kind is never reset, so we can use it without fear of threads working against
 * eachother. Same goes for alive boolean
 */
DK_CacheKind
DK_CacheBase_ts_getKind(
    DK_CacheBase* _this,
    DLRL_Exception* exception)
{
    DK_CacheKind kind = DK_CACHE_KIND_CACHE;/* default */

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    if(DK_Entity_getClassID(&_this->entity) == DK_CLASS_CACHE_ADMIN)
    {
        kind = DK_CACHE_KIND_CACHE;
    } else
    {
        assert(DK_Entity_getClassID(&_this->entity) ==
            DK_CLASS_CACHE_ACCESS_ADMIN);
        kind = DK_CACHE_KIND_CACHE_ACCESS;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return kind;
}

