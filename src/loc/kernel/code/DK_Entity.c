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

/* os abstraction layer includes */
#include "os_abstract.h"

/* DLRL utilities includes */
#include "DLRL_Report.h"

/* DLRL kernel includes */
#include "DK_Entity.h"

static LOC_string
DK_Types_getClassNameForClassID(
    DK_Class classID);

void
DK_Entity_us_init(
    DK_Entity* _this,
    DK_Class classID,
    void (* destroy)(
        DK_Entity *))
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(destroy);

    _this->destroy = destroy;
    _this->ref_count = 1;
    _this->classID = classID;
    DLRL_INFO(
        INF_REF_COUNT,
        "%s: %p initialised: %u",
        DK_Types_getClassNameForClassID(_this->classID),
        _this,
        _this->ref_count);

    DLRL_INFO(INF_EXIT);
}


DK_Entity*
DK_Entity_ts_duplicate(
    DK_Entity* _this)
{
    os_uint32 retVal;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    retVal = pa_increment(&(_this->ref_count));
    DLRL_INFO(
        INF_REF_COUNT,
        "%s: %p increased to %u",
        DK_Types_getClassNameForClassID(_this->classID),
        _this,
        retVal);

    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_Entity_ts_release(
    DK_Entity* _this)
{
    os_uint32 retVal;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    retVal = pa_decrement(&(_this->ref_count));
    DLRL_INFO(
        INF_REF_COUNT,
        "%s: %p decreased to %u",
        DK_Types_getClassNameForClassID(_this->classID),
        _this,
        retVal);
    if(retVal < 1)
    {
        _this->destroy(_this);
    }
    DLRL_INFO(INF_EXIT);
}

DK_Class
DK_Entity_getClassID(
    DK_Entity* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->classID;
}

LOC_string
DK_Types_getClassNameForClassID(
    DK_Class classID)
{
    LOC_string retVal= "??";

    DLRL_INFO(INF_ENTER);

    assert(classID < DK_Class_elements);
    switch (classID)
    {

    case DK_CLASS_CACHE_ACCESS_ADMIN:
        retVal = "CacheAccessAdmin ";
        break;
    case DK_CLASS_CACHE_ADMIN:
        retVal = "CacheAdmin          ";
        break;
    case DK_CLASS_OBJECT_ADMIN:
        retVal = "ObjectAdmin         ";
        break;
    case DK_CLASS_OBJECT_HOLDER:
        retVal = "ObjectHolder        ";
        break;
    case DK_CLASS_OBJECT_HOME_ADMIN:
        retVal = "ObjectHomeAdmin     ";
        break;
    case DK_CLASS_OBJECT_READER:
        retVal = "ObjectReader        ";
        break;
    case DK_CLASS_OBJECT_WRITER:
        retVal = "ObjectWriter        ";
        break;
    case DK_CLASS_TOPIC_INFO:
        retVal = "TopicInfo           ";
        break;
    case DK_CLASS_COLLECTION_READER:
        retVal = "CollectionReader    ";
        break;
    case DK_CLASS_SET_ADMIN:
        retVal = "SetAdmin            ";
        break;
    case DK_CLASS_MAP_ADMIN:
        retVal = "MapAdmin            ";
        break;
    case DK_CLASS_SELECTION_ADMIN:
        retVal = "SelectionAdmin      ";
        break;
    case DK_CLASS_EVENT_DISPATCHER:
        retVal = "EventDispatcher     ";
        break;
    case DK_Class_elements:
        retVal = "??????              ";
        break;
    default:
        retVal = "???                 ";
        break;
    }
    DLRL_INFO(INF_EXIT);
    return retVal;
}
