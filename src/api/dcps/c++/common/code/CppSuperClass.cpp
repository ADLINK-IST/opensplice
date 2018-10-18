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
#include "u_user.h"

#include "CppSuperClass.h"
#include "ReportUtils.h"

#define HMM_MAGIC       (0xabcdefed)

#define BAD_PARAMETER(x)                            \
    ((x->magic != HMM_MAGIC) ||                     \
     (x->objKind <= DDS::OpenSplice::UNDEFINED) ||  \
     (x->objKind >= DDS::OpenSplice::OBJECT_COUNT))

#define ALREADY_DELETED(x)                          \
     (x->deinitialized)



DDS::OpenSplice::CppSuperClassInterface::~CppSuperClassInterface() { }

DDS::OpenSplice::CppSuperClass::CppSuperClass(ObjectKind kind) :
    magic(HMM_MAGIC),
    objKind(kind),
    deinitialized(FALSE),
    domainId(-1)
{
    os_result osr;

    switch (kind) {
    case DDS::OpenSplice::DOMAINPARTICIPANTFACTORY:
    case DDS::OpenSplice::WAITSET:
    case DDS::OpenSplice::GUARDCONDITION:
    case DDS::OpenSplice::TYPESUPPORT:
        os_osInit();
        break;
    default:
        break;
    }

    osr = os_mutexInit (&this->mutex, NULL);
    if (osr == os_resultSuccess) {
        osr = os_condInit(&this->cond, &this->mutex, NULL);
        if (osr != os_resultSuccess) {
            CPP_PANIC("Could not initialize condition variable.");
        }
    } else {
        CPP_PANIC("Could not initialize mutex.");
    }
}

