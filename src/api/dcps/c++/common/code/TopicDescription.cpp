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
#include "TopicDescription.h"

#include "os_report.h"
#include "u_user.h"
#include "DomainParticipant.h"
#include "TypeSupportMetaHolder.h"
#include "ReportUtils.h"

DDS::OpenSplice::TopicDescription::TopicDescription() :
    tsMetaHolder(NULL),
    nrUsers(0),
    participant(NULL)
{
    // topic_name, type_name and expression are of type String_var and therefore do not need
    // explicit initialization, since the constructor of the String_var already takes care of that.
}

DDS::ReturnCode_t
DDS::OpenSplice::TopicDescription::nlReq_init(
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    const DDS::Char *type_name,
    const DDS::Char *expression,
    DDS::OpenSplice::TypeSupportMetaHolder *ts_meta_holder)
{
    assert (participant != NULL);
    assert (topic_name != NULL);
    assert (type_name != NULL);
    assert (expression != NULL);

    this->topic_name = DDS::string_dup(topic_name);
    this->type_name  = DDS::string_dup(type_name);
    this->expression = DDS::string_dup(expression);

    (void) DDS::DomainParticipant::_duplicate(participant);
    this->participant = participant;

    if (ts_meta_holder != NULL) {
        (void) DDS::Object::_duplicate(ts_meta_holder);
        this->tsMetaHolder = ts_meta_holder;
    }

    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::TopicDescription::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (this->nrUsers != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "TopicDescription still in use.");
    } else {
        DDS::release(this->participant);
        this->participant = NULL;

        if (this->tsMetaHolder != NULL) {
            DDS::release(this->tsMetaHolder);
            this->tsMetaHolder = NULL;
        }
    }

    return result;
}


DDS::OpenSplice::TopicDescription::~TopicDescription()
{
    // Delegate to parent.
}

char*
DDS::OpenSplice::TopicDescription::get_type_name()
{
    DDS::ReturnCode_t result;
    char* name = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        if (this->type_name != NULL ) {
            name = DDS::string_dup(this->type_name);
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return name;
}

char*
DDS::OpenSplice::TopicDescription::get_name()
{
    DDS::ReturnCode_t result;
    char* name = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        if (this->topic_name != NULL ) {
            name = DDS::string_dup(this->topic_name);
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return name;
}

DDS::DomainParticipant_ptr
DDS::OpenSplice::TopicDescription::get_participant()
{
    DDS::ReturnCode_t result;
    DDS::DomainParticipant_ptr participant = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        participant = DDS::DomainParticipant::_duplicate(this->participant);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return participant;
}

DDS::OpenSplice::TypeSupportMetaHolder*
DDS::OpenSplice::TopicDescription::get_typesupport_meta_holder()
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::TypeSupportMetaHolder *meta_holder = NULL;

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->tsMetaHolder == NULL) {
            /* The only way that tsMetaHolder == NULL here, is when the topic
             * proxy is obtained through find_topic() on the participant. */
            result = this->participant->nlReq_find_type_support_meta_holder(this->type_name, this->tsMetaHolder);
        }
        if (this->tsMetaHolder != NULL) {
            meta_holder = this->tsMetaHolder;
            DDS::Object::_duplicate(meta_holder);
        }
        this->unlock();
    }

    return meta_holder;
}

const char *
DDS::OpenSplice::TopicDescription::rlReq_get_topic_expression ()
{
    return this->expression;
}

void
DDS::OpenSplice::TopicDescription::wlReq_incrNrUsers()
{
    this->nrUsers++;
}

void
DDS::OpenSplice::TopicDescription::wlReq_decrNrUsers()
{
    this->nrUsers--;
}

::DDS::Long
DDS::OpenSplice::TopicDescription::rlReq_getNrUsers()
{
    return this->nrUsers;
}

const char *
DDS::OpenSplice::TopicDescription::rlReq_getName()
{
    return this->topic_name;
}
