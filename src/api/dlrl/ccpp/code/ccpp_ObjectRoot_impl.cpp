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
#include "ccpp_ObjectRoot_impl.h"
#include "DLRL_Report.h"

DDS::ObjectRoot_impl::ObjectRoot_impl() : object(NULL)
{
    DLRL_INFO(INF_ENTER);

    myOid.systemId = 0;
    myOid.localId = 0;
    myOid.serial = 0;
    prevTopicValid = false;

    DLRL_INFO(INF_EXIT);
}

DDS::ObjectRoot_impl::~ObjectRoot_impl()
{
    DLRL_INFO(INF_ENTER);
    if(object){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(object));
        object = NULL;
    }
    DLRL_INFO(INF_EXIT);
}

DK_ObjectAdmin*
DDS::ObjectRoot_impl::getObjectRootAdmin(
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

DK_MapAdmin*
DDS::ObjectRoot_impl::getIntMapAdmin(
    DLRL_Exception* exception,
    DDS::IntMap* collection,
    const char* name)
{
    DDS::IntMap_impl* rootImpl;
    DK_MapAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(collection);
    assert(name);

    rootImpl = dynamic_cast<DDS::IntMap_impl*>(collection);
    if(!rootImpl)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Invalid inheritance hierarchy.",
            name);
    }
    admin = rootImpl->map;
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

DK_MapAdmin*
DDS::ObjectRoot_impl::getStrMapAdmin(
    DLRL_Exception* exception,
    DDS::StrMap* collection,
    const char* name)
{
    DDS::StrMap_impl* rootImpl;
    DK_MapAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(collection);
    assert(name);

    rootImpl = dynamic_cast<DDS::StrMap_impl*>(collection);
    if(!rootImpl)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Invalid inheritance hierarchy.",
            name);
    }
    admin = rootImpl->map;
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

DK_SetAdmin*
DDS::ObjectRoot_impl::getSetAdmin(
    DLRL_Exception* exception,
    DDS::Set* collection,
    const char* name)
{
    DDS::Set_impl* rootImpl;
    DK_SetAdmin* admin;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(collection);
    assert(name);

    rootImpl = dynamic_cast<DDS::Set_impl*>(collection);
    if(!rootImpl)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s parameter does "
            "not represent a valid DLRL object. Invalid inheritance hierarchy.",
            name);
    }
    admin = rootImpl->set;
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
DDS::ObjectRoot_impl::m_oid(
    const DDS::DLRLOid & oid) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    /* not yet supported */
}

const DDS::DLRLOid &
DDS::ObjectRoot_impl::m_oid(
    void) const THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DLRL_INFO(INF_ENTER);

    /* not yet supported */

    DLRL_INFO(INF_EXIT);
    return myOid;
}

DDS::DLRLOid &
DDS::ObjectRoot_impl::m_oid(
    void) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DLRL_INFO(INF_ENTER);

    /* not yet supported */

    DLRL_INFO(INF_EXIT);
    return myOid;
}

void
DDS::ObjectRoot_impl::m_class_name(
    char * class_name) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    /* not yet supported */
}

void
DDS::ObjectRoot_impl::m_class_name(
    const char * class_name) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    /* not yet supported */
}

void
DDS::ObjectRoot_impl::m_class_name(
    const CORBA::String_var& class_name) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    /* not yet supported */
}

const char *
DDS::ObjectRoot_impl::m_class_name (
    void) const THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DLRL_INFO(INF_ENTER);

    /* not yet supported */

    DLRL_INFO(INF_EXIT);
    return NULL;
}

