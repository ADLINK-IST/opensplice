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
#include "ccpp_CacheAccess_impl.h"
#include "DLRL_Report.h"
#include "DLRL_Util.h"
#include "ccpp_DlrlUtils.h"

DDS::CacheAccess_impl::CacheAccess_impl() : access(NULL)
{
}

DDS::CacheAccess_impl::~CacheAccess_impl()
{
    DLRL_INFO(INF_ENTER);

    if(access){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(access));
        access = NULL;
    }

    DLRL_INFO(INF_EXIT);
}

DDS::Cache_ptr
DDS::CacheAccess_impl::owner(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::Cache_ptr owner = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    owner = DOWNCAST_DDS_CACHE(DK_CacheAccessAdmin_ts_getLSOwner(
        access,
        &exception,
        NULL));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return owner;
}

DDS::ContractSeq *
DDS::CacheAccess_impl::contracts(
    ) THROW_ORB_EXCEPTIONS
{

    DLRL_INFO(INF_ENTER);
    /* NOT YET SUPPORTED */
    DLRL_INFO(INF_EXIT);

    return new DDS::ContractSeq(0);
}

DDS::StringSeq *
DDS::CacheAccess_impl::type_names(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::StringSeq* typeSeq = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAccessAdmin_ts_getContainedTypeNames(
        access,
        &exception,
        NULL,
        reinterpret_cast<void**>(&typeSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!typeSeq){
        typeSeq = new DDS::StringSeq(0);
        DLRL_VERIFY_ALLOC(typeSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return typeSeq;
}

DDS::LongSeq*
DDS::CacheAccess_impl::contained_types(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::LongSeq* typeSeq = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAccessAdmin_ts_getContainedTypes(
        access,
        &exception,
        NULL,
        reinterpret_cast<void**>(&typeSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!typeSeq){
        typeSeq = new DDS::LongSeq();
        DLRL_VERIFY_ALLOC(typeSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return typeSeq;
}

void
DDS::CacheAccess_impl::write(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::DCPSError,
        DDS::PreconditionNotMet,
        DDS::InvalidObjects)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAccessAdmin_ts_write(access, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

void
DDS::CacheAccess_impl::purge(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::DCPSError)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAccessAdmin_ts_purge(access, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

DDS::Contract_ptr
DDS::CacheAccess_impl::create_contract(
    DDS::ObjectRoot * an_object,
    DDS::ObjectScope scope,
    ::CORBA::Long depth) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DLRL_INFO(INF_ENTER);
    /* remember: verify not null */
    /* NOT YET SUPPORTED */
    DLRL_INFO(INF_EXIT);
    return NULL;
}

void
DDS::CacheAccess_impl::delete_contract(
    DDS::Contract_ptr a_contract) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DLRL_INFO(INF_ENTER);
    /* remember: verify not null */
    /* NOT YET SUPPORTED */
    DLRL_INFO(INF_EXIT);
}

DDS::ObjectRootSeq *
DDS::CacheAccess_impl::get_invalid_objects(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::ObjectRootSeq* rootSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAccessAdmin_ts_getInvalidObjects(
        access,
        &exception,
        NULL,
        reinterpret_cast<void**>(&rootSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!rootSeq){
        rootSeq = new DDS::ObjectRootSeq(0);
        DLRL_VERIFY_ALLOC(rootSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return rootSeq;
}

DDS::ObjectRootSeq *
DDS::CacheAccess_impl::objects(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::ObjectRootSeq* rootSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAccessAdmin_ts_getObjects(
        access,
        &exception,
        NULL,
        reinterpret_cast<void**>(&rootSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!rootSeq){
        rootSeq = new DDS::ObjectRootSeq(0);
        DLRL_VERIFY_ALLOC(rootSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return rootSeq;
}

void
DDS::CacheAccess_impl::refresh(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::DCPSError,
        DDS::PreconditionNotMet)
{
    DLRL_INFO(INF_ENTER);
    /* NOT YET SUPPORTED */
    DLRL_INFO(INF_EXIT);
}

DDS::CacheKind
DDS::CacheAccess_impl::kind(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return CACHEACCESS_KIND;
}

DDS::CacheUsage
DDS::CacheAccess_impl::cache_usage(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::CacheUsage usage = READ_ONLY;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    usage = static_cast<DDS::CacheUsage>(DK_CacheAccessAdmin_ts_getCacheUsage(
        access,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return usage;
}
