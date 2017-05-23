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
#include "Condition.h"
#include "GuardCondition.h"
#include "WaitSet.h"
#include "ReportUtils.h"

DDS::GuardCondition::GuardCondition () :
    DDS::OpenSplice::Condition(DDS::OpenSplice::GUARDCONDITION),
    triggerValue(FALSE)
{
    // Assume init always succeed, since there is no way to communicate a failure.
    (void) init();
}

DDS::GuardCondition::~GuardCondition ()
{
    deinit();
}

DDS::ReturnCode_t
DDS::GuardCondition::init()
{
    return nlReq_init();
}

DDS::ReturnCode_t
DDS::GuardCondition::nlReq_init()
{
    return DDS::OpenSplice::Condition::nlReq_init();
}

DDS::ReturnCode_t
DDS::GuardCondition::wlReq_deinit()
{
    return DDS::OpenSplice::Condition::wlReq_deinit();
}

DDS::ReturnCode_t
DDS::GuardCondition::attachToWaitset (
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
                result = waitset->wlReq_attachGuardCondition(this);
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
DDS::GuardCondition::wlReq_detachFromWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (waitsets->removeElement(waitset)) {
        result = waitset->wlReq_detachGuardCondition(this);
    } else {
        /* Unable to take the given waitset is not a problem when de-initializing. */
        if (!this->deinitializing) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "This GuardCondition is being deleted.");
        }
    }
    return result;
}

DDS::ReturnCode_t
DDS::GuardCondition::detachFromWaitset (
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

DDS::Boolean
DDS::GuardCondition::get_trigger_value (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::Boolean triggerValue;

    CPP_REPORT_STACK();

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        triggerValue = this->triggerValue;
        this->unlock ();
    } else {
        triggerValue = FALSE;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return triggerValue;
}

DDS::ReturnCode_t
DDS::GuardCondition::set_trigger_value (
    DDS::Boolean value
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::ObjSeq_var wsList; // _var type will will clean sequence up afterwards.

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        DDS::ULong length;

        this->triggerValue = value;
        /* A copy of the list of waitsets is made to be triggered after
         * the condition is released. Triggering a Waitset will lock the
         * Waitset whereas the Waitset may lock the condition to get the
         * trigger value. So If the Condition is locked when triggering
         * a Waitset a deadlock may occur if the Waitset simultaneously
         * tries to get the conditions trigger value, by first taking a
         * copy of all waitsets and then releasing the condition before
         * triggering the Waitsets this situation is avoided.
         */
        wsList = waitsets->getObjSeq();
        this->unlock();
        length = wsList->length();
        for (DDS::ULong i = 0; i < length; i++)
        {
            DDS::WaitSet *ws = dynamic_cast<DDS::WaitSet *>(wsList[i].in());
            if (ws) {
                ws->trigger(this);
            } else {
                result = DDS::RETCODE_ERROR;
            }
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}
