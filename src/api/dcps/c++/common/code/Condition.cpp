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
#include "Condition.h"
#include "GuardCondition.h"
#include "WaitSet.h"
#include "MiscUtils.h"

DDS::OpenSplice::Condition::Condition(DDS::OpenSplice::ObjectKind kind) :
    DDS::OpenSplice::CppSuperClass(kind),
    waitsets(new ObjSet(FALSE)),
    deinitializing(FALSE)
{
    /* do nothing */
}

DDS::OpenSplice::Condition::~Condition()
{
    delete waitsets;
}

DDS::ReturnCode_t
DDS::OpenSplice::Condition::nlReq_init()
{
    DDS::ReturnCode_t result;

    result = waitsets->init();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Condition::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::WaitSet *waitset;
    DDS::ObjSeq_var wsObjects;
    DDS::ULong length;

    this->deinitializing = TRUE;
    wsObjects = waitsets->getObjSeq();
    length = wsObjects->length();
    for (DDS::ULong i = 0; (i < length) && (result == DDS::RETCODE_OK); i++) {
        waitset = dynamic_cast<DDS::WaitSet *>(wsObjects[i].in());
        assert(waitset);
        /* The waitset->detach_condition() wants access to this condition,
         * which means we have to unlock it for this small while to avoid
         * a potential cyclic deadlock. */
        this->unlock();
        result = waitset->detach_condition (this);
        (void)this->force_write_lock();
    }
    if (result == DDS::RETCODE_OK) {
        waitsets->clear();
        result = waitsets->deinit();
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::CppSuperClass::wlReq_deinit ();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Condition::isAlive()
{
    return DDS::RETCODE_OK;
}
