/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "u_statusCondition.h"
#include "Constants.h"
#include "Entity.h"
#include "StatusCondition.h"
#include "WaitSet.h"
#include "StatusUtils.h"
#include "ReportUtils.h"

DDS::OpenSplice::StatusCondition::StatusCondition() :
    DDS::OpenSplice::Condition(DDS::OpenSplice::STATUSCONDITION),
    uCondition(NULL),
    entity(NULL),
    enabledStatusMask(DDS::STATUS_MASK_ANY_V1_2)
{
}

DDS::OpenSplice::StatusCondition::~StatusCondition( )
{
    if (this->uCondition) {
        u_objectFree (u_object (this->uCondition));
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::init(
    DDS::OpenSplice::Entity *entity)
{
    return nlReq_init(entity);
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::nlReq_init(
    DDS::OpenSplice::Entity *entity)
{
    DDS::ReturnCode_t result;
    u_entity uEntity;

    result = DDS::OpenSplice::Condition::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        uEntity = entity->rlReq_get_user_entity();
        this->uCondition = u_statusConditionNew(uEntity);
        if (uCondition) {
            (void) DDS::Entity::_duplicate (entity);
            this->entity = entity;
            setDomainId(entity->getDomainId());
        } else {
            result = DDS::RETCODE_ERROR;
            CPP_REPORT(result, "Could not create StatusCondition.");
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::wlReq_deinit()
{
    DDS::ReturnCode_t result;
    result = DDS::OpenSplice::Condition::wlReq_deinit();
    if (result == DDS::RETCODE_OK)
    {
        if (this->uCondition) {
            result = uResultToReturnCode(u_objectClose (u_object (this->uCondition)));
        }
        if (this->entity) {
            DDS::release (this->entity);
            entity = NULL;
        }
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::attachToWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result;

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        if (!this->deinitializing) {
            if (!waitsets->containsElement(waitset)) {
                /* Don't _duplicate the waitset to avoid cyclic references messing up the garbage collection;
                 * it will detach itself when it is destructed. */
                result = waitset->wlReq_attachGeneralCondition(this, u_observable(uCondition));
                if (result == DDS::RETCODE_OK) {
                    DDS::Boolean insertOK;
                    insertOK = waitsets->insertElement(waitset);
                    if (!insertOK) {
                        result = DDS::RETCODE_OUT_OF_RESOURCES;
                    }
                    assert(insertOK);
                }
            } else {
                result = DDS::RETCODE_OK;
            }
        } else {
            /* Do not allow a Condition that is already deinitializing to be attached to a WaitSet. */
            result = DDS::RETCODE_ALREADY_DELETED;
        }
        this->unlock ();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::wlReq_detachFromWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (waitsets->removeElement(waitset)) {
        if (uCondition) {
            result = waitset->wlReq_detachGeneralCondition(this, u_observable(uCondition));
        }
    } else {
        /* Unable to take the given waitset is not a problem when de-initializing. */
        if (!this->deinitializing) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "This StatusCondition is being deleted.");
        }
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::detachFromWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result;

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_detachFromWaitset(waitset);
        this->unlock ();
    }
    return result;
}

DDS::StatusMask
DDS::OpenSplice::StatusCondition::get_enabled_statuses (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::StatusMask mask;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        mask = this->enabledStatusMask;
    } else {
        mask = 0;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return mask;
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::set_enabled_statuses (
  DDS::StatusMask statusMask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_result uResult;
    v_eventMask vMask;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK && uCondition) {
        vMask = DDS::OpenSplice::Utils::vEventMaskFromStatusMask (statusMask);
        uResult = u_statusCondition_set_mask(uCondition, vMask);
        result = uResultToReturnCode(uResult);
        if (result == DDS::RETCODE_OK) {
            this->enabledStatusMask = statusMask;
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Entity_ptr
DDS::OpenSplice::StatusCondition::get_entity (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Entity_ptr entity = NULL;
    DDS::OpenSplice::Entity *osplEntity;

    CPP_REPORT_STACK();

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        osplEntity = dynamic_cast<DDS::OpenSplice::Entity*>(this->entity);
        if (osplEntity) {
            /* Lock the entity to detect if it is deleted or not. */
            result = osplEntity->read_lock();
            if (result == DDS::RETCODE_OK) {
                entity = DDS::Entity::_duplicate (this->entity);
                osplEntity->unlock();
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return entity;
}

DDS::Boolean
DDS::OpenSplice::StatusCondition::get_trigger_value (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    c_ulong triggerValue;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        assert (this->uCondition != NULL);
        u_statusCondition_get_triggerValue(this->uCondition, &triggerValue);
    } else {
        triggerValue = 0;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return (triggerValue > 0);
}

u_statusCondition
DDS::OpenSplice::StatusCondition::get_user_object()
{
    DDS::ReturnCode_t result;
    u_statusCondition uCondition = NULL;

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        assert (this->uCondition != NULL);
        uCondition = this->uCondition;
    }

    return uCondition;
}

DDS::ReturnCode_t
DDS::OpenSplice::StatusCondition::isAlive()
{
    return DDS::OpenSplice::Utils::observableExists(u_observable(this->uCondition));
}
