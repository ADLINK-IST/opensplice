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

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_objManag.h"
#include "sac_report.h"


static DDS_ReturnCode_t
DDS_Object_init (
    DDS_Object _this,
    DDS_ObjectKind kind,
    _Object_destructor_t destructor)
{
    DDS_ReturnCode_t result;
    _Object object;
    os_result osr;

    object = _Object(_this);
    if (object != NULL) {
        object->destructor = destructor;
        object->kind = kind;
        object->domainId = DDS_DOMAIN_ID_INVALID;
        osr = os_mutexInit(&object->mutex, NULL);
        if (osr == os_resultSuccess) {
            osr = os_condInit(&object->cond, &object->mutex, NULL);
        }
        if (osr != os_resultSuccess) {
            result = DDS_RETCODE_ERROR;
        } else {
            result = DDS_RETCODE_OK;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Object (0x%x) seems to have a corrupted state", _this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Object_check (
    DDS_Object _this,
    DDS_ObjectKind kind)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Object o;

    if((_this == NULL) || (DDS__header(_this) == NULL)) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Object (0x%x) is not a valid %s pointer",
                    _this, DDS_ObjectKind_image(kind));
    } else {
        o = _Object(_this);
        switch (kind) {
        case DDS_ENTITY:
            if ((o->kind < DDS_ENTITY) ||
                (o->kind > DDS_MULTITOPIC))
            {
                result = DDS_RETCODE_BAD_PARAMETER;
            }
        break;
        case DDS_CONDITION:
            if ((o->kind < DDS_CONDITION) ||
                (o->kind > DDS_QUERYCONDITION))
            {
                result = DDS_RETCODE_BAD_PARAMETER;
            }
        break;
        case DDS_READCONDITION:
            if ((o->kind < DDS_READCONDITION) ||
                (o->kind > DDS_QUERYCONDITION))
            {
                result = DDS_RETCODE_BAD_PARAMETER;
            }
        break;
        case DDS_TOPICDESCRIPTION:
            if ((o->kind < DDS_TOPICDESCRIPTION) ||
                (o->kind > DDS_MULTITOPIC))
            {
                result = DDS_RETCODE_BAD_PARAMETER;
            }
        break;
        default:
            if (o->kind != kind) {
                if (o->kind == DDS_UNDEFINED) {
                    result = DDS_RETCODE_ALREADY_DELETED;
                } else {
                    result = DDS_RETCODE_ILLEGAL_OPERATION;
                }
            }
        }
        if (result != DDS_RETCODE_OK) {
            SAC_REPORT(result, "Object pointed to by 0x%x is of kind %s, not of the expected kind %s",
                        _this, DDS_ObjectKind_image(o->kind), DDS_ObjectKind_image(kind));
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_Object_check_and_assign(
    DDS_Object _this,
    DDS_ObjectKind kind,
    _Object *object)
{
    DDS_ReturnCode_t result;

    result = DDS_Object_check(_this, kind);
    if (result == DDS_RETCODE_OK) {
        *object = _Object(_this);
    } else {
        SAC_REPORT(result, "Claim on object of type %s pointed to by 0x%x failed",
                    DDS_ObjectKind_image(kind), _this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Object_claim (
    DDS_Object _this,
    DDS_ObjectKind kind,
    _Object *object)
{
    DDS_ReturnCode_t result;
    _Object o;
    os_result osr;

    result = DDS_Object_check(_this, kind);
    if (result == DDS_RETCODE_OK) {
        o = _Object(_this);
        osr = os_mutexLock_s(&o->mutex);
        if (osr == os_resultSuccess) {
            /* While a thread is waiting on the lock the object can be deleted.
             * An extra check is performed here to verify if the object is still valid.
             * Note however when the object is deleted it's memory is freed. Thus this
             * extra check will not protect the application from accessing freed memory.
             * It will only limit the chance that this occurs.
             */
            if (o->kind != DDS_UNDEFINED) {
                *object = o;
                result = DDS_RETCODE_OK;
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "Claim of object of type %s pointed to by 0x%x failed",
                            DDS_ObjectKind_image(kind), _this);
            }
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Locking of object of type %s pointed to by 0x%x failed with result %s",
                        DDS_ObjectKind_image(kind), _this, os_resultImage(osr));
        }
    } else {
        SAC_REPORT(result, "Claim of object of type %s pointed to by 0x%x failed",
                    DDS_ObjectKind_image(kind), _this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Object_wait_wlReq (
    DDS_Object _this)
{
    DDS_ReturnCode_t result;
    _Object o;
    os_result osr;
    os_duration timeout = 500*OS_DURATION_SECOND;

    o = _Object(_this);
    osr = os_condTimedWait(&o->cond, &o->mutex, timeout);
    if (osr != os_resultSuccess) {
        result = DDS_RETCODE_ERROR;
        SAC_REPORT(result, "Object timed wait failed with error %s",
                   os_resultImage(osr));
    } else {
        result = DDS_RETCODE_OK;
    }
    return result;
}

DDS_ReturnCode_t
DDS_Object_trigger (
    DDS_Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Object object;
    os_result osr;

    object = _Object(_this);
    if ((object == NULL) ||
        (DDS__header(_this) == NULL) ||
        (object->kind <= DDS_UNDEFINED) ||
        (object->kind >= DDS_OBJECT_COUNT))
    {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Object (0x%x) is not a valid pointer", _this);
    } else {
        osr = os_mutexLock_s(&object->mutex);
        if (osr == os_resultSuccess) {
            os_condBroadcast(&object->cond);
            os_mutexUnlock(&object->mutex);
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Locking of object of type %s pointed to by 0x%x failed with result %s",
                        DDS_ObjectKind_image(object->kind), _this, os_resultImage(osr));
        }
    }
    return result;
}

DDS_ReturnCode_t
_Object_trigger_claimed(
    _Object object)
{
    os_condBroadcast(&object->cond);
    return DDS_RETCODE_OK;
}


DDS_ReturnCode_t
DDS_Object_release (
    DDS_Object _this)
{
    DDS_ReturnCode_t result;
    _Object object;

    object = _Object(_this);
    if ((object == NULL) ||
        (DDS__header(_this) == NULL) ||
        (object->kind <= DDS_UNDEFINED) ||
        (object->kind >= DDS_OBJECT_COUNT))
    {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Object (0x%x) is not a valid pointer", _this);
    } else {
        os_mutexUnlock(&object->mutex);
        result = DDS_RETCODE_OK;
    }
    return result;
}

static DDS_ReturnCode_t
DDS_Object_deinit (
    DDS_Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_ObjectKind kind;
    _Object object;
    os_result osr;

    object = _Object(_this);
    if ((object == NULL) ||
        (DDS__header(_this) == NULL) ||
        (object->kind <= DDS_UNDEFINED) ||
        (object->kind >= DDS_OBJECT_COUNT))
    {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Object (0x%x) is not a valid pointer", _this);
    } else {
        osr = os_mutexLock_s(&object->mutex);
        kind = object->kind;
        if (osr == os_resultSuccess) {
            if (object->destructor) {
                result = object->destructor(object);
            }
            if (result == DDS_RETCODE_OK) {
                object->kind = DDS_UNDEFINED;
            }
            os_mutexUnlock(&object->mutex);
            if (result == DDS_RETCODE_OK) {
                os_condDestroy (&object->cond);
                os_mutexDestroy(&object->mutex);
            }
            if ((kind == DDS_WAITSET)        ||
                (kind == DDS_TYPESUPPORT)    ||
                (kind == DDS_GUARDCONDITION)) {
                os_osExit();
            }
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Locking of object of type %s pointed to by 0x%x failed with result %s",
                        DDS_ObjectKind_image(object->kind), _this, os_resultImage(osr));
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_Object_new (
    DDS_ObjectKind kind,
    _Object_destructor_t destructor,
    DDS_Object *object)
{
    DDS_ReturnCode_t result;
    DDS_long size;

    if (object != NULL) {
        result = DDS_RETCODE_OK;
        switch (kind) {
        case DDS_DOMAINFACTORY:
            size = sizeof(C_STRUCT(_DomainParticipantFactory));
        break;
        case DDS_DOMAINPARTICIPANT:
            size = sizeof(C_STRUCT(_DomainParticipant));
        break;
        case DDS_DOMAIN:
            size = sizeof(C_STRUCT(_Domain));
        break;
        case DDS_PUBLISHER:
            size = sizeof(C_STRUCT(_Publisher));
        break;
        case DDS_SUBSCRIBER:
            size = sizeof(C_STRUCT(_Subscriber));
        break;
        case DDS_DATAWRITER:
            size = sizeof(C_STRUCT(_DataWriter));
        break;
        case DDS_DATAREADER:
            size = sizeof(C_STRUCT(_DataReader));
        break;
        case DDS_DATAREADERVIEW:
            size = sizeof(C_STRUCT(_DataReaderView));
        break;
        case DDS_TOPIC:
            size = sizeof(C_STRUCT(_Topic));
        break;
        case DDS_CONTENTFILTEREDTOPIC:
            size = sizeof(C_STRUCT(_ContentFilteredTopic));
        break;
        case DDS_TYPESUPPORT:
            os_osInit();
            size = sizeof(C_STRUCT(_TypeSupport));
        break;
        case DDS_GUARDCONDITION:
            os_osInit();
            size = sizeof(C_STRUCT(_GuardCondition));
        break;
        case DDS_READCONDITION:
            size = sizeof(C_STRUCT(_ReadCondition));
        break;
        case DDS_QUERYCONDITION:
            size = sizeof(C_STRUCT(_QueryCondition));
        break;
        case DDS_STATUSCONDITION:
            size = sizeof(C_STRUCT(_StatusCondition));
        break;
        case DDS_WAITSET:
            os_osInit();
            size = sizeof(C_STRUCT(_WaitSet));
        break;
        case DDS_ERRORINFO:
            size = sizeof(C_STRUCT(_ErrorInfo));
        break;
        case DDS_QOSPROVIDER:
            size = sizeof(C_STRUCT(_QosProvider));
        break;
        default:
            *object = NULL;
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Invalid object kind (%d) specified", kind);
        break;
        }
        if (result == DDS_RETCODE_OK) {
            *object = DDS_alloc(size, (DDS_deallocatorType)DDS_Object_deinit);
            if (*object != NULL) {
                result = DDS_Object_init(*object, kind, destructor);
            } else {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
            }
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Object holder (0x%x) is not a valid pointer", object);
    }
    return result;
}

DDS_ObjectKind
DDS_Object_get_kind(
    DDS_Object _this)
{
    DDS_ObjectKind kind = DDS_UNDEFINED;
    _Object object;
    os_result osr;

    object = _Object(_this);
    if ((object != NULL) &&
        (DDS__header(_this) != NULL) &&
        (object->kind > DDS_UNDEFINED) &&
        (object->kind < DDS_OBJECT_COUNT))
    {
        osr = os_mutexLock_s(&object->mutex);
        if (osr == os_resultSuccess) {
            kind = object->kind;
            os_mutexUnlock(&object->mutex);
        } else {
            SAC_REPORT(DDS_RETCODE_ERROR, "Locking of object of type %s pointed to by 0x%x failed with result %s",
                        DDS_ObjectKind_image(object->kind), _this, os_resultImage(osr));
        }
    } else {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Object (0x%x) is not a valid pointer", _this);
    }
    return kind;
}

os_int32
DDS_Object_get_domain_id(
    DDS_Object _this)
{
	_Object object = _Object(_this);

	if ((DDS__header(_this) != NULL) &&
	    (object->kind > DDS_UNDEFINED) &&
		(object->kind < DDS_OBJECT_COUNT)) {
        return object->domainId;
    }
    return DDS_DOMAIN_ID_INVALID;
}

void
DDS_Object_set_domain_id(
    _Object object,
    os_int32 domainId)
{
    object->domainId = domainId;
}
