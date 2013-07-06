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
#include "ccpp_ObjectHome_impl.h"
#include "ccpp_ObjectRoot_impl.h"
#include "ccpp_Selection_impl.h"
#include "ccpp_CacheAccess_impl.h"
#include "DLRL_Report.h"

DDS::ObjectHome_impl::ObjectHome_impl() : home(NULL), isManaged(false)
{
}


DDS::ObjectHome_impl::~ObjectHome_impl()
{
    DLRL_INFO(INF_ENTER);
    /* If this c++ object home is linked to a kernel object home we must
     * release it's pointer and set it to null. We also need to call the delete
     * function before we release it to ensure the home resources are cleaned.
     * If the resources were already cleaned by the DLRL kernel then the delete
     * operation will behave as a no-op, however if this home was never
     * registered to a cache, then this way is the only way to clean it's
     * resources.
     */
    if(home)
    {
        if(!isManaged)
        {
            DK_ObjectHomeAdmin_ts_delete(home, NULL);
        }
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(home));
        home = NULL;
    }
    DLRL_INFO(INF_EXIT);
}

char *
DDS::ObjectHome_impl::name(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;
    LOC_string name = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    name = DK_ObjectHomeAdmin_ts_getName(
        home,
        &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return name;
}

char *
DDS::ObjectHome_impl::content_filter(
    ) THROW_ORB_EXCEPTIONS
{
    return NULL;
}

DDS::ObjectHome_ptr
DDS::ObjectHome_impl::parent(
    ) THROW_ORB_EXCEPTIONS
{
    return NULL;
}

DDS::ObjectHomeSeq *
DDS::ObjectHome_impl::children(
    ) THROW_ORB_EXCEPTIONS
{
    return new DDS::ObjectHomeSeq(0);
}

CORBA::Long
DDS::ObjectHome_impl::registration_index(
    ) THROW_ORB_EXCEPTIONS
{
    CORBA::Long index = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    index = static_cast<CORBA::Long>(DK_ObjectHomeAdmin_ts_getRegistrationIndex(
        home,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return index;
}

CORBA::Boolean
DDS::ObjectHome_impl::auto_deref(
    ) THROW_ORB_EXCEPTIONS
{
    CORBA::Boolean autoDeref = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    autoDeref = static_cast<CORBA::Boolean>(
        DK_ObjectHomeAdmin_ts_getAutoDeref(home, &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return autoDeref;
}

void
DDS::ObjectHome_impl::set_content_filter(
    const char * expression) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::SQLError,
        DDS::PreconditionNotMet)
{

}

void
DDS::ObjectHome_impl::set_auto_deref(
    ::CORBA::Boolean value) THROW_ORB_EXCEPTIONS
{

}

void
DDS::ObjectHome_impl::deref_all(
    ) THROW_ORB_EXCEPTIONS
{

}

void
DDS::ObjectHome_impl::underef_all(
    ) THROW_ORB_EXCEPTIONS
{

}

DDS::DataReader_ptr
DDS::ObjectHome_impl::get_datareader(
     const char * topic_name) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DDS::DataReader_ptr retVal = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, topic_name, "topic_name");

    retVal = DOWNCAST_DDS_READER(DK_ObjectHomeAdmin_ts_getLSDataReader(
        home,
        &exception,
        NULL,
        topic_name));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return retVal;
}

DDS::DataWriter_ptr
DDS::ObjectHome_impl::get_datawriter(
     const char * topic_name) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DDS::DataWriter_ptr retVal = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, topic_name, "topic_name");

    retVal = DOWNCAST_DDS_WRITER(DK_ObjectHomeAdmin_ts_getLSDataWriter(
        home,
        &exception,
        NULL,
        topic_name));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return retVal;
}

DDS::Topic_ptr
DDS::ObjectHome_impl::get_topic(
     const char * topic_name) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DDS::Topic_ptr retVal = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, topic_name, "topic_name");

    retVal = DOWNCAST_DDS_TOPIC(DK_ObjectHomeAdmin_ts_getLSTopic(
        home,
        &exception,
        NULL,
        topic_name));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return retVal;
}

char *
DDS::ObjectHome_impl::get_topic_name(
    const char * attribute_name) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    LOC_string topicName = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, attribute_name, "attribute_name");

    topicName = DK_ObjectHomeAdmin_ts_getTopicName(
        home,
        &exception,
        attribute_name);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return topicName;
}

DDS::StringSeq *
DDS::ObjectHome_impl::get_all_topic_names(
    ) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DDS::StringSeq* seq = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_ObjectHomeAdmin_ts_getAllTopicNames(
        home,
        &exception,
        NULL,
        reinterpret_cast<void**>(&seq));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return seq;
}

DK_ObjectAdmin*
DDS::ObjectHome_impl::getObjectRootAdmin(
    DLRL_Exception* exception,
    DDS::ObjectRoot* objectRoot,
    const char* name)
{
    DDS::ObjectRoot_impl* rootImpl;
    DK_ObjectAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(objectRoot);
    assert(name);

    rootImpl = dynamic_cast<DDS::ObjectRoot_impl*>(objectRoot);
    if(!rootImpl)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Invalid inheritance hierarchy.",
            name);
    }
    admin = rootImpl->object;
    if(!admin)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Unable to locate DLRL kernel "
            "object.", name);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return admin;
}

DK_CacheAccessAdmin*
DDS::ObjectHome_impl::getCacheAccessAdmin(
    DLRL_Exception* exception,
    DDS::CacheAccess_ptr access,
    const char* name)
{
    DDS::CacheAccess_impl* rootImpl;
    DK_CacheAccessAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(access);
    assert(name);

    rootImpl = dynamic_cast<DDS::CacheAccess_impl*>(access);
    if(!rootImpl)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Invalid inheritance hierarchy.",
            name);
    }
    admin = rootImpl->access;
    if(!admin)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Unable to locate DLRL kernel "
            "object.", name);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return admin;
}

DK_SelectionAdmin*
DDS::ObjectHome_impl::getSelectionAdmin(
    DLRL_Exception* exception,
    DDS::Selection_ptr selection,
    const char* name)
{
    DDS::Selection_impl* rootImpl;
    DK_SelectionAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(selection);
    assert(name);

    rootImpl = dynamic_cast<DDS::Selection_impl*>(selection);
    if(!rootImpl)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Invalid inheritance hierarchy.",
            name);
    }
    admin = rootImpl->selection;
    if(!admin)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Unable to locate DLRL kernel "
            "object.", name);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return admin;
}

void
DDS::ObjectHome_impl::setSelection(
    DDS::Selection_impl* cpp_selection,
    DK_SelectionAdmin* selection)
{
    DLRL_INFO(INF_ENTER);
    cpp_selection->selection = selection;
    DLRL_INFO(INF_EXIT);
}