DDS::OpenSplice::CppSuperClass::~CppSuperClass()
{
    os_condDestroy (&this->cond);
    os_mutexDestroy (&this->mutex);

    switch (objKind) {
    case DDS::OpenSplice::DOMAINPARTICIPANTFACTORY:
    case DDS::OpenSplice::WAITSET:
    case DDS::OpenSplice::GUARDCONDITION:
    case DDS::OpenSplice::TYPESUPPORT:
        os_osExit();
        break;
    default:
        break;
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::nlReq_init()
{
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::deinit()
{
    DDS::ReturnCode_t result;

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_deinit();
        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::wlReq_deinit()
{
    this->deinitialized = TRUE;
    return DDS::RETCODE_OK;
}

DDS::OpenSplice::ObjectKind
DDS::OpenSplice::CppSuperClass::get_kind()
{
    DDS::OpenSplice::ObjectKind kind = DDS::OpenSplice::UNDEFINED;

    if (this->read_lock() == DDS::RETCODE_OK) {
        kind = this->objKind;
        this->unlock();
    }
    return kind;
}

DDS::OpenSplice::ObjectKind
DDS::OpenSplice::CppSuperClass::rlReq_get_kind()
{
    return this->objKind;
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::read_lock()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    os_result osr;

    osr = os_mutexLock_s (&this->mutex);
    if (osr == os_resultSuccess) {
        if (BAD_PARAMETER(this)) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_PANIC("Object state is corrupted.");
        } else if (ALREADY_DELETED(this)) {
            result = DDS::RETCODE_ALREADY_DELETED;
        }

        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Entity not available");
            os_mutexUnlock(&this->mutex);
        }
    } else {
        result = DDS::RETCODE_ERROR;
        CPP_REPORT(result, "Could not read-lock mutex.");
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::write_lock()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    os_result osr;

    osr = os_mutexLock_s (&this->mutex);
    if (osr == os_resultSuccess) {
        if (BAD_PARAMETER(this)) {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else if (ALREADY_DELETED(this)) {
            result = DDS::RETCODE_ALREADY_DELETED;
        }

        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Entity not available");
            os_mutexUnlock(&this->mutex);
        }
    } else {
        result = DDS::RETCODE_ERROR;
        CPP_REPORT(result, "Could not write-lock mutex.");
    }

    return result;
}

void
DDS::OpenSplice::CppSuperClass::force_write_lock()
{
    os_mutexLock (&this->mutex);
}

void
DDS::OpenSplice::CppSuperClass::unlock ()
{
    if (BAD_PARAMETER(this)) {
        CPP_PANIC("Object is not initialized.");
    }
    os_mutexUnlock(&this->mutex);
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::wlReq_wait()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    os_duration timeout = 10*OS_DURATION_SECOND;
    os_result osr;

    osr = os_condTimedWait(&this->cond, &this->mutex, timeout);
    if (osr == os_resultTimeout) {
        result = DDS::RETCODE_TIMEOUT;
        CPP_REPORT(result, "Object timed wait timed-out.");
    } else if (osr != os_resultSuccess) {
        result = DDS::RETCODE_ERROR;
        CPP_REPORT(result, "Object timed wait failed.");
    }
    return result;
}

void
DDS::OpenSplice::CppSuperClass::wlReq_trigger()
{
    (void)os_condBroadcast(&this->cond);
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::check()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (BAD_PARAMETER(this)) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_PANIC("Object is not initialized.");
    } else if (ALREADY_DELETED(this)) {
        result = DDS::RETCODE_ALREADY_DELETED;
        CPP_REPORT(result, "Object is already deleted.");
    }

    return result;
}

os_int32
DDS::OpenSplice::CppSuperClass::getDomainId()
{
    return domainId;
}

void
DDS::OpenSplice::CppSuperClass::setDomainId(
    os_int32 _domainId)
{
    this->domainId = _domainId;
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::uResultToReturnCode(
    u_result uResult)
{
    DDS::ReturnCode_t result;

    switch (uResult) {
    case U_RESULT_OK:
        result = DDS::RETCODE_OK;
    break;
    case U_RESULT_OUT_OF_MEMORY:
        result = DDS::RETCODE_OUT_OF_RESOURCES;
    break;
    case U_RESULT_ILL_PARAM:
    case U_RESULT_HANDLE_EXPIRED:
        result = DDS::RETCODE_BAD_PARAMETER;
    break;
    case U_RESULT_CLASS_MISMATCH:
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
    break;
    case U_RESULT_DETACHING:
        result = DDS::RETCODE_ALREADY_DELETED;
    break;
    case U_RESULT_TIMEOUT:
        result = DDS::RETCODE_TIMEOUT;
    break;
    case U_RESULT_OUT_OF_RESOURCES:
        result = DDS::RETCODE_OUT_OF_RESOURCES;
    break;
    case U_RESULT_INCONSISTENT_QOS:
        result = DDS::RETCODE_INCONSISTENT_POLICY;
    break;
    case U_RESULT_IMMUTABLE_POLICY:
        result = DDS::RETCODE_IMMUTABLE_POLICY;
    break;
    case U_RESULT_PRECONDITION_NOT_MET:
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
    break;
    case U_RESULT_ALREADY_DELETED:
        result = DDS::RETCODE_ALREADY_DELETED;
    break;
    case U_RESULT_UNSUPPORTED:
        result = DDS::RETCODE_UNSUPPORTED;
    break;
    case U_RESULT_NOT_INITIALISED:
        result = DDS::RETCODE_NOT_ENABLED;
    break;
    case U_RESULT_NO_DATA:
        result = DDS::RETCODE_NO_DATA;
    break;
    case U_RESULT_UNDEFINED:
    case U_RESULT_INTERRUPTED:
    case U_RESULT_INTERNAL_ERROR:
    default:
        result = DDS::RETCODE_ERROR;
    break;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::CppSuperClass::osResultToReturnCode (
    os_result osResult)
{
    DDS::ReturnCode_t result;

    switch (osResult) {
    case os_resultSuccess:
        result = DDS::RETCODE_OK;
    break;
    case os_resultUnavailable:
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
    break;
    case os_resultTimeout:
        result = DDS::RETCODE_TIMEOUT;
    break;
    case os_resultBusy:
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
    break;
    case os_resultInvalid:
        result = DDS::RETCODE_BAD_PARAMETER;
    break;
    case os_resultFail:
    default:
        result = DDS::RETCODE_ERROR;
    break;
    }
    return result;
}

u_result
DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(
    DDS::ReturnCode_t result)
{
    u_result uResult;

    switch(result) {
    case DDS::RETCODE_OK:
        uResult = U_RESULT_OK;
    break;
    case DDS::RETCODE_UNSUPPORTED:
        uResult = U_RESULT_UNSUPPORTED;
    break;
    case DDS::RETCODE_BAD_PARAMETER:
        uResult = U_RESULT_ILL_PARAM;
    break;
    case DDS::RETCODE_PRECONDITION_NOT_MET:
        uResult = U_RESULT_CLASS_MISMATCH;
    break;
    case DDS::RETCODE_OUT_OF_RESOURCES:
        uResult = U_RESULT_OUT_OF_RESOURCES;
    break;
    /*case DDS::NOT_ENABLED:
        uResult = U_RESULT_NOT_ENABLED;
    break;*/
    case DDS::RETCODE_IMMUTABLE_POLICY:
        uResult = U_RESULT_IMMUTABLE_POLICY;
    break;
    case DDS::RETCODE_INCONSISTENT_POLICY:
        uResult = U_RESULT_INCONSISTENT_QOS;
    break;
    case DDS::RETCODE_ALREADY_DELETED:
        uResult = U_RESULT_ALREADY_DELETED;
    break;
    case DDS::RETCODE_TIMEOUT:
        uResult = U_RESULT_TIMEOUT;
    break;
    case DDS::RETCODE_ERROR:
    case DDS::RETCODE_NO_DATA:
    default:
        uResult = U_RESULT_INTERNAL_ERROR;
    break;
    }

    return uResult;
}

#undef ALREADY_DELETED
#undef BAD_PARAMETER
#undef HMM_MAGIC
