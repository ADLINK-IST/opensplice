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
#include "u_user.h"
#include "Domain.h"
#include "MiscUtils.h"
#include "ReportUtils.h"

DDS::OpenSplice::Domain::Domain(
) : DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::__DOMAIN),
    uDomain(NULL)
{
    /* do nothing */
}

DDS::OpenSplice::Domain::~Domain()
{
    /* do nothing */
}

DDS::ReturnCode_t
DDS::OpenSplice::Domain::init (
    DDS::DomainId_t domainId)
{
    return  nlReq_init(domainId);
}

DDS::ReturnCode_t
DDS::OpenSplice::Domain::nlReq_init (
    DDS::DomainId_t domainId)
{
    DDS::ReturnCode_t result;
    u_domain uDomain;
    u_result uResult;

    result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        uResult = u_domainOpen(&uDomain, NULL, (u_domainId_t)domainId, U_DOMAIN_DEFAULT_TIMEOUT);
        result = uResultToReturnCode(uResult);
        if (result == DDS::RETCODE_OK) {
            this->uDomain = uDomain;
        } else {
            CPP_REPORT(result, "Could not open Domain with DomainId_t '%d'.",
                domainId);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Domain::wlReq_deinit ()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_result uResult;

    if(this->uDomain) {
        uResult = u_domainClose(uDomain);
        result = uResultToReturnCode(uResult);
    }

    if (result == DDS::RETCODE_OK) {
        this->uDomain = NULL;
        result = DDS::OpenSplice::CppSuperClass::wlReq_deinit();
    }
    return result;
}

DDS::DomainId_t
DDS::OpenSplice::Domain::get_domain_id () THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_domainId_t domainId = DDS::DOMAIN_ID_INVALID;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        domainId = u_domainId(this->uDomain);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return static_cast<DDS::DomainId_t>(domainId);
}

DDS::ReturnCode_t
DDS::OpenSplice::Domain::create_persistent_snapshot (
    const DDS::Char *partition_expression,
    const DDS::Char *topic_expression,
    const DDS::Char *URI) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_result uResult;

    CPP_REPORT_STACK();

    if (partition_expression == NULL) {
        CPP_REPORT(result, "partition_expression '<NULL>' is invalid.");
    } else if (topic_expression == NULL) {
        CPP_REPORT(result, "topic_expression '<NULL>' is invalid.");
    } else if (URI == NULL) {
        CPP_REPORT(result, "URI '<NULL>' is invalid.");
    } else {
        result = this->check ();
    }

    if (result == DDS::RETCODE_OK) {
        uResult = u_domainCreatePersistentSnapshot (
            this->uDomain, partition_expression, topic_expression, URI);
        result = uResultToReturnCode(uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not create persistent snapshot of Domain.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}
