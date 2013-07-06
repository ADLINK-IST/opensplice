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
/* DLRL ccpp includes */
#include "ccpp_Cache_impl.h"
#include "ccpp_DlrlUtils.h"

/* DLRL kernel includes */
#include "DLRL_Kernel.h"

/* DLRL utility includes */
#include "DLRL_Exception.h"
#include "DLRL_Report.h"
#include "DLRL_Util.h"
#include "DLRL_Types.h"
#include "DLRL_Kernel_private.h"

/* DCPS ccpp includes */
#include "ccpp_Entity_impl.h"

/* DCPS gapi includes */
#include "gapi_domainParticipant.h"
#include "gapi_entity.h"

/* Defining header file include */
#include "ccpp_CacheFactory_impl.h"


class LocalCacheFactoryMutex
{
    public:
        os_mutex cfMutex;

        LocalCacheFactoryMutex(){
            os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
            if (os_mutexInit(&cfMutex, &mutexAttr) != os_resultSuccess){
                DLRL_REPORT(REPORT_ERROR, "CCPP_DLRL: Unable to create mutex");
            }
        }
};

static LocalCacheFactoryMutex localMutex;

DDS::CacheFactory_ptr _factorySelf = NULL;

DDS::CacheFactory::CacheFactory(){

}

DDS::CacheFactory::~CacheFactory(){

}

DDS::CacheFactory_ptr
DDS::CacheFactory::get_instance(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    os_result result;

    DLRL_INFO(INF_ENTER);
    DLRL_Exception_init(&exception);

    result = os_mutexLock(&(localMutex.cfMutex));
    if (result != os_resultSuccess){
        DLRL_REPORT(REPORT_ERROR, "CCPP_DLRL: Unable to obtain mutex");
    } else {
        if(!_factorySelf){
            _factorySelf = new DDS::CacheFactory();
            if(!_factorySelf){
                DLRL_REPORT(REPORT_ERROR, "CCPP_DLRL: Unable to allocate memory");
            } else {
                DK_CacheFactoryAdmin_ts_getInstance(&exception);
                DK_CacheFactoryAdmin_ts_registerLSCacheFactory(
                    UPCAST_DLRL_LS_OBJECT(
                        CacheFactoryInterface::_duplicate(_factorySelf)));
            }
        } /* else do nothing, we already have a cache factory instance */
    }

    DLRL_Exception_EXIT(exception);

    if(result == os_resultSuccess){ /* mutex lock was successfull, so we need to unlock it */
        result = os_mutexUnlock(&(localMutex.cfMutex));
        if (result != os_resultSuccess){
            DLRL_REPORT(REPORT_ERROR, "CCPP_DLRL: Unable to release mutex");
        } /* else do nothing, we succesfully unlocked the mutex */
    }

    DLRL_INFO(INF_EXIT);
    return DDS::CacheFactoryInterface::_duplicate(_factorySelf);
}

DDS::Cache_ptr
DDS::CacheFactory::create_cache(
    const char * name,
    DDS::CacheUsage cache_usage,
    DDS::DomainParticipant_ptr domain) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::DCPSError,
        DDS::AlreadyExisting)
{
    DLRL_Exception exception;
    gapi_domainParticipant gapiParticipant = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant _participant = NULL;
    u_participant uparticipant= NULL;
    DK_CacheAdmin* cache = NULL;
    DDS::Cache_impl* ccppCache = NULL;
    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, name, "name");
    DLRL_VERIFY_NOT_NULL(&exception, domain, "domain");

    gapiParticipant = reinterpret_cast<gapi_domainParticipant>(
        DDS_GET_GAPI_SELF(domain));
    _participant = gapi_domainParticipantClaim(gapiParticipant, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(&exception, result,
                                         "Unable to create cache.");
    uparticipant = _DomainParticipantUparticipant(_participant);
    if(uparticipant)
    {
        /* now create a proxy to this user layer participant which can be used by the DLRL
         * in a safe manner, as the user layer participant returned by the _DomainParticipantUparticipant
         * operation is owned by the gapi.
         */
        uparticipant = u_participant(DK_DCPSUtility_ts_createProxyUserEntity(&exception, u_entity(uparticipant)));
    }
    _EntityRelease(_participant);/* before the propagate */
    DLRL_Exception_PROPAGATE(&exception);/* after the release */
    ccppCache = new DDS::Cache_impl();
    DLRL_VERIFY_ALLOC(ccppCache, &exception, "Unable to create cache.");
    cache = DK_CacheFactoryAdmin_ts_createCache(
        &exception,
        UPCAST_DLRL_LS_OBJECT(DDS::Cache::_duplicate(ccppCache)),
        NULL,
        static_cast<DK_Usage>(cache_usage),
        name,
        uparticipant,
        UPCAST_DLRL_LS_OBJECT(DDS::DomainParticipant::_duplicate(domain)));
    DLRL_Exception_PROPAGATE(&exception);
    ccppCache->cache = cache;/*takes over ref count of the create!*/

    DLRL_Exception_EXIT(&exception);
    if(exception.exceptionID != DLRL_NO_EXCEPTION && ccppCache){
        CORBA::release(ccppCache);/*for the new*/
        CORBA::release(ccppCache);/*for the dup in the createCache call*/
        CORBA::release(domain);/*for the dup in the createCache call*/
        ccppCache = NULL;
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return ccppCache;
}

DDS::Cache_ptr
DDS::CacheFactory::find_cache_by_name(
    const char * name) THROW_ORB_EXCEPTIONS
{
    DK_CacheAdmin* cacheAdmin = NULL;
    DDS::Cache_ptr cache = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, name, "name");
    cacheAdmin = DK_CacheFactoryAdmin_ts_findCachebyName(name);
    if(cacheAdmin){
        cache = DOWNCAST_DDS_CACHE(DK_CacheAdmin_ts_getLSCache(
            cacheAdmin,
            &exception,
            NULL));
        DLRL_Exception_PROPAGATE(&exception);
    }

    DLRL_Exception_EXIT(exception);
    if(cacheAdmin){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(cacheAdmin));
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return cache;
}

void
DDS::CacheFactory::delete_cache(
    DDS::Cache_ptr a_cache) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::Cache_impl* cacheImpl = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    cacheImpl = dynamic_cast<DDS::Cache_impl*>(a_cache);
    DLRL_VERIFY_NOT_NULL(&exception, cacheImpl, "a_cache");
    DK_CacheFactoryAdmin_ts_deleteCache(&exception, NULL, cacheImpl->cache);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}
