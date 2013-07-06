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
#include "ccpp_DlrlUtils.h"
#include "ccpp_ObjectHome_impl.h"
#include "ccpp_CacheAccess_impl.h"

/* DLRL kernel includes */
#include "DLRL_Kernel.h"

/* DLRL utility includes */
#include "DLRL_Exception.h"
#include "DLRL_Report.h"
#include "DLRL_Util.h"
#include "DLRL_Types.h"

/* Defining header file include */
#include "ccpp_Cache_impl.h"

DDS::Cache_impl::Cache_impl() : cache(NULL)
{
}

DDS::Cache_impl::~Cache_impl()
{
    DLRL_INFO(INF_ENTER);

    if(cache){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(cache));
        cache = NULL;
    }

    DLRL_INFO(INF_EXIT);
}

DDS::DCPSState
DDS::Cache_impl::pubsub_state(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::DCPSState pubsubState = INITIAL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    pubsubState = static_cast<DDS::DCPSState>(
        DK_CacheAdmin_ts_getPubSubState(cache, &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return pubsubState;
}

DDS::CacheKind
DDS::Cache_impl::kind(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return CACHE_KIND;
}

DDS::DomainParticipant_ptr
DDS::Cache_impl::the_participant(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::DomainParticipant_ptr participant = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*may return null*/
    participant = DOWNCAST_DDS_PARTICIPANT(
        DK_CacheAdmin_ts_getLSParticipant(cache, &exception, NULL));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return participant;
}

DDS::Publisher_ptr
DDS::Cache_impl::the_publisher(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::Publisher_ptr publisher = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*may return null*/
    publisher = DOWNCAST_DDS_PUBLISHER(
        DK_CacheAdmin_ts_getLSPublisher(cache, &exception, NULL));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return publisher;
}

DDS::Subscriber_ptr
DDS::Cache_impl::the_subscriber(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::Subscriber_ptr subscriber = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*may return null*/
    subscriber = DOWNCAST_DDS_SUBSCRIBER(
        DK_CacheAdmin_ts_getLSSubscriber(cache, &exception, NULL));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return subscriber;
}

::CORBA::Boolean
DDS::Cache_impl::updates_enabled(
    ) THROW_ORB_EXCEPTIONS
{
    ::CORBA::Boolean enabled = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    enabled = static_cast<CORBA::Boolean>(
        DK_CacheAdmin_ts_getUpdatesEnabled(cache, &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return enabled;
}

void
DDS::Cache_impl::register_all_for_pubsub(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::BadHomeDefinition,
        DDS::DCPSError,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_registerAllForPubSub(cache, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

void
DDS::Cache_impl::enable_all_for_pubsub(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::DCPSError,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_enableAllForPubSub(cache, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

CORBA::Long
DDS::Cache_impl::register_home(
    DDS::ObjectHome_ptr a_home) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    CORBA::Long index = 0;
    DLRL_LS_object cppObject = NULL;
    DLRL_LS_object oldVal = NULL;
    DDS::ObjectHome_impl* homeImpl = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    homeImpl = dynamic_cast<DDS::ObjectHome_impl*>(a_home);
    DLRL_VERIFY_NOT_NULL(&exception, homeImpl, "a_home");

    if(!homeImpl->isManaged)
    {
        /* TODO when the binding of LS objects to kernel objects is improved,
         * this code can be simplified. See ticket dds300.
         */
        cppObject = UPCAST_DLRL_LS_OBJECT(CORBA::Object::_duplicate(a_home));
        /* now link the c++ object to the kernel objecthome */
        assert(cppObject);
        oldVal = DK_ObjectHomeAdmin_ts_registerLSObjectHome(
            homeImpl->home,
            &exception,
            cppObject);
        DLRL_Exception_PROPAGATE(&exception);
        assert(!oldVal);
        /* everything is ok so far, we will set cppObject to NULL now. This
         * prevents the cppObject from being release if an exception occured!
         */
        cppObject = NULL;
    }

    index = static_cast<CORBA::Long>(DK_CacheAdmin_ts_registerHome(
        cache,
        &exception,
        homeImpl->home));
    /* before propagating the exception, we need to undo the duplicate of
     * the home pointer if an exception occurred
     */
    if(exception.exceptionID != DLRL_NO_EXCEPTION)
    {
        DLRL_Exception exception2;
        DLRL_Exception_init(&exception2);
        cppObject = DK_ObjectHomeAdmin_ts_registerLSObjectHome(
            homeImpl->home,
            &exception2,
            NULL);
        /* if another exception occured, we must report it to an output file */
        if(exception2.exceptionID != DLRL_NO_EXCEPTION)
        {
            DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to "
            "unregister the c++ object home from the kernel object home.\n%s",
            DLRL_Exception_exceptionIDToString(exception.exceptionID),
            exception.exceptionMessage);
        }
    }
    DLRL_Exception_PROPAGATE(&exception);

    /* if everything went ok this far, then it means the home is now managed
     * by the kernel and we need to prevent that during the destructor of the home
     * the delete is called which would cause a deadlock
     */
    homeImpl->isManaged = true;

    DLRL_Exception_EXIT(&exception);
    if(cppObject)
    {
        CORBA::release(DOWNCAST_DDS_OBJECTHOME(cppObject));
        cppObject = NULL;
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return index;
}

DDS::ObjectHome_ptr
DDS::Cache_impl::find_home_by_name(
    const char * class_name) THROW_ORB_EXCEPTIONS
{
    DDS::ObjectHome_ptr home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, class_name, "class_name");
    home = DOWNCAST_DDS_OBJECTHOME(
        DK_CacheAdmin_ts_findLSHomeByName(
        cache,
        &exception,
        NULL,
        reinterpret_cast<LOC_const_string>(class_name)));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return home;
}

DDS::ObjectHome_ptr
DDS::Cache_impl::find_home_by_index(
    ::CORBA::Long index) THROW_ORB_EXCEPTIONS
{
    DDS::ObjectHome_ptr home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    home = DOWNCAST_DDS_OBJECTHOME(
        DK_CacheAdmin_ts_findLSHomeByIndex(
        cache,
        &exception,
        NULL,
        static_cast<LOC_long>(index)));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return home;
}

CORBA::Boolean
DDS::Cache_impl::attach_listener(
    DDS::CacheListener_ptr listener) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    CORBA::Boolean result = FALSE;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, listener, "listener");
    result = DK_CacheAdmin_ts_attachListener(
        cache,
        &exception,
        NULL,
        UPCAST_DLRL_LS_OBJECT(DDS::CacheListener::_duplicate(listener)));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);

    return result;
}

CORBA::Boolean
DDS::Cache_impl::detach_listener(
    DDS::CacheListener_ptr listener) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    CORBA::Boolean result = FALSE;
    DLRL_LS_object ls_listener = NULL;
    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, listener, "listener");
    ls_listener = DK_CacheAdmin_ts_detachListener(
        cache,
        &exception,
        NULL,
        UPCAST_DLRL_LS_OBJECT(listener));
    DLRL_Exception_PROPAGATE(&exception);
    if(ls_listener){
        CORBA::release(DOWNCAST_DDS_CACHELISTENER(ls_listener));
        result = TRUE;
    }
    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);

    return result;
}

