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
#include "v_kernel.h"
#include "v_status.h"
#include "u_entity.h"
#include "u_observable.h"

#include "Entity.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "ListenerDispatcher.h"


DDS::OpenSplice::Entity::Entity(
    ObjectKind kind
) : DDS::OpenSplice::CppSuperClass(kind),
    uEntity(NULL),
    interest(0),
    statusCondition(NULL),
    handle(U_INSTANCEHANDLE_NIL),
    listenerDispatcher(NULL),
    listenerEnabled(false),
    listener(NULL),
    listenerMask(0)
{
    /* do nothing */
}

DDS::OpenSplice::Entity::~Entity()
{
    if (this->uEntity) {
        u_objectFree(u_object(this->uEntity));
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::nlReq_init(
    u_entity uEntity)
{
    DDS::ReturnCode_t result;

    assert(uEntity);

    result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        this->uEntity = uEntity;
        this->handle = u_entityGetInstanceHandle(uEntity);
        if (u_observableGetY2038Ready(u_observable(uEntity))) {
            this->maxSupportedSeconds = OS_TIME_MAX_VALID_SECONDS;
        } else {
            this->maxSupportedSeconds = INT32_MAX;
        }
        if (u_observableSetUserData(u_observable(uEntity), this)) {
            result = DDS::RETCODE_ERROR;
            CPP_REPORT(result, "Could not initialize Entity.");
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::wlReq_deinit()
{
    DDS::ReturnCode_t result;
    u_result uResult;

    if (this->statusCondition) {
        this->statusCondition->deinit();
        DDS::release(this->statusCondition);
        this->statusCondition = NULL;
    }
    if (this->uEntity == NULL) {
        result = DDS::RETCODE_OK;
    } else {
        uResult = u_objectClose(u_object(this->uEntity));
        if (uResult == U_RESULT_ALREADY_DELETED) {
            /* It can happen that the u_object is already deleted when,
             *    1) this DDS::Entity is a global object and
             *    2) the atExit of the userlayer was already called.
             * It is the defined sequence for c++ shutdown that first
             * the atExit is called after which the global objects are
             * destroyed.
             * So, an U_RESULT_ALREADY_DELETED at this moment is an
             * valid result. */
            result = DDS::RETCODE_OK;
        } else {
            result = uResultToReturnCode(uResult);
        }
    }
    if (result == DDS::RETCODE_OK) {
        this->interest = 0;
        this->listener = NULL;
        this->listenerDispatcher = NULL;
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::CppSuperClass::wlReq_deinit();
    }
    return result;
}

DDS::Boolean
DDS::OpenSplice::Entity::rlReq_is_enabled()
{
    return u_entityEnabled (this->uEntity);
}

u_entity
DDS::OpenSplice::Entity::rlReq_get_user_entity()
{
    return this->uEntity;
}

/*
 * This function should only be used by DBTs.
 */
u_entity
DDS::OpenSplice::Entity::nlReq_get_user_entity_for_test()
{
    u_entity entity = NULL;
    if (this->check() == DDS::RETCODE_OK) {
        entity = this->uEntity;
    }
    return entity;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::enable (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_result uResult;

    CPP_REPORT_STACK();
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        if (this->uEntity == NULL) {
            result = DDS::RETCODE_ERROR;
        } else {
            uResult = u_entityEnable (this->uEntity);
            result = uResultToReturnCode (uResult);
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);
    return result;
}

DDS::StatusCondition_ptr
DDS::OpenSplice::Entity::get_statuscondition (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::StatusCondition_ptr condition = NULL;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->statusCondition == NULL) {
            this->statusCondition = new DDS::OpenSplice::StatusCondition ();
            if (this->statusCondition == NULL) {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not create StatusCondition.");
            } else {
                result = this->statusCondition->nlReq_init (this);
                if (result != DDS::RETCODE_OK) {
                    DDS::release(this->statusCondition);
                    this->statusCondition = NULL;
                }
            }
        }
        condition = DDS::StatusCondition::_duplicate(this->statusCondition); /* Also works when condition is NULL */
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return condition;
}

static void
rlReq_getStatusMask(
    v_public p,
    c_voidp arg)
{
    DDS::StatusMask *mask = reinterpret_cast<DDS::StatusMask *>(arg);
    c_ulong vMask;
    v_kind vKind;

    vMask = v_statusGetMask(v_entity(p)->status);
    vKind = v_objectKind(v_object(p));

    *mask = DDS::OpenSplice::Utils::vEventMaskToStatusMask(vMask, vKind);
}

DDS::StatusMask
DDS::OpenSplice::Entity::get_status_changes (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::StatusMask mask = 0;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uResult = u_observableAction(u_observable(this->uEntity), &rlReq_getStatusMask, &mask);
        result = this->uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return mask;
}

DDS::InstanceHandle_t
DDS::OpenSplice::Entity::get_instance_handle (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::InstanceHandle_t handle;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        handle = this->handle;
    } else {
        handle = DDS::HANDLE_NIL;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return handle;
}

void
DDS::OpenSplice::Entity::wlReq_set_listenerDispatcher (
    cmn_listenerDispatcher dispatcher
) THROW_ORB_EXCEPTIONS
{
    /* Expect both set and reset only once. */
    if (this->listenerDispatcher == NULL) {
        assert(dispatcher != NULL);
        this->listenerDispatcher = dispatcher;
    } else {
        assert(dispatcher == NULL);
        this->listenerDispatcher = NULL;
    }
}

cmn_listenerDispatcher
DDS::OpenSplice::Entity::rlReq_get_listenerDispatcher (
) THROW_ORB_EXCEPTIONS
{
    return this->listenerDispatcher;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::wlReq_set_listener_mask (
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_result uResult;

    assert (this->listenerDispatcher != NULL);

    if (mask != 0) {
        result = (DDS::ReturnCode_t)cmn_listenerDispatcher_add (
            this->listenerDispatcher,
            this->uEntity, NULL, NULL,
            DDS::OpenSplice::Utils::vEventMaskFromStatusMask(mask));
        if (result == DDS::RETCODE_OK) {
            this->listenerEnabled = true;
        }
    } else {
        uResult = u_entitySetListener (this->uEntity, NULL, NULL, 0);
        result = uResultToReturnCode (uResult);
        if (result == DDS::RETCODE_OK) {
            result = this->wlReq_wait_listener_removed ();
            if (result == DDS::RETCODE_OK) {
                result = cmn_listenerDispatcher_remove (
                    this->listenerDispatcher, this->uEntity);
            }
        }
    }

    if (result == DDS::RETCODE_OK) {
        this->listenerMask = mask;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::set_listener_mask (
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = this->wlReq_set_listener_mask(mask);
        this->unlock();
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::nlReq_set_listener (
    DDS::Listener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = this->wlReq_set_listener(a_listener, mask);
        this->unlock();
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::wlReq_set_listener (
    DDS::Listener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Listener_ptr tmpListener;

    CPP_REPORT_STACK();
    result = wlReq_set_listener_mask(mask);
    if (result == DDS::RETCODE_OK) {
        tmpListener = this->listener;
        this->listener = DDS::Listener::_duplicate(a_listener);
        DDS::release(tmpListener);
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);
    return result;
}

DDS::Listener_ptr
DDS::OpenSplice::Entity::nlReq_get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::Listener_ptr a_listener = NULL;
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        a_listener = DDS::Listener::_duplicate(this->listener);
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);
    return a_listener;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::nlReq_notify_listener_removed()
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->listenerEnabled == true) {
            this->listenerEnabled = false;
            this->wlReq_trigger();
        }
        this->unlock();
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::wlReq_wait_listener_removed()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    while ((result == DDS::RETCODE_OK) && this->listenerEnabled == true) {
        result = this->wlReq_wait();
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::disable_callbacks()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (u_entityDisableCallbacks(this->uEntity)) {
        this->listenerEnabled = true;
        result = this->wlReq_wait_listener_removed();
    }

    return result;
}

static void
Entity_resetDataAvailable(
   v_public p,
   c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    v_statusReset(v_entity(p)->status, V_EVENT_DATA_AVAILABLE);
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::reset_dataAvailable_status (
) THROW_ORB_EXCEPTIONS
{
    u_result uResult;

/* TODO OSPL-8179: This is a bit tricky, the entity may already been deleted and in that case
 * this operation will perform a dirty memory read.
 * It may be better to wipe all pending events belonging to an entity when it is deleted or
 * if that is too intrusive find another way to safely detect/avoid deletion.
 * NOTE: This was copied from SAC.
 */
    uResult = u_observableAction(u_observable(this->uEntity),
                                 Entity_resetDataAvailable,
                                 NULL);
    return this->uResultToReturnCode(uResult);
}

static void
Entity_resetOnDataOnReaders(
   v_public p,
   c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    v_statusReset(v_entity(p)->status, V_EVENT_ON_DATA_ON_READERS);
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::reset_on_data_on_readers_status (
) THROW_ORB_EXCEPTIONS
{
    u_result uResult;

/* TODO OSPL-8179: This is a bit tricky, the entity may already been deleted and in that case
 * this operation will perform a dirty memory read.
 * It may be better to wipe all pending events belonging to an entity when it is deleted or
 * if that is too intrusive find another way to safely detect/avoid deletion.
 * NOTE: This was copied from SAC.
 */
    uResult = u_observableAction(u_observable(this->uEntity),
                                 Entity_resetOnDataOnReaders,
                                 NULL);
    return this->uResultToReturnCode(uResult);
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::get_property (
        DDS::Property & a_property
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = this->check();

    if (result == DDS::RETCODE_OK) {
        if (this->uEntity == NULL) {
            result = DDS::RETCODE_ERROR;
            CPP_REPORT(result, "Internal error.");
        } else if(a_property.name.in() == NULL){
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Supplied Property name is invalid.");
        } else {
            result = DDS::RETCODE_UNSUPPORTED;
            CPP_REPORT(result, "Function has not been implemented yet.");
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Entity::set_property (
    const ::DDS::Property & a_property
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = this->check();

    if (result == DDS::RETCODE_OK) {
        if (this->uEntity == NULL) {
            result = DDS::RETCODE_ERROR;
            CPP_REPORT(result, "Internal error.");
        } else if(a_property.name.in() == NULL){
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Supplied Property name is invalid.");
        } else if(a_property.value.in() == NULL){
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Supplied Property value is invalid.");
        } else {
            result = DDS::RETCODE_UNSUPPORTED;
            CPP_REPORT(result, "Function has not been implemented yet.");
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}
