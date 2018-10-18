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
#include "DomainParticipant.h"
#include "TypeSupport.h"
#include "ReportUtils.h"

DDS::OpenSplice::TypeSupport::TypeSupport() :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::TYPESUPPORT)
{
    /* TODO: This function is a mock; implement properly. */
    DDS::OpenSplice::CppSuperClass::nlReq_init();
}

DDS::OpenSplice::TypeSupport::~TypeSupport()
{
    (void) deinit();
}

DDS::ReturnCode_t
DDS::OpenSplice::TypeSupport::wlReq_deinit()
{
    return DDS::OpenSplice::CppSuperClass::wlReq_deinit();
}

DDS::ReturnCode_t
DDS::OpenSplice::TypeSupport::register_type(
    DDS::DomainParticipant_ptr domain,
    const char * type_name) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DomainParticipant *participant;

    CPP_REPORT_STACK();

    if (domain == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "domain '<NULL>' is invalid.");
    } else if (type_name != NULL && strlen(type_name) == 0) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "type_name '' is invalid.");
    } else {
        participant =
            dynamic_cast<DDS::OpenSplice::DomainParticipant *>(domain);
        if (participant == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "domain is invalid, not of type '%s'.",
                "DDS::OpenSplice::DomainParticipant");
        }
    }

    if (result == DDS::RETCODE_OK) {
        if (type_name == NULL) {
            type_name = tsMetaHolder->get_type_name();
        }
        result = participant->nlReq_load_type_support_meta_holder(tsMetaHolder, type_name);
        /* If the participant is already deleted, then it means we received an bad parameter. */
        if (result == RETCODE_ALREADY_DELETED) {
            result = RETCODE_BAD_PARAMETER;
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

char *
DDS::OpenSplice::TypeSupport::get_type_name() THROW_ORB_EXCEPTIONS
{
    return DDS::string_dup(tsMetaHolder->get_type_name());
}

DDS::OpenSplice::TypeSupportMetaHolder *
DDS::OpenSplice::TypeSupport::get_metaHolder()
{
    return tsMetaHolder;
}