DDS::DLRLOid
DDS::ObjectRoot_impl::oid(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DK_ObjectID oid;/* on stack def */
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    if( myOid.systemId == 0 &&
        myOid.localId == 0 &&
        myOid.serial == 0)
    {
        DK_ObjectAdmin_ts_getObjectID(object, &exception, &oid);
        DLRL_Exception_PROPAGATE(&exception);
        myOid.systemId = oid.oid[0];
        myOid.localId = oid.oid[1];
        myOid.serial = oid.oid[2];
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return myOid;
}

CORBA::Long
DDS::ObjectRoot_impl::home_index(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    CORBA::Long index = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    index = static_cast<CORBA::Long>(DK_ObjectAdmin_ts_getHomeIndex(
        object,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return index;
}

DDS::ObjectState
DDS::ObjectRoot_impl::read_state(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DDS::ObjectState state = OBJECT_VOID;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    state = static_cast<DDS::ObjectState>(DK_ObjectAdmin_ts_getReadState(
        object,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return state;
}

DDS::ObjectState
DDS::ObjectRoot_impl::write_state (
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DDS::ObjectState state = OBJECT_VOID;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    state = static_cast<DDS::ObjectState>(DK_ObjectAdmin_ts_getWriteState(
        object,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return state;
}

DDS::ObjectHome_ptr
DDS::ObjectRoot_impl::object_home(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DK_ObjectHomeAdmin* homeAdmin = NULL;
    DDS::ObjectHome_ptr home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    homeAdmin = DK_ObjectAdmin_ts_getHome(object, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    home = DOWNCAST_DDS_OBJECTHOME(DK_ObjectAdmin_ts_getLSHome(
        object,
        &exception,
        NULL));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(homeAdmin){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(homeAdmin));
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return home;
}

/* TODO solution manner one, see cacheaccess->owner for manner two which is more
 * efficient in code size
 */
DDS::CacheBase_ptr
DDS::ObjectRoot_impl::owner(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DK_CacheBase* ownerAdmin = NULL;
    DDS::CacheBase_ptr owner = NULL;
    DDS::CacheKind kind = CACHE_KIND;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    ownerAdmin = DK_ObjectAdmin_ts_getOwner(object, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    kind = static_cast<DDS::CacheKind>(DK_CacheBase_ts_getKind(
        ownerAdmin,
        &exception));
    DLRL_Exception_PROPAGATE(&exception);
    if(kind == CACHE_KIND){
        owner = DOWNCAST_DDS_CACHEBASE(
            DK_CacheAdmin_ts_getLSCache(
                reinterpret_cast<DK_CacheAdmin*>(ownerAdmin),
                &exception,
                NULL));
        DLRL_Exception_PROPAGATE(&exception);
    } else {
        assert(kind == CACHEACCESS_KIND);
        owner = DOWNCAST_DDS_CACHEBASE(
            DK_CacheAccessAdmin_ts_getLSAccess(
                reinterpret_cast<DK_CacheAccessAdmin*>(ownerAdmin),
                &exception,
                NULL));
        DLRL_Exception_PROPAGATE(&exception);

    }

    DLRL_Exception_EXIT(&exception);
    if(ownerAdmin){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(ownerAdmin));
    }
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return owner;
}

void
DDS::ObjectRoot_impl::destroy(
    ) THROW_VALUETYPE_ORB_AND_USER_EXCEPTIONS(
        DDS::PreconditionNotMet)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_ObjectAdmin_ts_destroy(object, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

CORBA::Boolean
DDS::ObjectRoot_impl::is_modified(
    DDS::ObjectScope scope) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    ::CORBA::Boolean isModified = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    isModified = static_cast<CORBA::Boolean>(DK_ObjectAdmin_ts_isModified(
        object,
        &exception,
        static_cast<DK_ObjectScope>(scope)));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return isModified;
}

DDS::StringSeq *
DDS::ObjectRoot_impl::get_invalid_relations(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    DDS::StringSeq * seq = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_ObjectAdmin_ts_getInvalidRelations(
        object,
        &exception,
        NULL,
        reinterpret_cast<void**>(&seq));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return seq;
}

CORBA::ValueBase*
DDS::ObjectRoot_impl::_copy_value(
    )
{
    DLRL_INFO(INF_ENTER);
    /*dummy implementation*/

    DLRL_INFO(INF_EXIT);
    return NULL;
}