void
DDS::Cache_impl::enable_updates(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_enableUpdates(cache, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

void
DDS::Cache_impl::disable_updates(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_disableUpdates(cache, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

DDS::CacheAccess_ptr
DDS::Cache_impl::create_access(
    DDS::CacheUsage purpose) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DK_CacheAccessAdmin* accessAdmin = NULL;
    DDS::CacheAccess_impl* access = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    access = new DDS::CacheAccess_impl();
    DLRL_VERIFY_ALLOC(access, &exception, "Unable to allocate memory");
    DDS::CacheAccess::_duplicate(access);/*TODO maybe do it (dup) inside kernel see dds300*/
    accessAdmin = DK_CacheAdmin_ts_createAccess(
        cache,
        &exception,
        NULL,
        UPCAST_DLRL_LS_OBJECT(access),
        static_cast<DK_Usage>(purpose));
    DLRL_Exception_PROPAGATE(&exception);
    access->access = accessAdmin;/*takes over ref from the create*/

    DLRL_Exception_EXIT(&exception);
    if(exception.exceptionID != DLRL_NO_EXCEPTION){
        if(access){
            CORBA::release(access);/*the duplicate release*/
            CORBA::release(access);/*the new release*/
        }
        if(accessAdmin){
            DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(accessAdmin));
        }
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return access;
}

void
DDS::Cache_impl::delete_access(
    DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;
    DDS::CacheAccess_impl* accessImpl = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    accessImpl = dynamic_cast< DDS::CacheAccess_impl*>(access);
    DLRL_VERIFY_NOT_NULL(&exception, accessImpl, "access");
    DK_CacheAdmin_ts_deleteAccess(cache, &exception, NULL, accessImpl->access);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

DDS::ObjectRootSeq*
DDS::Cache_impl::objects(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::ObjectRootSeq* rootSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_getObjects(
        cache,
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

DDS::CacheUsage
DDS::Cache_impl::cache_usage(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::CacheUsage usage = READ_ONLY;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    usage = static_cast<DDS::CacheUsage>(DK_CacheAdmin_ts_getUsage(
        cache,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return usage;
}

void
DDS::Cache_impl::refresh(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::DCPSError,
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_refresh(cache, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

DDS::ObjectHomeSeq*
DDS::Cache_impl::homes(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::ObjectHomeSeq* homesSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_getLSHomes(
        cache,
        &exception,
        NULL,
        reinterpret_cast<void**>(&homesSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!homesSeq){
        homesSeq = new DDS::ObjectHomeSeq(0);
        DLRL_VERIFY_ALLOC(homesSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return homesSeq;
}

DDS::CacheAccessSeq *
DDS::Cache_impl::sub_accesses(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::CacheAccessSeq* accessSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_getLSAccesses(
        cache,
        &exception,
        NULL,
        reinterpret_cast<void**>(&accessSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!accessSeq){
        accessSeq = new DDS::CacheAccessSeq(0);
        DLRL_VERIFY_ALLOC(accessSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return accessSeq;
}

DDS::CacheListenerSeq *
DDS::Cache_impl::listeners(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    DDS::CacheListenerSeq* listenerSeq = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_CacheAdmin_ts_getListeners(
        cache,
        &exception,
        NULL,
        reinterpret_cast<void**>(&listenerSeq));
    DLRL_Exception_PROPAGATE(&exception);
    if(!listenerSeq){
        listenerSeq = new DDS::CacheListenerSeq(0);
        DLRL_VERIFY_ALLOC(listenerSeq, &exception, "Unable to allocate memory");
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return listenerSeq;
}
