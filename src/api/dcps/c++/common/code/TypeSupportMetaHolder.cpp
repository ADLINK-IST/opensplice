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
#include "DomainParticipant.h"
#include "TypeSupportMetaHolder.h"

DDS::OpenSplice::TypeSupportMetaHolder::TypeSupportMetaHolder(
        const char *typeName,
        const char *internalTypeName,
        const char *keyList) :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::TYPESUPPORTMETAHOLDER)
{
    DDS::OpenSplice::CppSuperClass::nlReq_init();

    this->typeName = typeName;
    // In case no explicit internalTypeName is set, then it is equal to the normal typeName.
    if (strlen(internalTypeName) == 0) {
        this->internalTypeName = typeName;
    } else {
        this->internalTypeName = internalTypeName;
    }
    this->keyList = keyList;
}


DDS::OpenSplice::TypeSupportMetaHolder::~TypeSupportMetaHolder()
{
    delete[] metaDescriptor;
    (void) deinit();
}

DDS::ReturnCode_t
DDS::OpenSplice::TypeSupportMetaHolder::wlReq_deinit()
{
    return DDS::OpenSplice::CppSuperClass::wlReq_deinit();
}

char *
DDS::OpenSplice::TypeSupportMetaHolder::get_meta_descriptor()
{
    ::DDS::ULong i;
    char *descriptorString;

    descriptorString = DDS::string_alloc(metaDescriptorLength);
    descriptorString[0] = '\0';
    for ( i= 0; i < metaDescriptorArrLength; i++) {
        os_strcat(descriptorString, metaDescriptor[i]);
    }
    return descriptorString;
}

const char *
DDS::OpenSplice::TypeSupportMetaHolder::get_key_list()
{
    return keyList;
}

const char *
DDS::OpenSplice::TypeSupportMetaHolder::get_type_name()
{
    return typeName;
}

const char *
DDS::OpenSplice::TypeSupportMetaHolder::get_internal_type_name()
{
    return internalTypeName;
}

DDS::OpenSplice::cxxCopyIn
DDS::OpenSplice::TypeSupportMetaHolder::get_copy_in()
{
    return copyIn;
}

DDS::OpenSplice::cxxCopyOut
DDS::OpenSplice::TypeSupportMetaHolder::get_copy_out()
{
    return copyOut;
}
