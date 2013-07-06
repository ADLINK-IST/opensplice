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
#include "ccpp_Set_impl.h"
#include "ccpp_DlrlUtils.h"
#include "DLRL_Report.h"
#include "ccpp_ObjectRoot_impl.h"

DDS::Set_impl::Set_impl(DK_SetAdmin* collection)
{
    DLRL_INFO(INF_ENTER);

    DK_Entity_ts_duplicate(reinterpret_cast<DK_Entity*>(collection));
    set = collection;

    DLRL_INFO(INF_EXIT);
}

DDS::Set_impl::~Set_impl()
{
    DLRL_INFO(INF_ENTER);
    if(set){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(set));
        set = NULL;
    }
    DLRL_INFO(INF_EXIT);
}

CORBA::Long
DDS::Set_impl::length(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    CORBA::Long length = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    length =static_cast<CORBA::Long>(DK_SetAdmin_ts_getLength(set, &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return length;
}

void
DDS::Set_impl::clear(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_SetAdmin_ts_clear(set, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

CORBA::ValueBase*
DDS::Set_impl::_copy_value(
    )
{
    DLRL_INFO(INF_ENTER);
    /*dummy implementation*/

    DLRL_INFO(INF_EXIT);
    return NULL;
}

DK_ObjectAdmin*
DDS::Set_impl::getObjectRootAdmin(
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
